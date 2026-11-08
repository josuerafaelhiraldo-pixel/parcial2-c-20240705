# Resultados de las pruebas P01 a P10

**Versión del firmware probada:** P01 y P03 a P09 con el commit `5e45328`. El commit `4bbb5be` agrega la traza de tiempos `APP_TRACE`, apagada por defecto: con `APP_TRACE = 0` el programa compila al mismo tamaño (text 20720, data 12, bss 2588) y las macros de traza no generan código. P02 se midió con ese commit y `APP_TRACE = 1` (text 21252, data 12, bss 2596); P10 y el firmware final usan `APP_TRACE = 0`
**Fecha de ejecución:** desde el 7 de noviembre de 2026 (Hito 4)
**VREF+ medido:** 3,30 V con multímetro entre 3V3 y GND, el 5 de noviembre de 2026 (`evidencias/vref_multimetro.jpg`)

Un estado PENDIENTE significa que la prueba todavía no se ejecutó. Ninguna prueba se marca PASS sin su evidencia.

| ID | Resultado esperado | Resultado observado | Estado | Evidencia |
| --- | --- | --- | --- | --- |
| P01 | Compila y arranca. MODE=RUN, THR=70, ALARM=0, PCT cercano a 0 | Compilación sin errores ni advertencias (text 20720, data 12, bss 2588). Con el potenciómetro al mínimo, en tres reinicios la primera respuesta a `STATUS` llegó con t = 36, 102 y 117 ms (menos de 200 ms): `MODE=RUN`, `THR=70`, `ALARM=0`, `PCT=0`, `ADC=1`. La primera telemetría sale en t = 1000 ms | PASS | `evidencias/P01_arranque.log`, `evidencias/P01_compilacion.png` |
| P02 | Adquisición cada 100 ms, LED cada 500 ms, telemetría cada 1000 ms (±5 %) | Con la traza `APP_TRACE = 1` y un registro de 80 s (808 `@ACQ`, 162 `@LED`, 81 telemetrías), los intervalos con el reloj de la placa son de 100 ms, 500 ms y 1000 ms exactos (0,00 % de desviación; todos dentro de ±5 %). Con el reloj del PC, los periodos medios son 99,31, 496,55 y 993,14 ms (0,69 % menos que los nominales, dentro de ±5 %). LD2 alterna 1, 0, 1, 0 en los 162 cambios sin repetirse | PASS | `evidencias/P02_tiempos.log`, `evidencias/P02_intervalos.txt`, `evidencias/P02_compilacion_traza.png` |
| P03 | PCT = 100 × Vin / VREF dentro de ±3 puntos; ADC en rango; redondeo correcto | Con VREF = 3,30 V: 0,00 V da ADC 0 y PCT 0 (esperado 0,0); 1,66 V da ADC 2066 y 2057, PCT 50 (esperado 50,3); 3,30 V da ADC 4085, PCT 100 (esperado 100,0). Diferencia máxima 0,3 puntos; ADC dentro de 0 a 4095; el redondeo coincide con 100 × ADC / 4095 | PASS | `evidencias/P03_niveles.log`, `evidencias/P03_multimetro_0pct.jpg`, `evidencias/P03_multimetro_50pct.jpg`, `evidencias/P03_multimetro_100pct.jpg` |
| P04 | Con THR = 70 y PCT = 60, 72, 67, 62: ALARM = 0, 1, 1, 0 | Respuestas de `STATUS`: PCT 61 da ALARM 0; PCT 74 da ALARM 1 (dos respuestas); PCT 67, llegando desde 75, da ALARM 1; PCT 62 da ALARM 0. La telemetría registra además PCT 72 con ALARM 1 y PCT 60 con ALARM 0. En los 122 registros del log ninguno incumple la regla (PCT ≥ 70 da ALARM 1; PCT < 65 da ALARM 0) | PASS | `evidencias/P04_histeresis.log` |
| P05 | Una alternancia por pulsación; sin repetición al mantener; rebotes ignorados | Cinco pulsaciones separadas: MODE pasó PAUSE, RUN, PAUSE, RUN, PAUSE y cada `STATUS` coincidió con la telemetría. Una pulsación mantenida unos 5 s: una sola alternancia (PAUSE a RUN en t=8146000) y ninguna más en los 14 s siguientes. El rebote natural de B1 no produjo alternancias extra | PASS | `evidencias/P05_boton.log`, `evidencias/P05_boton_mantenido.log` |
| P06 | En PAUSE la alarma está apagada y las demás tareas continúan; START restaura RUN y enciende la alarma | `STOP` con PCT 80 y la alarma encendida: `OK PAUSE` y la telemetría siguiente muestra ALARM 0. En 111 líneas de telemetría en PAUSE, 81 con PCT ≥ 70 (hasta 99), ALARM fue siempre 0; el ADC siguió cambiando (68 valores distintos) y `t` avanzó de 1000 en 1000 sin saltos. `START` con PCT 80: `OK RUN` y ALARM 1 en la telemetría siguiente; `START` con PCT 30: `OK RUN` y ALARM 0. LD2 siguió parpadeando en PAUSE (observación directa) | PASS | `evidencias/P06_pause_run.log` |
| P07 | `THR 50` enciende, `THR 80` apaga; se aceptan 10 y 95; respuestas `OK THR=n` | Con PCT 60 y la alarma apagada: `THR 50` da `OK THR=50` y ALARM 1; `THR 80` da `OK THR=80` y ALARM 0; `THR 10` da `OK THR=10` y ALARM 1; `THR 95` da `OK THR=95` y ALARM 0. En cada caso, la telemetría siguiente y el `STATUS` posterior muestran el THR nuevo y la alarma esperada | PASS | `evidencias/P07_umbral.log` |
| P08 | Un `ERR` por línea inválida; THR y modo sin cambios | Las siete líneas inválidas enviadas (`THR 9`, `THR 96`, `THR abc`, `THR 50x`, `THR 50 extra` y `HOLA` dos veces) dieron un solo `ERR` cada una: siete `ERR` y ningún `OK`. Las 70 líneas de telemetría y respuestas del registro muestran siempre THR 70, MODE RUN y ALARM 0, y el `STATUS` final responde con normalidad (t=10504684). Una línea vacía no produjo respuesta | PASS | `evidencias/P08_errores.log` |
| P09 | Una respuesta por comando completo; la línea de 33 caracteres da `ERR`; el STATUS posterior funciona | Con el macro `p09_fragmentos.ttl`: `STA` y, 2 s después, `TUS` con LF dieron una sola respuesta de `STATUS`, y ninguna durante la pausa. `STATUS` con CRLF y con LF dieron una respuesta cada uno. La línea de 32 caracteres, la de 33 con LF y la de 33 con CRLF dieron un solo `ERR` cada una. Cada `STATUS` posterior a un error respondió con normalidad. Total: 5 respuestas de `STATUS` y 3 `ERR`, ninguna respuesta de más. La telemetría siguió sin saltos (t de 1000 en 1000) | PASS | `evidencias/P09_fragmentos.log`, `pruebas/herramientas/p09_fragmentos.ttl` |
| P10 | Sin bloqueos ni líneas corruptas; respuestas ≤ 100 ms; tiempos dentro de ±5 % | Con el firmware final (`APP_TRACE = 0`), 150 comandos en 29,69 s (5,02 por segundo) más un `STATUS` final, con el modo y el umbral cambiando y el potenciómetro en movimiento: los 151 comandos tuvieron una sola respuesta del tipo esperado; la latencia (eco a respuesta) fue de 1 a 6 ms, mediana 3 ms, ninguna sobre 100 ms; cada `STATUS` mostró el `THR` y el `MODE` esperados; ninguna línea corrupta; la telemetría salió cada 1000 ms exactos con el reloj de la placa, sin huecos; el `STATUS` final respondió normalmente | PASS | `evidencias/P10_carga.log`, `evidencias/P10_latencias.txt`, `pruebas/herramientas/p10_carga.ttl`, `pruebas/herramientas/analizar_p10.c` |

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

