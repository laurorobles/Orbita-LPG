with open("Source/PluginEditor.h", "r") as f:
    h = f.read()

h = h.replace('#include "PluginProcessor.h"', '#include "PluginProcessor.h"\n#include "UIComponents.h"')

members = """    std::unique_ptr<juce::FileChooser> chooser; 
    
    // License
    ActivationOverlayComponent activationOverlay;
    juce::TextButton licenseBadgeButton{"ACTIVATE"};
    bool isActivated = false;
    void updateLicenseState();"""

h = h.replace('    std::unique_ptr<juce::FileChooser> chooser; ', members)

with open("Source/PluginEditor.h", "w") as f:
    f.write(h)
