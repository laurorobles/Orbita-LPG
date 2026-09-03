import re

with open("Source/PluginEditor.h", "r") as f:
    h = f.read()

# Add include and member
inc = '#pragma once\n#include <juce_audio_processors/juce_audio_processors.h>\n#include "PluginProcessor.h"\n#include "UIComponents.h"\n'
h = re.sub(r'#pragma once.*?#include "PluginProcessor.h"\n', inc, h, flags=re.DOTALL)

members = """    juce::TextButton loadBtn{"LOAD"}, saveBtn{"SAVE"};
    std::unique_ptr<juce::FileChooser> chooser;
    
    // License
    ActivationOverlayComponent activationOverlay;
    juce::TextButton licenseBadgeButton{"ACTIVATE"};
    bool isActivated = false;
    void updateLicenseState();"""
h = h.replace('    std::unique_ptr<juce::FileChooser> chooser;', members)

with open("Source/PluginEditor.h", "w") as f:
    f.write(h)


with open("Source/PluginEditor.cpp", "r") as f:
    c = f.read()

# Add License logic to constructor
c_init = """    startTimer(100);
    
    // License
    addAndMakeVisible(licenseBadgeButton);
    licenseBadgeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffe67e22));
    licenseBadgeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    updateLicenseState();
    
    licenseBadgeButton.onClick = [this]() {
        activationOverlay.isExpired = audioProcessor.demoExpired.load();
        activationOverlay.setVisible(true);
    };
    
    addChildComponent(activationOverlay);
    activationOverlay.onActivate = [this](const juce::String& key) {
        if (LicenseManager::saveLicense(key)) {
            updateLicenseState();
            activationOverlay.statusLabel.setText("License Activated Successfully! Welcome to ORBITA-LPG.", juce::dontSendNotification);
            juce::Timer::callAfterDelay(1500, [this]() { activationOverlay.setVisible(false); });
        } else {
            activationOverlay.statusLabel.setText("Invalid Serial Key. Please check and try again.", juce::dontSendNotification);
        }
    };
    
    activationOverlay.onContinueDemo = [this]() {
        activationOverlay.setVisible(false);
    };"""
c = c.replace('    startTimer(100);', c_init)

# Add updateLicenseState method
methods = """void OrbitaLPGAudioProcessorEditor::updateLicenseState() {
    isActivated = LicenseManager::isLicensed();
    if (isActivated) {
        licenseBadgeButton.setVisible(false);
        audioProcessor.demoExpired.store(false);
        audioProcessor.demoSampleCount = 0;
    } else {
        licenseBadgeButton.setVisible(true);
        if (audioProcessor.demoExpired.load()) {
            activationOverlay.isExpired = true;
            activationOverlay.setVisible(true);
        } else if (!activationOverlay.isVisible() && audioProcessor.demoSampleCount == 0) {
            activationOverlay.setVisible(true);
        }
    }
}

void OrbitaLPGAudioProcessorEditor::timerCallback() {"""
c = c.replace('void OrbitaLPGAudioProcessorEditor::timerCallback() {', methods)

# timer logic
t_log = """void OrbitaLPGAudioProcessorEditor::timerCallback() {
    if (!isActivated && audioProcessor.demoExpired.load() && !activationOverlay.isVisible()) {
        updateLicenseState();
    }"""
c = c.replace('void OrbitaLPGAudioProcessorEditor::timerCallback() {', t_log)

# resized logic
r_log = """        seqBtn.setBounds( r.removeFromLeft(60).reduced(0,2)); r.removeFromLeft(5); 
        configBtn.setBounds(r.removeFromRight(75).reduced(0,2));
        
        licenseBadgeButton.setBounds(r.removeFromRight(80).reduced(0,2)); r.removeFromRight(10);
        
        activationOverlay.setBounds(getLocalBounds());"""
c = c.replace('        seqBtn.setBounds( r.removeFromLeft(60).reduced(0,2)); r.removeFromLeft(5); \n        configBtn.setBounds(r.removeFromRight(75).reduced(0,2));', r_log)

with open("Source/PluginEditor.cpp", "w") as f:
    f.write(c)
