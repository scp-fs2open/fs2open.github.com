/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Stats/Scoring.h $
 * $Revision: 2.8.2.1 $
 * $Date: 2006-09-11 01:17:07 $
 * $Author: taylor $
 *
 * Scoring system structures, medals, rank, etc.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.8  2005/12/29 08:08:42  wmcoolmon
 * Codebase commit, most notably including objecttypes.tbl
 *
 * Revision 2.7  2005/07/13 03:35:32  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.6  2005/05/08 20:20:46  wmcoolmon
 * Dynamically allocated medals
 *
 * Revision 2.5  2005/02/04 10:12:33  taylor
 * merge with Linux/OSX tree - p0204
 *
 * Revision 2.4  2004/08/11 05:06:35  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.3  2004/03/05 09:02:05  Goober5000
 * Uber pass at reducing #includes
 * --Goober5000
 *
 * Revision 2.2  2003/01/27 00:51:08  DTP
 * Part of bumping MAX_SHIPS to 250 max_ship_types. Server now no more Crashes on kill, when max_shiptypes is above 200. Note Client still can't join. narrowing it down.
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
 * 6     8/30/99 5:01p Dave
 * Made d3d do less state changing in the nebula. Use new chat server for
 * PXO.
 * 
 * 5     8/26/99 8:49p Jefff
 * Updated medals screen and about everything that ever touches medals in
 * one way or another.  Sheesh.
 * 
 * 4     2/23/99 2:29p Dave
 * First run of oldschool dogfight mode. 
 * 
 * 3     1/29/99 2:08a Dave
 * Fixed beam weapon collisions with players. Reduced size of scoring
 * struct for multiplayer. Disabled PXO.
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 32    9/15/98 11:44a Dave
 * Renamed builtin ships and wepaons appropriately in FRED. Put in scoring
 * scale factors. Fixed standalone filtering of MD missions to non-MD
 * hosts.
 * 
 * 31    5/15/98 9:52p Dave
 * Added new stats for freespace. Put in artwork for viewing stats on PXO.
 * 
 * 30    5/15/98 4:12p Allender
 * removed redbook code.  Put back in ingame join timer.  Major fixups for
 * stats in multiplayer.  Pass correct score, medals, etc when leaving
 * game.  Be sure clients display medals, badges, etc.
 * 
 * 29    5/06/98 3:15p Allender
 * fixed some ranking problems.  Made vasudan support ship available in
 * multiplayer mode.
 * 
 * 28    5/05/98 2:10p Dave
 * Verify campaign support for testing. More new tracker code.
 * 
 * 27    4/27/98 6:02p Dave
 * Modify how missile scoring works. Fixed a team select ui bug. Speed up
 * multi_lag system. Put in new main hall.
 * 
 * 26    4/21/98 11:55p Dave
 * Put in player deaths statskeeping. Use arrow keys in the ingame join
 * ship select screen. Don't quit the game if in the debriefing and server
 * leaves.
 * 
 * 25    4/21/98 4:44p Dave
 * Implement Vasudan ships in multiplayer. Added a debug function to bash
 * player rank. Fixed a few rtvoice buffer overrun problems. Fixed ui
 * problem in options screen. 
 * 
 * 24    4/13/98 4:19p Hoffoss
 * Added rank voice file support.
 * 
 * 23    3/09/98 5:54p Dave
 * Fixed stats to take asteroid hits into account. Polished up UI stuff in
 * team select. Finished up pilot info popup. Tracked down and fixed
 * double click bug.
 * 
 * 22    3/09/98 4:30p Allender
 * multiplayer secondary weapon changes.  red-alert and cargo-known-delay
 * sexpressions.  Add time cargo revealed to ship structure
 * 
 * 21    3/02/98 4:24p Allender
 * change how scoring works.  Call scoring level close when debriefing is
 * initialized.  Made a "backout" function when mission is not accepted.
 * 
 * 20    2/05/98 5:34p Hoffoss
 * Changed code to allow each rank promotion to have it's own specific
 * debriefing text.
 * 
 * 19    2/04/98 6:36p Dave
 * Changed how stats are handled for net players.
 * 
 * 18    2/02/98 4:59p Hoffoss
 * Added a promotion text field to rank.tbl and code to support it in
 * FreeSpace.
 * 
 * 17    1/27/98 5:01p Dave
 * Put in support for leaving multiplayer games in the pause state. Fixed
 * a popup bug which freed saved screens incorrectly. Reworked scoring
 * kill and assist evaluation.
 * 
 * 16    1/27/98 4:23p Allender
 * enhanced internal scoring mechanisms.
 * 
 * 15    12/31/97 2:52p Hoffoss
 * Added award window stuff to debriefing screen.
 * 
 * 14    12/10/97 4:45p Dave
 * Added in more detailed support for multiplayer packet lag/loss. Fixed
 * some multiplayer stuff. Added some controls to the standalone.
 * 
 * 13    11/18/97 5:30p Dave
 * Reorganized some stats code. Made sure that mission stats (single and
 * multi) are only merged with alltime stats at the proper times.
 * 
 * 12    11/06/97 4:39p Allender
 * a ton of medal work.  Removed an uneeded elemen in the scoring
 * structure.  Fix up medals screen to apprioriate display medals (after
 * mask was changed).  Fix Fred to only display medals which may actually
 * be granted.  Added image_filename to player struct for Jason Hoffoss
 * 
 * 11    11/06/97 9:54a Dave
 * Dependant checkin
 * 
 * 10    11/05/97 4:43p Allender
 * reworked medal/rank system to read all data from tables.  Made Fred
 * read medals.tbl.  Changed ai-warp to ai-warp-out which doesn't require
 * waypoint for activation
 * 
 * 9     11/04/97 11:21p Allender
 * added gramt-promotion and grant-medal sexpressions.  Changed MAX_MEDALS
 * to NUM_MEDALS and made it be the correct number.  Temporaritly move
 * medal names to missionparse.cpp so Fred could get at them
 * 
 * 8     9/25/97 4:57p Dave
 * Finished up work on assigning kills and assists in player (and
 * multiplayer) stats.
 * 
 * 7     9/24/97 5:03p Dave
 * Beginning changes on how kills/assists are evaluated in both single and
 * multiplayer
 * 
 * 6     9/18/97 10:14p Dave
 * Updated the scoring system struct. Added rank.tbl. Modified how medal
 * names are displayed.
 * 
 * 5     9/18/97 9:21a Dave
 * Added view medals state. Changed pilot scoring struct to reflect.
 * 
 * 4     7/25/97 4:32p Dave
 * Added bonehead stats management.
 * 
 * 3     7/24/97 2:59p Dave
 * modified the scoring_struct type, and its corresponding init function
 * 
 * 2     5/01/97 10:24a Hoffoss
 * Created new scoring structure, and added code to save/restore it
 * to/from pilot file.
 * 
 * 1     4/30/97 10:19a Hoffoss
 *
 * $NoKeywords: $
 */

