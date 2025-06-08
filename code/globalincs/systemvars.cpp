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

static_assert(MAX_DETAIL_VALUE == 4, "MAX_DETAIL_VALUE is assumed to be 4 in SystemVars.cpp");

static_assert(static_cast<int>(DefaultDetailPreset::Num_detail_presets) == 4, "Code in ManagePilot assumes Num_detail_presets = 4");

// Detail preset stuff
detail_levels Detail_defaults[static_cast<int>(DefaultDetailPreset::Num_detail_presets)] = {
	{
		DefaultDetailPreset::Low,              // setting
                    // ================== Analogs ==================
		0,          // nebula_detail;          // 0=lowest detail, MAX_DETAIL_VALUE=highest detail
		0,          // detail_distance;        // 0=lowest MAX_DETAIL_VALUE=highest			
		0,          // hardware_textures;      // 0=max culling, MAX_DETAIL_VALUE=no culling
		0,          // num_small_debris;       // 0=min number, MAX_DETAIL_VALUE=max number
		0,          // num_particles;          // 0=min number, MAX_DETAIL_VALUE=max number
		0,          // num_stars;              // 0=min number, MAX_DETAIL_VALUE=max number
		0,          // shield_effects;         // 0=min, MAX_DETAIL_VALUE=max
		2,          // lighting;               // 0=min, MAX_DETAIL_VALUE=max		

                    // ================== Booleans ==================
		false,       // targetview_model;       // false=off, true=on		
		false,       // planets_suns;           // false=off, true=on		
		false,       // weapon_extras           // false=off, true=on
	},
	{
		DefaultDetailPreset::Medium,           // setting
                    // ================== Analogs ==================
		2,          // nebula_detail;          // 0=lowest detail, MAX_DETAIL_VALUE=highest detail
		2,          // detail_distance;        // 0=lowest MAX_DETAIL_VALUE=highest				
		2,          // hardware_textures;      // 0=max culling, MAX_DETAIL_VALUE=no culling
		2,          // num_small_debris;       // 0=min number, MAX_DETAIL_VALUE=max number
		2,          // num_particles;          // 0=min number, MAX_DETAIL_VALUE=max number
		2,          // num_stars;              // 0=min number, MAX_DETAIL_VALUE=max number
		2,          // shield_effects;         // 0=min, MAX_DETAIL_VALUE=max
		3,          // lighting;               // 0=min, MAX_DETAIL_VALUE=max		

                    // ================== Booleans ==================
		true,       // targetview_model;       // false=off, true=on		
		true,       // planets_suns;           // false=off, true=on
		true,       // weapon_extras           // false=off, true=on		
	},
	{
		DefaultDetailPreset::High,             // setting
                    // ================== Analogs ==================
		2,          // nebula_detail;          // 0=lowest detail, MAX_DETAIL_VALUE=highest detail
		2,          // detail_distance;        // 0=lowest MAX_DETAIL_VALUE=highest			
		3,          // hardware_textures;      // 0=max culling, MAX_DETAIL_VALUE=no culling
		3,          // num_small_debris;       // 0=min number, MAX_DETAIL_VALUE=max number
		3,          // num_particles;          // 0=min number, MAX_DETAIL_VALUE=max number
		4,          // num_stars;              // 0=min number, MAX_DETAIL_VALUE=max number
		3,          // shield_effects;         // 0=min, MAX_DETAIL_VALUE=max
		4,          // lighting;               // 0=min, MAX_DETAIL_VALUE=max		

                    // ================== Booleans ==================
		true,       // targetview_model;       // false=off, true=on	
		true,       // planets_suns;           // false=off, true=on
		true,       // weapon_extras           // false=off, true=on
	},
	{
		DefaultDetailPreset::VeryHigh,         // setting
                    // ================== Analogs ==================
		4,          // nebula_detail;          // 0=lowest detail, MAX_DETAIL_VALUE=highest detail
		4,          // detail_distance;        // 0=lowest MAX_DETAIL_VALUE=highest		
		4,          // hardware_textures;      // 0=max culling, MAX_DETAIL_VALUE=no culling
		4,          // num_small_debris;       // 0=min number, MAX_DETAIL_VALUE=max number
		4,          // num_particles;          // 0=min number, MAX_DETAIL_VALUE=max number
		4,          // num_stars;              // 0=min number, MAX_DETAIL_VALUE=max number
		4,          // shield_effects;         // 0=min, MAX_DETAIL_VALUE=max
		4,          // lighting;               // 0=min, MAX_DETAIL_VALUE=max		

                    // ================== Booleans ==================
		true,       // targetview_model;       // false=off, true=on		
		true,       // planets_suns;           // false=off, true=on
		true,       // weapon_extras           // false=off, true=on
	},
};

