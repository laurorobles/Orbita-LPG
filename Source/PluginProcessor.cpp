#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

static std::vector<int> generate_euclidean(int pulses, int steps, int offset) {
    std::vector<int> pattern(steps, 0);
    if (steps == 0) return pattern;
    if (pulses >= steps) { std::fill(pattern.begin(), pattern.end(), 1); return pattern; }
    
    int bucket = 0;
    for (int i = 0; i < steps; ++i) {
        bucket += pulses;
        if (bucket >= steps) {
            bucket -= steps;
            pattern[(i + offset) % steps] = 1;
        }
    }
    return pattern;
}

// Escalas: 0: Cromatica, 1: Mayor, 2: Menor, 3: Dorico, 4: Frigio, 5: Lidio, 6: Mixolidio, 7: Pentatonica Mayor, 8: Pentatonica Menor, 9: Armonica Menor
static const std::vector<std::vector<int>> SCALES = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, // Chromatic
    {0, 2, 4, 5, 7, 9, 11},                 // Major
    {0, 2, 3, 5, 7, 8, 10},                 // Minor
    {0, 2, 3, 5, 7, 9, 10},                 // Dorian
    {0, 1, 3, 5, 7, 8, 10},                 // Phrygian
    {0, 2, 4, 6, 7, 9, 11},                 // Lydian
    {0, 2, 4, 5, 7, 9, 10},                 // Mixolydian
    {0, 2, 4, 7, 9},                        // Pentatonic Major
    {0, 3, 5, 7, 10},                       // Pentatonic Minor
    {0, 2, 3, 5, 7, 8, 11}                  // Harmonic Minor
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

float WestCoastVoice::process(float& outL, float& outR, juce::AudioProcessorValueTreeState& apvts, int trackIdx, float chaos_val, int global_scale) {
    juce::String t = juce::String(trackIdx + 1);
    
    // Check pointers to avoid crash
    auto* p_pitch = apvts.getRawParameterValue("t" + t + "_pitch");
    if (!p_pitch) return 0.0f; // Si los parametros no se han inicializado
    
    float pitch = p_pitch->load();
    float drop = apvts.getRawParameterValue("t" + t + "_drop")->load();
    float morph = apvts.getRawParameterValue("t" + t + "_morph")->load();
    float fold_amt = apvts.getRawParameterValue("t" + t + "_fold")->load();
    float fm_mod = apvts.getRawParameterValue("t" + t + "_fm")->load();
    float rise = apvts.getRawParameterValue("t" + t + "_rise")->load();
    float fall = apvts.getRawParameterValue("t" + t + "_fall")->load();
    float resp = apvts.getRawParameterValue("t" + t + "_resp")->load();
    float brgt = apvts.getRawParameterValue("t" + t + "_brgt")->load();
    float noise = apvts.getRawParameterValue("t" + t + "_noise")->load();
    float vol = apvts.getRawParameterValue("t" + t + "_vol")->load();

    // Envelopes
    float rise_rate = 1.0f / (std::max(0.001f, rise) * sr);
    float fall_rate = 1.0f / (std::max(0.001f, fall) * sr);

    if (env_stage == 1) {
        env += rise_rate;
        if (env >= 1.0f) { env = 1.0f; env_stage = 2; }
    } else if (env_stage == 2) {
        env -= fall_rate;
        if (env <= 0.0f) { env = 0.0f; env_stage = 0; }
    }

    // Pitch & Quantization
    float q_pitch = quantize_pitch(pitch, global_scale);
    float freq = mtof(q_pitch);
    
    // Pitch Drop env
    if (drop > 0.01f) {
        freq *= (1.0f + env * drop * 2.0f);
    }
    
    // Chaos Mod
    if (chaos_val > 0.01f) {
        float chaos_mod = ((float)rand() / (float)RAND_MAX - 0.5f) * chaos_val * 0.1f; // +/- 5% max
        freq *= (1.0f + chaos_mod);
        fold_amt *= (1.0f + chaos_mod);
    }

    float phase_inc = freq / sr;
    phase += phase_inc;
    if (phase > 1.0f) phase -= 1.0f;

    // Oscillator Morphing (Triangle to Square)
    float tri = 4.0f * std::abs(phase - 0.5f) - 1.0f;
    float sqr = phase < 0.5f ? 1.0f : -1.0f;
    float sig = tri * (1.0f - morph) + sqr * morph;
    
    // FM Mod (self feedback simplification)
    if (fm_mod > 0.01f) {
        float fm_phase = phase + (sig * fm_mod * 0.5f);
        if (fm_phase > 1.0f) fm_phase -= 1.0f;
        if (fm_phase < 0.0f) fm_phase += 1.0f;
        sig = 4.0f * std::abs(fm_phase - 0.5f) - 1.0f;
    }
    
    // Wavefolder
    sig *= (1.0f + fold_amt * 6.0f);
    if (sig > 1.0f || sig < -1.0f) {
        sig = std::sin(sig * juce::MathConstants<float>::halfPi);
    }
    
    // Noise Mix
    if (noise > 0.01f) {
        float n_val = ((float)rand() / (float)RAND_MAX * 2.0f) - 1.0f;
        sig = sig * (1.0f - noise) + n_val * noise;
    }

    // Vactrol LPG
    // resp: 0.05=very sluggish vactrol, 1.0=instant VCA
    // Map: fast open = env going up uses resp*0.5, slow close = fall handles it
    float vactrol_open  = resp * 0.5f;   // fast enough to hear attack
    float vactrol_close = 0.05f + resp * 0.2f;
    float vactrol_speed = (env > lpg_state) ? vactrol_open : vactrol_close;
    lpg_state += (env - lpg_state) * vactrol_speed;
    
    // Brgt = brightness/cutoff. Simple filter approx:
    sig *= lpg_state; // VCA part
    // (A real LPG also filters, this is a simplified VCA for now, we can add a 1-pole filter).
    
    outL += sig * vol;
    outR += sig * vol;
    
    return sig;
}


