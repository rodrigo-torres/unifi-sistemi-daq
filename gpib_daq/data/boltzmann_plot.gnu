#!/usr/bin/gnuplot -persist
#
#    
#    	G N U P L O T
#    	Version 5.2 patchlevel 8    last modified 2019-12-01 
#    
#    	Copyright (C) 1986-1993, 1998, 2004, 2007-2019
#    	Thomas Williams, Colin Kelley and many others
#    
#    	gnuplot home:     http://www.gnuplot.info
#    	faq, bugs, etc:   type "help FAQ"
#    	immediate help:   type "help"  (plot window: hit 'h')
# set terminal qt 0 font "Sans,9"
# set output
unset clip points
set clip one
unset clip two
set errorbars front 1.000000 
set border 31 front lt black linewidth 1.000 dashtype solid
set zdata 
set ydata 
set xdata 
set y2data 
set x2data 
set boxwidth
set style fill  empty border
set style rectangle back fc  bgnd fillstyle   solid 1.00 border lt -1
set style circle radius graph 0.02 
set style ellipse size graph 0.05, 0.03 angle 0 units xy
set dummy x, y
set format x "% h" 
set format y "% h" 
set format x2 "% h" 
set format y2 "% h" 
set format z "% h" 
set format cb "% h" 
set format r "% h" 
set ttics format "% h"
set timefmt "%d/%m/%y,%H:%M"
set angles radians
set tics back
set grid nopolar
set grid xtics nomxtics ytics nomytics noztics nomztics nortics nomrtics \
 nox2tics nomx2tics noy2tics nomy2tics nocbtics nomcbtics
