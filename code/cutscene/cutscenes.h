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

typedef struct cutscene_info {
	cutscene_info() :
		viewable(false)
	{}

	char	filename[MAX_FILENAME_LEN];
	char	name[NAME_LENGTH];
	char	*description;
	int		cd;
	bool	viewable;
} cutscene_info;

extern SCP_vector<cutscene_info> Cutscenes;

// initializa table data
void cutscene_init();
int cutscene_get_cd_num(char *filename);

void cutscenes_screen_init();
void cutscenes_screen_close();
void cutscenes_screen_do_frame();

int cutscenes_validate_cd(char *mve_name, int prompt_for_cd = 1);
void cutscene_mark_viewable(char *filename);

#endif
