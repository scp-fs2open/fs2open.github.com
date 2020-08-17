/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _TECHMENU_H
#define _TECHMENU_H

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"

typedef struct {
	char name[NAME_LENGTH];
	SCP_string desc;
	char anim_filename[NAME_LENGTH];
	int  flags;
} intel_data;

// flags by Goober5000
#define IIF_DEFAULT_VALUE				0
#define IIF_IN_TECH_DATABASE			(1 << 0)	// in tech database? - Goober5000
#define IIF_DEFAULT_IN_TECH_DATABASE	(1 << 1)	// in tech database by default? - Goober5000

extern int Techroom_overlay_id;

extern SCP_vector<intel_data> Intel_info;

inline int intel_info_size()
{
	return static_cast<int>(Intel_info.size());
}

// function prototypes
void techroom_init();
void techroom_close();
void techroom_do_frame(float frametime);
void techroom_intel_init();			// called on startup so campaigns can manipulate tech room visibility
void techroom_intel_reset(); // for testing
int intel_info_lookup(const char *name);
extern void tech_reset_to_default();

#endif
