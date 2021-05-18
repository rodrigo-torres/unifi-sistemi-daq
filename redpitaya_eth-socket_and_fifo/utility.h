/****************************************************************************
 * Copyright (C) 2021 by Rodrigo Torres										*
 *   																		*
 *   utility.h																*
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

/*! \file utility.h
 * 	\brief Defines some useful macros for debugging
 */

#ifndef UTILITY_H_
#define UTILITY_H_


#include <errno.h>
#include <stdlib.h>
#include <string.h>

// Check we are using an standard C99 compiler or more recent
#if defined(__STDC__) && (__STDC_VERSION__ >= 199901L)
#define STANDARD_C_1999
#endif

// Some utility macros for debugging, etc...
#if defined(STANDARD_C_1999)

/*! \def UNUSED(_x)
 *  \brief A macro to call on unused variables to avoid compiler warnings
 */
#define UNUSED(_x) (void)_x

/*! \def PRINT_STD_LIBERROR(_call)
 *  \brief A macro that prints out the error message of a library function \a
 *  _call that appropriately sets the errno variable.
 *
 *  Trace information (file, line number, and enclosing function) is also
 *  printed for debugging purposes.
 */
#define PRINT_STD_LIBERROR(_call) \
	fprintf(stderr, \
			"\nRuntime Error:\n"\
			"  File \"%s\", line %d, in %s, from call to %s\n" \
			"  %s reports: %s\n\n", \
			__FILE__,__LINE__,__func__,_call, _call, strerror(errno))

/*! \def PRINT_LIBERROR(_call, _errmsg)
 *  \brief A macro that prints out the error message \a _errmsg of a library
 *  function \a _call
 *
 *  Trace information (file, line number, and enclosing function) is also
 *  printed for debugging purposes.
 */
#define PRINT_LIBERROR(_call,_errmsg) \
	fprintf(stderr, \
			"\nRuntime Error:\n"\
			"  File \"%s\", line %d, in %s, from call to %s\n" \
			"  %s reports: %s\n\n", \
			__FILE__,__LINE__,__func__,_call, _call, _errmsg)

/*! \def PRINT_ERRMSG(_msg)
 *  \brief A macro that prints out a generic error message \a _msg
 *
 *  Trace information (file, line number, and enclosing function) is also
 *  printed out for debugging purposes.
 */
#define PRINT_ERRMSG(_msg) \
	fprintf(stderr, \
			"\nRuntime Error:\n"\
			"  File \"%s\", line %d, in %s\n" \
			"  %s reports: %s\n\n", \
			__FILE__,__LINE__,__func__,__func__, _msg)

/*! \def PRINT_DBGMSG(_msg)
 *  \brief A macro that prints out a generic debug message \a _msg. The
 *  enclosing function is specified in the resulting debug message.
 */
#define PRINT_DBGMSG(_msg) \
	fprintf(stdout, "%s: %s\n", __func__, _msg)

#else

#endif


#endif /* UTILITY_H_ */
