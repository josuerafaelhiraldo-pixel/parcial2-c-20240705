/* app_config.h - Configuracion propia del segundo parcial (NUCLEO-F446RE).
 * Pines, polaridades, periodos y limites del mandato en un solo lugar.
 * Los requisitos RQ-xx se explican en docs/requisitos.md. */
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "stm32f4xx_hal.h"

/* ---- GPIO (docs/hardware.md) ---- */

/* LED de actividad: LD2 integrado, PA5, activo en alto. */
#define APP_LED_ACT_PORT        GPIOA
#define APP_LED_ACT_PIN         GPIO_PIN_5
#define APP_LED_ON_LEVEL        GPIO_PIN_SET
#define APP_LED_OFF_LEVEL       GPIO_PIN_RESET

/* LED de alarma: LED externo con 1 kOhm en PA6 (D12), activo en alto. */
#define APP_LED_ALARM_PORT      GPIOA
#define APP_LED_ALARM_PIN       GPIO_PIN_6

/* Pulsador B1: PC13, pull-up, activo en bajo (reposo = 1, pulsado = 0). */
#define APP_BTN_PORT            GPIOC
#define APP_BTN_PIN             GPIO_PIN_13
#define APP_BTN_PRESSED_LEVEL   GPIO_PIN_RESET

/* ---- Tiempos (RQ-02, RQ-06). Base de tiempo: TIM6 a 1 kHz (docs/temporizacion.md) ---- */
#define APP_TICK_HZ                 1000U
#define APP_ADC_PERIOD_MS           100U    /* una muestra del ADC cada 100 ms */
#define APP_LED_ACT_PERIOD_MS       500U    /* el LED de actividad cambia cada 500 ms */
#define APP_TELEMETRY_PERIOD_MS     1000U   /* una linea de telemetria cada 1000 ms */
#define APP_DEBOUNCE_MS             30U     /* nivel estable minimo del pulsador */
#define APP_BTN_SAMPLE_MS           1U      /* el pulsador se muestrea cada 1 ms */

/* ---- ADC (RQ-03, RQ-05): ADC1, canal IN0 en PA0, 12 bits, VREF+ = 3,3 V ---- */
#define APP_ADC_BITS            12U
#define APP_ADC_MAX             ((1UL << APP_ADC_BITS) - 1UL)   /* 4095 */
#define APP_VREF_MV             3300U   /* nominal; el valor real se mide y se anota en las pruebas */

/* PCT = redondear(100 * ADC / MAX) solo con enteros. El calculo se hace en 32 bits:
 * 100 * 4095 + 2047 = 411547 no cabe en 16 bits. MAX es impar: no hay empates .5.
 * Verificado para los 4096 valores en pruebas/herramientas/verificar_pct.c. */
#define APP_PCT_FROM_ADC(adc) \
    ((uint32_t)((100UL * (uint32_t)(adc) + (APP_ADC_MAX / 2UL)) / APP_ADC_MAX))

/* ---- Alarma (RQ-01, RQ-07, RQ-08, RQ-12) ---- */
#define APP_THR_DEFAULT_PCT     70U     /* umbral inicial */
#define APP_THR_MIN_PCT         10U     /* THR n acepta de 10 ... */
#define APP_THR_MAX_PCT         95U     /* ... a 95 */
#define APP_HYST_PCT            5U      /* apaga con PCT < THR - 5 */

/* ---- UART (RQ-09 a RQ-14): USART2, 115200 8N1 ---- */
#define APP_LINE_MAX            32U     /* caracteres por linea, sin el terminador */
#define APP_THR_MAX_DIGITS      3U      /* digitos admitidos en THR n (evita desbordar) */
#define APP_RX_BUF_SIZE         128U    /* potencia de 2; guarda como maximo 127 bytes */
#define APP_TX_BUF_SIZE         512U    /* potencia de 2; guarda como maximo 511 bytes */
#define APP_LINE_OUT_MAX        96U     /* mayor linea de salida: telemetria o respuesta */

/* ---- Comprobaciones en tiempo de compilacion ---- */
_Static_assert(APP_DEBOUNCE_MS >= 30U, "RQ-06: el antirrebote debe ser de al menos 30 ms");
_Static_assert((APP_DEBOUNCE_MS % APP_BTN_SAMPLE_MS) == 0U, "el antirrebote debe ser multiplo del muestreo");
_Static_assert((APP_LED_ACT_PERIOD_MS % APP_ADC_PERIOD_MS) == 0U, "los periodos deben ser multiplos del de 100 ms");
_Static_assert((APP_TELEMETRY_PERIOD_MS % APP_ADC_PERIOD_MS) == 0U, "los periodos deben ser multiplos del de 100 ms");
_Static_assert(APP_THR_MIN_PCT >= APP_HYST_PCT, "THR - 5 no puede ser negativo");
_Static_assert(APP_THR_MIN_PCT <= APP_THR_DEFAULT_PCT && APP_THR_DEFAULT_PCT <= APP_THR_MAX_PCT, "THR inicial fuera de rango");
_Static_assert((100ULL * APP_ADC_MAX + APP_ADC_MAX / 2ULL) <= 0xFFFFFFFFULL, "PCT desbordaria 32 bits");
_Static_assert((APP_RX_BUF_SIZE & (APP_RX_BUF_SIZE - 1U)) == 0U, "APP_RX_BUF_SIZE debe ser potencia de 2");
_Static_assert((APP_TX_BUF_SIZE & (APP_TX_BUF_SIZE - 1U)) == 0U, "APP_TX_BUF_SIZE debe ser potencia de 2");

#endif /* APP_CONFIG_H */
