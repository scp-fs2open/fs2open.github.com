/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Stats/Medals.h $
 * $Revision: 2.10 $
 * $Date: 2005-09-24 02:40:10 $
 * $Author: Goober5000 $
 * 
 * $Log: not supported by cvs2svn $
 * Revision 2.9  2005/09/14 20:03:40  taylor
 * fix ace badges not getting displayed in debriefing
 *
 * Revision 2.8  2005/07/13 03:35:32  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.7  2005/05/18 14:03:27  taylor
 * a medals crash fix that isn't influenced by the evil Java empire (Jens Granseuer)
 *
 * Revision 2.6  2005/05/14 16:16:48  phreak
 * medal_stuff needed a copy constructor and an overloaded assignment operator.
 *
 * Revision 2.5  2005/05/12 17:49:17  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 2.4  2005/05/08 20:20:46  wmcoolmon
 * Dynamically allocated medals
 *
 * Revision 2.3  2004/08/11 05:06:35  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.2  2004/03/05 09:02:05  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.1  2002/08/01 01:41:10  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 5     10/29/99 10:40p Jefff
 * hack to make german medal names display without actually changing them
 * 
 * 4     9/02/99 3:41p Jefff
 * changed badge voice handling to be similar to promotion voice handling
 * 
 * 3     8/26/99 8:49p Jefff
 * Updated medals screen and about everything that ever touches medals in
 * one way or another.  Sheesh.
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 8     4/10/98 4:51p Hoffoss
 * Made several changes related to tooltips.
 * 
 * 7     3/07/98 5:44p Dave
 * Finished player info popup. Ironed out a few todo bugs.
 * 
 * 6     1/27/98 4:23p Allender
 * enhanced internal scoring mechanisms.
 * 
 * 5     11/06/97 4:39p Allender
 * a ton of medal work.  Removed an uneeded elemen in the scoring
 * structure.  Fix up medals screen to apprioriate display medals (after
 * mask was changed).  Fix Fred to only display medals which may actually
 * be granted.  Added image_filename to player struct for Jason Hoffoss
 * 
 * 4     11/05/97 4:43p Allender
 * reworked medal/rank system to read all data from tables.  Made Fred
 * read medals.tbl.  Changed ai-warp to ai-warp-out which doesn't require
 * waypoint for activation
 *
 * $NoKeywords: $
 */

#ifndef FREESPACE_MEDAL_HEADER_FILE
#define FREESPACE_MEDAL_HEADER_FILE

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"

#pragma warning(push, 2)	// ignore all those warnings for Microsoft stuff
#include <vector>
#pragma warning(pop)

struct scoring_struct;
struct player;

#define MAX_BADGES	3
#define MAX_ASSIGNABLE_MEDALS		12				// index into Medals array of the first medal which cannot be assigned

extern scoring_struct *Player_score;

// NUM_MEDALS stored in scoring.h since needed for player scoring structure

typedef struct medal_stuff {
	char	name[NAME_LENGTH];
	char	bitmap[NAME_LENGTH];
	int	num_versions;
	int	kills_needed;
	int badge_num;

	//If this is a badge (kills_needed > 1)
	char voice_base[MAX_FILENAME_LEN];
	char *promotion_text;

	medal_stuff() {
		name[0] = '\0';
		bitmap[0] = '\0';
		num_versions = 1;
		kills_needed = 0;
		badge_num = 0;
		voice_base[0] = '\0';
		promotion_text = NULL;
	}

	~medal_stuff() {
		if (promotion_text) {
			vm_free(promotion_text);
			promotion_text = NULL;
		}
	}

	medal_stuff(const medal_stuff &m) { clone(m); }
	const medal_stuff &operator=(const medal_stuff &m);

private:
	void clone(const medal_stuff &m);

} medal_stuff;

/*
typedef struct badge_stuff {
	char voice_base[MAX_FILENAME_LEN];
	char *promotion_text;

	badge_stuff(){voice_base[0]='\0';promotion_text=NULL;}
	~badge_stuff(){if(promotion_text != NULL)vm_free(promotion_text);};
} badge_stuff;
*/

extern std::vector<medal_stuff> Medals;
//extern badge_stuff Badge_info[MAX_BADGES];
//extern int Badge_index[MAX_BADGES];				// array which contains indices into Medals to indicate which medals are badges

extern void parse_medal_tbl();

// modes for this screen
#define MM_NORMAL							0			// normal - run through the state code
#define MM_POPUP							1			// called from within some other tight loop (don't use gameseq_ functions)

// main medals screen
void medal_main_init(player *pl,int mode = MM_NORMAL);

// return 0 if the screen should close (used for MM_POPUP mode)
int medal_main_do();
void medal_main_close();

//void init_medal_palette();
void init_medal_bitmaps();
void init_snazzy_regions();
void blit_medals();
void blit_label(char *label,int *coords);
void blit_callsign();

// individual medals 

extern int Medal_ID;       // ID of the medal to display in this screen. Should be set by the caller

void blit_text();

void medals_translate_name(char *name, int max_len);

#endif
