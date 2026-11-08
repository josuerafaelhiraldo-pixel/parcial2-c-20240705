/* analizar_p10.c - Analiza el registro de Tera Term de la prueba P10 (carga de comandos).
 *
 * Entrada: un .log con marcas de tiempo (Archivo > Log, "Timestamp" en hora local) tomado con
 * Local echo activado mientras corre pruebas/herramientas/p10_carga.ttl. Cada linea empieza con
 * "[AAAA-MM-DD HH:MM:SS.mmm] ". Con Local echo, el texto enviado aparece en el registro con la
 * hora en que Tera Term lo envio; la respuesta de la placa aparece despues. La latencia es la
 * diferencia entre ambas marcas.
 *
 * El eco local puede caer en mitad de una linea de telemetria que estaba llegando (por ejemplo
 * "t=14000;ADC=24" + "STATUS" y luego "76;PCT=60;..."). El programa separa el eco, reune los dos
 * trozos de la linea y comprueba que forman una linea valida; eso se cuenta aparte.
 *
 * Comprueba:
 *   1. Cada comando recibe una sola respuesta del tipo esperado, antes del comando siguiente.
 *   2. Latencia de cada respuesta (eco a respuesta) de 100 ms o menos.
 *   3. Ninguna linea corrupta ni desconocida (se tolera el recorte de la primera y la ultima
 *      linea del registro).
 *   4. Cada STATUS refleja el estado esperado (THR y MODE tras los comandos anteriores).
 *   5. En todas las lineas de estado (telemetria y STATUS): PCT = redondeo de 100 * ADC / 4095;
 *      en PAUSE la alarma esta apagada; en RUN, PCT >= THR da ALARM 1 y PCT < THR - 5 da ALARM 0.
 *   6. Telemetria cada 1000 ms con el reloj de la placa (limite +-5 %), sin lineas perdidas.
 *   7. Ritmo de comandos: promedio de los 150 primeros.
 *
 * Compilar:  gcc -std=c11 -O2 -Wall -Wextra -o analizar_p10 analizar_p10.c -lm
 * Usar:      analizar_p10 P10_carga.log > P10_latencias.txt
 * Termina con 0 si todo cumple y con 1 si algo falla. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define MAX_EV        8000
#define LAT_MAX_MS    100.0
#define TOL_PCT       5.0
#define CARGA_N       150

typedef enum { CMD_STATUS, CMD_THR, CMD_START, CMD_STOP, CMD_OTRO } CmdTipo;
typedef enum { R_STATUS, R_THR, R_RUN, R_PAUSE, R_ERR } RespTipo;

typedef struct { unsigned long t; unsigned adc, pct, thr; int pausa, alarma; } Estado;

typedef struct {
    int      esEco;         /* 1 = comando enviado, 0 = respuesta o telemetria */
    double   ms;            /* hora del PC */
    int      linea;         /* numero de linea del registro */
    CmdTipo  cmd; int cmdN; /* si es eco */
    RespTipo resp; int respN; Estado st; /* si es respuesta */
    int      pegado;        /* la linea se reconstruyo juntando dos trozos */
    int      usado;         /* esta linea de estado se tomo como respuesta a un STATUS */
} Ev;

static Ev ev[MAX_EV];
static int nev;
static int unk[512], nunk;          /* lineas desconocidas: numero de linea del registro */
static void unk_add(int nl) { if (nunk < 512) unk[nunk++] = nl; }

static int leer_marca(const char *s, double *ms)
{
    int Y, M, D, h, m, sec, milli, used = 0;
    if (sscanf(s, "[%d-%d-%d %d:%d:%d.%d] %n", &Y, &M, &D, &h, &m, &sec, &milli, &used) < 7 || used == 0) return 0;
    *ms = ((h * 60.0 + m) * 60.0 + sec) * 1000.0 + milli;
    return used;
}

static int parse_estado(const char *s, Estado *e)
{
    unsigned long t; unsigned a, p, th; int al, n = 0; char modo[8];
    if (sscanf(s, "t=%lu;ADC=%u;PCT=%u;THR=%u;MODE=%7[A-Z];ALARM=%d%n", &t, &a, &p, &th, modo, &al, &n) != 6 || s[n] != '\0') return 0;
    if (strcmp(modo, "RUN") != 0 && strcmp(modo, "PAUSE") != 0) return 0;
    if (al != 0 && al != 1) return 0;
    e->t = t; e->adc = a; e->pct = p; e->thr = th; e->pausa = (modo[1] == 'A'); e->alarma = al;
    return 1;
}

