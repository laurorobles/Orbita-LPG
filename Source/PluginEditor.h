#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class OrbitaLookAndFeel : public juce::LookAndFeel_V4 {
public:
    OrbitaLookAndFeel() {
        setColour(juce::Slider::thumbColourId, juce::Colour(65, 229, 155));
        setColour(juce::Slider::trackColourId, juce::Colours::cyan);
        setColour(juce::Slider::backgroundColourId, juce::Colour(24, 30, 38));
        setColour(juce::Slider::textBoxTextColourId, juce::Colour(220, 230, 240));
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(42, 52, 64));
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(14, 18, 23));
        setColour(juce::TextButton::buttonColourId, juce::Colour(28, 34, 42));
        setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(24, 30, 38));
        setColour(juce::ComboBox::textColourId, juce::Colour(225, 235, 245));
        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(20, 25, 32));
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(38, 58, 72));
    }

    juce::Font getLabelFont(juce::Label&) override { return juce::FontOptions(10.0f, juce::Font::bold); }
    juce::Font getTextButtonFont(juce::TextButton&, int) override { return juce::FontOptions(10.5f, juce::Font::bold); }
    juce::Font getComboBoxFont(juce::ComboBox&) override { return juce::FontOptions(11.0f, juce::Font::bold); }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                              bool isHighlighted, bool isDown) override {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
        auto base = backgroundColour.withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.45f);
        if (isDown) base = base.brighter(0.18f);
        else if (isHighlighted) base = base.brighter(0.1f);
        g.setColour(base);
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(isHighlighted ? juce::Colours::cyan.withAlpha(0.55f) : juce::Colour(58, 70, 84));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float, float, const juce::Slider::SliderStyle style, juce::Slider& slider) override {
        if (style == juce::Slider::LinearBar) {
            auto r = juce::Rectangle<float>((float)x, (float)y, (float)width, (float)height).reduced(0.5f);
            g.setColour(juce::Colour(16, 20, 26));
            g.fillRoundedRectangle(r, 3.0f);
            float fillW = juce::jlimit(0.0f, r.getWidth(), sliderPos - r.getX());
            g.setColour(slider.findColour(juce::Slider::trackColourId).withAlpha(0.45f));
            g.fillRoundedRectangle(r.withWidth(fillW), 3.0f);
            g.setColour(juce::Colour(52, 64, 78));
            g.drawRoundedRectangle(r, 3.0f, 1.0f);
            return;
        }

        auto accent = slider.findColour(juce::Slider::trackColourId);
        if (style == juce::Slider::LinearVertical) {
            float cx = (float)x + (float)width * 0.5f;
            float trackW = 4.0f;
            g.setColour(juce::Colour(24, 30, 38));
            g.fillRoundedRectangle(cx - trackW * 0.5f, (float)y + 2.0f, trackW, (float)height - 4.0f, 2.0f);
            float bottom = (float)(y + height) - 2.0f;
            float fillH = std::max(0.0f, bottom - sliderPos);
            g.setColour(accent.withAlpha(0.85f));
            g.fillRoundedRectangle(cx - trackW * 0.5f, sliderPos, trackW, fillH, 2.0f);
            float thumbW = 14.0f, thumbH = 7.0f;
            g.setColour(accent.withAlpha(0.28f));
            g.fillRoundedRectangle(cx - thumbW * 0.5f - 2.0f, sliderPos - thumbH * 0.5f - 2.0f, thumbW + 4.0f, thumbH + 4.0f, 3.0f);
            g.setColour(accent.brighter(0.25f));
            g.fillRoundedRectangle(cx - thumbW * 0.5f, sliderPos - thumbH * 0.5f, thumbW, thumbH, 2.5f);
            g.setColour(juce::Colours::black.withAlpha(0.65f));
            g.drawRoundedRectangle(cx - thumbW * 0.5f, sliderPos - thumbH * 0.5f, thumbW, thumbH, 2.5f, 1.0f);
        } else {
            float cy = (float)y + (float)height * 0.5f;
            float trackH = 4.0f;
            g.setColour(juce::Colour(24, 30, 38));
            g.fillRoundedRectangle((float)x + 2.0f, cy - trackH * 0.5f, (float)width - 4.0f, trackH, 2.0f);
            float fillW = std::max(0.0f, sliderPos - ((float)x + 2.0f));
            g.setColour(accent.withAlpha(0.85f));
            g.fillRoundedRectangle((float)x + 2.0f, cy - trackH * 0.5f, fillW, trackH, 2.0f);
            float r = 5.5f;
            g.setColour(accent.withAlpha(0.25f));
            g.fillEllipse(sliderPos - r - 2.0f, cy - r - 2.0f, (r + 2.0f) * 2.0f, (r + 2.0f) * 2.0f);
            g.setColour(accent.brighter(0.25f));
            g.fillEllipse(sliderPos - r, cy - r, r * 2.0f, r * 2.0f);
        }
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
                          const float rotStart, const float rotEnd, juce::Slider& slider) override {
        auto radius = (float)juce::jmin(width, height) * 0.5f - 4.0f;
        auto cx = (float)x + (float)width * 0.5f;
        auto cy = (float)y + (float)height * 0.5f;
        auto angle = rotStart + sliderPos * (rotEnd - rotStart);
        auto accent = slider.findColour(juce::Slider::thumbColourId);

        juce::Path bgArc;
        bgArc.addCentredArc(cx, cy, radius + 1.5f, radius + 1.5f, 0.0f, rotStart, rotEnd, true);
        g.setColour(juce::Colour(34, 44, 54));
        g.strokePath(bgArc, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path valArc;
        valArc.addCentredArc(cx, cy, radius + 1.5f, radius + 1.5f, 0.0f, rotStart, angle, true);
        g.setColour(accent);
        g.strokePath(valArc, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        float bodyR = radius - 2.5f;
        g.setColour(juce::Colour(24, 30, 38));
        g.fillEllipse(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);
        g.setColour(juce::Colour(58, 72, 86));
        g.drawEllipse(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f, 1.0f);

        juce::Path p;
        p.addRoundedRectangle(-1.2f, -bodyR + 2.0f, 2.4f, bodyR * 0.58f, 1.0f);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(cx, cy));
        g.setColour(accent.brighter(0.3f));
        g.fillPath(p);
    }

    void drawComboBox(juce::Graphics& g, int, int, bool, int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override {
        auto bounds = box.getLocalBounds().toFloat().reduced(0.5f);
        g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
        g.fillRoundedRectangle(bounds, 4.0f);
        g.setColour(juce::Colour(52, 64, 78));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
        float ax = (float)buttonX + (float)buttonW * 0.5f;
        float ay = (float)buttonY + (float)buttonH * 0.5f;
        juce::Path path;
        path.startNewSubPath(ax - 3.2f, ay - 1.5f);
        path.lineTo(ax + 3.2f, ay - 1.5f);
        path.lineTo(ax, ay + 2.5f);
        path.closeSubPath();
        g.setColour(juce::Colours::cyan.withAlpha(0.8f));
        g.fillPath(path);
    }
};

class AboutOverlayComponent : public juce::Component {
public:
    std::function<void()> onClose;
    juce::TextButton closeBtn{"CLOSE"};
    juce::HyperlinkButton githubLink{"github.com/laurorobles/Orbita-LPG", juce::URL("https://github.com/laurorobles/Orbita-LPG")};

    AboutOverlayComponent() {
        addAndMakeVisible(closeBtn);
        closeBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(36, 48, 60));
        closeBtn.onClick = [this]() { if (onClose) onClose(); };
        addAndMakeVisible(githubLink);
        githubLink.setColour(juce::HyperlinkButton::textColourId, juce::Colours::cyan);
        githubLink.setFont(juce::FontOptions(12.0f, juce::Font::bold), false);
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour(0xdc0a0e14));
        auto card = getLocalBounds().withSizeKeepingCentre(520, 290).toFloat();
        g.setColour(juce::Colour(20, 25, 32));
        g.fillRoundedRectangle(card, 10.0f);
        g.setColour(juce::Colours::cyan.withAlpha(0.75f));
        g.drawRoundedRectangle(card, 10.0f, 1.5f);

        auto r = card.toNearestInt().reduced(22, 18);
        g.setColour(juce::Colours::cyan);
        g.setFont(juce::FontOptions(16.0f, juce::Font::bold));
        g.drawText("ORBITA-LPG  v2.0.0", r.removeFromTop(24), juce::Justification::centredLeft);

        g.setColour(juce::Colour(65, 229, 155));
        g.setFont(juce::FontOptions(11.5f, juce::Font::bold));
        g.drawText("Free & Open Source  |  GNU GPLv3 License  |  West Coast Polymetric Synth", r.removeFromTop(20), juce::Justification::centredLeft);

        r.removeFromTop(10);
        g.setColour(juce::Colour(200, 212, 224));
        g.setFont(juce::FontOptions(11.5f));
        g.drawFittedText(
            "SHORTCUTS:\n"
            "  Space: Play / Pause   |   1-6: Select Track   |   M: Mute Track\n"
            "  Enter: Strike LPG   |   Click Radar: Select Ring   |   Right-Click T1-T6: Mute\n\n"
            "MIDI ROUTING:\n"
            "  MIDI In Ch 1 (C1-F1): Drum Trigger   |   Ch 1-6: Chromatic Multi-Timbral\n"
            "  MIDI Out: Generative Polymetric Sequencer (Ch 1-6)\n\n"
            "Created by Lauro Robles / Extasis Records (laurorobles@gmail.com)",
            r.removeFromTop(155), juce::Justification::topLeft, 10);
    }

    void resized() override {
        auto card = getLocalBounds().withSizeKeepingCentre(520, 290).reduced(22, 16);
        auto bottom = card.removeFromBottom(28);
        closeBtn.setBounds(bottom.removeFromRight(80));
        githubLink.setBounds(bottom.removeFromLeft(280));
    }
};

