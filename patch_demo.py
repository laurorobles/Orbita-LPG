import sys

with open("Source/PluginProcessor.cpp", "r") as f:
    text = f.read()

target = "void OrbitaLPGAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {\n    juce::ScopedNoDenormals noDenormals; buffer.clear();"

replacement = """void OrbitaLPGAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    juce::ScopedNoDenormals noDenormals; buffer.clear();
    
    if (!LicenseManager::isLicensed()) {
        demoSampleCount += buffer.getNumSamples();
        if (demoSampleCount > getSampleRate() * 60 * 10) {
            demoExpired.store(true);
            return;
        }
    }"""

if target in text:
    text = text.replace(target, replacement)
    with open("Source/PluginProcessor.cpp", "w") as f:
        f.write(text)
    print("Patched successfully")
else:
    print("Target not found")
