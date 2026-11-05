# Configuración de STM32CubeMX

Proyecto nuevo desde el selector de placas: **NUCLEO-F446RE**. Nombre: `Parcial2_20240705`. Ubicación: la carpeta de este repositorio. Toolchain: STM32CubeIDE. Cuando pregunte «Initialize all peripherals with their default Mode?», responder **No**: solo se activan los periféricos que usa el mandato.

## Reloj (Clock Configuration)

| Parámetro | Valor |
| --- | --- |
| Fuente | HSI (16 MHz) hacia el PLL |
| HCLK | 84 MHz |
| APB1 prescaler | /2 (PCLK1 = 42 MHz, relojes de temporizadores = 84 MHz) |
| APB2 prescaler | /1 (PCLK2 = 84 MHz) |

## Periféricos

| Periférico | Configuración |
| --- | --- |
| SYS | Debug: Serial Wire. Timebase source: SysTick (queda para el HAL) |
| TIM6 | Activado. Prescaler = 83. Counter period (ARR) = 999. Counter mode: Up. auto-reload preload: Enable. Interrupción global de TIM6 (TIM6_DAC) activada |
| ADC1 | Entrada IN0 (PA0). Resolución 12 bits. Alineación derecha. Scan Conversion Mode: Disabled. Continuous Conversion Mode: Disabled. Discontinuous: Disabled. DMA Continuous Requests: Disabled. End Of Conversion Selection: EOC flag at the end of single conversion. External Trigger Conversion Source: Regular Conversion launched by software. Clock Prescaler: PCLK2 divided by 4. Sampling Time: 144 cycles. Interrupción global del ADC activada |
| USART2 | Modo Asynchronous. Baud rate 115200. Word length 8 bits. Parity None. Stop bits 1. PA2 (TX) y PA3 (RX). Interrupción global de USART2 activada. Sin DMA |
| PA5 | GPIO_Output, etiqueta `LED_ACT`. Sin pull. Nivel inicial: bajo |
| PA6 | GPIO_Output, etiqueta `LED_ALARM`. Sin pull. Nivel inicial: bajo |
| PC13 | GPIO_Input, etiqueta `BTN_USER`. Pull-up. Sin interrupción externa: el pulsador se muestrea con la base de tiempo |

## Prioridades de interrupción (NVIC)

| Interrupción | Prioridad | Motivo |
| --- | --- | --- |
| SysTick | 0 | La usa el HAL |
| TIM6_DAC | 1 | El tick no debe perder cuentas |
| USART2 | 2 | Un byte nuevo no puede perderse mientras el anterior espera |
| ADC | 3 | La más lenta de atender; su dato espera en el registro 7,4 µs como mínimo |

Todas con subprioridad 0. Los callbacks solo anotan datos; el trabajo se hace en `main`.

## Generación de código

En Project Manager → Code Generator:

- Copy only the necessary library files.
- Generate peripheral initialization as a pair of `.c/.h` files per peripheral.
- Keep User Code when re-generating.
- Delete previously generated files when not re-generated.

## Cambios en `main.c`

El código propio vive en archivos nuevos (`app.c`, `app.h`, `app_config.h`). En `main.c` solo se añaden tres líneas dentro de los bloques `USER CODE`:

| Bloque | Línea |
| --- | --- |
| `USER CODE Includes` | `#include "app.h"` |
| `USER CODE 2` | `App_Init();` |
| `USER CODE 3` (dentro de `while (1)`) | `App_Loop();` |

Los vectores de interrupción los genera CubeMX en `stm32f4xx_it.c` y llaman al HAL, que limpia las banderas. Los callbacks (`HAL_TIM_PeriodElapsedCallback`, `HAL_UART_RxCpltCallback`, `HAL_ADC_ConvCpltCallback`) son código propio.

## Resultado de la generación

El proyecto se generó con STM32CubeMX 6.18.1 y el paquete STM32Cube FW_F4 V1.28.3, y compila con 0 errores y 0 advertencias.

La configuración real coincide con las tablas anteriores: PLL con M = 16, N = 336 y P = 4 (HSI de 16 MHz a 84 MHz), TIM6 con PSC = 83 y ARR = 999, ADC1 en IN0 con 144 ciclos de muestreo, USART2 a 115200 8N1 y las cuatro prioridades indicadas. El único ajuste respecto al plan es de organización de archivos:

- La opción *Generate Under Root* quedó activa. El código generado queda en `Inc/`, `Src/` y `Startup/` dentro de `Parcial2_20240705/`, sin la carpeta `Core/` que se suponía en un principio. Es la misma estructura del parcial 1.
- Los archivos propios (`app.c`, `app.h`, `app_config.h`) están en esas mismas carpetas. CubeMX no los genera, así que una regeneración no los toca.
