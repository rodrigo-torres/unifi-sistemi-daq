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

//! \file common.h
//! @brief Defines common macros and function prototypes used by the three
//! basic gpib programs
//!

#ifndef COMMON_H_
#define COMMON_H_

#include "utility.h"


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

//! @brief Prints out a short manual page outlining the use of this program
//!
void PrintProgramUse();

//! @brief Performs garbage collection of global variables before triggering
//! program termination
//!
//! @param code is the termination code provided to the call of exit()
void CleanExit (int code);


#endif /* COMMON_H_ */
