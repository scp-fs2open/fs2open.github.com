/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
*/



#ifndef _SYSTEMVARS_H
#define _SYSTEMVARS_H

#include "globalincs/pstypes.h"
#include "math.h"

#define	GM_MULTIPLAYER					(1 << 0)
#define	GM_NORMAL						(1 << 1)
#define	GM_DEAD_DIED					(1 << 2)				//	Died, waiting to blow up.
#define	GM_DEAD_BLEW_UP				(1 << 3)				//	Blew up.
#define	GM_DEAD_ABORTED				(1 << 4)				//	Player pressed a key, aborting death sequence.
#define	GM_IN_MISSION					(1 << 5)				// Player is actually in the mission -- not at a pre-mission menu

#define	GM_DEAD							(GM_DEAD_DIED | GM_DEAD_BLEW_UP | GM_DEAD_ABORTED)

#define  GM_STANDALONE_SERVER			(1 << 8)
#define	GM_STATS_TRANSFER				(1 << 9)				// in the process of stats transfer
#define	GM_CAMPAIGN_MODE				(1 << 10)			// are we currently in a campaign.

#define	VM_EXTERNAL						(1 << 0)				//	Set if not viewing from player position.
#define	VM_TRACK						(1 << 1)				//	Set if viewer is tracking target.
#define	VM_DEAD_VIEW					(1 << 2)				//	Set if viewer is watching from dead view.
#define	VM_CHASE							(1 << 3)				//	Chase view.
#define	VM_OTHER_SHIP					(1 << 4)				//	View from another ship.
#define	VM_EXTERNAL_CAMERA_LOCKED	(1 << 5)				// External camera is locked in place (ie controls move ship not camera)
#define	VM_WARP_CHASE					(1	<< 6)				// View while warping out (form normal view mode)
#define	VM_PADLOCK_UP					(1 << 7)
#define	VM_PADLOCK_REAR				(1 << 8)
#define	VM_PADLOCK_LEFT				(1 << 9)
#define	VM_PADLOCK_RIGHT				(1 << 10)
#define	VM_WARPIN_ANCHOR				(1 << 11)			// special warpin camera mode
#define VM_TOPDOWN					(1 << 12)				//Camera is looking down on ship
#define VM_FREECAMERA				(1 << 13)				//Camera is not attached to any particular object, probably under SEXP control

#define	VM_PADLOCK_ANY (VM_PADLOCK_UP|VM_PADLOCK_REAR|VM_PADLOCK_LEFT|VM_PADLOCK_RIGHT)

//-----Cutscene stuff
//No bars
#define CUB_NONE			0
//List of types of bars
#define CUB_CUTSCENE		(1<<0)
//Styles to get to bars
#define CUB_GRADUAL			(1<<15)
extern float Cutscene_bars_progress, Cutscene_delta_time;
extern int Cutscene_bar_flags;

//-----Fadein stuff
struct shader;
extern shader Viewer_shader;

enum FadeType {
	FI_NONE,
	FI_FADEIN,
	FI_FADEOUT
};
extern FadeType Fade_type;
extern int Fade_start_timestamp;
extern int Fade_end_timestamp;


typedef struct vei {
	angles_t	angles;			//	Angles defining viewer location.
	float		distance;		//	Distance from which to view, plus 2x radius.
} vei;

typedef struct vci {
	angles_t	angles;
	float		distance;		// Distance from which to view, plus 3x radius
} vci;

extern fix Missiontime;
extern fix Skybox_timestamp;
extern fix Frametime;
extern int Framecount;

extern int Game_mode;

#define SINGLEPLAYER !(Game_mode & GM_MULTIPLAYER)

extern int Viewer_mode;
extern int Rand_count;

extern int Game_restoring;		// If set, this means we are restoring data from disk

// The detail level.  Anything below zero draws simple models earlier than it
// should.   Anything above zero draws higher detail models longer than it should.
// -2=lowest
// -1=low
// 0=normal (medium)
// 1=high
// 2=extra high
extern int Game_detail_level;

#define DETAIL_DEFAULT (0xFFFFFFFF)

#define DETAIL_FLAG_STARS			(1<<0)	// draw the stars
#define DETAIL_FLAG_NEBULAS		(1<<1)	// draw the motion debris
#define DETAIL_FLAG_MOTION			(1<<2)	// draw the motion debris
#define DETAIL_FLAG_PLANETS		(1<<3)	// draw planets
#define DETAIL_FLAG_MODELS			(1<<4)	// draw models not as blobs
#define DETAIL_FLAG_LASERS			(1<<5)	// draw lasers not as pixels
#define DETAIL_FLAG_CLEAR			(1<<6)	// clear screen background after each frame
#define DETAIL_FLAG_HUD				(1<<7)	// draw hud stuff
#define DETAIL_FLAG_FIREBALLS		(1<<8)	// draw fireballs
#define DETAIL_FLAG_COLLISION		(1<<9)	// use good collision detection


