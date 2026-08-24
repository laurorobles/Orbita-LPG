import re

with open("Source/PluginProcessor.cpp", "r") as f:
    content = f.read()

# Replace the loop in createParameterLayout with specific track defaults
old_layout = """    for (int i=0; i<6; ++i) {
        juce::String t = juce::String(i+1);
        layout.add(std::make_unique<juce::AudioParameterInt>("t"+t+"_steps", "T"+t+" Steps", 1, 32, 16));
        layout.add(std::make_unique<juce::AudioParameterInt>("t"+t+"_pulses", "T"+t+" Pulses", 0, 32, 4));
        layout.add(std::make_unique<juce::AudioParameterInt>("t"+t+"_offset", "T"+t+" Offset", 0, 32, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_pitch", "T"+t+" Pitch", 24.0f, 96.0f, 60.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_drop", "T"+t+" P.Drop", 0.0f, 1.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_morph", "T"+t+" Morph", 0.0f, 1.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_fold", "T"+t+" Fold", 0.0f, 1.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_fm", "T"+t+" FM Mod", 0.0f, 1.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_rise", "T"+t+" Rise", 0.001f, 1.0f, 0.01f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_fall", "T"+t+" Fall", 0.01f, 3.0f, 0.5f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_resp", "T"+t+" Resp", 0.05f, 1.0f, 0.5f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_brgt", "T"+t+" Brgt", 0.05f, 1.0f, 0.8f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_noise", "T"+t+" Noise", 0.0f, 1.0f, 0.0f));
        layout.add(std::make_unique<juce::AudioParameterFloat>("t"+t+"_vol", "T"+t+" Vol", 0.0f, 1.0f, 0.8f));
    }"""

