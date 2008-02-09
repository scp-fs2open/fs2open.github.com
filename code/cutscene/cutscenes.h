/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Cutscene/Cutscenes.h $
 * $Revision: 2.2 $
 * $Date: 2004-03-05 09:01:58 $
 * $Author: Goober5000 $
 *
 * Code for the cutscenes viewer screen
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2002/08/01 01:41:04  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:21  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:04  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 6     5/21/98 12:35a Lawrance
 * Tweak how CD is checked for
 * 
 * 5     5/10/98 10:05p Allender
 * only show cutscenes which have been seen before.  Made Fred able to
 * write missions anywhere, defaulting to player misison folder, not data
 * mission folder.  Fix FreeSpace code to properly read missions from
 * correct locations
 * 
 * 4     5/08/98 5:30p Lawrance
 * add cutscenes_validate_cd()
 * 
 * 3     5/08/98 4:07p Allender
 * more cutscene stuff
 * 
 * 2     4/17/98 6:33p Hoffoss
 * Made changes to the tech room group of screens.  Cutscenes screen is
 * now in a new file.
 *
 * $NoKeywords: $
 */

#ifndef _FREESPACE_CUTSCENES_SCREEN_HEADER_FILE
#define _FREESPACE_CUTSCENES_SCREEN_HEADER_FILE

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"

#define MAX_CUTSCENES	10

// this cutscene is always available.
#define INTRO_CUTSCENE_FLAG		(1<<0)

typedef struct cutscene_info
{
	char		filename[MAX_FILENAME_LEN];
	char		name[NAME_LENGTH];
	char		*description;
	int		cd;
} cutscene_info;

extern cutscene_info Cutscenes[MAX_CUTSCENES];
extern int Num_cutscenes;
extern int Cutscenes_viewable;

// initializa table data
void cutscene_init();
int cutscene_get_cd_num(char *filename);


void cutscenes_screen_init();
void cutscenes_screen_close();
void cutscenes_screen_do_frame();

int cutscenes_validate_cd(char *mve_name, int prompt_for_cd = 1);
void cutscene_mark_viewable(char *filename);

#endif
