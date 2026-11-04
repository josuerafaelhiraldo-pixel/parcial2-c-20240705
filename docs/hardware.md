# Hardware: pines, conexiones, VREF y ADC

Placa: STM32 NUCLEO-F446RE (microcontrolador STM32F446RE). Se reutiliza el montaje del primer parcial: LED externo con resistencia de 1 kΩ en PA6 y LD2 de la placa. Las polaridades de LD2 y B1 se midieron en la placa durante el primer parcial.

## Mapa de pines

| Función | Pin | Conector | Modo y nivel activo | Periférico | Reloj |
| --- | --- | --- | --- | --- | --- |
| Potenciómetro (cursor) | PA0 | CN8, pin 1 (A0) | Entrada analógica | ADC1, canal IN0 | 84 MHz / 4 = 21 MHz |
| LED de actividad | PA5 | LD2 integrado | Salida; activo en alto | GPIO | |
| LED de alarma | PA6 | CN5, pin 5 (D12), LED externo con 1 kΩ | Salida; activo en alto | GPIO | |
| Pulsador | PC13 | B1 integrado | Entrada con pull-up; activo en bajo (reposo = 1) | GPIO, muestreo por TIM6 | |
| UART TX | PA2 | Puente ST-LINK (USB) | USART2 | USART2 | |
| UART RX | PA3 | Puente ST-LINK (USB) | USART2 | USART2 | |
| Base de tiempo | | | | TIM6 | 84 MHz |

Los números de pin de los conectores (CN5, CN6, CN8) se verifican contra el manual de usuario de la placa (UM1724) antes de cablear.

## Esquema de conexiones

```
 Potenciómetro 10 kΩ                       LED de alarma externo

 3V3 (CN6, pin 4) ──────┐                  PA6 (CN5, pin 5, D12)
                        │                       │
                      ┌─┴─┐                  ┌──┴──┐
                      │   │                  │ 1 kΩ│
 PA0 / A0 ◄───────────┤ ◄─┤ cursor           └──┬──┘
 (CN8, pin 1)         │   │                     │
                      └─┬─┘                  ▲  │  ánodo (pata larga)
                        │                    │  │
 GND (CN6, pin 6) ──────┘                   ─┴─ │  cátodo
                                             │
                                  GND (CN5, pin 7) ┘

 LD2 (PA5) y B1 (PC13) son de la placa; no llevan cableado.
 UART: cable USB de la placa (ST-LINK) hacia la computadora; aparece como un puerto COM.
```

Precauciones:

- El potenciómetro se alimenta con 3V3, nunca con 5 V: la entrada PA0 admite como máximo VREF+ (3,3 V).
- Retire el puente PA6 → PA0 del primer parcial. Con ese puente, el LED de alarma inyectaría su señal en la entrada del ADC.
- El LED externo siempre lleva su resistencia de 1 kΩ.

## Referencia, resolución y canal del ADC

| Parámetro | Valor |
| --- | --- |
| Convertidor | ADC1 |
| Canal | IN0 (PA0) |
| Resolución N | 12 bits; rango del dato 0 a 4095 (2^12 − 1) |
| Alineación | Derecha |
| VREF+ | VDDA = 3,3 V nominal. Se mide con el multímetro en el pin 3V3 y se registra en `pruebas/resultados.md` |
| Rango de entrada | 0 V a VREF+ |
| LSB | 3,3 V / 4095 ≈ 0,806 mV |
| Disparo | Por software, una vez cada 100 ms desde la tarea de adquisición |
| Fin de conversión | Interrupción de EOC; el callback solo guarda el dato y levanta una bandera |
| Reloj del ADC | PCLK2 / 4 = 21 MHz (el límite es 36 MHz) |
| Tiempo de muestreo | 144 ciclos |
| Tiempo de conversión | (144 + 12) / 21 MHz ≈ 7,43 µs |

El tiempo de muestreo largo sirve para el potenciómetro de 10 kΩ, que como fuente tiene una impedancia de hasta 2,5 kΩ vista desde el cursor (en el punto medio). El valor de impedancia máxima recomendada y el límite de reloj del ADC se confirman en la hoja de datos del STM32F446RE.

## Conversión a porcentaje

```
PCT = (100 × ADC + MAX / 2) / MAX        con MAX = 2^N − 1 = 4095 (división entera)
```

Se calcula en `uint32_t`: el mayor valor intermedio es 100 × 4095 + 2047 = 411 547, que no cabe en 16 bits pero sí en 32. Como MAX es impar, el valor 100 × ADC / MAX nunca cae exactamente en .5, así que no hay empates de redondeo.

`pruebas/herramientas/verificar_pct.c` (programa de PC en C, se compila con `gcc`) compara esta expresión entera con el redondeo exacto para los 4096 valores posibles: 0 diferencias (salida en `pruebas/herramientas/verificar_pct_salida.txt`). También confirma el ejemplo del mandato: ADC = 2866 → PCT = 70.

| ADC | PCT |
| --- | --- |
| 0 | 0 |
| 20 | 0 |
| 21 | 1 |
| 1024 | 25 |
| 2047 | 50 |
| 2048 | 50 |
| 2866 | 70 |
| 4095 | 100 |

Cada punto porcentual equivale a unos 41 valores del ADC. Los niveles de la prueba P04 tienen margen amplio: PCT = 60 corresponde a ADC de 2437 a 2477, PCT = 62 a 2519 a 2559, PCT = 67 a 2724 a 2764 y PCT = 72 a 2928 a 2968.

## Exactitud esperada (P03)

La prueba P03 exige que PCT coincida con 100 × Vin / VREF dentro de ±3 puntos porcentuales, es decir, unos 123 valores del ADC o 99 mV. El error del ADC es de pocos LSB, de modo que la contribución dominante será la lectura del multímetro y el valor real de VREF+. Por eso Vin y VREF se miden con el mismo multímetro en cada punto.
