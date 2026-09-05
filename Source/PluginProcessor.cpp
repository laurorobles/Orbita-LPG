#include "PluginProcessor.h"
#include "LicenseManager.h"
#include "PluginEditor.h"
#include <cmath>

static const std::vector<std::vector<int>> SCALES = {
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, {0, 2, 4, 5, 7, 9, 11}, {0, 2, 3, 5, 7, 8, 10}, {0, 2, 3, 5, 7, 9, 10}, 
    {0, 1, 3, 5, 7, 8, 10}, {0, 2, 4, 6, 7, 9, 11}, {0, 2, 4, 5, 7, 9, 10}, {0, 2, 4, 7, 9}, {0, 3, 5, 7, 10}, 
    {0, 2, 3, 5, 7, 8, 11}, {0, 1, 4, 5, 7, 8, 10}, {0, 2, 3, 7, 8}, {0, 2, 4, 6, 8, 10}, {0, 1, 3, 4, 6, 7, 9, 10}
};

static float quantize_pitch(float raw_midi, int scale_idx, int root_note) {
    if (scale_idx < 1) scale_idx = 1; if (scale_idx > 14) scale_idx = 14;
    const auto& scale = SCALES[scale_idx - 1];
    int shifted = (int)std::round(raw_midi) - root_note;
    int octave = (int)std::floor(shifted / 12.0f); int note = (shifted % 12 + 12) % 12; 
    int closest_note = scale[0]; int min_dist = 100;
    for (int s : scale) { int dist = std::abs(note - s); if (dist < min_dist) { min_dist = dist; closest_note = s; } }
    return (float)(octave * 12 + closest_note + root_note);
}

void WestCoastVoice::trigger(float target_pitch, float drop_amt, float chaos_amt, float gate_len) {
    env_stage = 1; env = 0.0f; active_note = (int)target_pitch;
    gate_samples = gate_len;
    last_chaos_amt = chaos_amt; last_drop_amt = drop_amt;
    chaos_latch = (xorshift_float() * 2.0f - 1.0f) * chaos_amt;
    current_pitch = target_pitch + (chaos_latch * 12.0f);
}

void WestCoastVoice::releaseGate() { if (env_stage == 3) env_stage = 2; }

void WestCoastVoice::reset() {
    current_step = -1; env = 0.0f; env_stage = 0; gate_samples = 0.0f;
    lpg_state = 0.0f; lpf_state = 0.0f; bandpass_state = 0.0f; phase = 0.0f;
    active_midi_note = -1; midi_gate_samples = 0; last_fold_in = 0.0f;
}