## Detalle de P05

La telemetría muestra el cambio de `MODE` en la primera línea posterior a la pulsación. Las líneas son las de cada registro.

| Acción | Cambio de MODE en la telemetría | `STATUS` posterior | Registro y línea |
| --- | --- | --- | --- |
| Inicio | — | t=7627863, RUN | `P05_boton.log`, línea 5 |
| Pulsación 1 | PAUSE en t=7631000 (línea 9) | t=7637575, PAUSE | `P05_boton.log`, línea 17 |
| Pulsación 2 | RUN en t=7640000 (línea 20) | t=7643677, RUN | `P05_boton.log`, línea 25 |
| Pulsación 3 | PAUSE en t=7647000 (línea 29) | t=7649247, PAUSE | `P05_boton.log`, línea 33 |
| Pulsación 4 | RUN en t=7654000 (línea 38) | t=7656574, RUN | `P05_boton.log`, línea 42 |
| Pulsación 5 | PAUSE en t=7661000 (línea 47) | t=7661939, PAUSE | `P05_boton.log`, línea 49 |
| Mantenida unos 5 s | RUN en t=8146000 (línea 10), sin más cambios hasta t=8160000 | antes t=8141992, PAUSE; después t=8154530, RUN | `P05_boton_mantenido.log`, líneas 5 y 20 |

