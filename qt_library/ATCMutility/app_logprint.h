/** 
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Function prototypes and macro for logging and debug.
 */
#ifndef _APP_LOG_PRINT_H_
#define _APP_LOG_PRINT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#ifndef LOG_LEVEL
	//#define LOG_LEVEL info_e
#ifndef LOG_DISABLED
#define LOG_DISABLED
#endif
#endif

	/**
	 *
	 * @brief log level structure.
	 *
	 * @param none		suppress log
	 *
	 * @param error		print error log level
	 *
	 * @param warning	print warning log level
	 *
	 * @param info		print info log level
	 *
	 * @param verbose	print verbose log level
	 *
	 * @ingroup app_logprint
	 *
	 */
	enum log_level
	{
		none_e,
		error_e,
		warning_e,
		info_e,
		verbose_e
	};

	extern const char * _severity_[];

	/**
	 *
	 * @brief error map structure
	 *
	 * @param no_error		no error occured
	 *
	 * @param gen_error		a generic error occured
	 *
	 * @param last_error	number of error type
	 *
	 */
	enum error_type_e
	{
		no_error,
		gen_error,
		frmt_error,
		malfrm_error,
		missing_error,
		perm_error,
		last_error
	};

	extern int _error_code_;
	extern const char * _strerror_map_[];

	/**
	 *
	 * @brief macro to decode the error code into a string
	 *
	 * @param error		error code
	 *
	 * @ingroup app_logprint
	 *
	 */
#define STRERROR() _strerror_map_[_error_code_]


	/**
	 *
	 * @brief macro to print the log
	 *
	 * @param level 	log level
	 *
	 * @param format	log format
	 *
	 * @param args		argument to print
	 *
	 * @ingroup app_logprint
	 *
	 */
#ifndef LOG_DISABLED
#define LOG_PRINT(level, format, args...)  \
	if (level <= LOG_LEVEL) \
	{ \
		fprintf (stderr, "[%s:%s:%d] - %s", __FILE__, __func__, __LINE__, _severity_[level]); \
		fprintf (stderr, format , ## args); \
		fflush(stderr); \
	}
#else
#define LOG_PRINT(level, format, args...)
#endif
	/**
	 *
	 * @brief macro to print the log without the file and line info
	 *
	 * @param level 	log level
	 *
	 * @param format	log format
	 *
	 * @param args		argument to print
	 *
	 * @ingroup app_logprint
	 *
	 */

#ifndef LOG_DISABLED
#define LOG_PRINT_NO_INFO(level, format, args...)  \
	if (level <= LOG_LEVEL) \
	{ \
		fprintf (stderr, format , ## args); \
		fflush(stderr); \
	}
#else
#define LOG_PRINT_NO_INFO(level, format, args...)
#endif

#ifdef __cplusplus
}
#endif
#endif