float WestCoastVoice::process(float& outL, float& outR, const TrackParams& params) {
    if (!params.pitch) return 0.0f; 
    
    float morph = params.morph->load(); float fold_amt = params.fold->load() * (1.0f + chaos_latch);
    float fm_mod = params.fm->load(); float rise = params.rise->load(); float fall = params.fall->load();
    float resp = params.resp->load(); float brgt = params.brgt->load(); float reso = params.reso->load();
    float noise = params.noise->load(); float vol = params.vol->load(); 
    int m281 = (int)params.mode281->load(); int m292 = (int)params.mode292->load();

    float rise_rate = 1.0f / (std::max(0.001f, rise) * sr);
    float fall_rate = 1.0f / (std::max(0.001f, fall) * sr);

    if (gate_samples > 0.0f) {
        gate_samples -= 1.0f;
        if (gate_samples <= 0.0f && env_stage == 3) env_stage = 2;
    }

    if (env_stage == 1) {
        env += rise_rate; if (env >= 1.0f) { env = 1.0f; env_stage = (m281 == 1) ? 3 : 2; }
    } else if (env_stage == 2) {
        env -= fall_rate;
        if (env <= 0.0f) { 
            env = 0.0f; env_stage = 0; 
            if (m281 == 2) trigger(current_pitch - (chaos_latch * 12.0f), last_drop_amt, last_chaos_amt, 44100.0f); 
        }
    }

    float exp_env = env * env * env;
    float freq = 440.0f * std::pow(2.0f, (current_pitch - 69.0f) / 12.0f);
    if (params.drop->load() > 0.01f) freq *= (1.0f + exp_env * params.drop->load() * 2.0f); 
    phase += (freq / sr); if (phase > 1.0f) phase -= 1.0f;

    float tri = 4.0f * std::abs(phase - 0.5f) - 1.0f; float sqr = phase < 0.5f ? 1.0f : -1.0f;
    float sig = tri * (1.0f - morph) + sqr * morph;
    
    if (fm_mod > 0.01f) {
        float fm_phase = phase + (sig * fm_mod * 0.5f);
        if (fm_phase > 1.0f) fm_phase -= 1.0f; if (fm_phase < 0.0f) fm_phase += 1.0f;
        sig = 4.0f * std::abs(fm_phase - 0.5f) - 1.0f;
    }
    
    float fold_gain = 1.0f + fold_amt * 8.0f; 
    sig = std::sin(sig * fold_gain * juce::MathConstants<float>::halfPi);
    if (noise > 0.01f) sig = sig * (1.0f - noise) + ((xorshift_float() * 2.0f) - 1.0f) * noise;

    // 1. Física real del Vactrol: Ataque rápido (2ms), cierre dependiente de RESP (15ms a 1500ms)
    float attack_time = 2.0f;
    float decay_time = 15.0f + (resp * 1485.0f); 
    
    float vactrol_open = 1.0f - std::exp(-1000.0f / (attack_time * sr));
    float vactrol_close = 1.0f - std::exp(-1000.0f / (decay_time * sr));
    
    lpg_state += (exp_env - lpg_state) * ((exp_env > lpg_state) ? vactrol_open : vactrol_close);
    
    // 2. Control de Brillo (Cutoff) con curva exponencial para barridos orgánicos
    // Si m292 == 0 (VCA), esta variable no afecta la salida final.
    float cutoff = 50.0f + (brgt * brgt * 16000.0f * (m292 == 1 ? lpg_state : 1.0f));
    cutoff = std::max(20.0f, std::min(cutoff, (float)(sr * 0.45))); 
    
    float f = 2.0f * std::sin(juce::MathConstants<float>::pi * cutoff / sr);
    float q = 2.0f - (reso * 1.95f); 
    
    float highpass = sig - lpf_state - q * bandpass_state;
    bandpass_state += f * highpass;
    lpf_state += f * bandpass_state;
    
    lpf_state = std::tanh(lpf_state);
    bandpass_state = std::tanh(bandpass_state);

    float final_sig = 0.0f;
    if (m292 == 0) {
        final_sig = sig * exp_env; 
    } else if (m292 == 1) {
        final_sig = lpf_state * lpg_state; 
    } else if (m292 == 2) {
        final_sig = lpf_state * exp_env; 
    }
    
    outL = final_sig * vol; outR = final_sig * vol; return final_sig;
}

