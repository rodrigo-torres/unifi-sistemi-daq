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
#include <stdarg.h>
#include <stdio.h>
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

/*! \def PRINT_DBGMSG(_msg)
 *  \brief Prints out a generic debug message \a _msg. The enclosing function is
 *  also reported in the message
 *
 */
#define PRINT_MSG(_msg) \
	fprintf(stdout, "%s: %s\n", __func__, _msg)

/*! \def PRINT_TRACE_MSG(_msg)
 *  \brief Prints out a debug message that is more specific than \see PRINT_MSG.
 *  The filename, line number and enclosing function are reported in the message.
 *
 */
#define PRINT_TRACE_MSG(_msg) \
	fprintf(stdout, "File \"%s\", line %d, in %s: %s\n", \
			__FILE__,__LINE__,__func__, _msg)

/*! \def PRINT_STD_LIBERROR(_call)
 *  \brief Prints out the error message of a library function \a _call that
 *  appropriately sets the errno variable. Trace information is included in
 *  the message
 *
 */
#define PRINT_ERRNO_MSG(_call) \
	fprintf(stderr, \
			"\nRuntime Error:\n"\
			"  File \"%s\", line %d, in %s, from call to %s\n" \
			"  %s reports: %s\n\n", \
			__FILE__,__LINE__,__func__,_call, _call, strerror(errno))

/*! \def PRINT_LIBERROR(_call, _errmsg)
 *  \brief A macro that prints out the error message \a _errmsg of a library
 *  function \a _call. Trace information is included in the message
 *
 */
#define PRINT_LIB_ERR_MSG(_call,_errmsg) \
	fprintf(stderr, \
			"\nRuntime Error:\n"\
			"  File \"%s\", line %d, in %s, from call to %s\n" \
			"  %s reports: %s\n\n", \
			__FILE__,__LINE__,__func__,_call, _call, _errmsg)

/*! \def PRINT_ERRMSG(_msg)
 *  \brief A macro that prints out a generic error message \a _msg. The
 *  filename, line number and enclosing function are reported in the message.
 */
#define PRINT_ERR_MSG(_msg) \
	fprintf(stderr, \
			"\nRuntime Error:\n"\
			"  File \"%s\", line %d, in %s\n" \
			"  %s reports: %s\n\n", \
			__FILE__,__LINE__,__func__,__func__, _msg)

//! \def DBF_BUFF_SIZE
//! \brief Specifies the size of the debug message buffer
//!
#define DBF_BUFF_SIZE 100

//! @brief A typedef for an enum listing the types of different debug messages
//! available for use with the \see DebugMessage function
//!
typedef enum {
	kDebugMsg = 0,//!< kDebugMsg A debug message with enclosing function info
	kTraceMsg,    //!< kTraceMsg A debug message with trace info
	kLibError,    //!< kLibError An error message specifying the name of the
	 	 	 	  //!< library function that reported the error
	kErrorMSg     //!< kErrorMSg An error message
} DBG_MessageType;

char dbg_buffer [DBF_BUFF_SIZE]; ///< A buffer for the debug message

//! @brief Prints out a debug message with a format specified by the value of
//! the \a type argument.
//!
//! @param type of debug message to be printed out
//! @param function name of the library function that reported the error. If no
//! specific function reported the error, an empty C-string "" should be used.
//! @param format string as it would be passed to a call to printf
void DebugMessage(DBG_MessageType type,
					char const * function, char const * format, ...) {

	va_list args;
	va_start (args, format);
	vsnprintf(dbg_buffer, DBF_BUFF_SIZE, format, args);
	va_end (args);

	switch (type) {
		case kTraceMsg:
			PRINT_TRACE_MSG(dbg_buffer);
			break;
		case kDebugMsg:
			PRINT_MSG(dbg_buffer);
			break;
		case kLibError:
			PRINT_LIB_ERR_MSG(function, dbg_buffer);
			break;
		case kErrorMSg:
			PRINT_ERR_MSG(dbg_buffer);
			break;
		default:
			PRINT_ERR_MSG("Unrecognized debug message type");
			break;
	}
}

//! @brief Prints out an error message as specified by \see PRINT_ERRNO_MSG
//!
//! @param function name of the library function that set the errno variable
void DebugErrNumMessage(char const * function) {
	PRINT_ERRNO_MSG(function);
}


#else
#error "Please compile with a C99 compliant compiler"
#endif



#endif /* UTILITY_H_ */
