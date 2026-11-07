/* cmd.c - Analisis estricto de comandos y formato de las lineas de salida (RQ-11 a RQ-14). */
#include "cmd.h"
#include "app_config.h"
#include <stdbool.h>

static bool str_equal(const char *a, const char *b)
{
    while ((*a != '\0') && (*a == *b)) {
        a++;
        b++;
    }
    return (*a == *b);
}

void Cmd_Parse(const char *line, Cmd *out)
{
    out->type = CMD_ERR;
    out->value = 0U;

    if (str_equal(line, "STATUS")) {
        out->type = CMD_STATUS;
    } else if (str_equal(line, "START")) {
        out->type = CMD_START;
    } else if (str_equal(line, "STOP")) {
        out->type = CMD_STOP;
    } else if ((line[0] == 'T') && (line[1] == 'H') && (line[2] == 'R') && (line[3] == ' ')) {
        const char *p = &line[4];
        uint32_t value = 0U;
        uint8_t digits = 0U;

        while ((*p >= '0') && (*p <= '9')) {
            if (digits >= APP_THR_MAX_DIGITS) {
                return;                         /* mas de 3 digitos: ERR, sin riesgo de desbordar */
            }
            value = (value * 10U) + (uint32_t)(*p - '0');
            digits++;
            p++;
        }
        if ((digits == 0U) || (*p != '\0')) {
            return;                             /* sin digitos, o con algo despues de ellos */
        }
        if ((value < APP_THR_MIN_PCT) || (value > APP_THR_MAX_PCT)) {
            return;
        }
        out->type = CMD_THR;
        out->value = value;
    }
}

/* ---- escritura acotada de texto ---- */
typedef struct {
    char    *p;
    char    *end;
    bool     ok;
} Out;

static void out_ch(Out *o, char c)
{
    if (o->p < o->end) {
        *o->p = c;
        o->p++;
    } else {
        o->ok = false;
    }
}

static void out_str(Out *o, const char *s)
{
    while (*s != '\0') {
        out_ch(o, *s);
        s++;
    }
}

static void out_u32(Out *o, uint32_t v)
{
    char tmp[10];
    uint8_t n = 0U;

    do {
        tmp[n] = (char)('0' + (char)(v % 10U));
        n++;
        v /= 10U;
    } while (v != 0U);
    while (n > 0U) {
        n--;
        out_ch(o, tmp[n]);
    }
}

uint16_t Cmd_FormatStatus(char *buf, uint16_t size, const StatusData *s)
{
    Out o = { buf, buf + size, true };

    out_str(&o, "t=");
    out_u32(&o, s->t_ms);
    out_str(&o, ";ADC=");
    out_u32(&o, s->adc);
    out_str(&o, ";PCT=");
    out_u32(&o, s->pct);
    out_str(&o, ";THR=");
    out_u32(&o, s->thr);
    out_str(&o, ";MODE=");
    out_str(&o, (s->paused != 0U) ? "PAUSE" : "RUN");
    out_str(&o, ";ALARM=");
    out_ch(&o, (s->alarm != 0U) ? '1' : '0');
    out_str(&o, "\r\n");
    return o.ok ? (uint16_t)(o.p - buf) : 0U;
}

uint16_t Cmd_FormatThrOk(char *buf, uint16_t size, uint32_t thr)
{
    Out o = { buf, buf + size, true };

    out_str(&o, "OK THR=");
    out_u32(&o, thr);
    out_str(&o, "\r\n");
    return o.ok ? (uint16_t)(o.p - buf) : 0U;
}
