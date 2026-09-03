import re

with open("Source/PluginProcessor.h", "r") as f:
    h = f.read()

voice_decl = """class WestCoastVoice {
public:
    int active_midi_note = -1;
    int midi_gate_samples = 0;
    float last_fold_in = 0.0f;"""

h = h.replace("class WestCoastVoice {\npublic:", voice_decl)

with open("Source/PluginProcessor.h", "w") as f:
    f.write(h)

with open("Source/PluginProcessor.cpp", "r") as f:
    c = f.read()

# Fix samples_per_step undeclared error
# In processBlock, samples_per_step was removed by my regex!
c = c.replace('voices[t].midi_gate_samples = (int)(samples_per_step * 0.5f);', 'voices[t].midi_gate_samples = (int)((beat_samples * divisor) * 0.5f);')

with open("Source/PluginProcessor.cpp", "w") as f:
    f.write(c)

