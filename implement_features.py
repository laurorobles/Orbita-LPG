import re

# -------------------------------------------------------------
# 1. Update PluginProcessor.h
# -------------------------------------------------------------
with open("Source/PluginProcessor.h", "r") as f:
    h = f.read()

# Add pending MIDI notes to WestCoastVoice
voice_decl = """    float env = 0.0f;
    int env_stage = 0;
    float gate_samples = 0.0f;
    float current_pitch = 60.0f;
    float chaos_latch = 0.0f;
    float last_chaos_amt = 0.0f;
    float last_drop_amt = 0.0f;
    int current_step = -1;
    
    // Feature: MIDI Out + ADAA state
    int active_midi_note = -1;
    int midi_gate_samples = 0;
    float last_fold_in = 0.0f;"""

h = re.sub(r'    float env = 0\.0f;.*?int current_step = -1;', voice_decl, h, flags=re.DOTALL)

with open("Source/PluginProcessor.h", "w") as f:
    f.write(h)

# -------------------------------------------------------------
# 2. Update PluginProcessor.cpp (MIDI Out + ADAA Wavefolder)
# -------------------------------------------------------------
with open("Source/PluginProcessor.cpp", "r") as f:
    c = f.read()

# Inside processBlock, at the top of the sample loop:
loop_top = """    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        // Process pending Note Offs
        for(int t=0; t<6; ++t) {
            if (voices[t].midi_gate_samples > 0) {
                voices[t].midi_gate_samples--;
                if (voices[t].midi_gate_samples == 0 && voices[t].active_midi_note >= 0) {
                    midiMessages.addEvent(juce::MidiMessage::noteOff(t + 1, voices[t].active_midi_note, 0.0f), sample);
                    voices[t].active_midi_note = -1;
                }
            }
        }
        
        if ((host_is_playing || this->isPlaying) && seqEnabled) {"""

c = c.replace('    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {\n        if ((host_is_playing || this->isPlaying) && seqEnabled) {', loop_top)

# Inside trigger_hit:
trigger_code = """                    if (trigger_hit) {
                        float raw_p = tParams[t].pitch->load();
                        bool note_mode = tParams[t].notemode->load() > 0.5f;
                        float q_pitch = note_mode ? quantize_pitch(raw_p, g_scale, g_root) : raw_p;
                        voices[t].trigger(q_pitch, tParams[t].drop->load(), chaos, beat_samples * divisor * 0.5f);
                        
                        // Schedule MIDI Out Note
                        if (voices[t].active_midi_note >= 0) {
                            midiMessages.addEvent(juce::MidiMessage::noteOff(t + 1, voices[t].active_midi_note, 0.0f), sample);
                        }
                        voices[t].active_midi_note = (int)q_pitch;
                        voices[t].midi_gate_samples = (int)(samples_per_step * 0.5f);
                        midiMessages.addEvent(juce::MidiMessage::noteOn(t + 1, (int)q_pitch, tParams[t].vol->load()), sample);
                    }"""

c = re.sub(r'                    if \(trigger_hit\) \{.*?beat_samples \* divisor \* 0\.5f\);\n                    \}', trigger_code, c, flags=re.DOTALL)

# ADAA Wavefolder in WestCoastVoice::process
old_wf = """            float fold_amt = p.fold->load() * 5.0f;
            if (fold_amt > 0.0f) {
                sig = sig * (1.0f + fold_amt);
                sig = std::sin(sig * 1.5707963f);
            }"""

new_wf = """            float fold_amt = p.fold->load() * 5.0f;
            if (fold_amt > 0.0f) {
                sig = sig * (1.0f + fold_amt);
                // ADAA (Antiderivative Antialiasing) First-Order
                if (std::abs(sig - last_fold_in) > 1e-4f) {
                    float ad_x = -0.6366197f * std::cos(sig * 1.5707963f);
                    float ad_prev = -0.6366197f * std::cos(last_fold_in * 1.5707963f);
                    float new_sig = (ad_x - ad_prev) / (sig - last_fold_in);
                    last_fold_in = sig;
                    sig = new_sig;
                } else {
                    last_fold_in = sig;
                    sig = std::sin(sig * 1.5707963f);
                }
            } else { last_fold_in = sig; }"""

