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

## Video de la demostración

(Se agrega el enlace al cierre del proyecto.)
