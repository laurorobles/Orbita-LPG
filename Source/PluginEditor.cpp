#include "PluginProcessor.h"
#include "PluginEditor.h"

// =========================================================
// Bjorklund helper (editor)
// =========================================================
static std::vector<int> gen_euclid(int pulses, int steps, int offset) {
    std::vector<int> pat(steps, 0);
    if (steps == 0) return pat;
    if (pulses >= steps) { std::fill(pat.begin(), pat.end(), 1); return pat; }
    int bucket = 0;
    for (int i = 0; i < steps; ++i) {
        bucket += pulses;
        if (bucket >= steps) { bucket -= steps; pat[(i + offset) % steps] = 1; }
    }
    return pat;
}

// =========================================================
// Constructor
// =========================================================
OrbitaLPGAudioProcessorEditor::OrbitaLPGAudioProcessorEditor(OrbitaLPGAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&customLookAndFeel);
    setWantsKeyboardFocus(true);

    // ----- Label helper -----
    auto lbl = [this](juce::Label& l, const char* txt, juce::Colour col = juce::Colour(160,170,180)) {
        l.setText(txt, juce::dontSendNotification);
        l.setFont(juce::FontOptions(9.5f, juce::Font::bold));
        l.setColour(juce::Label::textColourId, col);
        l.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(l);
    };
    auto btn = [this](juce::TextButton& b, juce::Colour col = juce::Colour(30,35,40), juce::Colour textCol = juce::Colours::white) {
        b.setColour(juce::TextButton::buttonColourId, col);
        b.setColour(juce::TextButton::textColourOffId, textCol);
        addAndMakeVisible(b);
    };
    auto masterSld = [this](juce::Slider& s, juce::String suffix) {
        s.setSliderStyle(juce::Slider::LinearBar);
        s.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 18);
        s.setColour(juce::Slider::trackColourId, juce::Colour(38, 45, 52));
        s.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        s.setTextValueSuffix(suffix);
        addAndMakeVisible(s);
    };

    // Header
    titleLabel.setText("ORBITA-LPG   ///   6-VOICE MATRIX   coded by @laurorobles", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(200,210,220));
    addAndMakeVisible(titleLabel);

    btn(playBtn, juce::Colour(40,90,60), juce::Colours::lightgreen);
    btn(seqBtn,  juce::Colour(30,35,42), juce::Colours::cyan);
    btn(polyBtn, juce::Colour(30,35,42), juce::Colours::white);
    btn(configBtn, juce::Colour(25,30,38), juce::Colours::cyan);
    addAndMakeVisible(presetCombo);
    presetCombo.addItem("INIT", 1);
    presetCombo.setSelectedItemIndex(0);

    playBtn.onClick = [this]() {
        bool cur = audioProcessor.isPlaying;
        audioProcessor.isPlaying = !cur;
        playBtn.setButtonText(!cur ? "PAUSE" : "PLAY");
        playBtn.setColour(juce::TextButton::buttonColourId, !cur ? juce::Colour(100,30,30) : juce::Colour(40,90,60));
    };
    seqBtn.onClick = [this]() {
        audioProcessor.seqEnabled = !audioProcessor.seqEnabled;
        seqBtn.setButtonText(audioProcessor.seqEnabled ? "SEQ: ON" : "SEQ: OFF");
    };

    // Master
    lbl(mVolLbl,   "M.VOL",  juce::Colours::yellow);
    lbl(mDriveLbl, "DRIVE",  juce::Colour(230,140,30));
    lbl(mBpmLbl,   "BPM",    juce::Colour(160,170,180));
    lbl(mSwingLbl, "SWING",  juce::Colour(160,170,180));
    lbl(mChaosLbl, "CHAOS",  juce::Colour(160,170,180));
    lbl(mScaleLbl, "SCALE",  juce::Colours::cyan);

    masterSld(mVolSld,   "");
    masterSld(mDriveSld, "");
    masterSld(mBpmSld,   "");
    masterSld(mSwingSld, "");
    masterSld(mChaosSld, "");

    addAndMakeVisible(globalScaleCombo);
    for (auto* s : {"Chromatic","Major","Minor","Dorian","Phrygian","Lydian","Mixolydian","Pent. Maj","Pent. Min","Harm. Min"})
        globalScaleCombo.addItem(s, globalScaleCombo.getNumItems()+1);
    globalScaleCombo.setSelectedItemIndex(0);

    // APVTS attachments -- Master
    mAtt[0] = std::make_unique<SldAtt>(audioProcessor.apvts, "master_vol",   mVolSld);
    mAtt[1] = std::make_unique<SldAtt>(audioProcessor.apvts, "master_drive", mDriveSld);
    mAtt[2] = std::make_unique<SldAtt>(audioProcessor.apvts, "bpm",          mBpmSld);
    mAtt[3] = std::make_unique<SldAtt>(audioProcessor.apvts, "swing",        mSwingSld);
    mAtt[4] = std::make_unique<SldAtt>(audioProcessor.apvts, "chaos",        mChaosSld);
    gScaleAtt = std::make_unique<CmbAtt>(audioProcessor.apvts, "global_scale", globalScaleCombo);

    // Track buttons (inside radar module)
    for (int i = 0; i < 6; ++i) {
        tBtns[i].setButtonText("T" + juce::String(i+1));
        btn(tBtns[i], juce::Colour(22,28,35), juce::Colours::white);
        tBtns[i].onClick = [this,i]() { selectTrack(i); };
        tBtns[i].onRightClick = [this,i]() { toggleMute(i); };
    }

    // Patterns
    addAndMakeVisible(patternsCombo);
    patternsCombo.addItem("PATTERNS...", 1);
    patternsCombo.addItem("E(3,8) Tresillo", 2);
    patternsCombo.addItem("E(5,16) Bossa",   3);
    patternsCombo.addItem("E(7,16) Samba",   4);
    patternsCombo.addItem("E(4,16) Techno",  5);
    patternsCombo.setSelectedItemIndex(0);
    patternsCombo.onChange = [this]() {
        int id = patternsCombo.getSelectedId();
        if (id == 2) { stepsSld[currentTrack].setValue(8);  pulsesSld[currentTrack].setValue(3); offsetSld[currentTrack].setValue(0); }
        if (id == 3) { stepsSld[currentTrack].setValue(16); pulsesSld[currentTrack].setValue(5); offsetSld[currentTrack].setValue(0); }
        if (id == 4) { stepsSld[currentTrack].setValue(16); pulsesSld[currentTrack].setValue(7); offsetSld[currentTrack].setValue(0); }
        if (id == 5) { stepsSld[currentTrack].setValue(16); pulsesSld[currentTrack].setValue(4); offsetSld[currentTrack].setValue(0); }
        patternsCombo.setSelectedItemIndex(0, juce::dontSendNotification);
    };

    btn(rRandBtn, juce::Colour(40,20,50), juce::Colours::violet);

    // Rhythm sliders
    auto rhythmSld = [this](juce::Slider& s) {
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 28, 18);
        addAndMakeVisible(s);
    };
    lbl(stepsLbl,  "STEPS",  juce::Colour(160,170,180));
    lbl(pulsesLbl, "PULSES", juce::Colour(160,170,180));
    lbl(offsetLbl, "OFFSET", juce::Colour(160,170,180));
    for (int t = 0; t < 6; ++t) {
        rhythmSld(stepsSld[t]); rhythmSld(pulsesSld[t]); rhythmSld(offsetSld[t]);
    }

    // West Coast
    addAndMakeVisible(trackScaleCombo);
    for (auto* s : {"Chromatic","Major","Minor","Dorian","Phrygian","Lydian","Mixolydian","Pent. Maj","Pent. Min","Harm. Min"})
        trackScaleCombo.addItem(s, trackScaleCombo.getNumItems()+1);
    trackScaleCombo.setSelectedItemIndex(0);

    auto mode281Names = {"TRANS","SUST","CYCLE"};
    juce::Colour m281cols[] = {juce::Colour(60,160,80), juce::Colour(30,35,42), juce::Colour(30,35,42)};
    int mi = 0;
    for (auto* n : mode281Names) { mode281[mi].setButtonText(n); btn(mode281[mi], m281cols[mi], juce::Colours::white); mi++; }

    auto mode292Names = {"VCA","LPG","VCF"};
    juce::Colour m292cols[] = {juce::Colour(30,35,42), juce::Colour(20,60,80), juce::Colour(30,35,42)};
    int mi2 = 0;
    for (auto* n : mode292Names) { mode292[mi2].setButtonText(n); btn(mode292[mi2], m292cols[mi2], juce::Colours::white); mi2++; }

    lbl(mode281Lbl, "281:", juce::Colour(100,110,120));
    lbl(mode292Lbl, "292:", juce::Colour(100,110,120));

    btn(sRandBtn,   juce::Colour(30,35,42), juce::Colours::white);
    btn(copyLastBtn,juce::Colour(30,35,42), juce::Colours::white);
    btn(copyNextBtn,juce::Colour(30,35,42), juce::Colours::white);
    btn(noteBtn,    juce::Colour(20,50,70), juce::Colours::cyan);

    juce::String sNames[] = {"PITCH","P.DRP","MRPH","FOLD","FM","RISE","FALL","RESP","BRGT","VOL"};
    juce::Colour sCols[]  = {juce::Colours::white, juce::Colours::white, juce::Colours::white,
                              juce::Colours::white, juce::Colours::white, juce::Colour(80,200,100),
                              juce::Colour(80,200,100), juce::Colours::cyan, juce::Colours::cyan, juce::Colours::white};
    for (int t = 0; t < 6; ++t) {
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
    }

    // Echo
    btn(echoSyncBtn, juce::Colour(16,45,35), juce::Colour(65,229,155));
    juce::String eNames[] = {"TIME","FDBK","MIX","HPF","LPF","WOW"};
    juce::String eIds[]   = {"echo_time","echo_fdbk","echo_mix","echo_hpf","echo_lpf","echo_wow"};
    for (int i = 0; i < 6; ++i) {
        echoKnobs[i].slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        echoKnobs[i].slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 36, 13);
        addAndMakeVisible(echoKnobs[i].slider);
        lbl(echoKnobs[i].label, eNames[i].toRawUTF8(), juce::Colours::white);
        eAtt[i] = std::make_unique<SldAtt>(audioProcessor.apvts, eIds[i], echoKnobs[i].slider);
    }

    setSize(960, 500);
    startTimerHz(30);
    selectTrack(0);
}

