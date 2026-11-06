/* button.c - Pulsador B1 con antirrebote de 30 ms (RQ-06).
 * Sin interrupcion externa: main muestrea PC13 cada 1 ms. Un cambio de nivel se acepta
 * solo si el nuevo nivel dura APP_DEBOUNCE_MS sin interrupcion; los rebotes y los pulsos
 * cortos reinician la ventana y no producen eventos. */
#include "button.h"
#include "app_config.h"

static uint8_t  s_stable;       /* estado validado: 1 = pulsado */
static uint8_t  s_cand;         /* ultimo nivel leido (candidato) */
static uint32_t s_since;        /* instante en que el candidato empezo a mantenerse */

static uint8_t raw_pressed(void)
{
    return (HAL_GPIO_ReadPin(APP_BTN_PORT, APP_BTN_PIN) == APP_BTN_PRESSED_LEVEL) ? 1U : 0U;
}

void Button_Init(uint32_t now_ms)
{
    s_cand   = raw_pressed();
    s_stable = s_cand;
    s_since  = now_ms;
}

ButtonEvent Button_Service(uint32_t now_ms)
{
    uint8_t raw = raw_pressed();

    /* El nivel cambio respecto al ultimo muestreo: reiniciar la ventana de 30 ms. */
    if (raw != s_cand) {
        s_cand  = raw;
        s_since = now_ms;
        return BTN_EVT_NONE;
    }

    /* El nivel candidato difiere del validado y ya dura 30 ms: aceptar el cambio. */
    if ((s_cand != s_stable) && ((uint32_t)(now_ms - s_since) >= APP_DEBOUNCE_MS)) {
        s_stable = s_cand;
        return (s_stable != 0U) ? BTN_EVT_PRESS : BTN_EVT_RELEASE;
    }

    return BTN_EVT_NONE;
}
