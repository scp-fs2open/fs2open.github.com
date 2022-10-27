/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */

#include "globalincs/systemvars.h"

#include "debugconsole/console.h"
#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "io/timer.h"
#include "nebula/neb.h"
#include "options/Option.h"

fix Missiontime;
fix Frametime;
int	Framecount=0;

int Game_mode;

int Game_restoring = 0;		// If set, this means we are restoring data from disk

int	Viewer_mode;		//	Viewer's mode, see VM_xxxx flags.

//CUTSCENE STUFF
//Cutscene flags
int Cutscene_bar_flags = CUB_NONE;
//Time for gradual change in seconds
float Cutscene_delta_time = 1.0f;
//How far along a change is (0 to 1)
float Cutscene_bars_progress = 1.0f;

//FADEIN STUFF
shader Viewer_shader;
FadeType Fade_type = FI_NONE;
int Fade_start_timestamp = 0;
int Fade_end_timestamp = 0;

// The detail level.  Anything below zero draws simple models earlier than it
// should.   Anything above zero draws higher detail models longer than it should.
// -2=lowest
// -1=low
// 0=normal (medium)	
// 1=high
// 2=extra high
int Game_detail_level = 0;
uint Game_detail_flags = DETAIL_DEFAULT;	// see systemvars.h for explanation

angles	Viewer_slew_angles;			//	Angles of viewer relative to forward.
vei		Viewer_external_info;		//	Viewer angles to ship in external view.
vci		Viewer_chase_info;			// View chase camera information
vec3d leaning_position;

int Is_standalone;

int Interface_last_tick = -1;			// last timer tick on flip

#ifndef NDEBUG
// for debugging, used to print the currently processing filename on the loading screen
char Processing_filename[MAX_PATH_LEN];
#endif

// override states to skip rendering of certain elements, but without disabling them completely
bool Envmap_override = false;
bool Glowpoint_override = false;
bool Glowpoint_use_depth_buffer = true;
bool PostProcessing_override = false;
bool Shadow_override = false;
bool Trail_render_override = false;

bool Basemap_color_override_set = false;
float Basemap_color_override[4] = {0.0f, 0.0f, 0.0f, 1.0f};

bool Glowmap_color_override_set = false;
float Glowmap_color_override[3] = {0.0f, 0.0f, 0.0f};

bool Specmap_color_override_set = false;
float Specmap_color_override[3] = {0.0f, 0.0f, 0.0f};

bool Gloss_override_set = false;
float Gloss_override = 0.0f;

// Values used for noise for thruster animations
float Noise[NOISE_NUM_FRAMES] = { 
	0.468225f,
	0.168765f,
	0.318945f,
	0.292866f,
	0.553357f,
	0.468225f,
	0.180456f,
	0.418465f,
	0.489958f,
	1.000000f,
	0.468225f,
	0.599820f,
	0.664718f,
	0.294215f,
	0.000000f
};

// Variables for the loading callback hooks
static int cf_timestamp = -1;
static void (*cf_callback)(int count) = NULL;
static int cf_in_callback = 0;	
static int cb_counter = 0;
static int cb_last_counter = 0;
static int cb_delta_step = -1;

// Call this with the name of a function.  That function will
// then get called around 10x per second.  The callback function
// gets passed a 'count' which is how many times game_busy has
// been called since the callback was set.   It gets called
// one last time with count=-1 when you turn off the callback
// by calling game_busy_callback(NULL).   Game_busy_callback
// returns the current count, so you can tell how many times
// game_busy got called.
int game_busy_callback( void (*callback)(int count), int delta_step )
{
	if ( !callback ) {

		// Call it once more to finalize things
		cf_in_callback++;
		(*cf_callback)(-1);
		cf_in_callback--;

		cf_timestamp = -1;
		cf_callback = NULL;
	} else {
		cb_counter = 0;
		cb_last_counter = 0;
		cb_delta_step = delta_step;
		cf_timestamp = timer_get_milliseconds()+(1000/10);
		cf_callback = callback;

		// Call it once
		cf_in_callback++;
		(*cf_callback)(0);		// pass 0 first time!
		cf_in_callback--;
	
	}

	return cb_counter;
}

