import re

with open("Source/PluginEditor.h", "r") as f:
    h = f.read()

h = h.replace("juce::Label titleLabel;", "juce::Label titleLabel, creditLabel;")

with open("Source/PluginEditor.h", "w") as f:
    f.write(h)

with open("Source/PluginEditor.cpp", "r") as f:
    c = f.read()

init_code = """    titleLabel.setText("ORBITA-LPG   ///   6-VOICE MATRIX", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);
    
    creditLabel.setText("Lauro Robles / Extasis Records", juce::dontSendNotification);
    creditLabel.setFont(juce::Font(9.0f));
    creditLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.2f));
    creditLabel.setJustificationType(juce::Justification::bottomRight);
    addAndMakeVisible(creditLabel);"""

c = re.sub(r'    titleLabel\.setText\("ORBITA-LPG   ///   6-VOICE MATRIX", juce::dontSendNotification\);\n    titleLabel\.setFont\(juce::Font\(18\.0f, juce::Font::bold\)\);\n    titleLabel\.setColour\(juce::Label::textColourId, juce::Colours::white\);\n    addAndMakeVisible\(titleLabel\);', init_code, c)

resize_code = """    }
    
    creditLabel.setBounds(getWidth() - 155, getHeight() - 18, 150, 15);
}

void OrbitaLPGAudioProcessorEditor::selectTrack(int t) {"""

c = c.replace("""    }
}

void OrbitaLPGAudioProcessorEditor::selectTrack(int t) {""", resize_code)

with open("Source/PluginEditor.cpp", "w") as f:
    f.write(c)
