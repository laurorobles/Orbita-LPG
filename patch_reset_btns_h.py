import re

with open("Source/PluginEditor.h", "r") as f:
    h = f.read()

h = h.replace('juce::TextButton rRandBtn{"RAND"};', 'juce::TextButton rRandBtn{"RAND"};\n    juce::TextButton rResetBtn{"RESET"};')
h = h.replace('juce::TextButton sRandBtn{"RAND"};', 'juce::TextButton sRandBtn{"RAND"};\n    juce::TextButton sResetBtn{"RESET"};')

with open("Source/PluginEditor.h", "w") as f:
    f.write(h)