// Call whenever loading to display cursor
void game_busy(const char *filename)
{
	if ( cf_in_callback != 0 ) return;	// don't call callback if we're already in it.
	if ( cf_timestamp < 0 ) return;
	if ( !cf_callback ) return;

	cb_counter++;

//	mprintf(( "CB_COUNTER=%d\n", cb_counter ));

#ifndef NDEBUG
	if (filename != NULL) 
		strcpy_s(Processing_filename, filename);
#endif

	int t1 = timer_get_milliseconds();

	if ( (t1 > cf_timestamp) || ((cb_counter > cb_last_counter+155) && (cb_delta_step > 0)) )	{
		cb_last_counter = cb_counter;
		cf_in_callback++;
		(*cf_callback)(cb_counter);
		cf_in_callback--;
		cf_timestamp = t1 + (1000/10);
	}
}

#if MAX_DETAIL_LEVEL != 4
#error MAX_DETAIL_LEVEL is assumed to be 4 in SystemVars.cpp
#endif

#if NUM_DEFAULT_DETAIL_LEVELS != 4
#error NUM_DEFAULT_DETAIL_LEVELS is assumed to be 4 in SystemVars.cpp
#endif

// Detail level stuff
detail_levels Detail_defaults[NUM_DEFAULT_DETAIL_LEVELS] = {
	{				// Low
		0,			// setting
					// ===== Analogs (0-MAX_DETAIL_LEVEL) ====
		0,			// nebula_detail;				// 0=lowest detail, MAX_DETAIL_LEVEL=highest detail
		0,			// detail_distance;			// 0=lowest MAX_DETAIL_LEVEL=highest		
		0,			//	hardware_textures;			// 0=max culling, MAX_DETAIL_LEVEL=no culling
		0,			//	num_small_debris;			// 0=min number, MAX_DETAIL_LEVEL=max number
		0,			//	num_particles;				// 0=min number, MAX_DETAIL_LEVEL=max number
		0,			//	num_stars;					// 0=min number, MAX_DETAIL_LEVEL=max number
		0,			//	shield_effects;			// 0=min, MAX_DETAIL_LEVEL=max
		2,			// lighting;					// 0=min, MAX_DETAIL_LEVEL=max		

					// ====  Booleans ====
		0,			//	targetview_model;			// 0=off, 1=on		
		0,			//	planets_suns;				// 0=off, 1=on		
		0,			// weapon_extras
	},
	{				// Medium
		1,			// setting
					// ===== Analogs (0-MAX_DETAIL_LEVEL) ====
		2,			// nebula_detail;				// 0=lowest detail, MAX_DETAIL_LEVEL=highest detail
		2,			// detail_distance;			// 0=lowest MAX_DETAIL_LEVEL=highest		
		2,			//	hardware_textures;			// 0=max culling, MAX_DETAIL_LEVEL=no culling
		2,			//	num_small_debris;			// 0=min number, MAX_DETAIL_LEVEL=max number
		2,			//	num_particles;				// 0=min number, MAX_DETAIL_LEVEL=max number
		2,			//	num_stars;					// 0=min number, MAX_DETAIL_LEVEL=max number
		2,			//	shield_effects;			// 0=min, MAX_DETAIL_LEVEL=max
		3,			// lighting;					// 0=min, MAX_DETAIL_LEVEL=max		

		// ====  Booleans ====
		1,			//	targetview_model;			// 0=off, 1=on		
		1,			//	planets_suns;				// 0=off, 1=on
		1,			// weapon extras				
	},
	{				// High level
		2,			// setting
					// ===== Analogs (0-MAX_DETAIL_LEVEL) ====
		2,			// nebula_detail;				// 0=lowest detail, MAX_DETAIL_LEVEL=highest detail
		2,			// detail_distance;			// 0=lowest MAX_DETAIL_LEVEL=highest		
		3,			//	hardware_textures;			// 0=max culling, MAX_DETAIL_LEVEL=no culling
		3,			//	num_small_debris;			// 0=min number, MAX_DETAIL_LEVEL=max number
		3,			//	num_particles;				// 0=min number, MAX_DETAIL_LEVEL=max number
		4,			//	num_stars;					// 0=min number, MAX_DETAIL_LEVEL=max number
		3,			//	shield_effects;			// 0=min, MAX_DETAIL_LEVEL=max
		4,			// lighting;					// 0=min, MAX_DETAIL_LEVEL=max		

										// ====  Booleans ====
		1,			//	targetview_model;			// 0=off, 1=on		
		1,			//	planets_suns;				// 0=off, 1=on
		1,			// weapon_extras
	},
	{				// Highest level
		3,			// setting
					// ===== Analogs (0-MAX_DETAIL_LEVEL) ====
		4,			// nebula_detail;				// 0=lowest detail, MAX_DETAIL_LEVEL=highest detail
		4,			// detail_distance;			// 0=lowest MAX_DETAIL_LEVEL=highest		
		4,			//	hardware_textures;			// 0=max culling, MAX_DETAIL_LEVEL=no culling
		4,			//	num_small_debris;			// 0=min number, MAX_DETAIL_LEVEL=max number
		4,			//	num_particles;				// 0=min number, MAX_DETAIL_LEVEL=max number
		4,			//	num_stars;					// 0=min number, MAX_DETAIL_LEVEL=max number
		4,			//	shield_effects;			// 0=min, MAX_DETAIL_LEVEL=max
		4,			// lighting;					// 0=min, MAX_DETAIL_LEVEL=max		

										// ====  Booleans ====
		1,			//	targetview_model;			// 0=off, 1=on		
		1,			//	planets_suns;				// 0=off, 1=on
		1,			// weapon_extras
	},
};