new_layout = """    // T1: Kick
    layout.add(std::make_unique<juce::AudioParameterInt>("t1_steps", "T1 Steps", 1, 32, 16));
    layout.add(std::make_unique<juce::AudioParameterInt>("t1_pulses", "T1 Pulses", 0, 32, 4));
    layout.add(std::make_unique<juce::AudioParameterInt>("t1_offset", "T1 Offset", 0, 32, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t1_pitch", "T1 Pitch", 24.0f, 96.0f, 36.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t1_drop", "T1 P.Drop", 0.0f, 1.0f, 0.8f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t1_morph", "T1 Morph", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t1_fold", "T1 Fold", 0.0f, 1.0f, 0.2f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t1_fm", "T1 FM Mod", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t1_rise", "T1 Rise", 0.001f, 1.0f, 0.01f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t1_fall", "T1 Fall", 0.01f, 3.0f, 0.4f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t1_resp", "T1 Resp", 0.05f, 1.0f, 0.9f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t1_brgt", "T1 Brgt", 0.05f, 1.0f, 0.4f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t1_noise", "T1 Noise", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t1_vol", "T1 Vol", 0.0f, 1.0f, 0.9f));

    // T2: Snare / Ping
    layout.add(std::make_unique<juce::AudioParameterInt>("t2_steps", "T2 Steps", 1, 32, 16));
    layout.add(std::make_unique<juce::AudioParameterInt>("t2_pulses", "T2 Pulses", 0, 32, 2));
    layout.add(std::make_unique<juce::AudioParameterInt>("t2_offset", "T2 Offset", 0, 32, 4));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t2_pitch", "T2 Pitch", 24.0f, 96.0f, 72.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t2_drop", "T2 P.Drop", 0.0f, 1.0f, 0.3f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t2_morph", "T2 Morph", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t2_fold", "T2 Fold", 0.0f, 1.0f, 0.4f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t2_fm", "T2 FM Mod", 0.0f, 1.0f, 0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t2_rise", "T2 Rise", 0.001f, 1.0f, 0.01f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t2_fall", "T2 Fall", 0.01f, 3.0f, 0.3f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t2_resp", "T2 Resp", 0.05f, 1.0f, 0.8f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t2_brgt", "T2 Brgt", 0.05f, 1.0f, 0.9f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t2_noise", "T2 Noise", 0.0f, 1.0f, 0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t2_vol", "T2 Vol", 0.0f, 1.0f, 0.8f));

    // T3: Hihat
    layout.add(std::make_unique<juce::AudioParameterInt>("t3_steps", "T3 Steps", 1, 32, 16));
    layout.add(std::make_unique<juce::AudioParameterInt>("t3_pulses", "T3 Pulses", 0, 32, 16));
    layout.add(std::make_unique<juce::AudioParameterInt>("t3_offset", "T3 Offset", 0, 32, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t3_pitch", "T3 Pitch", 24.0f, 96.0f, 84.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t3_drop", "T3 P.Drop", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t3_morph", "T3 Morph", 0.0f, 1.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t3_fold", "T3 Fold", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t3_fm", "T3 FM Mod", 0.0f, 1.0f, 0.8f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t3_rise", "T3 Rise", 0.001f, 1.0f, 0.01f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t3_fall", "T3 Fall", 0.01f, 3.0f, 0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t3_resp", "T3 Resp", 0.05f, 1.0f, 0.95f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t3_brgt", "T3 Brgt", 0.05f, 1.0f, 0.9f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t3_noise", "T3 Noise", 0.0f, 1.0f, 0.9f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t3_vol", "T3 Vol", 0.0f, 1.0f, 0.6f));

    // T4: Bongo/Wood
    layout.add(std::make_unique<juce::AudioParameterInt>("t4_steps", "T4 Steps", 1, 32, 8));
    layout.add(std::make_unique<juce::AudioParameterInt>("t4_pulses", "T4 Pulses", 0, 32, 3));
    layout.add(std::make_unique<juce::AudioParameterInt>("t4_offset", "T4 Offset", 0, 32, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t4_pitch", "T4 Pitch", 24.0f, 96.0f, 55.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t4_drop", "T4 P.Drop", 0.0f, 1.0f, 0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t4_morph", "T4 Morph", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t4_fold", "T4 Fold", 0.0f, 1.0f, 0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t4_fm", "T4 FM Mod", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t4_rise", "T4 Rise", 0.001f, 1.0f, 0.02f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t4_fall", "T4 Fall", 0.01f, 3.0f, 0.2f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t4_resp", "T4 Resp", 0.05f, 1.0f, 0.7f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t4_brgt", "T4 Brgt", 0.05f, 1.0f, 0.6f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t4_noise", "T4 Noise", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t4_vol", "T4 Vol", 0.0f, 1.0f, 0.8f));

    // T5: FM Pluck
    layout.add(std::make_unique<juce::AudioParameterInt>("t5_steps", "T5 Steps", 1, 32, 16));
    layout.add(std::make_unique<juce::AudioParameterInt>("t5_pulses", "T5 Pulses", 0, 32, 5));
    layout.add(std::make_unique<juce::AudioParameterInt>("t5_offset", "T5 Offset", 0, 32, 2));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t5_pitch", "T5 Pitch", 24.0f, 96.0f, 48.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t5_drop", "T5 P.Drop", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t5_morph", "T5 Morph", 0.0f, 1.0f, 0.2f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t5_fold", "T5 Fold", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t5_fm", "T5 FM Mod", 0.0f, 1.0f, 0.6f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t5_rise", "T5 Rise", 0.001f, 1.0f, 0.01f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t5_fall", "T5 Fall", 0.01f, 3.0f, 0.4f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t5_resp", "T5 Resp", 0.05f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t5_brgt", "T5 Brgt", 0.05f, 1.0f, 0.8f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t5_noise", "T5 Noise", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t5_vol", "T5 Vol", 0.0f, 1.0f, 0.8f));

    // T6: Drone / Bass
    layout.add(std::make_unique<juce::AudioParameterInt>("t6_steps", "T6 Steps", 1, 32, 16));
    layout.add(std::make_unique<juce::AudioParameterInt>("t6_pulses", "T6 Pulses", 0, 32, 1));
    layout.add(std::make_unique<juce::AudioParameterInt>("t6_offset", "T6 Offset", 0, 32, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t6_pitch", "T6 Pitch", 24.0f, 96.0f, 36.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t6_drop", "T6 P.Drop", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t6_morph", "T6 Morph", 0.0f, 1.0f, 1.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t6_fold", "T6 Fold", 0.0f, 1.0f, 0.8f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t6_fm", "T6 FM Mod", 0.0f, 1.0f, 0.2f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t6_rise", "T6 Rise", 0.001f, 1.0f, 0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t6_fall", "T6 Fall", 0.01f, 3.0f, 2.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t6_resp", "T6 Resp", 0.05f, 1.0f, 0.1f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t6_brgt", "T6 Brgt", 0.05f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t6_noise", "T6 Noise", 0.0f, 1.0f, 0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("t6_vol", "T6 Vol", 0.0f, 1.0f, 0.8f));
"""

content = content.replace(old_layout, new_layout)
with open("Source/PluginProcessor.cpp", "w") as f:
    f.write(content)
