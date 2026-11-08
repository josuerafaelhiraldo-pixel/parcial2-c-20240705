# Evidencias

Aquí se guardan las capturas, los registros de la terminal y las fotos de las mediciones de las pruebas P01 a P10.

## Convención de nombres

- Registros de Tera Term: `Pxx_descripcion.log` (por ejemplo `P04_histeresis.log`). No se editan: son la evidencia original.
- Capturas y fotos: `Pxx_descripcion.png` o `.jpg`.
- Análisis de tiempos: `Pxx_descripcion.txt`.

Cada fila de `pruebas/resultados.md` cita el archivo que la respalda.

## Archivos del montaje y del Hito 1

| Archivo | Contenido |
| --- | --- |
| `montaje_cableado.jpg` | Foto del montaje: potenciómetro en PA0, LED de alarma con 1 kΩ en PA6 y la placa por USB |
| `vref_multimetro.jpg` | Medición de VREF con el multímetro entre 3V3 y GND: 3,30 V |
| `P05_boton_depurador_1_pausa.png` | Vista Live Expressions con `s_alarmMode` = `ALARM_MODE_PAUSE` tras la primera pulsación |
| `P05_boton_depurador_2_run.png` | La misma vista con `ALARM_MODE_RUN` tras la segunda pulsación |

## Archivos del Hito 2

| Archivo | Contenido |
| --- | --- |
| `P03_depurador_0pct.png` | Live Expressions con el potenciómetro en un extremo: `s_raw` = 2, `s_pct` = 0 |
| `P03_depurador_50pct.png` | En el centro: `s_raw` = 2048, `s_pct` = 50 |
| `P03_depurador_100pct.png` | En el otro extremo: `s_raw` = 4094, `s_pct` = 100 |
| `P04_depurador_1_pct60.png` | THR = 70, `s_pct` = 60: `s_alarmOn` = 0 |
| `P04_depurador_2_pct72.png` | `s_pct` = 72: `s_alarmOn` = 1 |
| `P04_depurador_3_pct67.png` | `s_pct` = 67, viniendo desde arriba: `s_alarmOn` sigue en 1 (banda de histéresis) |
| `P04_depurador_4_pct62.png` | `s_pct` = 62: `s_alarmOn` = 0 |
| `P04_led_alarma_encendido.jpg` | Foto del LED externo de alarma encendido |
| `P04_led_alarma_apagado.jpg` | Foto del LED externo de alarma apagado |
| `P05_boton_con_adc_1_pausa.png` | B1 pasa a PAUSE con el ADC activo (`s_raw` sigue cambiando) |
| `P05_boton_con_adc_2_run.png` | B1 vuelve a RUN |

## Archivos del Hito 3

| Archivo | Contenido |
| --- | --- |
| `P01_enlace_uart_eco.png` | Enlace UART a 115200 8N1 con el firmware de eco: el texto escrito vuelve desde la placa (Local echo apagado en Tera Term) |
| `P09_prueba_rapida_comandos.png` | Telemetría cada segundo, `STATUS`, `THR 50` con `OK THR=50`, `STOP` con `OK PAUSE` y `START` con `OK RUN` |
| `P09_prueba_rapida_errores_1.png` | `THR 9`, `THR 96`, `THR abc`, `THR 50x`, `THR 50 extra` y `status`: un `ERR` cada uno, THR sigue en 50 |
| `P09_prueba_rapida_errores_2.png` | `HOLA` y una línea larga: un `ERR` cada una, y el `STATUS` posterior responde |

Con Local echo activado, el texto escrito puede aparecer pegado a una línea de telemetría. Es el eco local de Tera Term, no una mezcla de líneas de la placa.

## Pruebas formales (Hito 4)

| Archivo | Contenido |
| --- | --- |
| `P01_compilacion.png` | Consola de CubeIDE: `Build Finished. 0 errors, 0 warnings` y tamaños del programa |
| `P01_arranque.log` | Registro con marcas de tiempo: `STATUS` cada 100 ms (macro `pruebas/herramientas/p01_status_rafaga.ttl`) y tres reinicios con RESET; tras cada uno la primera respuesta llega con t menor de 200 ms |
| `P02_compilacion_traza.png` | Consola de CubeIDE con `APP_TRACE = 1`: 0 errores, 1 advertencia (`APP_TRACE activo`) y tamaños del programa con la traza (text 21252, data 12, bss 2596) |
| `P02_tiempos.log` | Registro con marcas de tiempo de 80 s con la traza activa: líneas `@ACQ` (cada 100 ms), `@LED` (cada 500 ms) y telemetría (cada 1000 ms) |
| `P02_intervalos.txt` | Salida de `pruebas/herramientas/medir_tiempos.c` sobre `P02_tiempos.log`: intervalos con el reloj de la placa y del PC, alternancia de LD2 y resultado |
| `P03_niveles.log` | Registro con marcas de tiempo de los tres niveles del potenciómetro (mínimo, centro y máximo) con las respuestas de `STATUS` de cada uno |
| `P03_multimetro_0pct.jpg` | Multímetro entre A0 y GND con el potenciómetro en el mínimo: 0,00 V |
| `P03_multimetro_50pct.jpg` | Multímetro con el potenciómetro en el centro: 1,66 V |
| `P03_multimetro_100pct.jpg` | Multímetro con el potenciómetro en el máximo: 3,30 V |
| `P04_histeresis.log` | Registro con marcas de tiempo de la histéresis con THR = 70: respuestas de `STATUS` con PCT 61, 74, 67 y 62 y la telemetría de las subidas y bajadas por la banda de 65 a 69 |
| `P05_boton.log` | Registro con marcas de tiempo de cinco pulsaciones separadas de B1: la telemetría y `STATUS` muestran PAUSE, RUN, PAUSE, RUN, PAUSE |
| `P05_boton_mantenido.log` | Registro de una pulsación mantenida unos 5 s: una sola alternancia de PAUSE a RUN y ninguna más en los 14 s siguientes |
| `P06_pause_run.log` | Registro con marcas de tiempo de `STOP` y `START` con PCT alto y bajo: en PAUSE la alarma queda apagada con PCT hasta 99, el ADC y la telemetría continúan, y `START` restaura RUN y decide la alarma con la última muestra |
| `P07_umbral.log` | Registro con marcas de tiempo de los cambios de umbral `THR 50`, `THR 80`, `THR 10` y `THR 95` con PCT 60: respuesta `OK THR=n`, telemetría y `STATUS` posteriores con el THR nuevo y la alarma esperada |
| `P08_errores.log` | Registro con marcas de tiempo de las líneas inválidas `THR 9`, `THR 96`, `THR abc`, `THR 50x`, `THR 50 extra` y `HOLA`: un `ERR` por línea, con THR 70 y MODE RUN sin cambios en toda la telemetría |
| `P09_fragmentos.log` | Registro con marcas de tiempo del macro `pruebas/herramientas/p09_fragmentos.ttl`: `STATUS` fragmentado, terminadores CRLF y LF, líneas de 32 y 33 caracteres. Resultado: 5 respuestas de `STATUS` y 3 `ERR`, sin respuestas de más |

## Video de la demostración

(Se agrega el enlace al cierre del proyecto.)
