/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _FREESPACE_MULTIPLAYER_LOGFILE_HEADER_FILE
#define _FREESPACE_MULTIPLAYER_LOGFILE_HEADER_FILE

#include "globalincs/pstypes.h"

// ----------------------------------------------------------------------------------------------------
// MULTI LOGFILE DEFINES/VARS
//


// ----------------------------------------------------------------------------------------------------
// MULTI LOGFILE FUNCTIONS
//

// initialize the multi logfile
void multi_log_init();

// close down the multi logfile
void multi_log_close();

// give some processing time to the logfile system so it can check up on stuff
void multi_log_process();

// printf function itself called by the ml_printf macro
void ml_printf(SCP_FORMAT_STRING const char *format, ...) SCP_FORMAT_STRING_ARGS(1, 2);

// string print function
void ml_string(const char *string, int add_time = 1);

#endif
