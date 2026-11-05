/* tick.c - Base de tiempo: contador de milisegundos con TIM6 (RQ-02).
 * TIM6 desborda cada 1 ms (PSC = 83, ARR = 999 con reloj de 84 MHz; docs/temporizacion.md).
 * La interrupcion solo incrementa el contador: el trabajo lo hace main. */
#include "tick.h"
#include "tim.h"

/* Solo escribe la ISR de TIM6; main solo lee. */
static volatile uint32_t s_ms;

void Tick_Init(void)
{
    s_ms = 0U;

    /* La inicializacion del temporizador genera un evento de actualizacion que deja la
     * bandera UIF levantada. Se limpia antes de habilitar la interrupcion para que no se
     * cuente un milisegundo falso al arrancar. */
    __HAL_TIM_CLEAR_FLAG(&htim6, TIM_FLAG_UPDATE);
    (void)HAL_TIM_Base_Start_IT(&htim6);
}

uint32_t Tick_Ms(void)
{
    return s_ms;
}

/* Callback del HAL, llamado desde TIM6_DAC_IRQHandler (stm32f4xx_it.c). SysTick queda para
 * el HAL, asi que aqui solo llega TIM6; se comprueba la instancia por si se agregan otros. */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6) {
        s_ms++;
    }
}
