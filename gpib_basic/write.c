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

//! \file write.c
//! \brief Attempts to open a GPIB device at the address specified by -a, and
//! sends the command specified by -c

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gpib/ib.h>
#include <unistd.h>

#define BUFFER_SIZE 100

int  gpib_address;					///< GPIB address of device
char gpib_command [BUFFER_SIZE];	///< GPIB command to be sent


int main(int argc, char * argv []) {
	ProcessArguments(argc, argv);

	snprintf(dbg_buffer, DBG_BUFF_SIZE,
			"Sending command \"%s\" to address %d", gpib_command, gpib_address);
	PRINT_MSG(dbg_buffer);

	// Apre un device
	int ud = ibdev(0, gpib_address, 0, 11, 1, 0x400 | 0x0A);
	if (ud == -1) {
		snprintf(dbg_buffer, DBG_BUFF_SIZE,
				"Could not open GPIB device, error %d", iberr);
		PRINT_LIB_ERR_MSG("ibdev", dbg_buffer);
		CleanExit(EXIT_FAILURE);
	}

	// Send then command
	ibwrt(ud, gpib_command, strlen(gpib_command));

	snprintf(dbg_buffer, DBG_BUFF_SIZE, "Command sent with status %d", ibsta);
	PRINT_MSG(dbg_buffer);
	snprintf(dbg_buffer, DBG_BUFF_SIZE,
			"%d bytes of %d bytes sent", ibcnt, strlen(gpib_command));
	PRINT_MSG(dbg_buffer);

	CleanExit(EXIT_SUCCESS);
	return 0;
}

void CleanExit(int code) {
	exit(code);
}

void PrintProgramUse() {
	puts("write - sends a gpib command to the specified address.");
	puts("");
	puts("Arguments:");
	puts("[REQUIRED] -a {address}\tgpib address of device");
	puts("[REQUIRED] -c {string}\tcommand to be sent");
	puts("[OPTIONAL] -h \t\tprint this help menu on the screen");
}

void ProcessArguments(int argc, char * argv []) {
	int c, address_set = 0, command_set = 0;
	while ((c = getopt(argc,argv,"a:c:h")) != -1) {
		switch (c) {
		case 'a':
			gpib_address = atoi(optarg);
			if (gpib_address > 30 || gpib_address < 0) {
				PRINT_ERR_MSG("gpib address provided is outside of valid range");
				CleanExit(EXIT_FAILURE);
			}
			address_set = 1;
			break;
		case 'c':
			strncpy(gpib_command, optarg, BUFFER_SIZE);
			if (gpib_command[BUFFER_SIZE - 1] != '\0') {
				// Command was truncated and gpib_command is not a C-string
				PRINT_ERR_MSG("gpib command provided is longer than 100 bytes");
				CleanExit(EXIT_FAILURE);
			}
			command_set = 1;
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

	if (!address_set || !command_set) {
		PRINT_ERR_MSG("One or more required parameters are missing");
		CleanExit(EXIT_FAILURE);
	}
}
