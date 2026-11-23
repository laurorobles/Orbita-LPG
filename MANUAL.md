# 📖 USER MANUAL — ORBITA-LPG

Welcome to **Orbita-LPG** (v2.0.0 — Free & Open Source, GPLv3). This manual covers the interface, Euclidean sequencer, West Coast synthesis engine, effects, and MIDI routing.

## 1. Interface Overview
The resizable interface is organized into four main modules:

- **Top Bar:** Preset `LOAD` / `SAVE` (`.xml`), Master Kit selector (`00. Init / Empty` through `25. Lo-Fi House`), `PLAY` / `STOP`, `SEQ: ON/OFF`, and `ABOUT` (shortcuts, MIDI map, and project info).
- **Left Panel (Euclidean Radar & Rhythm Controls):** Concentric rings representing Tracks 1–6 (innermost = T1, outermost = T6). Each ring glows dynamically with its voice's optical Vactrol energy. Click any ring to select it, or right-click to mute it.
- **Top-Right Panel (Master System):** `M.VOL`, `DRIVE`, `BPM`, `SWING`, `CHAOS`, Global `G.SCALE` and `G.ROOT`, plus a real-time stereo VU meter.
- **Middle-Right Panel (West Coast Synthesis 259/281/292):** Per-track synth preset menu, `RAND`, `< COPY` / `COPY >`, `NOTE` / `Hz` toggle, `281` and `292` mode selectors, `STRIKE` ping button, and 12 color-coded vertical faders grouped into `OSC 259`, `ENV 281`, `LPG 292`, and `MIX`.
- **Bottom-Right Panel (RE-201 Space Echo):** Stereo tape delay with `SYNC: ON/OFF`, `TIME`, `FDBK`, `MIX`, and `WOW`.

## 2. Keyboard Shortcuts
- **`Space`** — Play / Pause internal transport
- **`1` – `6`** — Select Track 1 to 6
- **`M`** — Mute / Unmute currently selected track
- **`Enter`** — Strike / ping the currently selected voice's LPG
- **`Esc`** — Close About overlay
- **Right-Click on `T1`–`T6` or Radar Ring** — Mute / Unmute track

## 3. The Euclidean Sequencer
Select a track (`T1`–`T6` or click its radar ring) to edit its pattern:
- **Steps (`2–32`):** Total cycle length.
- **Pulses (`0–32`):** Number of evenly distributed active hits (`0` silences the pattern).
- **Offset (`0–31`):** Rotates the pattern clockwise.
- **Rate (`1/4`, `1/8`, `1/16`, `1/32`):** Independent clock divider per track for true polymetry.
- **Toussaint Rhythms:** Classic ethnomusicological Euclidean patterns (Tresillo, Cinquillo, Bossa, Samba, Rumba, Aka, and polyrhythms).
- **MUTATE:** Gently mutates the current track by ±1 in Steps, Pulses, or Offset.

## 4. West Coast Synthesis Engine
### OSC 259
- **PITCH:** Base tuning in quantized musical notes (`NOTE`) or free frequency (`Hz`).
- **P.DRP:** Exponential pitch-drop transient for kicks, toms, and zaps.
- **MRPH:** Continuous waveform morph from Triangle (`0.0`) to Square (`1.0`).
- **FOLD:** 1st-order ADAA Wavefolder for alias-free harmonic folding.
- **FM:** Auto-FM phase feedback for bells and metallic overtones.

### ENV 281
- **Modes:** `TRANS` (one-shot percussive), `SUST` (holds while gate is open), `CYCLE` (self-retriggering LFO/loop).
- **RISE / FALL:** Attack and Decay times.

### LPG 292
- **Modes:** `VCA` (clean amp), `LPG` (coupled optical vactrol filter + amp), `VCF` (resonant 12 dB/oct filter).
- **RESP:** Vactrol optical decay inertia (15 ms – 1500 ms).
- **BRGT / RESO:** Filter cutoff brightness and resonance Q.

### MIX
- **NOISE:** Pre-LPG white noise mix.
- **VOL:** Voice output level.

## 5. MIDI & Multi-Output Audio
- **MIDI In (Drum Mode):** Channel 1, notes `C1` (36) to `F1` (41) trigger Tracks 1–6 using each track's internal pitch.
- **MIDI In (Chromatic Mode):** Channels 1–6 play Tracks 1–6 chromatically (quantized if `NOTE` mode is enabled).
- **MIDI Out:** The Euclidean sequencer outputs sample-accurate MIDI notes on Channels 1–6 to drive other plugins or external hardware.
- **Multi-Out Audio:** Route individual stereo buses (`Track 1` – `Track 6`) in your DAW mixer alongside the `Master` bus.

---
*Orbita-LPG is free and open-source software (GPLv3). Developed by Lauro Robles / Extasis Records (laurorobles@gmail.com).*
