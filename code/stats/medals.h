/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef FREESPACE_MEDAL_HEADER_FILE
#define FREESPACE_MEDAL_HEADER_FILE

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"

class scoring_struct;
class player;

#define MAX_BADGES				3
extern int Rank_medal_index;

extern scoring_struct *Player_score;

// NUM_MEDALS stored in scoring.h since needed for player scoring structure

typedef struct medal_stuff {
	char	name[NAME_LENGTH];
	char	bitmap[MAX_FILENAME_LEN];
	char	debrief_bitmap[MAX_FILENAME_LEN];
	int	num_versions;
	bool version_starts_at_1;
	int	kills_needed;

	//If this is a badge (kills_needed > 0)
	char voice_base[MAX_FILENAME_LEN];
	char *promotion_text;

	medal_stuff() {
		name[0] = '\0';
		bitmap[0] = '\0';
		debrief_bitmap[0] = '\0';
		num_versions = 1;
		version_starts_at_1 = false;
		kills_needed = 0;
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

extern SCP_vector<medal_stuff> Medals;

extern void parse_medal_tbl();

// modes for this screen
#define MM_NORMAL				0		// normal - run through the state code
#define MM_POPUP				1		// called from within some other tight loop (don't use gameseq_ functions)

// main medals screen
void medal_main_init(player *pl, int mode = MM_NORMAL);

// return 0 if the screen should close (used for MM_POPUP mode)
int medal_main_do();
void medal_main_close();

int medals_info_lookup(const char *name);

#endif
