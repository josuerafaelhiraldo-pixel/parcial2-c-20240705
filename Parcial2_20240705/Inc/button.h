/* button.h - Pulsador B1 con antirrebote de 30 ms (RQ-06). */
#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

typedef enum {
    BTN_EVT_NONE = 0,
    BTN_EVT_PRESS,      /* pulsacion validada: una sola vez por pulsacion */
    BTN_EVT_RELEASE     /* liberacion validada */
} ButtonEvent;

/* Lee el nivel actual como estado inicial. Si B1 ya estaba pulsado al arrancar, no se
 * genera ningun evento hasta que se suelte y se vuelva a pulsar. */
void Button_Init(uint32_t now_ms);

/* Se llama cada APP_BTN_SAMPLE_MS (1 ms). Un cambio de nivel solo se acepta cuando el
 * nuevo nivel se mantiene estable APP_DEBOUNCE_MS (30 ms), tanto al pulsar como al
 * soltar. No hay repeticion: mientras siga pulsado, no sale otro evento. */
ButtonEvent Button_Service(uint32_t now_ms);

#endif /* BUTTON_H */
