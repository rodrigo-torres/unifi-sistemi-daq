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

#ifndef DAQ_SETUP_H
#define DAQ_SETUP_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>
#include <gpib/ib.h>
#include <unistd.h>
#include <sys/time.h>


#define GPIB_EOS_CHAR 0x00A
#define GPIB_EOS_ENBL 0x400
#define GPIB_EOS_FLAG (GPIB_EOS_ENBL | GPIB_EOS_CHAR)
#define GPIB_ERR_FLAG (0x8000 | 0x4000)
#define GPIB_BUFF_SIZE 20
#define GPIB_SEM_NAME_SIZE 50

//! @brief
//!
typedef struct {
	int dev;			///< GPIB device address
	int idev;			///< GPIB device ID assigned by ibdev
	int chan;			///< HP 6627A DC power supply channel
	int lock;			///< GPIB lock file descriptor
	float v_start;		///< Start value of voltage scan range
	float v_stop;		///< Stop value of voltage scan range
	float v_step;		///< Computed voltage step value
	float i_lim;		///< Current limit for HP 6627A DC power supply
	float rate;			///< Cadence of measurements for a single voltage step
	int nt;				///< Number of measurements per voltage step
	int nv;				///< Number of voltage steps for v_start to v_stop
	float * time;				///< Data array for time points
	float * volt;				///< Data array for voltage samples
	float * curr;				///< Data array for current samples
	struct timeval origin;		///< Records the beginning of the DAQ session
	struct timeval next;		///< Records time-point of next measurement
	struct timeval interval;	///< Time interval between successive samples
} DAQParamsTypedef;

static DAQParamsTypedef params;

//! @brief
//!
static void GPIB_Setup();

//! @brief
//!
static void GPIB_Test();

//! @brief
//!
//! @param params
//! @param message
static void GPIB_Send(DAQParamsTypedef * params, char const * message);

//! @brief
//!
//! @param params
//! @param index
//! @return
static float GPIB_Measure (DAQParamsTypedef * params, int index);

//! @brief
//!
//! @param params
//! @param code
static void GPIB_ReleaseLockUponFailure(DAQParamsTypedef * params, int code);

static void GPIB_Setup() {

	// Open the GPIB device
	params.idev = ibdev(0, params.dev, 0, 11, 1, GPIB_EOS_FLAG);
	if (params.idev == -1) {
		printf("GPIB_ERR %d: Couldn't open the GPIB device\n", iberr);
		CleanExit(-1);
	}

	char buff [50];
	FILE * gpib_sem = NULL;

	snprintf(buff, 50, "/var/lock/gpib/%d", params.dev);
	if ( (gpib_sem = fopen(buff, "w")) == NULL ) {
		printf("Couldn't open the lock file for the specified GPIB address\n");
		CleanExit(-1);
	}

	// Get the file descriptor for the measurement
	params.lock = fileno(gpib_sem);

	// Set the current limit for the measurement
	sprintf(buff, "iset %d,%f", params.chan, params.i_lim);
	GPIB_Send(&params, buff);
}

///
/// Simple turn ON / turn OFF test of lamp
static void GPIB_Test() {
	char buff [20];

	sprintf(buff, "vset %d,5", params.chan);
	GPIB_Send(&params, buff);

	sleep(5);

	sprintf(buff, "vset %d,0", params.chan);
	GPIB_Send(&params, buff);
}

static void GPIB_Send(DAQParamsTypedef * params, char const * message) {
	if (message == NULL) {
		printf("GPIB_ERR: Sending message from NULL pointer!\n");
		return;
	}

	printf("GPIB: Sending \"%s\" to %d\n", message, params->dev); // log message

	// ENTERING "critical" section
	if ( lockf(params->lock, F_LOCK, 0) == -1) {
		printf("GPIB_ERR: Failed to obtain lock for GPIB device.\n");
		CleanExit(-1);
	}

	ibwrt(params->idev, message, strlen(message));

	// EXITING "critical" section
	if ( lockf(params->lock, F_ULOCK, 0) == -1) {
		printf("GPIB_ERR: Failed to unlock GPIB device.\n");
		CleanExit(-1);
	}

	if ((ibsta & 0x8000)) { // Error in last GPIB operation
		printf("GPIB_ERR: Send failed with status 0x%x and code 0x%x\n",
				ibsta, iberr);
		CleanExit(-1);
	}

}

