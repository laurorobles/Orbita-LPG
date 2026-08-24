# 🪐 Órbita-LPG

<p align="center">
  <img src="assets/screenshot.png" alt="Órbita-LPG Interface" width="900" style="border-radius:8px;box-shadow:0 4px 16px rgba(0,0,0,0.4);" />
</p>

**Órbita-LPG** is an experimental West Coast synthesizer and 6-track Euclidean sequencer VST3/AU plugin built with JUCE 8.

Each of the 6 independent tracks generates rhythms using the **Bjorklund Euclidean algorithm**, feeds a morphable oscillator through a **Wavefolder + FM** circuit, and shapes the final sound with a physical **Vactrol Low Pass Gate (LPG)** emulation — all synchronized to a built-in clock or your DAW's transport. A **Space Echo** with Wow/Flutter gives life to the output.

> 🔑 **License:** [http://laurorobles.gumroad.com](http://laurorobles.gumroad.com)
> 🏷️ **Developer:** Extasis Records / Lauro Robles

---

## Features

- **6 independent Euclidean tracks** — Steps, Pulses, Offset per track
- **West Coast synthesis per track** — Triangle↔Square morph, Wavefolder, FM feedback, Pitch Drop
- **Vactrol LPG emulation** — Rise, Fall, and Vactrol Response (Resp) parameters
- **Global Chaos** — stochastic micro-modulation of pitch and fold per voice
- **Global Scale quantizer** — Chromatic, Major, Minor, Dorian, Phrygian, Lydian, Mixolydian, Pentatonic Major, Pentatonic Minor, Harmonic Minor
- **Space Echo with Wow/Flutter** — time-based delay with analog tape flutter simulation
- **6 individual stereo outputs** + Master output (multi-bus)
- **DAW sync** + internal BPM + Swing
- **Formats:** VST3 · AU · Standalone

---

## Installation

### macOS
```bash
installer/INSTALL_MAC.command
```

### Windows
```
installer\INSTALL_WINDOWS.bat
```

### Linux
```bash
installer/INSTALL_LINUX.sh
```
