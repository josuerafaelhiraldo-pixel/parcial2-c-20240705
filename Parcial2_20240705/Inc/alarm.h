/* alarm.h - Modo de operacion (RUN / PAUSE) y alarma con histeresis (RQ-01, RQ-06, RQ-07, RQ-08).
 *
 * Reglas (tabla completa en docs/arquitectura.md):
 *   RUN:   PCT >= THR enciende; PCT < THR - 5 apaga; entre ambos conserva el estado.
 *   PAUSE: la alarma esta apagada, sin importar la muestra.
 *   Cada cambio de modo, de THR o de muestra reevalua con las mismas reglas.
 */
#ifndef ALARM_H
#define ALARM_H

#include <stdint.h>

typedef enum {
    ALARM_MODE_RUN = 0,
    ALARM_MODE_PAUSE
} AlarmMode;

/* Estado despues del reinicio: RUN, THR = 70, alarma apagada y sin muestra. */
void Alarm_Init(void);

AlarmMode Alarm_GetMode(void);

/* Establece el modo y reevalua. En PAUSE la alarma queda apagada al instante. */
void Alarm_SetMode(AlarmMode mode);

/* Cambia de RUN a PAUSE y viceversa (pulsacion valida del boton). */
void Alarm_ToggleMode(void);

/* Cambia THR si esta entre 10 y 95 y reevalua; devuelve 1. Fuera de rango devuelve 0
 * y no cambia nada. */
uint8_t Alarm_SetThr(uint32_t thr);
uint32_t Alarm_GetThr(void);

/* Registra una muestra (PCT de 0 a 100) y reevalua. */
void Alarm_OnSample(uint32_t pct);

/* Ultimo PCT recibido y si ya hubo alguna muestra desde el reinicio. */
uint32_t Alarm_GetPct(void);
uint8_t Alarm_HasSample(void);

/* 1 si la alarma esta encendida. */
uint8_t Alarm_IsOn(void);

#endif /* ALARM_H */
