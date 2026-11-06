/* app.c - Une los modulos de la aplicacion.
 * Los modulos se agregan por hitos (docs/plan_de_trabajo.md). */
#include "app.h"
#include "app_config.h"
#include "tick.h"
#include "sched.h"
#include "button.h"
#include "alarm.h"

static uint32_t s_tLed;     /* instante programado del proximo cambio del LED de actividad */
static uint32_t s_tBtn;     /* instante programado del proximo muestreo del pulsador */

void App_Init(void)
{
    Tick_Init();            /* arranca TIM6: desde aqui el contador cuenta milisegundos */
    uint32_t now = Tick_Ms();

    s_tLed = now;
    s_tBtn = now;
    Alarm_Init();           /* RUN y alarma apagada despues del reinicio */
    Button_Init(now);
}

void App_Loop(void)
{
    uint32_t now = Tick_Ms();

    /* Pulsador: se muestrea cada 1 ms; una pulsacion valida alterna RUN y PAUSE (RQ-06). */
    if (Sched_Due(&s_tBtn, APP_BTN_SAMPLE_MS, now) != 0U) {
        if (Button_Service(now) == BTN_EVT_PRESS) {
            Alarm_ToggleMode();
        }
    }

    /* LED de actividad: cambia de estado cada 500 ms (RQ-02). Sigue activo en PAUSE. */
    if (Sched_Due(&s_tLed, APP_LED_ACT_PERIOD_MS, now) != 0U) {
        HAL_GPIO_TogglePin(APP_LED_ACT_PORT, APP_LED_ACT_PIN);
    }

    /* Aqui se atienden, sin esperas, el ADC y la UART. */
}
