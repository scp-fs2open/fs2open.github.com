/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _MULTI_LAG_HEADER_FILE
#define _MULTI_LAG_HEADER_FILE

#ifndef NDEBUG
	// #define MULTI_USE_LAG								
#endif

#include "globalincs/pstypes.h"

#ifdef _WIN32
struct fd_set;
#endif
struct timeval;

// initialize multiplayer lagloss. in non-debug situations, this call does nothing
void multi_lag_init();

// shutdown multiplayer lag
void multi_lag_close();

// select for multi_lag
int multi_lag_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *except_fds, const timeval *timeout);

// recvfrom for multilag
int multi_lag_recvfrom(uint s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen);

#endif
