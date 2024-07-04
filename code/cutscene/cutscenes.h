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

#include "globalincs/flagset.h"
#include "globalincs/globals.h"
#include "globalincs/pstypes.h"

namespace Cutscene
{
	FLAG_LIST(Cutscene_Flags)
	{
		Viewable = 0,
		Always_viewable,
		Never_viewable,

		NUM_VALUES
	};
}

typedef struct cutscene_info
{
	char filename[MAX_FILENAME_LEN];
	char name[NAME_LENGTH];
	char* description;
	flagset<Cutscene::Cutscene_Flags> flags;
	SCP_map<SCP_string, SCP_string> custom_data;
} cutscene_info;

extern SCP_vector<cutscene_info> Cutscenes;

extern bool Movie_active;

// initializa table data
void cutscene_init();

void cutscenes_screen_init();

void cutscenes_screen_close();

void cutscenes_screen_do_frame();

void cutscene_mark_viewable(const char* filename);

int get_cutscene_index_by_name(const char* name);

#endif
