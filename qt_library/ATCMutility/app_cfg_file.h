/** 
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief HMI API to manage a configuration file
 */
#ifndef _APP_CFG_FILE_H_
#define _APP_CFG_FILE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define MAX_LINE 1024

/**
 * read an entry into the configuration file
 *
 * @param filename: configuration file name
 * @param label: parameter name to read
 * @param value: parameter value read
 *
 * @return	0 read, <0 error
 *
 */
int readCfgVal(const char * filename, const char * valuename, char * valueread);

/**
 * update an entry into the configuration file
 *
 * @param filename configuration file name
 * @param label parameter name to add or update
 * @param value parameter value to add or update
 *
 * @return	0 added, 1 updated, <0 error
 *
 */
int writeCfgVal(const char * filename, const char * label, const char * value);

/**
 * read an entry into the configuration file with a bash syntax
 *
 * @param filename: configuration file name
 * @param label: parameter name to read
 * @param value: parameter value read
 *
 * @return	0 read, <0 error
 *
 */
int readCfgValBash(const char * filename, const char * valuename, char * valueread);

/**
 * update an entry into the configuration file with a bash syntax
 *
 * @param filename configuration file name
 * @param label parameter name to add or update
 * @param value parameter value to add or update
 *
 * @return	0 added, 1 updated, <0 error
 *
 */
int writeCfgValBash(const char * filename, const char * label, const char * value);

/**
 * delete an entry from a configuration file
 *
 * @param filename configuration file name
 * @param label parameter name to add or update
 *
 * @return	0 not found, 1 deleted, <0 error
 *
 */
int deleteCfgVal(const char * filename, const char * label);

#ifdef __cplusplus
}
#endif
#endif
