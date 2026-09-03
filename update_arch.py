import re

with open("ARCHITECTURE.md", "r") as f:
    text = f.read()

# Update graph block
new_graph = '''```mermaid
graph TD
    CLK["Clock / DAW Transport\n(BPM, Swing, Phase-Lock PPQ Sync)"] --> SEQ

    subgraph TRACKS["6 Tracks Independientes"]
        SEQ["Euclidean Sequencer\n(Bjorklund O(n) Bucket, Rate Divisors)"]
        SEQ -->|Trigger| ENV["Rise/Fall Envelope\n(281 Mode: TRANS/SUST/CYCLE)"]

        OSC["Triangle↔Square Oscillator\n(Morph parameter)"]
        OSC -->|FM self-feedback| OSC
        OSC --> WF["Wavefolder\nsin(x·π/2) non-linear saturation"]
        WF --> NZ["Noise Mix\n(white noise blend)"]

        ENV -->|LPG trigger| VACTROL["Vactrol LPG Emulation\n(292 Mode: VCA/LPG/VCF)\nvactrol_state += (env - state) × speed"]
        NZ --> VACTROL
        VACTROL --> VOL["Track Volume"]
    end

    VOL -->|Σ 6 voices| ECHO

    subgraph FX["Global FX"]
        ECHO["Space Echo\njuce::dsp::DelayLine\n+ Lagrange3rd Intp\n+ Wow LFO (sine flutter)"]
    end

    ECHO --> MASTER["Master Drive + Clip\n→ Stereo Output"]

    CHAOS["Chaos\n(stochastic pitch/fold mod)"] -.-> OSC
    CHAOS -.-> WF
    SCALE["Global Scale & Root Quantizer\n(14 scales, Note vs Hz Mode)"] -.-> OSC
```'''
text = re.sub(r'```mermaid.*?```', new_graph, text, flags=re.DOTALL)

with open("ARCHITECTURE.md", "w") as f:
    f.write(text)

