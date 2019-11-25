/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
 */

#ifndef _FREESPACE_CUTSCENES_SCREEN_HEADER_FILE
#define _FREESPACE_CUTSCENES_SCREEN_HEADER_FILE

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"

#define CF_VIEWABLE			(1<<0)
#define CF_ALWAYS_VIEWABLE	(1<<1)
#define CF_NEVER_VIEWABLE	(1<<2)

typedef struct cutscene_info
{
	char filename[MAX_FILENAME_LEN];
	char name[NAME_LENGTH];
	char* description;
	int flags;
} cutscene_info;

extern SCP_vector<cutscene_info> Cutscenes;

// initializa table data
void cutscene_init();

void cutscenes_screen_init();

void cutscenes_screen_close();

void cutscenes_screen_do_frame();

void cutscene_mark_viewable(const char* filename);

#endif
