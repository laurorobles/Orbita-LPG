# 🪐 ÓRBITA-LPG

**Órbita-LPG** es un ecosistema generativo y sintetizador polimétrico de 6 voces. Su arquitectura está inspirada de manera intrínseca en la filosofía de diseño modular de la **Costa Oeste (West Coast)** de Estados Unidos, un paradigma pionerizado por genios como Don Buchla en los años 70. 

A diferencia de la síntesis sustractiva tradicional oriental (Moog), donde comienzas con una onda rica en armónicos y utilizas un filtro para sustraerlos, Órbita-LPG utiliza el paradigma aditivo y no-lineal. Comienzas con un tono relativamente puro y utilizas matemáticas no-lineales, modulación y saturación para *inyectar* complejidad armónica extrema. 

El sintetizador está compuesto por una matriz de **6 Canales Independientes**, donde cada canal es un instrumento y un cerebro rítmico en sí mismo. Cada uno de los canales cuenta con los siguientes módulos interconectados:

### ⚙️ El Motor de Síntesis por Voz (West Coast Voice)
*   **Oscilador Morphing (Generador Primario):** El núcleo de cada voz. Permite transicionar el oscilador de manera continua y fluida desde una onda Triangular (pura y percusiva) hasta una onda Cuadrada (hueca y rica en armónicos impares).
*   **Wavefolder ADAA:** En lugar de saturar la señal cortando los picos (clipping), el Wavefolder "dobla" la onda sobre sí misma usando la función matemática `sin(x * pi/2)`. Esto genera armónicos metálicos y texturas ricas. Implementa un sistema de **Oversampling Anti-Derivative (ADAA)** integrado, que elimina virtualmente las frecuencias de *aliasing* digital consumiendo 0% extra de CPU.
*   **Auto-FM Feedback:** Un índice de modulación de fase (Inspirado en el mítico Buchla 259) que permite que el oscilador se module a sí mismo, creando tonos de campanas, FM agresivo y ruido caótico de banda ancha.
*   **Envolvente Pitch-Drop:** Un generador de transitorios logarítmicos dedicado exclusivamente a la afinación del oscilador. Perfecto para sintetizar bombos tipo 808/909, toms analógicos o "zaps" láser.
*   **Inyector de Ruido (Noise Mix):** Añade ruido blanco texturizado antes de la etapa de amplificación para simular el golpeo físico de una baqueta sobre una membrana o el soplido en un tubo.

### 💡 El Low Pass Gate (LPG) y Vactrol Óptico
El sonido generado pasa finalmente por nuestro modelado matemático de un **Vactrol**, el componente opto-eléctrico clásico de la síntesis West Coast (inspirado en el Buchla 292). Tiene 3 modos de operación:
1.  **VCA (Voltage Controlled Amplifier):** Comportamiento lineal puro para percusiones secas.
2.  **VCF (Voltage Controlled Filter):** Filtro puro que revela los controles ocultos de `Brightness` y `Resonance` para barridos armónicos.
3.  **LPG (Low Pass Gate):** El modo rey. Acopla el filtro y el volumen simultáneamente.
Además de los clásicos controles de ataque (`Rise`) y decaimiento (`Fall`), el módulo LPG incluye un control de **Response**. Este parámetro simula la "memoria" óptica o inercia física de la foto-resistencia del Vactrol, impartiendo una percusividad acústica inigualable (como el golpeo sobre madera, membranas o xilófonos conocidos como "Buchla Bongos").

### 🎲 El Cerebro: Secuenciador Euclidiano Polimétrico
El verdadero corazón de Órbita-LPG es su secuenciador matriz. En lugar de un clásico secuenciador de pasos lineal, utiliza **6 secuenciadores Euclidianos** simultáneos (basados en el algoritmo de Bjorklund).
*   Configura `Steps` (longitud del ciclo), `Pulses` (notas activas) y `Offset` (rotación) de forma independiente para cada una de las 6 voces.
*   **Divisores de Reloj Independientes:** Cada voz puede correr a su propia velocidad (`Rate`: 1/4, 1/8, 1/16, 1/32). Esto permite crear "Polimetría Infinita", donde los ritmos se desincronizan, evolucionan y se vuelven a entrelazar orgánicamente tras decenas de compases, generando un paisaje sonoro vivo con apenas unos pocos clics.
*   **Modo Hz vs Note:** Desconecta los secuenciadores de la matriz tonal y afina los osciladores en frecuencias absolutas (`Hz`), o enciérralos en una cuadrícula musical controlada por el...

### 🌐 Módulos Globales y FX
*   **Cuantizador de Escalas:** Una tónica global y 14 escalas musicales distintas obligan a todos los osciladores a mantenerse en armonía perfecta. El generador nunca tocará una nota fuera de la escala elegida.
*   **Space Echo (Analog Delay):** Un módulo de delay maestro estéreo en la salida del plugin. Emula las imperfecciones mecánicas de una cinta magnética antigua mediante algoritmos Wow/Flutter interpolados por LFOs, dándole una dimensión profunda al sonido.
*   **Generador de Caos y Swing:** Añade imperfección humana a la sincronía y saltos probabilísticos en el algoritmo Euclidiano.
*   **MIDI Out Generativo:** Órbita-LPG no solo genera sonido; ¡también genera MIDI! Envía todo su caos polimétrico hacia tu DAW (`MIDI NoteOn` / `NoteOff`) para controlar otros sintetizadores, cajas de ritmo y hardware analógico externo.
*   **Gestor de Presets (XML):** Carga y guarda tus universos sonoros en archivos `.xml` para compartirlos o intercambiarlos entre Ableton, Logic, FL Studio, etc.

El resultado es un instrumento donde **el ritmo define el timbre y el timbre define el ritmo**.

---

## 🛠 Instalación y Requisitos

**Compatibilidad:**
- **macOS** (10.13+): VST3, AU, CLAP, Standalone. *(Soporte nativo Apple Silicon M1/M2/M3 y Intel)*.
- **Windows** (10/11 64-bit): VST3, CLAP, Standalone.
- **Linux** (Ubuntu 20.04+): VST3, CLAP, Standalone.

**Pasos de Instalación:**
1. Descarga el archivo `.zip` para tu sistema operativo desde la sección de **Releases** en GitHub.
2. Descomprime y ejecuta los instaladores o arrastra los archivos a tus carpetas correspondientes:
   - **Windows:** `C:\Program Files\Common Files\VST3`
   - **Mac VST3:** `/Library/Audio/Plug-Ins/VST3`
   - **Mac AU:** `/Library/Audio/Plug-Ins/Components`
3. Abre tu DAW, escanea tus plugins y busca **Orbita-LPG**.
4. ¡Disfruta! El plugin incluye 10 minutos de uso continuo, tras los cuales deberás introducir tu **Licencia Oficial** en la ventana flotante.

---

## 🔑 Obtención de Licencia

Órbita-LPG requiere de una clave de licencia (16 caracteres) para uso comercial ilimitado. Puedes adquirirla y apoyar el desarrollo en nuestra tienda:

> **[🛒 Obtener Licencia en Gumroad](http://laurorobles.gumroad.com)**

---
*Desarrollado con ❤️ usando JUCE por Extasis Records / Lauro Robles.*
