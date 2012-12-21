/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _FREESPACE_LOGFILE_HEADER_FILE
#define _FREESPACE_LOGFILE_HEADER_FILE

// ----------------------------------------------------------------------------------------------------
// LOGFILE DEFINES/VARS
//
#define LOGFILE_MULTI_LOG		0
#define LOGFILE_EVENT_LOG		1


// ----------------------------------------------------------------------------------------------------
// MULTI LOGFILE FUNCTIONS
//

// initialize the multi logfile
bool logfile_init(int logfile_type);

// close down the multi logfile
void logfile_close(int logfile_type);

// printf function itself called by the log_printf macro
void log_printf(int logfile_type, char *format, ...);

// string print function
void log_string(int logfile_type, const char *string, int add_time = 0);

#endif