OrbitaLPGAudioProcessor::OrbitaLPGAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withOutput("Master", juce::AudioChannelSet::stereo(), true)
                       .withOutput("Track 1", juce::AudioChannelSet::stereo(), false)
                       .withOutput("Track 2", juce::AudioChannelSet::stereo(), false)
                       .withOutput("Track 3", juce::AudioChannelSet::stereo(), false)
                       .withOutput("Track 4", juce::AudioChannelSet::stereo(), false)
                       .withOutput("Track 5", juce::AudioChannelSet::stereo(), false)
                       .withOutput("Track 6", juce::AudioChannelSet::stereo(), false)),
       apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    p_master_vol = apvts.getRawParameterValue("master_vol"); p_master_drive = apvts.getRawParameterValue("master_drive");
    p_bpm = apvts.getRawParameterValue("bpm"); p_is_playing = apvts.getRawParameterValue("is_playing");
    p_chaos = apvts.getRawParameterValue("chaos"); p_global_scale = apvts.getRawParameterValue("global_scale"); p_global_root = apvts.getRawParameterValue("global_root");
    p_echo_time = apvts.getRawParameterValue("echo_time"); p_echo_fdbk = apvts.getRawParameterValue("echo_fdbk");
    p_echo_mix = apvts.getRawParameterValue("echo_mix"); p_echo_wow = apvts.getRawParameterValue("echo_wow"); p_echo_sync = apvts.getRawParameterValue("echo_sync");

    for(int i=0; i<6; ++i) {
        juce::String t = juce::String(i+1);
        tParams[i].steps = apvts.getRawParameterValue("t"+t+"_steps"); tParams[i].pulses = apvts.getRawParameterValue("t"+t+"_pulses");
        tParams[i].offset = apvts.getRawParameterValue("t"+t+"_offset"); tParams[i].rate = apvts.getRawParameterValue("t"+t+"_rate");
        tParams[i].pitch = apvts.getRawParameterValue("t"+t+"_pitch"); tParams[i].drop = apvts.getRawParameterValue("t"+t+"_drop");
        tParams[i].morph = apvts.getRawParameterValue("t"+t+"_morph"); tParams[i].fold = apvts.getRawParameterValue("t"+t+"_fold");
        tParams[i].fm = apvts.getRawParameterValue("t"+t+"_fm"); tParams[i].rise = apvts.getRawParameterValue("t"+t+"_rise");
        tParams[i].fall = apvts.getRawParameterValue("t"+t+"_fall"); tParams[i].resp = apvts.getRawParameterValue("t"+t+"_resp");
        tParams[i].brgt = apvts.getRawParameterValue("t"+t+"_brgt"); tParams[i].reso = apvts.getRawParameterValue("t"+t+"_reso");
        tParams[i].noise = apvts.getRawParameterValue("t"+t+"_noise"); tParams[i].vol = apvts.getRawParameterValue("t"+t+"_vol");
        tParams[i].notemode = apvts.getRawParameterValue("t"+t+"_notemode");
        tParams[i].mode281 = apvts.getRawParameterValue("t"+t+"_mode281"); tParams[i].mode292 = apvts.getRawParameterValue("t"+t+"_mode292");
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout OrbitaLPGAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("master_vol", "Master Vol", 0.0f, 1.0f, 0.8f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("master_drive", "Drive", 0.0f, 1.0f, 0.2f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("bpm", "BPM", 20.0f, 300.0f, 124.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("swing", "Swing", 0.0f, 0.5f, 0.15f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("chaos", "Chaos", 0.0f, 1.0f, 0.05f));
    layout.add(std::make_unique<juce::AudioParameterInt>("global_scale", "Global Scale", 1, 14, 4));
    layout.add(std::make_unique<juce::AudioParameterInt>("global_root", "Global Root", 0, 11, 0));
    layout.add(std::make_unique<juce::AudioParameterBool>("is_playing", "Playing", false));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("echo_time", "Echo Time", 0.05f, 1.0f, 0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("echo_fdbk", "Echo Fdbk", 0.0f, 0.9f, 0.4f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("echo_mix", "Echo Mix", 0.0f, 1.0f, 0.15f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("echo_wow", "Echo Wow", 0.0f, 1.0f, 0.2f));
    layout.add(std::make_unique<juce::AudioParameterBool>("echo_sync", "Echo Sync", true));

    juce::NormalisableRange<float> riseRange(0.001f, 1.0f, 0.001f, 0.3f);
    juce::NormalisableRange<float> fallRange(0.01f, 3.0f, 0.001f, 0.3f);

    auto addTrackParams = [&](int id, int steps, int pulses, int offset, float pitch, float drop, float morph, float fold, float fm, float rise, float fall, float resp, float brgt, float reso, float noise, float vol, int m281, int m292) {
        juce::String t = juce::String(id);
        layout.add(std::make_unique<juce::AudioParameterInt>("t"+t+"_steps", "T"+t+" Steps", 2, 24, steps));
        layout.add(std::make_unique<juce::AudioParameterInt>("t"+t+"_pulses", "T"+t+" Pulses", 1, 24, pulses));
        layout.add(std::make_unique<juce::AudioParameterInt>("t"+t+"_offset", "T"+t+" Offset", 0, 23, offset));
        layout.add(std::make_unique<juce::AudioParameterChoice>("t"+t+"_rate", "T"+t+" Rate", juce::StringArray{"1/4", "1/8", "1/16", "1/32"}, 2));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_pitch", "T"+t+" Pitch", 24.0f, 96.0f, pitch));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_drop", "T"+t+" P.Drop", 0.0f, 1.0f, drop));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_morph", "T"+t+" Morph", 0.0f, 1.0f, morph));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_fold", "T"+t+" Fold", 0.0f, 1.0f, fold));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_fm", "T"+t+" FM Mod", 0.0f, 1.0f, fm));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_rise", "T"+t+" Rise", riseRange, rise));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_fall", "T"+t+" Fall", fallRange, fall));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_resp", "T"+t+" Resp", 0.05f, 1.0f, resp));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_brgt", "T"+t+" Brgt", 0.05f, 1.0f, brgt));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_reso", "T"+t+" Reso", 0.0f, 1.0f, reso));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_noise", "T"+t+" Noise", 0.0f, 1.0f, noise));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_vol", "T"+t+" Vol", 0.0f, 1.0f, vol));
        layout.add(std::make_unique<juce::AudioParameterBool>("t"+t+"_notemode", "T"+t+" Note Mode", true));
        layout.add(std::make_unique<juce::AudioParameterChoice>("t"+t+"_mode281", "T"+t+" 281 Mode", juce::StringArray{"TRANS", "SUST", "CYCLE"}, m281));
        layout.add(std::make_unique<juce::AudioParameterChoice>("t"+t+"_mode292", "T"+t+" 292 Mode", juce::StringArray{"VCA", "LPG", "VCF"}, m292));
    };

    // Init Patch: T1 has a basic Kick, T2-T6 are completely initialized to 0 pulses and neutral settings
    addTrackParams(1, 16, 4, 0, 36.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.001f, 0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.8f, 0, 1); 
    for(int i=2; i<=6; i++) {
        addTrackParams(i, 16, 0, 0, 60.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.3f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0, 1); 
    } 

    return layout;
}

