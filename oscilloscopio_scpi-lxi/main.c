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

//! @file main.c
//! @brief Start a connection to an lxi instrument
//!

#include "utility.h"

#include <lxi.h>
#include <sys/time.h>
#include <unistd.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define LXI_OVERHEAD 24
#define LXI_TIMEOUT 10000

//! @brief A data structure to hold some related parameters of the lxi
//! instrument
//!
typedef struct {
	int hlxi; 			///< Handle of the lxi device
	float leng;			///< Length of the data frame (in bytes) to be received
	float v_div;		///< Voltage division of the oscilloscope (in V)
	float v_off;		///< Voltage offset of the oscilloscope (in V)
	char addr [100];	///< Address of the lxi instrument
} LXI_DAQParameters;

static LXI_DAQParameters params; ///< Instance of the DAQ variables struct

bool lxi_connected = false; ///< Status variable for lxi connection
char * data = NULL; ///< A pointer to the data buffer (dynamically allocated)
FILE * file = NULL; ///< A handle for the data output file

//! @brief Sends an GPIB command to the instrument and copies the reply to a
//! buffer provided by the user.
//!
//! @param device is the handle returned by lxi_connect()
//! @param send is a c-string with the lxi command to be sent
//! @param buff is a buffer for the reply message
//! @param length is the size of the buffer in bytes
//! @param timeout
//! @return 0 upon success, or -1 otherwise
int lxi_txrx(int device, char const * send, char * buff, int length, int timeout);

//! @brief Interrogates the lxi instrument in an attempt to populate the
//! variables in the LXI_DAQParameters data structure.
//!
//! @return 0 upon success, or -1 otherwise.
int lxi_getparams();

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


//! @brief Performs garbage collection of global variables before triggering
//! program termination
//!
//! @param code is the termination code provided to the call of exit()
void CleanExit(int code);

int main (int argc, char * argv []) {
	int retval;

	// 1. Process the program arguments
	ProcessArguments();

	// 2. Open the output file
    if ( (file = fopen("out.txt", "w")) == NULL ) {
    	PRINT_STD_LIBERROR("fopen");
    	CleanExit(EXIT_FAILURE);
    }

	// 3. Start the lxi library and a connection to the lxi instrument
	if (lxi_init() == LXI_ERROR) {
		PRINT_LIBERROR("lxi_init", "Could not initialize lxi library");
		CleanExit(EXIT_FAILURE);
	}

	printf("main: Attempting a connection to lxi address %s\n", params.addr);
	params.hlxi = lxi_connect (params.addr, 0, NULL, LXI_TIMEOUT, VXI11);
	if (params.hlxi == LXI_ERROR) {
		PRINT_LIBERROR("lxi_connect", "Could not start connection");
		CleanExit(EXIT_FAILURE);
	}
	lxi_connected = true;

	if (lxi_getparams() == -1) {
		// An error message, if any, is already printed inside the function
		CleanExit(EXIT_FAILURE);
	}
	int const kSize = params.leng + LXI_OVERHEAD;

	// Allocate memory for data buffer based on the side obtained from call
	// to lxi_getparams
	if ( (data = calloc(kSize, sizeof (char))) == NULL ) {
		PRINT_LIBERROR("calloc", "Memory allocation failed");
		CleanExit(EXIT_FAILURE);
	}
		
    retval = lxi_send(params.hlxi,
    					"C1:WF? DAT2", strlen("C1:WF? DAT2"), LXI_TIMEOUT);
	if (retval == LXI_ERROR) {
		PRINT_LIBERROR("lxi_send", "Data transmission failed");
		CleanExit(EXIT_FAILURE);
	}

	retval = lxi_receive (params.hlxi, data, kSize, LXI_TIMEOUT);
	if (retval != kSize) {
		// Reuse the data buffer for the error message
		snprintf(data, sizeof(data),
					"Inconsistent trace, read %d of %d bytes\n", retval, kSize);
		PRINT_LIBERROR("lxi_receive", data);
		CleanExit(EXIT_FAILURE);
	}

	// OUTPUT THE DATA
	// 1. Print the header and footer
	fprintf(file,"[HEADER]\n");
	fprintf(file, "%d %.22s -> %x %x\n",
				retval, data, data[kSize - 2], data[kSize - 1]);
	fprintf(file,"[DATA]\n");
	for (int j = LXI_OVERHEAD - 2; j < kSize - 2; ++j) {
		fprintf(file, "%0.3f ", data[j] * (params.v_div / 25) - params.v_off);
		if ( ((j - 20) % 16) == 0) { // Print 16 bytes per line
			fprintf(file, "\n");
		}
	}
	
	CleanExit(EXIT_SUCCESS);
	return 0; // Never executed
}

