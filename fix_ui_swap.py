import re

with open("Source/PluginEditor.h", "r") as f:
    h = f.read()

# Replace singular sliders with arrays of 6
h = re.sub(r'juce::Slider stepsSld, pulsesSld, offsetSld;', 'juce::Slider stepsSld[6], pulsesSld[6], offsetSld[6];', h)
h = re.sub(r'UIControl vSliders\[10\];', 'UIControl vSliders[6][10];', h)
h = re.sub(r'std::unique_ptr<SldAtt> rAtt\[3\];', 'std::unique_ptr<SldAtt> rAtt[6][3];', h)
h = re.sub(r'std::unique_ptr<SldAtt> sAtt\[10\];', 'std::unique_ptr<SldAtt> sAtt[6][10];', h)

with open("Source/PluginEditor.h", "w") as f:
    f.write(h)

with open("Source/PluginEditor.cpp", "r") as f:
    c = f.read()

# In the constructor, we need to initialize all 6 sets of sliders and their attachments.
# We also need to change patternsCombo to update the current track's sliders.
# And selectTrack will just loop through 6 and setVisible(true/false).

c = c.replace('rhythmSld(stepsSld); rhythmSld(pulsesSld); rhythmSld(offsetSld);',
"""for (int t = 0; t < 6; ++t) {
        rhythmSld(stepsSld[t]); rhythmSld(pulsesSld[t]); rhythmSld(offsetSld[t]);
    }""")

# Patterns combo
c = c.replace('if (id == 2) { stepsSld.setValue(8);  pulsesSld.setValue(3); offsetSld.setValue(0); }',
'if (id == 2) { stepsSld[currentTrack].setValue(8);  pulsesSld[currentTrack].setValue(3); offsetSld[currentTrack].setValue(0); }')
c = c.replace('if (id == 3) { stepsSld.setValue(16); pulsesSld.setValue(5); offsetSld.setValue(0); }',
'if (id == 3) { stepsSld[currentTrack].setValue(16); pulsesSld[currentTrack].setValue(5); offsetSld[currentTrack].setValue(0); }')
c = c.replace('if (id == 4) { stepsSld.setValue(16); pulsesSld.setValue(7); offsetSld.setValue(0); }',
'if (id == 4) { stepsSld[currentTrack].setValue(16); pulsesSld[currentTrack].setValue(7); offsetSld[currentTrack].setValue(0); }')
c = c.replace('if (id == 5) { stepsSld.setValue(16); pulsesSld.setValue(4); offsetSld.setValue(0); }',
'if (id == 5) { stepsSld[currentTrack].setValue(16); pulsesSld[currentTrack].setValue(4); offsetSld[currentTrack].setValue(0); }')

# vSliders initialization
old_vslider_init = """    for (int i = 0; i < 10; ++i) {
        vSliders[i].slider.setSliderStyle(juce::Slider::LinearVertical);
        vSliders[i].slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 36, 13);
        addAndMakeVisible(vSliders[i].slider);
        lbl(vSliders[i].label, sNames[i].toRawUTF8(), sCols[i]);
    }"""

new_vslider_init = """    for (int t = 0; t < 6; ++t) {
        juce::String ts = "t" + juce::String(t+1) + "_";
        rAtt[t][0] = std::make_unique<SldAtt>(audioProcessor.apvts, ts+"steps",  stepsSld[t]);
        rAtt[t][1] = std::make_unique<SldAtt>(audioProcessor.apvts, ts+"pulses", pulsesSld[t]);
        rAtt[t][2] = std::make_unique<SldAtt>(audioProcessor.apvts, ts+"offset", offsetSld[t]);
        
        juce::String sIds[] = {"pitch","drop","morph","fold","fm","rise","fall","resp","brgt","vol"};
        for (int i = 0; i < 10; ++i) {
            vSliders[t][i].slider.setSliderStyle(juce::Slider::LinearVertical);
            vSliders[t][i].slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 36, 13);
            addChildComponent(vSliders[t][i].slider);
            if (t == 0) lbl(vSliders[0][i].label, sNames[i].toRawUTF8(), sCols[i]);
            sAtt[t][i] = std::make_unique<SldAtt>(audioProcessor.apvts, ts+sIds[i], vSliders[t][i].slider);
        }
    }"""
c = c.replace(old_vslider_init, new_vslider_init)


# selectTrack
old_select = """    juce::String ts = "t" + juce::String(t+1) + "_";
    rAtt[0] = std::make_unique<SldAtt>(audioProcessor.apvts, ts+"steps",  stepsSld);
    rAtt[1] = std::make_unique<SldAtt>(audioProcessor.apvts, ts+"pulses", pulsesSld);
    rAtt[2] = std::make_unique<SldAtt>(audioProcessor.apvts, ts+"offset", offsetSld);
    juce::String sIds[] = {"pitch","drop","morph","fold","fm","rise","fall","resp","brgt","vol"};
    for (int i = 0; i < 10; ++i)
        sAtt[i] = std::make_unique<SldAtt>(audioProcessor.apvts, ts+sIds[i], vSliders[i].slider);"""

new_select = """    for (int i = 0; i < 6; ++i) {
        bool show = (i == t);
        stepsSld[i].setVisible(show);
        pulsesSld[i].setVisible(show);
        offsetSld[i].setVisible(show);
        for (int j = 0; j < 10; ++j) {
            vSliders[i][j].slider.setVisible(show);
        }
    }"""
c = c.replace(old_select, new_select)


# resized() - setBounds for all 6 tracks
c = c.replace('placeRhythm(stepsLbl,  stepsSld);',
"""for (int t = 0; t < 6; ++t) {
            stepsSld[t].setBounds(row);
        }
        placeRhythm(stepsLbl, stepsSld[0]); // dummy to consume bounds""")
c = c.replace('placeRhythm(pulsesLbl, pulsesSld);',
"""for (int t = 0; t < 6; ++t) pulsesSld[t].setBounds(row);
        placeRhythm(pulsesLbl, pulsesSld[0]);""")
c = c.replace('placeRhythm(offsetLbl, offsetSld);',
"""for (int t = 0; t < 6; ++t) offsetSld[t].setBounds(row);
        placeRhythm(offsetLbl, offsetSld[0]);""")


c = c.replace('vSliders[i].label.setBounds(cell.removeFromTop(12));', 'vSliders[0][i].label.setBounds(cell.removeFromTop(12));')
c = c.replace('vSliders[i].slider.setBounds(cell);', 
"""for (int t = 0; t < 6; ++t) {
                vSliders[t][i].slider.setBounds(cell);
            }""")

with open("Source/PluginEditor.cpp", "w") as f:
    f.write(c)

print("done")
