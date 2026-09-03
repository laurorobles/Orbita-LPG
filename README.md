# 🪐 ÓRBITA-LPG

**Órbita-LPG** es un sintetizador generativo de 6 voces inspirado intrínsecamente en la filosofía de diseño modular de la **Costa Oeste (West Coast)** de Estados Unidos, pionerizada por Don Buchla en los años 70. 

A diferencia de la síntesis sustractiva tradicional (donde empiezas con una onda rica en armónicos y usas un filtro para restarlos), Órbita-LPG utiliza el paradigma aditivo/no-lineal: comienzas con una onda pura y usas un **Wavefolder** y **Frecuencia Modulada (FM)** para inyectar complejidad armónica masiva. Posteriormente, el sonido es esculpido por un **Low Pass Gate (LPG) basado en Vactrols**, un componente opto-eléctrico que imparte una percusividad acústica inigualable, simulando la física del golpeo sobre madera, membranas o metales.

El verdadero corazón del instrumento es su **Secuenciador Euclidiano Polimétrico**. Las 6 voces operan sobre longitudes de ciclo independientes que se desincronizan y entrelazan orgánicamente, generando patrones evolutivos ("polimetría infinita") con apenas unos pocos ajustes. El ritmo define el timbre y el timbre define el ritmo.

## ✨ Posibilidades y Características

- **Matriz Generativa:** 6 secuenciadores basados en el algoritmo de Bjorklund (`Steps`, `Pulses`, `Offset` y divisores de reloj `1/4 a 1/32`).
- **Motor West Coast por Voz:**
  - **Oscilador Morphing:** Transición continua de Triangular a Cuadrada.
  - **Wavefolder ADAA:** Saturación matemática extrema (`sin(x*pi/2)`) con *Oversampling Anti-Derivative* integrado que elimina frecuencias aliasing consumiendo 0% de CPU.
  - **Auto-FM Feedback:** Modulación de fase para conseguir timbres metálicos (Buchla 259 style).
  - **Envolvente Pitch-Drop:** Decaimientos logarítmicos para lograr bajos contundentes tipo 808/909.
- **Triple-Modo Vactrol (LPG):** Escoge entre VCA (Amplificador), LPG (Filtro+Amplitud de inercia lenta) o VCF (Filtro puro) con parámetros de `Rise`, `Fall` y `Response` (inercia óptica).
- **Cuantización Global y Note/Hz:** Fuerza a todos los secuenciadores a tocar notas dentro de 14 escalas musicales y una tónica global, o libéralos a modo `Hz` (frecuencia pura).
- **MIDI Out:** Funciona como un **cerebro MIDI**. Transmite notas y dinámicas (basadas en tu secuencia) hacia el host DAW para controlar sintetizadores externos.
- **Space Echo:** Delay analógico estéreo con flutter de cinta Wow (LFO interpolado).
- **Gestor de Presets (XML):** Carga y guarda tus parches `.xml` para intercambiarlos entre Ableton, Logic, FL Studio, etc.

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
