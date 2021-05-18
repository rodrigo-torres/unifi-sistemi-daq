/****************************************************************************
 * Copyright (C) 2021 by Rodrigo Torres										*
 *   																		*
 *   main.c																	*
 *   																		*
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

/*! \file main.c
    \brief A Documented file.

    Details.
 */


#include "gnuplot.h"
#include "red_pitaya.h"
#include "utility.h"

#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define THR 1000
#define CRSIZE 100
#define WVSIZE 200
#define RDSIZE 16414

#define FIFO_FILENAME "/tmp/redpitaya_fifo_rodrigo"

int daq_go = 1;			///< A status variable, set to 1 when DAQ is active
int fd = -1;			///< A file descriptor for the tcp connection
int fdfifo = -1;		///< A file descriptor for the data FIFO
FILE * gnuplot = NULL;	///< A handle for the gnuplot pipe

//! @brief Callback function that triggers the end of DAQ when a SIGINT signal
//! is intercepted
//!
//! @param signum is the number associated to the signal intercepted
void SignalHandler (int signum);

//! @brief Performs garbage collection of global variables before triggering
//! program termination
//!
//! @param code is the termination code provided to the call of exit()
void CleanExit (int code);

int main(int argc, char * argv[]) {
	UNUSED(argc);
	UNUSED(argv);
	UNUSED(gnuplot);

	// Handle interruption of DAQ program
	if (signal (SIGINT, &SignalHandler) == SIG_ERR) {
		PRINT_STD_LIBERROR("signal");
		CleanExit(EXIT_FAILURE);
	}

	if (signal (SIGPIPE, &SignalHandler) == SIG_ERR) {
		PRINT_STD_LIBERROR("signal");
		CleanExit(EXIT_FAILURE);
	}

//	gnuplot = GNUPlot_Configure();
//	if (gnuplot == NULL) {
//		CleanExit(EXIT_FAILURE);
//	}

	// Open FIFO for data
	PRINT_DBGMSG("Creating data output FIFO");
	if ( (fdfifo = mkfifo(FIFO_FILENAME, 0644)) == -1) {
		PRINT_STD_LIBERROR("mkfifo");
		CleanExit(EXIT_FAILURE);
	}

	if ( (fdfifo = open(FIFO_FILENAME, O_WRONLY)) == -1 ) {
		PRINT_STD_LIBERROR("open");
		CleanExit(EXIT_FAILURE);
	}
	PRINT_DBGMSG("FIFO created");

	// Establish a connection to the red pitaya
	PRINT_DBGMSG("Attempting connection to the RedPitaya");
	fd = RedPitaya_Connect();
	if (fd == -1) {
		//RedPitaya_Connect already outputs error message
		CleanExit(EXIT_FAILURE);
	}

	int16_t rdbuff [RDSIZE]; // A buffer of 32768 bytes + 60 bytes overhead
	int16_t crbuff [CRSIZE];
	int16_t wvbuff [WVSIZE];

	PRINT_DBGMSG("Starting acquisition...");
	// Acquisition loop
	int iw = 0, ir = 0, wt = 0;
	int data = 0;
	while (daq_go) {
		// Read the buffer in a loop
		int result = 0;
		for (int nt = 0; nt < sizeof (rdbuff); nt += result) {
			result = read (fd, (uint8_t *)rdbuff + nt, sizeof (rdbuff) - nt); // BUG bad address because using sizeof(rdbuff) + nt instead of - nt
			if (result < 0) {
				PRINT_STD_LIBERROR("read");
				CleanExit(EXIT_FAILURE);
			}
		}

		// Rising trigger logic
		crbuff[iw] = rdbuff [RPITAYA_HEAD_OFFSET];
		iw = (++iw) >= CRSIZE ? 0 : iw;
//		if (++iw >= CRSIZE) {
//			iw = 0;
//		}
		for (int nt = RPITAYA_HEAD_OFFSET + 1; nt < RDSIZE; ++nt) { // BUG use RDSIZE to avoid SEGFAULT
			crbuff[iw] = rdbuff [nt];
			iw = (++iw) >= CRSIZE ? 0 : iw;

			// Trigger logic
			if (rdbuff[nt] > THR && rdbuff[nt-1] < THR && data == 0) {
				// NOTE: Current write position of the cyclic buffer corresponds
				// to the oldest buffer entry since counter is increased
				// immediately after each write operation.
				for (ir = iw, wt = 0; wt < CRSIZE - 1; ++wt) {
					if (ir >= CRSIZE) {
						ir = 0;
					}
					wvbuff [wt] = crbuff [ir++];
				}
				data = 1;
			}

			if (data) {
				wvbuff [wt++] = rdbuff[nt];
				if (wt >= WVSIZE) {
					data = 0;
					// Process the data. Print the waveform to the screen
					write(fdfifo, wvbuff, sizeof (wvbuff));
					//GNUPlot_Plot(gnuplot, wvbuff, WVSIZE);
					//usleep(500000);
				}
			}


		}
	}
	PRINT_DBGMSG("Stopping acquisition...");
	CleanExit(EXIT_SUCCESS);
	return 0; // Never executed
};


void SignalHandler (int signum) {
	UNUSED(signum);
	daq_go = 0;
};

void CleanExit(int code) {
	if (fd != -1) {
		close(fd);
	}
	if (fdfifo != -1) {
		close (fdfifo);
	}
	unlink(FIFO_FILENAME); // Unlink regardless
	if (gnuplot != NULL) {
		fclose(gnuplot);
	}
	PRINT_DBGMSG("Garbage has been collected");
	exit(code);
}



