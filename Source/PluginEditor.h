#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class OrbitaLookAndFeel : public juce::LookAndFeel_V4 {
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
    void timerCallback() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    
    bool keyPressed(const juce::KeyPress& key) override;
    void selectTrack(int t);
    void toggleMute(int t);
    void setParam(juce::String id, float val);

private:
    OrbitaLPGAudioProcessor& audioProcessor;
    OrbitaLookAndFeel customLookAndFeel;
    
    int currentTrack = 0;
    bool trackMutes[6] = {false};

    juce::Label titleLabel;
    juce::TextButton playBtn{"PLAY"}, seqBtn{"SEQ: ON"}, polyBtn{"POLY"};
    juce::ComboBox presetCombo;
    juce::TextButton configBtn{"CONFIG"};

    juce::Slider mVolSld, mDriveSld, mBpmSld, mSwingSld, mChaosSld;
    juce::Label mVolLbl, mDriveLbl, mBpmLbl, mSwingLbl, mChaosLbl, mScaleLbl;
    juce::ComboBox globalScaleCombo;

    TrackButton tBtns[6];
    juce::ComboBox patternsCombo;
    juce::TextButton rRandBtn{"RAND"}, rResetBtn{"RESET"};
    juce::Slider stepsSld[6], pulsesSld[6], offsetSld[6];
    juce::Label stepsLbl, pulsesLbl, offsetLbl;

    juce::TextButton mode281[3], mode292[3];
    juce::TextButton sRandBtn{"RAND"}, sResetBtn{"RESET"}, copyLastBtn{"COPY TO LAST"}, copyNextBtn{"COPY TO NEXT"};
    juce::TextButton noteBtns[6]; 
    juce::Label mode281Lbl, mode292Lbl;
    UIControl vSliders[6][10]; 

    juce::TextButton echoSyncBtn{"SYNC: OFF"};
    UIControl echoKnobs[6];

    juce::Rectangle<int> topArea, radarArea, masterArea, rhythmArea, synthArea, echoArea;

    using SldAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CmbAtt = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using BtnAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;
    
    std::unique_ptr<SldAtt> mAtt[5];
    std::unique_ptr<CmbAtt> gScaleAtt;
    std::unique_ptr<SldAtt> eAtt[6];
    
    std::unique_ptr<SldAtt> rAtt[6][3];
    std::unique_ptr<SldAtt> sAtt[6][10];
    std::unique_ptr<BtnAtt> nAtt[6]; 

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrbitaLPGAudioProcessorEditor)
};