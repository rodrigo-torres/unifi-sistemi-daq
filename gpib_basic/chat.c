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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gpib/ib.h>
#include <sys/time.h>
#include <unistd.h>

#define BUFFER_SIZE 100
#define GPIB_SEM_NAME_SIZE 50


int  gpib_address;					///< GPIB address of device

char gpib_sem_name [GPIB_SEM_NAME_SIZE]; ///< Filename of GPIB semaphore
FILE * gpib_sem = NULL;

bool run_program = true;
int fd; // File descriptor for the GPIB pseudofile lock

void ChatWithInstrument(int ud);
int GetInputFromUser(char * input, int length);

int main(int argc, char * argv []) {
  ProcessArguments(argc, argv);

  // Open the GPIB device
  int ud = ibdev(0, gpib_address, 0, 11, 1, GPIB_EOS_FLAG);
  if (ud == -1) {
	snprintf(dbg_buffer, DBG_BUFF_SIZE,
			"Could not open GPIB device, error %d", iberr);
	PRINT_LIB_ERR_MSG("ibdev", dbg_buffer);
	CleanExit(EXIT_FAILURE);
  }

  snprintf(gpib_sem_name,GPIB_SEM_NAME_SIZE,"/var/lock/gpib/%d",gpib_address);
  if ( (gpib_sem = fopen(gpib_sem_name, "w")) == NULL ) {
	PRINT_LIB_ERR_MSG("fopen", "Could not open GPIB semaphore file");
	CleanExit(EXIT_FAILURE);
  }
  
  ChatWithInstrument(ud);
  
  CleanExit(EXIT_SUCCESS);
  return 0;
}

void CleanExit(int code) {
	if (gpib_sem) {
		fclose(gpib_sem);
	}
	exit(code);
}

void PrintProgramUse() {
	puts("chat - interactive chat with a GPIB device.");
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



void ChatWithInstrument(int ud) {
  static char buffer [BUFFER_SIZE];	   // buffer for chatting
  struct timeval tempistica [3], diff; // timeval structs for statistics
  int nread = 0; 				// number of bytes read from user input
  int wrt_stat, rd_stat;  	// status variables for read/write operations

  while (run_program) {
	PRINT_MSG("Ready for user input...");


    nread = GetInputFromUser(buffer, BUFFER_SIZE);
    if (nread == BUFFER_SIZE) {
    	PRINT_MSG("The input string is longer than 100 bytes!");
    	break;
    } else {
    	buffer[nread] = '\0'; // Transform vector into C-string
    }

    if (!run_program) { // Prompts program termination
      PRINT_MSG("Terminating program upon user's request");
      return;
    }

    // ENTERING "critical" section 
    if ( lockf(fd, F_LOCK, 0) == -1 ) {
      PRINT_LIB_ERR_MSG("lock", "Failed to lock GPIB semaphore");
      return;
    }   
    
    // Send then command
    gettimeofday(tempistica + 0, NULL);
    if (buffer[0]) {
      wrt_stat = ibwrt (ud, buffer, nread);
    }   
    gettimeofday(tempistica + 1, NULL);
    if ( !(ibsta & 0x8000) ) {
      // Read the reply if there was no error in the transmission
      rd_stat = ibrd (ud, buffer, BUFFER_SIZE - 1);
    }    
    gettimeofday(tempistica + 2, NULL);

    // EXITING "critical" section
    if ( lockf(fd, F_ULOCK, 0) == -1 ) {
        PRINT_LIB_ERR_MSG("lock", "Failed to unlock GPIB semaphore");
        return;
    }
    
    buffer[ibcnt] = '\0'; // ASCII vector to C-string
    
    printf("Received reply : \"%s\"\n", buffer);
    
    // Print some statitics onto the screen
    timersub(tempistica + 1, tempistica + 0, &diff);
    printf("Write time (usec): %d\n", 1000000 * diff.tv_sec + diff.tv_usec);
    timersub(tempistica + 2, tempistica + 1, &diff);
    printf("Read time (usec): %d\n", 1000000 * diff.tv_sec + diff.tv_usec);
    printf("Write / Read status: %x / %x \n", wrt_stat, rd_stat);
  }
}


int GetInputFromUser(char * input, int length) {
  int c;

  for (int j = 0; j < BUFFER_SIZE; ++j) {
    c = getchar();

    switch (c) {
    case EOF:
    	run_program = false;
    	//no break
    case '\n':
    	// No more input to process
    	return j;
    default:
    	input[j] = (char)c;
    	break;
    }
  }

  return length;
}

