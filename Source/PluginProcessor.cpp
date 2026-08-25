#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

static const std::vector<std::vector<int>> SCALES = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, 
    {0, 2, 4, 5, 7, 9, 11},                 
    {0, 2, 3, 5, 7, 8, 10},                 
    {0, 2, 3, 5, 7, 9, 10},                 
    {0, 1, 3, 5, 7, 8, 10},                 
    {0, 2, 4, 6, 7, 9, 11},                 
    {0, 2, 4, 5, 7, 9, 10},                 
    {0, 2, 4, 7, 9},                        
    {0, 3, 5, 7, 10},                       
    {0, 2, 3, 5, 7, 8, 11}                  
};

static float quantize_pitch(float raw_midi, int scale_idx) {
    if (scale_idx < 1) scale_idx = 1;
    if (scale_idx > 10) scale_idx = 10;
    const auto& scale = SCALES[scale_idx - 1];
    
    int midi_int = (int)std::round(raw_midi);
    int octave = midi_int / 12;
    int note = midi_int % 12;
    
    int closest_note = scale[0];
    int min_dist = 100;
    for (int s : scale) {
        int dist = std::abs(note - s);
        if (dist < min_dist) {
            min_dist = dist;
            closest_note = s;
        }
    }
    return (float)(octave * 12 + closest_note);
}

static float mtof(float midi) { return 440.0f * std::pow(2.0f, (midi - 69.0f) / 12.0f); }

float WestCoastVoice::process(float& outL, float& outR, const TrackParams& params, float chaos_val, int global_scale) {
    if (!params.pitch) return 0.0f; // Salvavidas
    
    float pitch = params.pitch->load();
    float drop = params.drop->load();
    float morph = params.morph->load();
    float fold_amt = params.fold->load();
    float fm_mod = params.fm->load();
    float rise = params.rise->load();
    float fall = params.fall->load();
    float resp = params.resp->load();
    float brgt = params.brgt->load(); // Actualmente sin filtro implementado, se usa como VCA
    float noise = params.noise->load();
    float vol = params.vol->load();
    bool note_mode = params.notemode->load() > 0.5f;

    float rise_rate = 1.0f / (std::max(0.001f, rise) * sr);
    float fall_rate = 1.0f / (std::max(0.001f, fall) * sr);

    if (env_stage == 1) {
        env += rise_rate;
        if (env >= 1.0f) { env = 1.0f; env_stage = 2; }
    } else if (env_stage == 2) {
        env -= fall_rate;
        if (env <= 0.0f) { env = 0.0f; env_stage = 0; }
    }

    // Cuantizar solo si está en modo nota
    float current_pitch = note_mode ? quantize_pitch(pitch, global_scale) : pitch;
    float freq = mtof(current_pitch);
    
    if (drop > 0.01f) {
        freq *= (1.0f + env * drop * 2.0f);
    }
    
    if (chaos_val > 0.01f) {
        float chaos_mod = ((float)rand() / (float)RAND_MAX - 0.5f) * chaos_val * 0.1f;
        freq *= (1.0f + chaos_mod);
        fold_amt *= (1.0f + chaos_mod);
    }

    float phase_inc = freq / sr;
    phase += phase_inc;
    if (phase > 1.0f) phase -= 1.0f;

    float tri = 4.0f * std::abs(phase - 0.5f) - 1.0f;
    float sqr = phase < 0.5f ? 1.0f : -1.0f;
    float sig = tri * (1.0f - morph) + sqr * morph;
    
    if (fm_mod > 0.01f) {
        float fm_phase = phase + (sig * fm_mod * 0.5f);
        if (fm_phase > 1.0f) fm_phase -= 1.0f;
        if (fm_phase < 0.0f) fm_phase += 1.0f;
        sig = 4.0f * std::abs(fm_phase - 0.5f) - 1.0f;
    }
    
    sig *= (1.0f + fold_amt * 6.0f);
    if (sig > 1.0f || sig < -1.0f) {
        sig = std::sin(sig * juce::MathConstants<float>::halfPi);
    }
    
    if (noise > 0.01f) {
        float n_val = ((float)rand() / (float)RAND_MAX * 2.0f) - 1.0f;
        sig = sig * (1.0f - noise) + n_val * noise;
    }

    float vactrol_open  = resp * 0.5f;   
    float vactrol_close = 0.05f + resp * 0.2f;
    float vactrol_speed = (env > lpg_state) ? vactrol_open : vactrol_close;
    lpg_state += (env - lpg_state) * vactrol_speed;
    
    sig *= lpg_state; 
    
    outL += sig * vol;
    outR += sig * vol;
    
    return sig;
}

