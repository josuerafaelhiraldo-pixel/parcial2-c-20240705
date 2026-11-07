/* adc_in.c - ADC1 canal IN0 con fin de conversion por interrupcion (RQ-03, RQ-05, RQ-04).
 * La interrupcion solo guarda el dato y levanta una bandera; la conversion a porcentaje
 * y la decision de la alarma las hace main. */
#include "adc_in.h"
#include "app_config.h"
#include "adc.h"

/* Los escribe la interrupcion del ADC y los lee main. Main inicia cada conversion y la
 * toma antes de iniciar la siguiente, por lo que la interrupcion no puede escribir mientras
 * main lee. */
static volatile uint16_t s_raw;
static volatile uint8_t  s_new;
static uint32_t s_missed;

uint8_t AdcIn_Start(void)
{
    if (HAL_ADC_Start_IT(&hadc1) != HAL_OK) {
        s_missed++;
        return 0U;
    }
    return 1U;
}

uint8_t AdcIn_Take(uint16_t *raw)
{
    if (s_new == 0U) {
        return 0U;
    }
    s_new = 0U;
    *raw = s_raw;
    return 1U;
}

uint16_t AdcIn_Last(void)
{
    return s_raw;
}

uint32_t AdcIn_Missed(void)
{
    return s_missed;
}

/* Fin de conversion: el HAL la llama desde ADC_IRQHandler. Leer el dato baja la bandera EOC. */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1) {
        s_raw = (uint16_t)(HAL_ADC_GetValue(hadc) & APP_ADC_MAX);
        s_new = 1U;
    }
}
