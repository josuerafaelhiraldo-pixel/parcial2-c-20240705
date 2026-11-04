# Temporización: cálculo del TIMER

La base temporal es **TIM6**, un temporizador básico de 16 bits sin pines de entrada ni de salida. Genera una interrupción cada 1 ms. SysTick queda reservado para el HAL (`HAL_Delay` y los tiempos de espera internos); las tareas del programa no dependen de él.

## Cadena de reloj

| Etapa | Valor |
| --- | --- |
| Fuente | HSI, 16 MHz |
| PLL | Los factores que genera CubeMX para 84 MHz (por ejemplo M = 16, N = 336, P = 4); se confirman en `SystemClock_Config` al generar el proyecto |
| HCLK (SYSCLK) | 84 MHz |
| APB1 (prescaler /2) | PCLK1 = 42 MHz |
| Reloj de TIM6 | 84 MHz (los temporizadores de APB1 reciben el doble de PCLK1 cuando el prescaler de APB1 es distinto de 1) |
| APB2 (prescaler /1) | PCLK2 = 84 MHz |
| Reloj del ADC | PCLK2 / 4 = 21 MHz |

## Cálculo del tick de 1 ms

```
f_TIM6 = 84 MHz
f_cuenta = f_TIM6 / (PSC + 1) = 84 MHz / 84 = 1 MHz          con PSC = 83
f_tick   = f_cuenta / (ARR + 1) = 1 MHz / 1000 = 1 kHz        con ARR = 999
T_tick   = 1 ms
```

PSC = 83 y ARR = 999 caben en los 16 bits del temporizador (máximo 65 535) y dan exactamente 1000 Hz: el período es un número entero de cuentas y no se acumula error.

## Tareas periódicas

La interrupción de TIM6 solo hace `ms++`. El programa principal compara el contador con el instante programado de cada tarea y avanza ese instante sumando el período (no copiando la hora actual), así un retraso ocasional no se acumula:

```
if ((uint32_t)(ahora - ultimo) >= periodo) { ultimo += periodo; /* ejecutar la tarea */ }
```

| Tarea | Período | Ticks | Tolerancia ±5 % |
| --- | --- | --- | --- |
| Adquisición ADC | 100 ms | 100 | ±5 ms |
| LED de actividad (cambio de estado) | 500 ms | 500 | ±25 ms |
| Telemetría | 1000 ms | 1000 | ±50 ms |

Los tres períodos son múltiplos exactos del tick. El error esperado es de un tick (1 ms) por la granularidad con que el programa principal atiende la tarea, es decir, 1 % en la tarea más rápida, dentro de la tolerancia.

## Contador de tiempo y desbordamiento

- `t` de la telemetría es el contador de milisegundos desde el reinicio. Es de 32 bits sin signo y se desborda a los 49,7 días.
- Todo plazo se evalúa con resta sin signo, que da el tiempo transcurrido correcto aunque el contador haya dado la vuelta.
- La lectura del contador desde el programa principal es atómica: es una palabra de 32 bits alineada en un Cortex-M4.
- El LED de actividad cambia cada 500 ms, así que su ciclo completo es de 1 s.

## Primera muestra

La primera adquisición se programa en el primer ciclo del programa (t ≈ 0), y las siguientes cada 100 ms exactos. Así existe una muestra válida casi de inmediato, y `STATUS` enviado a partir de 100 ms ya la tiene (P01 lo envía a los 200 ms).

## Presupuesto de la respuesta (≤ 100 ms)

A 115200 baudios con 10 bits por byte, cada carácter tarda unos 87 µs. Una línea de telemetría de unos 48 caracteres con CRLF ocupa la línea unos 4,2 ms. Con el programa principal sin esperas (cada vuelta mucho menor a 1 ms), la respuesta a un comando sale en pocos milisegundos, muy por debajo de los 100 ms. La medición real de la latencia se hace en P10.
