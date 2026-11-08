# Segundo parcial 2L C3-2026: monitor de señal analógica con alarma configurable

- **Estudiante:** Josué Hiraldo · **Matrícula:** 20240705
- **Placa:** STM32 NUCLEO-F446RE (microcontrolador STM32F446RE, Cortex-M4)
- **Versión final:** etiqueta `v1.0`. El firmware no cambia desde el commit `4bbb5be`; los commits posteriores son documentación, herramientas de prueba y evidencias.
- **Resultado de las pruebas:** P01 a P10 en PASS, cada una con su evidencia (`pruebas/resultados.md`).
- **Video de la demostración:** enlace en `evidencias/README.md`, sección «Video de la demostración».

Un potenciómetro representa el nivel de una variable de proceso. El firmware lo adquiere con el ADC, ejecuta tareas periódicas con un TIMER de hardware, controla dos LED y un pulsador con GPIO, y se comunica con la computadora por UART. Todo corre en un único firmware, sin RTOS y sin retardos bloqueantes.

## Funcionamiento

- Tras cada reinicio: modo RUN, umbral THR = 70 %, alarma apagada.
- Cada 100 ms se toma una muestra del ADC y se convierte a porcentaje (PCT, de 0 a 100).
- En RUN la alarma se enciende con PCT ≥ THR y se apaga con PCT < THR − 5; dentro de esa banda conserva su estado (histéresis). En PAUSE la alarma está apagada, pero el ADC, el LED de actividad, la telemetría y los comandos siguen funcionando.
- El LED de actividad (LD2) cambia de estado cada 500 ms. El LED externo muestra la alarma.
- El pulsador B1 alterna RUN y PAUSE con antirrebote de 30 ms.
- Cada 1000 ms sale por la UART una línea de telemetría. Los comandos `STATUS`, `THR n`, `START` y `STOP` consultan o cambian el estado.

## Resumen de la configuración

| Elemento | Valor |
| --- | --- |
| Reloj | HSI de 16 MHz y PLL con M = 16, N = 336, P = 4: 16 MHz / 16 × 336 / 4 = 84 MHz (HCLK); APB1 a 42 MHz (prescaler /2), APB2 a 84 MHz (prescaler /1). Valores tomados de `SystemClock_Config` en `main.c` |
| Base de tiempo | TIM6, 1 kHz (PSC = 83, ARR = 999, reloj de TIM6 de 84 MHz) |
| Adquisición | ADC1, canal IN0 (PA0), 12 bits, VREF+ = 3,30 V medido, una muestra cada 100 ms |
| LED de actividad | LD2 (PA5), cambia cada 500 ms |
| LED de alarma | LED externo en PA6 con resistencia de 1 kΩ |
| Pulsador | B1 (PC13), activo en bajo, antirrebote de 30 ms |
| UART | USART2 (PA2/PA3, puente ST-LINK), 115200 baudios, 8 bits, sin paridad, 1 bit de parada |
| Telemetría | una línea cada 1000 ms |

## Mapa de pines

| Función | Pin | Conector | Modo y nivel activo |
| --- | --- | --- | --- |
| Potenciómetro (cursor) | PA0 | CN8, pin 1 (A0) | Entrada analógica, ADC1 canal IN0 |
| LED de actividad | PA5 | LD2 integrado | Salida, activo en alto |
| LED de alarma | PA6 | CN5, pin 5 (D12), con 1 kΩ | Salida, activo en alto |
| Pulsador | PC13 | B1 integrado | Entrada con pull-up, activo en bajo (reposo = 1) |
| UART TX | PA2 | Puente ST-LINK (USB) | USART2 |
| UART RX | PA3 | Puente ST-LINK (USB) | USART2 |

## Esquema de conexiones

```
 Potenciómetro 10 kΩ                       LED de alarma externo

 3V3 (CN6, pin 4) ──────┐                  PA6 (CN5, pin 5, D12)
                        │                       │
                      ┌─┴─┐                  ┌──┴──┐
                      │   │                  │ 1 kΩ│
 PA0 / A0 ◄───────────┤ ◄─┤ cursor           └──┬──┘
 (CN8, pin 1)         │   │                     │
                      └─┬─┘                  ▲  │  ánodo (pata larga)
                        │                    │  │
 GND (CN6, pin 6) ──────┘                   ─┴─ │  cátodo
                                             │
                                  GND (CN5, pin 7) ┘

 LD2 (PA5) y B1 (PC13) son de la placa; no llevan cableado.
 UART: cable USB de la placa (ST-LINK) hacia la computadora; aparece como un puerto COM.
```

