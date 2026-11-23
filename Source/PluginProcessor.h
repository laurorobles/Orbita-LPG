#pragma once
#include <JuceHeader.h>
#include <array>
#include <atomic>
#include <cmath>
#include <vector>

namespace OrbitaDSP {
    inline bool isEuclideanHit(int step, int pulses, int steps, int offset) {
        if (steps <= 0 || pulses <= 0) return false;
        if (pulses >= steps) return true;
        int idx = ((step - offset) % steps + steps) % steps;
        return ((idx * pulses) % steps) < pulses;
    }

    inline float quantizePitch(float raw_midi, int scale_idx, int root_note) {
        static const std::vector<std::vector<int>> SCALES = {
            {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, {0, 2, 4, 5, 7, 9, 11}, {0, 2, 3, 5, 7, 8, 10}, {0, 2, 3, 5, 7, 9, 10},
            {0, 1, 3, 5, 7, 8, 10}, {0, 2, 4, 6, 7, 9, 11}, {0, 2, 4, 5, 7, 9, 10}, {0, 2, 4, 7, 9}, {0, 3, 5, 7, 10},
            {0, 2, 3, 5, 7, 8, 11}, {0, 1, 4, 5, 7, 8, 10}, {0, 2, 3, 7, 8}, {0, 2, 4, 6, 8, 10}, {0, 1, 3, 4, 6, 7, 9, 10}
        };
        scale_idx = juce::jlimit(1, 14, scale_idx);
        const auto& scale = SCALES[(size_t)(scale_idx - 1)];
        int shifted = (int)std::round(raw_midi) - root_note;
        int octave = (int)std::floor(shifted / 12.0f);
        int note = (shifted % 12 + 12) % 12;
        int closest_note = scale[0];
        int min_dist = 100;
        for (int s : scale) {
            int dist = std::abs(note - s);
            if (dist < min_dist) { min_dist = dist; closest_note = s; }
        }
        return (float)(octave * 12 + closest_note + root_note);
    }
}

struct TrackParams {
    std::atomic<float>* steps = nullptr;
    std::atomic<float>* pulses = nullptr;
    std::atomic<float>* offset = nullptr;
    std::atomic<float>* rate = nullptr;
    std::atomic<float>* pitch = nullptr;
    std::atomic<float>* drop = nullptr;
    std::atomic<float>* morph = nullptr;
    std::atomic<float>* fold = nullptr;
    std::atomic<float>* fm = nullptr;
    std::atomic<float>* rise = nullptr;
    std::atomic<float>* fall = nullptr;
    std::atomic<float>* resp = nullptr;
    std::atomic<float>* brgt = nullptr;
    std::atomic<float>* reso = nullptr;
    std::atomic<float>* noise = nullptr;
    std::atomic<float>* vol = nullptr;
    std::atomic<float>* notemode = nullptr;
    std::atomic<float>* mode281 = nullptr;
    std::atomic<float>* mode292 = nullptr;
};

class WestCoastVoice {
public:
    int active_midi_note = -1;
    int midi_gate_samples = 0;
    std::atomic<int> current_step{-1};
    std::atomic<float> visual_lpg{0.0f};
    float lpg_state = 0.0f;
    float current_pitch = 60.0f;
    int active_note = 60;

    void prepare(double sampleRate) {
        sr = sampleRate > 0.0 ? sampleRate : 44100.0;
        vactrol_open = 1.0f - std::exp(-1000.0f / (2.0f * (float)sr));
    }
    void trigger(float target_pitch, float drop_amt, float chaos_amt, float gate_len);
    void releaseGate();
    void reset();
    float process(float& outL, float& outR, const TrackParams& params);

private:
    double sr = 44100.0;
    float vactrol_open = 0.011f;
    float phase = 0.0f;
    float env = 0.0f;
    int env_stage = 0;
    float gate_samples = 0.0f;
    float chaos_latch = 0.0f;
    float last_chaos_amt = 0.0f;
    float last_drop_amt = 0.0f;
    float last_fold_in = 0.0f;
    float lpf_state = 0.0f;
    float bandpass_state = 0.0f;

    uint32_t prng_state = 2463534242u;
    float xorshift_float() {
        prng_state ^= prng_state << 13;
        prng_state ^= prng_state >> 17;
        prng_state ^= prng_state << 5;
        return (float)prng_state / (float)0xFFFFFFFFu;
    }
};

class OrbitaLPGAudioProcessor : public juce::AudioProcessor {
public:
    OrbitaLPGAudioProcessor();
    ~OrbitaLPGAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Orbita-LPG"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    std::atomic<float>* p_master_vol = nullptr;
    std::atomic<float>* p_master_drive = nullptr;
    std::atomic<float>* p_bpm = nullptr;
    std::atomic<float>* p_swing = nullptr;
    std::atomic<float>* p_chaos = nullptr;
    std::atomic<float>* p_global_scale = nullptr;
    std::atomic<float>* p_global_root = nullptr;

    std::atomic<float>* p_echo_time = nullptr;
    std::atomic<float>* p_echo_fdbk = nullptr;
    std::atomic<float>* p_echo_mix = nullptr;
    std::atomic<float>* p_echo_wow = nullptr;
    std::atomic<float>* p_echo_sync = nullptr;

    TrackParams tParams[6];
    std::atomic<bool> isPlaying{false};
    std::atomic<bool> hostPlaying{false};
    std::atomic<bool> seqEnabled{true};
    std::atomic<bool> pendingReset{false};
    std::array<std::atomic<bool>, 6> trackMutes{};
    std::array<std::atomic<bool>, 6> pendingStrike{};
    std::atomic<float> peakL{0.0f};
    std::atomic<float> peakR{0.0f};

    double internal_ppq = 0.0;
    WestCoastVoice voices[6];

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayL{192000};
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayR{192000};
    float smoothedDelayL = 12000.0f;
    float smoothedDelayR = 12200.0f;
    float echoFilterL = 0.0f;
    float echoFilterR = 0.0f;
    float lfo_phase = 0.0f;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrbitaLPGAudioProcessor)
};
