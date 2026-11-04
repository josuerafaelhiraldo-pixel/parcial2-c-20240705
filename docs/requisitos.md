# Requisitos del mandato

Cada requisito tiene un identificador (RQ) para citarlo en el código, en las pruebas y en los commits. La columna «Mandato» indica el numeral del enunciado; «Prueba» indica con qué prueba se verifica (ver `pruebas/plan_de_pruebas.md`).

## Condiciones iniciales

| ID | Requisito | Mandato | Prueba | Criterio |
| --- | --- | --- | --- | --- |
| RQ-01 | Tras cada reinicio: modo RUN, THR = 70 %, alarma apagada y contador de tiempo en cero. La alarma se evalúa tras la primera muestra válida. No se guardan parámetros en memoria no volátil. | Condiciones iniciales | P01 | Integración |

## Adquisición y temporización

| ID | Requisito | Mandato | Prueba | Criterio |
| --- | --- | --- | --- | --- |
| RQ-02 | Un TIMER de hardware es la base temporal: muestra del ADC cada 100 ms, LED de actividad cada 500 ms y telemetría cada 1000 ms. Se justifica el cálculo de reloj, prescaler y período. | 1 | P02 | TIMER |
| RQ-03 | PCT = redondear(100 × ADC / (2^N − 1)), entero de 0 a 100, con tipos que eviten desbordamiento. Se declaran N, VREF y el canal en el README. | 2 | P03 | ADC |
| RQ-04 | Sin retardos bloqueantes para programar tareas ni eliminar rebotes. Interrupciones breves; las cadenas UART se procesan fuera de ellas. El periférico TIMER debe estar identificado y configurado (no basta una función de temporización preexistente). | 3 | P10 | TIMER, Integración |
| RQ-05 | ADC, referencia y canal documentados y configurados; lectura válida en todo el rango. | Recursos y montaje, 2 | P03 | ADC |

## GPIO y lógica de alarma

| ID | Requisito | Mandato | Prueba | Criterio |
| --- | --- | --- | --- | --- |
| RQ-06 | Una pulsación válida alterna RUN y PAUSE. Nivel estable de al menos 30 ms, una sola acción por pulsación, sin repetición al mantener el botón. La liberación también se valida antes de aceptar otra pulsación. | 4 | P05 | GPIO |
| RQ-07 | En RUN la alarma se enciende con PCT ≥ THR y se apaga con PCT < THR − 5; dentro de la banda conserva su estado. En PAUSE la alarma se fuerza apagada, pero siguen el ADC, el LED de actividad, la telemetría y los comandos. | 5 | P04, P06 | Integración |
| RQ-08 | Al pasar de PAUSE a RUN o al cambiar THR en RUN, la decisión de alarma se reinicia con la última muestra (encendida si PCT ≥ THR). Desde la siguiente muestra se aplica la histéresis. Sin muestra válida, se espera por ella. Cambiar THR en PAUSE mantiene la alarma apagada. | 6 | P06, P07 | Integración |

## Protocolo UART

| ID | Requisito | Mandato | Prueba | Criterio |
| --- | --- | --- | --- | --- |
| RQ-09 | UART a 115200 baudios, 8 bits, sin paridad, 1 bit de parada. | Recursos y montaje | P01 | UART |
| RQ-10 | Líneas ASCII terminadas en LF o CRLF, sensibles a mayúsculas, de hasta 32 caracteres sin contar el terminador. Línea vacía: se ignora. Línea demasiado larga: se descarta completa hasta LF, responde ERR y se procesa la siguiente. Un comando inválido no cambia ningún parámetro. | 7 | P08, P09 | UART |
| RQ-11 | `STATUS` responde de inmediato con el formato de la telemetría. | 7 | P09, P10 | UART |
| RQ-12 | `THR n` acepta un entero decimal de 10 a 95 y responde `OK THR=n`. Rechaza texto adicional, decimales y valores fuera de rango con `ERR`. | 7 | P07, P08 | UART |
| RQ-13 | `START` establece RUN y responde `OK RUN`; si ya está en RUN conserva la decisión de alarma. `STOP` establece PAUSE, apaga la alarma y responde `OK PAUSE`. Cualquier otro comando responde `ERR` sin detener las tareas. | 7 | P06, P08 | UART |
| RQ-14 | Cada respuesta termina en CRLF y se completa dentro de 100 ms desde el LF. Telemetría y STATUS: `t=1200;ADC=2866;PCT=70;THR=70;MODE=RUN;ALARM=1`. Las respuestas y la telemetría no se mezclan en una misma línea. | 8 | P09, P10 | UART, Integración |

## Entrega

| ID | Requisito | Mandato | Criterio |
| --- | --- | --- | --- |
| RQ-15 | Repositorio GitHub con el proyecto completo (código, configuración del IDE, archivos de compilación, instrucciones). | Entregables 1 | Entrega |
| RQ-16 | README con nombre y matrícula, placa y microcontrolador, herramientas y versiones, mapa de pines, esquema de conexiones, VREF, resolución del ADC, configuración de la UART, cálculo del TIMER e instrucciones. | Entregables 2 | Entrega |
| RQ-17 | Resultados P01 a P10 con PASS o FAIL, valores medidos y evidencias; video breve o capturas y trazas suficientes. | Entregables 3 | Entrega |
| RQ-18 | Historial de commits con avance progresivo y versión final identificada por hash o etiqueta. | Entregables 4 | Entrega |
| RQ-19 | Explicar el TIMER, la conversión del ADC, el antirrebote y la recepción UART, e identificar el código de cada función. | Condiciones de evaluación | Entrega |

## Rúbrica y dónde se cubre

| Criterio | Máximo | Requisitos | Pruebas |
| --- | --- | --- | --- |
| GPIO | 4 | RQ-06 (y la configuración eléctrica de `docs/hardware.md`) | P05 |
| TIMER | 4 | RQ-02, RQ-04 | P02, P10 |
| ADC | 4 | RQ-03, RQ-05 | P03, P04 |
| UART | 5 | RQ-09 a RQ-14 | P01, P08, P09 |
| Integración | 4 | RQ-01, RQ-07, RQ-08, RQ-04 | P04, P06, P07, P10 |
| Entrega y verificación | 4 | RQ-15 a RQ-19 | todas |
| **Total** | **25** | | |

El enunciado indica que no se asigna crédito por declarar que una función existe: cada subcriterio necesita evidencia. Por eso `pruebas/resultados.md` no marca PASS ninguna prueba que no tenga su archivo de evidencia.