## Detalle de P06

Las líneas son las de `evidencias/P06_pause_run.log`. THR = 70 durante toda la prueba.

| Paso | Respuesta | Telemetría o estado resultante | Línea |
| --- | --- | --- | --- |
| `STATUS` en PAUSE con PCT 82 | t=9266276, PAUSE, ALARM 0 | La alarma está apagada aunque PCT ≥ 70 | 7 |
| Potenciómetro en PAUSE (PCT 94, 91 y 99) | — | ALARM 0 en todas las líneas | 39, 60 a 72, 138 |
| `STATUS` en PAUSE con PCT 91 | t=9329331, PAUSE, ALARM 0 | — | 74 |
| `START` con PCT 80 | `OK RUN` | t=9351000, RUN, ALARM 1 | 97 y 98 |
| `STATUS` en RUN con PCT 80 | t=9364307, RUN, ALARM 1 | — | 113 |
| `STOP` con PCT 80 y alarma encendida | `OK PAUSE` | t=9382000, PAUSE, ALARM 0 (PCT 80) | 132 y 133 |
| `START` con PCT 30 | `OK RUN` | t=9402000, RUN, ALARM 0 | 154 y 155 |
| `STATUS` en RUN con PCT 30 | t=9411254, RUN, ALARM 0 | — | 166 |

## Detalle de P07

PCT = 60 durante toda la prueba, con MODE = RUN. Las líneas son las de `evidencias/P07_umbral.log`.

| Comando | Respuesta | Primera telemetría posterior | `STATUS` posterior |
| --- | --- | --- | --- |
| `STATUS` inicial | t=9927942, THR 70, ALARM 0 (línea 5) | — | — |
| `THR 50` | `OK THR=50` (línea 15) | THR 50, ALARM 1 (línea 16) | t=9943832, THR 50, ALARM 1 (línea 25) |
| `THR 80` | `OK THR=80` (línea 36) | THR 80, ALARM 0 (línea 37) | t=9960879, THR 80, ALARM 0 (línea 46) |
| `THR 10` | `OK THR=10` (línea 57) | THR 10, ALARM 1 (línea 58) | t=9976903, THR 10, ALARM 1 (línea 66) |
| `THR 95` | `OK THR=95` (línea 77) | THR 95, ALARM 0 (línea 78) | t=9992233, THR 95, ALARM 0 (línea 86) |

## Detalle de P08

THR = 70, MODE = RUN y PCT = 60 durante toda la prueba. Las líneas son las de `evidencias/P08_errores.log`.

| Línea enviada | Respuesta | Líneas del registro |
| --- | --- | --- |
| `STATUS` inicial | t=10450849, THR 70, RUN, ALARM 0 | 4 y 5 |
| `THR 9` | `ERR` | 14 y 15 |
| `THR 96` | `ERR` | 22 y 23 |
| `THR abc` | `ERR` | 30 y 31 |
| `THR 50x` | `ERR` | 39 y 40 |
| `THR 50 extra` | `ERR` | 46 a 48 |
| Enter sin texto | ninguna respuesta | 55 |
| `HOLA` | `ERR` | 56 a 58 |
| `HOLA` (segunda vez) | `ERR` | 61 y 62 |
| `STATUS` final | t=10504684, THR 70, RUN, ALARM 0 | 75 y 76 |

## Detalle de P09

Las líneas son las de `evidencias/P09_fragmentos.log`. Las respuestas de `STATUS` se distinguen de la telemetría porque su `t` no es múltiplo de 1000. La última columna es la diferencia de marcas de tiempo entre el eco del texto enviado y la respuesta.