OrbitaLPGAudioProcessorEditor::~OrbitaLPGAudioProcessorEditor() {
    setLookAndFeel(nullptr);
}

void OrbitaLPGAudioProcessorEditor::selectTrack(int t) {
    currentTrack = t;
    for (int i = 0; i < 6; ++i) {
        bool sel = (i == t);
        bool mut = trackMutes[i];
        tBtns[i].setColour(juce::TextButton::buttonColourId, sel ? juce::Colour(50,55,20) : juce::Colour(22,28,35));
        tBtns[i].setColour(juce::TextButton::textColourOffId, mut ? juce::Colours::darkgrey : (sel ? juce::Colours::yellow : juce::Colours::white));
    }
    for (int i = 0; i < 6; ++i) {
        bool show = (i == t);
        stepsSld[i].setVisible(show);
        pulsesSld[i].setVisible(show);
        offsetSld[i].setVisible(show);
        for (int j = 0; j < 10; ++j) {
            vSliders[i][j].slider.setVisible(show);
        }
    }
}

void OrbitaLPGAudioProcessorEditor::toggleMute(int t) {
    trackMutes[t] = !trackMutes[t];
    audioProcessor.trackMutes[t] = trackMutes[t];
    bool mut = trackMutes[t];
    bool sel = (t == currentTrack);
    tBtns[t].setColour(juce::TextButton::textColourOffId, mut ? juce::Colours::darkgrey : (sel ? juce::Colours::yellow : juce::Colours::white));
}

