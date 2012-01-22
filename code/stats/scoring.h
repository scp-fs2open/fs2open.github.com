/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
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
void scoring_add_damage_to_weapon(object *weapon_obj,object *other_obj,float damage);
int scoring_eval_kill(object *ship_obj);
int scoring_eval_kill_on_weapon(object *weapon_obj, object *other_obj);
void scoring_eval_assists(ship *sp,int killer_sig, bool enemy_player = false);

// bash the passed player to the specified rank
void scoring_bash_rank(player *pl,int rank);

// eval a hit on an object (for primary and secondary hit purposes)
void scoring_eval_hit(object *hit_obj, object *other_obj, int from_blast = 0);

// get a scaling factor for adding/subtracting from mission score
float scoring_get_scale_factor();

#endif
