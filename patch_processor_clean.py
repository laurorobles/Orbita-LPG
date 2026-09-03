with open("Source/PluginProcessor.h", "r") as f:
    h = f.read()

# Add demo variables safely
h = h.replace('    float lfo_phase = 0.0f;', '    float lfo_phase = 0.0f;\n    std::atomic<bool> demoExpired{false};\n    int demoSampleCount = 0;')

with open("Source/PluginProcessor.h", "w") as f:
    f.write(h)

with open("Source/PluginProcessor.cpp", "r") as f:
    c = f.read()

# include LicenseManager after JuceHeader
c = c.replace('#include "PluginProcessor.h"', '#include "PluginProcessor.h"\n#include "LicenseManager.h"')

prep = """void OrbitaLPGAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    if (LicenseManager::isLicensed()) {
        demoExpired.store(false);
        demoSampleCount = 0;
    }"""
c = c.replace('void OrbitaLPGAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {', prep)

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
