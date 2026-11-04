/* verificar_pct.c - Verifica la formula de porcentaje del mandato para todos los valores del ADC.
 *
 * Mandato: PCT = redondear(100 * ADC / (2^N - 1)).
 * Implementacion del firmware (solo enteros, mismo calculo que APP_PCT_FROM_ADC en app_config.h):
 *
 *     PCT = (100 * ADC + MAX / 2) / MAX        con MAX = 2^N - 1
 *
 * Se compara con el redondeo exacto, calculado sin coma flotante, para cada valor posible del
 * ADC. Tambien se comprueba que no hay empates .5, que la funcion no decrece y que el mayor
 * valor intermedio cabe en 32 bits. Es un programa de PC (no se compila para la placa).
 *
 * Compilar y ejecutar:   gcc -std=c11 -Wall -Wextra -pedantic verificar_pct.c -o verificar_pct
 *                        ./verificar_pct [N]      (N = bits de resolucion; por defecto 12)
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint32_t max_adc;

static uint32_t pct_entero(uint32_t adc)
{
    return (uint32_t)((100ULL * adc + (max_adc / 2ULL)) / max_adc);
}

/* Redondeo al entero mas cercano de 100*adc/max: floor((200*adc + max) / (2*max)). */
static uint32_t pct_exacto(uint32_t adc)
{
    return (uint32_t)((200ULL * adc + max_adc) / (2ULL * max_adc));
}

int main(int argc, char **argv)
{
    unsigned n = (argc > 1) ? (unsigned)atoi(argv[1]) : 12U;
    uint32_t a;
    uint32_t diferencias = 0U, empates = 0U;
    int monotona = 1;
    uint64_t intermedio;
    static const uint32_t refs[] = { 0U, 1U, 20U, 21U, 1024U, 2047U, 2048U, 2866U, 3071U, 3072U, 4095U };
    static const uint32_t pcts[] = { 60U, 62U, 67U, 72U };
    unsigned i;

    if ((n < 2U) || (n > 16U)) {
        fprintf(stderr, "N debe estar entre 2 y 16\n");
        return 2;
    }
    max_adc = (1UL << n) - 1UL;

    for (a = 0U; a <= max_adc; a++) {
        if (pct_entero(a) != pct_exacto(a)) {
            diferencias++;
        }
        if (((200ULL * a) % max_adc == 0ULL) && (((200ULL * a) / max_adc) % 2ULL == 1ULL)) {
            empates++;                      /* 100*a/max cae exactamente en k + 0,5 */
        }
        if ((a < max_adc) && (pct_entero(a) > pct_entero(a + 1U))) {
            monotona = 0;
        }
    }
    intermedio = 100ULL * max_adc + max_adc / 2ULL;

    printf("Resolucion N = %u bits, MAX = %lu\n", n, (unsigned long)max_adc);
    printf("Valores comprobados: %lu\n", (unsigned long)max_adc + 1UL);
    printf("Diferencias entre formula entera y redondeo exacto: %lu\n", (unsigned long)diferencias);
    printf("Empates exactos (x.5): %lu  (MAX es impar, por eso no hay)\n", (unsigned long)empates);
    printf("Funcion no decreciente: %s\n", monotona ? "si" : "no");
    printf("PCT(0) = %lu, PCT(MAX) = %lu\n", (unsigned long)pct_entero(0U), (unsigned long)pct_entero(max_adc));
    printf("Mayor valor intermedio 100*ADC + MAX/2 = %llu\n", (unsigned long long)intermedio);
    printf("  cabe en uint32 (< 4294967296): %s\n", (intermedio < 4294967296ULL) ? "si" : "no");
    printf("  cabe en uint16 (< 65536): %s  -> por eso se usa uint32\n", (intermedio < 65536ULL) ? "si" : "no");

    if (n == 12U) {
        printf("\nPuntos de referencia (N = 12)\n");
        for (i = 0U; i < sizeof refs / sizeof refs[0]; i++) {
            printf("  ADC = %5lu -> PCT = %3lu   (100*ADC/MAX = %7.3f)\n", (unsigned long)refs[i],
                   (unsigned long)pct_entero(refs[i]), 100.0 * (double)refs[i] / (double)max_adc);
        }
        printf("\nEjemplo del mandato (ADC = 2866 -> PCT = 70): %s\n", (pct_entero(2866U) == 70U) ? "coincide" : "NO coincide");
        for (i = 0U; i < sizeof pcts / sizeof pcts[0]; i++) {
            uint32_t primero = 0U, ultimo = 0U, hallado = 0U;
            for (a = 0U; a <= max_adc; a++) {
                if (pct_entero(a) == pcts[i]) {
                    if (!hallado) { primero = a; hallado = 1U; }
                    ultimo = a;
                }
            }
            printf("  PCT = %lu: ADC desde %lu hasta %lu\n", (unsigned long)pcts[i], (unsigned long)primero, (unsigned long)ultimo);
        }
        if (pct_entero(2866U) != 70U) {
            return 1;
        }
    }
    return (diferencias != 0U || empates != 0U || !monotona || intermedio >= 4294967296ULL) ? 1 : 0;
}
