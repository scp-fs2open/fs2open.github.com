/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __MISSIONCMDBRIEF_H__
#define __MISSIONCMDBRIEF_H__

#define CMD_BRIEF_STAGES_MAX	10

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"
#include "graphics/generic.h"

struct anim;
struct anim_instance;

struct cmd_brief_stage {
	SCP_string text;  // text to display
	char ani_filename[MAX_FILENAME_LEN];  // associated ani file to play
	char wave_filename[MAX_FILENAME_LEN]; // associated wav file to play
	int wave;  // instance number of above
};

struct cmd_brief {
	int num_stages;
	cmd_brief_stage stage[CMD_BRIEF_STAGES_MAX];
};

extern cmd_brief Cmd_briefs[MAX_TVT_TEAMS];
extern cmd_brief *Cur_cmd_brief;  // pointer to one of the Cmd_briefs elements (the active one)

void cmd_brief_init(int stages);
void cmd_brief_close();
void cmd_brief_do_frame(float frametime);
void cmd_brief_hold();
void cmd_brief_unhold();

void cmd_brief_pause();
void cmd_brief_unpause();

int mission_has_cmd_brief();

#endif // __MISSIONCMDBRIEF_H__
