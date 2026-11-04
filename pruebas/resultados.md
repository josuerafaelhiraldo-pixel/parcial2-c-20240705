# Resultados de las pruebas P01 a P10

**Versión del firmware probada:** (hash del commit o etiqueta; se completa en cada ejecución)
**Fecha de ejecución:** (pendiente)
**VREF+ medido:** (pendiente)

Un estado PENDIENTE significa que la prueba todavía no se ejecutó. Ninguna prueba se marca PASS sin su evidencia.

| ID | Resultado esperado | Resultado observado | Estado | Evidencia |
| --- | --- | --- | --- | --- |
| P01 | Compila y arranca. MODE=RUN, THR=70, ALARM=0, PCT cercano a 0 | | PENDIENTE | |
| P02 | Adquisición cada 100 ms, LED cada 500 ms, telemetría cada 1000 ms (±5 %) | | PENDIENTE | |
| P03 | PCT = 100 × Vin / VREF dentro de ±3 puntos; ADC en rango; redondeo correcto | | PENDIENTE | |
| P04 | Con THR = 70 y PCT = 60, 72, 67, 62: ALARM = 0, 1, 1, 0 | | PENDIENTE | |
| P05 | Una alternancia por pulsación; sin repetición al mantener; rebotes ignorados | | PENDIENTE | |
| P06 | En PAUSE la alarma está apagada y las demás tareas continúan; START restaura RUN y enciende la alarma | | PENDIENTE | |
| P07 | `THR 50` enciende, `THR 80` apaga; se aceptan 10 y 95; respuestas `OK THR=n` | | PENDIENTE | |
| P08 | Un `ERR` por línea inválida; THR y modo sin cambios | | PENDIENTE | |
| P09 | Una respuesta por comando completo; la línea de 33 caracteres da `ERR`; el STATUS posterior funciona | | PENDIENTE | |
| P10 | Sin bloqueos ni líneas corruptas; respuestas ≤ 100 ms; tiempos dentro de ±5 % | | PENDIENTE | |

## Observaciones y FAIL explicados

(Se completa al ejecutar las pruebas.)
