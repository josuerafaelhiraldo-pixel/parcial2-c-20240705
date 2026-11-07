# Resultados de las pruebas P01 a P10

**Versión del firmware probada:** commit `5e45328` (último cambio de firmware; los commits posteriores solo tocan documentación y pruebas)
**Fecha de ejecución:** desde el 7 de noviembre de 2026 (Hito 4)
**VREF+ medido:** 3,30 V con multímetro entre 3V3 y GND, el 5 de noviembre de 2026 (`evidencias/vref_multimetro.jpg`)

Un estado PENDIENTE significa que la prueba todavía no se ejecutó. Ninguna prueba se marca PASS sin su evidencia.

| ID | Resultado esperado | Resultado observado | Estado | Evidencia |
| --- | --- | --- | --- | --- |
| P01 | Compila y arranca. MODE=RUN, THR=70, ALARM=0, PCT cercano a 0 | Compilación sin errores ni advertencias (text 20720, data 12, bss 2588). Con el potenciómetro al mínimo, en tres reinicios la primera respuesta a `STATUS` llegó con t = 36, 102 y 117 ms (menos de 200 ms): `MODE=RUN`, `THR=70`, `ALARM=0`, `PCT=0`, `ADC=1`. La primera telemetría sale en t = 1000 ms | PASS | `evidencias/P01_arranque.log`, `evidencias/P01_compilacion.png` |
| P02 | Adquisición cada 100 ms, LED cada 500 ms, telemetría cada 1000 ms (±5 %) | | PENDIENTE | |
| P03 | PCT = 100 × Vin / VREF dentro de ±3 puntos; ADC en rango; redondeo correcto | Con VREF = 3,30 V: 0,00 V da ADC 0 y PCT 0 (esperado 0,0); 1,66 V da ADC 2066 y 2057, PCT 50 (esperado 50,3); 3,30 V da ADC 4085, PCT 100 (esperado 100,0). Diferencia máxima 0,3 puntos; ADC dentro de 0 a 4095; el redondeo coincide con 100 × ADC / 4095 | PASS | `evidencias/P03_niveles.log`, `evidencias/P03_multimetro_0pct.jpg`, `evidencias/P03_multimetro_50pct.jpg`, `evidencias/P03_multimetro_100pct.jpg` |
| P04 | Con THR = 70 y PCT = 60, 72, 67, 62: ALARM = 0, 1, 1, 0 | Respuestas de `STATUS`: PCT 61 da ALARM 0; PCT 74 da ALARM 1 (dos respuestas); PCT 67, llegando desde 75, da ALARM 1; PCT 62 da ALARM 0. La telemetría registra además PCT 72 con ALARM 1 y PCT 60 con ALARM 0. En los 122 registros del log ninguno incumple la regla (PCT ≥ 70 da ALARM 1; PCT < 65 da ALARM 0) | PASS | `evidencias/P04_histeresis.log` |
| P05 | Una alternancia por pulsación; sin repetición al mantener; rebotes ignorados | | PENDIENTE | |
| P06 | En PAUSE la alarma está apagada y las demás tareas continúan; START restaura RUN y enciende la alarma | | PENDIENTE | |
| P07 | `THR 50` enciende, `THR 80` apaga; se aceptan 10 y 95; respuestas `OK THR=n` | | PENDIENTE | |
| P08 | Un `ERR` por línea inválida; THR y modo sin cambios | | PENDIENTE | |
| P09 | Una respuesta por comando completo; la línea de 33 caracteres da `ERR`; el STATUS posterior funciona | | PENDIENTE | |
| P10 | Sin bloqueos ni líneas corruptas; respuestas ≤ 100 ms; tiempos dentro de ±5 % | | PENDIENTE | |

## Detalle de P03

VREF+ = 3,30 V (`evidencias/vref_multimetro.jpg`). El multímetro tiene una resolución de 0,01 V. La columna «Vin según el ADC» es `ADC × 3,30 / 4095` y sirve para comprobar que la foto y la respuesta de `STATUS` corresponden al mismo punto del potenciómetro.

| Nivel | Vin medido | PCT esperado | Respuesta de `STATUS` (ADC / PCT) | Vin según el ADC | Diferencia de PCT |
| --- | --- | --- | --- | --- | --- |
| Mínimo | 0,00 V | 0,0 | 0 / 0 y 0 / 0 | 0,000 V | 0,0 |
| Centro | 1,66 V | 50,3 | 2066 / 50 y 2057 / 50 | 1,665 V y 1,658 V | 0,3 |
| Máximo | 3,30 V | 100,0 | 4085 / 100 | 3,292 V | 0,0 |

## Detalle de P04

THR = 70, así que la alarma enciende con PCT ≥ 70 y apaga con PCT < 65. Las líneas son las de `evidencias/P04_histeresis.log`.

