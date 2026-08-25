import re
with open("Source/PluginEditor.cpp", "r") as f: c = f.read()

if "OrbitaLPGAudioProcessorEditor::timerCallback()" not in c:
    c = c.replace("OrbitaLPGAudioProcessorEditor::OrbitaLPGAudioProcessorEditor(OrbitaLPGAudioProcessor& p)", "void OrbitaLPGAudioProcessorEditor::timerCallback() {\n    repaint(); // Trigger LED updates\n}\n\nvoid OrbitaLPGAudioProcessorEditor::setParam(juce::String id, float val) {\n    if (auto* param = audioProcessor.apvts.getParameter(id)) {\n        param->beginChangeGesture();\n        param->setValueNotifyingHost(param->convertTo0to1(val));\n        param->endChangeGesture();\n    }\n}\n\nOrbitaLPGAudioProcessorEditor::OrbitaLPGAudioProcessorEditor(OrbitaLPGAudioProcessor& p)")
    c = c.replace("    updatePitchDisplay();\n", "    updatePitchDisplay();\n    startTimerHz(30);\n")

led_code = """    // --- Draw LEDs ---
    auto si = synthArea.reduced(10);
    si.removeFromTop(14 + 20 + 5 + 20 + 10); // Skip to faders
    int sw = si.getWidth() / 10;
    auto ledZone = si.removeFromLeft(sw); // We can draw LEDs near the left edge
    for (int t=0; t<6; ++t) {
        float state = audioProcessor.voices[t].lpg_state;
        juce::Colour ledCol = juce::Colours::red.withAlpha(state);
        g.setColour(ledCol);
        g.fillEllipse(synthArea.getX() + 5, synthArea.getY() + 100 + t*30, 8, 8);
        g.setColour(juce::Colours::black);
        g.drawEllipse(synthArea.getX() + 5, synthArea.getY() + 100 + t*30, 8, 8, 1.0f);
    }
"""
if "Draw LEDs" not in c:
    c = c.replace("    // Pinta lineas de los rotarios", led_code + "\n    // Pinta lineas de los rotarios")

old_preset = r'                stepsSld\[t\]\.setValue\(st\); pulsesSld\[t\]\.setValue\(pu\); offsetSld\[t\]\.setValue\(of\);'
new_preset = r'                juce::String ts = "t" + juce::String(t+1) + "_"; setParam(ts+"steps", st); setParam(ts+"pulses", pu); setParam(ts+"offset", of);'
c = re.sub(old_preset, new_preset, c)

def repl(m): return f'setParam("t" + juce::String(t+1) + "_{["pitch","drop","morph","fold","fm","rise","fall","resp","brgt","vol"][int(m.group(1))]}", {m.group(2)});'
c = re.sub(r'vSliders\[t\]\[([0-9])\]\.slider\.setValue\((.*?)\);', repl, c)

def repl_ct(m): return f'setParam("t" + juce::String(currentTrack+1) + "_{["pitch","drop","morph","fold","fm","rise","fall","resp","brgt","vol"][int(m.group(1))]}", {m.group(2)});'
c = re.sub(r'vSliders\[currentTrack\]\[([0-9])\]\.slider\.setValue\((.*?)\);', repl_ct, c)

c = c.replace("setTextBoxStyle(juce::Slider::TextBoxBelow, false, 38, 11)", "setTextBoxStyle(juce::Slider::TextBoxBelow, false, 42, 14)")
c = c.replace("int requiredWidth = 10 * 38; // 38px por fader", "int requiredWidth = 10 * 46; // 46px por fader")
c = c.replace("noteBtn.setBounds(    sRow1.removeFromRight(50).reduced(0,1));", "noteBtn.setBounds(    sRow1.removeFromRight(70).reduced(0,1));")
c = c.replace("copyNextBtn.setBounds(sRow1.removeFromRight(90).reduced(0,1));", "copyNextBtn.setBounds(sRow1.removeFromRight(80).reduced(0,1));")
c = c.replace("copyLastBtn.setBounds(sRow1.removeFromRight(90).reduced(0,1));", "copyLastBtn.setBounds(sRow1.removeFromRight(80).reduced(0,1));")

btn_def = """    btn(rRandBtn,   juce::Colour(30,35,42), juce::Colours::white);
    btn(rResetBtn,  juce::Colour(60,20,25), juce::Colours::white);

    btn(sRandBtn,   juce::Colour(30,35,42), juce::Colours::white);
    btn(sResetBtn,  juce::Colour(60,20,25), juce::Colours::white);"""
c = re.sub(r'    btn\(rRandBtn,.*?white\);\n    btn\(sRandBtn,.*?white\);', btn_def, c, flags=re.DOTALL)

logic = """
    rResetBtn.onClick = [this]() {
        juce::String ts = "t" + juce::String(currentTrack+1) + "_";
        setParam(ts+"steps", 16.0f);
        setParam(ts+"pulses", 4.0f);
        setParam(ts+"offset", 0.0f);
    };

    sResetBtn.onClick = [this]() {
        juce::String ts = "t" + juce::String(currentTrack+1) + "_";
        setParam(ts+"pitch", 60.0f);
        setParam(ts+"drop", 0.0f);
        setParam(ts+"morph", 0.0f);
        setParam(ts+"fold", 0.0f);
        setParam(ts+"fm", 0.0f);
        setParam(ts+"rise", 0.01f);
        setParam(ts+"fall", 0.5f);
        setParam(ts+"resp", 0.5f);
        setParam(ts+"brgt", 0.8f);
        setParam(ts+"noise", 0.0f);
        setParam(ts+"vol", 0.8f);
    };
"""
if "rResetBtn.onClick" not in c:
    c = c.replace('sRandBtn.onClick = [this]() {', logic + '\n    sRandBtn.onClick = [this]() {')

c = c.replace('rRandBtn.setBounds(rowA.removeFromLeft(55).reduced(0,1));', 'rRandBtn.setBounds(rowA.removeFromLeft(50).reduced(0,1));\n        rowA.removeFromLeft(4);\n        rResetBtn.setBounds(rowA.removeFromLeft(50).reduced(0,1));')
c = c.replace('sRandBtn.setButtonText("RANDOM");', 'sRandBtn.setButtonText("RAND");')
c = c.replace('sRandBtn.setBounds(sRow2.removeFromLeft(80).reduced(0,1)); sRow2.removeFromLeft(15);', 'sRandBtn.setBounds(sRow2.removeFromLeft(55).reduced(0,1)); sRow2.removeFromLeft(4);\n        sResetBtn.setBounds(sRow2.removeFromLeft(55).reduced(0,1)); sRow2.removeFromLeft(15);')

with open("Source/PluginEditor.cpp", "w") as f: f.write(c)
