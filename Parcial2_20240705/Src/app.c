/* app.c - Une los modulos de la aplicacion.
 * Los modulos se agregan por hitos (docs/plan_de_trabajo.md). */
#include "app.h"
#include "app_config.h"
#include "tick.h"
#include "sched.h"
#include "button.h"
#include "alarm.h"
#include "adc_in.h"
#include "console.h"
#include "line.h"
#include "cmd.h"

static uint32_t   s_tLed;       /* instante programado del proximo cambio del LED de actividad */
static uint32_t   s_tBtn;       /* instante programado del proximo muestreo del pulsador */
static uint32_t   s_tAdc;       /* instante programado de la proxima conversion del ADC */
static uint32_t   s_tTel;       /* instante programado de la proxima telemetria */
static uint16_t   s_lastRaw;    /* ultimo valor del ADC, tomado junto con su porcentaje */
static LineReader s_line;
static uint32_t   s_rxFaults;   /* ultimo valor visto de Console_RxFaultCount */
static uint32_t   s_telSkipped; /* telemetrias omitidas porque no cabian en la cola de TX */

/* Llena los datos de STATUS y de la telemetria con el estado actual. Los dos usan el mismo formato. */
static uint16_t build_status(char *buf, uint16_t size, uint32_t now)
{
    StatusData s;

    s.t_ms   = now;
    s.adc    = s_lastRaw;
    s.pct    = Alarm_GetPct();
    s.thr    = Alarm_GetThr();
    s.paused = (Alarm_GetMode() == ALARM_MODE_PAUSE) ? 1U : 0U;
    s.alarm  = Alarm_IsOn();
    return Cmd_FormatStatus(buf, size, &s);
}

/* Ejecuta un comando completo y encola su unica respuesta (RQ-11 a RQ-13). */
static void run_command(const char *text, uint32_t now)
{
    char out[APP_LINE_OUT_MAX];
    uint16_t n;
    Cmd c;

    Cmd_Parse(text, &c);
    switch (c.type) {
    case CMD_STATUS:
        n = build_status(out, (uint16_t)sizeof(out), now);
        (void)Console_WriteBytes((const uint8_t *)out, n);
        break;
    case CMD_THR:
        (void)Alarm_SetThr(c.value);       /* ya viene validado: reevalua la alarma */
        n = Cmd_FormatThrOk(out, (uint16_t)sizeof(out), c.value);
        (void)Console_WriteBytes((const uint8_t *)out, n);
        break;
    case CMD_START:
        Alarm_SetMode(ALARM_MODE_RUN);
        (void)Console_Write(CMD_REPLY_RUN);
        break;
    case CMD_STOP:
        Alarm_SetMode(ALARM_MODE_PAUSE);   /* apaga la alarma al instante */
        (void)Console_Write(CMD_REPLY_PAUSE);
        break;
    default:
        (void)Console_Write(CMD_REPLY_ERR);
        break;
    }
}

/* Lee los bytes recibidos. Antes de leer cada uno comprueba que cabe en la cola de TX la mayor
 * respuesta posible: asi ninguna respuesta se pierde, y si la cola esta llena el byte espera
 * en el buffer de RX. */
static void serve_console(uint32_t now)
{
    uint32_t faults = Console_RxFaultCount();
    uint8_t  b;

    if (faults != s_rxFaults) {
        s_rxFaults = faults;
        Line_Poison(&s_line);               /* se perdieron bytes: la linea en curso no es confiable */
    }

    while ((Console_TxFree() >= APP_LINE_OUT_MAX) && (Console_ReadByte(&b) != false)) {
        switch (Line_Feed(&s_line, b)) {
        case LINE_READY:
            run_command(Line_Text(&s_line), now);
            break;
        case LINE_ERROR:
            (void)Console_Write(CMD_REPLY_ERR);
            break;
        default:                            /* LINE_NONE y LINE_EMPTY: no hay nada que responder */
            break;
        }
    }
}

void App_Init(void)
{
    Tick_Init();            /* arranca TIM6: desde aqui el contador cuenta milisegundos */
    uint32_t now = Tick_Ms();

    s_tLed = now;
    s_tBtn = now;
    s_tTel = now;
    s_tAdc = now - APP_ADC_PERIOD_MS;   /* la primera conversion se inicia en la primera vuelta */
    Alarm_Init();           /* RUN y alarma apagada despues del reinicio */
    Button_Init(now);
    Line_Init(&s_line);
    Console_Init();         /* USART2: recepcion por interrupcion y cola de transmision */
    s_rxFaults = Console_RxFaultCount();
}

void App_Loop(void)
{
    uint32_t now = Tick_Ms();
    uint16_t raw;

    Console_Service();

    /* Pulsador: se muestrea cada 1 ms; una pulsacion valida alterna RUN y PAUSE (RQ-06). */
    if (Sched_Due(&s_tBtn, APP_BTN_SAMPLE_MS, now) != 0U) {
        if (Button_Service(now) == BTN_EVT_PRESS) {
            Alarm_ToggleMode();
        }
    }

    /* ADC: una conversion cada 100 ms (RQ-02). Termina por interrupcion; aqui solo se inicia. */
    if (Sched_Due(&s_tAdc, APP_ADC_PERIOD_MS, now) != 0U) {
        (void)AdcIn_Start();
    }

    /* Muestra nueva: se convierte a porcentaje fuera de la interrupcion (RQ-03) y la alarma
     * decide con ella (RQ-07, RQ-08). */
    if (AdcIn_Take(&raw) != 0U) {
        s_lastRaw = raw;
        Alarm_OnSample(APP_PCT_FROM_ADC(raw));
    }

    /* Comandos: las respuestas se encolan antes que la telemetria (RQ-11 a RQ-14). */
    serve_console(now);

    /* LED de actividad: cambia de estado cada 500 ms (RQ-02). Sigue activo en PAUSE. */
    if (Sched_Due(&s_tLed, APP_LED_ACT_PERIOD_MS, now) != 0U) {
        HAL_GPIO_TogglePin(APP_LED_ACT_PORT, APP_LED_ACT_PIN);
    }

    /* Telemetria: una linea cada 1000 ms (RQ-02, RQ-14). Solo se encola si cabe entera y deja
     * sitio para una respuesta; si no, se omite y no se mezcla con nada. */
    if (Sched_Due(&s_tTel, APP_TELEMETRY_PERIOD_MS, now) != 0U) {
        char out[APP_LINE_OUT_MAX];
        uint16_t n = build_status(out, (uint16_t)sizeof(out), now);

        if ((n != 0U) && (Console_TxFree() >= (uint16_t)(n + APP_LINE_OUT_MAX))) {
            (void)Console_WriteBytes((const uint8_t *)out, n);
        } else {
            s_telSkipped++;
        }
    }
}
