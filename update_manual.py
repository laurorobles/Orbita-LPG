import re

with open("MANUAL.md", "r") as f:
    text = f.read()

# Update Space Echo table
old_echo_table = '''| **Echo HPF** | 20 – 2000 Hz | Filtro paso alto en el feedback — elimina graves del loop de delay para evitar acumulación de baja frecuencia. |
| **Echo LPF** | 1k – 20k Hz | Filtro paso bajo en el feedback — oscurece las repeticiones para simular la degradación de cinta. |
| **Wow** | 0 – 1 | Fluctuación de velocidad de la cinta (LFO senoidal sobre el tiempo de delay). Añade el característico movimiento "warbly" de los delays analógicos vintage. |'''

new_echo_table = '''| **Echo Sync** | On / Off | Sincroniza el tiempo de delay al BPM del proyecto o lo deja libre en milisegundos. |
| **Wow** | 0 – 1 | Fluctuación de velocidad de la cinta (LFO senoidal sobre el tiempo de delay). Añade el característico movimiento "warbly" de los delays analógicos vintage con interpolación Lagrange3rd. |'''

text = text.replace(old_echo_table, new_echo_table)

# Update Scales
old_scales = '''| 8 | Pentatónica Mayor | 0-2-4-7-9 |
| 9 | Pentatónica Menor | 0-3-5-7-10 |
| 10 | Menor Armónica | 0-2-3-5-7-8-11 |'''

new_scales = '''| 8 | Pentatónica Mayor | 0-2-4-7-9 |
| 9 | Pentatónica Menor | 0-3-5-7-10 |
| 10 | Menor Armónica | 0-2-3-5-7-8-11 |
| 11 | Phrygian Dominant | 0-1-4-5-7-8-10 |
| 12 | Hirajoshi | 0-2-3-7-8 |
| 13 | Whole Tone | 0-2-4-6-8-10 |
| 14 | Diminished | 0-1-3-4-6-7-9-10 |

Además de la escala, puedes elegir la **Tónica Global (Global Root)** (C, C#, D, etc.). Todos los canales que tengan encendido el botón `FREQ: NOTE` se limitarán automáticamente a estas notas válidas. Los canales que estén en modo `FREQ: Hz` ignorarán la escala y realizarán barridos de frecuencia continuos.'''

text = text.replace(old_scales, new_scales)

# Update Global params table
old_global = '''| `echo_hpf` | Echo HPF | 20–2000 Hz | 20 |
| `echo_lpf` | Echo LPF | 1k–20k Hz | 20000 |
| `echo_wow` | Echo Wow | 0–1 | 0.1 |'''

new_global = '''| `global_root` | Global Root | C - B | C |
| `is_playing` | Playing | On / Off | Off |
| `echo_wow` | Echo Wow | 0–1 | 0.1 |
| `echo_sync` | Echo Sync | On / Off | On |'''

text = text.replace(old_global, new_global)

# Update Track params table
old_track = '''| `tN_pitch` | Pitch | 24–96 MIDI | 60 (C4) |
| `tN_drop` | Pitch Drop | 0–1 | 0.0 |
| `tN_morph` | Morph | 0–1 | 0.0 (Triangle) |
| `tN_fold` | Wavefold | 0–1 | 0.0 |
| `tN_fm` | FM Mod | 0–1 | 0.0 |
| `tN_rise` | Rise | 0.001–1 s | 0.01 |
| `tN_fall` | Fall | 0.01–3 s | 0.5 |
| `tN_resp` | Vactrol Resp | 0.05–1 | 0.5 |
| `tN_brgt` | Brightness | 0.05–1 | 0.8 |
| `tN_reso` | Filter resonance Q. | 0-1 | 0.0 |
| `tN_noise` | Noise | 0–1 | 0.0 |
| `tN_vol` | Volume | 0–1 | 0.8 |'''

new_track = '''| `tN_rate` | Rate | 1/4 - 1/32 | 1/16 |
| `tN_pitch` | Pitch / Hz | 24–96 MIDI | 60 (C4) |
| `tN_notemode` | Note Mode | Hz / NOTE | NOTE |
| `tN_drop` | Pitch Drop | 0–1 | 0.0 |
| `tN_morph` | Morph | 0–1 | 0.0 (Triangle) |
| `tN_fold` | Wavefold | 0–1 | 0.0 |
| `tN_fm` | FM Mod | 0–1 | 0.0 |
| `tN_rise` | Rise | 0.001–1 s | 0.01 |
| `tN_fall` | Fall | 0.01–3 s | 0.5 |
| `tN_mode281` | 281 Env Mode | TRANS/SUST/CYCLE | TRANS |
| `tN_resp` | Vactrol Resp | 0.05–1 | 0.5 |
| `tN_brgt` | Brightness | 0.05–1 | 0.8 |
| `tN_reso` | Reso | 0-1 | 0.0 |
| `tN_mode292` | 292 Vactrol Mode | VCA / LPG / VCF | LPG |
| `tN_noise` | Noise | 0–1 | 0.0 |
| `tN_vol` | Volume | 0–1 | 0.8 |'''

text = re.sub(r'\| `tN_pitch` \| Pitch \| 24–96 MIDI \| 60 \(C4\) \|.*?\| `tN_vol` \| Volume \| 0–1 \| 0\.8 \|', new_track, text, flags=re.DOTALL)

with open("MANUAL.md", "w") as f:
    f.write(text)

