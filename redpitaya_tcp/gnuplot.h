/****************************************************************************
 * Copyright (C) 2021 by Rodrigo Torres                                     *
 *                                                                          *
 *   This program free software: you can redistribute it and/or modify it   *
 *   under the terms of the GNU Lesser General Public License as published  *
 *   by the Free Software Foundation, either version 3 of the License, or   *
 *   (at your option) any later version.                                    *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU Lesser General Public License for more details.                    *
 *                                                                          *
 *   You should have received a copy of the GNU Lesser General Public       *
 *   License along with this program.                                       *
 ***************************************************************************/

#ifndef DAQ_SETUP_H
#define DAQ_SETUP_H

#include "utility.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define GNUPLOT_TITLE	"Test"
#define GNUPLOT_X_LABEL "Sample Point"
#define GNUPLOT_Y_LABEL "ADC Bin"

static FILE * GNUPlot_Configure();
static void GNUPlot_Plot(FILE * plot, int16_t const * data, int const points);

static FILE * GNUPlot_Configure() {
	FILE * fgplot = popen("gnuplot -persist","w");
	if (fgplot == NULL) {
		PRINT_STD_LIBERROR("popen");
		return NULL;
	}

	// Aggiunge uno spazio, perché a volte perde il primo carattere
	fprintf(fgplot," set term x11 0 \n");
	fprintf(fgplot, "set title \"%s\"\n", GNUPLOT_TITLE);
	fprintf(fgplot," set xlabel \"%s\"\n", GNUPLOT_X_LABEL);
	fprintf(fgplot," set ylabel \"%s\"\n", GNUPLOT_Y_LABEL);
	fprintf(fgplot," set xtics nomirror\n");
	fprintf(fgplot," set ytics nomirror\n");
	fprintf(fgplot," set style data linesp\n");
	fprintf(fgplot," set grid\n");

	return fgplot;
}



static void GNUPlot_Plot(FILE * plot, int16_t const * data, int const points) {
	if (points == 1) {
		return; // Avoid GNUplot complaints
	}

	fprintf(plot, " plot '-'\n");
	for (int j = 0; j < points; ++j) {
		fprintf(plot, " %d %d\n", j, data[j]);
	}
	fprintf(plot, "e\n");
	fflush(plot);
}

#endif // daq_setup.h



