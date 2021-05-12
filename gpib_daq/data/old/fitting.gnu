load 'temp_macro.gnu'

g = 8.802e-6
ta = 293.433563 # Ambient temperature in Kelvin

pwr(v,i) = v * i
tmp(v,i) = ts((g*v/i)*1000000)
sbl(t) = ac * 1.0e-14 * (t**a - ta**a) # Stefan-Boltzmann
fou(t) = bc * 1e-3 * (t - ta)**b # Fourier

a = 4.0
b = 1.0
ac = 0.75
bc = 0.9
