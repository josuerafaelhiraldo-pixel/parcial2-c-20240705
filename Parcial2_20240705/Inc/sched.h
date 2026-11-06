/* sched.h - Plazos de las tareas periodicas, sin deriva (RQ-02). */
#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>

/* Devuelve 1 si ya transcurrio 'period_ms' desde el instante programado '*last', y en ese
 * caso avanza '*last' sumando el periodo (no copiando la hora actual). Asi un retraso
 * ocasional del programa principal no se acumula: la tarea siguiente se programa respecto
 * al instante ideal. La resta sin signo da el tiempo transcurrido correcto aunque el
 * contador de milisegundos haya dado la vuelta (docs/temporizacion.md).
 * Devuelve 0 si todavia no toca. */
uint8_t Sched_Due(uint32_t *last, uint32_t period_ms, uint32_t now_ms);

#endif /* SCHED_H */
