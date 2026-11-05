/* app.c - Une los modulos de la aplicacion.
 * Los modulos se agregan por hitos (docs/plan_de_trabajo.md). */
#include "app.h"
#include "app_config.h"
#include "tick.h"

void App_Init(void)
{
    Tick_Init();            /* arranca TIM6: desde aqui el contador cuenta milisegundos */
}

void App_Loop(void)
{
    /* Aqui se atienden, sin esperas, el pulsador, el ADC, la UART y las tareas periodicas. */
}
