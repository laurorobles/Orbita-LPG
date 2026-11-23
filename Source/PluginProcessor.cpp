#include "PluginProcessor.h"
#include "PluginEditor.h"

static inline long long swungStepIndex(double ppq, double divisor, double swing) {
    if (ppq < 0.0) return -1;
    double pairLen = 2.0 * divisor;
    long long pairIdx = (long long)std::floor(ppq / pairLen);
    double rem = ppq - (double)pairIdx * pairLen;
    double split = (1.0 + swing * 0.5) * divisor;
    return pairIdx * 2 + (rem >= split ? 1 : 0);
}

void WestCoastVoice::trigger(float target_pitch, float drop_amt, float chaos_amt, float gate_len) {
    env_stage = 1;
    env = 0.0f;
    active_note = (int)std::round(target_pitch);
    gate_samples = gate_len;
    last_chaos_amt = chaos_amt;
    last_drop_amt = drop_amt;
    chaos_latch = (xorshift_float() * 2.0f - 1.0f) * chaos_amt;
    current_pitch = target_pitch + (chaos_latch * 12.0f);
}

void WestCoastVoice::releaseGate() {
    if (env_stage == 3) env_stage = 2;
}

void WestCoastVoice::reset() {
    current_step.store(-1, std::memory_order_relaxed);
    visual_lpg.store(0.0f, std::memory_order_relaxed);
    env = 0.0f;
    env_stage = 0;
    gate_samples = 0.0f;
    lpg_state = 0.0f;
    lpf_state = 0.0f;
    bandpass_state = 0.0f;
    phase = 0.0f;
    active_midi_note = -1;
    midi_gate_samples = 0;
    last_fold_in = 0.0f;
}

float WestCoastVoice::process(float& outL, float& outR, const TrackParams& params) {
    if (!params.pitch) return 0.0f;

    float morph = params.morph->load(std::memory_order_relaxed);
    float fold_amt = std::max(0.0f, params.fold->load(std::memory_order_relaxed) * (1.0f + chaos_latch));
    float fm_mod = params.fm->load(std::memory_order_relaxed);
    float rise = params.rise->load(std::memory_order_relaxed);
    float fall = params.fall->load(std::memory_order_relaxed);
    float resp = params.resp->load(std::memory_order_relaxed);
    float brgt = params.brgt->load(std::memory_order_relaxed);
    float reso = params.reso->load(std::memory_order_relaxed);
    float noise = params.noise->load(std::memory_order_relaxed);
    float vol = params.vol->load(std::memory_order_relaxed);
    float drop = params.drop->load(std::memory_order_relaxed);
    int m281 = (int)params.mode281->load(std::memory_order_relaxed);
    int m292 = (int)params.mode292->load(std::memory_order_relaxed);

    float rise_rate = 1.0f / (std::max(0.001f, rise) * (float)sr);
    float fall_rate = 1.0f / (std::max(0.001f, fall) * (float)sr);

    if (gate_samples > 0.0f) {
        gate_samples -= 1.0f;
        if (gate_samples <= 0.0f && env_stage == 3) env_stage = 2;
    }

    if (env_stage == 1) {
        env += rise_rate;
        if (env >= 1.0f) { env = 1.0f; env_stage = (m281 == 1) ? 3 : 2; }
    } else if (env_stage == 2) {
        env -= fall_rate;
        if (env <= 0.0f) {
            env = 0.0f;
            env_stage = 0;
            if (m281 == 2) trigger(current_pitch - (chaos_latch * 12.0f), last_drop_amt, last_chaos_amt, (float)sr);
        }
    }

    float exp_env = env * env * env;
    float freq = 440.0f * std::pow(2.0f, (current_pitch - 69.0f) / 12.0f);
    if (drop > 0.01f) freq *= (1.0f + exp_env * drop * 2.0f);
    phase += (freq / (float)sr);
    if (phase >= 1.0f) phase -= std::floor(phase);

    float tri = 4.0f * std::abs(phase - 0.5f) - 1.0f;
    float sqr = phase < 0.5f ? 1.0f : -1.0f;
    float sig = tri * (1.0f - morph) + sqr * morph;

    if (fm_mod > 0.01f) {
        float fm_phase = phase + (sig * fm_mod * 0.5f);
        fm_phase -= std::floor(fm_phase);
        sig = 4.0f * std::abs(fm_phase - 0.5f) - 1.0f;
    }

    float fold_gain = 1.0f + fold_amt * 8.0f;
    float fold_in = sig * fold_gain * juce::MathConstants<float>::halfPi;
    float diff = fold_in - last_fold_in;
    if (std::abs(diff) > 1.0e-5f)
        sig = (-std::cos(fold_in) + std::cos(last_fold_in)) / diff;
    else
        sig = std::sin(0.5f * (fold_in + last_fold_in));
    last_fold_in = fold_in;

    if (noise > 0.01f) sig = sig * (1.0f - noise) + ((xorshift_float() * 2.0f) - 1.0f) * noise;

    float decay_time = 15.0f + (resp * 1485.0f);
    float vactrol_close = 1.0f - std::exp(-1000.0f / (decay_time * (float)sr));
    lpg_state += (exp_env - lpg_state) * ((exp_env > lpg_state) ? vactrol_open : vactrol_close);

    float cutoff = 50.0f + (brgt * brgt * 16000.0f * (m292 == 1 ? lpg_state : 1.0f));
    cutoff = juce::jlimit(20.0f, (float)(sr * 0.45), cutoff);

    float f = 2.0f * std::sin(juce::MathConstants<float>::pi * cutoff / (float)sr);
    float q = 2.0f - (reso * 1.95f);

    float highpass = sig - lpf_state - q * bandpass_state;
    bandpass_state = std::tanh(bandpass_state + f * highpass);
    lpf_state = std::tanh(lpf_state + f * bandpass_state);

    float final_sig = 0.0f;
    if (m292 == 0) final_sig = sig * exp_env;
    else if (m292 == 1) final_sig = lpf_state * lpg_state;
    else final_sig = lpf_state * exp_env;

    visual_lpg.store(juce::jlimit(0.0f, 1.0f, (m292 == 1 ? lpg_state : exp_env) * (vol > 0.001f ? 1.0f : 0.25f)), std::memory_order_relaxed);

    outL = final_sig * vol;
    outR = final_sig * vol;
    return final_sig;
}

