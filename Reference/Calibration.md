# Calibración — v1.2.0

Se mantiene la calibración lineal inicial de 3920 respuestas y se añaden 3075 medidas de interacciones del UADx EQP-1A 1.2.16, realizadas a 48 kHz. Los filtros resultantes se derivan para 44,1/48/96/192 kHz y se comparan con UADx en las cuatro.

La nueva ruta conjunta de agudos reproduce las resonancias de Boost y Atten fuertes. Una corrección adicional reproduce parte de la carga entre atenuación de graves y agudos. Las curvas aisladas conservan su modelo anterior. Se usan funciones de transferencia interpoladas, factorización estable y transiciones suaves.

La calibración no introduce código ni binarios de UAD. El motor funciona sin UADx. No reproduce ruido, saturación ni distorsión. FIX debe estar apagado para comparar ganancias absolutas.

Consulta Auditoria.md y Compiled-comparison.json para los 476 casos, las mejoras y las diferencias pendientes. Los máximos de esos lotes no son límites universales y no se garantiza equivalencia 1:1 de fase o audio. Interaction-fit.json describe el ajuste interno de los modelos; no debe confundirse con el error global del plugin combinado.

Datos integrados: Source/CalibrationData.h y Source/InteractionData.h. La factoración está en Source/InteractionMath.h y su comprobación en Tests/InteractionMathTests.cpp.