struct UIControl { juce::Slider slider; juce::Label label; };

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
    void mouseDown(const juce::MouseEvent& e) override;
    bool keyPressed(const juce::KeyPress& key) override;
    void selectTrack(int t);
    void toggleMute(int t);

private:
    OrbitaLPGAudioProcessor& audioProcessor;
    OrbitaLookAndFeel customLookAndFeel;
    juce::TooltipWindow tooltipWindow{this, 700};
    int currentTrack = 0;

    juce::Label titleLabel, creditLabel;
    juce::ComboBox kitCombo;
    juce::TextButton playBtn{"PLAY"}, stopBtn{"STOP"}, seqBtn{"SEQ: ON"}, aboutBtn{"ABOUT"};
    juce::TextButton loadBtn{"LOAD"}, saveBtn{"SAVE"};
    std::unique_ptr<juce::FileChooser> chooser;
    AboutOverlayComponent aboutOverlay;

    juce::Slider mVolSld, mDriveSld, mBpmSld, mSwingSld, mChaosSld;
    juce::Label mVolLbl, mDriveLbl, mBpmLbl, mSwingLbl, mChaosLbl, mScaleLbl, mRootLbl;
    juce::ComboBox globalScaleCombo, globalRootCombo;

    TrackButton tBtns[6];
    juce::ComboBox patternsCombo, rateCombo[6];
    juce::Label rateLbl;
    juce::TextButton rMutateBtn{"MUTATE"};
    juce::Slider stepsSld[6], pulsesSld[6], offsetSld[6];
    juce::Label stepsLbl, pulsesLbl, offsetLbl;

    juce::ComboBox synthPresetCombo[6];
    juce::ComboBox mode281Combo[6], mode292Combo[6];
    juce::Label mode281Lbl, mode292Lbl;
    juce::TextButton sRandBtn{"RAND"}, strikeBtn{"STRIKE"}, copyLastBtn{"< COPY"}, copyNextBtn{"COPY >"};
    juce::TextButton noteBtns[6];
    UIControl vSliders[6][12];

    juce::TextButton echoSyncBtn{"SYNC: OFF"};
    UIControl echoKnobs[4];

    juce::Rectangle<int> topArea, radarArea, radarPureArea, masterArea, meterArea, synthArea, echoArea;
    juce::Rectangle<int> synthSliderGridArea;
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
