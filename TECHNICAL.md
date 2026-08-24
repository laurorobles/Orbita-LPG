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
| Parámetros globales | 12 |
| Parámetros por track (× 6) | 14 × 6 = 84 |
| **Total de parámetros** | **96** |

## 5. I/O
| | Cantidad |
|---|---|
| **Entradas de audio** | 0 (generativo) |
| **Salidas de audio** | 1 Master Stereo + 6 Track Stereo = **7 buses estéreo** |
| **MIDI input** | No |
| **MIDI output** | No |

## 6. Motor de DSP
| Componente | Implementación |
|---|---|
| Oscilador | Triangle/Square morph con FM self-feedback |
| Wavefolder | Saturación armónica no lineal (sin(x·π/2)) |
| LPG / Vactrol | Emulación digital de LDR fotoresistencia (exponencial retardada) |
| Envolvente | Rise/Fall con 3 estados (idle/attack/decay) |
| Cuantización | Snap a escala musical (10 escalas) |
| Delay | juce::dsp::DelayLine (estéreo) con Wow LFO senoidal |
| Sequencer | Bjorklund Euclidean (O(n) bucket algorithm) |
| Chaos | Ruido blanco multiplicativo en pitch y fold |

> **Licencia:** [http://laurorobles.gumroad.com](http://laurorobles.gumroad.com)
