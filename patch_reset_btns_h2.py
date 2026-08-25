import re

with open("Source/PluginEditor.h", "r") as f:
    h = f.read()

h = h.replace('juce::TextButton sRandBtn{"RAND"}, copyLastBtn{"COPY TO LAST"}, copyNextBtn{"COPY TO NEXT"}, noteBtn{"NOTE"};', 'juce::TextButton sRandBtn{"RAND"}, sResetBtn{"RESET"}, copyLastBtn{"COPY TO LAST"}, copyNextBtn{"COPY TO NEXT"}, noteBtn{"NOTE"};')

with open("Source/PluginEditor.h", "w") as f:
    f.write(h)