/* "STATUS", "START", "STOP", "HOLA" o "THR n" completo */
static int parse_eco(const char *s, CmdTipo *c, int *n)
{
    int k = 0, v;
    *n = 0;
    if (strcmp(s, "STATUS") == 0) { *c = CMD_STATUS; return 1; }
    if (strcmp(s, "START") == 0)  { *c = CMD_START;  return 1; }
    if (strcmp(s, "STOP") == 0)   { *c = CMD_STOP;   return 1; }
    if (strcmp(s, "HOLA") == 0)   { *c = CMD_OTRO;   return 1; }
    if (sscanf(s, "THR %d%n", &v, &k) == 1 && s[k] == '\0') { *c = CMD_THR; *n = v; return 1; }
    return 0;
}

/* Si la linea termina en un eco de comando, devuelve la posicion donde empieza (> 0); si no, -1 */
static int eco_al_final(const char *s)
{
    static const char *fijos[] = { "STATUS", "START", "STOP", "HOLA" };
    size_t L = strlen(s);
    size_t i;
    int d = 0;

    for (i = 0; i < sizeof fijos / sizeof fijos[0]; i++) {
        size_t l = strlen(fijos[i]);
        if (L > l && strcmp(s + L - l, fijos[i]) == 0) return (int)(L - l);
    }
    while (d < 3 && L > (size_t)d && s[L - 1 - (size_t)d] >= '0' && s[L - 1 - (size_t)d] <= '9') d++;
    if (d > 0 && L > (size_t)d + 4U && strncmp(s + L - (size_t)d - 4U, "THR ", 4) == 0) return (int)(L - (size_t)d - 4U);
    return -1;
}

static int parse_resp(const char *s, RespTipo *r, int *n, Estado *e)
{
    int k = 0, v;
    if (parse_estado(s, e)) { *r = R_STATUS; return 1; }
    if (strcmp(s, "OK RUN") == 0)   { *r = R_RUN;   return 1; }
    if (strcmp(s, "OK PAUSE") == 0) { *r = R_PAUSE; return 1; }
    if (strcmp(s, "ERR") == 0)      { *r = R_ERR;   return 1; }
    if (sscanf(s, "OK THR=%d%n", &v, &k) == 1 && s[k] == '\0') { *r = R_THR; *n = v; return 1; }
    return 0;
}

static const char *nom_cmd(CmdTipo c, int n, char *buf)
{
    switch (c) {
    case CMD_STATUS: return "STATUS";
    case CMD_START:  return "START";
    case CMD_STOP:   return "STOP";
    case CMD_THR:    sprintf(buf, "THR %d", n); return buf;
    default:         return "HOLA";
    }
}

static int pct_ok(unsigned adc, unsigned pct)
{
    return pct == (unsigned)((100UL * adc + 2047UL) / 4095UL);
}

/* invariantes de una linea de estado; devuelve 1 si se cumplen */
static int invariantes(const Estado *e)
{
    if (e->adc > 4095U) return 0;
    if (!pct_ok(e->adc, e->pct)) return 0;
    if (e->pausa && e->alarma) return 0;
    if (!e->pausa) {
        if (e->pct >= e->thr && !e->alarma) return 0;
        if (e->pct + 5U < e->thr && e->alarma) return 0;
    }
    return 1;
}

static int cmp_dbl(const void *a, const void *b)
{
    double x = *(const double *)a, y = *(const double *)b;
    return (x > y) - (x < y);
}

