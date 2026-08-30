#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class OrbitaLookAndFeel : public juce::LookAndFeel_V4 {
public:
    juce::Font getLabelFont(juce::Label& label) override { return juce::FontOptions(10.5f, juce::Font::bold); }
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
        if (!shouldDrawButtonAsDown) { g.setColour(juce::Colours::black); g.fillRect(bounds.translated(3.0f, 3.0f)); }
        g.setColour(baseColour); g.fillRect(shouldDrawButtonAsDown ? bounds.translated(1.0f, 1.0f) : bounds);
        g.setColour(juce::Colours::black); g.drawRect(shouldDrawButtonAsDown ? bounds.translated(1.0f, 1.0f) : bounds, 1.5f);
    }
    
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotStart, const float rotEnd, juce::Slider& slider) override {
        auto radius = (float) juce::jmin(width / 2, height / 2) - 4.0f;
        auto centreX = (float) x + (float) width  * 0.5f; auto centreY = (float) y + (float) height * 0.5f;
        auto rx = centreX - radius; auto ry = centreY - radius; auto rw = radius * 2.0f;
        auto angle = rotStart + sliderPos * (rotEnd - rotStart);
        g.setColour(juce::Colours::black); g.fillEllipse(rx + 3.0f, ry + 3.0f, rw, rw);
        g.setColour(slider.findColour(juce::Slider::thumbColourId)); g.fillEllipse(rx, ry, rw, rw);
        g.setColour(juce::Colours::black); g.drawEllipse(rx, ry, rw, rw, 1.5f);
        juce::Path p; p.addRectangle(-1.0f, -radius, 2.0f, radius * 0.7f);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        g.setColour(juce::Colours::black); g.fillPath(p);
    }

    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override
    {
        auto cornerSize = 3.0f;
        g.setColour(box.findColour (juce::ComboBox::backgroundColourId));
        g.fillRoundedRectangle (box.getLocalBounds().toFloat(), cornerSize);
        g.setColour(juce::Colour(50, 60, 70));
        g.drawRoundedRectangle (box.getLocalBounds().toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);
        juce::Rectangle<int> arrowZone (buttonX, buttonY, buttonW, buttonH);
        juce::Path path;
        float ax = (float)arrowZone.getCentreX(); float ay = (float)arrowZone.getCentreY();
        path.startNewSubPath (ax - 3.0f, ay - 1.5f); path.lineTo (ax + 3.0f, ay - 1.5f);
        path.lineTo (ax, ay + 2.0f); path.closeSubPath();
        g.setColour (box.findColour (juce::ComboBox::arrowColourId).isOpaque() ? box.findColour (juce::ComboBox::arrowColourId) : juce::Colours::cyan.withAlpha(0.7f));
        g.fillPath (path);
    }
};

struct UIControl { juce::Slider slider; juce::Label label; };
class TrackButton : public juce::TextButton {
public:
    std::function<void()> onRightClick;
    void mouseDown(const juce::MouseEvent& e) override {
        if (e.mods.isRightButtonDown() || e.mods.isCtrlDown()) { if (onRightClick) onRightClick(); } else juce::TextButton::mouseDown(e);
    }
};

class OrbitaLPGAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    OrbitaLPGAudioProcessorEditor(OrbitaLPGAudioProcessor&);
    ~OrbitaLPGAudioProcessorEditor() override;
    void timerCallback() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress& key) override;
    void selectTrack(int t);
    void toggleMute(int t);

private:
    OrbitaLPGAudioProcessor& audioProcessor;
    OrbitaLookAndFeel customLookAndFeel;
    juce::TooltipWindow tooltipWindow{ this, 700 };
    int currentTrack = 0; bool trackMutes[6] = {false};

    juce::Label titleLabel;
    juce::ComboBox kitCombo;
    juce::TextButton playBtn{"PLAY"}, stopBtn{"STOP"}, seqBtn{"SEQ: ON"}, configBtn{"CONFIG"}; 
    juce::Slider mVolSld, mDriveSld, mBpmSld, mSwingSld, mChaosSld;
    juce::Label mVolLbl, mDriveLbl, mBpmLbl, mSwingLbl, mChaosLbl, mScaleLbl, mRootLbl;
    juce::ComboBox globalScaleCombo, globalRootCombo;

    TrackButton tBtns[6];
    juce::ComboBox patternsCombo, rateCombo[6];
    juce::Label rateLbl;
    juce::TextButton rMutateBtn{"MUTATE"};
    juce::Slider stepsSld[6], pulsesSld[6], offsetSld[6];
    juce::Label stepsLbl, pulsesLbl, offsetLbl;

    // NUEVO: Un menú de presets para CADA track
    juce::ComboBox synthPresetCombo[6];
    juce::ComboBox mode281Combo[6], mode292Combo[6];
    juce::Label mode281Lbl, mode292Lbl;
    juce::TextButton sRandBtn{"RAND"}, strikeBtn{"STRIKE"}, copyLastBtn{"< COPY"}, copyNextBtn{"COPY >"};
    juce::TextButton noteBtns[6]; 
    UIControl vSliders[6][12];

    juce::TextButton echoSyncBtn{"SYNC: OFF"};
    UIControl echoKnobs[6];
    juce::Rectangle<int> topArea, radarArea, masterArea, rhythmArea, synthArea, echoArea;
    juce::Image logoImage;
    
    using SldAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CmbAtt = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using BtnAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;
    
    std::unique_ptr<SldAtt> mAtt[5], eAtt[4];
    std::unique_ptr<CmbAtt> gScaleAtt, gRootAtt;
    std::unique_ptr<BtnAtt> eSyncAtt;
    std::unique_ptr<SldAtt> rAtt[6][3], sAtt[6][12]; 
    std::unique_ptr<CmbAtt> rRateAtt[6];
    std::unique_ptr<BtnAtt> nAtt[6]; 
    std::unique_ptr<CmbAtt> m281Att[6], m292Att[6];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrbitaLPGAudioProcessorEditor)
};