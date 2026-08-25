import re

with open("Source/PluginEditor.h", "r") as f:
    h = f.read()

# Update getLabelFont
h = h.replace("juce::FontOptions(8.5f, juce::Font::bold)", "juce::FontOptions(10.5f, juce::Font::bold)")

with open("Source/PluginEditor.h", "w") as f:
    f.write(h)

with open("Source/PluginEditor.cpp", "r") as f:
    c = f.read()

# Update TextBox bounds for synth sliders
c = c.replace("setTextBoxStyle(juce::Slider::TextBoxBelow, false, 38, 11)", "setTextBoxStyle(juce::Slider::TextBoxBelow, false, 42, 14)")

# Update fader widths in synth area
old_fader = "int requiredWidth = 10 * 38; // 38px por fader"
new_fader = "int requiredWidth = 10 * 46; // 46px por fader"
c = c.replace(old_fader, new_fader)

# Make sure Echo Knobs also get the larger text box
# Actually the first replace already updated both because both were 38, 11!

with open("Source/PluginEditor.cpp", "w") as f:
    f.write(c)

