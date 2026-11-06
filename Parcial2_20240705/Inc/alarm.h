/* alarm.h - Modo de operacion (RUN / PAUSE) y alarma (RQ-06, RQ-07, RQ-08).
 * En este commit solo existe el modo; la logica de la alarma con histeresis se agrega
 * en el Hito 2. */
#ifndef ALARM_H
#define ALARM_H

typedef enum {
    ALARM_MODE_RUN = 0,
    ALARM_MODE_PAUSE
} AlarmMode;

/* Estado despues del reinicio: RUN y alarma apagada. */
void Alarm_Init(void);

AlarmMode Alarm_GetMode(void);

/* Establece el modo. En PAUSE la alarma queda apagada. */
void Alarm_SetMode(AlarmMode mode);

/* Cambia de RUN a PAUSE y viceversa (pulsacion valida del boton). */
void Alarm_ToggleMode(void);

#endif /* ALARM_H */
