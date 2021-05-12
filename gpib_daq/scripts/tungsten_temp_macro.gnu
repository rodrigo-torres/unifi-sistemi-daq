#
# rs(x): resistivita' (microohm*cm) vs. temperatura (kelvin)
# ts(x): temperatura (kelvin) vs. resistivita' (microohm*cm)
#

wa1 = -3.28353;  wa2 = 0.0264003; wa3 = 1.79125e-6
wa4 = -3.238;    wa5 = 0.0263579; wa6 = 1.80302e-06
wa7 = -0.724112; wa8 = 0.0191576; wa9 = 6.88698e-06

wb1 = 207.697; wb2 = 33.7132; wb3 = -0.0366286
wb4 = 133.745; wb5 = 36.2054; wb6 = -0.0581687
wb7 = 49.5998; wb8 = 46.4533; wb9 = -0.368427

rs(x) = x > 1900 ? wa1 + wa2*x + wa3*x*x : \
	x > 800 ? wa4 + wa5*x + wa6*x*x : wa7 + wa8*x + wa9*x*x
ts(x) = x > 53.35 ? wb1 + wb2*x + wb3*x*x : \
	x > 19.00 ? wb4 + wb5*x + wb6*x*x : wb7 + wb8*x + wb9*x*x