// Global used to access detail presets in game and libs
detail_levels Detail = Detail_defaults[static_cast<int>(DefaultDetailPreset::Num_detail_presets) - 1];

const SCP_vector<std::pair<int, std::pair<const char*, int>>> DetailLevelValues = {{ 0, {"Minimum", 1680}},
                                                                                   { 1, {"Low", 1160}},
                                                                                   { 2, {"Medium", 1161}},
                                                                                   { 3, {"High", 1162}},
                                                                                   { 4, {"Ultra", 1721}}};

static void parse_model_detail_func()
{
	int value[static_cast<int>(DefaultDetailPreset::Num_detail_presets)];
	stuff_int_list(value, static_cast<int>(DefaultDetailPreset::Num_detail_presets), RAW_INTEGER_TYPE);

	for (int i = 0; i < static_cast<int>(DefaultDetailPreset::Num_detail_presets); i++) {

		if (value[i] < 0 || value[i] > MAX_DETAIL_VALUE) {
			error_display(0, "%i is an invalid detail level value!", value[i]);
		} else {
			change_default_detail_level(static_cast<DefaultDetailPreset>(i), DetailSetting::DetailDistance, value[i]);
		}
	}
}

const auto ModelDetailOption __UNUSED = options::OptionBuilder<int>("Graphics.Detail",
                     std::pair<const char*, int>{"Model Detail", 1739},
                     std::pair<const char*, int>{"Detail level of models", 1740})
                     .importance(8)
                     .category(std::make_pair("Graphics", 1825))
                     .values(DetailLevelValues)
                     .default_func([](){return Detail.detail_distance;})
                     .change_listener([](int val, bool) {
                          Detail.detail_distance = val;
                          return true;
                     })
                     .flags({options::OptionFlags::RetailBuiltinOption})
                     .parser(parse_model_detail_func)
                     .finish();

static void parse_texture_detail_func()
{
	int value[static_cast<int>(DefaultDetailPreset::Num_detail_presets)];
	stuff_int_list(value, static_cast<int>(DefaultDetailPreset::Num_detail_presets), RAW_INTEGER_TYPE);

	for (int i = 0; i < static_cast<int>(DefaultDetailPreset::Num_detail_presets); i++) {

		if (value[i] < 0 || value[i] > MAX_DETAIL_VALUE) {
			error_display(0, "%i is an invalid detail level value!", value[i]);
		} else {
			change_default_detail_level(static_cast<DefaultDetailPreset>(i), DetailSetting::HardwareTextures, value[i]);
		}
	}
}

const auto TexturesOption __UNUSED = options::OptionBuilder<int>("Graphics.Texture",
                     std::pair<const char*, int>{"3D Hardware Textures", 1362},
                     std::pair<const char*, int>{"Level of detail of textures", 1720})
                     .importance(6)
                     .category(std::make_pair("Graphics", 1825))
                     .values(DetailLevelValues)
                     .default_func([](){return Detail.hardware_textures;})
                     .change_listener([](int val, bool) {
                          Detail.hardware_textures = val;
                          return true;
                     })
                     .flags({options::OptionFlags::RetailBuiltinOption})
                     .parser(parse_texture_detail_func)
                     .finish();

