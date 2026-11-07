/* cmd.h - Analisis de comandos y formato de las lineas de salida (RQ-11 a RQ-14).
 * Funciones puras: no usan el HAL ni la UART, por eso se prueban en el PC. */
#ifndef CMD_H
#define CMD_H

#include <stdint.h>

typedef enum {
    CMD_ERR = 0,        /* cualquier linea no valida */
    CMD_STATUS,
    CMD_THR,            /* value = n, ya validado entre 10 y 95 */
    CMD_START,
    CMD_STOP
} CmdType;

typedef struct {
    CmdType  type;
    uint32_t value;
} Cmd;

/* Analiza una linea (sin terminador). Solo se aceptan, escritos exactamente:
 *   STATUS | START | STOP | THR n   con n de 1 a 3 digitos decimales y valor de 10 a 95.
 * Todo lo demas da CMD_ERR: espacios sobrantes, minusculas, signos, decimales, texto extra. */
void Cmd_Parse(const char *line, Cmd *out);

/* Datos de la linea de telemetria y de STATUS. */
typedef struct {
    uint32_t t_ms;
    uint16_t adc;
    uint32_t pct;
    uint32_t thr;
    uint8_t  paused;    /* 0 = RUN, 1 = PAUSE */
    uint8_t  alarm;     /* 0 o 1 */
} StatusData;

/* Escribe "t=...;ADC=...;PCT=...;THR=...;MODE=RUN|PAUSE;ALARM=0|1\r\n" en buf y devuelve el
 * largo (con CRLF). Devuelve 0 si no cabe en size bytes. No escribe el NUL. */
uint16_t Cmd_FormatStatus(char *buf, uint16_t size, const StatusData *s);

/* Escribe "OK THR=n\r\n". Devuelve el largo o 0 si no cabe. */
uint16_t Cmd_FormatThrOk(char *buf, uint16_t size, uint32_t thr);

/* Respuestas fijas, todas terminadas en CRLF. */
#define CMD_REPLY_RUN   "OK RUN\r\n"
#define CMD_REPLY_PAUSE "OK PAUSE\r\n"
#define CMD_REPLY_ERR   "ERR\r\n"

#endif /* CMD_H */