bool OrbitaLPGAudioProcessorEditor::keyPressed(const juce::KeyPress& key) {
    if (key.isKeyCode(juce::KeyPress::spaceKey)) {
        playBtn.triggerClick();
        return true;
    }
    return false;
}

void OrbitaLPGAudioProcessorEditor::timerCallback() { repaint(radarArea); }

// =========================================================
// resized() — Layout maestro con regla de 2px
// =========================================================
void OrbitaLPGAudioProcessorEditor::resized() {
    auto b = getLocalBounds().reduced(12);

    // Header
    topArea = b.removeFromTop(24);
    b.removeFromTop(8);

    // Columna izquierda: Radar + controles de pista abajo
    radarArea = b.removeFromLeft(420);
    b.removeFromLeft(12);
    auto rightCol = b;

    // Columna derecha: Master / Synth / Echo
    masterArea = rightCol.removeFromTop(70);
    rightCol.removeFromTop(8);
    synthArea  = rightCol.removeFromTop(290);
    rightCol.removeFromTop(8);
    echoArea   = rightCol;

    // ---- Header ----
    {
        auto r = topArea;
        titleLabel.setBounds(r.removeFromLeft(350));
        playBtn.setBounds(r.removeFromLeft(55).reduced(0,2));   r.removeFromLeft(5);
        seqBtn.setBounds( r.removeFromLeft(65).reduced(0,2));   r.removeFromLeft(5);
        polyBtn.setBounds(r.removeFromLeft(55).reduced(0,2));
        configBtn.setBounds(r.removeFromRight(75).reduced(0,2));r.removeFromRight(5);
        presetCombo.setBounds(r.removeFromRight(120).reduced(0,2));
    }

    // ---- Radar Module (left) — reserva zona inferior para controles ----
    {
        auto ra = radarArea;
        // Zona inferior fija de 110px para track controls
        auto controlZone = ra.removeFromBottom(110);
        // 'ra' es ahora el radar puro

        // Dentro del control zone: padding
        controlZone.reduce(8, 6);

        // Fila A: T1-T6  |  PATTERNS  |  RAND
        auto rowA = controlZone.removeFromTop(22);
        for (int i = 0; i < 6; ++i)
            tBtns[i].setBounds(rowA.removeFromLeft(30).reduced(1,0));
        rowA.removeFromLeft(10);
        patternsCombo.setBounds(rowA.removeFromLeft(120).reduced(0,1));
        rowA.removeFromLeft(5);
        rRandBtn.setBounds(rowA.removeFromLeft(55).reduced(0,1));

        controlZone.removeFromTop(6);

        // Filas B, C, D: STEPS / PULSES / OFFSET con regla 2px label
        auto placeRhythm = [&](juce::Label& l, juce::Slider* sArr) {
            auto row = controlZone.removeFromTop(20);
            int lw = 45;
            l.setBounds(row.removeFromLeft(lw));
            row.removeFromLeft(2); // REGLA 2PX
            for(int i=0; i<6; i++) sArr[i].setBounds(row);
            controlZone.removeFromTop(4);
        };
        placeRhythm(stepsLbl, stepsSld);
        placeRhythm(pulsesLbl, pulsesSld);
        placeRhythm(offsetLbl, offsetSld);
    }

    // ---- Master Module ----
    {
        auto mi = masterArea.reduced(10);
        mi.removeFromTop(14); // titulo
        auto row1 = mi.removeFromTop(12);
        mi.removeFromTop(2); // 2PX
        auto row2 = mi.removeFromTop(20);

        auto place = [&](juce::Label& l, juce::Slider& s, int w) {
            l.setBounds(row1.removeFromLeft(w));
            s.setBounds(row2.removeFromLeft(w).reduced(2,0));
            row1.removeFromLeft(8); row2.removeFromLeft(8);
        };
        place(mVolLbl, mVolSld, 50); place(mDriveLbl, mDriveSld, 50);
        place(mBpmLbl, mBpmSld, 50); place(mSwingLbl, mSwingSld, 50);
        place(mChaosLbl, mChaosSld, 50);
        row1.removeFromLeft(5); row2.removeFromLeft(5);
        mScaleLbl.setBounds(row1.removeFromLeft(100));
        globalScaleCombo.setBounds(row2.removeFromLeft(100).reduced(2,0));
    }

    // ---- Synth Module ----
    {
        auto si = synthArea.reduced(10);
        si.removeFromTop(14); // titulo

        // Fila 1: botones derecha
        auto sRow1 = si.removeFromTop(20);
        noteBtn.setBounds(    sRow1.removeFromRight(50).reduced(0,1)); sRow1.removeFromRight(4);
        copyNextBtn.setBounds(sRow1.removeFromRight(90).reduced(0,1)); sRow1.removeFromRight(4);
        copyLastBtn.setBounds(sRow1.removeFromRight(90).reduced(0,1)); sRow1.removeFromRight(4);
        sRandBtn.setBounds(   sRow1.removeFromRight(50).reduced(0,1));

        si.removeFromTop(5);

        // Fila 2: Track scale + mode buttons
        auto sRow2 = si.removeFromTop(20);
        trackScaleCombo.setBounds(sRow2.removeFromLeft(100).reduced(0,1)); sRow2.removeFromLeft(15);
        mode281Lbl.setBounds(sRow2.removeFromLeft(28));
        for (int i = 0; i < 3; ++i) mode281[i].setBounds(sRow2.removeFromLeft(42).reduced(0,1));
        sRow2.removeFromLeft(15);
        mode292Lbl.setBounds(sRow2.removeFromLeft(28));
        for (int i = 0; i < 3; ++i) mode292[i].setBounds(sRow2.removeFromLeft(33).reduced(0,1));

        si.removeFromTop(10);

        // Fila 3: Faders verticales con regla 2px (label encima, 2px, fader)
        int sw = si.getWidth() / 10;
        for (int i = 0; i < 10; ++i) {
            auto cell = si.removeFromLeft(sw);
            vSliders[0][i].label.setBounds(cell.removeFromTop(12));
            cell.removeFromTop(2); // REGLA 2PX
            for (int t = 0; t < 6; ++t) {
                vSliders[t][i].slider.setBounds(cell);
            }
        }
    }

    // ---- Echo Module ----
    {
        auto ei = echoArea.reduced(10);
        ei.removeFromTop(14); // titulo
        echoSyncBtn.setBounds(ei.removeFromLeft(80).withSizeKeepingCentre(70, 20));
        ei.removeFromLeft(5);
        int kw = ei.getWidth() / 6;
        for (int i = 0; i < 6; ++i) {
            auto cell = ei.removeFromLeft(kw);
            auto kb = cell.withSizeKeepingCentre(38, 38);
            echoKnobs[i].slider.setBounds(kb);
            // Label 2px ENCIMA del knob
            echoKnobs[i].label.setBounds(kb.getX()-8, kb.getY()-14, kb.getWidth()+16, 12);
        }
    }
}