int main(int argc, char **argv)
{
    FILE *f;
    char linea[512], pend[512];
    double pendMs = 0.0; int pendLinea = 0, hayPend = 0;
    int nl = 0, desconocidas = 0, pegadas = 0, tolerada = 0, ok = 1;
    int i, ultimaConLinea = 0, primeraConLinea = 0;

    if (argc != 2) { fprintf(stderr, "uso: %s registro.log\n", argv[0]); return 2; }
    f = fopen(argv[1], "r");
    if (f == NULL) { perror(argv[1]); return 2; }

    /* ---- 1. lectura y clasificacion de las lineas ---- */
    while (fgets(linea, sizeof linea, f) != NULL) {
        double ms; int off; char *s; size_t L = strlen(linea);
        CmdTipo c; int n; RespTipo r; Estado e;

        while (L > 0 && (linea[L - 1] == '\n' || linea[L - 1] == '\r')) linea[--L] = '\0';
        nl++;
        off = leer_marca(linea, &ms);
        if (off == 0) { if (L > 0) { unk_add(nl); printf("linea sin marca de tiempo (%d): %s\n", nl, linea); } continue; }
        s = linea + off;
        if (*s == '\0') continue;               /* linea en blanco */
        ultimaConLinea = nl;
        if (primeraConLinea == 0) primeraConLinea = nl;

        if (hayPend) {                          /* segundo trozo de una linea partida por un eco */
            char junta[1024];
            snprintf(junta, sizeof junta, "%s%s", pend, s);
            if (parse_resp(junta, &r, &n, &e)) {
                Ev *x = &ev[nev++];
                memset(x, 0, sizeof *x);
                x->ms = pendMs; x->linea = pendLinea; x->resp = r; x->respN = n; x->st = e; x->pegado = 1;
                pegadas++; hayPend = 0; continue;
            }
            unk_add(pendLinea); printf("linea partida que no se pudo reunir (%d): [%s] + [%s]\n", pendLinea, pend, s);
            hayPend = 0;
        }
        if (parse_eco(s, &c, &n)) {
            Ev *x = &ev[nev++]; memset(x, 0, sizeof *x);
            x->esEco = 1; x->ms = ms; x->linea = nl; x->cmd = c; x->cmdN = n; continue;
        }
        if (parse_resp(s, &r, &n, &e)) {
            Ev *x = &ev[nev++]; memset(x, 0, sizeof *x);
            x->ms = ms; x->linea = nl; x->resp = r; x->respN = n; x->st = e; continue;
        }
        {
            int p = eco_al_final(s);
            if (p > 0) {
                char pre[512];
                Ev *x = &ev[nev++]; memset(x, 0, sizeof *x);
                memcpy(pre, s, (size_t)p); pre[p] = '\0';
                if (!parse_eco(s + p, &c, &n)) { unk_add(nl); nev--; printf("eco no valido (%d): %s\n", nl, s); continue; }
                x->esEco = 1; x->ms = ms; x->linea = nl; x->cmd = c; x->cmdN = n;
                if (parse_resp(pre, &r, &n, &e)) {   /* el trozo anterior ya era una linea completa */
                    Ev *y = &ev[nev++]; memset(y, 0, sizeof *y);
                    y->ms = ms; y->linea = nl; y->resp = r; y->respN = n; y->st = e; y->pegado = 1;
                    /* la respuesta va antes del eco en el registro: intercambiar */
                    { Ev t = *x; *x = *y; *y = t; }
                    pegadas++;
                } else {
                    strcpy(pend, pre); pendMs = ms; pendLinea = nl; hayPend = 1;
                    /* el trozo pendiente se emite despues del eco: su evento se agrega al reunirse */
                }
                continue;
            }
        }
        /* linea desconocida: se tolera si es la primera o la ultima del registro (recorte al abrir o cerrar) */
        unk_add(nl);
        printf("linea no reconocida (%d): %s\n", nl, s);
    }
    fclose(f);
    /* La primera y la ultima linea del registro pueden estar recortadas (el registro se abre y se cierra
     * en mitad de una linea): se informan pero no cuentan como corruptas. */
    for (i = 0; i < nunk; i++) {
        if (unk[i] == primeraConLinea || unk[i] == ultimaConLinea) tolerada++;
        else desconocidas++;
    }
    if (hayPend) { if (pendLinea == ultimaConLinea) tolerada++; else desconocidas++; }

    /* ---- 2. pareja de cada comando con su respuesta ---- */
    {
        int nEco = 0, nResp = 0, mal = 0, lentas = 0, ambiguas = 0, estadoMal = 0, invMal = 0;
        double lat[MAX_EV]; int nlat = 0;
        double primerEco = 0, eco150 = 0; int eco150ok = 0;
        unsigned thr = 70; int pausa = 0; int tengoEstado = 0;
        int k;

        /* estado inicial: ultima linea de estado antes del primer comando */
        for (i = 0; i < nev && !ev[i].esEco; i++) {
            if (ev[i].resp == R_STATUS) { thr = ev[i].st.thr; pausa = ev[i].st.pausa; tengoEstado = 1; }
        }
        printf("Estado inicial tomado del registro: THR=%u, MODE=%s%s\n\n", thr, pausa ? "PAUSE" : "RUN", tengoEstado ? "" : " (supuesto)");
        printf("%-4s %-9s %-14s %-14s %-10s %s\n", "N", "Comando", "Hora del eco", "Hora respuesta", "Latencia", "Resultado");

        for (i = 0; i < nev; i++) {
            char buf[16], hh[32], hr[32];
            int j, cuenta = 0, idx = -1, amb = -1;
            const Ev *e = &ev[i];
            double lt; const char *res = "OK";
            if (!e->esEco) continue;
            nEco++;
            if (nEco == 1) primerEco = e->ms;
            if (nEco == CARGA_N) { eco150 = e->ms; eco150ok = 1; }

            /* respuestas entre este eco y el siguiente. Una linea de estado cuyo t no es multiplo de 1000
             * (con 2 ms de margen) solo puede ser respuesta a un STATUS; si t si lo es, puede ser la
             * telemetria del momento. */
            {
                int nEstado = 0, nNoPeriodica = 0, primeraEstado = -1, primeraNoPer = -1;
                for (j = i + 1; j < nev && !ev[j].esEco; j++) {
                    const Ev *r = &ev[j];
                    if (r->resp == R_STATUS) {
                        nEstado++;
                        if (primeraEstado < 0) primeraEstado = j;
                        if (r->st.t % 1000UL > 2UL) { nNoPeriodica++; if (primeraNoPer < 0) primeraNoPer = j; }
                        continue;
                    }
                    {
                        int esperada = 0;
                        switch (e->cmd) {
                        case CMD_THR:   esperada = (r->resp == R_THR); break;
                        case CMD_START: esperada = (r->resp == R_RUN); break;
                        case CMD_STOP:  esperada = (r->resp == R_PAUSE); break;
                        case CMD_OTRO:  esperada = (r->resp == R_ERR); break;
                        default: break;
                        }
                        if (esperada) { cuenta++; if (idx < 0) idx = j; }
                        else { mal++; printf("   respuesta inesperada tras el comando %d (linea %d)\n", nEco, r->linea); }
                    }
                }
                if (e->cmd == CMD_STATUS) {
                    if (nNoPeriodica >= 1) { cuenta = nNoPeriodica; idx = primeraNoPer; }
                    else if (nEstado == 2) { cuenta = 1; idx = primeraEstado; ambiguas++; }   /* respuesta y telemetria con t casi igual */
                    else if (nEstado == 1) { cuenta = 0; amb = primeraEstado; }
                    if (idx >= 0) ev[idx].usado = 1;
                } else if (nNoPeriodica > 0) {
                    mal++; printf("   linea de estado inesperada tras el comando %d\n", nEco);
                }
            }
            if (cuenta == 0 && amb >= 0) { res = "AMBIGUO: una sola linea de estado"; mal++; cuenta = -1; }
            if (cuenta == 0 || cuenta > 1) { mal++; res = cuenta == 0 ? "SIN RESPUESTA" : "RESPUESTAS DE MAS"; }
            sprintf(hh, "%02d:%02d:%06.3f", (int)(e->ms / 3600000.0), (int)fmod(e->ms / 60000.0, 60.0), fmod(e->ms / 1000.0, 60.0));
            if (idx >= 0) {
                const Ev *r = &ev[idx];
                lt = r->ms - e->ms;
                lat[nlat++] = lt;
                sprintf(hr, "%02d:%02d:%06.3f", (int)(r->ms / 3600000.0), (int)fmod(r->ms / 60000.0, 60.0), fmod(r->ms / 1000.0, 60.0));
                if (lt > LAT_MAX_MS) { lentas++; res = "LENTA"; }
                if (e->cmd == CMD_THR && r->resp == R_THR && r->respN != e->cmdN) { mal++; res = "THR DISTINTO"; }
                /* modelo de estado */
                if (e->cmd == CMD_STATUS && r->resp == R_STATUS) {
                    if (r->st.thr != thr || r->st.pausa != pausa) { estadoMal++; res = "ESTADO DISTINTO"; }
                }
                printf("%-4d %-9s %-14s %-14s %7.0f ms  %s%s\n", nEco, nom_cmd(e->cmd, e->cmdN, buf), hh, hr, lt, res, r->pegado ? " (linea reunida)" : "");
            } else {
                printf("%-4d %-9s %-14s %-14s %10s  %s\n", nEco, nom_cmd(e->cmd, e->cmdN, buf), hh, "-", "-", res);
            }
            if (e->cmd == CMD_THR && e->cmdN >= 10 && e->cmdN <= 95) thr = (unsigned)e->cmdN;
            if (e->cmd == CMD_STOP) pausa = 1;
            if (e->cmd == CMD_START) pausa = 0;
        }

        /* ---- 3. invariantes y telemetria ---- */
        {
            int nEst = 0, nTel = 0, huecos = 0, fuera = 0;
            unsigned long tPrev = 0; int tienePrev = 0;
            double pcPrimero = 0, pcUltimo = 0; unsigned long tPrimero = 0, tUltimo = 0;
            unsigned long dMin = 0xFFFFFFFFUL, dMax = 0;

            for (k = 0; k < nev; k++) {
                if (ev[k].esEco) continue;
                if (ev[k].resp == R_STATUS) {
                    nEst++;
                    if (ev[k].usado && ev[k].st.t % 1000UL <= 2UL) { /* respuesta con t casi multiplo de 1000: no es telemetria */ }
                    if (!invariantes(&ev[k].st)) { invMal++; printf("invariante incumplida (linea %d): t=%lu ADC=%u PCT=%u THR=%u %s ALARM=%d\n", ev[k].linea, ev[k].st.t, ev[k].st.adc, ev[k].st.pct, ev[k].st.thr, ev[k].st.pausa ? "PAUSE" : "RUN", ev[k].st.alarma); }
                    if (!ev[k].usado && ev[k].st.t % 1000UL <= 2UL) {
                        nTel++;
                        if (tienePrev) {
                            unsigned long d = ev[k].st.t - tPrev;
                            if (d < dMin) dMin = d;
                            if (d > dMax) dMax = d;
                            if (d != 1000UL) { huecos++; }
                            if ((double)d < 1000.0 * (1.0 - TOL_PCT / 100.0) || (double)d > 1000.0 * (1.0 + TOL_PCT / 100.0)) fuera++;
                        } else { pcPrimero = ev[k].ms; tPrimero = ev[k].st.t; }
                        tPrev = ev[k].st.t; tienePrev = 1; pcUltimo = ev[k].ms; tUltimo = ev[k].st.t;
                    }
                }
                nResp++;
            }

            qsort(lat, (size_t)nlat, sizeof lat[0], cmp_dbl);
            printf("\n==== Resumen ====\n");
            printf("Comandos enviados: %d; respuestas pareadas: %d; sin respuesta o de mas: %d; respuestas inesperadas incluidas en ese conteo\n", nEco, nlat, mal);
            if (nlat > 0) {
                double suma = 0; for (k = 0; k < nlat; k++) suma += lat[k];
                printf("Latencia (eco a respuesta, reloj del PC): minima %.0f ms, mediana %.0f ms, media %.1f ms, percentil 95 %.0f ms, maxima %.0f ms\n",
                       lat[0], lat[nlat / 2], suma / nlat, lat[(int)(0.95 * (nlat - 1))], lat[nlat - 1]);
            }
            printf("Respuestas con latencia mayor de %.0f ms: %d\n", LAT_MAX_MS, lentas);
            printf("STATUS cuyo THR o MODE no coincide con los comandos anteriores: %d\n", estadoMal);
            printf("Lineas de estado revisadas (telemetria y STATUS): %d; invariantes incumplidas (PCT, alarma en PAUSE, alarma con la histeresis): %d\n", nEst, invMal);
            printf("Lineas reunidas tras un eco local en medio: %d; STATUS atendidos con la telemetria en el mismo milisegundo: %d\n", pegadas, ambiguas);
            printf("Lineas corruptas o desconocidas: %d (recortes en la primera o la ultima linea del registro, aceptados: %d)\n", desconocidas, tolerada);
            printf("Telemetria: %d lineas; intervalo con el reloj de la placa de %lu a %lu ms; intervalos distintos de 1000 ms: %d; fuera de +-%.0f %%: %d\n",
                   nTel, nTel > 1 ? dMin : 0UL, nTel > 1 ? dMax : 0UL, huecos, TOL_PCT, fuera);
            if (nTel > 1) {
                double placa = (double)(tUltimo - tPrimero), pc = pcUltimo - pcPrimero;
                printf("Reloj de la placa contra reloj del PC (telemetria): placa %.0f ms, PC %.0f ms, diferencia %+.3f %%\n", placa, pc, (placa - pc) * 100.0 / pc);
            }
            if (eco150ok) {
                double dur = eco150 - primerEco;
                printf("Ritmo de la carga: %d comandos en %.2f s (del primero al %d); %.2f comandos por segundo\n", CARGA_N, dur / 1000.0, CARGA_N, (CARGA_N - 1) / (dur / 1000.0));
                if ((CARGA_N - 1) / (dur / 1000.0) < 5.0 - 0.25) ok = 0;
            } else {
                printf("Ritmo de la carga: hay menos de %d comandos en el registro\n", CARGA_N);
                ok = 0;
            }
            if (mal || lentas || estadoMal || invMal || desconocidas || fuera || nEco < CARGA_N) ok = 0;
        }
    }
    printf("\nRESULTADO: %s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
