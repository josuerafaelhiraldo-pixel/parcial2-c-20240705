/* app.c - Une los modulos de la aplicacion.
 * Los modulos se agregan por hitos (docs/plan_de_trabajo.md). */
#include "app.h"
#include "app_config.h"
#include "tick.h"
#include "sched.h"
#include "button.h"
#include "alarm.h"
#include "adc_in.h"
#include "console.h"

static uint32_t s_tLed;     /* instante programado del proximo cambio del LED de actividad */
static uint32_t s_tBtn;     /* instante programado del proximo muestreo del pulsador */
static uint32_t s_tAdc;     /* instante programado de la proxima conversion del ADC */

void App_Init(void)
{
    Tick_Init();            /* arranca TIM6: desde aqui el contador cuenta milisegundos */
    uint32_t now = Tick_Ms();

    s_tLed = now;
    s_tBtn = now;
    s_tAdc = now - APP_ADC_PERIOD_MS;   /* la primera conversion se inicia en la primera vuelta */
    Alarm_Init();           /* RUN y alarma apagada despues del reinicio */
    Button_Init(now);
    Console_Init();         /* USART2: recepcion por interrupcion y cola de transmision */
}

void App_Loop(void)
{
    uint32_t now = Tick_Ms();
    uint16_t raw;
    uint8_t  b;

    Console_Service();

    /* Pulsador: se muestrea cada 1 ms; una pulsacion valida alterna RUN y PAUSE (RQ-06). */
    if (Sched_Due(&s_tBtn, APP_BTN_SAMPLE_MS, now) != 0U) {
        if (Button_Service(now) == BTN_EVT_PRESS) {
            Alarm_ToggleMode();
        }
    }

    /* ADC: una conversion cada 100 ms (RQ-02). Termina por interrupcion; aqui solo se inicia. */
    if (Sched_Due(&s_tAdc, APP_ADC_PERIOD_MS, now) != 0U) {
        (void)AdcIn_Start();
    }

    /* Muestra nueva: se convierte a porcentaje fuera de la interrupcion (RQ-03) y la alarma
     * decide con ella (RQ-07, RQ-08). */
    if (AdcIn_Take(&raw) != 0U) {
        Alarm_OnSample(APP_PCT_FROM_ADC(raw));
    }

    /* UART, prueba de enlace: devuelve cada byte recibido. Se reemplaza por los comandos. */
    while (Console_ReadByte(&b) != false) {
        (void)Console_WriteBytes(&b, 1U);
    }

    /* LED de actividad: cambia de estado cada 500 ms (RQ-02). Sigue activo en PAUSE. */
    if (Sched_Due(&s_tLed, APP_LED_ACT_PERIOD_MS, now) != 0U) {
        HAL_GPIO_TogglePin(APP_LED_ACT_PORT, APP_LED_ACT_PIN);
    }
}
