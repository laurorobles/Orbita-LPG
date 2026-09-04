# 🔬 TECHNICAL SHEET — ORBITA-LPG

## 1. General Specifications
- **Format:** VST3, AU, CLAP, Standalone
- **OS:** Windows 10/11 (64-bit), macOS 10.13+ (Intel & Apple Silicon), Linux (Ubuntu 20.04+)
- **Architecture:** C++17 / JUCE 8.0.1
- **Voices:** 6 Independent Monophonic Channels
- **Audio Engine:** 64-bit floating point precision (Double precision internal math)
- **Sample Rate:** Supports 44.1kHz to 192kHz (DAW dependent)

## 2. DSP Engine Architecture
### Oscillators (Per Voice)
- **Algorithm:** Pure sine wave generator converted to Triangle/Square via Chebyshev polynomials and Waveshaping.
- **Morphing:** Continuous crossfade and shape interpolation (`Morph`).
- **Phase Modulation:** `Auto-FM` feedback loop scaled by `1.0 + (FM * 4.0)`.

### Wavefolder (ADAA)
- **Algorithm:** First-Order Anti-Derivative Anti-Aliasing (ADAA).
- **Core Function:** $f(x) = \sin(x \times \frac{\pi}{2})$
- **Anti-Derivative:** $F(x) = -\frac{2}{\pi} \cos(x \times \frac{\pi}{2})$
- **Implementation:** Computes the integral difference over a single sample delay to completely eliminate Nyquist folding frequencies without needing upsampling/downsampling latency.

### Low Pass Gate (Vactrol Model)
- **Filter Topology:** State Variable Filter (SVF) - 12dB/Octave Low Pass.
- **Vactrol Emulation:** The control voltage (CV) passes through a custom slew-limiter (Inertia filter) that models the slow decay of a Cadmium Sulfide (CdS) photoresistor.
- **Modes:**
  - VCA: Linear amplitude scaling.
  - VCF: Linear cutoff frequency scaling (`Brightness` mapped to base frequency, `Resonance` to Q).
  - LPG: Simultaneous, non-linear scaling of Cutoff and Amplitude with vactrol-style ring/decay memory.

### Master Effects
- **Space Echo:** Stereo delay line with independent L/R channel processing.
- **Tape Wow/Flutter:** Dual LFO setup (low frequency Wow, high frequency Flutter) modulating the read pointers of a 3rd-Order Lagrange interpolated delay buffer.

## 3. Sequencer & Timing
- **Clock Source:** APVTS (`juce::AudioPlayHead`).
- **Sync Method:** Strict Sample-Accurate PPQ (Pulses Per Quarter Note) phase-locking. Re-calculates position every block based on absolute DAW timeline. Does not drift upon looping or seeking.
- **Algorithm:** Bjorklund's Euclidean algorithm. Computes the most even distribution of $K$ pulses over $N$ steps.
- **MIDI Out:** Sample-accurate `juce::MidiMessage::noteOn` and `noteOff` generation. Note length is strictly derived from the `Fall` parameter translated into samples based on the host tempo.

## 4. Parameter Tree (APVTS)
- Exposes 127 automatable parameters to the host DAW.
- See `PluginProcessor.cpp` for the full layout tree.
- **Preset Management:** Full XML state serialization natively via `getStateInformation` and externally via the GUI `SAVE/LOAD` buttons.

## 5. Security & Licensing
- **License System:** 16-character serial key validation.
- **Cryptographic Hash:** Uses dual XOR 64-bit salt masking against random values.
- **Demo Mode:** `demoExpired` atomic flag triggers after exactly $SampleRate \times 60 \times 10$ samples (10 minutes). Bypasses audio output silently.
