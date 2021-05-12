## Analysis Procedure

1. On each "raw output log" text file, run the convert.py script convert the file into a CSV format. 

2. Then on each of these resulting files run the analysis.py python script to generate a second CSV file with the means values for the measured voltages and currents, and their associated errors

3. For the ambient temperature measures, plot voltage vs current and fit a straight line in the "linear" regime, the inverse slope of the fit will be our measured value of the resistance at ambient temperature, (because $$V = R I$$).

4. For lamp #1, a fit in the voltage range [0V,0.2V] gives the following converging fit. A plot of this fit is found in the files "lamp1_ambient_measure_0V-0.2Vfit_zoom.png" and "lamp1_ambient_measure_0V-0.2Vfit.png"

degrees of freedom    (FIT_NDF)                        : 2
rms of residuals      (FIT_STDFIT) = sqrt(WSSR/ndf)    : 0.00712961
variance of residuals (reduced chisquare) = WSSR/ndf   : 5.08314e-05

Final set of parameters            Asymptotic Standard Error
=======================            ==========================
m               = 1.58672          +/- 0.06305      (3.973%)
c               = 0.00107836       +/- 0.006084     (564.2%)

correlation matrix of the fit parameters:
                m      c      
m               1.000 
c              -0.810  1.000 


5. From the plots it can be seen that the data starts to diverge already from the linear model for voltage values as low as 0.15V. For lamp #1 the fit was made again but now in the voltage range [0V,0.125V]. The plot of this model is found in "lamp1_ambient_measure_0V-0.125Vfit.png", and is visibly a better model. The fit converged and gave the following parameters,

degrees of freedom    (FIT_NDF)                        : 1
rms of residuals      (FIT_STDFIT) = sqrt(WSSR/ndf)    : 0.00214824
variance of residuals (reduced chisquare) = WSSR/ndf   : 4.61494e-06

Final set of parameters            Asymptotic Standard Error
=======================            ==========================
m               = 1.68981          +/- 0.02943      (1.742%)
c               = -0.00257699      +/- 0.001999     (77.57%)

correlation matrix of the fit parameters:
                m      c      
m               1.000 
c              -0.784  1.000 

6. Step 5 above as repeated for the data obtained from a second ambient temperature measurement with lamp 1. The fit converged with these following parameters

degrees of freedom    (FIT_NDF)                        : 1
rms of residuals      (FIT_STDFIT) = sqrt(WSSR/ndf)    : 0.000525705
variance of residuals (reduced chisquare) = WSSR/ndf   : 2.76366e-07

Final set of parameters            Asymptotic Standard Error
=======================            ==========================
m               = 1.66331          +/- 0.006879     (0.4136%)
c               = -0.00270275      +/- 0.0004913    (18.18%)

correlation matrix of the fit parameters:
                m      c      
m               1.000 
c              -0.786  1.000 

7. Step 5 above was repeated again for the ambient temperature measurement obtained with lamp 3. The fit converged with these following parameters. As seen, the measured value is substantially different

degrees of freedom    (FIT_NDF)                        : 1
rms of residuals      (FIT_STDFIT) = sqrt(WSSR/ndf)    : 0.000780488
variance of residuals (reduced chisquare) = WSSR/ndf   : 6.09162e-07

Final set of parameters            Asymptotic Standard Error
=======================            ==========================
m               = 0.70546          +/- 0.01093      (1.549%)
c               = -0.000113991     +/- 0.0007517    (659.5%)

correlation matrix of the fit parameters:
                m      c      
m               1.000 
c              -0.800  1.000 

8. Inside the scripts folder, the "tungsten_temp_macro.gnu" contains definitions for the temperature of a tungsten wire as a function of its resisitivity, and for the resistivity of the tungsten wire as function of its temperature.

9. Inside the scripts folder, compile "r_to_t.c" which is a C program that takes as a first parameter a reading of a resistance in the laboratory, which is then used to provided a temperature reading of the laboratory.

10. In gnuplot load the "tungsten_temp_macro" and input to the function the temperatures obtained by feeding the values inside the "20210415_temperatura" file to the "r_to_t" C program. These functions are actually fits to the data of Jones 1926 paper

11. Three temperature measurements were taken at different times in the morning. However it was failed note which temperature readings corresponded to which measurements. Fortunately, the variation was small.

The temperatures read were:
Temperature is 20.056879 corresponding to 108.280000
Temperature is 20.283563 corresponding to 108.368000
Temperature is 20.373757 corresponding to 108.403000

And the corresponding resisitivity values are:
gnuplot> print rs(20.056879+273.15)
5.48510366202547
gnuplot> print rs(20.283563+273.15)
5.49036222781169
gnuplot> print rs(20.373757+273.15)
5.4924547253037

The mean resistivity is 5.489306872 (microohm * cm) and the standard error associated is 0.001785426 (about 0.032% error)

12. From the values above, the geometric factors are calculated from the formula $$ g = \frac{R_amb}{\rho_amb} $$

For lamp 1, geometric factor is 109523.998 (1/cm) with 0.891% error at 2-sigma interval
	and 1/g would be 9.13041907e-06 cm
For lamp 3, geometric factor is 258232.022 (1/cm) with 1.581% error at 2-sigma interval
	and 1/g would be 3.87248642e-06 cm
	
13. Lastly, using these values for the geometric factor, a plot was made of the electrical power against the (calculated) temperature for each lamp. The data was plot with a model consisting of two components: the first, the Stefan-Boltzmann law, and the second, a Fourier heat dissipation factor
	
For lamp 1, the fit converged as follows

After 2173 iterations the fit converged.
final sum of squares of residuals : 0.00457906
rel. change during last iteration : -7.84289e-06

degrees of freedom    (FIT_NDF)                        : 20
rms of residuals      (FIT_STDFIT) = sqrt(WSSR/ndf)    : 0.0151312
variance of residuals (reduced chisquare) = WSSR/ndf   : 0.000228953

Final set of parameters            Asymptotic Standard Error
=======================            ==========================
ac              = 50.9781          +/- 10.34        (20.29%)
a               = 4.04478          +/- 0.02481      (0.6133%)
bc              = 0.811097         +/- 0.3186       (39.28%)
b               = 1.00574          +/- 0.06424      (6.387%)

correlation matrix of the fit parameters:
                ac     a      bc     b      
ac              1.000 
a              -1.000  1.000 
bc              0.895 -0.891  1.000 
b              -0.917  0.913 -0.999  1.000 

For lamp 3, the fit converged as follows

After 87 iterations the fit converged.
final sum of squares of residuals : 0.000724698
rel. change during last iteration : -8.30736e-06

degrees of freedom    (FIT_NDF)                        : 20
rms of residuals      (FIT_STDFIT) = sqrt(WSSR/ndf)    : 0.00601954
variance of residuals (reduced chisquare) = WSSR/ndf   : 3.62349e-05

Final set of parameters            Asymptotic Standard Error
=======================            ==========================
ac              = 13.4788          +/- 2.837        (21.05%)
a               = 4.09221          +/- 0.02568      (0.6276%)
bc              = 0.353176         +/- 0.0861       (24.38%)
b               = 1.06098          +/- 0.03981      (3.752%)

correlation matrix of the fit parameters:
                ac     a      bc     b      
ac              1.000 
a              -1.000  1.000 
bc              0.896 -0.891  1.000 
b              -0.918  0.913 -0.999  1.000 



CONCLUSION. Both lamps follow the Stefan-Boltzmann law, the fit parameter 'a' is approximately four which is consistent with a dependency on the fourth power of the temperature.