OrbitaLPGAudioProcessor::~OrbitaLPGAudioProcessor() {}
void OrbitaLPGAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    if (LicenseManager::isLicensed()) {
        demoExpired.store(false);
        demoSampleCount = 0;
    }
    for (int i = 0; i < 6; i++) voices[i].prepare(sampleRate);
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32)samplesPerBlock, 2 };
    delayL.prepare(spec); delayR.prepare(spec);
}
void OrbitaLPGAudioProcessor::releaseResources() {}
bool OrbitaLPGAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const { return true; }

void OrbitaLPGAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals; buffer.clear();
    
    if (!LicenseManager::isLicensed()) {
        demoSampleCount += buffer.getNumSamples();
        if (demoSampleCount > getSampleRate() * 60 * 10) {
            demoExpired.store(true);
            return;
        }
    }
    auto* playhead = getPlayHead(); 
    
    bool host_is_playing = false;
    double host_ppq = -1.0;
    float bpm = p_bpm ? p_bpm->load() : 124.0f;
    
    if (playhead != nullptr) {
        if (auto pos = playhead->getPosition()) {
            host_is_playing = pos->getIsPlaying();
            if (auto hostBpm = pos->getBpm()) bpm = (float)*hostBpm;
            if (auto hostPpq = pos->getPpqPosition()) host_ppq = *hostPpq;
        }
    }
    
    if (host_is_playing && host_ppq >= 0.0) {
        internal_ppq = host_ppq;
    }
    
    float chaos = p_chaos ? p_chaos->load() : 0.0f;
    int g_scale = p_global_scale ? (int)p_global_scale->load() : 1;
    int g_root = p_global_root ? (int)p_global_root->load() : 0;
    double sr = getSampleRate();
    float beat_samples = (sr * 60.0f) / bpm;
    double ppq_per_sample = 1.0 / (double)beat_samples;
    
    for (const auto metadata : midiMessages) {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn() || msg.isNoteOff()) {
            int ch = msg.getChannel();
            int note = msg.getNoteNumber();
            
            // MODO 1: Drum Machine (Canal 1, C1 a F1)
            if (ch == 1 && note >= 36 && note <= 41) {
                int trackIdx = note - 36;
                if (msg.isNoteOn()) {
                    float raw_p = tParams[trackIdx].pitch->load();
                    bool note_mode = tParams[trackIdx].notemode->load() > 0.5f;
                    float q_pitch = note_mode ? quantize_pitch(raw_p, g_scale, g_root) : raw_p;
                    voices[trackIdx].trigger(q_pitch, tParams[trackIdx].drop->load(), chaos, 999999.0f); // Sustain infinito hasta NoteOff
                } else {
                    voices[trackIdx].releaseGate();
                }
            }
            // MODO 2: Sintetizador Cromático Multitímbrico (Canales 1 al 6)
            else if (ch >= 1 && ch <= 6) {
                int trackIdx = ch - 1;
                if (msg.isNoteOn()) {
                    float raw_p = (float)note; 
                    bool note_mode = tParams[trackIdx].notemode->load() > 0.5f;
                    float q_pitch = note_mode ? quantize_pitch(raw_p, g_scale, g_root) : raw_p;
                    voices[trackIdx].trigger(q_pitch, tParams[trackIdx].drop->load(), chaos, 999999.0f);
                } else {
                    if (voices[trackIdx].active_note == note) voices[trackIdx].releaseGate();
                }
            }
        }
    }
    midiMessages.clear(); 
    
    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        // Process pending Note Offs
        for(int t=0; t<6; ++t) {
            if (voices[t].midi_gate_samples > 0) {
                voices[t].midi_gate_samples--;
                if (voices[t].midi_gate_samples == 0 && voices[t].active_midi_note >= 0) {
                    midiMessages.addEvent(juce::MidiMessage::noteOff(t + 1, voices[t].active_midi_note, 0.0f), sample);
                    voices[t].active_midi_note = -1;
                }
            }
        }
        
        if ((host_is_playing || this->isPlaying) && seqEnabled) {
            for(int t=0; t<6; ++t) {
                static const float DIVISORS[4] = {1.0f, 0.5f, 0.25f, 0.125f};
                float divisor = DIVISORS[(int)tParams[t].rate->load() & 3];
                
                double absolute_step = internal_ppq / (double)divisor;
                double previous_step = (internal_ppq - ppq_per_sample) / (double)divisor;
                
                if (std::floor(absolute_step) > std::floor(previous_step)) {
                    int steps = (int)tParams[t].steps->load();
                    int pulses = (int)tParams[t].pulses->load();
                    int offset = (int)tParams[t].offset->load();
                    
                    voices[t].current_step = ((long long)std::floor(absolute_step)) % steps;
                    if (voices[t].current_step < 0) voices[t].current_step += steps;
                    
                    bool trigger_hit = false;
                    if (pulses >= steps) trigger_hit = true;
                    else if (pulses > 0) {
                        int current_val = ((voices[t].current_step + offset) * pulses) % steps;
                        trigger_hit = (current_val < pulses);
                    }
                    
                    if (trigger_hit) {
                        float raw_p = tParams[t].pitch->load();
                        bool note_mode = tParams[t].notemode->load() > 0.5f;
                        float q_pitch = note_mode ? quantize_pitch(raw_p, g_scale, g_root) : raw_p;
                        voices[t].trigger(q_pitch, tParams[t].drop->load(), chaos, beat_samples * divisor * 0.5f);
                        
                        // Schedule MIDI Out Note
                        if (voices[t].active_midi_note >= 0) {
                            midiMessages.addEvent(juce::MidiMessage::noteOff(t + 1, voices[t].active_midi_note, 0.0f), sample);
                        }
                        voices[t].active_midi_note = (int)q_pitch;
                        voices[t].midi_gate_samples = (int)((beat_samples * divisor) * 0.5f);
                        midiMessages.addEvent(juce::MidiMessage::noteOn(t + 1, (int)q_pitch, tParams[t].vol->load()), sample);
                    }
                }
            }
        }
        
        if (host_is_playing || this->isPlaying) {
            internal_ppq += ppq_per_sample;
        }

        float masterL = 0.0f, masterR = 0.0f;
        for (int t = 0; t < 6; ++t) {
            float trackL = 0.0f, trackR = 0.0f;
            if (!trackMutes[t]) voices[t].process(trackL, trackR, tParams[t]);
            
            int busIdx = t + 1; 
            if (busIdx < getBusCount(false) && getChannelCountOfBus(false, busIdx) == 2) {
                int lChan = getChannelIndexInProcessBlockBuffer(false, busIdx, 0);
                int rChan = getChannelIndexInProcessBlockBuffer(false, busIdx, 1);
                buffer.addSample(lChan, sample, trackL); buffer.addSample(rChan, sample, trackR);
            }
            masterL += trackL; masterR += trackR;
        }
        
        float dTimeRaw = p_echo_time->load(); float delay_samples;
        if (p_echo_sync->load() > 0.5f) {
            if (dTimeRaw < 0.2f) delay_samples = beat_samples * 0.125f; 
            else if (dTimeRaw < 0.4f) delay_samples = beat_samples * 0.25f; 
            else if (dTimeRaw < 0.6f) delay_samples = beat_samples * 0.5f; 
            else if (dTimeRaw < 0.8f) delay_samples = beat_samples * 0.75f; 
            else delay_samples = beat_samples * 1.0f; 
        } else { delay_samples = dTimeRaw * sr; }

        float dWow = p_echo_wow->load(); lfo_phase += 0.5f / sr; if (lfo_phase > 1.0f) lfo_phase -= 1.0f;
        if (dWow > 0.01f) delay_samples += std::sin(lfo_phase * juce::MathConstants<float>::twoPi) * dWow * 20.0f;
        delay_samples = std::max(1.0f, std::min(delay_samples, (float)sr * 2.0f));

        delayL.setDelay(delay_samples); delayR.setDelay(delay_samples);
        float dOutL = delayL.popSample(0); float dOutR = delayR.popSample(0); 
        float dFdbk = p_echo_fdbk->load(); delayL.pushSample(0, masterL + dOutL * dFdbk); delayR.pushSample(0, masterR + dOutR * dFdbk);
        float dMix = p_echo_mix->load(); masterL = masterL * (1.0f - dMix) + dOutL * dMix; masterR = masterR * (1.0f - dMix) + dOutR * dMix;

        float mDrive = p_master_drive->load();
        masterL *= (1.0f + mDrive * 4.0f); masterR *= (1.0f + mDrive * 4.0f);
        masterL = std::tanh(masterL); masterR = std::tanh(masterR);

        float mVol = p_master_vol->load();
        if (buffer.getNumChannels() > 0) buffer.addSample(0, sample, masterL * mVol);
        if (buffer.getNumChannels() > 1) buffer.addSample(1, sample, masterR * mVol);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new OrbitaLPGAudioProcessor(); }
juce::AudioProcessorEditor* OrbitaLPGAudioProcessor::createEditor() { return new OrbitaLPGAudioProcessorEditor(*this); }
void OrbitaLPGAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    if (xml != nullptr) copyXmlToBinary(*xml, destData);
}
void OrbitaLPGAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr) {
        if (xmlState->hasTagName(apvts.state.getType())) {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}