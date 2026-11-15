# Calibración lineal contra UADx

Referencia local: UADx Pultec EQP-1A EQ 1.2.16. Se midió audio de entrada/salida; no se extrajo código ni se incluye el plugin de UAD.

3920 respuestas de impulso a 44,1, 48, 96 y 192 kHz: 484 combinaciones de graves, 462 de agudos y Bandwidth, 33 de atenuación de agudos y una referencia plana por frecuencia de muestreo. Salida UADx en 0 dB y EQ activada. La captura conserva la respuesta anterior al pico y la latencia declarada.

El modelo utiliza filtros lineales estables ajustados en magnitud y una respuesta base FIR con paso alto de 1 Hz. Los valores entre puntos se interpolan; los pasos extendidos sin equivalente UADx se interpolan o extrapolan. No reproduce distorsión, saturación ni ruido. FIX debe estar apagado para comparar magnitudes absolutas.

El error de ajuste de las ramas calibradas es menor de 0,047 dB entre 20 Hz y 20 kHz. Esa cifra describe el ajuste de ramas, no todas las combinaciones posibles ni la respuesta de fase.

La validación independiente del VST3 compilado contiene 220 casos: 164 aislados, 48 combinados y ocho a diferentes niveles. En los aislados, máximo 0,263 dB y error RMS medio de 0,051 dB. En los combinados, máximo 4,479 dB y error RMS medio de 0,439 dB. Persisten interacciones entre bandas que el modelo en cascada no reproduce completamente. **No es un clon 1:1 completo de UADx.**

La fase y las no linealidades no se han igualado. La latencia es 86 muestras a 44,1/48 kHz, 118 a 96 kHz y 151 a 192 kHz, y se comunica al host. El bypass conserva el retardo para mantener la compensación del DAW.

Evidencia: Calibration-summary.json, Compiled-comparison.json y Comparacion-UADx.png. Los datos empíricos finales están integrados en Source/CalibrationData.h.
