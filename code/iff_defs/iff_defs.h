/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */



#ifndef _IFF_DEFS_H_
#define _IFF_DEFS_H_

#include "globalincs/pstypes.h"
#include "globalincs/globals.h"
#include "graphics/2d.h"

class object;

// Goober5000 - new IFF color system
#define IFF_COLOR_SELECTION			0
#define IFF_COLOR_MESSAGE			1
#define IFF_COLOR_TAGGED			2
#define MAX_IFF_COLORS				(MAX_IFFS + 3)

// iff flags
#define IFFF_SUPPORT_ALLOWED				(1 << 0)	// this IFF can call for support
#define IFFF_EXEMPT_FROM_ALL_TEAMS_AT_WAR	(1 << 1)	// self-explanatory
#define IFFF_ORDERS_HIDDEN					(1 << 2)	// the HUD will not show a targeted ship's orders
#define IFFF_ORDERS_SHOWN					(1 << 3)	// the HUD will show a targeted ship's orders (friendly has by default)
#define IFFF_WING_NAME_HIDDEN				(1 << 4)	// the HUD will not show a targeted ship's name if it is in a wing
#define MAX_IFF_FLAGS						5

// Goober5000
typedef struct iff_info {

	// required stuff
	char iff_name[NAME_LENGTH];
	int color_index;							// treat this as private and use iff_get_color or iff_get_color_by_team

	// relationships
	int attackee_bitmask;						// treat this as private and use iff_get_attackee_mask or iff_x_attacks_y
	int attackee_bitmask_all_teams_at_war;		// treat this as private and use iff_get_attackee_mask or iff_x_attacks_y
	int observed_color_index[MAX_IFFS];			// treat this as private and use iff_get_color or iff_get_color_by_team

	// flags
	int flags;
	int default_parse_flags;
	int default_parse_flags2;

	// used internally, not parsed
	int ai_rearm_timestamp;

} iff_info;

extern int Num_iffs;
extern iff_info Iff_info[MAX_IFFS];

extern int Iff_traitor;

// radar blip stuff
extern int radar_iff_color[5][2][4];

// color stuff
extern int iff_get_alpha_value(bool is_bright);
extern int iff_init_color(int r, int g, int b);

// load the iff table
extern void iff_init();

// search for iff
extern int iff_lookup(char *iff_name);

// attack stuff
// NB: As far as the differences between I attack him and he attacks me, think of a hidden traitor on your own team.
// If he fires at you, you don't react unless you are coded to attack him, because you are oblivious.
extern int iff_get_attackee_mask(int attacker_team);
extern int iff_get_attacker_mask(int attackee_team);
extern int iff_x_attacks_y(int team_x, int team_y);

// mask stuff
extern int iff_get_mask(int team);
extern int iff_matches_mask(int team, int mask);

// get color stuff
extern color *iff_get_color(int color_index, int is_bright);
extern color *iff_get_color_by_team(int team, int seen_from_team, int is_bright);
extern color *iff_get_color_by_team_and_object(int team, int seen_from_team, int is_bright, object *objp);

#endif
