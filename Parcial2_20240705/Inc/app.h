/* app.h - Punto de entrada de la aplicacion. main.c solo llama a estas dos funciones. */
#ifndef APP_H
#define APP_H

/* Se llama una vez, despues de que CubeMX inicializa los perifericos. */
void App_Init(void);

/* Se llama en cada vuelta de while(1). No bloquea: no usa retardos. */
void App_Loop(void);

#endif /* APP_H */
