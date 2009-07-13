/*
 * Species_Defs.h
 * Extended Species Support for FS2 Open
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */



#ifndef _SPECIES_DEFS_H_
#define _SPECIES_DEFS_H_

#include "globalincs/pstypes.h"
#include "globalincs/globals.h"
#include "graphics/generic.h"
#include "gamesnd/gamesnd.h"
#include "mission/missionbriefcommon.h"
#include "hud/hudparse.h"

// for bitmap thrusters
typedef struct thrust_pair_bitmap {
	generic_bitmap normal;
	generic_bitmap afterburn;

	thrust_pair_bitmap( )
	{
	}
} thrust_pair_bitmap;

// for animated thrusters
typedef struct thrust_pair {
	generic_anim normal;
	generic_anim afterburn;

	thrust_pair( )
	{
	}
} thrust_pair;

typedef struct thrust_info {
	thrust_pair flames;
	thrust_pair glow;

	thrust_info( )
	{
	}
} thrust_info;


// Currently the only species-specific feature not in species_info is ship debris.  This is because
// ship debris chunks are treated as asteroids and tied so tightly into the asteroid code that
// separating them makes the code much more complicated.

typedef struct species_info {

	char species_name[NAME_LENGTH];
	int default_iff;
	float awacs_multiplier;

	union {
		struct {
			int r, g, b;
		} rgb;
		int a1d[3];
	} fred_color;

	generic_bitmap debris_texture;
	generic_anim shield_anim;
	thrust_info thruster_info;

	// Bobboau's thruster stuff
	thrust_pair_bitmap thruster_secondary_glow_info;
	thrust_pair_bitmap thruster_tertiary_glow_info;

	// the members below this comment are not parsed in species_defs.tbl

	game_snd snd_flyby_fighter;
	game_snd snd_flyby_bomber;

	generic_anim icon_bitmaps[MAX_BRIEF_ICONS];
	hud_anim icon_highlight_anims[MAX_BRIEF_ICONS];
	hud_anim icon_fade_anims[MAX_BRIEF_ICONS];


	// constructor to initialize everything to 0
	species_info( )
		: default_iff( 0 ), awacs_multiplier( 0 )
	{
		species_name[ 0 ] = NULL;
		memset( fred_color.a1d, 0, sizeof( fred_color.a1d ) );
	}

} species_info;

extern SCP_vector<species_info> Species_info;


// load up the species_defs.tbl into the correct data areas
// IMPORTANT: If Num_species != 3, icons.tbl, asteroid.tbl, and sounds.tbl have to be modified to compensate!
void species_init();

#endif
