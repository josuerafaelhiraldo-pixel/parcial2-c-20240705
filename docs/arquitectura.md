# Arquitectura

Programa sin RTOS y sin retardos bloqueantes. Las interrupciones solo anotan datos; `main` hace todo el trabajo con comparaciones de tiempo.

## Flujo de datos

| Evento de hardware | Interrupción (solo anota) | Dato compartido | Lo procesa `main` |
| --- | --- | --- | --- |
| TIM6, 1 kHz | `HAL_TIM_PeriodElapsedCallback`: `s_ms++` | Contador de milisegundos | Planificador: ADC, LED y telemetría; antirrebote del botón |
| Fin de conversión del ADC | `HAL_ADC_ConvCpltCallback`: guarda el dato y levanta una bandera | Último valor y bandera de muestra nueva | Conversión a porcentaje y decisión de alarma |
| Byte recibido por USART2 | `HAL_UART_RxCpltCallback`: byte al búfer circular de RX y rearma la recepción | Búfer circular de RX | Armado de líneas y análisis de comandos |
| Byte enviado por USART2 | `HAL_UART_TxCpltCallback`: inicia el siguiente tramo | Cola circular de TX | Las respuestas y la telemetría se encolan completas |

## Módulos previstos

| Módulo | Responsabilidad |
| --- | --- |
| `app_config.h` | Pines, polaridades, períodos, límites y tamaños de búfer |
| `tick` | Contador de milisegundos con TIM6 |
| `sched` (o dentro de `app`) | Instantes de las tareas de 100 ms, 500 ms y 1000 ms, sin deriva |
| `button` | Antirrebote de 30 ms, una acción por pulsación y validación de la liberación |
| `adc_in` | Disparo de la conversión y lectura del resultado |
| `alarm` | Histéresis, modos RUN / PAUSE y reglas de reanudación |
| `ringbuf` | Búfer circular de un productor y un consumidor |
| `line` | Armado de líneas (LF / CRLF, 32 caracteres, descarte de líneas largas) |
| `cmd` | Análisis de `STATUS`, `THR n`, `START` y `STOP` |
| `console` | UART no bloqueante: RX por interrupción y cola de TX |
| `app` | `App_Init` y `App_Loop`: une todo |

El búfer circular, el armado de líneas y la consola siguen el diseño del primer parcial; se adaptan a este protocolo y se revisan antes de reutilizarlos.

## Tareas periódicas (RQ-02)

| Tarea | Período | Acción |
| --- | --- | --- |
| Adquisición | 100 ms | Inicia la conversión del ADC con interrupción |
| LED de actividad | 500 ms | Cambia el estado de LD2 |
| Telemetría | 1000 ms | Encola la línea de estado |
| Antirrebote | 1 ms | Muestrea PC13 y avanza el filtro |

## Máquina de estados y alarma (RQ-06 a RQ-08)

Estados: RUN y PAUSE. Se pasa de uno a otro con una pulsación válida del botón o con `START` y `STOP`. La alarma es una salida de un solo bit con las reglas siguientes. Si todavía no hay muestra válida, no se decide nada.

| Situación | Acción sobre la alarma |
| --- | --- |
| Llega una muestra en RUN con PCT ≥ THR | Encendida |
| Llega una muestra en RUN con PCT < THR − 5 | Apagada |
| Llega una muestra en RUN con THR − 5 ≤ PCT < THR | Conserva su estado (banda de histéresis) |
| Muestra en PAUSE | Apagada |
| `STOP` o pulsación que pasa a PAUSE | Se apaga al instante |
| Paso de PAUSE a RUN | Reevalúa con la última muestra: encendida si PCT ≥ THR; si no, apagada |
| `THR n` válido en RUN | Reevalúa con la última muestra, igual que arriba |
| `THR n` válido en PAUSE | Guarda THR; la alarma sigue apagada |
| `START` estando en RUN | No cambia nada; conserva la decisión actual |
| Primera muestra tras el reinicio | Reevalúa: encendida si PCT ≥ THR |

Ejemplo de P04 con THR = 70 (apaga por debajo de 65), partiendo de alarma apagada:

| PCT | Decisión | Alarma |
| --- | --- | --- |
| 60 | 60 < 65 | 0 |
| 72 | 72 ≥ 70 | 1 |
| 67 | banda 65 a 69 | 1 (conserva) |
| 62 | 62 < 65 | 0 |

THR mínimo es 10, por lo que THR − 5 nunca es negativo.

## Protocolo UART (RQ-09 a RQ-14)

Entrada: líneas ASCII terminadas en LF o CRLF; sensibles a mayúsculas; hasta 32 caracteres sin contar el terminador. Salida: cada línea termina en CRLF.

| Entrada | Respuesta | Efecto |
| --- | --- | --- |
| `STATUS` | `t=...;ADC=...;PCT=...;THR=...;MODE=...;ALARM=...` | Ninguno |
| `THR n` con n entre 10 y 95 | `OK THR=n` | Cambia THR y reevalúa según la tabla de arriba |
| `START` | `OK RUN` | Establece RUN |
| `STOP` | `OK PAUSE` | Establece PAUSE y apaga la alarma |
| Cualquier otra línea no vacía | `ERR` | Ninguno |
| Línea vacía | (nada) | Ninguno |
| Más de 32 caracteres | `ERR` al llegar el LF | Se descarta toda la línea |

Decisiones de análisis, para que el comportamiento sea determinista:

1. Se aceptan exactamente los comandos del mandato. Cualquier carácter extra (espacios al final, mayúsculas distintas, texto adicional) da `ERR`.
2. `THR` lleva un solo espacio y luego solo dígitos decimales (de 1 a 3). Con signo, punto decimal, letras, texto adicional o sin argumento da `ERR`. Se rechazan más de 3 dígitos para no desbordar el cálculo. Después del análisis el valor debe estar entre 10 y 95.
3. El terminador es LF; un CR justo antes del LF se descarta (CRLF) y no cuenta en el largo de la línea. Un CR en cualquier otro lugar invalida la línea (`ERR`).
4. Un carácter no imprimible marca la línea como inválida; responde `ERR` al llegar el LF.
5. Una línea larga se descarta completa hasta el LF, responde un solo `ERR` y la siguiente se procesa con normalidad.
6. Un comando inválido nunca cambia THR, el modo ni la alarma.

## Política de transmisión (RQ-14)

- Cada respuesta se encola como una línea completa. Si no cabe, el programa principal no lee el comando siguiente hasta poder encolarla; así no se pierden respuestas.
- La telemetría se encola solo si cabe entera. Una línea de telemetría nunca se mezcla con otra, porque nunca se encolan líneas a medias.
- Las respuestas tienen prioridad sobre la telemetría.

## Decisiones abiertas

| Tema | Opciones | Se decide en |
| --- | --- | --- |
| Verificación de tiempos de P02 | Líneas de traza (`@ACQ`, `@LED`) con una bandera de compilación, activada solo durante la medición; o un pin de diagnóstico | Hito 4 |
| Tamaño de los búferes | RX de 128 bytes y TX de 512 como punto de partida | Hito 3 |
| Reutilización del código del primer parcial | `ringbuf`, `line` y `console` adaptados | Hito 3 |
