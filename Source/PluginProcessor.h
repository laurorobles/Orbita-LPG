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
    std::atomic<float>* notemode; 
};

class OrbitaLPGAudioProcessor;

class WestCoastVoice {
public:
    void prepare(double sampleRate) { sr = sampleRate; }
    void trigger(int note, float vel) { 
        env_stage = 1; env = 0.0f; active_note = note; 
    }
    float process(float& outL, float& outR, const TrackParams& params, float chaos_val, int global_scale);
    
    int current_step = 0;
    float lpg_state = 0.0f;
private:
    double sr = 44100.0;
    float phase = 0.0f;
    float env = 0.0f;
    int env_stage = 0; 
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
    
    bool isPlaying = false;
    bool seqEnabled = true;
    int step_samples_counter = 0;
    bool trackMutes[6] = {false};
    WestCoastVoice voices[6];

    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayL{48000 * 2};
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayR{48000 * 2};
    juce::dsp::Oversampling<float> oversampler;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrbitaLPGAudioProcessor)
};