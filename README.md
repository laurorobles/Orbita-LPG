# 🪐 ORBITA-LPG

**Orbita-LPG** is a generative ecosystem and 6-voice polymetric synthesizer. Its architecture is intrinsically inspired by the **West Coast** modular design philosophy of the United States—a paradigm pioneered by geniuses like Don Buchla in the 1970s.

Unlike traditional subtractive (East Coast / Moog-style) synthesis, where you start with a harmonically rich waveform and use a filter to subtract harmonics, Orbita-LPG utilizes an additive and non-linear paradigm. You start with a relatively pure tone and use non-linear mathematics, modulation, and saturation to *inject* extreme harmonic complexity.

The synthesizer consists of a matrix of **6 Independent Channels**, where each channel is both an instrument and a rhythmic brain. Each channel features the following interconnected modules:

### ⚙️ The West Coast Synthesis Engine (Voice)
*   **Morphing Oscillator (Primary Generator):** The core of each voice. It allows continuous and fluid morphing from a Triangle wave (pure and percussive) to a Square wave (hollow and rich in odd harmonics).
*   **ADAA Wavefolder:** Instead of saturating the signal by clipping the peaks, the Wavefolder "folds" the wave back onto itself using the mathematical function `sin(x * pi/2)`. This generates rich, metallic harmonics and textures. It implements an integrated **Anti-Derivative Anti-Aliasing (ADAA) Oversampling** system, which virtually eliminates digital aliasing frequencies while consuming 0% extra CPU.
*   **Auto-FM Feedback:** A phase modulation index (inspired by the legendary Buchla 259) that allows the oscillator to modulate itself, creating bell tones, aggressive FM, and chaotic wide-band noise.
*   **Pitch-Drop Envelope:** A logarithmic transient generator dedicated exclusively to the oscillator's tuning. Perfect for synthesizing 808/909 style kicks, analog toms, or laser "zaps".
*   **Noise Mix:** Injects textured white noise before the amplification stage to simulate the physical strike of a drumstick on a membrane or breath through a tube.

### 💡 The Low Pass Gate (LPG) and Optical Vactrol
The generated sound ultimately passes through our mathematical model of a **Vactrol**, the classic opto-electrical component of West Coast synthesis (inspired by the Buchla 292). It features 3 operational modes:
1.  **VCA (Voltage Controlled Amplifier):** Pure linear behavior for dry percussions.
2.  **VCF (Voltage Controlled Filter):** Pure filter that reveals hidden `Brightness` and `Resonance` controls for harmonic sweeps.
3.  **LPG (Low Pass Gate):** The king mode. Couples the filter and volume simultaneously.
In addition to the classic attack (`Rise`) and decay (`Fall`) controls, the LPG module includes a **Response** parameter. This parameter simulates the optical "memory" or physical inertia of the Vactrol's photo-resistor, imparting an unparalleled acoustic percussiveness (like striking wood, membranes, or xylophones, famously known as "Buchla Bongos").

### 🎲 The Brain: Polymetric Euclidean Sequencer
The true heart of Orbita-LPG is its sequencer matrix. Instead of a classic linear step sequencer, it utilizes **6 simultaneous Euclidean sequencers** (based on the Bjorklund algorithm).
*   Configure `Steps` (cycle length), `Pulses` (active notes), and `Offset` (rotation) independently for each of the 6 voices.
*   **Independent Clock Dividers:** Each voice can run at its own speed (`Rate`: 1/4, 1/8, 1/16, 1/32). This allows for "Infinite Polymetry", where rhythms fall out of sync, evolve, and organically weave back together over dozens of bars, generating a living soundscape with just a few clicks.
*   **Hz vs Note Mode:** Disconnect the sequencers from the tonal matrix and tune the oscillators in absolute frequencies (`Hz`), or lock them into a musical grid controlled by the...

### 🌐 Global Modules and FX
*   **Scale Quantizer:** A global Root and 14 different musical scales force all oscillators to remain in perfect harmony. The generative engine will never play an out-of-scale note.
*   **Space Echo (Analog Delay):** A master stereo delay module on the plugin's output. It emulates the mechanical imperfections of vintage magnetic tape using Wow/Flutter algorithms interpolated by LFOs, giving the sound profound depth.
*   **Chaos and Swing Generator:** Adds human imperfection to the timing and probabilistic jumps in the Euclidean algorithm.
*   **Generative MIDI Out:** Orbita-LPG doesn't just generate sound; it generates MIDI! It sends all of its polymetric chaos to your DAW (`MIDI NoteOn` / `NoteOff`) to control other synthesizers, drum machines, and external analog hardware.
*   **Preset Manager (XML):** Load and save your sonic universes in `.xml` files to share or seamlessly swap them between Ableton, Logic, FL Studio, etc.

The result is an instrument where **rhythm defines timbre, and timbre defines rhythm**.

---

## 🛠 Installation and Requirements

**Compatibility:**
- **macOS** (10.13+): VST3, AU, CLAP, Standalone. *(Native support for Apple Silicon M1/M2/M3 and Intel)*.
- **Windows** (10/11 64-bit): VST3, CLAP, Standalone.
- **Linux** (Ubuntu 20.04+): VST3, CLAP, Standalone.

**Installation Steps:**
1. Download the `.zip` file for your operating system from the **Releases** section on GitHub.
2. Unzip and run the installers, or drag the files into your corresponding plugin folders:
   - **Windows:** `C:\Program Files\Common Files\VST3`
   - **Mac VST3:** `/Library/Audio/Plug-Ins/VST3`
   - **Mac AU:** `/Library/Audio/Plug-Ins/Components`
3. Open your DAW, scan your plugins, and look for **Orbita-LPG**.
4. Enjoy! The plugin includes 10 minutes of continuous evaluation time, after which you must enter your **Official License** in the floating activation window.

---

## 🔑 Obtaining a License

Orbita-LPG requires a 16-character license key for unlimited commercial use. You can purchase one and support the development at our store:

> **[🛒 Get License on Gumroad](http://laurorobles.gumroad.com)**

---
*Developed with ❤️ using JUCE by Extasis Records / Lauro Robles.*
