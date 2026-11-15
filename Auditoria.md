# Auditoría — EQP-1A 1.1.0

15 de noviembre de 2026. Revisión local de código, DSP, interfaz y comparación empírica con UADx Pultec EQP-1A EQ 1.2.16.

## Resultado

Las comprobaciones funcionales de DSP y pluginval nivel 5 están aprobadas. La equivalencia 1:1 completa con UADx no está conseguida: quedan diferencias en algunas combinaciones entre bandas.

Se recogieron 3920 respuestas de referencia a 44,1, 48, 96 y 192 kHz. El ajuste de las ramas calibradas tiene un error máximo de 0,047 dB en la banda medida. Esa cifra no describe la respuesta combinada ni la fase.

La comparación independiente del VST3 compilado tiene 220 casos. En 164 casos aislados, el máximo es 0,263 dB y el error RMS medio es 0,051 dB. En 48 casos combinados, el máximo es 4,479 dB y el error RMS medio es 0,439 dB. Se comprobaron además ocho casos a distintos niveles. Los resultados completos están en Reference/Compiled-comparison.json y su gráfica.

## Funcionalidad comprobada

- Mono/estéreo, parámetros y estado mediante pluginval nivel 5: SUCCESS.
- Curva conjunta de Boost/Atten de graves contrastada con UADx, incluido el valle resonante.
- Bandwidth modifica tanto anchura como altura de la curva de agudos.
- FIX encendido mantiene el RMS después de estabilizarse; apagado conserva la ganancia de EQ.
- Power conserva el paso directo con su retardo de compensación.
- Todos los pasos de agudos producen salida finita a 44,1, 48, 96 y 192 kHz.
- Listas exactas de nueve pasos de graves y 22 de agudos, estado y ausencia de mando Output.
- Apertura y reapertura a 934 × 238 después de ampliar el editor.
- Linealidad con FIX apagado y ausencia de ruido generado en silencio.
- Captura JUCE revisada: cinco controles superiores y tres inferiores, con ATTEN SEL arriba como en EQP-1A. Sin medidor ni solapamientos observados.

## Implementación

Filtros estables por canal con mezclas entre puntos de calibración, FIR de referencia y paso alto de 1 Hz. Sin saturación, ruido aleatorio ni modelado no lineal. La ruta de audio usa memoria preasignada y no accede a disco ni a red. FIX está enlazado en estéreo y limitado a ±36 dB.

Latencia comunicada al DAW: 86 muestras a 44,1/48 kHz, 118 a 96 kHz y 151 a 192 kHz. El bypass utiliza un retardo equivalente.

## Límites

El modelo en cascada no reproduce todas las interacciones de la red pasiva original. No se han igualado fase, distorsión ni sobrecarga. Las frecuencias extra sin equivalente UADx se interpolan o extrapolan. FIX iguala RMS, no picos instantáneos. No se ha comprobado cada DAW.

## Evidencia

- Validation/DSP-1.1.0.txt
- Validation/pluginval-1.1.0.txt
- Reference/Calibration.md
- Reference/Calibration-summary.json
- Reference/Compiled-comparison.json
- Reference/Comparacion-UADx.png
- Preview.png
- Tests/Tests.cpp