| Paso del macro | Texto enviado | Respuesta | Líneas del registro | Eco a respuesta |
| --- | --- | --- | --- | --- |
| 1a | `STA` sin terminador | ninguna: durante la pausa de 2 s solo llega telemetría | 21 y 22 | — |
| 1b | `TUS` con LF (`STATUS` completo) | una respuesta de `STATUS`, t=10894344 | 23 y 24 | 3 ms |
| 2 | `STATUS` con CRLF | una respuesta, t=10896379 | 27 a 29 | 4 ms |
| 3 | `STATUS` con LF | una respuesta, t=10898411 | 32 y 33 | 3 ms |
| 4 | 32 caracteres `A` con LF | un `ERR` | 36 y 37 | 3 ms |
| 5 | 33 caracteres `A` con LF | un `ERR` | 40 y 41 | 2 ms |
| 6 | `STATUS` con LF | una respuesta, t=10904515 | 44 y 45 | 3 ms |
| 7a | 33 caracteres `A` con CRLF | un `ERR` (un solo `ERR` aunque el terminador tenga dos bytes) | 48 a 50 | 2 ms |
| 7b | `STATUS` con LF | una respuesta, t=10908579 | 53 y 54 | 2 ms |

Resumen: 5 respuestas de `STATUS` (líneas 24, 29, 33, 45 y 54) y 3 `ERR` (líneas 37, 41 y 50). Las 52 líneas de telemetría del registro (t=10873000 a t=10924000) están completas y sin saltos.

## Detalle de P02

Firmware: commit `4bbb5be` con `APP_TRACE = 1` (`evidencias/P02_compilacion_traza.png`: 0 errores, 1 advertencia que avisa de la traza). Las cifras son la salida de `pruebas/herramientas/medir_tiempos.c` sobre `evidencias/P02_tiempos.log`, guardada en `evidencias/P02_intervalos.txt`. El registro dura 80,1 s (del 7 de noviembre de 2026, 20:19:42 a 20:21:02).

| Tarea | Periodo nominal | Límite de ±5 % | Líneas | Intervalos con el reloj de la placa (mínimo, medio, máximo) | Periodo medio con el reloj del PC |
| --- | --- | --- | --- | --- | --- |
| ADC (`@ACQ`) | 100 ms | 95 a 105 ms | 808 | 100, 100,00 y 100 ms | 99,31 ms (−0,694 %) |
| LED LD2 (`@LED`) | 500 ms | 475 a 525 ms | 162 | 500, 500,00 y 500 ms | 496,55 ms (−0,689 %) |
| Telemetría | 1000 ms | 950 a 1050 ms | 81 | 1000, 1000,00 y 1000 ms | 993,14 ms (−0,686 %) |

- Ningún intervalo de la placa sale de ±5 % (cero fuera de límite en las tres tareas): no falta ninguna línea de traza ni de telemetría.
- LD2 cambió 162 veces alternando 1 y 0, con cero repetidos seguidos. El nivel se lee del pin después de cambiarlo.
- Reloj de la placa contra reloj del PC, con la telemetría de la primera a la última línea: la placa avanzó 80000 ms y el PC midió 79451 ms (la placa va un 0,691 % más rápido).

## Detalle de P10

Firmware: el final, con `APP_TRACE = 0` (text 20720, data 12, bss 2588). Procedimiento: RESET de la placa (RUN, THR 70), registro con marcas de tiempo, Local echo activado y ejecución de `pruebas/herramientas/p10_carga.ttl`, que envía 30 ciclos de 5 comandos, uno cada 200 ms, y un `STATUS` final. Durante la prueba se movió el potenciómetro de un extremo al otro (PCT de 99 a 0 y de nuevo a 99). La salida de `pruebas/herramientas/analizar_p10.c` sobre el registro está en `evidencias/P10_latencias.txt`, con una fila por comando.

| Comando | Cantidad | Respuesta esperada | Latencia mínima | Latencia media | Latencia máxima |
| --- | --- | --- | --- | --- | --- |
| `STATUS` | 61 | línea de estado | 1 ms | 2,7 ms | 4 ms |
| `THR n` (n de 40 a 75) | 30 | `OK THR=n` | 1 ms | 2,6 ms | 4 ms |
| `STOP` | 15 | `OK PAUSE` | 2 ms | 2,8 ms | 4 ms |
| `START` | 15 | `OK RUN` | 2 ms | 3,1 ms | 4 ms |
| `HOLA` (línea inválida) | 30 | `ERR` | 1 ms | 2,0 ms | 6 ms |
| Total | 151 | una respuesta cada uno | 1 ms | 2,6 ms | 6 ms |

