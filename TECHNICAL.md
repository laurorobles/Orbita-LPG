# 🔬 TECHNICAL SHEET — ORBITA-LPG

## 1. General Specifications
- **Version:** 2.0.0
- **License:** GNU General Public License v3.0 (GPLv3) — Free & Open Source
- **Formats:** VST3, AU (macOS), CLAP, Standalone
- **Platforms:** Windows 10/11 (x64), macOS 10.13+ (Universal arm64 + x86_64), Linux (x64)
- **Framework:** C++17 / JUCE 8.0.15
- **Voices:** 6 Independent Monophonic Channels + Stereo Master & 6 Stereo Multi-Outputs
- **Sample Rates:** 44.1 kHz to 192 kHz (dynamic delay buffer sizing)

## 2. DSP Engine Architecture
### Oscillators (Per Voice)
- **Core Generator:** Phase-accumulated Triangle/Square morphing oscillator (`Morph`).
- **Pitch Drop:** Exponential envelope modulation on oscillator frequency.
- **Phase Modulation:** Auto-FM feedback loop (`fm_phase = phase + sig * fm_mod * 0.5`).
- **Noise Generator:** Lock-free 32-bit Xorshift PRNG for white noise and Chaos latch.

### Wavefolder (1st-Order ADAA)
- **Core Function:** $f(x) = \sin(x)$ with input scaling $x = \text{sig} \cdot (1 + 8 \cdot \text{fold}) \cdot \frac{\pi}{2}$.
- **Anti-Derivative:** $F(x) = -\cos(x)$.
- **Implementation:** Computes $\frac{F(x_n) - F(x_{n-1})}{x_n - x_{n-1}}$ with midpoint fallback when $|x_n - x_{n-1}| < 10^{-5}$, drastically suppressing aliasing with zero latency.

### Low Pass Gate (Optical Vactrol Model)
- **Filter Topology:** 12 dB/oct State Variable Filter (SVF) with `tanh` internal state saturation.
- **Vactrol Emulation:** Asymmetric one-pole slew limiter (fast 2 ms optical rise, variable 15 ms – 1500 ms CdS decay via `RESP`).
- **Modes:** `VCA` (amplitude only), `LPG` (coupled cutoff & amplitude driven by `lpg_state`), `VCF` (filter cutoff + `exp_env` amplitude).

### Master Bus & Stereo Space Echo
- **Stereo Spread:** Subtle per-track panning in the Master sum (Track 1 centered) while individual Track multi-outputs remain centered.
- **Space Echo:** Dual 3rd-order Lagrange interpolated delay lines (`Lagrange3rd`), dynamically sized to `2.5 * sampleRate` for full 192 kHz safety.
- **Tape Character:** One-pole smoothed delay time (click-free tape pitch-glide), quadrature LFO Wow/Flutter (`sin`/`cos`), slight R-channel stereo offset, and 1-pole low-pass + `tanh` saturation in the feedback loop.
- **Master Drive:** Soft-clip `tanh` saturation stage followed by Master Volume and lock-free stereo peak metering.

## 3. Sequencer & Timing
- **Clock Sync:** Sample-accurate PPQ tracking synced to `juce::AudioPlayHead` when DAW transport is active, or internal BPM clock otherwise.
- **Swing Engine:** Monotonic swung-step boundary calculation shifting odd steps by `swing * 0.5 * divisor` in PPQ without drift.
- **Euclidean Distribution:** Shared O(1) deterministic Bresenham-Euclidean helper (`OrbitaDSP::isEuclideanHit`) guaranteeing 100% alignment between audio triggers and radar visuals across 2–32 steps and 0–32 pulses.
- **MIDI Out:** Sample-accurate `noteOn` / `noteOff` generation on Channels 1–6.

## 4. Thread Safety & State
- **Lock-Free Audio Thread:** Zero allocations, zero disk I/O, and atomic communication (`std::atomic`) for transport, mutes, manual strike triggers, reset requests, step indicators, Vactrol glow levels, and stereo VU metering.
- **APVTS:** 126 automatable parameters with full DAW state persistence and external `.xml` preset import/export.

---
*Developed by Lauro Robles / Extasis Records (laurorobles@gmail.com).*
