/* app.c - Une los modulos de la aplicacion.
 * Los modulos se agregan por hitos (docs/plan_de_trabajo.md). */
#include "app.h"
#include "app_config.h"
#include "tick.h"
#include "sched.h"

static uint32_t s_tLed;     /* instante programado del proximo cambio del LED de actividad */

void App_Init(void)
{
    Tick_Init();            /* arranca TIM6: desde aqui el contador cuenta milisegundos */
    s_tLed = Tick_Ms();
}

void App_Loop(void)
{
    uint32_t now = Tick_Ms();

    /* LED de actividad: cambia de estado cada 500 ms (RQ-02). */
    if (Sched_Due(&s_tLed, APP_LED_ACT_PERIOD_MS, now) != 0U) {
        HAL_GPIO_TogglePin(APP_LED_ACT_PORT, APP_LED_ACT_PIN);
    }

    /* Aqui se atienden, sin esperas, el pulsador, el ADC y la UART. */
}
