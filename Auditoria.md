# Actualización v1.4.0 — color sutil

Se añade una rama simétrica de saturación suave con ADAA de primer orden, antes de FIX y Output. Sin ruido aleatorio, con cero exacto tras reset y entrada nula. La ruta lineal conserva su latencia. No pretende reproducir la saturación del UAD. ADAA reduce aliasing, pero no garantiza eliminarlo; la coloración depende del nivel de entrada y de los realces EQ.

Validación actual: Validation/DSP-1.4.0.txt y Validation/pluginval-1.4.0.txt. Las mediciones UAD y de linealidad que siguen son históricas, anteriores a esta etapa. Las sesiones anteriores cambian ligeramente su sonido al cargar esta versión.

# Auditoría — EQP-1A 1.3.1

15 de noviembre de 2026. Revisión funcional, numérica y comparación de audio contra UADx EQP-1A 1.2.16 instalado localmente.

## Escala y edición numérica

Se instanció el UADx EQP-1A 1.2.16 instalado y se consultaron las dimensiones de su editor mediante VST3/JUCE. Tamaño estabilizado: 713 × 225. Se aplica esa proporción, límites de 356 × 112 a 2852 × 900 y apertura/reapertura compacta a 428 × 135. Se conservan dos filas de controles y Output después de FIX.

La edición por doble clic se verifica tanto en mandos como en sus etiquetas de valor, comprobando el valor recibido por el procesador. Se prueban coma decimal, Hz/kHz/dB, paso cercano en selectores, límites, texto inválido y cancelación. No se utiliza el doble clic para restablecer valores.

## Output después de FIX

Se añade un parámetro de −12 a +12 dB, inicial 0 dB, al final de la lista para conservar los índices de los parámetros anteriores. Se aplica después de la ganancia calculada por FIX y queda fuera de su medición. Transición de 35 ms, automatización y recuperación de estado. Power/bypass desconecta la ganancia manual.

La comprobación mide +6 y −6 dB con FIX activado, +6 dB con FIX desactivado, bypass a nivel unidad y recuperación del valor. Las comparaciones de curvas siguientes corresponden al motor v1.2.0 y se conservan con Output a 0 dB.

## Resultado

Pruebas DSP, linealidad, silencio y pluginval nivel 5 aprobadas. Mejora sustancial de las interacciones, conservando las curvas aisladas. No se declara equivalencia 1:1 completa.

| Comparación | Error RMS medio v1.1 → v1.2 | Máximo observado v1.1 → v1.2 |
|---|---:|---:|
| 48 combinaciones anteriores | 0,439 → 0,109 dB | 4,479 → 1,079 dB |
| 160 combinaciones adicionales | 0,591 → 0,192 dB | 20,752 → 1,752 dB |
| 96 casos del lote final | 0,523 → 0,138 dB | 22,017 → 7,123 dB |

Las 164 curvas aisladas anteriores conservan un máximo de 0,263 dB. Se incluyen además ocho casos a distintos niveles. Total comparado: 476 casos de binarios VST3 compilados, a 44,1, 48, 96 y 192 kHz.

Medición con impulso de amplitud 0,01, 65536 muestras, FIX apagado y salida UADx a 0 dB. Error de magnitud evaluado en 512 frecuencias logarítmicas de 20 Hz a 20 kHz. Se conserva y comunica la latencia. Las cifras no son una garantía para todas las posiciones, niveles o frecuencias continuas.

## Cambios del motor

3075 respuestas nuevas de interacción medidas a 48 kHz: 1500 de graves con atenuación de agudos y 1575 de Boost/Atten/Bandwidth de agudos conjuntos. Modelos derivados para las cuatro frecuencias de muestreo y comprobados contra referencias UADx en las cuatro.

El modelo conjunto de agudos se factoriza en biquads después de interpolar su función de transferencia. Se comprueba la estabilidad: se rechaza un diseño inválido y se utiliza una interpolación conservadora o la ruta independiente estable. Su contribución aumenta suavemente con Boost y Atten fuertes. Las curvas aisladas mantienen su ruta anterior.

La interacción amplia entre atenuaciones se corrige mediante secciones de Q fijo y ganancias calibradas. No se incluye ruido aleatorio, saturación, válvulas ni transformadores no lineales. No se carga UADx en el producto.

## Comprobaciones

- DSP a 44,1/48/96/192 kHz, todos los pasos de agudos y Bandwidth.
- FIX RMS, bypass con retardo y coincidencia estéreo para entradas idénticas.
- Linealidad al doblar la entrada con FIX apagado; silencio digital sin ruido generado.
- Automatización, estado, buses y editor: pluginval nivel 5 SUCCESS.
- Factorización numérica: 6300 diseños calibrados comprobados, cero fallos. Error relativo máximo de reconstrucción 2,1 × 10⁻¹¹ en las frecuencias de prueba.
- Selectores ampliados, Output después de FIX, ausencia de medidor y apertura/reapertura pequeña.
- Interfaz conservada: cinco controles arriba y cuatro abajo, incluyendo Output, ATTEN SEL arriba.
- Procesamiento con memoria preasignada; sin acceso a disco o red en el audio.

## Límites pendientes

Persisten interacciones de orden superior y diferencias de interpolación en resonancias estrechas. El lote final incluye un extremo con 7,123 dB de desviación observada a 192 kHz, frente a 8,470 dB de la versión anterior en ese mismo caso. Algunos ajustes individuales pueden empeorar pese a la mejora media. La fase, la sobrecarga y los armónicos no se han igualado. El objetivo elegido es un motor limpio e independiente.

Los pasos extra no existentes en UADx son aproximaciones. No se ha validado cada DAW. Latencia: 86 muestras a 44,1/48 kHz, 118 a 96 kHz y 151 a 192 kHz.

## Evidencia

- Validation/DSP-1.3.1.txt
- Validation/Math-1.2.0.txt
- Validation/pluginval-1.3.1.txt
- Reference/Compiled-comparison.json
- Reference/Comparacion-UADx-v1.2.png
- Reference/Interaction-fit.json
- Tests/Tests.cpp y Tests/InteractionMathTests.cpp