set grid layerdefault   lt 0 linecolor 0 linewidth 0.500,  lt 0 linecolor 0 linewidth 0.500
unset raxis
set theta counterclockwise right
set style parallel front  lt black linewidth 2.000 dashtype solid
set key title "" center
set key fixed left top vertical Right noreverse enhanced autotitle nobox
set key noinvert samplen 4 spacing 1 width 0 height 0 
set key maxcolumns 0 maxrows 0
set key noopaque
unset label
unset arrow
set style increment default
unset style line
unset style arrow
set style histogram clustered gap 2 title textcolor lt -1
unset object
set style textbox transparent margins  1.0,  1.0 border  lt -1 linewidth  1.0
set offsets 0, 0, 0, 0
set pointsize 1
set pointintervalbox 1
set encoding utf8
unset polar
unset parametric
unset decimalsign
unset micro
unset minussign
set view 60, 30, 1, 1
set view azimuth 0
set rgbmax 255
set samples 100, 100
set isosamples 10, 10
set surface 
unset contour
set cntrlabel  format '%8.3g' font '' start 5 interval 20
set mapping cartesian
set datafile separator ","
unset hidden3d
set cntrparam order 4
set cntrparam linear
set cntrparam levels 5
set cntrparam levels auto
set cntrparam firstlinetype 0 unsorted
set cntrparam points 5
set size ratio 0 1,1
set origin 0,0
set style data points
set style function lines
unset xzeroaxis
unset yzeroaxis
unset zzeroaxis
unset x2zeroaxis
unset y2zeroaxis
set xyplane relative 0.5
set tics scale  1, 0.5, 1, 1, 1
set mxtics 5.000000
set mytics 5.000000
set mztics default
set mx2tics default
set my2tics default
set mcbtics default
set mrtics default
set nomttics
set xtics border in scale 1,0.5 nomirror norotate  autojustify
set xtics  norangelimit autofreq 
set ytics border in scale 1,0.5 nomirror norotate  autojustify
set ytics  norangelimit autofreq 
set ztics border in scale 1,0.5 nomirror norotate  autojustify
set ztics  norangelimit autofreq 
unset x2tics
unset y2tics
set cbtics border in scale 1,0.5 mirror norotate  autojustify
set cbtics  norangelimit autofreq 
set rtics axis in scale 1,0.5 nomirror norotate  autojustify
set rtics  norangelimit autofreq 
unset ttics
set title "LAMP 3. Stefan-Boltzmann law \\& Fourier dissipation of an incandescent tungsten wire" 
set title  font "" textcolor lt -1 norotate
set timestamp bottom 
set timestamp "" 
set timestamp  font "" textcolor lt -1 norotate
set trange [ * : * ] noreverse nowriteback
set urange [ * : * ] noreverse nowriteback
set vrange [ * : * ] noreverse nowriteback
set xlabel "Temperature (Kelvin)" 
set xlabel  font "" textcolor lt -1 norotate
set x2label "" 
set x2label  font "" textcolor lt -1 norotate
set xrange [ * : * ] noreverse writeback
set x2range [ * : * ] noreverse writeback
set ylabel "Power (W)" 
set ylabel  font "" textcolor lt -1 rotate
set y2label "" 
set y2label  font "" textcolor lt -1 rotate
set yrange [ * : * ] noreverse writeback
set y2range [ * : * ] noreverse writeback
set zlabel "" 
set zlabel  font "" textcolor lt -1 norotate
set zrange [ * : * ] noreverse writeback
set cblabel "" 
set cblabel  font "" textcolor lt -1 rotate
set cbrange [ * : * ] noreverse writeback
set rlabel "" 
set rlabel  font "" textcolor lt -1 norotate
set rrange [ * : * ] noreverse writeback
unset logscale
unset jitter
set zero 1e-08
set lmargin  -1
set bmargin  -1
set rmargin  -1
set tmargin  -1
set locale "en_US.UTF-8"
set pm3d explicit at s
set pm3d scansautomatic
set pm3d interpolate 1,1 flush begin noftriangles noborder corners2color mean
set pm3d nolighting
set palette positive nops_allcF maxcolors 0 gamma 1.5 color model RGB 
set palette rgbformulae 7, 5, 15
set colorbox default
set colorbox vertical origin screen 0.9, 0.2 size screen 0.05, 0.6 front  noinvert bdefault
set style boxplot candles range  1.50 outliers pt 7 separation 1 labels auto unsorted
set loadpath 
set fontpath 
set psdir
set fit brief errorvariables nocovariancevariables errorscaling prescale nowrap v5
y(x)=m*x+c
rs(x) = x > 1900 ? wa1 + wa2*x + wa3*x*x : 	x > 800 ? wa4 + wa5*x + wa6*x*x : wa7 + wa8*x + wa9*x*x
ts(x) = x > 53.35 ? wb1 + wb2*x + wb3*x*x : 	x > 19.00 ? wb4 + wb5*x + wb6*x*x : wb7 + wb8*x + wb9*x*x
tmp(v,i) = ts((g*v/i)*1000000)
pwr(v,i) = v * i
sbl(t) = ac * 1.0e-14 * (t**a - ta**a)
fou(t) = bc * 1e-3 * (t - ta)**b
tmp_err(x,y,x_err,y_err)=tmp(x+x_err,y+y_err)-tmp(x,y)
pwr_err(x,y,x_err,y_err)=pwr(x+x_err,y+y_err)-pwr(x,y)
GNUTERM = "qt"
file = "20210415_lampadina3_scansione0V-12V_step0.5V_means.csv"
x = 0.0
m = 0.705459885767828
c = -0.000113991043384233
FIT_CONVERGED = 1
FIT_NDF = 20
FIT_STDFIT = 0.00601954370419744
FIT_WSSR = 0.000724698128134862
FIT_P = 1.0
FIT_NITER = 87
m_err = 0.0109264189444955
c_err = 0.000751714330275835
file1 = "20210415_lampadina1_scansione0V-1V_step0.05V_means.csv"
file3 = "20210415_lampadina3_scansione0V-1V_step0.05V_means.csv"
wa1 = -3.28353
wa2 = 0.0264003
wa3 = 1.79125e-06
wa4 = -3.238
wa5 = 0.0263579
wa6 = 1.80302e-06
wa7 = -0.724112
wa8 = 0.0191576
wa9 = 6.88698e-06
wb1 = 207.697
wb2 = 33.7132
wb3 = -0.0366286
wb4 = 133.745
wb5 = 36.2054
wb6 = -0.0581687
wb7 = 49.5998
wb8 = 46.4533
wb9 = -0.368427
g = 3.87248642e-06
ta = 293.388066333333
ac = 13.4788448258224
a = 4.09221272023639
bc = 0.35317625958633
b = 1.06097644773159
ac_err = 2.83684547339129
a_err = 0.0256828849158898
bc_err = 0.0860999728719015
b_err = 0.0398066080362842
## Last datafile plotted: "20210415_lampadina3_scansione0V-12V_step0.5V_means.csv"
plot file ev ::2 u (tmp($2,$4)):(pwr($2,$4)):(tmp_err($2,$4,$3,$5)) w xerr lc black lw 1 title "Lamp 3 averaged data", sbl(x)+fou(x) lc "red" dt 2 title "Stefan-Boltzmann law \\& Fourier dissipation fit"
## fit sbl(x)+fou(x) file ev ::2 u (tmp($2,$4)):(pwr($2,$4)) via ac,a,bc,b
#    EOF
