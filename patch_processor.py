import re

with open("Source/PluginProcessor.h", "r") as f:
    h = f.read()

# Add demo variables
proc_vars = """    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayR{96000};
    float lfo_phase = 0.0f;
    
    // License
    std::atomic<bool> demoExpired{false};
    int demoSampleCount = 0;"""

h = h.replace('    float lfo_phase = 0.0f;', proc_vars)

with open("Source/PluginProcessor.h", "w") as f:
    f.write(h)

with open("Source/PluginProcessor.cpp", "r") as f:
    c = f.read()

# Include LicenseManager.h
c = '#include "LicenseManager.h"\n' + c

# Reset demo timer on prepareToPlay
prep = """void OrbitaLPGAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    if (LicenseManager::isLicensed()) {
        demoExpired.store(false);
        demoSampleCount = 0;
    }"""
c = c.replace('void OrbitaLPGAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {', prep)

# Add demo timer logic in processBlock
pblock = """    if (!LicenseManager::isLicensed()) {
        demoSampleCount += buffer.getNumSamples();
        if (demoSampleCount > getSampleRate() * 60 * 10) {
            demoExpired.store(true);
            buffer.clear();
            return;
        }
    }
    
    // Convert transport info
"""
c = c.replace('    // Convert transport info\n', pblock)

with open("Source/PluginProcessor.cpp", "w") as f:
    f.write(c)

