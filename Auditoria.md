# Auditoría — EQP-1A 1.0.1

15 de noviembre de 2026. Auditoría local de código, DSP, interfaz e instalación. No es una comparación acústica con UAD.

## Resultado

Comprobaciones aprobadas. Compilación Release x64 con Visual Studio 2026 y JUCE 9.0.5. VST3 y standalone generados e instalados. El VST3 instalado termina pluginval 1.0.4 en nivel 5 con SUCCESS.

## DSP

Pruebas con cuatro tonos simultáneos (30 Hz, 1 kHz, 4 kHz y 8 kHz), repetidas a 44,1, 48, 96 y 192 kHz:

- Respuesta neutra: desviación RMS menor de 0,03 dB.
- Realce de graves y agudos verificado mediante relaciones espectrales.
- Atenuación de graves simultánea al realce, sin cancelación trivial.
- Bandwidth: Broad amplía la respuesta respecto a Sharp; comprobado fuera del centro de la banda.
- Atenuación de agudos comprobada.
- FIX encendido: diferencia RMS menor de 0,15 dB en los casos principales tras estabilizarse, y menor de 0,25 dB en el barrido de todos los pasos de agudos.
- FIX apagado: conserva la ganancia de la EQ.
- Power apagado: paso directo con diferencia menor de 0,03 dB.
- Salidas finitas y canales idénticos para entradas estéreo idénticas.
- Los 22 pasos de agudos se procesan en las cuatro frecuencias de muestreo sin fallos de estas comprobaciones.

## Parámetros, estado e interfaz

- Nueve pasos de graves: 20 a 100 Hz en incrementos de 10 Hz.
- 22 pasos de agudos: 1 a 20 kHz, más 30 y 40 kHz.
- Recuperación de ajustes serializados.
- Sin parámetro ni mando Output; FIX es un interruptor automatizable.
- Apertura a 934 × 238 y reapertura a ese tamaño después de ampliar el editor.
- Captura real de JUCE revisada: panel de 1334 × 340, dos filas de cuatro mandos, un único Bandwidth, sin medidor ni solapamientos observados.
- Pluginval nivel 5 contra el binario instalado: consultar el informe original para las pruebas de parámetros, estado, procesamiento, buses y editor.

## Revisión e instalación

Revisadas las rutas de procesamiento, límites de frecuencia, filtros por canal, suavizado, bypass y compensación RMS. El procesamiento usa memoria preasignada y no accede a disco ni a red. FIX está enlazado en estéreo y limitado a ±36 dB; no hay limitador de picos. Los parámetros se suavizan durante los cambios.

Instalación efectiva en la carpeta VST3 del usuario y en Programs/Papielguapo/EQP-1A, con accesos directos en Escritorio e Inicio. La carpeta VST3 del sistema no era escribible y el instalador utilizó su alternativa de usuario.

## Límites conocidos

No se ha realizado comparación de audio contra UAD ni validación en cada DAW. Se utilizan filtros digitales propios, sin reproducción del circuito pasivo, válvulas o transformadores. FIX iguala RMS tras estabilizarse, no picos instantáneos. Las frecuencias internas se limitan a 0,45 × sample rate: para alcanzar 40 kHz se requiere 96 kHz o más. Sin sobremuestreo ni latencia añadida por bloques.

## Evidencia

- Validation/DSP-final.txt
- Validation/pluginval-installed-final.txt
- Preview.png
- Tests/Tests.cpp
