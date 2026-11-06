# Resultados de las pruebas P01 a P10

**Versión del firmware probada:** (hash del commit o etiqueta; se completa en cada ejecución)
**Fecha de ejecución:** (pendiente)
**VREF+ medido:** 3,30 V con multímetro entre 3V3 y GND, el 5 de noviembre de 2026 (`evidencias/vref_multimetro.jpg`)

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

## Comprobaciones parciales del Hito 1

Son verificaciones intermedias hechas durante el desarrollo, el 5 de noviembre de 2026. No sustituyen a las pruebas P02 y P05 finales, que se ejecutan con el firmware definitivo y la terminal serie, por eso sus estados arriba siguen en PENDIENTE.

| Prueba | Comprobación | Resultado | Evidencia |
| --- | --- | --- | --- |
| P02 (parcial) | LD2 cambia de estado cada 500 ms: un ciclo completo por segundo, contado a simple vista. El LED externo de alarma queda apagado tras el reinicio | Correcto | Observación directa |
| P05 (parcial) | Con el depurador de CubeIDE se observa `s_alarmMode`: una pulsación pasa de RUN a PAUSE, otra vuelve a RUN. Mantener B1 unos 5 s produce un solo cambio, sin repetición. LD2 sigue parpadeando en PAUSE | Correcto | `evidencias/P05_boton_depurador_1_pausa.png`, `evidencias/P05_boton_depurador_2_run.png` |

## Observaciones y FAIL explicados

(Se completa al ejecutar las pruebas.)
