import re

with open('Source/PluginProcessor.cpp', 'r') as f:
    c = f.read()

# Fix quantize_pitch floor division
c = c.replace('int octave = shifted / 12;', 'int octave = (int)std::floor(shifted / 12.0f);')

# Optimize rate branching
old_rate = """                int rate_idx = (int)tParams[t].rate->load();
                float divisor = 0.25f; 
                if (rate_idx == 0) divisor = 1.0f; else if (rate_idx == 1) divisor = 0.5f;
                else if (rate_idx == 2) divisor = 0.25f; else if (rate_idx == 3) divisor = 0.125f;"""
new_rate = """                static const float DIVISORS[4] = {1.0f, 0.5f, 0.25f, 0.125f};
                float divisor = DIVISORS[(int)tParams[t].rate->load() & 3];"""
c = c.replace(old_rate, new_rate)

with open('Source/PluginProcessor.cpp', 'w') as f:
    f.write(c)

with open('Source/PluginEditor.cpp', 'r') as f:
    e = f.read()

e = e.replace('int octave = shifted / 12;', 'int octave = (int)std::floor(shifted / 12.0f);')

with open('Source/PluginEditor.cpp', 'w') as f:
    f.write(e)
