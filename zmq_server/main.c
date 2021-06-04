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

#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <zmq.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HIST_SIZE 8192

#ifdef TEST_SERVER
#define DEV_PATH "text.txt"
#else
#define DEV_PATH "/dev/silenar"
#endif
#define BUF_SIZE 100

//! \brief struct event defines the data for each SILENA ADC event
//!
struct event {
  int32_t tv_sec;   ///< Event timestamp, seconds field
  int32_t tv_usec;  ///< Event timestamp, microseconds field
  uint32_t value;   ///< Event ADC value
};

int daq_go = 1;			///< A status variable, set to 1 when DAQ is active
int running = 0;
int fd = -1;			///< A file descriptor for the char device

void * context = NULL;
void * responder = NULL;

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

int main(int argc, char * argv[]) {
	struct event event;
	int retval;

	UNUSED(argc);
	UNUSED(argv);
	
  // Handle interruption of server program
	if (signal (SIGINT, &SignalHandler) == SIG_ERR) {
		PRINT_STD_LIBERROR("signal");
		CleanExit(EXIT_FAILURE);
	}
	
	// Open the char device
	fd = open(DEV_PATH, O_RDWR, 0);
	if (fd == -1) {
	  PRINT_STD_LIBERROR("open");
	  CleanExit(EXIT_FAILURE);	  
	}
	
	// Configure the ZMQ socket
	context   = zmq_ctx_new();
	responder = zmq_socket(context, ZMQ_REP);
	
	if ( zmq_bind(responder, "tcp://*:5555") ) {
	  PRINT_STD_LIBERROR("zmq_bind");
	  CleanExit(EXIT_FAILURE);
	}

	PRINT_DBGMSG("Server started.");
	
	while (daq_go) {
	  // Wait for a connection
	  retval = zmq_recv(responder, buffer, BUF_SIZE - 1, 0);
	  if (retval == -1) {
	    PRINT_STD_LIBERROR("zmq_recv");
	    CleanExit(EXIT_FAILURE);
	  }
	  buffer[retval] = '\0';
		printf("received: %s\n", buffer);
	  
	  // Process the command with syntax: <COMMAND_ID> <ARG>
	  // COMMAND_ID shall be one single capital ASCII letter
	  // ARG depends on the COMMAND_ID and shall be a number, where appropriate
	  switch(buffer[0]) {
	  case 'S': // Start the acquisition 
      retval = write(fd, "S", 1);
      if (retval != 1) {
	      PRINT_STD_LIBERROR("write");
	      // Warn the client
	      zmq_send(responder, "ERR 1", 5, 0);
      }
      else {
        running = 1;
	      zmq_send(responder, "OK", 2, 0);
     
      }
      break;	  
	  case 'E': // Stop the acquisition 
      retval = write(fd, "S", 1);
      if (retval != 1) {
	      PRINT_STD_LIBERROR("write");
	      // Warn the client
	      zmq_send(responder, "ERR 2", 5, 0);
      }
      else {
        running = 0; 
	      zmq_send(responder, "OK", 2, 0);     
      }
      break;	  
	  case 'R': // Receive data
	    if(!running) { 
	      PRINT_DBGMSG("Data requested but DAQ is not running.");
	      zmq_send(responder, "ERR 3", 5, 0);	
	      break;    
	    }
	    // Read one event at a time
	    retval = read(fd, &event, sizeof (struct event));
	    if (retval != sizeof (struct event) || retval == -1) {	   
	      PRINT_STD_LIBERROR("read");
	      zmq_send(responder, "ERR 4", 5, 0);     
	    }
	    else {
	      sprintf(buffer, "OK ");
	      memcpy(buffer + 3, &event, sizeof(struct event));
	      zmq_send(responder, buffer, sizeof(struct event) + 3, 0);
	    }
	    break;
	  default:
	    PRINT_DBGMSG("Command not recognized.");
	    zmq_send(responder, "ERR 5", 5, 0);	  
	  }
	}

	// Stop the acquisition
	if (running) {
    retval = write(fd, "E", 1);
    if (retval != 1) {
	    PRINT_STD_LIBERROR("write");
	    CleanExit(EXIT_FAILURE);	
    }
	}

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
	zmq_close(responder);
	zmq_ctx_destroy(context);
	fflush(stdout);
	fflush(stderr);
	PRINT_DBGMSG("Garbage has been collected");
	exit(code);
}