// =========================================================
// paint()
// =========================================================
void OrbitaLPGAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(13, 16, 20));

    // Draw module backgrounds
    auto drawModule = [&](juce::Rectangle<int> r, juce::Colour bg, juce::Colour border) {
        g.setColour(bg);
        g.fillRoundedRectangle(r.toFloat(), 6.0f);
        g.setColour(border);
        g.drawRoundedRectangle(r.toFloat(), 6.0f, 1.5f);
    };

    drawModule(radarArea,  juce::Colour(11,14,18), juce::Colour(40,50,60));
    drawModule(masterArea, juce::Colour(20,24,28), juce::Colour(50,60,70));
    drawModule(synthArea,  juce::Colour(18,22,26), juce::Colour(50,55,65));
    drawModule(echoArea,   juce::Colour(14,18,22), juce::Colour(35,55,45));

    // Module titles
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));

    g.setColour(juce::Colour(110,125,140));
    g.drawText("MASTER SYSTEM", masterArea.withTrimmedLeft(10).withTrimmedTop(6).withHeight(14), juce::Justification::topLeft);

    g.setColour(juce::Colour(65,229,155));
    g.drawText("RE-201 SPACE ECHO", echoArea.withTrimmedLeft(10).withTrimmedTop(6).withHeight(14), juce::Justification::topLeft);

    g.setColour(juce::Colour(200,150,50));
    g.drawText("WEST COAST SYNTHESIS  259/281/292", synthArea.withTrimmedLeft(10).withTrimmedTop(6).withHeight(14), juce::Justification::topLeft);

    // --- Radar visual ---
    auto radarPure = radarArea;
    radarPure.removeFromBottom(110); // misma reserva que en resized()

    auto center = radarPure.getCentre().toFloat();
    float maxR   = std::min(radarPure.getWidth(), radarPure.getHeight()) * 0.45f;

    // Fondo circular
    g.setColour(juce::Colour(8, 10, 13));
    g.fillEllipse(center.x - maxR, center.y - maxR, maxR*2, maxR*2);

    // Cross-hair sutil
    g.setColour(juce::Colour(30, 38, 48));
    g.drawLine(center.x - maxR, center.y, center.x + maxR, center.y, 1.0f);
    g.drawLine(center.x, center.y - maxR, center.x, center.y + maxR, 1.0f);

    // Anillos + polígonos
    for (int t = 0; t < 6; ++t) {
        float r = (t + 1) * (maxR / 6.8f);
        bool sel = (t == currentTrack);
        bool mut = trackMutes[t];

        juce::Colour ringCol = mut ? juce::Colour(50,55,60) :
                               sel ? juce::Colours::yellow.withAlpha(0.35f) :
                                     juce::Colours::cyan.withAlpha(0.18f);
        g.setColour(ringCol);
        g.drawEllipse(center.x - r, center.y - r, r*2, r*2, 1.0f);

        juce::String ts = "t" + juce::String(t+1) + "_";
        int steps  = (int)audioProcessor.apvts.getRawParameterValue(ts+"steps") ->load();
        int pulses = (int)audioProcessor.apvts.getRawParameterValue(ts+"pulses")->load();
        int offset = (int)audioProcessor.apvts.getRawParameterValue(ts+"offset")->load();
        auto pat = gen_euclid(pulses, steps, offset);

        juce::Colour dotCol = mut ? juce::Colour(60,65,70) :
                              sel ? juce::Colours::yellow : juce::Colours::cyan;

        juce::Path poly;
        bool first = true;
        for (int i = 0; i < steps; ++i) {
            if (pat[i] != 1) continue;
            float angle = (i / (float)steps) * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;
            float px = center.x + std::cos(angle) * r;
            float py = center.y + std::sin(angle) * r;
            if (first) { poly.startNewSubPath(px, py); first = false; }
            else        { poly.lineTo(px, py); }
            g.setColour(dotCol);
            g.fillEllipse(px - 3.5f, py - 3.5f, 7.0f, 7.0f);
        }
        if (!first) {
            poly.closeSubPath();
            g.setColour(dotCol.withAlpha(0.55f));
            g.strokePath(poly, juce::PathStrokeType(1.5f));
        }

        // Playhead
        int cur = audioProcessor.voices[t].current_step % (steps > 0 ? steps : 1);
        float pAngle = (cur / (float)(steps > 0 ? steps : 16)) * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;
        float px = center.x + std::cos(pAngle) * r;
        float py = center.y + std::sin(pAngle) * r;
        g.setColour(juce::Colours::white.withAlpha(0.8f));
        g.drawEllipse(px - 5.0f, py - 5.0f, 10.0f, 10.0f, 2.0f);
    }
}