extern uint Game_detail_flags;

extern angles	Viewer_slew_angles;
extern vei		Viewer_external_info;
extern vci		Viewer_chase_info;
extern vec3d leaning_position;

extern int Is_standalone;
extern int Interface_framerate;				// show interface framerate during flips
extern int Interface_last_tick;				// last timer tick on flip

#define NOISE_NUM_FRAMES 15

// Noise numbers go from 0 to 1.0
extern float Noise[NOISE_NUM_FRAMES];


// override states to skip rendering of certain elements, but without disabling them completely
extern bool Basemap_override;
extern bool Envmap_override;
extern bool Specmap_override;
extern bool Normalmap_override;
extern bool Heightmap_override;
extern bool Glowpoint_override;
extern bool Glowpoint_use_depth_buffer;
extern bool GLSL_override;
extern bool PostProcessing_override;
extern bool Teamcolor_override;
extern bool Shadow_override;

// game skill levels
#define	NUM_SKILL_LEVELS	5

//====================================================================================
// DETAIL LEVEL STUFF
// If you change any of this, be sure to increment the player file version
// in FreeSpace\ManagePilot.cpp and change Detail_defaults in SystemVars.cpp
// or bad things will happen, I promise.
//====================================================================================

#define MAX_DETAIL_LEVEL 4			// The highest valid value for the "analog" detail level settings

// If you change this, update player file in ManagePilot.cpp
typedef struct detail_levels {

	int		setting;						// Which default setting this was created from.   0=lowest... NUM_DEFAULT_DETAIL_LEVELS-1, -1=Custom

	// "Analogs"
	int		nebula_detail;				// 0=lowest detail, MAX_DETAIL_LEVEL=highest detail
	int		detail_distance;			// 0=lowest MAX_DETAIL_LEVEL=highest
	int		hardware_textures;		// 0=max culling, MAX_DETAIL_LEVEL=no culling
	int		num_small_debris;			// 0=min number, MAX_DETAIL_LEVEL=max number
	int		num_particles;				// 0=min number, MAX_DETAIL_LEVEL=max number
	int		num_stars;					// 0=min number, MAX_DETAIL_LEVEL=max number
	int		shield_effects;			// 0=min, MAX_DETAIL_LEVEL=max
	int		lighting;					// 0=min, MAX_DETAIL_LEVEL=max

	// Booleans
	int		targetview_model;			// 0=off, 1=on
	int		planets_suns;				// 0=off, 1=on
	int		weapon_extras;				// extra weapon details. trails, glows
} detail_levels;

// Global values used to access detail levels in game and libs
extern detail_levels Detail;

#define NUM_DEFAULT_DETAIL_LEVELS	4	// How many "predefined" detail levels there are

// Call this with:
// 0 - lowest
// NUM_DEFAULT_DETAIL_LEVELS - highest
// To set the parameters in Detail to some set of defaults
void detail_level_set(int level);

// Returns the current detail level or -1 if custom.
int current_detail_level();

//=========================================================
// Functions to profile frame performance

typedef struct profile_sample {
	uint profile_instances;
	int open_profiles;
	//char name[256];
	SCP_string name;
	uint start_time;	// in microseconds
	uint accumulator;
	uint children_sample_time;
	uint num_parents;
	uint num_children;
	int parent;
} profile_sample;

typedef struct profile_sample_history {
	bool valid;
	//char name[256];
	SCP_string name;
	float avg;
	float min;
	float max;
	uint avg_micro_sec;
	uint min_micro_sec;
	uint max_micro_sec;
} profile_sample_history;

extern SCP_string profile_output;

void profile_init();
void profile_deinit();
void profile_begin(const char* name);
void profile_begin(SCP_string &output_handle, const char* name);
void profile_end(const char* name);
void profile_dump_output();
void store_profile_in_history(SCP_string &name, float percent, uint time);
void get_profile_from_history(SCP_string &name, float* avg, float* min, float* max, uint *avg_micro_sec, uint *min_micro_sec, uint *max_micro_sec);

class profile_auto
{
	SCP_string name;
public:
	profile_auto(const char* profile_name): name(profile_name)
	{
		profile_begin(profile_name);
	}

	~profile_auto()
	{
		profile_end(name.c_str());
	}
};

// Helper macro to encapsulate a single function call in a profile_begin()/profile_end() pair.
#define PROFILE(name, function) { profile_begin(name); function; profile_end(name); }

//====================================================================================
// Memory stuff from WinDebug.cpp
extern int TotalRam;
void windebug_memwatch_init();

#define MAX_LIGHTS 256
#define MAX_LIGHT_LEVELS 16

#define safe_kill(a) if(a)vm_free(a)


// Goober5000
void insertion_sort(void *array, size_t array_size, size_t element_size, int (*fncompare)(const void *, const void *));


#endif
