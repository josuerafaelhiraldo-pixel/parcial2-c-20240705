/* line.c - Armado de lineas de comando (RQ-10). Maquina de estados de un byte por llamada. */
#include "line.h"

static void line_reset(LineReader *lr)
{
    lr->len = 0U;
    lr->buf[0] = '\0';
    lr->ready = false;
    lr->pendingCR = false;
    lr->tooLong = false;
    lr->bad = false;
    lr->damaged = false;
}

void Line_Init(LineReader *lr)
{
    line_reset(lr);
}

const char *Line_Text(const LineReader *lr)
{
    return lr->buf;
}

void Line_Poison(LineReader *lr)
{
    if (lr->ready) {
        line_reset(lr);         /* la linea entregada ya se uso; la danada es la siguiente */
    }
    lr->damaged = true;
}

LineEvent Line_Feed(LineReader *lr, uint8_t c)
{
    LineEvent ev;

    if (lr->ready) {
        line_reset(lr);         /* el texto anterior ya se proceso */
    }

    if (c == (uint8_t)'\n') {
        if (lr->tooLong || lr->bad || lr->damaged) {
            ev = LINE_ERROR;
        } else if (lr->len == 0U) {
            ev = LINE_EMPTY;
        } else {
            lr->buf[lr->len] = '\0';
            lr->ready = true;   /* el texto queda valido hasta el siguiente byte */
            return LINE_READY;
        }
        line_reset(lr);
        return ev;
    }

    if (lr->pendingCR) {
        lr->bad = true;         /* un CR que no iba justo antes del LF */
        lr->pendingCR = false;
    }

    if (c == (uint8_t)'\r') {
        lr->pendingCR = true;
        return LINE_NONE;
    }

    if ((c < 0x20U) || (c > 0x7EU)) {
        lr->bad = true;
        return LINE_NONE;
    }

    if (lr->tooLong) {
        return LINE_NONE;       /* se descarta hasta el LF */
    }
    if (lr->len >= APP_LINE_MAX) {
        lr->tooLong = true;     /* el caracter 33 */
        return LINE_NONE;
    }
    lr->buf[lr->len] = (char)c;
    lr->len++;
    return LINE_NONE;
}
