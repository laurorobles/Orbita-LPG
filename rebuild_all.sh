cat << 'PYEOF' > patch_dsp_2.py
import re
with open("Source/PluginProcessor.h", "r") as f: h = f.read()
if "juce::dsp::Oversampling<float> oversampler" not in h:
    h = h.replace("juce::dsp::DelayLine<float> delayL{48000 * 2};\n    juce::dsp::DelayLine<float> delayR{48000 * 2};", "juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayL{48000 * 2};\n    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayR{48000 * 2};\n    juce::dsp::Oversampling<float> oversampler;")
    h = h.replace("float lpg_state = 0.0f;", "public:\n    float lpg_state = 0.0f;\nprivate:")
with open("Source/PluginProcessor.h", "w") as f: f.write(h)

with open("Source/PluginProcessor.cpp", "r") as f: c = f.read()
if "oversampler(2, 1" not in c:
    c = c.replace("apvts(*this, nullptr, \"PARAMETERS\", createParameterLayout())", "apvts(*this, nullptr, \"PARAMETERS\", createParameterLayout()),\n       oversampler(2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true)")

c = c.replace("""    for (int i = 0; i < 6; i++) {
        voices[i].prepare(sampleRate);
        voices[i].current_step = -1;
    }
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;""", """    oversampler.initProcessing(samplesPerBlock);
    
    for (int i = 0; i < 6; i++) {
        voices[i].prepare(sampleRate * oversampler.getOversamplingFactor());
        voices[i].current_step = -1;
    }
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;""")

old_process = """        float outL = 0.0f, outR = 0.0f;
        for (int t=0; t<6; ++t) {
            if (!trackMutes[t]) voices[t].process(outL, outR, tParams[t], chaos, g_scale, quantize[t].load());
        }"""
new_process = """        // --- OVERSAMPLING BLOCK ---
        juce::AudioBuffer<float> dummy (2, 1);
        dummy.clear();
        juce::dsp::AudioBlock<float> dummyBlock (dummy);
        juce::dsp::AudioBlock<float> upsampledBlock = oversampler.processSamplesUp(dummyBlock);
        
        int factor = (int)oversampler.getOversamplingFactor();
        for (int sub_sample = 0; sub_sample < factor; ++sub_sample) {
            float subL = 0.0f, subR = 0.0f;
            for (int t=0; t<6; ++t) {
                if (!trackMutes[t]) voices[t].process(subL, subR, tParams[t], chaos, g_scale, quantize[t].load());
            }
            upsampledBlock.setSample(0, sub_sample, subL);
            upsampledBlock.setSample(1, sub_sample, subR);
        }
        
        oversampler.processSamplesDown(dummyBlock);
        juce::dsp::AudioBlock<float> outBlock = dummyBlock;
        float outL = outBlock.getSample(0, 0);
        float outR = outBlock.getSample(1, 0);
        // --------------------------"""
c = c.replace(old_process, new_process)
with open("Source/PluginProcessor.cpp", "w") as f: f.write(c)
PYEOF

cat << 'PYEOF' > fix_h.py
import re
with open("Source/PluginEditor.h", "r") as f: h2 = f.read()
new_laf = """class OrbitaLookAndFeel : public juce::LookAndFeel_V4 {
public:
    juce::Font getLabelFont(juce::Label& label) override {
        return juce::FontOptions(10.5f, juce::Font::bold);
    }
    OrbitaLookAndFeel() {
        setColour(juce::Slider::thumbColourId, juce::Colours::cyan); 
        setColour(juce::TextButton::buttonColourId, juce::Colour(30, 35, 40));
        setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(30, 35, 40));
    }
    
    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, 
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);
        auto baseColour = backgroundColour.withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);
        if (!shouldDrawButtonAsDown) {
            g.setColour(juce::Colours::black);
            g.fillRect(bounds.translated(3.0f, 3.0f));
        }
        g.setColour(baseColour);
        g.fillRect(shouldDrawButtonAsDown ? bounds.translated(1.0f, 1.0f) : bounds);
        g.setColour(juce::Colours::black);
        g.drawRect(shouldDrawButtonAsDown ? bounds.translated(1.0f, 1.0f) : bounds, 1.5f);
    }
    
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotStart, const float rotEnd, juce::Slider& slider) override {
        auto radius = (float) juce::jmin(width / 2, height / 2) - 4.0f;
        auto centreX = (float) x + (float) width  * 0.5f;
        auto centreY = (float) y + (float) height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotStart + sliderPos * (rotEnd - rotStart);
        
        g.setColour(juce::Colours::black);
        g.fillEllipse(rx + 3.0f, ry + 3.0f, rw, rw);
        
        juce::Colour thumbCol = slider.findColour(juce::Slider::thumbColourId);
        g.setColour(thumbCol);
        g.fillEllipse(rx, ry, rw, rw);
        
        g.setColour(juce::Colours::black);
        g.drawEllipse(rx, ry, rw, rw, 1.5f);
        
        juce::Path p;
        auto pointerLength = radius * 0.7f;
        auto pointerThickness = 2.0f;
        p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.setColour(juce::Colours::black);
        g.fillPath(p);
    }
};"""
h2 = re.sub(r'class OrbitaLookAndFeel : public juce::LookAndFeel_V4 \{.*?\n\};\n', new_laf + "\n", h2, flags=re.DOTALL)
if "bool showHz[6]" not in h2: h2 = h2.replace("bool trackMutes[6] = {false};", "bool trackMutes[6] = {false};\n    bool showHz[6] = {false};")
if "public juce::Timer" not in h2: h2 = h2.replace("class OrbitaLPGAudioProcessorEditor  : public juce::AudioProcessorEditor", "class OrbitaLPGAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer")
if "void setParam" not in h2: h2 = h2.replace("void toggleMute(int t);", "void toggleMute(int t);\n    void timerCallback() override;\n    void setParam(juce::String id, float val);")
if "rResetBtn" not in h2:
    h2 = h2.replace('juce::TextButton sRandBtn{"RAND"}, copyLastBtn', 'juce::TextButton sRandBtn{"RAND"}, sResetBtn{"RESET"}, copyLastBtn')
    h2 = h2.replace('juce::TextButton rRandBtn{"RAND"};', 'juce::TextButton rRandBtn{"RAND"};\n    juce::TextButton rResetBtn{"RESET"};')
with open("Source/PluginEditor.h", "w") as f: f.write(h2)
PYEOF

cat << 'PYEOF' > patch_ui.py
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
PYEOF

python3 patch_dsp_2.py
python3 fix_h.py
python3 patch_ui.py
cmake --build build --config Release