// Global used to access detail levels in game and libs
detail_levels Detail = Detail_defaults[NUM_DEFAULT_DETAIL_LEVELS - 1];

const SCP_vector<std::pair<int, SCP_string>> DetailLevelValues = {{ 0, "Minimum" },
                                                                  { 1, "Low" },
                                                                  { 2, "Medium" },
                                                                  { 3, "High" },
                                                                  { 4, "Ultra" }, };

const auto ModelDetailOption =
	options::OptionBuilder<int>("Graphics.Detail", "Model Detail", "Detail level of models").importance(8).category(
		"Graphics").values(DetailLevelValues).default_val(MAX_DETAIL_LEVEL).change_listener([](int val, bool) {
		Detail.detail_distance = val;
		return true;
	}).finish();

const auto TexturesOption = options::OptionBuilder<int>("Graphics.Texture",
                                                        "3D Hardware Textures",
                                                        "Level of detail of textures").importance(6).category("Graphics").values(
	DetailLevelValues).default_val(MAX_DETAIL_LEVEL).change_listener([](int val, bool) {
	Detail.hardware_textures = val;
	return true;
}).finish();

const auto ParticlesOption = options::OptionBuilder<int>("Graphics.Particles",
                                                         "Particles",
                                                         "Level of detail for particles").importance(5).category(
	"Graphics").values(DetailLevelValues).default_val(MAX_DETAIL_LEVEL).change_listener([](int val, bool) {
	Detail.num_particles = val;
	return true;
}).finish();

const auto SmallDebrisOption =
	options::OptionBuilder<int>("Graphics.SmallDebris", "Impact Effects", "Level of detail of impact effects").category(
		"Graphics").values(DetailLevelValues).default_val(MAX_DETAIL_LEVEL).importance(4).change_listener([](int val,
	                                                                                                         bool) {
		Detail.num_small_debris = val;
		return true;
	}).finish();

const auto ShieldEffectsOption = options::OptionBuilder<int>("Graphics.ShieldEffects",
                                                             "Shield Hit Effects",
                                                             "Level of detail of shield impacts").importance(3).category(
	"Graphics").values(DetailLevelValues).default_val(MAX_DETAIL_LEVEL).change_listener([](int val, bool) {
	Detail.shield_effects = val;
	return true;
}).finish();

const auto StarsOption =
	options::OptionBuilder<int>("Graphics.Stars", "Stars", "Number of stars in the mission").importance(2).category(
		"Graphics").values(DetailLevelValues).default_val(MAX_DETAIL_LEVEL).change_listener([](int val, bool) {
		Detail.num_stars = val;
		return true;
	}).finish();

// Call this with:
// 0 - lowest
// NUM_DETAIL_LEVELS - highest
// To set the parameters in Detail to some set of defaults
void detail_level_set(int level)
{
	if ( level < 0 )	{
		Detail.setting = -1;
		return;
	}
	Assert( level >= 0 );
	Assert( level < NUM_DEFAULT_DETAIL_LEVELS );

	Detail = Detail_defaults[level];
}

