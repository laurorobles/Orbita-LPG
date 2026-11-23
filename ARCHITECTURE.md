# 🏛️ SOFTWARE ARCHITECTURE — ORBITA-LPG

This document outlines the C++ class structure, thread boundaries, and real-time audio/UI data flow in **Orbita-LPG v2.0.0**.

## 1. Core Components

### `OrbitaDSP` Namespace (`Source/PluginProcessor.h`)
Shared, header-only, allocation-free math helpers used identically by both the audio engine and the GUI:
- `isEuclideanHit(step, pulses, steps, offset)` — Deterministic O(1) Euclidean rhythm evaluator.
- `quantizePitch(raw_midi, scale_idx, root_note)` — 14-scale pitch quantizer.

### `WestCoastVoice` (`Source/PluginProcessor.h`, `Source/PluginProcessor.cpp`)
Encapsulates one monophonic synthesis channel (6 instances inside `OrbitaLPGAudioProcessor`):
- Maintains oscillator phase, 281 envelope state, 1st-order ADAA wavefolder history (`last_fold_in`), Vactrol slew state (`lpg_state`), SVF states (`lpf_state`, `bandpass_state`), and MIDI Out gate counters.
- Publishes `current_step` and `visual_lpg` atomically for lock-free UI rendering.

### `OrbitaLPGAudioProcessor` (`Source/PluginProcessor.h`, `Source/PluginProcessor.cpp`)
Inherits from `juce::AudioProcessor`:
- Owns the `juce::AudioProcessorValueTreeState` (`apvts`) and cached `std::atomic<float>*` parameter pointers.
- Handles DAW PlayHead sync, swung Euclidean stepping, incoming/outgoing MIDI, 6-voice DSP rendering, multi-bus routing, Stereo Space Echo, Master Drive, and stereo peak metering.
- Communicates with the UI exclusively through lock-free atomics (`isPlaying`, `hostPlaying`, `seqEnabled`, `pendingReset`, `trackMutes`, `pendingStrike`, `peakL`, `peakR`).

### `OrbitaLPGAudioProcessorEditor` (`Source/PluginEditor.h`, `Source/PluginEditor.cpp`)
Inherits from `juce::AudioProcessorEditor` and `juce::Timer`:
- Uses `OrbitaLookAndFeel` for custom rotary arcs, color-coded faders, and dark neubrutalist controls.
- Renders the interactive Vactrol-glow radar, synth group headers, embedded `BinaryData::logo_png`, and master stereo VU meter at 30 Hz.
- Hosts `AboutOverlayComponent` for open-source license info, shortcuts, and MIDI documentation.

## 2. Audio Data Flow (`processBlock`)
1. **Pending UI Actions:** Consumes atomic `pendingReset` and `pendingStrike[0..5]` flags safely on the audio thread.
2. **Transport & Clock:** Reads `AudioPlayHead` for host playing state, BPM, and PPQ position.
3. **MIDI In:** Processes incoming NoteOn/NoteOff for Drum mode (`Ch 1, C1–F1`) and Chromatic mode (`Ch 1–6`), then clears the buffer for MIDI Out.
4. **Per-Sample Loop:**
   - Emits expired `MIDI NoteOff` events.
   - Evaluates swung Euclidean step transitions across all 6 tracks, triggering voices and emitting `MIDI NoteOn` events.
   - Processes unmuted voices through Morph -> Auto-FM -> ADAA Wavefolder -> Noise -> Vactrol LPG/VCA/VCF, writing to individual multi-out buses and summing into the stereo Master bus.
   - Processes Stereo RE-201 Space Echo (smoothed delay time, quadrature LFO Wow/Flutter, warm feedback filter) and Master `tanh` Drive.
5. **Metering:** Updates atomic `peakL` / `peakR` values for the GUI meter.

---
*Developed by Lauro Robles / Extasis Records (laurorobles@gmail.com).*