static void parse_particles_detail_func()
{
	int value[static_cast<int>(DefaultDetailPreset::Num_detail_presets)];
	stuff_int_list(value, static_cast<int>(DefaultDetailPreset::Num_detail_presets), RAW_INTEGER_TYPE);

	for (int i = 0; i < static_cast<int>(DefaultDetailPreset::Num_detail_presets); i++) {

		if (value[i] < 0 || value[i] > MAX_DETAIL_VALUE) {
			error_display(0, "%i is an invalid detail level value!", value[i]);
		} else {
			change_default_detail_level(static_cast<DefaultDetailPreset>(i), DetailSetting::NumParticles, value[i]);
		}
	}
}

const auto ParticlesOption __UNUSED = options::OptionBuilder<int>("Graphics.Particles",
                     std::pair<const char*, int>{"Particles", 1363},
                     std::pair<const char*, int>{"Level of detail for particles", 1717})
                     .importance(5)
                     .category(std::make_pair("Graphics", 1825))
                     .values(DetailLevelValues)
                     .default_func([](){return Detail.num_particles;})
                     .change_listener([](int val, bool) {
                          Detail.num_particles = val;
                          return true;
                     })
                     .flags({options::OptionFlags::RetailBuiltinOption})
                     .parser(parse_particles_detail_func)
                     .finish();

static void parse_debris_detail_func()
{
	int value[static_cast<int>(DefaultDetailPreset::Num_detail_presets)];
	stuff_int_list(value, static_cast<int>(DefaultDetailPreset::Num_detail_presets), RAW_INTEGER_TYPE);

	for (int i = 0; i < static_cast<int>(DefaultDetailPreset::Num_detail_presets); i++) {

		if (value[i] < 0 || value[i] > MAX_DETAIL_VALUE) {
			error_display(0, "%i is an invalid detail level value!", value[i]);
		} else {
			change_default_detail_level(static_cast<DefaultDetailPreset>(i), DetailSetting::NumSmallDebris, value[i]);
		}
	}
}

const auto SmallDebrisOption __UNUSED = options::OptionBuilder<int>("Graphics.SmallDebris", 
                     std::pair<const char*, int>{"Impact Effects", 1364}, 
                     std::pair<const char*, int>{"Level of detail of impact effects", 1743})
                     .category(std::make_pair("Graphics", 1825))
                     .values(DetailLevelValues)
                     .default_func([](){return Detail.num_small_debris;})
                     .importance(4)
                     .change_listener([](int val,bool) {
                          Detail.num_small_debris = val;
                          return true;
                     })
                     .flags({options::OptionFlags::RetailBuiltinOption})
                     .parser(parse_debris_detail_func)
                     .finish();

static void parse_shield_detail_func()
{
	int value[static_cast<int>(DefaultDetailPreset::Num_detail_presets)];
	stuff_int_list(value, static_cast<int>(DefaultDetailPreset::Num_detail_presets), RAW_INTEGER_TYPE);

	for (int i = 0; i < static_cast<int>(DefaultDetailPreset::Num_detail_presets); i++) {

		if (value[i] < 0 || value[i] > MAX_DETAIL_VALUE) {
			error_display(0, "%i is an invalid detail level value!", value[i]);
		} else {
			change_default_detail_level(static_cast<DefaultDetailPreset>(i), DetailSetting::ShieldEffects, value[i]);
		}
	}
}

const auto ShieldEffectsOption __UNUSED = options::OptionBuilder<int>("Graphics.ShieldEffects",
                     std::pair<const char*, int>{"Shield Hit Effects", 1718},
                     std::pair<const char*, int>{"Level of detail of shield impacts", 1719})
                     .importance(3)
                     .category(std::make_pair("Graphics", 1825))
                     .values(DetailLevelValues)
                     .default_func([](){return Detail.shield_effects;})
                     .change_listener([](int val, bool) {
                          Detail.shield_effects = val;
                          return true;
                     })
                     .flags({options::OptionFlags::RetailBuiltinOption})
                     .parser(parse_shield_detail_func)
                     .finish();

