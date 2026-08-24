import re

with open("Source/PluginEditor.cpp", "r") as f:
    c = f.read()

old = """        auto placeRhythm = [&](juce::Label& l, juce::Slider& s) {
            auto row = controlZone.removeFromTop(20);
            int lw = 45;
            l.setBounds(row.removeFromLeft(lw));
            row.removeFromLeft(2); // REGLA 2PX
            s.setBounds(row);
            controlZone.removeFromTop(4);
        };
        for (int t = 0; t < 6; ++t) {
            stepsSld[t].setBounds(row);
        }
        placeRhythm(stepsLbl, stepsSld[0]); // dummy to consume bounds
        for (int t = 0; t < 6; ++t) pulsesSld[t].setBounds(row);
        placeRhythm(pulsesLbl, pulsesSld[0]);
        for (int t = 0; t < 6; ++t) offsetSld[t].setBounds(row);
        placeRhythm(offsetLbl, offsetSld[0]);"""

new = """        auto placeRhythm = [&](juce::Label& l, juce::Slider* sArr) {
            auto row = controlZone.removeFromTop(20);
            int lw = 45;
            l.setBounds(row.removeFromLeft(lw));
            row.removeFromLeft(2); // REGLA 2PX
            for(int i=0; i<6; i++) sArr[i].setBounds(row);
            controlZone.removeFromTop(4);
        };
        placeRhythm(stepsLbl, stepsSld);
        placeRhythm(pulsesLbl, pulsesSld);
        placeRhythm(offsetLbl, offsetSld);"""

c = c.replace(old, new)

with open("Source/PluginEditor.cpp", "w") as f:
    f.write(c)

print("done")
