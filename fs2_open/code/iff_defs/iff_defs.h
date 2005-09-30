/*
 * Created by Ian "Goober5000" Warfield for the Freespace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */

/*
 * $Logfile: /Freespace2/code/iff_defs/iff_defs.h $
 * $Revision: 1.2 $
 * $Date: 2005-09-30 03:40:40 $
 * $Author: Goober5000 $
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2005/09/27 05:25:18  Goober5000
 * initial commit of basic IFF code
 * --Goober5000
 *
 */


#ifndef _IFF_DEFS_H_
#define _IFF_DEFS_H_

#include "globalincs/pstypes.h"
#include "globalincs/globals.h"
#include "graphics/2d.h"




// PLEASE NOTE: There is currently a conflict between the IFF colors (below)
// and the IFF colors as defined in ship.h.  Since neither iff_defs file
// #includes ship.h, everything is fine.  But this will need to be resolved
// before making the final commit.
//
// Incidentally, instead of using BRIEF_IFF_FRIENDLY, HOSTILE, NEUTRAL, change
// these to BRIEF_TEXT_*, etc. and update the Brief_text_colors array and
// references accordingly.



// Goober5000 - new IFF color system
#define IFF_COLOR_SELECTION			0
#define IFF_COLOR_MESSAGE			1
#define IFF_COLOR_TAGGED			2
#define MAX_IFF_COLORS				(MAX_IFFS + 3)

extern color Iff_colors[MAX_IFF_COLORS][2];


// iff flags
#define IFFF_EXEMPT_FROM_ALL_TEAMS_AT_WAR	(1 << 0)
#define MAX_IFF_FLAGS						1

// Goober5000
typedef struct iff_info {

	// required stuff
	char iff_name[NAME_LENGTH];
	int color_index;

	// relationships
	int attack_bitmask;
	int observed_color_index[MAX_IFFS];

	// flags
	int flags;
	int default_ship_flags;


	// constructor to initialize everything to 0
	iff_info()
	{
		memset(this, 0, sizeof(iff_info));
	}

} iff_info;


extern int Num_iffs;
extern iff_info Iff_info[MAX_IFFS];

extern int Iff_traitor;



// load the iff table
void iff_init();


#endif
