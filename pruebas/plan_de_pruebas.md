# Plan de pruebas P01 a P10

Cada prueba registra el resultado esperado, el observado, PASS o FAIL y una evidencia (`pruebas/resultados.md`). Una prueba sin evidencia no se marca como aprobada. Cualquier FAIL se explica.

## Preparación común

- Placa conectada por USB. Tera Term en el puerto COM del ST-LINK: 115200, 8 bits, sin paridad, 1 bit de parada, sin control de flujo.
- Tera Term, Setup → Terminal → New-line → **Transmit: LF** (o CR+LF). Con el valor predeterminado (CR) los comandos no terminan en LF y la placa no los ejecuta.
- Registro de Tera Term con marca de tiempo activada, un archivo por prueba en `evidencias/`.
- Multímetro para medir la tensión del potenciómetro (Vin) y VREF+ (pin 3V3).
- Puente PA6 → PA0 del primer parcial retirado.

## Procedimientos

| ID | Procedimiento | Esperado | Cómo se mide | Evidencia prevista |
| --- | --- | --- | --- | --- |
| P01 | Compilar, cargar, reiniciar con el potenciómetro cerca del mínimo; enviar `STATUS` a los 200 ms | Compila y arranca. `MODE=RUN`, `THR=70`, `ALARM=0`, PCT cercano a 0 | Registro con la respuesta; captura de la compilación | `P01_arranque.log`, `P01_compilacion.png` |
| P02 | Durante 10 s observar el LED de actividad y los eventos de adquisición; registrar 10 líneas periódicas sin enviar comandos | Adquisición cada 100 ms, LED cada 500 ms, telemetría cada 1000 ms; tolerancia ±5 % | Marcas de tiempo de las líneas de traza y de la telemetría (no el parpadeo a simple vista) | `P02_tiempos.log`, tabla de intervalos |
| P03 | Aplicar aproximadamente 0 %, 50 % y 100 % de VREF; medir Vin; esperar 300 ms por punto | PCT = 100 × Vin / VREF ±3 puntos; ADC dentro de 0 a 4095; redondeo correcto | Multímetro y respuesta de `STATUS` | `P03_niveles.log`, fotos del multímetro |
| P04 | Con THR = 70, llevar PCT a 60, 72, 67 y 62 (≥ 300 ms por nivel) | ALARM = 0, 1, 1, 0 según el PCT mostrado | `STATUS` en cada nivel | `P04_histeresis.log` |
| P05 | 5 pulsaciones separadas; mantener una 2 s y liberar; si es posible, rebotes de menos de 30 ms | Una alternancia por pulsación; ninguna repetición al mantener; los rebotes no alternan | Registro de modo con `STATUS` tras cada pulsación | `P05_boton.log` |
| P06 | Con PCT ≈ 80 enviar `STOP`; mover el potenciómetro; `STATUS`; luego `START` con PCT ≈ 80 | En PAUSE la alarma está apagada y ADC, actividad y telemetría continúan; `START` vuelve a RUN y enciende la alarma | Registro con `STATUS` y telemetría | `P06_pause_run.log` |
| P07 | En RUN con PCT ≈ 60 y THR = 70: `THR 50`, `THR 80`; además `THR 10` y `THR 95` | `OK THR=n`; con 50 la alarma enciende; con 80 apaga; se aceptan los dos límites | Registro | `P07_umbral.log` |
| P08 | `THR 9`, `THR 96`, `THR abc`, `THR 50x`, `THR 50 extra` y un comando desconocido | Un `ERR` por línea; THR y modo sin cambios; la aplicación sigue funcionando | Registro, `STATUS` al final | `P08_errores.log` |
| P09 | `STATUS` fragmentado en dos envíos; LF y CRLF; una línea de 33 caracteres con LF y luego `STATUS` | Cada comando completo responde una vez; la línea larga da `ERR`; el `STATUS` siguiente funciona; no se ejecutan fragmentos | Registro | `P09_fragmentos.log` |
| P10 | 30 s con `STATUS` a 5 comandos/s, variando la entrada y alternando el modo; medir latencias | Sin bloqueo ni líneas corruptas; respuestas ≤ 100 ms; tiempos periódicos dentro de ±5 %; modo y alarma según las reglas | Macro de Tera Term y análisis de las marcas de tiempo | `P10_carga.log`, `P10_latencias.txt` |

## Notas de medición

- **P02:** se mide el instante del evento o el inicio de la línea serial, no el parpadeo. Para los eventos de 100 ms y 500 ms se usará una traza de diagnóstico documentada (ver «Decisiones abiertas» en `docs/arquitectura.md`).
- **P03:** Vin y VREF+ se miden en cada punto con el mismo multímetro; la tolerancia es de ±3 puntos porcentuales.
- **P04:** los niveles dan margen: cada punto porcentual equivale a unos 41 valores del ADC (ver `docs/hardware.md`). Se ajusta según el PCT mostrado, no según la posición del potenciómetro.
- **P10:** las marcas de tiempo del registro tienen la resolución de la terminal y el jitter de Windows; ambos son pequeños frente a 100 ms. Si hay traza del lado de la placa (instante del LF recibido y de la respuesta encolada), se usa como segunda medición.
