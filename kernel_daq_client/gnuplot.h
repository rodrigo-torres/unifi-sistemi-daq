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

//! \file gnuplot.h
//! \brief Function declarations and definitions to set up a pipe to Gnuplot to
//! plot histograms
//!

#ifndef DAQ_SETUP_H
#define DAQ_SETUP_H

#include "utility.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

enum GNUPlot_Flags {
	kNoMirror,
	kGrid,
	kLogX,
	kLogY
};

typedef struct {
	char const * title;
	char const * xlabel;
	char const * ylabel;
	char const * style;
	char const * line_color;
	int flags;
} GNUPlot_ParamsTypedef;


//! \brief
//!
//! \param params
//!
//! \return A pointer to the gnuplot pipe handle if successfull, NULL otherwise
static FILE * GNUPlot_Configure(GNUPlot_ParamsTypedef * params);

//! \brief
//!
//! \param plot
//! \param data
//! \param points
static void GNUPlot_Plot(FILE * plot, int16_t const * data, int const points);

static FILE * GNUPlot_Configure(GNUPlot_ParamsTypedef * params) {
	if(params == NULL) {
		return NULL;
	}

	FILE * fgplot = popen("gnuplot -persist","w");
	if (fgplot == NULL) {
		PRINT_STD_LIBERROR("popen");
		return NULL;
	}

	// Aggiunge uno spazio, perché a volte perde il primo carattere
	fprintf(fgplot," set term x11 0 \n");
	if (params->title) {
		fprintf(fgplot, "set title \"%s\"\n", params->title);
	}
	if (params->xlabel) {
		fprintf(fgplot," set xlabel \"%s\"\n", params->xlabel);
	}
	if (params->ylabel) {
		fprintf(fgplot," set ylabel \"%s\"\n", params->ylabel);
	}
	if (params->style) {
		fprintf(fgplot," set style data %s\n", params->style);
	}
    fprintf(fgplot, " set style data steps\n");
	if (params->flags & kNoMirror) {
		fprintf(fgplot," set xtics nomirror\n");
		fprintf(fgplot," set ytics nomirror\n");
	}
	if (params->flags & kGrid) {
		fprintf(fgplot," set grid\n");
	}
	if (params->flags & kLogX) {
		fprintf(fgplot," set logscale x\n");
	}
	if (params->flags & kLogY) {
		fprintf(fgplot," set logscale y\n");
	}

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
