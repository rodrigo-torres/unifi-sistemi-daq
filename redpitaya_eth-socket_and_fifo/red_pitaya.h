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

#ifndef RED_PITAYA_H
#define RED_PITAYA_H

#include "utility.h"

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>


#define RPITAYA_ADDR  "rp-f06e73.local"
#define RPITAYA_PORT  "8900"
#define RPITAYA_HEAD_OFFSET 30

static int RedPitaya_Connect();

static int RedPitaya_Connect() {
	struct addrinfo hints, * paddr, * naddr;
	int status, fd;

	// Initialize the structure for an IPv4/TCP connection
	memset(&hints, 0, sizeof (struct addrinfo)); // Initialization IS required
	hints.ai_flags    = 0;
	hints.ai_family	  = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;

	if ((status = getaddrinfo(RPITAYA_ADDR, RPITAYA_PORT, &hints, &naddr)) != 0) {
		PRINT_LIBERROR("getaddrinfo", gai_strerror(status));
		return -1;
	}

	paddr = naddr;
	do { // If getaddrinfo succeeds there will be at least one address
		printf("RedPitaya_Connect: Found address");
		for (int i = 0; i < naddr->ai_addrlen; ++i) {
			printf(" %02x", *((uint8_t *)naddr->ai_addr + i));
		}
		printf("\n");
		naddr = naddr->ai_next;
	} while (naddr != NULL);

	if (paddr->ai_next != NULL) {
		PRINT_ERRMSG("Cannot proceed! More than one choice for connection!");
		freeaddrinfo(paddr);
		return -1;
	}

	// Create a socket for the connection
	if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
		PRINT_STD_LIBERROR("socket");
		freeaddrinfo(paddr);
		return -1;
	}

	// Connect to the Red Pitaya
	if ( (status = connect(fd, paddr->ai_addr, paddr->ai_addrlen)) == -1 ) {
		PRINT_STD_LIBERROR("connect");
		freeaddrinfo(paddr);
		close(fd);
		return -1;
	}
	printf("RedPitaya_Connect: Connected to service %s on address %s\n",
			RPITAYA_PORT, RPITAYA_ADDR);

	freeaddrinfo(paddr);
	return fd;
};

#endif // red_pitaya.h



