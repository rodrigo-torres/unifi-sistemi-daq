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

#include "utility.h"

#include <sys/time.h>
#include <unistd.h>


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char input [100];
struct timeval * vtime = NULL;

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

int main() {
	// Each sample takes about 16 bytes in memory
	int const kMaxSamples = HintMaxMemoryAvailable() / sizeof (struct timeval);

	printf("Maximum number of samples allowed is %d.\n", kMaxSamples);
	printf("How many samples should I take? (Input a number)\n\n");


	// Safe, copies a maximum of specified characters
	if (fgets(input, sizeof (input), stdin) == NULL) {
		DebugMessage(kDebugMsg, "", "No input received");
		//PRINT_DBGMSG("No input received");
		CleanExit(EXIT_FAILURE);
	}

	long int samples = strtol(input, NULL, 10);
	if (errno != 0) {
		DebugErrNumMessage("strtol");
		//PRINT_STD_LIBERROR("strtol");
		CleanExit(EXIT_FAILURE);
	}
	else if (samples == 0) {
		DebugMessage(kLibError, "strtol", "Possible conversion error");
		//PRINT_LIBERROR("strtol", "Taking 0 samples, possible conversion error");
		CleanExit(EXIT_FAILURE);
	}
	else if (samples > kMaxSamples) {
		DebugMessage(kDebugMsg,"","Truncating samples value to maximum allowed");
		//PRINT_DBGMSG("Truncating samples value to maximum allowed");
		samples = kMaxSamples;
	}

	DebugMessage(kDebugMsg, "", "Taking %llu samples", samples);

	if ( (vtime = calloc(samples, sizeof (struct timeval))) == NULL ) {
		DebugErrNumMessage("calloc");
		CleanExit(EXIT_FAILURE);
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

	printf("%-10u <= t (us) < %-11u counts: %d\n", 0, 1, histo[0]);
	for (int i = 1; i < 32; ++i) {
		printf("%-10u <= t (us) < %-11u counts: %d\n",
				0b1U << (i - 1), 0b1U << i, histo[i]);
	}

	CleanExit(EXIT_SUCCESS);
	return 0;
}


void CleanExit(int code) {
	// Specific clean-up procedure goe here
	if (vtime) {
		free(vtime);
	}

	DebugMessage(kDebugMsg, "", "Garbage has been collected");
	exit(code);
}


