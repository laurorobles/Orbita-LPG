import re

with open("Source/PluginProcessor.h", "r") as f:
    h = f.read()

if "double internal_ppq" not in h:
    h = h.replace("bool isPlaying = false;", "bool isPlaying = false;\n    double internal_ppq = 0.0;")

with open("Source/PluginProcessor.h", "w") as f:
    f.write(h)

with open("Source/PluginProcessor.cpp", "r") as f:
    c = f.read()

# Replace getStateInformation and setStateInformation
old_state = "void OrbitaLPGAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {}\nvoid OrbitaLPGAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {}"
new_state = """void OrbitaLPGAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    if (xml != nullptr) copyXmlToBinary(*xml, destData);
}
void OrbitaLPGAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState != nullptr) {
        if (xmlState->hasTagName(apvts.state.getType())) {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}"""
c = c.replace(old_state, new_state)

# Replace sync logic in processBlock
sync_regex = re.compile(r'    bool is_playing = this->isPlaying;.*?float beat_samples = \(sr \* 60\.0f\) / bpm;\n', re.DOTALL)

new_sync = """    bool host_is_playing = false;
    double host_ppq = -1.0;
    float bpm = p_bpm ? p_bpm->load() : 124.0f;
    
    if (playhead != nullptr) {
        if (auto pos = playhead->getPosition()) {
            host_is_playing = pos->getIsPlaying();
            if (auto hostBpm = pos->getBpm()) bpm = (float)*hostBpm;
            if (auto hostPpq = pos->getPpqPosition()) host_ppq = *hostPpq;
        }
    }
    
    if (host_is_playing && host_ppq >= 0.0) {
        internal_ppq = host_ppq;
    }
    
    float chaos = p_chaos ? p_chaos->load() : 0.0f;
    int g_scale = p_global_scale ? (int)p_global_scale->load() : 1;
    int g_root = p_global_root ? (int)p_global_root->load() : 0;
    double sr = getSampleRate();
    float beat_samples = (sr * 60.0f) / bpm;
    double ppq_per_sample = 1.0 / (double)beat_samples;
"""
c = sync_regex.sub(new_sync, c)

# Replace the inner sequence loop
loop_regex = re.compile(r'        if \(is_playing && seqEnabled\) \{.*?            \}\n        \}\n', re.DOTALL)

new_loop = """        if ((host_is_playing || this->isPlaying) && seqEnabled) {
            for(int t=0; t<6; ++t) {
                static const float DIVISORS[4] = {1.0f, 0.5f, 0.25f, 0.125f};
                float divisor = DIVISORS[(int)tParams[t].rate->load() & 3];
                
                double absolute_step = internal_ppq / (double)divisor;
                double previous_step = (internal_ppq - ppq_per_sample) / (double)divisor;
                
                if (std::floor(absolute_step) > std::floor(previous_step)) {
                    int steps = (int)tParams[t].steps->load();
                    int pulses = (int)tParams[t].pulses->load();
                    int offset = (int)tParams[t].offset->load();
                    
                    voices[t].current_step = ((long long)std::floor(absolute_step)) % steps;
                    if (voices[t].current_step < 0) voices[t].current_step += steps;
                    
                    bool trigger_hit = false;
                    if (pulses >= steps) trigger_hit = true;
                    else if (pulses > 0) {
                        int current_val = ((voices[t].current_step + offset) * pulses) % steps;
                        trigger_hit = (current_val < pulses);
                    }
                    
                    if (trigger_hit) {
                        float raw_p = tParams[t].pitch->load();
                        bool note_mode = tParams[t].notemode->load() > 0.5f;
                        float q_pitch = note_mode ? quantize_pitch(raw_p, g_scale, g_root) : raw_p;
                        voices[t].trigger(q_pitch, tParams[t].drop->load(), chaos, beat_samples * divisor * 0.5f);
                    }
                }
            }
        }
        
        if (host_is_playing || this->isPlaying) {
            internal_ppq += ppq_per_sample;
        }
"""
c = loop_regex.sub(new_loop, c)

with open("Source/PluginProcessor.cpp", "w") as f:
    f.write(c)

