/* medir_tiempos.c - Analiza el registro de Tera Term de la prueba P02 (RQ-02).
 *
 * Entrada: un .log con marcas de tiempo (Archivo > Log, "Timestamp" en hora local) tomado con el
 * firmware compilado con APP_TRACE = 1. Reconoce tres tipos de linea:
 *   @ACQ;t=<ms>               muestra del ADC entregada (cada 100 ms)
 *   @LED;t=<ms>;LD2=<0|1>     cambio del LED de actividad (cada 500 ms)
 *   t=<ms>;ADC=..;..;ALARM=n  telemetria (cada 1000 ms)
 * Cada linea del registro empieza con "[AAAA-MM-DD HH:MM:SS.mmm] " (hora del PC).
 *
 * Calcula, para cada tarea:
 *   - los intervalos con el reloj de la placa (el t de cada linea): minimo, medio, maximo y la
 *     desviacion maxima respecto al periodo nominal; el criterio es +-5 %;
 *   - el periodo medio con el reloj del PC (primera a ultima linea de la tarea). Sirve de
 *     referencia independiente: comprueba que el reloj de la placa avanza al ritmo real;
 *   - para LD2, que el nivel alterne 1, 0, 1, 0 sin repetirse.
 * Las marcas de tiempo del PC tienen jitter de unos milisegundos (USB y Tera Term), por eso los
 * intervalos individuales del PC solo se informan, no deciden el resultado.
 *
 * Compilar:  gcc -std=c11 -O2 -Wall -Wextra -o medir_tiempos medir_tiempos.c
 * Usar:      medir_tiempos P02_tiempos.log > P02_intervalos.txt
 * Termina con 0 si todo cumple y con 1 si algo falla. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define TOL_PCT        5.0     /* tolerancia del enunciado */
#define MAX_MUESTRAS   200000

typedef struct {
    const char *nombre;
    uint32_t    nominal;               /* periodo nominal en ms */
    uint32_t    n;                     /* lineas reconocidas */
    uint32_t    tPlaca0, tPlacaN;      /* primer y ultimo t de la placa */
    double      pc0, pcN;              /* primera y ultima hora del PC, en ms */
    uint32_t    dMin, dMax;            /* intervalos de la placa */
    double      dSuma;
    double      pcMin, pcMax;          /* intervalos del PC (solo informativos) */
    double      pcPrev;
    uint32_t    tPrev;
    uint32_t    fueraDeLimite;         /* intervalos de la placa fuera de +-5 % */
    int         tiene;
} Tarea;

static void tarea_init(Tarea *t, const char *nombre, uint32_t nominal)
{
    memset(t, 0, sizeof *t);
    t->nombre = nombre;
    t->nominal = nominal;
    t->dMin = 0xFFFFFFFFu;
    t->pcMin = 1e18;
}

static void tarea_add(Tarea *t, uint32_t tPlaca, double pc)
{
    if (t->tiene) {
        uint32_t d = tPlaca - t->tPrev;              /* aritmetica de 32 bits: tolera el desborde */
        double lo = t->nominal * (1.0 - TOL_PCT / 100.0);
        double hi = t->nominal * (1.0 + TOL_PCT / 100.0);
        double dp = pc - t->pcPrev;

        if (d < t->dMin) t->dMin = d;
        if (d > t->dMax) t->dMax = d;
        t->dSuma += d;
        if ((double)d < lo || (double)d > hi) t->fueraDeLimite++;
        if (dp < t->pcMin) t->pcMin = dp;
        if (dp > t->pcMax) t->pcMax = dp;
    } else {
        t->tPlaca0 = tPlaca;
        t->pc0 = pc;
    }
    t->tPrev = tPlaca;
    t->pcPrev = pc;
    t->tPlacaN = tPlaca;
    t->pcN = pc;
    t->n++;
    t->tiene = 1;
}

/* "[2026-11-07 19:18:02.079] " -> milisegundos desde las 00:00 de ese dia. Devuelve el largo del
 * prefijo, o 0 si la linea no lo tiene. Un registro que cruza la medianoche no se admite. */
static int leer_marca(const char *s, double *ms)
{
    int Y, M, D, h, m, sec, milli, used = 0;

    if (sscanf(s, "[%d-%d-%d %d:%d:%d.%d] %n", &Y, &M, &D, &h, &m, &sec, &milli, &used) < 7 || used == 0) {
        return 0;
    }
    *ms = ((h * 60.0 + m) * 60.0 + sec) * 1000.0 + milli;
    return used;
}

static int es_telemetria(const char *s, uint32_t *t)
{
    unsigned a, p, th; int al; char modo[8]; unsigned long tt; int n = 0;

    if (sscanf(s, "t=%lu;ADC=%u;PCT=%u;THR=%u;MODE=%7[A-Z];ALARM=%d%n", &tt, &a, &p, &th, modo, &al, &n) == 6
        && (s[n] == '\0')) {
        *t = (uint32_t)tt;
        return 1;
    }
    return 0;
}

