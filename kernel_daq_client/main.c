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
#include "utility.h"

#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HIST_SIZE 8192
#define DEV_PATH "/dev/silenar"

//! \brief struct event defines the data for each SILENA ADC event
//!
struct event {
  int32_t tv_sec;   ///< Event timestamp, seconds field
  int32_t tv_usec;  ///< Event timestamp, microseconds field
  uint32_t value;   ///< Event ADC value
};

int daq_go = 1;			///< A status variable, set to 1 when DAQ is active
int fd = -1;			///< A file descriptor for the char device
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
	struct event event;
	int16_t histo [HIST_SIZE];
	int retval, serviced;

	UNUSED(argc);
	UNUSED(argv);

	// Handle interruption of DAQ program
	if (signal (SIGINT, &SignalHandler) == SIG_ERR) {
		PRINT_STD_LIBERROR("signal");
		CleanExit(EXIT_FAILURE);
	}
	
	// Configure GNUplot
	GNUPlot_ParamsTypedef gplot_config;
	gplot_config.title  = "Silena DAQ";
	gplot_config.xlabel = "Channel";
	gplot_config.ylabel = "Counts";
	//gplot_config.style  = "lc black";
	gplot_config.flags  = kNoMirror;	
	
	gnuplot = GNUPlot_Configure(&gplot_config);
	if (gnuplot == NULL) {
	  PRINT_DBGMSG("Could not configure the Gnuplot pipe!");
	  CleanExit(EXIT_FAILURE);	  
	}
	
	// Open the char device
	fd = open(DEV_PATH, O_RDWR, 0);
	if (fd == -1) {
	  PRINT_DBGMSG("Could not open the char device!");
	  CleanExit(EXIT_FAILURE);	  
	}
	
	memset(histo, 0, sizeof (histo));
	serviced = 0;

	// Start the acquisition
    retval = write(fd, "S", 1);
    if (retval != 1) {
      perror("");
	  PRINT_DBGMSG("Could not start acquisition.");
	  CleanExit(EXIT_FAILURE);	
    }

	
	while (daq_go) {
	  // Read one event at a time
	  retval = read(fd, &event, sizeof (struct event));
	  if (retval != sizeof (struct event) || retval == -1) {
	    // Errore nella lettura
	    PRINT_DBGMSG("Error in read operation.");
	    continue;
	  }
	  
	  if ( 0 <= event.value && event.value < HIST_SIZE) {
	    ++histo[event.value];
	  }
	  
	  if ((++serviced % 100) == 99) { // Plot every 100 events
	    GNUPlot_Plot(gnuplot, histo, HIST_SIZE);		  
	  }	
	}

	// Stop the acquisition
    retval = write(fd, "E", 1);
    if (retval != 1) {
	  PRINT_DBGMSG("Could not stop the acquisition.");
	  CleanExit(EXIT_FAILURE);	
    }


	PRINT_DBGMSG("Stopping acquisition.");
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
	if (gnuplot != NULL) {
		fclose(gnuplot);
	}
	PRINT_DBGMSG("Garbage has been collected");
	fflush(stdout);
	fflush(stderr);
	exit(code);
}
