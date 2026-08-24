#pragma once
#include <JuceHeader.h>


struct TrackParams {
    std::atomic<float>* steps;
    std::atomic<float>* pulses;
    std::atomic<float>* offset;
    std::atomic<float>* pitch;
    std::atomic<float>* drop;
    std::atomic<float>* morph;
    std::atomic<float>* fold;
    std::atomic<float>* fm;
    std::atomic<float>* rise;
    std::atomic<float>* fall;
    std::atomic<float>* resp;
    std::atomic<float>* brgt;
    std::atomic<float>* noise;
    std::atomic<float>* vol;
};

class OrbitaLPGAudioProcessor; // forward decl

class WestCoastVoice {
public:
    void prepare(double sampleRate) { sr = sampleRate; }
    void trigger(int note, float vel) { 
        env_stage = 1; env = 0.0f; active_note = note; 
    }
    float process(float& outL, float& outR, juce::AudioProcessorValueTreeState& apvts, int trackIdx, float chaos_val, int global_scale);
    
    int current_step = 0;
private:
    double sr = 44100.0;
    float phase = 0.0f;
    float env = 0.0f;
    int env_stage = 0; // 0=idle, 1=rise, 2=fall
    float lpg_state = 0.0f;
    int active_note = 60;
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
    
    // Cached Parameters for DSP (Avoids string allocs in audio thread)
    std::atomic<float>* p_master_vol = nullptr;
    std::atomic<float>* p_bpm = nullptr;
    std::atomic<float>* p_is_playing = nullptr;
    std::atomic<float>* p_chaos = nullptr;
    std::atomic<float>* p_global_scale = nullptr;
    
    std::atomic<float>* p_echo_time = nullptr;
    std::atomic<float>* p_echo_fdbk = nullptr;
    std::atomic<float>* p_echo_mix = nullptr;
    std::atomic<float>* p_echo_wow = nullptr;
    
    TrackParams tParams[6];

    
    // Playback
    bool isPlaying = false;
    bool seqEnabled = true;
    int step_samples_counter = 0;
    bool trackMutes[6] = {false};
    WestCoastVoice voices[6];

    // Delay
    juce::dsp::DelayLine<float> delayL{48000 * 2};
    juce::dsp::DelayLine<float> delayR{48000 * 2};
    float delay_fb_L = 0.0f;
    float delay_fb_R = 0.0f;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrbitaLPGAudioProcessor)
};
