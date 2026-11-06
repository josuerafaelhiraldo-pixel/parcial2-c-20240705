# Plan de trabajo

Apertura: lunes 2 de noviembre de 2026. Cierre: domingo 8 de noviembre de 2026, 23:50. Tiempo sugerido: 4 horas de trabajo efectivo. Cada hito termina con commits que dejan el proyecto compilando.

## Hito 0: organización (4 de noviembre)

- [x] Estructura del repositorio, `.gitignore` y `.gitattributes`.
- [x] Requisitos con trazabilidad a pruebas y rúbrica.
- [x] Hardware: pines, conexiones, VREF, resolución y canal del ADC.
- [x] Cálculo del TIMER y plan de tareas periódicas.
- [x] Configuración de CubeMX y arquitectura.
- [x] Plan de pruebas y plantilla de resultados P01 a P10.
- [x] Verificación de la fórmula de porcentaje para los 4096 valores del ADC.
- [x] Proyecto de CubeIDE generado y compilando.
- [x] `app_config.h` con las constantes del mandato y enlace `App_Init` / `App_Loop`.

## Hito 1: base de tiempo y GPIO

- [x] TIM6 a 1 kHz con contador de milisegundos y planificador de tareas sin deriva.
- [x] LED de actividad cada 500 ms.
- [x] Pulsador con antirrebote de 30 ms, una acción por pulsación y validación de la liberación.
- [x] Prueba parcial: P02 (LED, a simple vista) y P05 (pulsador, con el depurador). La medición con tiempos queda para la terminal serie.

## Hito 2: ADC y alarma

- [ ] Adquisición cada 100 ms con ADC1 y conversión a porcentaje.
- [ ] Alarma con histéresis y modos RUN / PAUSE.
- [ ] Reglas al reanudar y al cambiar THR.
- [ ] Prueba parcial: P03, P04.

## Hito 3: UART

- [ ] Recepción por interrupción con búfer circular y armado de líneas (LF / CRLF, 32 caracteres).
- [ ] Cola de transmisión por líneas completas.
- [ ] Comandos `STATUS`, `THR n`, `START`, `STOP` y `ERR`.
- [ ] Telemetría cada 1000 ms.
- [ ] Prueba parcial: P01, P06 a P09.

## Hito 4: integración y pruebas

- [ ] Carga: P10 (5 comandos por segundo durante 30 s).
- [ ] Ejecución completa de P01 a P10 con registros y capturas.
- [ ] Corrección de los FAIL o explicación documentada.

## Hito 5: entrega

- [ ] README completo (conexiones, VREF, resolución, UART, cálculo del TIMER, instrucciones).
- [ ] `pruebas/resultados.md` con PASS o FAIL y su evidencia.
- [ ] Video breve de la demostración.
- [ ] Etiqueta de la versión final y entrega en ITLAVIRTUAL antes del cierre.

## Criterio de commits

Un commit por cambio con sentido propio, que compile. El asunto dice qué cambia; el cuerpo explica por qué y qué requisito cubre (por ejemplo RQ-06). Los resultados de prueba se commitean junto con su evidencia.
