/* console.h - Consola por USART2 (puerto serie virtual del ST-LINK), 115200 8N1 (RQ-09).
 *
 *  - Recepcion por interrupcion, un byte a la vez, hacia un buffer circular.
 *  - Transmision por interrupcion desde una cola circular. Ninguna funcion espera: si el mensaje
 *    no cabe entero en la cola, no se escribe nada (nunca quedan lineas a medias) y se cuenta.
 *  - Esta consola es la unica duena de USART2.
 */
#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t rxBytes;          /* bytes recibidos y aceptados por la interrupcion */
    uint32_t rxRingOverflow;   /* bytes perdidos porque el buffer de RX estaba lleno */
    uint32_t uartOverrun;      /* errores ORE del hardware */
    uint32_t uartFraming;      /* errores de trama (FE) */
    uint32_t uartNoise;        /* errores de ruido (NE) */
    uint32_t uartParity;       /* errores de paridad (PE) */
    uint32_t txBytes;          /* bytes enviados */
    uint32_t txDropped;        /* mensajes descartados por cola de TX llena */
} ConsoleStats;

void Console_Init(void);       /* despues de MX_USART2_UART_Init() */
void Console_Service(void);    /* en cada vuelta de main: rearma la recepcion y reintenta la transmision */

bool Console_ReadByte(uint8_t *b);                       /* no bloquea */
bool Console_WriteBytes(const uint8_t *p, uint16_t n);   /* todo o nada; no bloquea */
bool Console_Write(const char *s);                       /* todo o nada; no bloquea */

void Console_GetStats(ConsoleStats *out);

/* Espacio libre de la cola de TX: permite saber si una linea cabe antes de generarla. */
uint16_t Console_TxFree(void);

/* Suma de todo lo que pudo perder o danar bytes de RX (buffer lleno y errores ORE, FE, NE, PE).
 * Solo crece: quien la lee detecta un cambio y descarta la linea afectada. */
uint32_t Console_RxFaultCount(void);

#endif /* CONSOLE_H */
