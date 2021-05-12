#load 'tungsten_temp_macro.gnu'

# Geometric factor (in 1/cm) for the tungsten wire, change as appropriate
g = 8.802e-6
# Ambient temperature in Kelvin, change in gnuplot with the correct value
ta = 293.433563 

#Power from voltage and current measurements
pwr(v,i) = v * i
#Convert resistivity to temperature (based on fits to the data from 1926 Jones' paper)
tmp(v,i) = ts((g*v/i)*1000000)
#Stefan-Boltzmann component
sbl(t) = ac * 1.0e-14 * (t**a - ta**a)
#Fourier dissipation component
fou(t) = bc * 1e-3 * (t - ta)**b

a = 4.0
b = 1.0
ac = 0.75
bc = 0.9