static int informe(const Tarea *t, double *peorDesv)
{
    double n1 = (double)(t->n - 1U);
    double medio, desv, pcMedio, pcDesv;
    double lo = t->nominal * (1.0 - TOL_PCT / 100.0);
    double hi = t->nominal * (1.0 + TOL_PCT / 100.0);
    int ok;

    if (t->n < 3U) {
        printf("%-22s sin datos suficientes (%u lineas)\n", t->nombre, (unsigned)t->n);
        return 0;
    }
    medio = t->dSuma / n1;
    desv = fmax(fabs((double)t->dMin - t->nominal), fabs((double)t->dMax - t->nominal)) * 100.0 / t->nominal;
    pcMedio = (t->pcN - t->pc0) / n1;
    pcDesv = (pcMedio - t->nominal) * 100.0 / t->nominal;
    ok = (t->fueraDeLimite == 0U) && (fabs(pcDesv) <= TOL_PCT);
    if (desv > *peorDesv) *peorDesv = desv;

    printf("%s (periodo nominal %u ms, limite %.0f a %.0f ms)\n", t->nombre, (unsigned)t->nominal, lo, hi);
    printf("  lineas: %u, intervalos: %u\n", (unsigned)t->n, (unsigned)(t->n - 1U));
    printf("  reloj de la placa: minimo %u ms, medio %.2f ms, maximo %u ms, desviacion maxima %.2f %%, fuera de +-%.0f %%: %u\n",
           (unsigned)t->dMin, medio, (unsigned)t->dMax, desv, TOL_PCT, (unsigned)t->fueraDeLimite);
    printf("  reloj del PC: periodo medio %.2f ms (%+.3f %%); intervalos sueltos de %.0f a %.0f ms (informativo, jitter del PC)\n",
           pcMedio, pcDesv, t->pcMin, t->pcMax);
    printf("  resultado: %s\n\n", ok ? "CUMPLE" : "NO CUMPLE");
    return ok;
}

int main(int argc, char **argv)
{
    FILE *f;
    char linea[512];
    Tarea acq, led, tel;
    uint32_t desconocidas = 0, lineasTotal = 0, ldRepetido = 0, ldTotal = 0;
    int ldPrev = -1, ok = 1;
    double peor = 0.0, pcIni = 0.0, pcFin = 0.0;
    int hayPc = 0;

    if (argc != 2) {
        fprintf(stderr, "uso: %s registro.log\n", argv[0]);
        return 2;
    }
    f = fopen(argv[1], "r");
    if (f == NULL) {
        perror(argv[1]);
        return 2;
    }
    tarea_init(&acq, "ADC (@ACQ)", 100U);
    tarea_init(&led, "LED LD2 (@LED)", 500U);
    tarea_init(&tel, "Telemetria", 1000U);

    while (fgets(linea, sizeof linea, f) != NULL) {
        double pc;
        int off;
        char *s;
        size_t L = strlen(linea);

        while (L > 0 && (linea[L - 1] == '\n' || linea[L - 1] == '\r')) linea[--L] = '\0';
        if (L == 0) continue;
        lineasTotal++;
        off = leer_marca(linea, &pc);
        if (off == 0) { desconocidas++; continue; }
        s = linea + off;
        if (!hayPc) { pcIni = pc; hayPc = 1; }
        pcFin = pc;

        {
            unsigned long t; int ld; int n = 0;
            uint32_t tt;

            if (sscanf(s, "@ACQ;t=%lu%n", &t, &n) == 1 && s[n] == '\0') {
                tarea_add(&acq, (uint32_t)t, pc);
            } else if (sscanf(s, "@LED;t=%lu;LD2=%d%n", &t, &ld, &n) == 2 && s[n] == '\0' && (ld == 0 || ld == 1)) {
                tarea_add(&led, (uint32_t)t, pc);
                ldTotal++;
                if (ld == ldPrev) ldRepetido++;
                ldPrev = ld;
            } else if (es_telemetria(s, &tt)) {
                tarea_add(&tel, tt, pc);
            } else {
                desconocidas++;
                printf("linea no reconocida: %s\n", linea);
            }
        }
    }
    fclose(f);

    printf("Registro: %s\n", argv[1]);
    printf("Lineas: %u; no reconocidas: %u; ventana del PC: %.1f s\n\n", (unsigned)lineasTotal, (unsigned)desconocidas, (pcFin - pcIni) / 1000.0);

    ok &= informe(&acq, &peor);
    ok &= informe(&led, &peor);
    ok &= informe(&tel, &peor);

    printf("LD2 alterna 1, 0, 1, 0: %u cambios, repetidos seguidos: %u -> %s\n", (unsigned)ldTotal, (unsigned)ldRepetido,
           (ldTotal > 2U && ldRepetido == 0U) ? "CUMPLE" : "NO CUMPLE");
    if (!(ldTotal > 2U && ldRepetido == 0U)) ok = 0;

    if (tel.n >= 3U) {
        double placa = (double)(tel.tPlacaN - tel.tPlaca0);
        double pc = tel.pcN - tel.pc0;
        printf("Reloj de la placa contra reloj del PC (telemetria, primera a ultima): placa %.0f ms, PC %.0f ms, diferencia %+.3f %%\n",
               placa, pc, (placa - pc) * 100.0 / pc);
        if (fabs((placa - pc) * 100.0 / pc) > TOL_PCT) ok = 0;
    }
    printf("\nDesviacion maxima de un intervalo entre las tres tareas: %.2f %% (limite %.0f %%)\n", peor, TOL_PCT);
    printf("RESULTADO: %s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
