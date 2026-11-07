/* adc_in.h - Lectura del potenciometro con ADC1, canal IN0 (PA0) (RQ-03, RQ-05).
 * La conversion se inicia desde main y termina por interrupcion: nada espera al ADC. */
#ifndef ADC_IN_H
#define ADC_IN_H

#include <stdint.h>

/* Inicia una conversion y vuelve de inmediato. Devuelve 1 si la inicio y 0 si el HAL
 * la rechazo (en ese caso se cuenta en AdcIn_Missed y la muestra se pierde). */
uint8_t AdcIn_Start(void);

/* Si hay un resultado nuevo, lo copia en *raw (0 a 4095), baja la bandera y devuelve 1.
 * Si no hay ninguno devuelve 0 y no toca *raw. */
uint8_t AdcIn_Take(uint16_t *raw);

/* Ultimo valor convertido; 0 antes de la primera conversion. */
uint16_t AdcIn_Last(void);

/* Veces que no se pudo iniciar una conversion. En funcionamiento normal vale 0. */
uint32_t AdcIn_Missed(void);

#endif /* ADC_IN_H */