static float GPIB_Measure (DAQParamsTypedef * params, int index) {
	static struct timeval before, after, diff;
	static char vbuf [GPIB_BUFF_SIZE], ibuf [GPIB_BUFF_SIZE];

	// Prepare the buffers to: ask for voltage, ask for current.
	sprintf(vbuf, "vout? %d", params->chan);
	sprintf(ibuf, "iout? %d", params->chan);

	// Sinchronize
	if (params->origin.tv_sec == params->next.tv_sec &&
			params->origin.tv_usec == params->next.tv_usec) {
		// Avoid double-sinchronization on first run
		timeradd(&params->next, &params->interval, &params->next);
	}
	else {
		timeradd(&params->next, &params->interval, &params->next);
		gettimeofday(&before, NULL);
		timersub(&params->next, &before, &after);
		int sleep_time = after.tv_sec * 1000000 + after.tv_usec;
		if (sleep_time) {
			usleep(sleep_time);
		}
	}

	// Enter critical section
	if ( lockf(params->lock, F_LOCK, 0) == -1) {
		printf("GPIB_ERR: Failed to obtain lock for GPIB device.\n");
		CleanExit(-1);
	}

	gettimeofday(&before, NULL);
	// Request voltage value
	ibwrt (params->idev, vbuf, strlen(vbuf));
	if (ibsta & GPIB_ERR_FLAG) {
		GPIB_ReleaseLockUponFailure(params, 0);
	}
	// Read voltage
	ibrd (params->idev, vbuf, GPIB_BUFF_SIZE - 1);
	if (ibsta & GPIB_ERR_FLAG) {
		GPIB_ReleaseLockUponFailure(params, 1);
	}
	vbuf[ibcnt] = '\0';

	// Request current value
	ibwrt (params->idev, ibuf, strlen(ibuf));
	if (ibsta & GPIB_ERR_FLAG) {
		GPIB_ReleaseLockUponFailure(params, 2);
	}
	// Read current
	ibrd (params->idev, ibuf, GPIB_BUFF_SIZE - 1);
	if (ibsta & GPIB_ERR_FLAG) {
		GPIB_ReleaseLockUponFailure(params, 3);
	}
	ibuf[ibcnt] = '\0'; // Now a C-string

	gettimeofday(&after, NULL);

	// Exit critical section
	if ( lockf(params->lock, F_ULOCK, 0) == -1) {
		printf("GPIB_ERR: Failed to unlock GPIB device.\n");
		CleanExit(-1);
	}

	// We exited critical section with time stats and V/I measurement
	// Timestamp will be difference between mesurement and origin
	timersub(&before, &params->origin, &diff);
	*(params->time + index) = (float)diff.tv_sec + (float)diff.tv_usec / 1000000;
	*(params->volt + index) = atof(vbuf);
	*(params->curr + index) = atof(ibuf);

	timersub(&after, &before, &diff);

	return (float)(diff.tv_sec * 1000) + (float)(diff.tv_usec / 1000);
}

static void GPIB_ReleaseLockUponFailure(DAQParamsTypedef * params, int code) {
	static char const * operation_code[4] = {
			"'voltage request'",
			"'voltage read'",
			"'current request'",
			"'current read'"
	};

	lockf(params->lock, F_ULOCK, 0);
	printf("GPIB_ERR: Measure failed at %s step with status 0x%x "
			"and code 0x%x\n", operation_code[code], ibsta, iberr);
	CleanExit(-1);
}


#endif // daq_setup.h



