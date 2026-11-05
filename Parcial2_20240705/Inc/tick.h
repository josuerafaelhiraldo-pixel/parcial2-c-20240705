/* tick.h - Base de tiempo: contador de milisegundos con TIM6 (RQ-02). */
#ifndef TICK_H
#define TICK_H

#include <stdint.h>

/* Pone el contador en 0 y arranca TIM6 con interrupcion cada 1 ms.
 * Se llama una vez desde App_Init, despues de MX_TIM6_Init. */
void Tick_Init(void);

/* Milisegundos desde el reinicio. Es de 32 bits sin signo y da la vuelta a los 49,7 dias:
 * los plazos se comparan siempre con resta sin signo (ver sched.h). La lectura es atomica. */
uint32_t Tick_Ms(void);

#endif /* TICK_H */
