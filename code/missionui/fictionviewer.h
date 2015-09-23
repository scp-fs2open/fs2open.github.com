/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */



#ifndef __FICTION_VIEWER_H__
#define __FICTION_VIEWER_H__

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"
#include "graphics/2d.h"


// since we may now have multiple possible fiction stages, activated by a formula
typedef struct fiction_viewer_stage {
	char story_filename[MAX_FILENAME_LEN];
	char font_filename[MAX_FILENAME_LEN];
	char voice_filename[MAX_FILENAME_LEN];

	char ui_name[NAME_LENGTH];
	char background[GR_NUM_RESOLUTIONS][MAX_FILENAME_LEN];

	int formula;
} fiction_viewer_stage;

extern SCP_vector<fiction_viewer_stage> Fiction_viewer_stages;


// management stuff
void fiction_viewer_init();
void fiction_viewer_close();
void fiction_viewer_do_frame(float frametime);

// fiction stuff
bool mission_has_fiction();
int fiction_viewer_ui_name_to_index(const char *ui_name);
void fiction_viewer_reset();
void fiction_viewer_load(int stage);

void fiction_viewer_pause();
void fiction_viewer_unpause();

#endif
