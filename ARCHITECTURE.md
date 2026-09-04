# 🏛️ SOFTWARE ARCHITECTURE — ORBITA-LPG

This document provides a high-level overview of the C++ class structure and data flow in Orbita-LPG.

## 1. Core Classes
### `OrbitaLPGAudioProcessor` (PluginProcessor.h/cpp)
The main brain of the plugin. Inherits from `juce::AudioProcessor`.
- **Responsibilities:**
  - Handles the audio callback `processBlock`.
  - Manages the `juce::AudioProcessorValueTreeState` (APVTS) which holds all parameters.
  - Syncs with the DAW via `getPlayHead()`.
  - Distributes DSP work to the 6 voices.
  - Handles MIDI output buffering.

### `WestCoastVoice` (PluginProcessor.h)
A struct representing a single synthesizer channel. 6 instances are created in the AudioProcessor.
- **Responsibilities:**
  - Stores phase accumulators for the oscillator.
  - Stores states for the ADAA wavefolder, Vactrol slew limiters, and SVF filter.
  - Processes audio sample-by-sample for its specific channel.
  - Tracks active MIDI notes and schedules `NoteOff` messages.

### `OrbitaLPGAudioProcessorEditor` (PluginEditor.h/cpp)
The GUI class. Inherits from `juce::AudioProcessorEditor`.
- **Responsibilities:**
  - Renders the Neubrutalist UI framework.
  - Draws the Euclidean circles dynamically in `paint()`.
  - Uses `juce::AudioProcessorValueTreeState::SliderAttachment` (and Button/ComboBox) to bind UI controls to DSP parameters in real-time.
  - Manages the License Activation Overlay.

### `LicenseManager` (LicenseManager.h)
Static class managing DRM.
- **Responsibilities:**
  - Hashes and validates 16-character keys.
  - Reads/Writes the `license.key` file in the user's OS AppData folder.

## 2. Audio Data Flow (processBlock)
1. **Clock Sync:** Reads DAW `currentPositionInfo`. Calculates absolute PPQ position.
2. **Buffer Clear:** Silences the audio buffer if `demoExpired` is true and `!LicenseManager::isLicensed()`.
3. **MIDI Out Clear:** Clears the incoming `juce::MidiBuffer` to prepare for output generation.
4. **Voice Processing Loop (Channels 1-6):**
   - **Sequencer Update:** Calculates the Euclidean pattern. Checks if the PPQ has crossed a gate threshold.
   - **Trigger:** If a pulse triggers, resets the voice's pitch envelope and Vactrol logic. Queues a `MIDI NoteOn`.
   - **DSP Loop (Sample by Sample):**
     - Generates Morphing wave.
     - Applies Auto-FM.
     - Applies ADAA Wavefolder.
     - Adds Noise.
     - Applies Vactrol LPG / VCA / VCF logic.
     - Tracks note length and queues `MIDI NoteOff` when the gate samples run out.
5. **Master Bus Processing:**
   - Sums all 6 voices.
   - Applies Global Drive.
   - Applies Space Echo (Delay with Wow/Flutter).
   - Hard clips at -0.1 dBFS.

## 3. UI Rendering Flow
- The GUI runs at a fixed 30fps via `juce::Timer`.
- `timerCallback()` checks if the sequencer positions have moved and triggers a `repaint()` if necessary.
- `paint()` computes polar coordinates ($r \cos(\theta), r \sin(\theta)$) to draw the rotating dots and active pulses on the 6 concentric radar rings.
- The `ActivationOverlayComponent` floats above everything and intercepts mouse clicks until a valid license is entered.
