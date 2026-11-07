/* alarm.c - Modo de operacion (RUN / PAUSE) y alarma con histeresis (RQ-01, RQ-06 a RQ-08). */
#include "alarm.h"
#include "app_config.h"

/* Solo main los escribe y los lee. */
static AlarmMode s_alarmMode;
static uint8_t   s_alarmOn;
static uint32_t  s_thr;
static uint32_t  s_pct;
static uint8_t   s_hasSample;

/* Decide el estado de la alarma con el modo, THR y la ultima muestra, y lo lleva al LED.
 * Es la unica funcion que cambia s_alarmOn, para que todas las reglas queden en un lugar. */
static void alarm_evaluate(void)
{
    if ((s_alarmMode == ALARM_MODE_PAUSE) || (s_hasSample == 0U)) {
        s_alarmOn = 0U;                         /* PAUSE o sin muestra: apagada */
    } else if (s_pct >= s_thr) {
        s_alarmOn = 1U;                         /* PCT >= THR: enciende */
    } else if ((s_pct + APP_HYST_PCT) < s_thr) {
        s_alarmOn = 0U;                         /* PCT < THR - 5: apaga (sin restar, no hay negativos) */
    } else {
        /* Banda de histeresis, THR - 5 <= PCT < THR: conserva el estado. */
    }

    HAL_GPIO_WritePin(APP_LED_ALARM_PORT, APP_LED_ALARM_PIN,
                      (s_alarmOn != 0U) ? APP_LED_ON_LEVEL : APP_LED_OFF_LEVEL);
}

void Alarm_Init(void)
{
    s_alarmMode = ALARM_MODE_RUN;
    s_alarmOn = 0U;
    s_thr = APP_THR_DEFAULT_PCT;
    s_pct = 0U;
    s_hasSample = 0U;
    alarm_evaluate();                           /* deja el LED apagado */
}

AlarmMode Alarm_GetMode(void)
{
    return s_alarmMode;
}

void Alarm_SetMode(AlarmMode mode)
{
    s_alarmMode = mode;
    alarm_evaluate();       /* PAUSE apaga; al volver a RUN decide con la ultima muestra */
}

void Alarm_ToggleMode(void)
{
    Alarm_SetMode((s_alarmMode == ALARM_MODE_RUN) ? ALARM_MODE_PAUSE : ALARM_MODE_RUN);
}

uint8_t Alarm_SetThr(uint32_t thr)
{
    if ((thr < APP_THR_MIN_PCT) || (thr > APP_THR_MAX_PCT)) {
        return 0U;
    }
    s_thr = thr;
    alarm_evaluate();
    return 1U;
}

uint32_t Alarm_GetThr(void)
{
    return s_thr;
}

void Alarm_OnSample(uint32_t pct)
{
    s_pct = pct;
    s_hasSample = 1U;
    alarm_evaluate();       /* en PAUSE la muestra se guarda, pero la alarma sigue apagada */
}

uint32_t Alarm_GetPct(void)
{
    return s_pct;
}

uint8_t Alarm_HasSample(void)
{
    return s_hasSample;
}

uint8_t Alarm_IsOn(void)
{
    return s_alarmOn;
}