OrbitaLPGAudioProcessor::OrbitaLPGAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withOutput("Master", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Track 1", juce::AudioChannelSet::stereo(), false)
                     .withOutput("Track 2", juce::AudioChannelSet::stereo(), false)
                     .withOutput("Track 3", juce::AudioChannelSet::stereo(), false)
                     .withOutput("Track 4", juce::AudioChannelSet::stereo(), false)
                     .withOutput("Track 5", juce::AudioChannelSet::stereo(), false)
                     .withOutput("Track 6", juce::AudioChannelSet::stereo(), false)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    p_master_vol = apvts.getRawParameterValue("master_vol");
    p_master_drive = apvts.getRawParameterValue("master_drive");
    p_bpm = apvts.getRawParameterValue("bpm");
    p_swing = apvts.getRawParameterValue("swing");
    p_chaos = apvts.getRawParameterValue("chaos");
    p_global_scale = apvts.getRawParameterValue("global_scale");
    p_global_root = apvts.getRawParameterValue("global_root");
    p_echo_time = apvts.getRawParameterValue("echo_time");
    p_echo_fdbk = apvts.getRawParameterValue("echo_fdbk");
    p_echo_mix = apvts.getRawParameterValue("echo_mix");
    p_echo_wow = apvts.getRawParameterValue("echo_wow");
    p_echo_sync = apvts.getRawParameterValue("echo_sync");

    for (int i = 0; i < 6; ++i) {
        trackMutes[i].store(false);
        pendingStrike[i].store(false);
        juce::String t = juce::String(i + 1);
        tParams[i].steps = apvts.getRawParameterValue("t" + t + "_steps");
        tParams[i].pulses = apvts.getRawParameterValue("t" + t + "_pulses");
        tParams[i].offset = apvts.getRawParameterValue("t" + t + "_offset");
        tParams[i].rate = apvts.getRawParameterValue("t" + t + "_rate");
        tParams[i].pitch = apvts.getRawParameterValue("t" + t + "_pitch");
        tParams[i].drop = apvts.getRawParameterValue("t" + t + "_drop");
        tParams[i].morph = apvts.getRawParameterValue("t" + t + "_morph");
        tParams[i].fold = apvts.getRawParameterValue("t" + t + "_fold");
        tParams[i].fm = apvts.getRawParameterValue("t" + t + "_fm");
        tParams[i].rise = apvts.getRawParameterValue("t" + t + "_rise");
        tParams[i].fall = apvts.getRawParameterValue("t" + t + "_fall");
        tParams[i].resp = apvts.getRawParameterValue("t" + t + "_resp");
        tParams[i].brgt = apvts.getRawParameterValue("t" + t + "_brgt");
        tParams[i].reso = apvts.getRawParameterValue("t" + t + "_reso");
        tParams[i].noise = apvts.getRawParameterValue("t" + t + "_noise");
        tParams[i].vol = apvts.getRawParameterValue("t" + t + "_vol");
        tParams[i].notemode = apvts.getRawParameterValue("t" + t + "_notemode");
        tParams[i].mode281 = apvts.getRawParameterValue("t" + t + "_mode281");
        tParams[i].mode292 = apvts.getRawParameterValue("t" + t + "_mode292");
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout OrbitaLPGAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>("master_vol", "Master Vol", 0.0f, 1.0f, 0.8f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("master_drive", "Drive", 0.0f, 1.0f, 0.15f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("bpm", "BPM", 20.0f, 300.0f, 124.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("swing", "Swing", 0.0f, 0.5f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("chaos", "Chaos", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterInt>("global_scale", "Global Scale", 1, 14, 4));
    layout.add(std::make_unique<juce::AudioParameterInt>("global_root", "Global Root", 0, 11, 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>("echo_time", "Echo Time", 0.05f, 1.0f, 0.65f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("echo_fdbk", "Echo Fdbk", 0.0f, 0.9f, 0.35f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("echo_mix", "Echo Mix", 0.0f, 1.0f, 0.12f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("echo_wow", "Echo Wow", 0.0f, 1.0f, 0.2f));
    layout.add(std::make_unique<juce::AudioParameterBool>("echo_sync", "Echo Sync", true));

    juce::NormalisableRange<float> riseRange(0.001f, 1.0f, 0.001f, 0.3f);
    juce::NormalisableRange<float> fallRange(0.01f, 3.0f, 0.001f, 0.3f);

    auto addTrackParams = [&](int id, int steps, int pulses, int offset, float pitch, float drop, float morph, float fold, float fm, float rise, float fall, float resp, float brgt, float reso, float noise, float vol, int m281, int m292) {
        juce::String t = juce::String(id);
        layout.add(std::make_unique<juce::AudioParameterInt>("t" + t + "_steps", "T" + t + " Steps", 2, 32, steps));
        layout.add(std::make_unique<juce::AudioParameterInt>("t" + t + "_pulses", "T" + t + " Pulses", 0, 32, pulses));
        layout.add(std::make_unique<juce::AudioParameterInt>("t" + t + "_offset", "T" + t + " Offset", 0, 31, offset));
        layout.add(std::make_unique<juce::AudioParameterChoice>("t" + t + "_rate", "T" + t + " Rate", juce::StringArray{"1/4", "1/8", "1/16", "1/32"}, 2));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t" + t + "_pitch", "T" + t + " Pitch", 24.0f, 96.0f, pitch));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t" + t + "_drop", "T" + t + " P.Drop", 0.0f, 1.0f, drop));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t" + t + "_morph", "T" + t + " Morph", 0.0f, 1.0f, morph));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t" + t + "_fold", "T" + t + " Fold", 0.0f, 1.0f, fold));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t" + t + "_fm", "T" + t + " FM Mod", 0.0f, 1.0f, fm));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t" + t + "_rise", "T" + t + " Rise", riseRange, rise));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t" + t + "_fall", "T" + t + " Fall", fallRange, fall));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t" + t + "_resp", "T" + t + " Resp", 0.05f, 1.0f, resp));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t" + t + "_brgt", "T" + t + " Brgt", 0.05f, 1.0f, brgt));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t" + t + "_reso", "T" + t + " Reso", 0.0f, 1.0f, reso));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t" + t + "_noise", "T" + t + " Noise", 0.0f, 1.0f, noise));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t" + t + "_vol", "T" + t + " Vol", 0.0f, 1.0f, vol));
        layout.add(std::make_unique<juce::AudioParameterBool>("t" + t + "_notemode", "T" + t + " Note Mode", true));
        layout.add(std::make_unique<juce::AudioParameterChoice>("t" + t + "_mode281", "T" + t + " 281 Mode", juce::StringArray{"TRANS", "SUST", "CYCLE"}, m281));
        layout.add(std::make_unique<juce::AudioParameterChoice>("t" + t + "_mode292", "T" + t + " 292 Mode", juce::StringArray{"VCA", "LPG", "VCF"}, m292));
    };

    addTrackParams(1, 16, 4, 0, 36.0f, 0.8f, 0.0f, 0.0f, 0.0f, 0.001f, 0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.85f, 0, 1);
    for (int i = 2; i <= 6; ++i) {
        addTrackParams(i, 16, 0, 0, 60.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.001f, 0.3f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0, 1);
    }

    return layout;
}

