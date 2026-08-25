import re

with open("Source/PluginEditor.h", "r") as f:
    h = f.read()

# Add the new buttons
if "juce::TextButton rResetBtn;" not in h:
    h = h.replace("juce::TextButton rRandBtn;", "juce::TextButton rRandBtn;\n    juce::TextButton rResetBtn;")
    h = h.replace("juce::TextButton sRandBtn;", "juce::TextButton sRandBtn;\n    juce::TextButton sResetBtn;")

    with open("Source/PluginEditor.h", "w") as f:
        f.write(h)

with open("Source/PluginEditor.cpp", "r") as f:
    c = f.read()

# Register the buttons in the constructor
btn_def = """    btn(rRandBtn,   juce::Colour(30,35,42), juce::Colours::white);
    btn(rResetBtn,  juce::Colour(60,20,25), juce::Colours::white);
    rResetBtn.setButtonText("RESET");

    btn(sRandBtn,   juce::Colour(30,35,42), juce::Colours::white);
    btn(sResetBtn,  juce::Colour(60,20,25), juce::Colours::white);
    sResetBtn.setButtonText("RESET");"""

c = re.sub(r'    btn\(rRandBtn,.*?white\);\n    btn\(sRandBtn,.*?white\);', btn_def, c, flags=re.DOTALL)

# Add onClick logic right after sRandBtn.onClick
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

c = c.replace('sRandBtn.onClick = [this]() {', logic + '\n    sRandBtn.onClick = [this]() {')

# Adjust bounds in resized()
# Rhythm area
old_r_bounds = """        rRandBtn.setBounds(rowA.removeFromLeft(55).reduced(0,1));"""
new_r_bounds = """        rRandBtn.setBounds(rowA.removeFromLeft(50).reduced(0,1));
        rowA.removeFromLeft(4);
        rResetBtn.setBounds(rowA.removeFromLeft(50).reduced(0,1));"""
c = c.replace(old_r_bounds, new_r_bounds)

# Synth area
old_s_bounds = """        sRandBtn.setButtonText("RANDOM"); // Mas visible
        sRandBtn.setBounds(sRow2.removeFromLeft(80).reduced(0,1)); sRow2.removeFromLeft(15);"""
new_s_bounds = """        sRandBtn.setButtonText("RAND"); // Mas visible
        sRandBtn.setBounds(sRow2.removeFromLeft(55).reduced(0,1)); sRow2.removeFromLeft(4);
        sResetBtn.setBounds(sRow2.removeFromLeft(55).reduced(0,1)); sRow2.removeFromLeft(15);"""
c = c.replace(old_s_bounds, new_s_bounds)

with open("Source/PluginEditor.cpp", "w") as f:
    f.write(c)