juce::AudioProcessorValueTreeState::ParameterLayout OrbitaLPGAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("master_vol", "Master Vol", 0.0f, 1.0f, 0.8f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("master_drive", "Drive", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("bpm", "BPM", 20.0f, 300.0f, 120.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("swing", "Swing", 0.0f, 0.5f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("chaos", "Chaos", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterInt>("global_scale", "Global Scale", 1, 10, 1));
    layout.add(std::make_unique<juce::AudioParameterBool>("is_playing", "Playing", false));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("echo_time", "Echo Time", 0.05f, 1.0f, 0.3f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("echo_fdbk", "Echo Fdbk", 0.0f, 0.9f, 0.4f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("echo_mix", "Echo Mix", 0.0f, 1.0f, 0.3f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("echo_hpf", "Echo HPF", 20.0f, 2000.0f, 20.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("echo_lpf", "Echo LPF", 1000.0f, 20000.0f, 20000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("echo_wow", "Echo Wow", 0.0f, 1.0f, 0.1f));

    for(int i=1; i<=6; i++) {
        juce::String t = juce::String(i);
        layout.add(std::make_unique<juce::AudioParameterInt>("t"+t+"_steps", "T"+t+" Steps", 1, 32, 16));
        layout.add(std::make_unique<juce::AudioParameterInt>("t"+t+"_pulses", "T"+t+" Pulses", 0, 32, 4));
        layout.add(std::make_unique<juce::AudioParameterInt>("t"+t+"_offset", "T"+t+" Offset", 0, 32, 0));
        
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_pitch", "T"+t+" Pitch", 24.0f, 96.0f, 60.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_drop", "T"+t+" P.Drop", 0.0f, 1.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_morph", "T"+t+" Morph", 0.0f, 1.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_fold", "T"+t+" Fold", 0.0f, 1.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_fm", "T"+t+" FM Mod", 0.0f, 1.0f, 0.0f));
        
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_rise", "T"+t+" Rise", 0.001f, 1.0f, 0.01f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_fall", "T"+t+" Fall", 0.01f, 3.0f, 0.5f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_resp", "T"+t+" Resp", 0.05f, 1.0f, 0.5f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_brgt", "T"+t+" Brgt", 0.05f, 1.0f, 0.8f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_noise", "T"+t+" Noise", 0.0f, 1.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_vol", "T"+t+" Vol", 0.0f, 1.0f, 0.8f));
        
        // Nuevo parámetro de Note Mode
        layout.add(std::make_unique<juce::AudioParameterBool>("t"+t+"_notemode", "T"+t+" Note Mode", true));
    }
    
    return layout;
}

OrbitaLPGAudioProcessor::OrbitaLPGAudioProcessor()
     : AudioProcessor (BusesProperties().withOutput("Master", juce::AudioChannelSet::stereo(), true)),
       apvts(*this, nullptr, "PARAMETERS", createParameterLayout()),
       oversampler(2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true)
{
    // CACHEO DE PARÁMETROS PARA EVITAR BUSQUEDAS EN EL HILO DE AUDIO
    for(int i=0; i<6; ++i) {
        juce::String t = juce::String(i+1);
        tParams[i].steps = apvts.getRawParameterValue("t"+t+"_steps");
        tParams[i].pulses = apvts.getRawParameterValue("t"+t+"_pulses");
        tParams[i].offset = apvts.getRawParameterValue("t"+t+"_offset");
        tParams[i].pitch = apvts.getRawParameterValue("t"+t+"_pitch");
        tParams[i].drop = apvts.getRawParameterValue("t"+t+"_drop");
        tParams[i].morph = apvts.getRawParameterValue("t"+t+"_morph");
        tParams[i].fold = apvts.getRawParameterValue("t"+t+"_fold");
        tParams[i].fm = apvts.getRawParameterValue("t"+t+"_fm");
        tParams[i].rise = apvts.getRawParameterValue("t"+t+"_rise");
        tParams[i].fall = apvts.getRawParameterValue("t"+t+"_fall");
        tParams[i].resp = apvts.getRawParameterValue("t"+t+"_resp");
        tParams[i].brgt = apvts.getRawParameterValue("t"+t+"_brgt");
        tParams[i].noise = apvts.getRawParameterValue("t"+t+"_noise");
        tParams[i].vol = apvts.getRawParameterValue("t"+t+"_vol");
        tParams[i].notemode = apvts.getRawParameterValue("t"+t+"_notemode");
    }
    
    p_master_vol = apvts.getRawParameterValue("master_vol");
    p_bpm = apvts.getRawParameterValue("bpm");
    p_is_playing = apvts.getRawParameterValue("is_playing");
    p_chaos = apvts.getRawParameterValue("chaos");
    p_global_scale = apvts.getRawParameterValue("global_scale");
    p_echo_time = apvts.getRawParameterValue("echo_time");
    p_echo_fdbk = apvts.getRawParameterValue("echo_fdbk");
    p_echo_mix = apvts.getRawParameterValue("echo_mix");
    p_echo_wow = apvts.getRawParameterValue("echo_wow");
}

OrbitaLPGAudioProcessor::~OrbitaLPGAudioProcessor() {}

void OrbitaLPGAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    for (int i = 0; i < 6; i++) {
        voices[i].prepare(sampleRate);
    }
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 2;
    
    delayL.prepare(spec);
    delayR.prepare(spec);
}

void OrbitaLPGAudioProcessor::releaseResources() {}
bool OrbitaLPGAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const { return true; }

void OrbitaLPGAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    auto* playhead = getPlayHead();
    juce::AudioPlayHead::CurrentPositionInfo posInfo;
    
    bool is_playing = this->isPlaying;
    float bpm = p_bpm ? p_bpm->load() : 120.0f;
    
    if (playhead != nullptr && playhead->getCurrentPosition(posInfo)) {
        if (posInfo.isPlaying) is_playing = true;
        if (posInfo.bpm > 0.0) bpm = posInfo.bpm;
    }
    
    float chaos = p_chaos ? p_chaos->load() : 0.0f;
    int g_scale = p_global_scale ? (int)p_global_scale->load() : 1;

    double sr = getSampleRate();
    int samples_per_16th = (sr * 60.0) / (bpm * 4.0);
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        if (is_playing && seqEnabled) {
            if (step_samples_counter >= samples_per_16th) {
                step_samples_counter = 0;
                for(int t=0; t<6; ++t) {
                    int steps = tParams[t].steps ? (int)tParams[t].steps->load() : 16;
                    int pulses = tParams[t].pulses ? (int)tParams[t].pulses->load() : 4;
                    int offset = tParams[t].offset ? (int)tParams[t].offset->load() : 0;
                    
                    if (steps > 0) {
                        voices[t].current_step = (voices[t].current_step + 1) % steps;
                        
                        // Array estático (Reemplazo del std::vector para evitar reservar memoria aquí)
                        int pat[32] = {0}; 
                        if (pulses >= steps) { 
                            for(int i=0; i<steps && i<32; i++) pat[i] = 1;
                        }
                        else if (pulses > 0) {
                            int bucket = 0;
                            for (int i = 0; i < steps && i < 32; ++i) {
                                bucket += pulses;
                                if (bucket >= steps) { 
                                    bucket -= steps; 
                                    pat[(i + offset) % steps] = 1; 
                                }
                            }
                        }
                        
                        if (pat[voices[t].current_step] == 1) {
                            voices[t].trigger(60, 1.0f);
                        }
                    }
                }
            }
            step_samples_counter++;
        }

        float outL = 0.0f, outR = 0.0f;
        for (int t=0; t<6; ++t) {
            if (!trackMutes[t]) voices[t].process(outL, outR, tParams[t], chaos, g_scale);
        }
        
        float dTime = p_echo_time ? p_echo_time->load() : 0.3f;
        float dFdbk = p_echo_fdbk ? p_echo_fdbk->load() : 0.4f;
        float dMix  = p_echo_mix ? p_echo_mix->load() : 0.3f;
        float dWow  = p_echo_wow ? p_echo_wow->load() : 0.0f;

        float max_delay_samples = sr * 2.0f;
        float delay_samples = dTime * sr;
        
        if (dWow > 0.01f) {
            float lfo = std::sin(sample * 0.0005f);
            delay_samples += lfo * dWow * 20.0f; 
        }
        if (delay_samples > max_delay_samples) delay_samples = max_delay_samples;
        if (delay_samples < 1.0f) delay_samples = 1.0f;

        delayL.setDelay(delay_samples);
        delayR.setDelay(delay_samples);

        float dOutL = delayL.popSample(0);
        float dOutR = delayR.popSample(0); 

        delayL.pushSample(0, outL + dOutL * dFdbk);
        delayR.pushSample(0, outR + dOutR * dFdbk);

        outL = outL * (1.0f - dMix) + dOutL * dMix;
        outR = outR * (1.0f - dMix) + dOutR * dMix;

        float mVol = p_master_vol ? p_master_vol->load() : 0.8f;
        float masterOut = std::max(-1.0f, std::min(1.0f, outL * mVol)); 
        
        if (buffer.getNumChannels() > 0) buffer.addSample(0, sample, masterOut);
        if (buffer.getNumChannels() > 1) buffer.addSample(1, sample, std::max(-1.0f, std::min(1.0f, outR * mVol)));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new OrbitaLPGAudioProcessor();
}

juce::AudioProcessorEditor* OrbitaLPGAudioProcessor::createEditor() {
    return new OrbitaLPGAudioProcessorEditor(*this);
}

void OrbitaLPGAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void OrbitaLPGAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr) {
        if (xmlState->hasTagName(apvts.state.getType())) {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}