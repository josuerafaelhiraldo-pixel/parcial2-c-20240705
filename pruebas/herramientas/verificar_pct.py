#!/usr/bin/env python3
"""Verifica la formula de porcentaje del mandato para todos los valores del ADC.

Mandato: PCT = redondear(100 * ADC / (2^N - 1)).
Implementacion prevista en el firmware (solo enteros, sin flotantes):

    PCT = (100 * ADC + (MAX / 2)) / MAX        con MAX = 2^N - 1

Este script compara esa expresion entera con el redondeo exacto (aritmetica de
fracciones, sin errores de coma flotante) para cada valor posible del ADC, y
comprueba que no hay desbordamiento en 32 bits. Solo usa la biblioteca estandar.

Uso:  python verificar_pct.py [N]      (N = resolucion en bits, por defecto 12)
"""
import sys
from fractions import Fraction

N = int(sys.argv[1]) if len(sys.argv) > 1 else 12
MAX = (1 << N) - 1


def pct_entero(adc: int) -> int:
    return (100 * adc + MAX // 2) // MAX


def pct_exacto(adc: int) -> int:
    """Redondeo al entero mas cercano; los empates (imposibles, ver abajo) subirian."""
    x = Fraction(100 * adc, MAX)
    return int(x + Fraction(1, 2))  # floor(x + 1/2) para x >= 0


errores = [a for a in range(MAX + 1) if pct_entero(a) != pct_exacto(a)]
empates = [a for a in range(MAX + 1) if (200 * a) % MAX == 0 and (100 * a) % MAX != 0
           and ((200 * a) // MAX) % 2 == 1]
monotona = all(pct_entero(a) <= pct_entero(a + 1) for a in range(MAX))
maximo_intermedio = 100 * MAX + MAX // 2

print(f"Resolucion N = {N} bits, MAX = {MAX}")
print(f"Valores comprobados: {MAX + 1}")
print(f"Diferencias entre formula entera y redondeo exacto: {len(errores)}")
print(f"Empates exactos (x.5) posibles: {len(empates)}  (MAX es impar, por eso no hay)")
print(f"Funcion no decreciente: {monotona}")
print(f"PCT(0) = {pct_entero(0)}, PCT(MAX) = {pct_entero(MAX)}")
print(f"Mayor valor intermedio 100*ADC + MAX/2 = {maximo_intermedio}")
print(f"  cabe en uint32 (< {2**32}): {maximo_intermedio < 2**32}")
print(f"  cabe en uint16 (< {2**16}): {maximo_intermedio < 2**16}  -> por eso se usa uint32")
print()
print("Puntos de referencia (N = 12)" if N == 12 else "Puntos de referencia")
for a in (0, 1, 20, 21, 1024, 2047, 2048, 2866, 3071, 3072, MAX):
    if a <= MAX:
        print(f"  ADC = {a:5d} -> PCT = {pct_entero(a):3d}   (100*ADC/MAX = {100 * a / MAX:7.3f})")
print()
if N == 12:
    # Valores del mandato: con ADC = 2866 el ejemplo de telemetria da PCT = 70.
    assert pct_entero(2866) == 70
    print("Ejemplo del mandato (ADC = 2866 -> PCT = 70): coincide")
    # Primer ADC de cada porcentaje (util para elegir niveles de prueba P04).
    primeros = {}
    for a in range(MAX + 1):
        primeros.setdefault(pct_entero(a), a)
    for p in (60, 62, 67, 72):
        print(f"  PCT = {p}: ADC desde {primeros[p]} hasta {max(a for a in range(MAX + 1) if pct_entero(a) == p)}")
sys.exit(1 if (errores or not monotona or maximo_intermedio >= 2**32) else 0)
