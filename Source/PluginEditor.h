#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class OrbitaLookAndFeel : public juce::LookAndFeel_V4 {
public:
    OrbitaLookAndFeel() { 
        setColour(juce::Slider::thumbColourId, juce::Colours::cyan); 
        setColour(juce::TextButton::buttonColourId, juce::Colour(30, 35, 40));
        setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(30, 35, 40));
    }
    
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, const float rotStart, const float rotEnd, juce::Slider&) override {
        auto radius = (float) juce::jmin(width / 2, height / 2) - 2.0f;
        auto centreX = (float) x + (float) width  * 0.5f;
        auto centreY = (float) y + (float) height * 0.5f;
        g.setColour(juce::Colour(22, 26, 30));
        g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);
        g.setColour(juce::Colour(45, 55, 65));
        g.drawEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 1.5f);
        auto angle = rotStart + sliderPos * (rotEnd - rotStart);
        juce::Path p; p.addRoundedRectangle(-radius * 0.12f, -radius * 0.75f, radius * 0.24f, radius * 0.75f, 1.0f);
        g.setColour(juce::Colour(65, 229, 155));
        g.fillPath(p, juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    }
    
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float minPos, float maxPos, const juce::Slider::SliderStyle style, juce::Slider& s) override {
        if (style == juce::Slider::LinearVertical) {
            auto centreX = (float) x + (float) width * 0.5f;
            g.setColour(juce::Colour(20, 25, 30));
            g.fillRoundedRectangle(centreX - 2.0f, (float)y, 4.0f, (float)height, 2.0f);
            g.setColour(juce::Colour(200, 210, 220));
            g.drawEllipse(centreX - 6.0f, sliderPos - 6.0f, 12.0f, 12.0f, 2.0f);
        } else if (style == juce::Slider::LinearHorizontal) {
            auto centreY = (float) y + (float) height * 0.5f;
            g.setColour(juce::Colour(20, 25, 30));
            g.fillRoundedRectangle((float)x, centreY - 2.0f, (float)width, 4.0f, 2.0f);
            g.setColour(juce::Colour(200, 210, 220));
            g.drawEllipse(sliderPos - 6.0f, centreY - 6.0f, 12.0f, 12.0f, 2.0f);
        }
    }
    
    void drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour& bg, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
        auto bounds = b.getLocalBounds().toFloat().reduced(1.0f);
        g.setColour(bg);
        g.fillRoundedRectangle(bounds, 3.0f);
        g.setColour(b.findColour(juce::TextButton::textColourOffId).withAlpha(0.3f));
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
    }
};

struct UIControl {
    juce::Slider slider;
    juce::Label label;
};

class TrackButton : public juce::TextButton {
public:
    std::function<void()> onRightClick;
    void mouseDown(const juce::MouseEvent& e) override {
        if (e.mods.isRightButtonDown() || e.mods.isCtrlDown()) {
            if (onRightClick) onRightClick();
        } else {
            juce::TextButton::mouseDown(e);
        }
    }
};

class OrbitaLPGAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    OrbitaLPGAudioProcessorEditor(OrbitaLPGAudioProcessor&);
    ~OrbitaLPGAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    bool keyPressed(const juce::KeyPress& key) override;

    void selectTrack(int t);
    void toggleMute(int t);

private:
    OrbitaLPGAudioProcessor& audioProcessor;
    OrbitaLookAndFeel customLookAndFeel;
    
    int currentTrack = 0;
    bool trackMutes[6] = {false};

    // Header
    juce::Label titleLabel;
    juce::TextButton playBtn{"PLAY"}, seqBtn{"SEQ: ON"}, polyBtn{"POLY"};
    juce::ComboBox presetCombo;
    juce::TextButton configBtn{"CONFIG"};

    // Master System
    juce::Slider mVolSld, mDriveSld, mBpmSld, mSwingSld, mChaosSld;
    juce::Label mVolLbl, mDriveLbl, mBpmLbl, mSwingLbl, mChaosLbl, mScaleLbl;
    juce::ComboBox globalScaleCombo;

    // Track Rhythm Matrix
    TrackButton tBtns[6];
    juce::ComboBox patternsCombo;
    juce::TextButton rRandBtn{"RAND"};
    juce::Slider stepsSld[6], pulsesSld[6], offsetSld[6];
    juce::Label stepsLbl, pulsesLbl, offsetLbl;

    // West Coast Synthesis
    juce::ComboBox trackScaleCombo;
    juce::TextButton mode281[3], mode292[3];
    juce::TextButton sRandBtn{"RAND"}, copyLastBtn{"COPY TO LAST"}, copyNextBtn{"COPY TO NEXT"}, noteBtn{"NOTE"};
    juce::Label mode281Lbl, mode292Lbl;
    UIControl vSliders[6][10]; 

    // Space Echo
    juce::TextButton echoSyncBtn{"SYNC: OFF"};
    UIControl echoKnobs[6];

    // Layout areas
    juce::Rectangle<int> topArea, radarArea, masterArea, rhythmArea, synthArea, echoArea;

    // Attachments
    using SldAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CmbAtt = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    
    std::unique_ptr<SldAtt> mAtt[5];
    std::unique_ptr<CmbAtt> gScaleAtt;
    std::unique_ptr<SldAtt> eAtt[6];
    
    std::unique_ptr<SldAtt> rAtt[6][3];
    std::unique_ptr<SldAtt> sAtt[6][10];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrbitaLPGAudioProcessorEditor)
};
