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
