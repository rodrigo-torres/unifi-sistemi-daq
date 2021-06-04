/****************************************************************************
 * Copyright (C) 2021 by Rodrigo Torres					*
 *   									*
 *   main.c								*
 *   									*
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

#include "utility.h"
#include "gnuplot.h"

#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <zmq.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HIST_SIZE 8192
#define BUF_SIZE 100

#ifdef TEST_CLIENT
#define ADDRESS "127.0.0.1"
#else
#define ADDRESS "10.0.42.22"
#endif

//! \brief struct event defines the data for each SILENA ADC event
//!
struct event {
  int32_t tv_sec;   ///< Event timestamp, seconds field
  int32_t tv_usec;  ///< Event timestamp, microseconds field
  uint32_t value;   ///< Event ADC value
};

int daq_go = 1;			///< A status variable, set to 1 when DAQ is active
FILE * gnuplot = NULL; ///< The file descriptor for the Gnuplot pipe

void * context = NULL;
void * requester = NULL;

char buffer [BUF_SIZE];

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

//! @brief Sends a request message using the TCP connection created at program
//! startup and waits for a reply from the server. The reply message is
//! automatically checked for format and errors
//!
//! @param request is a c-string message to be sent to the server
//! @param reply is a buffer for the server reply, also a c-string
//! @param size is the size of the reply buffer. The function automatically
//! appends a terminating null-string even when the reply is truncated
//!
//! @return 0 on success, and -1 in case of error. If -1 is returned, the reply
//! buffer is possibly not a c-string.
int zmq_txrx(char const * request, char * reply, int size);

//! @brief Interprets the error code reported by the server
//!
//! @param msg the error reply message with format: "ERR <number>"
void InterpretServerError(char const * msg);

int main(int argc, char * argv[]) {
	struct event event;
	int16_t histo [HIST_SIZE];
	int retval;

	UNUSED(argc);
	UNUSED(argv);

	// Handle interruption of DAQ program
	if (signal(SIGINT, &SignalHandler) == SIG_ERR) {
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
  
	// Configure the ZMQ socket
	context   = zmq_ctx_new();
	requester = zmq_socket(context, ZMQ_REQ);

	if ( zmq_connect(requester, "tcp://127.0.0.1:5555") ) {
	  PRINT_STD_LIBERROR("zmq_connect");
	  CleanExit(EXIT_FAILURE);
	}

	PRINT_DBGMSG("Connection started.");
		
	memset(histo, 0, sizeof (histo));

	// Start the acquisition
	retval = zmq_txrx("S", buffer, BUF_SIZE);
	if (retval == -1) {
		PRINT_DBGMSG("Error starting acquisition");
		CleanExit(EXIT_FAILURE);
	}

	while (daq_go) {
	  // Request data one byte at a time
	  retval = zmq_txrx("R", buffer, BUF_SIZE);
	  if (retval == -1) {
	 	  PRINT_DBGMSG("Error receiving data");
			continue;
		}
		printf("%s", buffer);
		getc(stdin);
	}

	// Stop the acquisition
	retval = zmq_txrx("E", buffer, BUF_SIZE);
	if (retval == -1) {
		PRINT_DBGMSG("Error stopping acquisition");
		CleanExit(EXIT_FAILURE);
	}

	CleanExit(EXIT_SUCCESS);
	return 0; // Never executed
};

int zmq_txrx(char const * request, char * reply, int size) {
	int retval;
	// Send message
	printf("zmq_txrx: sending '%s'\n", request);
	if ( zmq_send(requester, request, strlen(request), 0) == -1 ) {
    PRINT_STD_LIBERROR("zmq_send");
		return -1;
	}
	// Receive reply
  retval = zmq_recv(requester, reply, size - 1, 0);
  if (retval == -1) { // Error in reception
    PRINT_STD_LIBERROR("zmq_recv");
		return -1;
	}
  if (retval > BUF_SIZE - 1) { // Message was truncated
  	PRINT_DBGMSG("Received message is truncated");
	  return -1;
	}
  buffer[retval] = '\0';
	printf("zmq_txrx: received '%s'\n", buffer);

	// Check response for format and reported errors
	switch ( (bool)strncmp(buffer, "OK", 2) |
						((bool)strncmp(buffer, "ERR", 3) << 1) ) {
		case 0: // String is 'OK' and 'ERR' simultaneously
			__attribute__((fallthrough));
		case 3: // String is not 'OK' and 'ERR' simultaneously
			// Report format error
			PRINT_DBGMSG("Wrong format of server response");
			retval = -1;
			break;
		case 2: // Server reports no error
			retval = 0;
			break;
		case 1: // Server reports an error
			InterpretServerError(buffer);
			retval = -1;
			break;
	}
	return retval;
}

void InterpretServerError(char const * msg) {
	printf("server reports: %s", buffer);
}

void SignalHandler (int signum) {
	UNUSED(signum);
	daq_go = 0;
};

void CleanExit(int code) {
	if (gnuplot != NULL) {
		pclose(gnuplot);
	}
	zmq_close(requester);
	zmq_ctx_destroy(context);
	fflush(stdout);
	fflush(stderr);
	PRINT_DBGMSG("Garbage has been collected");
	exit(code);
}