juce::AudioProcessorValueTreeState::ParameterLayout OrbitaLPGAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    // Global
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

    // Tracks
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
    }
    
    return layout;
}

OrbitaLPGAudioProcessor::OrbitaLPGAudioProcessor()
     : AudioProcessor (BusesProperties()
           .withOutput("Master", juce::AudioChannelSet::stereo(), true)
           .withOutput("T1", juce::AudioChannelSet::stereo(), false)
           .withOutput("T2", juce::AudioChannelSet::stereo(), false)
           .withOutput("T3", juce::AudioChannelSet::stereo(), false)
           .withOutput("T4", juce::AudioChannelSet::stereo(), false)
           .withOutput("T5", juce::AudioChannelSet::stereo(), false)
           .withOutput("T6", juce::AudioChannelSet::stereo(), false)),
       apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
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
    // Use processor's own play state (set by UI spacebar/button)
    bool is_playing = this->isPlaying;
    
    float bpm = 120.0f;
    if (auto* p = apvts.getRawParameterValue("bpm")) bpm = p->load();
    
    // Also sync to DAW transport if available
    if (playhead != nullptr && playhead->getCurrentPosition(posInfo)) {
        if (posInfo.isPlaying) is_playing = true;
        if (posInfo.bpm > 0.0) bpm = posInfo.bpm;
    }
    
    float chaos = 0.0f;
    if (auto* p = apvts.getRawParameterValue("chaos")) chaos = p->load();
    
    int g_scale = 1;
    if (auto* p = apvts.getRawParameterValue("global_scale")) g_scale = (int)p->load();

    double sr = getSampleRate();
    int samples_per_16th = (sr * 60.0) / (bpm * 4.0);
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        if (is_playing && seqEnabled) {
            if (step_samples_counter >= samples_per_16th) {
                step_samples_counter = 0;
                for(int t=0; t<6; ++t) {
                    juce::String ts = "t" + juce::String(t+1) + "_";
                    int steps = 16, pulses = 4, offset = 0;
                    if (auto* p = apvts.getRawParameterValue(ts+"steps")) steps = (int)p->load();
                    if (auto* p = apvts.getRawParameterValue(ts+"pulses")) pulses = (int)p->load();
                    if (auto* p = apvts.getRawParameterValue(ts+"offset")) offset = (int)p->load();
                    
                    if (steps > 0) {
                        voices[t].current_step = (voices[t].current_step + 1) % steps;
                        std::vector<int> pat(steps, 0);
                        if (pulses >= steps) { std::fill(pat.begin(), pat.end(), 1); }
                        else if (pulses > 0) {
                            int bucket = 0;
                            for (int i = 0; i < steps; ++i) {
                                bucket += pulses;
                                if (bucket >= steps) { bucket -= steps; pat[(i + offset) % steps] = 1; }
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
            if (!trackMutes[t]) voices[t].process(outL, outR, apvts, t, chaos, g_scale);
        }
        
        
        // Space Echo DSP
        float dTime = 0.3f, dFdbk = 0.4f, dMix = 0.3f, dWow = 0.0f;
        if (auto* p = apvts.getRawParameterValue("echo_time")) dTime = p->load();
        if (auto* p = apvts.getRawParameterValue("echo_fdbk")) dFdbk = p->load();
        if (auto* p = apvts.getRawParameterValue("echo_mix")) dMix = p->load();
        if (auto* p = apvts.getRawParameterValue("echo_wow")) dWow = p->load();

        float max_delay_samples = sr * 2.0f;
        float delay_samples = dTime * sr;
        
        // Wow (flutter)
        if (dWow > 0.01f) {
            float lfo = std::sin(sample * 0.0005f);
            delay_samples += lfo * dWow * 20.0f; 
        }
        if (delay_samples > max_delay_samples) delay_samples = max_delay_samples;
        if (delay_samples < 1.0f) delay_samples = 1.0f;

        delayL.setDelay(delay_samples);
        delayR.setDelay(delay_samples);

        float dOutL = delayL.popSample(0);
        float dOutR = delayR.popSample(0); // Using single channel delay line per object

        delayL.pushSample(0, outL + dOutL * dFdbk);
        delayR.pushSample(0, outR + dOutR * dFdbk);

        outL = outL * (1.0f - dMix) + dOutL * dMix;
        outR = outR * (1.0f - dMix) + dOutR * dMix;

        float mVol = 0.8f;
        if (auto* p = apvts.getRawParameterValue("master_vol")) mVol = p->load();
        
        float masterOut = std::max(-1.0f, std::min(1.0f, outL * mVol)); // soft clip
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