static void parse_stars_detail_func()
{
	int value[static_cast<int>(DefaultDetailPreset::Num_detail_presets)];
	stuff_int_list(value, static_cast<int>(DefaultDetailPreset::Num_detail_presets), RAW_INTEGER_TYPE);

	for (int i = 0; i < static_cast<int>(DefaultDetailPreset::Num_detail_presets); i++) {

		if (value[i] < 0 || value[i] > MAX_DETAIL_VALUE) {
			error_display(0, "%i is an invalid detail level value!", value[i]);
		} else {
			change_default_detail_level(static_cast<DefaultDetailPreset>(i), DetailSetting::NumStars, value[i]);
		}
	}
}

const auto StarsOption __UNUSED = options::OptionBuilder<int>("Graphics.Stars", 
                     std::pair<const char*, int>{"Stars", 1366}, 
                     std::pair<const char*, int>{"Number of stars in the mission", 1698})
                     .importance(2)
                     .category(std::make_pair("Graphics", 1825))
                     .values(DetailLevelValues)
                     .default_func([](){return Detail.num_stars;})
                     .change_listener([](int val, bool) {
                          Detail.num_stars = val;
                          return true;
                     })
                     .flags({options::OptionFlags::RetailBuiltinOption})
                     .parser(parse_stars_detail_func)
                     .finish();

// Call this with:
// 0 - lowest
// Num_detail_presets - highest
// To set the parameters in Detail to some set of defaults
void detail_level_set(DefaultDetailPreset preset)
{
	if (preset == DefaultDetailPreset::Custom) {
		Detail.setting = DefaultDetailPreset::Custom;
		return;
	}

	Detail = Detail_defaults[static_cast<int>(preset)];
}

// Change detail values 0 - MAX_DETAIL_VALUE
void change_default_detail_level(DefaultDetailPreset preset, DetailSetting selection, int value)
{

	// Use a switch statement for more readable access to the struct members
	switch (selection) {
	case DetailSetting::NebulaDetail:
		Detail_defaults[static_cast<int>(preset)].nebula_detail = value;
		break;
	case DetailSetting::DetailDistance:
		Detail_defaults[static_cast<int>(preset)].detail_distance = value;
		break;
	case DetailSetting::HardwareTextures:
		Detail_defaults[static_cast<int>(preset)].hardware_textures = value;
		break;
	case DetailSetting::NumSmallDebris:
		Detail_defaults[static_cast<int>(preset)].num_small_debris = value;
		break;
	case DetailSetting::NumParticles:
		Detail_defaults[static_cast<int>(preset)].num_particles = value;
		break;
	case DetailSetting::NumStars:
		Detail_defaults[static_cast<int>(preset)].num_stars = value;
		break;
	case DetailSetting::ShieldEffects:
		Detail_defaults[static_cast<int>(preset)].shield_effects = value;
		break;
	case DetailSetting::Lighting:
		Detail_defaults[static_cast<int>(preset)].lighting = value;
		break;
	default:
		Assertion(false, "Invalid detail selection. Get a coder!");
	}
}

// Change detail values bool overload
void change_default_detail_level(DefaultDetailPreset preset, DetailSetting selection, bool value)
{

	// Use a switch statement for more readable access to the struct members
	switch (selection) {
	case DetailSetting::TargetViewModel:
		Detail_defaults[static_cast<int>(preset)].targetview_model = value;
		break;
	case DetailSetting::PlanetsSuns:
		Detail_defaults[static_cast<int>(preset)].planets_suns = value;
		break;
	case DetailSetting::WeaponExtras:
		Detail_defaults[static_cast<int>(preset)].weapon_extras = value;
		break;
	default:
		Assertion(false, "Invalid detail selection. Get a coder!");
	}
}

// Returns the current detail preset or -1 if custom.
DefaultDetailPreset current_detail_preset()
{
//	return Detail.setting;
	int i;

	for (i = 0; i < static_cast<int>(DefaultDetailPreset::Num_detail_presets); i++) {
		if ( memcmp( &Detail, &Detail_defaults[i], sizeof(detail_levels) )==0 )	{
			return static_cast<DefaultDetailPreset>(i);
		}
	}
	return DefaultDetailPreset::Custom;
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