| Nivel | PCT objetivo | Respuesta de `STATUS` (PCT / ALARM) | Línea del log | Cómo se llegó |
| --- | --- | --- | --- | --- |
| 1 | 60 | 61 / 0 | 51 | Bajando desde 72: la telemetría muestra PCT 72 con ALARM 1 (línea 31) y PCT 60 con ALARM 0 (línea 32) |
| 2 | 72 | 74 / 1 (dos respuestas) | 72 y 79 | Subiendo; la telemetría muestra PCT 71 con ALARM 1 (línea 67) |
| 3 | 67 | 67 / 1 | 96 | Bajando desde 75 (línea 86): dentro de la banda de 65 a 69 conserva la alarma encendida |
| 4 | 62 | 62 / 0 | 117 | Bajando: PCT 65 conserva ALARM 1 (línea 102) y PCT 61 ya da ALARM 0 (línea 103) |

Comprobación de la banda en los dos sentidos, con la telemetría:

- Subiendo, PCT 69 tiene ALARM 0 (línea 66) y PCT 71 ya tiene ALARM 1 (línea 67).
- Bajando, PCT 69, 68, 67 y 65 tienen ALARM 1 (líneas 90 a 102) y PCT 61 tiene ALARM 0 (línea 103).

## Comprobaciones parciales del Hito 1

Son verificaciones intermedias hechas durante el desarrollo, el 5 de noviembre de 2026. No sustituyen a las pruebas P02 y P05 finales, que se ejecutan con el firmware definitivo y la terminal serie, por eso sus estados arriba siguen en PENDIENTE.

| Prueba | Comprobación | Resultado | Evidencia |
| --- | --- | --- | --- |
| P02 (parcial) | LD2 cambia de estado cada 500 ms: un ciclo completo por segundo, contado a simple vista. El LED externo de alarma queda apagado tras el reinicio | Correcto | Observación directa |
| P05 (parcial) | Con el depurador de CubeIDE se observa `s_alarmMode`: una pulsación pasa de RUN a PAUSE, otra vuelve a RUN. Mantener B1 unos 5 s produce un solo cambio, sin repetición. LD2 sigue parpadeando en PAUSE | Correcto | `evidencias/P05_boton_depurador_1_pausa.png`, `evidencias/P05_boton_depurador_2_run.png` |

## Comprobaciones parciales del Hito 2

Hechas el 6 de noviembre de 2026 con el depurador (vista Live Expressions) y el LED externo. No sustituyen a P03 y P04 finales, que se ejecutan con la terminal serie y el multímetro; por eso sus estados arriba siguen en PENDIENTE.

| Prueba | Comprobación | Resultado | Evidencia |
| --- | --- | --- | --- |
| P03 (parcial) | Potenciómetro en los extremos y en el centro: `s_raw` = 2, 2048 y 4094; `s_pct` = 0, 50 y 100. Coincide con el redondeo de 100 × ADC / 4095 | Correcto | `evidencias/P03_depurador_0pct.png`, `evidencias/P03_depurador_50pct.png`, `evidencias/P03_depurador_100pct.png` |
| P04 (parcial) | Con THR = 70: PCT = 60, 72, 67 (bajando desde 72) y 62 dan `s_alarmOn` = 0, 1, 1 y 0; el LED externo lo sigue | Correcto | `evidencias/P04_depurador_1_pct60.png` a `evidencias/P04_depurador_4_pct62.png`, `evidencias/P04_led_alarma_encendido.jpg`, `evidencias/P04_led_alarma_apagado.jpg` |
| P04 (umbrales) | Al subir despacio enciende en PCT = 70; al bajar despacio apaga en PCT = 64 (THR − 5 = 65 y apaga por debajo) | Correcto | Observación directa con Live Expressions |
| P06 (parcial) | Con la alarma encendida, B1 pasa a PAUSE y el LED se apaga al instante; B1 otra vez vuelve a RUN y enciende de inmediato si PCT ≥ 70. Si se baja el potenciómetro en PAUSE y se reanuda, queda apagada. Las muestras siguen llegando en PAUSE | Correcto | `evidencias/P05_boton_con_adc_1_pausa.png`, `evidencias/P05_boton_con_adc_2_run.png` |
| P01 (parcial) | Tras el reinicio, la primera muestra decide la alarma: con el potenciómetro alto el LED enciende casi al instante; con el potenciómetro bajo queda apagado | Correcto | Observación directa |

## Comprobaciones parciales del Hito 3

Hechas el 7 de noviembre de 2026 con Tera Term (COM8, 115200 8N1, New-line Transmit = LF, Local echo activado). No sustituyen a P01 y P06 a P09 finales, que se ejecutan con registros y marcas de tiempo en el Hito 4; por eso sus estados arriba siguen en PENDIENTE. Con Local echo activado, el texto escrito puede verse pegado a una línea de telemetría: es solo la pantalla de Tera Term, no la placa.