- El potenciómetro se alimenta con 3V3, nunca con 5 V: la entrada PA0 admite como máximo VREF+.
- El puente PA6 → PA0 del primer parcial está retirado.
- El LED externo siempre lleva su resistencia de 1 kΩ.

## VREF y resolución del ADC

| Parámetro | Valor |
| --- | --- |
| Convertidor y canal | ADC1, canal IN0 (PA0) |
| Resolución | 12 bits: datos de 0 a 4095 (2^12 − 1) |
| VREF+ | 3,30 V, medido con multímetro entre 3V3 y GND el 5 de noviembre de 2026 (`evidencias/vref_multimetro.jpg`) |
| LSB | 3,30 V / 4095 ≈ 0,806 mV |
| Reloj del ADC | PCLK2 / 4 = 21 MHz; tiempo de muestreo de 144 ciclos |
| Disparo y fin | Inicio por software cada 100 ms; fin de conversión por interrupción, que solo guarda el dato |

**Conversión a porcentaje.** Solo con enteros, en 32 bits para evitar desbordamiento:

```
PCT = (100 × ADC + 2047) / 4095        (división entera; 4095 es impar y no hay empates .5)
```

La macro está en `app_config.h` (`APP_PCT_FROM_ADC`) y se verificó para los 4096 valores posibles en `pruebas/herramientas/verificar_pct.c`.

## Cálculo del TIMER

Base de tiempo: **TIM6**, temporizador básico de 16 bits. Con HCLK = 84 MHz y APB1 con prescaler /2, el reloj de TIM6 es 84 MHz (los temporizadores de APB1 reciben el doble de PCLK1).

```
f_cuenta = 84 MHz / (PSC + 1) = 84 MHz / 84 = 1 MHz          con PSC = 83
f_tick   = f_cuenta / (ARR + 1) = 1 MHz / 1000 = 1 kHz        con ARR = 999
T_tick   = 1 ms
```

PSC y ARR caben en 16 bits y dan exactamente 1000 Hz. La interrupción de TIM6 solo hace `ms++`. El programa principal compara el contador con el instante programado de cada tarea y lo avanza sumando el período, por lo que un retraso ocasional no se acumula y la resta sin signo tolera el desbordamiento del contador:

```
if ((uint32_t)(ahora - ultimo) >= periodo) { ultimo += periodo; /* ejecutar la tarea */ }
```

| Tarea | Período | Ticks |
| --- | --- | --- |
| Adquisición del ADC | 100 ms | 100 |
| LED de actividad | 500 ms | 500 |
| Telemetría | 1000 ms | 1000 |
| Muestreo del pulsador | 1 ms | 1 |

El detalle está en `docs/temporizacion.md`.

## Protocolo UART

Líneas ASCII terminadas en LF o CRLF, sensibles a mayúsculas, de hasta 32 caracteres sin contar el terminador. Cada respuesta termina en CRLF.

| Línea recibida | Respuesta |
| --- | --- |
| `STATUS` | una línea con el formato de la telemetría |
| `THR n` (n de 10 a 95, un espacio) | `OK THR=n` |
| `START` | `OK RUN` |
| `STOP` | `OK PAUSE` |
| cualquier otra, o de más de 32 caracteres | `ERR`, sin cambiar ningún parámetro |
| línea vacía | se ignora |

Telemetría y `STATUS`: `t=1200;ADC=2866;PCT=70;THR=70;MODE=RUN;ALARM=1`. `t` son los milisegundos desde el reinicio.

La recepción es por interrupción a un búfer circular de 128 bytes; las líneas se arman y se ejecutan fuera de la interrupción. La transmisión usa una cola de 512 bytes que solo admite líneas completas, de modo que una respuesta nunca se mezcla con la telemetría. Detalle en `docs/arquitectura.md`.

## Estructura del repositorio

| Carpeta o archivo | Contenido |
| --- | --- |
| `docs/requisitos.md` | Requisitos del mandato con su prueba y su criterio de la rúbrica |
| `docs/plan_de_trabajo.md` | Hitos y criterio de commits |
| `docs/hardware.md` | Mapa de pines, conexiones, VREF, resolución y canal del ADC |
| `docs/temporizacion.md` | Cálculo del reloj, el prescaler y el período del TIMER |
| `docs/cubemx.md` | Configuración de CubeMX periférico por periférico |
| `docs/arquitectura.md` | Tareas, máquina de estados, alarma con histéresis, protocolo UART y traza de tiempos |
| `Parcial2_20240705/` | Proyecto de STM32CubeIDE: `Inc/`, `Src/`, `Startup/`, `Drivers/` y el archivo `.ioc` de CubeMX |
| `pruebas/` | Plan de pruebas, resultados P01 a P10 y herramientas (macros de Tera Term y programas en C) |
| `evidencias/` | Capturas, registros de la terminal y enlace al video |