OrbitaLPGAudioProcessor::~OrbitaLPGAudioProcessor() {}

void OrbitaLPGAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    for (int i = 0; i < 6; ++i) voices[i].prepare(sampleRate);
    int maxDelay = (int)(sampleRate * 2.5) + 128;
    delayL.setMaximumDelayInSamples(maxDelay);
    delayR.setMaximumDelayInSamples(maxDelay);
    juce::dsp::ProcessSpec spec{sampleRate, (juce::uint32)std::max(1, samplesPerBlock), 1};
    delayL.prepare(spec);
    delayR.prepare(spec);
    delayL.reset();
    delayR.reset();
    smoothedDelayL = (float)(sampleRate * 0.25);
    smoothedDelayR = (float)(sampleRate * 0.255);
    echoFilterL = 0.0f;
    echoFilterR = 0.0f;
}

void OrbitaLPGAudioProcessor::releaseResources() {}
bool OrbitaLPGAudioProcessor::isBusesLayoutSupported(const BusesLayout&) const { return true; }

void OrbitaLPGAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    if (pendingReset.exchange(false, std::memory_order_relaxed)) {
        internal_ppq = 0.0;
        for (int i = 0; i < 6; ++i) voices[i].reset();
    }

    bool host_is_playing = false;
    double host_ppq = -1.0;
    float bpm = p_bpm ? p_bpm->load(std::memory_order_relaxed) : 124.0f;

    if (auto* playhead = getPlayHead()) {
        if (auto pos = playhead->getPosition()) {
            host_is_playing = pos->getIsPlaying();
            if (auto hostBpm = pos->getBpm()) bpm = (float)*hostBpm;
            if (auto hostPpq = pos->getPpqPosition()) host_ppq = *hostPpq;
        }
    }

    hostPlaying.store(host_is_playing, std::memory_order_relaxed);
    if (host_is_playing && host_ppq >= 0.0) internal_ppq = host_ppq;

    float swing = p_swing ? p_swing->load(std::memory_order_relaxed) : 0.0f;
    float chaos = p_chaos ? p_chaos->load(std::memory_order_relaxed) : 0.0f;
    int g_scale = p_global_scale ? (int)p_global_scale->load(std::memory_order_relaxed) : 1;
    int g_root = p_global_root ? (int)p_global_root->load(std::memory_order_relaxed) : 0;
    double sr = getSampleRate() > 0.0 ? getSampleRate() : 44100.0;
    float beat_samples = (float)((sr * 60.0) / std::max(20.0f, bpm));
    double ppq_per_sample = 1.0 / (double)beat_samples;

    for (int t = 0; t < 6; ++t) {
        if (pendingStrike[t].exchange(false, std::memory_order_relaxed)) {
            float raw_p = tParams[t].pitch->load(std::memory_order_relaxed);
            bool note_mode = tParams[t].notemode->load(std::memory_order_relaxed) > 0.5f;
            float q_pitch = note_mode ? OrbitaDSP::quantizePitch(raw_p, g_scale, g_root) : raw_p;
            voices[t].trigger(q_pitch, tParams[t].drop->load(std::memory_order_relaxed), 0.0f, beat_samples * 0.25f);
        }
    }

    for (const auto metadata : midiMessages) {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn() || msg.isNoteOff()) {
            int ch = msg.getChannel();
            int note = msg.getNoteNumber();
            if (ch == 1 && note >= 36 && note <= 41) {
                int trackIdx = note - 36;
                if (msg.isNoteOn()) {
                    float raw_p = tParams[trackIdx].pitch->load(std::memory_order_relaxed);
                    bool note_mode = tParams[trackIdx].notemode->load(std::memory_order_relaxed) > 0.5f;
                    float q_pitch = note_mode ? OrbitaDSP::quantizePitch(raw_p, g_scale, g_root) : raw_p;
                    voices[trackIdx].trigger(q_pitch, tParams[trackIdx].drop->load(std::memory_order_relaxed), chaos, 999999.0f);
                } else {
                    voices[trackIdx].releaseGate();
                }
            } else if (ch >= 1 && ch <= 6) {
                int trackIdx = ch - 1;
                if (msg.isNoteOn()) {
                    float raw_p = (float)note;
                    bool note_mode = tParams[trackIdx].notemode->load(std::memory_order_relaxed) > 0.5f;
                    float q_pitch = note_mode ? OrbitaDSP::quantizePitch(raw_p, g_scale, g_root) : raw_p;
                    voices[trackIdx].trigger(q_pitch, tParams[trackIdx].drop->load(std::memory_order_relaxed), chaos, 999999.0f);
                } else if (voices[trackIdx].active_note == note) {
                    voices[trackIdx].releaseGate();
                }
            }
        }
    }
    midiMessages.clear();

    const bool running = host_is_playing || isPlaying.load(std::memory_order_relaxed);
    const bool seqOn = seqEnabled.load(std::memory_order_relaxed);
    static constexpr float DIVISORS[4] = {1.0f, 0.5f, 0.25f, 0.125f};
    static constexpr float TRACK_PAN_L[6] = {1.0f, 1.0f, 0.84f, 1.0f, 0.76f, 1.0f};
    static constexpr float TRACK_PAN_R[6] = {1.0f, 0.84f, 1.0f, 0.76f, 1.0f, 1.0f};

    float blockPeakL = 0.0f;
    float blockPeakR = 0.0f;
    float maxDelaySamples = (float)(sr * 2.4);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        for (int t = 0; t < 6; ++t) {
            if (voices[t].midi_gate_samples > 0) {
                voices[t].midi_gate_samples--;
                if (voices[t].midi_gate_samples == 0 && voices[t].active_midi_note >= 0) {
                    midiMessages.addEvent(juce::MidiMessage::noteOff(t + 1, voices[t].active_midi_note, 0.0f), sample);
                    voices[t].active_midi_note = -1;
                }
            }
        }

        if (running && seqOn) {
            for (int t = 0; t < 6; ++t) {
                float divisor = DIVISORS[(int)tParams[t].rate->load(std::memory_order_relaxed) & 3];
                long long curStepIdx = swungStepIndex(internal_ppq, (double)divisor, (double)swing);
                long long prevStepIdx = swungStepIndex(internal_ppq - ppq_per_sample, (double)divisor, (double)swing);

                if (curStepIdx > prevStepIdx) {
                    int steps = std::max(1, (int)tParams[t].steps->load(std::memory_order_relaxed));
                    int pulses = (int)tParams[t].pulses->load(std::memory_order_relaxed);
                    int offset = (int)tParams[t].offset->load(std::memory_order_relaxed);

                    int stepPos = (int)(curStepIdx % steps);
                    if (stepPos < 0) stepPos += steps;
                    voices[t].current_step.store(stepPos, std::memory_order_relaxed);

                    if (OrbitaDSP::isEuclideanHit(stepPos, pulses, steps, offset)) {
                        float raw_p = tParams[t].pitch->load(std::memory_order_relaxed);
                        bool note_mode = tParams[t].notemode->load(std::memory_order_relaxed) > 0.5f;
                        float q_pitch = note_mode ? OrbitaDSP::quantizePitch(raw_p, g_scale, g_root) : raw_p;
                        voices[t].trigger(q_pitch, tParams[t].drop->load(std::memory_order_relaxed), chaos, beat_samples * divisor * 0.5f);

                        int midiNote = juce::jlimit(0, 127, (int)std::round(q_pitch));
                        if (voices[t].active_midi_note >= 0) {
                            midiMessages.addEvent(juce::MidiMessage::noteOff(t + 1, voices[t].active_midi_note, 0.0f), sample);
                        }
                        voices[t].active_midi_note = midiNote;
                        voices[t].midi_gate_samples = std::max(1, (int)((beat_samples * divisor) * 0.5f));
                        float vel = juce::jlimit(0.0f, 1.0f, tParams[t].vol->load(std::memory_order_relaxed));
                        midiMessages.addEvent(juce::MidiMessage::noteOn(t + 1, midiNote, vel), sample);
                    }
                }
            }
        }

        if (running) internal_ppq += ppq_per_sample;

        float masterL = 0.0f, masterR = 0.0f;
        for (int t = 0; t < 6; ++t) {
            float trackL = 0.0f, trackR = 0.0f;
            if (!trackMutes[t].load(std::memory_order_relaxed)) {
                voices[t].process(trackL, trackR, tParams[t]);
            } else {
                voices[t].visual_lpg.store(0.0f, std::memory_order_relaxed);
            }

            int busIdx = t + 1;
            if (busIdx < getBusCount(false) && getChannelCountOfBus(false, busIdx) == 2) {
                int lChan = getChannelIndexInProcessBlockBuffer(false, busIdx, 0);
                int rChan = getChannelIndexInProcessBlockBuffer(false, busIdx, 1);
                buffer.addSample(lChan, sample, trackL);
                buffer.addSample(rChan, sample, trackR);
            }
            masterL += trackL * TRACK_PAN_L[t];
            masterR += trackR * TRACK_PAN_R[t];
        }

        float dTimeRaw = p_echo_time->load(std::memory_order_relaxed);
        float targetDelay = 0.0f;
        if (p_echo_sync->load(std::memory_order_relaxed) > 0.5f) {
            if (dTimeRaw < 0.2f) targetDelay = beat_samples * 0.125f;
            else if (dTimeRaw < 0.4f) targetDelay = beat_samples * 0.25f;
            else if (dTimeRaw < 0.6f) targetDelay = beat_samples * 0.5f;
            else if (dTimeRaw < 0.8f) targetDelay = beat_samples * 0.75f;
            else targetDelay = beat_samples * 1.0f;
        } else {
            targetDelay = dTimeRaw * (float)sr;
        }

        float dWow = p_echo_wow->load(std::memory_order_relaxed);
        lfo_phase += 0.55f / (float)sr;
        if (lfo_phase >= 1.0f) lfo_phase -= 1.0f;
        float wowL = dWow > 0.005f ? std::sin(lfo_phase * juce::MathConstants<float>::twoPi) * dWow * 22.0f : 0.0f;
        float wowR = dWow > 0.005f ? std::cos(lfo_phase * juce::MathConstants<float>::twoPi) * dWow * 22.0f : 0.0f;

        float tgtL = juce::jlimit(1.0f, maxDelaySamples, targetDelay + wowL);
        float tgtR = juce::jlimit(1.0f, maxDelaySamples, targetDelay * 1.018f + wowR);
        smoothedDelayL += (tgtL - smoothedDelayL) * 0.0015f;
        smoothedDelayR += (tgtR - smoothedDelayR) * 0.0015f;

        delayL.setDelay(smoothedDelayL);
        delayR.setDelay(smoothedDelayR);
        float dOutL = delayL.popSample(0);
        float dOutR = delayR.popSample(0);

        float dFdbk = p_echo_fdbk->load(std::memory_order_relaxed);
        echoFilterL += (dOutL - echoFilterL) * 0.35f;
        echoFilterR += (dOutR - echoFilterR) * 0.35f;
        delayL.pushSample(0, std::tanh(masterL + echoFilterL * dFdbk));
        delayR.pushSample(0, std::tanh(masterR + echoFilterR * dFdbk));

        float dMix = p_echo_mix->load(std::memory_order_relaxed);
        masterL = masterL * (1.0f - dMix) + dOutL * dMix;
        masterR = masterR * (1.0f - dMix) + dOutR * dMix;

        float mDrive = p_master_drive->load(std::memory_order_relaxed);
        masterL = std::tanh(masterL * (1.0f + mDrive * 4.0f));
        masterR = std::tanh(masterR * (1.0f + mDrive * 4.0f));

        float mVol = p_master_vol->load(std::memory_order_relaxed);
        float outSampleL = masterL * mVol;
        float outSampleR = masterR * mVol;
        if (buffer.getNumChannels() > 0) buffer.addSample(0, sample, outSampleL);
        if (buffer.getNumChannels() > 1) buffer.addSample(1, sample, outSampleR);

        blockPeakL = std::max(blockPeakL, std::abs(outSampleL));
        blockPeakR = std::max(blockPeakR, std::abs(outSampleR));
    }

    float prevL = peakL.load(std::memory_order_relaxed);
    float prevR = peakR.load(std::memory_order_relaxed);
    peakL.store(blockPeakL > prevL ? blockPeakL : prevL * 0.85f, std::memory_order_relaxed);
    peakR.store(blockPeakR > prevR ? blockPeakR : prevR * 0.85f, std::memory_order_relaxed);
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
    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType())) {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}
