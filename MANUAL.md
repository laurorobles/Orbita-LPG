# 🪐 ÓRBITA-LPG — MANUAL DE USUARIO EXHAUSTIVO

**Versión:** 1.0.0  
**Desarrollador:** Extasis Records / Lauro Robles  
**Licencia:** [http://laurorobles.gumroad.com](http://laurorobles.gumroad.com)

---

## ÍNDICE

1. [Filosofía — La Vía de la Costa Oeste](#1-filosofía)
2. [Descripción General del Instrumento](#2-descripción-general)
3. [Sección de Transporte Global](#3-sección-de-transporte-global)
4. [Los 6 Tracks Euclidianos](#4-los-6-tracks-euclidianos)
5. [Motor de Síntesis West Coast por Track](#5-motor-de-síntesis-west-coast-por-track)
6. [Vactrol Low Pass Gate (LPG)](#6-vactrol-low-pass-gate-lpg)
7. [Space Echo con Wow y Flutter](#7-space-echo-con-wow-y-flutter)
8. [Cuantizador de Escala Global](#8-cuantizador-de-escala-global)
9. [Guía de Parámetros Completa](#9-guía-de-parámetros-completa)
10. [Flujo de Señal (Signal Flow)](#10-flujo-de-señal)
11. [Consejos y Recetas de Sonido](#11-consejos-y-recetas-de-sonido)
12. [Instalación y Activación](#12-instalación-y-activación)

---

## 1. FILOSOFÍA

Órbita-LPG nace del diseño de Don Buchla y la tradición "West Coast": en lugar de usar un oscilador rico en armónicos y un filtro que los elimina, se comienza con una forma de onda simple y se añaden armónicos de forma controlada y expresiva. El ritmo y el timbre son inseparables — cada golpe del secuenciador dispara directamente el circuito de sonido, creando percusiones que respiran y viven.

El Algoritmo de Bjorklund (originalmente diseñado para sincronizar aceleradores de partículas nucleares, adaptado por Toussaint en 2004 para el análisis de ritmos del mundo) distribuye los golpes de la forma más equidistante posible dentro de un ciclo. El resultado: los ritmos africanos, latinos e IDM más complejos emergen de dos simples números.

---

## 2. DESCRIPCIÓN GENERAL

Órbita-LPG contiene **6 voces** (tracks) completamente independientes. Cada voz tiene su propio:

- Secuenciador euclidiano (Steps / Pulses / Offset)
- Oscilador morphable (Triangle ↔ Square)
- Wavefolder + FM feedback
- Envolvente de amplitud (Rise / Fall)
- Vactrol LPG con Response (Resp)
- Noise mixer
- Pitch Drop
- Volumen individual

Todas las voces comparten el reloj global (BPM / Swing), la escala de cuantización y el control de Chaos. El output final pasa por un Space Echo estéreo antes de salir por el bus Master.

---

## 3. SECCIÓN DE TRANSPORTE GLOBAL

| Parámetro | Rango | Descripción |
|---|---|---|
| **BPM** | 20 – 300 | Tempo interno del secuenciador. Si el DAW está en Play, se sincroniza automáticamente al tempo del proyecto. |
| **Swing** | 0 – 0.5 | Añade humanización a las notas pares del patrón, retrasándolas ligeramente para dar sensación de groove. |
| **Chaos** | 0 – 1 | Introduce micro-modulaciones aleatorias de hasta ±5% en el pitch y el fold de cada voz en cada disparo. A valores bajos añade calidez analógica; a valores altos desestabiliza el sistema de forma orgánica. |
| **Global Scale** | 1 – 10 | Escala musical a la que se cuantizan los pitches de todos los tracks. Ver sección 8. |
| **Master Vol** | 0 – 1 | Volumen final post-echo. |
| **Drive** | 0 – 1 | Saturación suave en el bus master. |
| **Play / Stop** | — | Inicia o detiene el secuenciador interno. La barra espaciadora también funciona como atajo. El secuenciador también responde al transporte del DAW. |

---

## 4. LOS 6 TRACKS EUCLIDIANOS

Cada track tiene 3 parámetros que definen su patrón rítmico:

| Parámetro | Rango | Descripción |
|---|---|---|
| **Steps** | 1 – 32 | Longitud total del ciclo del patrón en pasos de corchea (16th notes). |
| **Pulses** | 0 – 32 | Número de golpes distribuidos equitativamente por el algoritmo de Bjorklund. |
| **Offset** | 0 – 32 | Rotación del patrón. Desplaza el inicio para crear contratiempos o polimetrías. |

### Patrones Euclidianos Famosos

| Steps | Pulses | Nombre / Uso |
|---|---|---|
| 16 | 4 | Cuatro al piso — Techno / House clásico |
| 8 | 3 | Tresillo cubano / Reggae |
| 16 | 5 | Clave de Bossa Nova / Son cubano |
| 16 | 7 | Samba / IDM avanzado |
| 12 | 5 | Quintillo en 12 |
| 13 | 5 | Polirritmo africano complejo |

Combinar dos o más tracks con Steps distintos (ej. 16 y 13) crea **polimetría** — los ciclos nunca se alinean exactamente igual, generando texturas infinitas y orgánicas.

---

## 5. MOTOR DE SÍNTESIS WEST COAST POR TRACK

Cuando el secuenciador euclidiano detecta un "1" (pulso) en el paso actual, dispara la voz. El flujo de síntesis es:

### Oscilador
El oscilador genera una onda base cuya forma puede morpharse entre **Triangular** (suave, rica en armónicos impares) y **Cuadrada** (intensa, más brillante) con el parámetro **Morph**.

### FM Feedback
El parámetro **FM Mod** aplica retroalimentación de fase — la salida del oscilador modula su propia fase en el siguiente ciclo. Esto genera parciales adicionales y un timbre más complejo y metálico sin necesidad de un oscilador modulador separado.

### Pitch Drop
Con **P.Drop** activo, el pitch del oscilador comienza en un punto más alto al inicio del disparo y decae exponencialmente siguiendo la envolvente. Crea el característico sonido de bombo electrónico (tombo, kick 808) o efectos de pitch-bend percusivos.

### Wavefolder
El **Fold** pliega la señal contra sí misma cuando supera los límites de ±1. Matemáticamente: `sig = sin(sig * π/2)` al saturar. Añade armónicos pares e impares de forma no lineal — pequeñas cantidades añaden calor; valores altos crean sonidos metálicos complejos similares a los Buchla 259.

### Noise
Mezcla ruido blanco con la señal del oscilador. Útil para parches de hi-hat, snare o texturas ruidosas.

---

## 6. VACTROL LOW PASS GATE (LPG)

El corazón del sonido "West Coast". El LPG combina el comportamiento de un filtro paso bajos y un amplificador en un único circuito controlado por luz (Vactrol = LED + LDR fotoresistencia).

A diferencia del ADSR tradicional:
- **No hay Sustain.** La voz siempre decae naturalmente.
- **Rise** controla el ataque (qué tan rápido sube la amplitud al recibir un trigger).
- **Fall** controla el decaimiento (qué tan rápido baja la amplitud después del pico).
- **Resp (Response)** simula la inercia del Vactrol físico: valores bajos = el gate abre y cierra muy lentamente (sonido de madera resonante, "plonk"). Valores altos = respuesta casi instantánea (sonido percusivo seco).

La ecuación de la simulación:
```
vactrol_speed = (env > lpg_state) ? resp * 0.5 : 0.05 + resp * 0.2
lpg_state += (env - lpg_state) * vactrol_speed
output = signal * lpg_state
```

---

## 7. SPACE ECHO CON WOW Y FLUTTER

Inspirado en el Roland RE-201 Space Echo, el efecto de delay final añade profundidad y movimiento analógico.

| Parámetro | Rango | Descripción |
|---|---|---|
| **Echo Time** | 0.05 – 1.0 s | Tiempo de delay. Sincroniza manualmente con el tempo para obtener efectos rítmicos. |
| **Echo Fdbk** | 0 – 0.9 | Feedback (repeticiones). A 0.9 el delay se regenera infinitamente. |
| **Echo Mix** | 0 – 1 | Balance señal seca vs. señal con delay. |
| **Echo Sync** | On / Off | Sincroniza el tiempo de delay al BPM del proyecto o lo deja libre en milisegundos. |
| **Wow** | 0 – 1 | Fluctuación de velocidad de la cinta (LFO senoidal sobre el tiempo de delay). Añade el característico movimiento "warbly" de los delays analógicos vintage con interpolación Lagrange3rd. |

---

## 8. CUANTIZADOR DE ESCALA GLOBAL

El parámetro **Global Scale** cuantiza el pitch de todos los tracks a la escala seleccionada:

| Valor | Escala | Intervalos |
|---|---|---|
| 1 | Cromática | 0-1-2-3-4-5-6-7-8-9-10-11 |
| 2 | Mayor | 0-2-4-5-7-9-11 |
| 3 | Menor Natural | 0-2-3-5-7-8-10 |
| 4 | Dórica | 0-2-3-5-7-9-10 |
| 5 | Frigia | 0-1-3-5-7-8-10 |
| 6 | Lidia | 0-2-4-6-7-9-11 |
| 7 | Mixolidia | 0-2-4-5-7-9-10 |
| 8 | Pentatónica Mayor | 0-2-4-7-9 |
| 9 | Pentatónica Menor | 0-3-5-7-10 |
| 10 | Menor Armónica | 0-2-3-5-7-8-11 |
| 11 | Phrygian Dominant | 0-1-4-5-7-8-10 |
| 12 | Hirajoshi | 0-2-3-7-8 |
| 13 | Whole Tone | 0-2-4-6-8-10 |
| 14 | Diminished | 0-1-3-4-6-7-9-10 |

Además de la escala, puedes elegir la **Tónica Global (Global Root)** (C, C#, D, etc.). Todos los canales que tengan encendido el botón `FREQ: NOTE` se limitarán automáticamente a estas notas válidas. Los canales que estén en modo `FREQ: Hz` ignorarán la escala y realizarán barridos de frecuencia continuos.

---

## 9. GUÍA DE PARÁMETROS COMPLETA

### Parámetros Globales
| ID | Nombre | Rango | Default |
|---|---|---|---|
| `master_vol` | Master Vol | 0–1 | 0.8 |
| `master_drive` | Drive | 0–1 | 0.0 |
| `bpm` | BPM | 20–300 | 120 |
| `swing` | Swing | 0–0.5 | 0.0 |
| `chaos` | Chaos | 0–1 | 0.0 |
| `global_scale` | Global Scale | 1–10 | 1 |
| `echo_time` | Echo Time | 0.05–1.0 s | 0.3 |
| `echo_fdbk` | Echo Feedback | 0–0.9 | 0.4 |
| `echo_mix` | Echo Mix | 0–1 | 0.3 |
| `global_root` | Global Root | C - B | C |
| `is_playing` | Playing | On / Off | Off |
| `echo_wow` | Echo Wow | 0–1 | 0.1 |
| `echo_sync` | Echo Sync | On / Off | On |

### Parámetros por Track (reemplazar `N` por 1–6)
| ID | Nombre | Rango | Default |
|---|---|---|---|
| `tN_steps` | Steps | 1–32 | 16 |
| `tN_pulses` | Pulses | 0–32 | 4 |
| `tN_offset` | Offset | 0–32 | 0 |
| `tN_rate` | Rate | 1/4 - 1/32 | 1/16 |
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
| `tN_vol` | Volume | 0–1 | 0.8 |

---

## 10. FLUJO DE SEÑAL

```
Clock / DAW Sync
      │
      ▼
Euclidean Sequencer (Steps, Pulses, Offset)
      │ Trigger
      ▼
┌─────────────────────────────────────────┐
│  West Coast Voice (× 6)                 │
│                                         │
│  Triangle↔Square Oscillator             │
│      │                                  │
│      ├── FM Feedback ─────────┐          │
│      │                       │          │
│      ▼                       │          │
│  Wavefolder (Fold)           │          │
│      │                       │          │
│      ▼                       ▼          │
│  Noise Mix ──────────── Pitch Drop Env  │
│      │                       │          │
│      ▼                       │          │
│  Vactrol LPG (Rise, Fall, Resp)         │
│      │                                  │
│      ▼                                  │
│  Volume ─────────────────────────────── │
└─────────────────────────────────────────┘
      │ Σ (6 voices summed)
      ▼
  Space Echo (Time, Fdbk, Mix, Wow)
      │
      ▼
  Master Drive → Master Vol → Output
```

---

## 11. CONSEJOS Y RECETAS DE SONIDO

### Bombo 808
- Track 1: Steps=16, Pulses=4, Offset=0
- Pitch=36, P.Drop=0.7, Morph=0, Fold=0
- Rise=0.001, Fall=0.8, Resp=0.9

### Hi-Hat Euclidiano
- Track 2: Steps=16, Pulses=7, Offset=3
- Pitch=80, Morph=0.5, Noise=0.7, Fold=0.3
- Rise=0.001, Fall=0.05, Resp=1.0

### Textura Metálica
- Track 3: Steps=13, Pulses=5, Offset=2
- Pitch=60, Fold=0.8, FM=0.5, Morph=0.3
- Chaos=0.4

### Polimetría Infinita
- Track 1: Steps=16, Pulses=3
- Track 2: Steps=12, Pulses=5
- Track 3: Steps=9, Pulses=4
- Resultado: el patrón completo se repite cada 432 pasos (LCM de 16, 12 y 9)

---

## 12. INSTALACIÓN Y ACTIVACIÓN

Descarga el instalador correspondiente a tu sistema operativo desde [GitHub Releases](https://github.com/laurorobles/Orbita-LPG/releases) o desde tu cuenta en [Gumroad](http://laurorobles.gumroad.com).

### Modos de operación
- **Demo (10 min):** El plugin opera completamente durante 10 minutos. Después el audio se silencia hasta introducir una clave de licencia.
- **Licencia completa:** Introduce tu clave en la ventana de activación para desbloquear el uso ilimitado.

> 🔑 **Adquiere tu licencia en:** [http://laurorobles.gumroad.com](http://laurorobles.gumroad.com)