Módulos del firmware (`Parcial2_20240705/Src`): `tick` (TIM6), `sched` (plazos sin deriva), `button` (antirrebote), `alarm` (histéresis y modos), `adc_in` (ADC por interrupción), `ringbuf`, `console` (USART2), `line` (armado de líneas), `cmd` (comandos) y `app` (`App_Init` y `App_Loop`).

## Herramientas

STM32CubeIDE 2.2.0, STM32CubeMX 6.18.1, paquete de firmware STM32Cube FW_F4 V1.28.3 (HAL), compilador arm-none-eabi-gcc incluido en CubeIDE, Tera Term (puerto COM del ST-LINK), multímetro para VREF y la tensión del potenciómetro, Windows con PowerShell 5.1.

## Compilar y ejecutar

1. Clonar el repositorio.
2. En STM32CubeIDE 2.2.0: **File → Import → General → Existing Projects into Workspace**. Como directorio raíz, la carpeta `Parcial2_20240705` del repositorio, con *Copy projects into workspace* sin marcar.
3. **Project → Build Project**. La configuración Debug compila con 0 errores y 0 advertencias (text 20720, data 12, bss 2588).
4. Conectar la NUCLEO por USB y cargar el programa con **Run**.
5. Abrir Tera Term sobre el puerto COM del ST-LINK: 115200 baudios, 8 bits, sin paridad, 1 bit de parada, sin control de flujo. En *Setup → Terminal*, **New-line Transmit = LF** (o CR+LF); con el valor por defecto (CR) la línea nunca termina y el firmware no recibe el comando.

## Reproducir las pruebas

Los registros se toman con Tera Term (**Archivo → Log...**, con *Timestamp* en hora local y *Plain text*). Los procedimientos están en `pruebas/plan_de_pruebas.md`.

| Herramienta | Uso |
| --- | --- |
| `pruebas/herramientas/p01_status_rafaga.ttl` | Macro de Tera Term: `STATUS` cada 100 ms (P01) |
| `pruebas/herramientas/p09_fragmentos.ttl` | Macro: `STATUS` fragmentado, LF, CRLF y líneas de 32 y 33 caracteres (P09) |
| `pruebas/herramientas/p10_carga.ttl` | Macro: 150 comandos a 5 por segundo con modo y umbral cambiando (P10) |
| `pruebas/herramientas/medir_tiempos.c` | Analiza el registro de P02 (intervalos de ADC, LED y telemetría) |
| `pruebas/herramientas/analizar_p10.c` | Analiza el registro de P10 (respuestas, latencia, estado, líneas corruptas) |
| `pruebas/herramientas/verificar_pct.c` | Verifica la fórmula de PCT para los 4096 valores del ADC |

Los programas en C se compilan en el PC con cualquier compilador, por ejemplo `gcc -std=c11 -O2 -o medir_tiempos medir_tiempos.c -lm`.

**Traza de tiempos (P02).** El firmware final no escribe nada además de la telemetría y las respuestas. Para medir los períodos de 100 ms y 500 ms, `app_config.h` define `APP_TRACE`: con valor 1 se escriben líneas `@ACQ;t=<ms>` y `@LED;t=<ms>;LD2=<n>`. Con `APP_TRACE = 0` esas líneas no se compilan y el programa tiene el mismo tamaño. P02 se midió con `APP_TRACE = 1`; todas las demás pruebas, con `APP_TRACE = 0`.

## Resultados

Resumen de `pruebas/resultados.md`, donde están los valores completos, las tablas de detalle y las observaciones. Las diez pruebas se ejecutaron el 7 de noviembre de 2026.

