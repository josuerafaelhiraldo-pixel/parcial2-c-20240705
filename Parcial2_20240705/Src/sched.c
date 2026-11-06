/* sched.c - Plazos de las tareas periodicas, sin deriva (RQ-02). */
#include "sched.h"

uint8_t Sched_Due(uint32_t *last, uint32_t period_ms, uint32_t now_ms)
{
    if ((uint32_t)(now_ms - *last) >= period_ms) {
        *last += period_ms;
        return 1U;
    }
    return 0U;
}
