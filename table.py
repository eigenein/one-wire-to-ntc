#!/usr/bin/env python3

import math

# Model parameters for NTC 10k.
# See also: http://www.giangrandi.ch/electronics/ntc/ntc.shtml
R25 = 9979.65  # Ohm, NTC 25°C.
BETA = 3920.69  # Kelvin, NTC sensitivity.
T25 = 298.15  # Kelvin, 25°C.
T25_INV = 1 / T25
R_FIXED = 10000 # Ohm, fixed resistor.

print("#ifndef NTC_TABLE_H_")
print("#define NTC_TABLE_H_")
print()
print("const static float TABLE[] PROGMEM = {")

# Iterating over 12-bit precision values.
for i in range(0x1000):
    signed_i = (i & 0x7FF) - (i & 0x800)  # sign extension, the top-most bit is a sign
    temperature_c = signed_i / 16  # the LSB is `2 ** -4`
    temperature_k = temperature_c + 273.15
    resistance = R25 * math.exp(BETA * (1 / temperature_k - T25_INV))
    voltage = resistance / (resistance + R_FIXED)  # voltage scaled to 0..1
    adc_value = int(voltage * 4095)  # scale to MCP4725
    print(f"    {adc_value},  // #{i:04X} {signed_i:5d} {temperature_c:.4f}°C R: {resistance:.0f} V: {voltage:.3f}")

print("};")
print()
print("#endif")
