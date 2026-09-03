# 📋 FICHA TÉCNICA — ÓRBITA-LPG (v1.0.0)

## 1. Información General
| Campo | Valor |
|---|---|
| **Nombre** | Órbita-LPG |
| **Versión** | 1.0.0 |
| **Desarrollador** | Extasis Records / Lauro Robles |
| **Tipo** | Sintetizador West Coast + Secuenciador Euclidiano |
| **Tecnología** | JUCE 8.0.1 (C++20) |
| **Licencia** | Comercial — [laurorobles.gumroad.com](http://laurorobles.gumroad.com) |

## 2. Formatos y Compatibilidad
| Formato | macOS | Windows | Linux |
|---|---|---|---|
| VST3 | ✅ | ✅ | ✅ |
| AU (AudioUnit) | ✅ | — | — |
| Standalone | ✅ | ✅ | ✅ |
| CLAP | ✅ | ✅ | ✅ |

## 3. Requisitos de Sistema
| | Mínimo | Recomendado |
|---|---|---|
| **macOS** | 10.13 High Sierra | 12+ Monterey |
| **Windows** | Windows 10 (64-bit) | Windows 11 |
| **Linux** | Ubuntu 20.04+ | Ubuntu 22.04+ |
| **RAM** | 512 MB | 2 GB |
| **CPU** | Intel/AMD x64 o Apple Silicon | M1+ / Intel i5+ |
| **Sample Rate** | 44100 Hz | 44100 – 96000 Hz |

## 4. Parámetros de DSP
| Categoría | Cantidad |
|---|---|
| Parámetros globales | 13 |
| Parámetros por track (× 6) | 19 × 6 = 114 |
| **Total de parámetros** | **127** |

## 5. I/O
| | Cantidad |
|---|---|
| **Entradas de audio** | 0 (generativo) |
| **Salidas de audio** | 1 Master Stereo + 6 Track Stereo = **7 buses estéreo** |
| **MIDI input** | Sí (Canal 1: Drum Machine C1-F1. Canales 1-6: Multitímbrico) |
| **MIDI output** | No |

## 6. Motor de DSP
| Componente | Implementación |
|---|---|
| Oscilador | Triangle/Square morph con FM self-feedback |
| Wavefolder | Saturación armónica no lineal (sin(x·π/2)) |
| LPG / Vactrol | Emulación digital de LDR fotoresistencia (exponencial retardada) con 3 modos (VCA, LPG, VCF) |
| Envolvente | Rise/Fall con 3 estados (Transient, Sustain, Cycle) |
| Cuantización | Snap a escala musical y tónica global (14 escalas) con modo Hz |
| Delay | juce::dsp::DelayLine con Interpolación Lagrange3rd, Sync BPM y Wow LFO |
| Sequencer | Bjorklund Euclidean, Rate Divisor, Phase-Lock PPQ Host Sync |
| Chaos | Ruido blanco multiplicativo en pitch y fold |

> **Licencia:** [http://laurorobles.gumroad.com](http://laurorobles.gumroad.com)
