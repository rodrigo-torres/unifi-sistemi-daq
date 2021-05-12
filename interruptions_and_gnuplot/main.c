/****************************************************************************
 * Copyright (C) 2021 by Rodrigo Torres										*
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

//! \file main.c
//! @brief A program that measures the time elapsed between successive calls to
//! the LINUX function gettimeofday and prints out on the screen a report
//! of the values measured
//!

#include "gnuplot.h"
#include "utility.h"

#include <sys/time.h>
#include <unistd.h>

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char *optarg;

int max_samples;	///< Maximum number of time samples allowed
int samples;		///< Number of time samples to be taken

bool plot_data = false;	///< Status variable, if true data will be plotted
///< instead of being saved to a file
FILE * fgplot = NULL;	///< File handle for the gnuplot pipe
FILE * f 	  = NULL;	///< File handle for data output file
struct timeval * vtime = NULL;	///< Data buffer for time samples

//! @brief Uses the POSIX getopt function to process the program arguments,
//! initializing global variables as is needed.
//!
//! @note If an argument is unrecognized, or if a global variable cannot be
//! properly initialized, the function triggers program termination.
//!
//! @param argc is the number of program arguments available
//! @param argv is the c-string associated to each of the arguments
//!
void ProcessArguments(int argc, char * argv []);

//! @brief Prints out a short manual page outlining the use of this program
//!
void PrintProgramUse();

//! @brief Hints as to the maximum memory size available to allocate for
//! the data buffer.
//!
//! The method uses the LINUX sysconf function to get the number of memory
//! pages available to the program, and the size of these pages. The function
//! suggests to use at most 25% of this memory available.
//!
//! @return A recommended maximum value of memory to use (in bytes)
long int HintMaxMemoryAvailable() {
	return (sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE)) / 4;
}

//! @brief Performs garbage collection of global variables before triggering
//! program termination
//!
//! @param code is the termination code provided to the call of exit()
void CleanExit (int code);


int main(int argc, char * argv[]) {
	// Also initializes the max_samples variable
	ProcessArguments(argc, argv);

	DebugMessage(kDebugMsg, "", "Taking %llu samples", samples);

	if ( (vtime = calloc(samples, sizeof (struct timeval))) == NULL ) {
		DebugErrNumMessage("calloc");
		CleanExit(EXIT_FAILURE);
	}

	if(!plot_data) {
		if ( (f = fopen("out.txt","w")) == NULL ) {
			PRINT_ERRNO_MSG("fopen");
			CleanExit(EXIT_FAILURE);
		}
		fprintf(f,"#log2(t)\tconteggi\n");
	}

	// Take the number of samples specified
	for (int i = 0; i < samples; ++i) {
		gettimeofday(vtime + i, NULL);
	}

	struct timeval diff;
	typeof (diff.tv_usec) delta;	// Variable for microsecond differences

	int logdelta;		// MSB counter
	int histo [32];    	// MSB histogram of variable delta
	memset(histo, 0, sizeof (histo));

	// Process the data
	for (int i = 0; i < samples - 1; ++i) {
		timersub(vtime + i + 1, vtime + i, &diff);
		delta = 1000000 * diff.tv_sec + diff.tv_usec;

		logdelta = 0;
		while (delta) {
			++logdelta;
			delta >>= 1;
		}
		++histo[logdelta];
	}

	if (plot_data) {
		int max_bin = 32;
		for (int i = 31; i > 0; --i) {
			if (histo[i] != 0) {
				max_bin = i;
				break;
			}
		}
		GNUPlot_SetXRange(fgplot, 0, max_bin + 5);
		GNUPlot_Plot(fgplot, histo, 32);
	}
	else {
		//fprintf(f,"%-10u <= t (us) < %-11u counts: %d\n", 0, 1, histo[0]);
		for (int i = 0; i < 32; ++i) {
			fprintf(f,"%d\t%d\n", i, histo[i]);
		}
		PRINT_MSG("Data printed out to file \"out.txt\"");
	}


	CleanExit(EXIT_SUCCESS);
	return 0;
}

void ProcessArguments(int argc, char * argv[]) {
	int c;

	max_samples = HintMaxMemoryAvailable() / sizeof (struct timeval);

	if (argc < 2) {
		PrintProgramUse();
		CleanExit(EXIT_SUCCESS);
	}

	while ((c = getopt(argc,argv,"n:ph")) != -1) {
		switch (c) {
		case 'n':
			if (optarg == NULL) {
				PRINT_ERR_MSG("Empty argument for option \"-n\"");
				CleanExit(EXIT_FAILURE); // Program termination
			}

			samples = strtol(optarg, NULL, 10);
			if (errno != 0) {
				DebugErrNumMessage("strtol");
				//PRINT_STD_LIBERROR("strtol");
				CleanExit(EXIT_FAILURE);
			}
			else if (samples == 0) {
				DebugMessage(kLibError, "strtol", "Possible conversion error");
				//PRINT_LIBERROR("strtol", "Possible conversion error");
				CleanExit(EXIT_FAILURE);
			}
			else if (samples > max_samples) {
				DebugMessage(kDebugMsg,"","Truncating samples value to maximum allowed");
				//PRINT_DBGMSG("Truncating samples value to maximum allowed");
				samples = max_samples;
			}
			break;
		case 'p':
			// Plot the data. Don't save to file
			plot_data = 1;
			if ( (fgplot = GNUPlot_Configure()) == NULL ) {
				PRINT_LIB_ERR_MSG("GNUPlot_Configure",
						"Could not open gnuplot pipe.");
				CleanExit(EXIT_FAILURE);
			}
			break;
		case 'h':
			PrintProgramUse();
			CleanExit(EXIT_SUCCESS);
			break;
		case '?':
			// fallthrough
		default:
			PRINT_ERR_MSG("Unrecognized program argument.");
			PrintProgramUse();
			CleanExit(EXIT_FAILURE);
			break;
		}
	}

}

void PrintProgramUse () {
	puts(
	"interruptions - measures program interruptions. Measures the time elapsed "
	"between successive calls to the LINUX function gettimeofday and either "
	"plots the data or prints out on the screen a report of the values "
	"measured.");

	puts("");
	puts("Arguments:");
	puts("[REQUIRED] -n {number}\tnumber of time samples to take");
	puts("[OPTIONAL] -p \t\tplot the data and do not save to file");
	puts("[OPTIONAL] -h \t\tprint this help menu on the screen");
}

void CleanExit(int code) {

	if (fgplot) {
		pclose(fgplot);
	}
	if (f) {
		fclose(f);
	}
	if (vtime) {
		free (vtime);
	}

	exit(code);
}
