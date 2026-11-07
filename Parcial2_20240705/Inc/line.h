/* line.h - Armado de lineas de comando a partir de los bytes recibidos (RQ-10).
 * No usa el HAL ni espera: recibe un byte y devuelve un evento.
 *
 * Reglas (docs/arquitectura.md, protocolo UART):
 *  - El terminador es LF. Un CR justo antes del LF se descarta (CRLF) y no cuenta en el largo.
 *    Un CR en cualquier otro lugar invalida la linea.
 *  - Hasta APP_LINE_MAX (32) caracteres sin contar el terminador. Con uno mas, la linea se descarta
 *    completa hasta el LF y produce un solo LINE_ERROR; la siguiente linea empieza limpia.
 *  - Solo son validos los caracteres imprimibles (0x20 a 0x7E). Cualquier otro, incluido el
 *    tabulador, invalida la linea.
 *  - Una linea vacia (solo LF o solo CRLF) produce LINE_EMPTY: no es un comando y no se responde.
 *  - Line_Poison() lo usa quien detecta que RX perdio bytes: la linea en curso se descarta y
 *    produce LINE_ERROR al llegar su LF.
 */
#ifndef LINE_H
#define LINE_H

#include <stdint.h>
#include <stdbool.h>
#include "app_config.h"

typedef enum {
    LINE_NONE = 0,      /* byte consumido; todavia no hay linea completa */
    LINE_READY,         /* LF: Line_Text() tiene una linea valida y no vacia */
    LINE_EMPTY,         /* LF de una linea vacia: se ignora */
    LINE_ERROR          /* LF de una linea invalida (larga, con caracteres no validos o danada): responder ERR */
} LineEvent;

typedef struct {
    char    buf[APP_LINE_MAX + 1U];
    uint8_t len;
    bool    ready;      /* buf tiene una linea entregada; se limpia con el siguiente byte */
    bool    pendingCR;  /* el byte anterior fue CR */
    bool    tooLong;
    bool    bad;        /* caracter no valido o CR fuera de lugar */
    bool    damaged;    /* RX perdio bytes durante esta linea */
} LineReader;

void        Line_Init(LineReader *lr);
LineEvent   Line_Feed(LineReader *lr, uint8_t c);
const char *Line_Text(const LineReader *lr);     /* valido tras LINE_READY, hasta el siguiente Line_Feed */
void        Line_Poison(LineReader *lr);

#endif /* LINE_H */
