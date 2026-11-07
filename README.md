# Segundo parcial 2L C3-2026: monitor de señal analógica con alarma configurable

- **Estudiante:** Josué Hiraldo · **Matrícula:** 20240705
- **Placa:** STM32 NUCLEO-F446RE (microcontrolador STM32F446RE, Cortex-M4)
- **Estado:** en desarrollo. Última actualización: 7 de noviembre de 2026.

Un potenciómetro representa el nivel de una variable de proceso. El firmware lo adquiere con el ADC, ejecuta tareas periódicas con un TIMER de hardware, controla dos LED y un pulsador con GPIO, y se comunica con la computadora por UART. Todo corre en un único firmware, sin RTOS y sin retardos bloqueantes.

## Resumen de la configuración

| Elemento | Valor |
| --- | --- |
| Base de tiempo | TIM6, 1 kHz (PSC = 83, ARR = 999 con reloj de 84 MHz) |
| Adquisición | ADC1, canal IN0 (PA0), 12 bits, VREF+ = 3,3 V, una muestra cada 100 ms |
| LED de actividad | LD2 (PA5), cambia cada 500 ms |
| LED de alarma | LED externo en PA6 con resistencia de 1 kΩ |
| Pulsador | B1 (PC13), activo en bajo, antirrebote de 30 ms |
| UART | USART2 (PA2/PA3, puente ST-LINK), 115200 baudios, 8N1 |
| Telemetría | una línea cada 1000 ms |

El detalle de cada decisión está en `docs/`.

## Estructura del repositorio

| Carpeta o archivo | Contenido |
| --- | --- |
| `docs/requisitos.md` | Requisitos del mandato con su prueba y su criterio de la rúbrica |
| `docs/plan_de_trabajo.md` | Hitos y lista de commits previstos hasta la entrega |
| `docs/hardware.md` | Mapa de pines, conexiones, VREF, resolución y canal del ADC |
| `docs/temporizacion.md` | Cálculo del reloj, el prescaler y el período del TIMER |
| `docs/cubemx.md` | Configuración de CubeMX periférico por periférico |
| `docs/arquitectura.md` | Tareas, máquina de estados, alarma con histéresis y protocolo UART |
| `Parcial2_20240705/` | Proyecto de STM32CubeIDE: `Inc/`, `Src/`, `Startup/`, `Drivers/` y el archivo `.ioc` de CubeMX |
| `pruebas/` | Plan de pruebas, resultados P01 a P10 y herramientas de verificación |
| `evidencias/` | Capturas, registros de la terminal y enlace al video |

## Herramientas

STM32CubeIDE 2.2.0, STM32CubeMX 6.18.1, Tera Term (COM del ST-LINK), multímetro para medir VREF y la tensión del potenciómetro. Paquete de firmware STM32Cube FW_F4 V1.28.3 (HAL), compilador arm-none-eabi-gcc incluido en CubeIDE.

## Compilar y ejecutar

1. Clonar el repositorio.
2. En STM32CubeIDE 2.2.0: **File → Import → General → Existing Projects into Workspace**. Como directorio raíz, la carpeta `Parcial2_20240705` del repositorio, con *Copy projects into workspace* sin marcar.
3. **Project → Build Project**. La configuración Debug compila sin errores ni advertencias.
4. Conectar la NUCLEO por USB y cargar el programa con **Run**.
5. Abrir Tera Term sobre el puerto COM del ST-LINK: 115200 baudios, 8 bits, sin paridad, 1 bit de parada, sin control de flujo. En *Setup → Terminal*, **New-line Transmit = LF** (o CR+LF); con el valor por defecto (CR) la línea nunca termina y el firmware no recibe el comando.

El proyecto se genera con CubeMX; los valores de cada periférico están en `docs/cubemx.md`. Los pasos de carga y de prueba en la placa se verifican en los hitos siguientes (ver `docs/plan_de_trabajo.md`).

## Resultados

Los resultados de P01 a P10 estarán en `pruebas/resultados.md`. Hasta que cada prueba tenga su evidencia, su estado es PENDIENTE.
