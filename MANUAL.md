# 📖 USER MANUAL — ORBITA-LPG

Welcome to Orbita-LPG. This manual will guide you through the interface, synthesis concepts, and the generative sequencer matrix.

## 1. Interface Overview
The interface is divided into functional blocks, designed with a "Neubrutalist" aesthetic that prioritizes visual clarity over skeuomorphic design.

- **Top Bar:** Preset Manager (`LOAD`/`SAVE`), Sequence Toggle (`SEQ: ON/OFF`), Global Configuration (`CONFIG`), and License Activation.
- **Left Radar:** The visual representation of the 6 Euclidean Sequencers. Each ring represents a Voice (Track). The outermost ring is Voice 1 (Kick/Bass), and the innermost is Voice 6 (Hi-Hats/Noise).
- **Right Panel (Track Controls):** The synthesis and sequence parameters for the currently selected track.
- **Bottom Bar:** Global Macro Controls (Volume, Drive, Swing, Chaos, Space Echo) and the Global Scale Quantizer.

## 2. The Euclidean Sequencer
Select a track by clicking its corresponding button (T1 to T6) above the radar.

### Rhythm Controls
- **Steps:** The total length of the cycle (1 to 32 steps).
- **Pulses:** The number of active triggers distributed evenly across the steps.
- **Offset:** Rotates the pattern to change the starting beat.
- **Rate:** The clock divider synchronized to the DAW tempo (1/4, 1/8, 1/16, 1/32).

*Pro Tip:* Set T1 to 16 Steps / 4 Pulses for a standard 4/4 Kick drum. Set T2 to 15 Steps / 7 Pulses for an evolving, syncopated polymetric percussion that dances around the Kick.

## 3. West Coast Synthesis Engine
Once a pulse is triggered, it fires the synthesis engine for that track.

### Oscillator
- **Pitch:** Base tuning of the voice (Note mode or Hz mode).
- **Morph:** Morphs the waveform from a Triangle (0%) to a Square (100%).
- **Drop:** An aggressive, analog-style pitch envelope. Essential for creating kicks and percussion.
- **FM:** Auto-Frequency Modulation (Phase Modulation). Modulates the oscillator with itself, creating aggressive, metallic, and bell-like overtones.
- **Noise:** Mixes in white noise before the filter stage. Great for snares, hi-hats, and physical modeling strikes.

### Vactrol / Low Pass Gate (LPG)
- **Mode Toggle:** Click the `VCA`, `LPG`, or `VCF` text to switch modes.
  - *VCA:* Standard amplifier.
  - *VCF:* Standard Low Pass Filter.
  - *LPG:* Low Pass Gate (Filter and Amplifier linked to a vactrol model).
- **Rise:** Attack time.
- **Fall:** Decay time (and Note length for MIDI out).
- **Response:** The "memory" of the vactrol. Higher values create a sluggish, acoustic-like ringing decay (the "Buchla Bongo" effect).
- **Brgt (Brightness):** Sets the base frequency of the filter (only active in VCF mode).
- **Reso (Resonance):** Emphasizes the cutoff frequency (only active in VCF mode).

## 4. Global Controls & MIDI
- **Scale & Root:** Forces all oscillators to snap to musical scales (e.g., C Minor Harmonic).
- **Note / Hz Mode:** The `NOTE` button next to the Pitch knob switches between quantized musical notes and absolute frequency values.
- **Swing:** Delays even-numbered 16th notes for groove.
- **Chaos:** Introduces probability. Higher values cause random sequencer triggers to fail, humanizing the rhythm.
- **Echo:** Blends a master analog-style tape delay with Wow/Flutter.
- **Drive:** Master saturation before output.

### Using MIDI Out
Orbita-LPG automatically broadcasts its sequences as standard MIDI notes to your DAW. Route the MIDI output of the Orbita-LPG track to another VST in your DAW to use Orbita as a generative MIDI sequencer for your favorite synths!

## 5. Licensing and Demo
Orbita-LPG runs in fully functional Demo Mode for **10 minutes** per session. Once the demo expires, the audio will go silent and an activation window will appear.
Enter your 16-character license key (purchased from Gumroad) and click `ACTIVATE` to unlock the plugin forever.