La latencia es la diferencia entre las marcas de tiempo del texto enviado (eco local) y de la respuesta, ambas del reloj del PC. Resultados del análisis:

- Comandos: 151 (150 de la carga y el `STATUS` final); respuestas: 151, ninguna de más ni de menos. El tipo de cada respuesta coincide con el comando (`OK THR=n` con el n enviado).
- Ritmo: los 150 comandos de la carga ocuparon 29,69 s, es decir 5,02 comandos por segundo.
- Latencia: mediana 3 ms, percentil 95 de 4 ms, máxima 6 ms. Ninguna respuesta pasó de 100 ms.
- Estado: en cada `STATUS`, `THR` y `MODE` coinciden con los comandos anteriores (cero diferencias).
- Coherencia: en las 147 líneas de estado (telemetría y `STATUS`), PCT coincide con el redondeo de 100 × ADC / 4095, la alarma está apagada en PAUSE y respeta la histéresis con el THR vigente (cero incumplimientos). La alarma cambió de estado varias veces durante la prueba.
- Líneas corruptas o desconocidas: cero.
- Telemetría: 86 líneas; todos los intervalos son de 1000 ms con el reloj de la placa (cero fuera de ±5 %, cero huecos). Con el reloj del PC, la placa avanzó 85000 ms mientras el PC midió 84406 ms (la placa va 0,70 % más rápido, igual que en P02).
- Los periodos de 100 ms (ADC) y 500 ms (LED) bajo carga no se midieron en P10 porque el firmware final no tiene traza; se midieron en P02. La telemetría sí se mide aquí, y es la tarea que comparte la cola de TX con las respuestas.

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
- **P05, rebotes.** Los rebotes de menos de 30 ms no se pueden generar a mano. El rebote mecánico natural de B1 en cada una de las seis pulsaciones no produjo alternancias adicionales.
- **P05, pulsación mantenida.** El registro no marca el instante en que se soltó el botón; el botón se mantuvo unos 5 s según el procedimiento. Lo que muestra el registro es que, tras la única alternancia, el modo no cambia en los 14 s siguientes (t=8146000 a t=8160000).
- **P05, potenciómetro.** En las líneas 36 y 37 de `P05_boton.log` el PCT baja de 62 a 0 porque se movió el potenciómetro; no interviene en la prueba.
- **P06, estado inicial.** El registro empieza en PAUSE, el modo que había quedado antes de abrirlo, de modo que el primer `STOP` (líneas 22 a 24) se envió estando ya en PAUSE y respondió `OK PAUSE` sin cambios. La transición de RUN a PAUSE con la alarma encendida está en el segundo `STOP` (líneas 131 a 133).
- **P06, LD2.** El parpadeo de LD2 durante la PAUSE se comprobó a simple vista; el registro solo prueba que el programa principal sigue en marcha (la telemetría continúa cada segundo).
- **P07, restauración de THR.** El último paso del procedimiento (volver a `THR 70`) no se completó: el texto se escribió dos veces antes de pulsar Enter (líneas 95 y 101), la placa recibió `THR 70THR 70` y respondió `ERR` (línea 102) sin cambiar THR, que quedó en 95 hasta el final del registro (línea 114). Ese paso no forma parte del criterio de P07.
- **P08, `HOLA` dos veces.** `HOLA` se escribió dos veces (líneas 56 y 61) y cada una recibió su propio `ERR`; por eso hay siete `ERR` para seis líneas distintas del procedimiento. La línea 55 es un Enter sin texto y no produjo respuesta.
- **P09, la línea de 32 caracteres.** Tiene el largo máximo permitido, así que la placa la acepta como línea y responde `ERR` porque no es un comando conocido (línea 37). La línea de 33 caracteres se rechaza por largo; en ambos casos el resultado visible es un solo `ERR`. Lo que prueba el paso 4 es que el límite de 32 no produce un error adicional ni descarta el `STATUS` siguiente.
- **P09, marcas de tiempo repetidas.** Las 14 líneas de telemetría de las líneas 7 a 20 tienen la misma marca (19:18:21.23x) porque Tera Term estuvo detenido unos 14 s con el cuadro de selección del macro abierto y registró los datos acumulados al cerrarlo. No se perdió ninguna línea: `t` avanza de 1000 en 1000 de la línea 1 a la 70. Las marcas del macro (desde la línea 21) son las válidas para los intervalos de la tabla.
- **P09, líneas en blanco.** Las líneas 28 y 49 aparecen solo en los pasos con CRLF y son el eco local del retorno de carro y del salto de línea enviados; en los pasos con LF no hay línea en blanco. La placa no envía líneas vacías.
- **P09, eco pegado.** En la línea 21 `STA` quedó pegado al inicio de una línea de telemetría (`STAt=10893000;...`) por el eco local de Tera Term; la telemetría llegó completa.
- **P02, qué marca cada traza.** `@ACQ` se escribe cuando `main` recoge la muestra, no cuando se inicia la conversión; la conversión termina en microsegundos, por lo que ambos instantes coinciden dentro del milisegundo. `@LED` se escribe justo después de cambiar el LED, con el nivel que quedó en el pin.
- **P02, intervalos sueltos del PC.** Con el reloj del PC, cada intervalo suelto varía más que con el de la placa: de 84 a 115 ms en `@ACQ`, de 482 a 511 ms en `@LED` y de 979 a 1006 ms en la telemetría. Es el retraso variable del USB (ST-LINK) y de Tera Term, que reparte las líneas del puerto serie en bloques. Por eso el criterio de P02 usa los intervalos del reloj de la placa y, como referencia independiente, el periodo medio del PC sobre toda la ventana, donde ese retraso se compensa.
- **P02, reloj de la placa.** El reloj de la placa va 0,69 % más rápido que el del PC. Es muy inferior a la tolerancia del enunciado (±5 %) y no se corrigió.
- **P02, inicio del registro.** El registro empieza en `t=143300`, a mitad de un segundo, porque se abrió con la placa ya en marcha. No hay reinicios ni diálogos abiertos durante la medición.
- **P02, primer intento descartado.** El primer registro (20:13) se tomó con la placa corriendo todavía el firmware sin traza, porque la versión compilada no se había cargado; solo tenía telemetría y se borró. Las mediciones válidas son las de `P02_tiempos.log`.
- **P10, primer intento descartado.** La primera ejecución dio 4,64 comandos por segundo (pausa de 190 ms en el macro, que con el tiempo de envío y la resolución del temporizador de Windows daba unos 215 ms por comando). Todo lo demás salió bien en ese intento (151 respuestas, latencia máxima de 9 ms, estado correcto, telemetría a 1000 ms), pero no llegaba a 5 comandos por segundo. Se bajó la pausa del macro a 176 ms y se repitió; el registro de esa primera ejecución no se conservó.
- **P10, marcas de tiempo repetidas.** Las 41 líneas de telemetría de las líneas 5 a 45 de `P10_carga.log` tienen la misma marca (20:43:53.54x y .56x): Tera Term estuvo detenido unos 41 s con el cuadro de selección del macro abierto y registró los datos acumulados al cerrarlo. No falta ninguna línea (`t` avanza de 1000 en 1000). Las marcas de los comandos, que son las que sirven para medir la latencia, empiezan en la línea 47.
- **P10, eco local en la telemetría.** En la línea 187 el eco de `HOLA` cayó en mitad de una línea de telemetría (`t=73000;ADC=2130;PCT=52;THR=65;MODE=PA` + `HOLA`, y el resto `USE;ALARM=0` en la línea 188). Es el eco local de Tera Term, no una línea corrupta de la placa: el analizador reunió los dos trozos y la línea completa es válida. La respuesta `ERR` llegó aparte, en la línea 189.
- **P10, fin del registro.** Después del `STATUS` final (línea 378) el registro siguió abierto unos 9 s más, con telemetría normal (hasta `t=100000`). La placa quedó en RUN con THR 70, como el último ciclo del macro.
- **P10, qué comprueba cada parte.** El macro prueba que ninguna respuesta se pierde ni se mezcla con otra línea con 5 comandos por segundo, incluidas las líneas inválidas, los cambios de modo y de umbral. Los casos extremos (cola de TX llena y pérdida de bytes de RX) no se pueden provocar con Tera Term a 5 comandos por segundo; la política que los cubre está descrita en `docs/arquitectura.md` (secciones «Política de transmisión» y «Pérdida de bytes en RX»).
