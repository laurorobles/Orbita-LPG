import re
with open("Source/PluginProcessor.h", "r") as f: h = f.read()
if "juce::dsp::Oversampling<float> oversampler" not in h:
    h = h.replace("juce::dsp::DelayLine<float> delayL{48000 * 2};\n    juce::dsp::DelayLine<float> delayR{48000 * 2};", "juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayL{48000 * 2};\n    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayR{48000 * 2};\n    juce::dsp::Oversampling<float> oversampler;")
    h = h.replace("float lpg_state = 0.0f;", "public:\n    float lpg_state = 0.0f;\nprivate:")
with open("Source/PluginProcessor.h", "w") as f: f.write(h)

with open("Source/PluginProcessor.cpp", "r") as f: c = f.read()
if "oversampler(2, 1" not in c:
    c = c.replace("apvts(*this, nullptr, \"PARAMETERS\", createParameterLayout())", "apvts(*this, nullptr, \"PARAMETERS\", createParameterLayout()),\n       oversampler(2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true)")

c = c.replace("""    for (int i = 0; i < 6; i++) {
        voices[i].prepare(sampleRate);
        voices[i].current_step = -1;
    }
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;""", """    oversampler.initProcessing(samplesPerBlock);
    
    for (int i = 0; i < 6; i++) {
        voices[i].prepare(sampleRate * oversampler.getOversamplingFactor());
        voices[i].current_step = -1;
    }
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;""")

old_process = """        float outL = 0.0f, outR = 0.0f;
        for (int t=0; t<6; ++t) {
            if (!trackMutes[t]) voices[t].process(outL, outR, tParams[t], chaos, g_scale, quantize[t].load());
        }"""
new_process = """        // --- OVERSAMPLING BLOCK ---
        juce::AudioBuffer<float> dummy (2, 1);
        dummy.clear();
        juce::dsp::AudioBlock<float> dummyBlock (dummy);
        juce::dsp::AudioBlock<float> upsampledBlock = oversampler.processSamplesUp(dummyBlock);
        
        int factor = (int)oversampler.getOversamplingFactor();
        for (int sub_sample = 0; sub_sample < factor; ++sub_sample) {
            float subL = 0.0f, subR = 0.0f;
            for (int t=0; t<6; ++t) {
                if (!trackMutes[t]) voices[t].process(subL, subR, tParams[t], chaos, g_scale, quantize[t].load());
            }
            upsampledBlock.setSample(0, sub_sample, subL);
            upsampledBlock.setSample(1, sub_sample, subR);
        }
        
        oversampler.processSamplesDown(dummyBlock);
        juce::dsp::AudioBlock<float> outBlock = dummyBlock;
        float outL = outBlock.getSample(0, 0);
        float outR = outBlock.getSample(1, 0);
        // --------------------------"""
c = c.replace(old_process, new_process)
with open("Source/PluginProcessor.cpp", "w") as f: f.write(c)
