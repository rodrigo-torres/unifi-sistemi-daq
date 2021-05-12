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


//! \file read
//! \brief Attempts to read from a GPIB device

#include "common.h"

#include <gpib/ib.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 100

int  gpib_address;					///< GPIB address of device

int main(int argc, char * argv []) {
	char buffer [BUFFER_SIZE];

	UNUSED(argc);
	UNUSED(argv);

	ProcessArguments(argc, argv);

	// Open the GPIB device
	int ud = ibdev(0, gpib_address, 0, 11, 1, 0x400 | 0x0A);
	if (ud == -1) {
		snprintf(dbg_buffer, DBG_BUFF_SIZE,
				"Could not open GPIB device, error %d", iberr);
		PRINT_LIB_ERR_MSG("ibdev", dbg_buffer);
		CleanExit(EXIT_FAILURE);
	}

	if (ibrd(ud, buffer, BUFFER_SIZE - 1) & GPIB_ERR_FLAG) {
		snprintf(dbg_buffer, DBG_BUFF_SIZE,
				"Reception failed with status %d, and error %d", ibsta, iberr);
		PRINT_MSG(dbg_buffer);
		CleanExit(EXIT_FAILURE);
	}

	buffer[ibcnt] = '\0';	// Transform byte vector into a C-string
	printf("main: Received %s\n", buffer);

	CleanExit(EXIT_SUCCESS);
	return 0;
}

void CleanExit(int code) {
	exit(code);
}

void PrintProgramUse() {
	puts("read - reads up to 100 bytes from a gpib device.");
	puts("");
	puts("Arguments:");
	puts("[REQUIRED] -a {address}\tgpib address of device");
	puts("[OPTIONAL] -h \t\tprint this help menu on the screen");
}

void ProcessArguments(int argc, char * argv []) {
	int c, address_set = 0;
	while ((c = getopt(argc,argv,"a:h")) != -1) {
		switch (c) {
		case 'a':
			gpib_address = atoi(optarg);
			if (gpib_address > 30 || gpib_address < 0) {
				PRINT_ERR_MSG("gpib address provided is outside of valid range");
				CleanExit(EXIT_FAILURE);
			}
			address_set = 1;
			break;
		case 'h':
			PrintProgramUse();
			CleanExit(EXIT_SUCCESS);
			break;
		default: // Includes '?' case
			PrintProgramUse();
			CleanExit(EXIT_FAILURE);
			break;
		}
	}

	if (!address_set) {
		PRINT_ERR_MSG("A required parameter is missing");
		CleanExit(EXIT_FAILURE);
	}
}