#ifndef _SCORING_HEADER_FILE
#define _SCORING_HEADER_FILE

#include <time.h>
#include "globalincs/pstypes.h"
#include "globalincs/globals.h"

struct player;
struct ship;
struct object;

#define NUM_RANKS				10

#define RANK_ENSIGN				0
#define RANK_LT_JUNIOR			1
#define RANK_LT					2
#define RANK_LT_CMDR				3
#define RANK_CMDR					4
#define RANK_CAPTAIN				5
#define RANK_COMMODORE			6
#define RANK_REAR_ADMIRAL		7
#define RANK_VICE_ADMIRAL		8
#define RANK_ADMIRAL  			9

#define MAX_FREESPACE1_RANK	RANK_COMMODORE
#define MAX_FREESPACE2_RANK	RANK_ADMIRAL

/*
	The ins and outs of when/where stats are stored and retreived - BE SURE TO FOLLOW THESE GUIDELINES

   SINGLE PLAYER :
		scoring_level_init() is called from game_level_init(). This zeroes out mission specific stats
		scoring_level_close() is called from the debriefing screen when the player hits accept. This saves the mission
			stats to alltime stats, and updates any campaign stats
		NOTE : in single player mode, if the player is going back to replay an old mission, the stats shouldn't be 
			stored again

   MULTI PLAYER :
		scoring_level_init() is called in game_level_init() again.
		init_multiplayer_stats() is called on all computers in the game when moving _into_ the MISSION_SYNC state

		scoring_level_close() is called on all machines when the host selects accept. If the host is not on the standalone
			he sends a packet to all players indicating that they should save their data. If he _is_ on the standalone, he should 
			send only to the standalone, and then it rebroadcasts the packet ot everyone else.		
*/