| Prueba | Comprobación | Resultado | Evidencia |
| --- | --- | --- | --- |
| P01 (parcial) | Enlace UART a 115200 8N1: con el firmware de eco, lo escrito en Tera Term vuelve desde la placa (Local echo apagado) | Correcto | `evidencias/P01_enlace_uart_eco.png` |
| P02 (parcial) | La telemetría llega una vez por segundo: los valores de `t` avanzan de 1000 en 1000. La medición con marcas de tiempo queda para el Hito 4 | Correcto | `evidencias/P09_prueba_rapida_comandos.png` |
| P06 (parcial) | `STOP` responde `OK PAUSE` y la telemetría sigue con `MODE=PAUSE;ALARM=0` aunque PCT = 80 supere THR = 50. `START` responde `OK RUN` y la telemetría siguiente muestra `MODE=RUN;ALARM=1` | Correcto | `evidencias/P09_prueba_rapida_comandos.png` |
| P07 (parcial) | `THR 50` responde `OK THR=50` y el `STATUS` siguiente muestra `THR=50`. Los valores 10, 80 y 95 se prueban en P07 final | Correcto | `evidencias/P09_prueba_rapida_comandos.png` |
| P08 (parcial) | `THR 9`, `THR 96`, `THR abc`, `THR 50x`, `THR 50 extra`, `status` y `HOLA` dan un solo `ERR` cada uno; THR sigue en 50 y el modo no cambia | Correcto | `evidencias/P09_prueba_rapida_errores_1.png`, `evidencias/P09_prueba_rapida_errores_2.png` |
| P09 (parcial) | Una línea de más de 32 caracteres da un solo `ERR` y el `STATUS` siguiente responde con normalidad. El límite exacto de 32 y 33 caracteres se prueba en P09 final | Correcto | `evidencias/P09_prueba_rapida_errores_2.png` |

## Observaciones y FAIL explicados

- **P01, contenido del registro.** `P01_arranque.log` reúne varias ejecuciones del macro y tres reinicios en un solo archivo. Cada reinicio se ve como un salto de `t` a un valor pequeño (líneas 429, 519 y 708).
- **P01, eco local dentro de la telemetría.** Cuatro líneas muestran el texto `STATUS` incrustado en una línea de telemetría (por ejemplo `...MODE=RUSTATUS` y luego `N;ALARM=0`, líneas 251, 495, 685 y 755). Es el eco local de Tera Term escrito en medio de la línea que estaba llegando; la placa no envía ese texto.
- **P01, línea cortada por el reinicio.** La línea 516 (`t=5118;...;ALA`) quedó incompleta porque se pulsó RESET mientras la placa la transmitía. Durante P10 no se reinicia la placa.
- **P01, marcas de tiempo repetidas.** Las 47 líneas de telemetría de las líneas 13 a 59 tienen la misma marca (16:14:57.09x): Tera Term estuvo detenido mientras había un cuadro de configuración abierto y registró los datos acumulados al cerrarlo. Por eso no se abren cuadros de diálogo durante una medición.
- **P03, texto pegado en la terminal.** La línea 6 de `P03_niveles.log` es la ruta del archivo pegada por error en Tera Term mientras se registraba; la placa respondió con un solo `ERR` (línea 7) y siguió funcionando con normalidad.
- **P03, ajuste del potenciómetro.** Entre las líneas 40 y 99 la telemetría muestra el potenciómetro en movimiento mientras se buscaba el nivel del centro. Las respuestas de `STATUS` se pidieron con el mando quieto.
- **P03, nivel máximo.** Tiene una sola respuesta a `STATUS`; los niveles mínimo y centro tienen dos.
- **P03, hora de las fotos.** Las fotos se tomaron después de cerrar el registro (17:24:21, 17:29:34 y 17:30:49; el registro termina a las 17:23:03). El voltaje de cada foto coincide, dentro de la resolución de 0,01 V del multímetro, con el voltaje que corresponde al ADC de las respuestas de `STATUS` (tabla de arriba).
- **P03, ADC en el máximo.** Con 3,30 V el ADC marca 4085, equivalente a 3,292 V: 0,008 V menos, por debajo de la resolución del multímetro.
- **P04, estado inicial.** El registro empieza con el potenciómetro en PCT 81 y la alarma encendida; el primer `STATUS` (línea 24) confirma `THR=70` y `MODE=RUN`. El nivel 1 se alcanza bajando, de modo que la prueba comprueba el apagado desde la alarma encendida.
- **P04, valores de los niveles 1 y 2.** El potenciómetro no se detuvo exactamente en PCT 60 y 72: las respuestas de `STATUS` son PCT 61 y 74, en el mismo lado del umbral. Los valores exactos 72 y 60 están en la telemetría (líneas 31 y 32).
- **P04, `STATUS` con Enter tardío.** El texto `STATUS` de la línea 58 se escribió sin pulsar Enter; la respuesta de la línea 72 llegó al pulsarlo, unos 12 s después, y refleja ese instante (PCT 74), no el de escribirlo. La línea 53 (`STATUS}`) y la línea 75 (`STATUS` sobre texto pendiente, es decir `STATUSSTATUS`) son líneas inválidas y la placa respondió `ERR` en ambas.