c = c.replace(old_wf, new_wf)

# Reset ADAA on reset()
c = c.replace('lpg_state = 0.0f; lpf_state = 0.0f; bandpass_state = 0.0f; phase = 0.0f;', 'lpg_state = 0.0f; lpf_state = 0.0f; bandpass_state = 0.0f; phase = 0.0f;\n    active_midi_note = -1; midi_gate_samples = 0; last_fold_in = 0.0f;')

with open("Source/PluginProcessor.cpp", "w") as f:
    f.write(c)

# -------------------------------------------------------------
# 3. Update PluginEditor.h and PluginEditor.cpp (Presets)
# -------------------------------------------------------------
with open("Source/PluginEditor.h", "r") as f:
    e_h = f.read()

e_h = e_h.replace('juce::TextButton playBtn{"PLAY"}, stopBtn{"STOP"}, seqBtn{"SEQ: ON"}, configBtn{"CONFIG"};',
                  'juce::TextButton playBtn{"PLAY"}, stopBtn{"STOP"}, seqBtn{"SEQ: ON"}, configBtn{"CONFIG"};\n    juce::TextButton loadBtn{"LOAD"}, saveBtn{"SAVE"};\n    std::unique_ptr<juce::FileChooser> chooser;')

with open("Source/PluginEditor.h", "w") as f:
    f.write(e_h)

with open("Source/PluginEditor.cpp", "r") as f:
    e_c = f.read()

preset_btns = """    addAndMakeVisible(loadBtn); addAndMakeVisible(saveBtn);
    loadBtn.setTooltip("Load a custom .xml preset"); saveBtn.setTooltip("Save current state to .xml preset");
    loadBtn.onClick = [this]() {
        chooser = std::make_unique<juce::FileChooser>("Load Preset", juce::File(), "*.xml");
        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc) {
                if (fc.getResult().existsAsFile()) {
                    juce::XmlDocument xmlDoc(fc.getResult());
                    if (auto xml = xmlDoc.getDocumentElement()) { audioProcessor.apvts.replaceState(juce::ValueTree::fromXml(*xml)); }
                }
            });
    };
    saveBtn.onClick = [this]() {
        chooser = std::make_unique<juce::FileChooser>("Save Preset", juce::File(), "*.xml");
        chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc) {
                if (auto xml = audioProcessor.apvts.copyState().createXml()) { xml->writeTo(fc.getResult()); }
            });
    };
"""
e_c = e_c.replace('    addAndMakeVisible(kitCombo);', preset_btns + '    addAndMakeVisible(kitCombo);')

# Resize Logic
resize_old = """        titleLabel.setBounds(r.removeFromLeft(160)); 
        kitCombo.setBounds(r.removeFromLeft(180).reduced(0, 2)); r.removeFromLeft(10);
        playBtn.setBounds(r.removeFromLeft(55).reduced(0,2)); r.removeFromLeft(5);
        stopBtn.setBounds(r.removeFromLeft(55).reduced(0,2)); r.removeFromLeft(5); 
        seqBtn.setBounds( r.removeFromLeft(65).reduced(0,2)); r.removeFromLeft(5); 
        configBtn.setBounds(r.removeFromRight(75).reduced(0,2));"""

resize_new = """        titleLabel.setBounds(r.removeFromLeft(145)); 
        loadBtn.setBounds(r.removeFromLeft(45).reduced(0, 2)); r.removeFromLeft(2);
        saveBtn.setBounds(r.removeFromLeft(45).reduced(0, 2)); r.removeFromLeft(5);
        kitCombo.setBounds(r.removeFromLeft(160).reduced(0, 2)); r.removeFromLeft(10);
        playBtn.setBounds(r.removeFromLeft(50).reduced(0,2)); r.removeFromLeft(5);
        stopBtn.setBounds(r.removeFromLeft(50).reduced(0,2)); r.removeFromLeft(5); 
        seqBtn.setBounds( r.removeFromLeft(60).reduced(0,2)); r.removeFromLeft(5); 
        configBtn.setBounds(r.removeFromRight(75).reduced(0,2));"""

e_c = e_c.replace(resize_old, resize_new)

with open("Source/PluginEditor.cpp", "w") as f:
    f.write(e_c)

