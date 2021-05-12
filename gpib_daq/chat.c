/****************************************************************************
 * Copyright (C) 2021 by Rodrigo Torres*
 *   *
 *   main.c*
 *   *
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

#include "common.h"
#include "gnuplot.h"
#include "gpib_daq.h"

#include <string.h>

bool test_mode = false;   ///< Run (or not) program in test mode
bool output_file = false; ///< Plot data in gnuplot
bool output_plot = false; ///< Output data to a CSV file

char out_filename [100];  ///< User-chosen name for output file;

FILE * vplot = NULL; ///< File handle for the voltage plot
FILE * iplot = NULL; ///< File handle for the current plot
FILE * fdata = NULL; ///< File handle for the output data file

//! @brief
//!
//! @param argc
//! @param argv
//! @return
int main(int argc, char * argv []) {
	char buff [100]; // A buffer for the GPIB requests/replies

	// The four functions below terminate program if the state is invalid
	ProcessArguments(argc, argv);
	GPIB_Setup();

	if (output_plot) {
		vplot = GNUPlot_Configure("Voltage (V)", "");
		iplot = GNUPlot_Configure("Current (A)", "");
	}
	if (output_file) {
		fdata = fopen(out_filename, "w");
		if (fdata) {
			printf("ERROR! Could not open output data file\n");
			CleanExit(EXIT_FAILURE);
		}
		// Print the header
		fprintf(fdata,
				"MEASUREMENT,VOLT_STEP(V),SAMPLE,TIMESTAMP(s),TIME_ERR(ms),"
				"VOLTAGE(V),CURRENT(A)\n");
	}

	// State is valid, start the DAQ loop

	gettimeofday(&params.origin, NULL);
	params.next = params.origin;

	int j = 0;			// Counter for the current sample point
	float voltage, tc; // Variables for set voltage point and measurement duration

	for (int jv = 0; jv < params.nv; ++jv) {
		voltage = params.v_start + jv * params.v_step;
		sprintf(buff, "vset %d,%f", params.chan, voltage);
		GPIB_Send(&params, buff);

		for (int jr = 0; jr < params.nt; ++jr) {
			tc = GPIB_Measure(&params, j);

			++j;
			if (output_plot) {
				GNUPlot_Plot(vplot, params.volt, j);
				GNUPlot_Plot(iplot, params.curr, j);
			}
			if (output_file) {
				fprintf(fdata, "%d,%d,%d,%f,%f,%f,%f\n", j, jv, jr,
						params.time[j], tc, params.volt[j], params.curr[j]);
			} else {
				printf("j %d jv %d jr %d: %f s %f ms %f V %f A\n", j, jv, jr,
						params.time[j], tc, params.volt[j], params.curr[j]);
			}
		}
	}

	// Block to go back
	for (int jv = params.nv - 1; jv >= 0; --jv) {
		voltage = params.v_start + jv * params.v_step;
		sprintf(buff, "vset %d,%f", params.chan, voltage);
		GPIB_Send(&params, buff);

		for (int jr = 0; jr < params.nt; ++jr) {
			tc = GPIB_Measure(&params, j);
			++j;
			if (output_plot) {
				GNUPlot_Plot(vplot, params.volt, j);
				GNUPlot_Plot(iplot, params.curr, j);
			}
			if (output_file) {
				fprintf(fdata, "%d,%d,%d,%f,%f,%f,%f\n", j, jv, jr,
						params.time[j], tc, params.volt[j], params.curr[j]);
			} else {
				printf("j %d jv %d jr %d: %f s %f ms %f V %f A\n", j, jv, jr,
						params.time[j], tc, params.volt[j], params.curr[j]);
			}
		}
	}

	CleanExit(EXIT_SUCCESS);
	return 0;
}


// Function definitions
void ProcessArguments (int argc, char * argv[]) {
	static struct option long_options[] =
	{
			{"device",	required_argument, NULL, 'a'},
			{"channel", required_argument, NULL, 'c'},
			{"vstart",	required_argument, NULL, 'b'},
			{"vstop", 	required_argument, NULL, 'e'},
			{"vstep",   required_argument, NULL, 's'},
			{"ilim", 	required_argument, NULL, 'i'},
			{"samples", required_argument, NULL, 'n'},
			{"fileout", required_argument, NULL, 'f'},
			{"plotout", no_argument, NULL, 'p'},
			{"test",	no_argument, NULL, 't'},
			{"help",	no_argument, NULL, 'h'},
			{0, 0, 0, 0}
	};

	DAQParamsTypedef defaults =
	{ .dev = 60, .chan = 5, .v_start = 0., .v_stop = 5., .v_step = 0.5,
			.i_lim = 1., .rate = 1., .nt = 5 };

	params = defaults;

	char err_message [100], c;
	bool invalid_setup = false;
	int index = 0;

	while ( (c = getopt_long (argc, argv, "b:c:a:e:i:n:s:f:pth",
			long_options,&index)) != -1) {
		switch (c) {
		case 'b': // User override of voltage start
			params.v_start = atof(optarg);
			break;
		case 'c': // User override of channel
			params.chan = atoi(optarg);
			break;
		case 'a': // User override of GPIB device address
			params.dev = atoi(optarg);
			break;
		case 'e': // User override of voltage end
			params.v_stop = atof(optarg);
			break;
		case 'h':
			PrintProgramUse();
			CleanExit(EXIT_SUCCESS);
			break;
		case 'i': // User override of current limit
			params.i_lim = atof(optarg);
			break;
		case 'n':
			break;
		case 's': // User override of voltage step
			params.v_step = atof(optarg);
			break;
		case 't':
			test_mode = true;
			break;
		case 'f':
			output_file = true;
			strncpy(out_filename, optarg, 100);
			if (out_filename[99] != '\0') {
				printf("Error! Filename is too long\n");
			}
			break;
		case 'p':
			output_plot = true;
			break;
		default: // Unrecognized parameter
			invalid_setup = true;
			break;
		}
	}
	//
	// SANITY CHECKS
	// 1. Is the voltage range valid?
	if (params.v_stop <= params.v_start
			|| params.v_stop > 12. || params.v_start < 0.) {
		printf("ERROR! Invalid voltage scan range (%d,%d)\n",
				params.v_start, params.v_stop);
		invalid_setup = true;
	}
	// 2. Is the voltage step reasonable?
	if (params.v_step < 0.05) {
		printf("ERROR! Voltage step %0.4fV is too small\n", params.v_step);
		invalid_setup = true;
	}
	// 3. Is the HP 6627A DC power supply channel valid?
	if ( params.chan > 4 || params.chan < 1 ) {
		printf("ERROR! Channel %d is not a valid channel\n", params.chan);
		invalid_setup = true;
	}
	// 4. Is the GPIB device address valid?
	if ( params.dev > 30 || params.dev < 0 ) {
		printf("ERROR! Address %d is not a valid GPIB address\n", params.dev);
		invalid_setup = true;
	}
	// 5. Is the measurement rate sensible (not less than a few ms)?
	if (params.rate < 0.05 // avoid division by zero
			|| params.rate > 100) {
		printf("ERROR! Invalid rate!\n");
		invalid_setup = true;
	}

	if (invalid_setup) {
		CleanExit(EXIT_FAILURE);
	}

	// Compute sampling interval
	float interval = 1. / params.rate; // in units of second
	params.interval.tv_sec = (int)(interval); // truncated
	params.interval.tv_usec= (interval - (int)(interval)) * 1000000;

	// Setup data structures
	float val;
	params.nv = 0;
	while (true) {
		val = params.v_start + params.nv * params.v_step;
		if (((params.v_start < val) && (params.v_stop < val)) ||
				((params.v_start > val) && (params.v_stop > val))) {
			break;
		}
		++params.nv;
	}

	int const kArraySize = 2 * params.nt * params.nv;
	float * ptr = (float *) malloc (kArraySize * 3 * sizeof (float));
	if (ptr == NULL) {
		printf("ERROR! Couldn't allocate memory\n");
		CleanExit(EXIT_FAILURE);
	}
	params.time = ptr;
	params.volt = ptr + kArraySize;
	params.curr = ptr + 2 * kArraySize;

	return;
}


void CleanExit(int ret_code) {
	if (vplot) {
		pclose(vplot);
	}
	if (iplot) {
		pclose(iplot);
	}
	if (fdata) {
		fclose(fdata);
	}
	exit(ret_code);
}


void PrintProgramUse() {
	puts("lampscan [OPTION]...\n"
			"Scan V/I of a tungsten lamp using a programmable power supply.\n"
			"\n"
			"-b V_START\tset start point of voltage scan\n"
			"-e V_STOP\tset end point of voltage scan\n"
			"-s V_STEP\tset voltage step for DAQ\n"
			"-c CHANNEL\tselect HP 6627A DC power supply channel\n"
			"-a ADDRESS\tselect GPIB device address\n"
			"-i LIMIT\tset HP 6627A DC power supply current limit\n"
			"-n SAMPLES\tset number of samples per voltage step\n"
			"-f FILENAME\toutput the data to a file\n"
			"-p \t\toutput the data to gnuplot\n"
			"   --test\tturn ON/OFF test for lamp specified by -c and -d\n"
			"   --help\tshow this help menu\n"
			"\n"
			"lampscan is a program...\n");
}




