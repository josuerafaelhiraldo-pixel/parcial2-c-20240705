/* alarm.c - Modo de operacion (RUN / PAUSE) y alarma (RQ-06, RQ-07, RQ-08). */
#include "alarm.h"
#include "app_config.h"

/* Solo main lo escribe y lo lee. */
static AlarmMode s_alarmMode;

static void alarm_led_off(void)
{
    HAL_GPIO_WritePin(APP_LED_ALARM_PORT, APP_LED_ALARM_PIN, APP_LED_OFF_LEVEL);
}

void Alarm_Init(void)
{
    s_alarmMode = ALARM_MODE_RUN;
    alarm_led_off();
}

AlarmMode Alarm_GetMode(void)
{
    return s_alarmMode;
}

void Alarm_SetMode(AlarmMode mode)
{
    s_alarmMode = mode;
    if (mode == ALARM_MODE_PAUSE) {
        alarm_led_off();            /* en PAUSE la alarma esta forzada a apagada */
    }
}

void Alarm_ToggleMode(void)
{
    Alarm_SetMode((s_alarmMode == ALARM_MODE_RUN) ? ALARM_MODE_PAUSE : ALARM_MODE_RUN);
}