| ID | Prueba | Resultado | Evidencia |
| --- | --- | --- | --- |
| P01 | Compila y arranca | PASS: 0 errores y 0 advertencias; primera respuesta a `STATUS` en t = 36, 102 y 117 ms; RUN, THR 70, ALARM 0 | `evidencias/P01_arranque.log`, `P01_compilacion.png` |
| P02 | Tiempos de ADC, LED y telemetría | PASS: intervalos de 100, 500 y 1000 ms exactos con el reloj de la placa; periodos medios con el reloj del PC a 0,69 % de los nominales | `evidencias/P02_tiempos.log`, `P02_intervalos.txt` |
| P03 | Porcentaje contra el voltaje medido | PASS: 0,00, 1,66 y 3,30 V dan PCT 0, 50 y 100; diferencia máxima 0,3 puntos | `evidencias/P03_niveles.log` y fotos del multímetro |
| P04 | Histéresis de la alarma | PASS: ALARM 0, 1, 1, 0 en los cuatro niveles; ningún incumplimiento en 122 registros | `evidencias/P04_histeresis.log` |
| P05 | Pulsador | PASS: una alternancia por pulsación; pulsación mantenida 5 s, una sola alternancia | `evidencias/P05_boton.log`, `P05_boton_mantenido.log` |
| P06 | PAUSE y reanudación | PASS: en PAUSE la alarma queda apagada con PCT hasta 99 y las demás tareas siguen; `START` restaura RUN | `evidencias/P06_pause_run.log` |
| P07 | Umbral con `THR n` | PASS: `THR 50`, `80`, `10` y `95` responden `OK THR=n` y cambian la alarma como corresponde | `evidencias/P07_umbral.log` |
| P08 | Líneas inválidas | PASS: siete líneas inválidas, siete `ERR`, ningún cambio de THR ni de modo | `evidencias/P08_errores.log` |
| P09 | Fragmentos, terminadores y línea larga | PASS: 5 respuestas de `STATUS` y 3 `ERR`, ninguna de más | `evidencias/P09_fragmentos.log` |
| P10 | Carga de 5 comandos por segundo | PASS: 151 comandos, 151 respuestas, latencia máxima de 6 ms, cero líneas corruptas, telemetría a 1000 ms | `evidencias/P10_carga.log`, `P10_latencias.txt` |

## Limitaciones conocidas

- **Reloj interno.** El sistema usa el oscilador interno HSI, de precisión cercana a ±1 %. En los registros el reloj de la placa avanza un 0,69 % más rápido que el del PC (P02) y un 0,70 % en P10. Está muy dentro de la tolerancia de ±5 % y no se corrigió.
- **Pérdida de bytes de recepción.** Si el búfer de RX se desborda o el USART señala un error, la línea en curso termina con un solo `ERR` y no se ejecuta a medias. Si el búfer tenía bytes de líneas anteriores sin procesar, ese `ERR` puede caer en otra línea distinta de la que perdió el byte. En P10, con 5 comandos por segundo, no hubo líneas corruptas ni respuestas perdidas (`docs/arquitectura.md`).
- **Telemetría con la cola llena.** Si la cola de TX no tiene sitio para una línea completa y una respuesta, la telemetría de ese segundo se omite en lugar de mezclarse con otra línea. No se observó en P02 ni en P10: todos los intervalos de telemetría fueron de 1000 ms.
- **Formato de `THR n`.** Exige un solo espacio y de uno a tres dígitos; admite ceros a la izquierda (`THR 050` equivale a 50). Cualquier otra forma responde `ERR`.
- **Eco local de Tera Term.** Con *Local echo* activado, el texto enviado puede aparecer pegado dentro de una línea de telemetría en el registro; es la pantalla de Tera Term, no la placa. El analizador de P10 reconstruye esas líneas.
- **Marcas de tiempo del PC.** Dependen del USB y de Tera Term: intervalos sueltos varían varios milisegundos y un cuadro de diálogo abierto durante el registro hace que varias líneas lleguen con la misma hora. Por eso P02 mide con el reloj de la placa, y las latencias de P10 son de eco a respuesta con el reloj del PC (incluyen el tránsito por el USB).
- **P02 con una versión de diagnóstico.** Los períodos de 100 ms y 500 ms se midieron con `APP_TRACE = 1`, que no es la versión final; con `APP_TRACE = 0` el código generado es el mismo, comprobado por el tamaño del programa. Bajo la carga de P10 solo se midió la telemetría.
- **Rebotes del pulsador.** Los rebotes de menos de 30 ms no se pueden generar a mano; en P05 se comprobó que el rebote natural de B1 no produce alternancias adicionales, y que mantener el botón produce una sola. El registro no marca el instante en que se soltó el botón.
- **Casos extremos de UART.** La cola de TX llena y la pérdida de bytes de RX no se pueden provocar con Tera Term a 5 comandos por segundo; la política que los cubre está en `docs/arquitectura.md`.