void ProcessArguments(int argc, char * argv []) {
	int opt;
	while ( (opt = getopt (argc, argv, "a:")) != -1) {
		switch (opt) {
		case 'a': // Specifies the address to connect to
			memcpy(params.addr, optarg, sizeof (params.addr));
			break;
		case '?':
		default:
			// getopt already outputs an error message
			CleanExit(EXIT_FAILURE);
			break;
		}
	}
}

void CleanExit(int code) {
	if (lxi_connected) {
		lxi_disconnect(params.hlxi);
	}
	if (data != NULL) {
		free(data);
	}
	if (file != NULL) {
		fclose(file);
	}
	exit(code);
}

int lxi_txrx(int device, char const * send, char * buff, int length, int timeout) {
	int retval;

	printf("lxi_send: Sending \"%s\"\n", send);
	retval = lxi_send(device, send, strlen(send), timeout);
	if (retval == LXI_ERROR) {
		return -1;
	}

	retval = lxi_receive (device, buff, length - 1,  timeout);
	if (retval == LXI_ERROR) {
		PRINT_LIBERROR("lxi_receive", "Data reception failed");
		return -1;
	}
	if (retval == length - 1) {
		PRINT_LIBERROR("lxi_receive","Possible truncation of message received");
		return -1;
	}
	buff[retval] = '\0'; // Conversion to c-string
	printf("lxi_receive: Received \"%s\"\n", buff);

	return 0;
}

int lxi_getparams() {
	char buff [100], unit [5];
	int retval;

	if ( lxi_txrx(params.hlxi, "*IDN?", buff, 100, LXI_TIMEOUT) == LXI_ERROR) {
		// lxi_txrx already prints an error message
		return -1;
	}
	if ( lxi_txrx(params.hlxi, "TDIV?", buff, 100, LXI_TIMEOUT) == LXI_ERROR) {
		return -1;
	}
	if ( lxi_txrx(params.hlxi, "C1:VDIV?", buff, 100, LXI_TIMEOUT) == LXI_ERROR) {
		return -1;
	}
	retval = sscanf(buff, "C1:VDIV %f%c", &params.v_div, unit);
	if (retval != 2) {
		puts("ERROR: Could not determine number of voltage divisions");
		return -1;
	}

	if ( lxi_txrx(params.hlxi, "C1:OFST?", buff, 100, LXI_TIMEOUT) == LXI_ERROR) {
		return -1;
	}
	retval = sscanf(buff, "C1:OFST %f%c", &params.v_off, unit);
	if (retval != 2) {
		PRINT_ERRMSG("Could not get value for voltage offset");
		return -1;
	}

	if ( lxi_txrx(params.hlxi, "SANU? C1", buff, 100, LXI_TIMEOUT) == LXI_ERROR) {
		return -1;
	}
	retval = sscanf (buff, "SANU %f%s", &params.leng, unit);
	if (retval != 2) {
		PRINT_ERRMSG("Could not get value for number of sample points");
		return -1;
	}

	if (strcmp(unit, "pts") == 0) {
		// No need to scale the length variable
	}
	else if (strcmp(unit, "M") == 0) {
		params.leng *= 1e6;
	}
	else if (strcmp(unit, "k") == 0) {
		params.leng *= 1e3;
	}
	else {
		PRINT_ERRMSG("Could not understand unit for number of sample points");
		return -1;
	}

	return 0;
}
