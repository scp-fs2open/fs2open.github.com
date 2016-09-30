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

typedef struct cutscene_info
{
	char filename[MAX_FILENAME_LEN];
	char name[NAME_LENGTH];
	char* description;
	bool viewable;
} cutscene_info;

#if SCP_COMPILER_IS_GNU
#pragma GCC diagnostic push
// This suppresses a GCC bug where it thinks that the Cutscenes from the enum class below shadows a global variable
#pragma GCC diagnostic ignored "-Wshadow"
#endif
extern SCP_vector<cutscene_info> Cutscenes;
#if SCP_COMPILER_IS_GNU
#pragma GCC diagnostic pop
#endif

// initializa table data
void cutscene_init();

void cutscenes_screen_init();

void cutscenes_screen_close();

void cutscenes_screen_do_frame();

void cutscene_mark_viewable(const char* filename);

#endif