typedef struct rank_stuff {
	char		name[NAME_LENGTH];		// name of this rank
	char		*promotion_text;			// text to display when promoted to this rank
	int		points;						// points needed to reach this rank
	char		bitmap[MAX_FILENAME_LEN];		// bitmap of this rank medal
	char		promotion_voice_base[MAX_FILENAME_LEN];
} rank_stuff;

#define STATS_FLAG_INVALID			(1<<0)
#define STATS_FLAG_CAMPAIGN		(1<<1)
#define STATS_FLAG_MULTIPLAYER	(1<<2)

typedef struct scoring_struct {
	int flags;

	// All-time total
	int score;								// all time score
	int rank;								// all time rank
	int medals[MAX_MEDALS];				// all time medal counts

	//ushort kills[MAX_SHIP_CLASSES];		// only valid kills (i.e. not on friendlies).
	int kills[MAX_SHIP_CLASSES];		//DTP for bumped max_ships
	int assists;							// alltime assists
	int kill_count;						// total alltime kills
	int kill_count_ok;					// total valid alltime kills (no friendlies)
	unsigned int p_shots_fired;		// primary weapon shots fired
	unsigned int s_shots_fired;		// secondary weapon shots fired

	unsigned int p_shots_hit;			// primary weapon shots hit
	unsigned int s_shots_hit;			// secondary weapon shots hit

	unsigned int p_bonehead_hits;		// alltime primary friendly hits
	unsigned int s_bonehead_hits;		// alltime secondary friendly hits
   int          bonehead_kills;		// alltime friendly kills

	unsigned int missions_flown;		// total # of missions flown
	unsigned int flight_time;			// total # of flight hours the player has
	_fs_time_t last_flown;					// last time the player has flown
	_fs_time_t last_backup;					// so we can easily call scoring_level_backout()

	// Mission total
	int m_medal_earned;					// which medal (if any) earned this mission
	int m_badge_earned;					// which badge was earned.  Calculated after mission is over
	int m_promotion_earned;				// was a promotion earned.  Calculated after mission is over

	int m_score;
	//ushort m_kills[MAX_SHIP_CLASSES];     // this will represent all kills in the mission (bonehead or not)
	int m_kills[MAX_SHIP_CLASSES];		//DTP max
	//ushort m_okKills[MAX_SHIP_CLASSES];   // this will be only the "valid" kills the player made
	int m_okKills[MAX_SHIP_CLASSES];			//DTP max
	int m_kill_count;						// total kills for this mission
	int m_kill_count_ok;             // total (non-friendly) kills for this mission
	int m_assists;							// player assits for the mission
   unsigned int mp_shots_fired;		// primary shots fired for the mission
	unsigned int ms_shots_fired;		// secondary shots fired for the mission
	unsigned int mp_shots_hit;			// primary shots hit for the mission
	unsigned int ms_shots_hit;			// secondary shots hit for the mission
	unsigned int mp_bonehead_hits;	// primary friendly hits for the mission
	unsigned int ms_bonehead_hits;	// secondary friendly hits for the mission
	int m_bonehead_kills;				// # of friendly kills for the mission
	int m_player_deaths;					// player deaths for the mission (really only useful for multiplayer)

	// kills by player for multiplayer dogfight
	//ushort m_dogfight_kills[MAX_PLAYERS];
	int m_dogfight_kills[MAX_PLAYERS];//DTP max

} scoring_struct;

extern rank_stuff Ranks[NUM_RANKS];

void init_scoring_element(scoring_struct *s);

void parse_rank_tbl();
void scoring_level_init( scoring_struct *score );
void scoring_level_close(int accepted = 1);
void scoring_backout_accept( scoring_struct *score );
void scoring_do_accept( scoring_struct *score );

// function to give a medal to a player if he earned it
void scoring_check_medal(scoring_struct *sc);

void scoring_add_damage(object *ship_obj,object *other_obj,float damage);
void scoring_eval_kill(object *ship_obj);
void scoring_eval_assists(ship *sp,int killer_sig);

// bash the passed player to the specified rank
void scoring_bash_rank(player *pl,int rank);

// eval a hit on an object (for primary and secondary hit purposes)
void scoring_eval_hit(object *hit_obj, object *other_obj, int from_blast = 0);

// get a scaling factor for adding/subtracting from mission score
float scoring_get_scale_factor();

#endif
