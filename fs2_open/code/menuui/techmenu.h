/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/MenuUI/TechMenu.h $
 * $Revision: 2.2 $
 * $Date: 2003-08-22 07:24:07 $
 * $Author: Goober5000 $
 *
 * Header file for code that controls the Tech Room menu
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  2003/03/03 04:28:37  Goober5000
 * fixed the tech room bug!  yay!
 * --Goober5000
 *
 * Revision 2.0  2002/06/03 04:02:24  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:10  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 5     8/23/99 11:20a Jefff
 * Increased TECH_INTEL_DESC_LEN
 * 
 * 4     8/10/99 3:45p Jefff
 * Put the smack down on the tech room.  Its all new, but tastefully done.
 * 
 * 3     10/13/98 2:47p Andsager
 * Remove reference to Tech_shivan_species_avail
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 11    5/05/98 1:49a Lawrance
 * Add in missing help overlays
 * 
 * 10    4/23/98 10:42p Hoffoss
 * Added species section to techroom.  Still missing description text,
 * because this hasn't been created yet.
 * 
 * 9     4/14/98 10:24p Hoffoss
 * Started on new tech room.
 * 
 * 8     4/02/98 5:40p Hoffoss
 * Added the Load Mission screen to FreeSpace.
 * 
 * 7     8/31/97 6:38p Lawrance
 * pass in frametime to do_frame loop
 * 
 * 6     11/21/96 7:14p Lawrance
 * converted menu code to use a file (menu.tbl) to get the data for the
 * menu
 * 
 * 5     11/13/96 4:02p Lawrance
 * complete over-haul of the menu system and the states associated with
 * them
 * 
 * 4     11/13/96 8:32a Lawrance
 * streamlined menu code
 * 
 * 3     11/06/96 8:54a Lawrance
 * added revision templates, made more efficient
 *
 * $NoKeywords: $
 *
*/


#ifndef _TECHMENU_H
#define _TECHMENU_H

#define MAX_INTEL_ENTRIES			30
#define TECH_INTEL_DESC_LEN		5120

typedef struct {
	char name[32];
	char desc[TECH_INTEL_DESC_LEN];
	char anim_filename[32];
	int  flags;
} intel_data;

// flags by Goober5000
#define IIF_DEFAULT_VALUE				0
#define IIF_IN_TECH_DATABASE			(1 << 0)	// in tech database? - Goober5000
#define IIF_DEFAULT_IN_TECH_DATABASE	(1 << 1)	// in tech database by default? - Goober5000

extern intel_data Intel_info[MAX_INTEL_ENTRIES];
extern int Intel_info_size;


// function prototypes
void techroom_init();
void techroom_close();
void techroom_do_frame(float frametime);
int techroom_on_ships_tab();
void techroom_intel_init();			// called on startup so campaigns can manipulate tech room visibility
int intel_info_lookup(char *name);
extern void tech_reset_to_default();

#endif