// Returns the current detail level or -1 if custom.
int current_detail_level()
{
//	return Detail.setting;
	int i;

	for (i=0; i<NUM_DEFAULT_DETAIL_LEVELS; i++ )	{
		if ( memcmp( &Detail, &Detail_defaults[i], sizeof(detail_levels) )==0 )	{
			return i;
		}
	}
	return -1;
}

#ifndef NDEBUG
DCF(detail_level,"Change the detail level")
{
	int value;

	if (dc_optional_string_either("help", "--help")) {
		dc_printf( "Usage: detail_level [n]\n");
		dc_printf("[n]  -- is detail level.\n");
			dc_printf("\t0 is 'normal' detail,\n");
			dc_printf("\tnegative values are lower, and\n");
			dc_printf("\tpositive values are higher.\n\n");
		
		dc_printf("No parameter resets it to default.\n");
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("Detail level set to %d\n", Game_detail_level);
		return;
	}

	if (dc_maybe_stuff_int(&value)) {
		Game_detail_level = value;
		dc_printf("Detail level set to %i\n", Game_detail_level);

	} else {
		Game_detail_level = 0;
		dc_printf("Detail level reset\n");
	}
}

DCF(detail, "Turns on/off parts of the game for speed testing" )
{
	int value;

	if (dc_optional_string_either("help", "--help")) {
		dc_printf( "Usage: detail [n]\n");
		dc_printf("[n] is detail bit to toggle:\n" );
		dc_printf( "   1: draw the stars\n" );
		dc_printf( "   2: draw the nebulas\n" );
		dc_printf( "   4: draw the motion debris\n" );
		dc_printf( "   8: draw planets\n" );
		dc_printf( "  16: draw models not as blobs\n" );
		dc_printf( "  32: draw lasers not as pixels\n" );
		dc_printf( "  64: clear screen background after each frame\n" );
		dc_printf( " 128: draw hud stuff\n" );
		dc_printf( " 256: draw fireballs\n" );
		dc_printf( " 512: do collision detection\n\n" );

		dc_printf("No argument will toggle between highest/lowest detail settings\n");
		return;
	}

	if (dc_optional_string_either("status", "--status") || dc_optional_string_either("?", "--?")) {
		dc_printf("Detail flags set to 0x%08x\n", Game_detail_flags);
		dc_printf( "   1: draw the stars: %s\n", ((Game_detail_flags & 1) ? "on" : "off"));
		dc_printf( "   2: draw the nebulas: %s\n", ((Game_detail_flags & 2)?"on" : "off"));
		dc_printf( "   4: draw the motion debris: %s\n", ((Game_detail_flags & 4) ? "on" : "off"));
		dc_printf( "   8: draw planets: %s\n", ((Game_detail_flags & 8) ? "on" : "off"));
		dc_printf( "  16: draw models not as blobs: %s\n", ((Game_detail_flags & 16) ? "on" : "off"));
		dc_printf( "  32: draw lasers not as pixels: %s\n", ((Game_detail_flags & 32) ? "on" : "off"));
		dc_printf( "  64: clear screen background after each frame: %s\n", ((Game_detail_flags & 64) ? "on" : "off"));
		dc_printf( " 128: draw hud stuff: %s\n", ((Game_detail_flags & 128) ? "on" : "off"));
		dc_printf( " 256: draw fireballs: %s\n", ((Game_detail_flags & 256) ? "on" : "off"));
		dc_printf( " 512: do collision detection: %s\n", ((Game_detail_flags & 512) ? "on" : "off"));
		return;
	}

	if (dc_maybe_stuff_int(&value)) {
		Game_detail_flags ^= value;
	
	} else {
		if (Game_detail_flags == DETAIL_DEFAULT) {
			Game_detail_flags = DETAIL_FLAG_CLEAR;
			dc_printf( "Detail flags set lowest (except has screen clear)\n" );
		} else {
			Game_detail_flags = DETAIL_DEFAULT;
			dc_printf( "Detail flags set highest\n" );
		}
	}
}
#endif

// Stuff that can't be included in vmallocator.h

std::locale SCP_default_locale("");

void SCP_tolower(char *str)
{
	for (; *str != '\0'; ++str)
		*str = SCP_tolower(*str);
}

void SCP_toupper(char *str)
{
	for (; *str != '\0'; ++str)
		*str = SCP_toupper(*str);
}
