# PPG Wave 3 Clone

Sintetizador wavetable inspirado conceptualmente en el PPG Wave 3.3.
Implementación original con JUCE 8, compatible con Ableton Live 12.

## Estado
- **Fase 1** completada: arquitectura, parámetros, 2 osciladores wavetable, polifonía (8 voces), UI genérica.
- Fases futuras: filtro multimodo, LFOs, matriz de modulación, efectos, presets, UI personalizada.

## Compilación
Este repositorio usa GitHub Actions. Cada `push` a `main` compila automáticamente y publica el VST3/AU como artefacto descargable.

## Instalación en macOS
1. Descarga el artefacto desde la pestaña **Actions**.
2. Copia el `.vst3` a `~/Library/Audio/Plug-Ins/VST3/`.
3. Ejecuta: `xattr -cr ~/Library/Audio/Plug-Ins/VST3/"PPG Wave 3 Clone.vst3"`
4. Abre Ableton Live, Preferencias → Plug-ins → Rescan.