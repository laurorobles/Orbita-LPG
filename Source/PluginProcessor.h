#pragma once
#include <JuceHeader.h>

struct TrackParams {
    std::atomic<float>* steps;
    std::atomic<float>* pulses;
    std::atomic<float>* offset;
    std::atomic<float>* rate; 
    std::atomic<float>* pitch;
    std::atomic<float>* drop;
    std::atomic<float>* morph;
    std::atomic<float>* fold;
    std::atomic<float>* fm;
    std::atomic<float>* rise;
    std::atomic<float>* fall;
    std::atomic<float>* resp;
    std::atomic<float>* brgt;
    std::atomic<float>* reso; 
    std::atomic<float>* noise;
    std::atomic<float>* vol;
    std::atomic<float>* notemode; 
    std::atomic<float>* mode281; 
    std::atomic<float>* mode292; 
};

class WestCoastVoice {
public:
    void prepare(double sampleRate) { sr = sampleRate; }
    void trigger(float target_pitch, float drop_amt, float chaos_amt, float gate_len);
    void releaseGate();
    void reset();
    float process(float& outL, float& outR, const TrackParams& params);
    
    int current_step = -1;
    float lpg_state = 0.0f;
    float current_pitch = 60.0f;
    int active_note = 60;
    
private:
    double sr = 44100.0;
    float phase = 0.0f;
    float env = 0.0f;
    int env_stage = 0; 
    float gate_samples = 0.0f;
    float chaos_latch = 0.0f;
    float last_chaos_amt = 0.0f;
    float last_drop_amt = 0.0f;
    float lpf_state = 0.0f;
    float bandpass_state = 0.0f; 
    
    uint32_t prng_state = 2463534242; 
    float xorshift_float() {
        prng_state ^= prng_state << 13; prng_state ^= prng_state >> 17;
        prng_state ^= prng_state << 5; return (prng_state / (float)0xFFFFFFFF);
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
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int index) override {}
    const juce::String getProgramName(int index) override { return {}; }
    void changeProgramName(int index, const juce::String& newName) override {}
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    
    std::atomic<float>* p_master_vol = nullptr;
    std::atomic<float>* p_master_drive = nullptr;
    std::atomic<float>* p_bpm = nullptr;
    std::atomic<float>* p_is_playing = nullptr;
    std::atomic<float>* p_chaos = nullptr;
    std::atomic<float>* p_global_scale = nullptr;
    std::atomic<float>* p_global_root = nullptr;
    
    std::atomic<float>* p_echo_time = nullptr;
    std::atomic<float>* p_echo_fdbk = nullptr;
    std::atomic<float>* p_echo_mix = nullptr;
    std::atomic<float>* p_echo_wow = nullptr;
    std::atomic<float>* p_echo_sync = nullptr;
    
    TrackParams tParams[6];
    bool isPlaying = false;
    double internal_ppq = 0.0;
    bool seqEnabled = true;
    
    float track_samples_counter[6] = {0.0f}; 
    bool trackMutes[6] = {false};
    WestCoastVoice voices[6];

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayL{96000};
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayR{96000};
    float lfo_phase = 0.0f;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrbitaLPGAudioProcessor)
};