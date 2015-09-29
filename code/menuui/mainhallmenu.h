/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef _MAIN_HALL_MENU_HEADER_FILE
#define _MAIN_HALL_MENU_HEADER_FILE

#include "globalincs/globals.h"
#include "globalincs/pstypes.h"

// CommanderDJ - this is now dynamic
// #define MAIN_HALLS_MAX			10

class main_hall_region
{
public:
	int mask;
	char key;
	SCP_string description;
	int action;
	SCP_string lua_action;

	main_hall_region(int _mask, char _key, const SCP_string &_description, int _action, const SCP_string &_lua_action)
		: mask(_mask), key(_key), description(_description), action(_action), lua_action(_lua_action)
	{}

	main_hall_region()
		: mask(0), key(0), description(), action(0), lua_action()
	{}
};

class main_hall_defines
{
public:
	// mainhall name identifier
	SCP_string name;

	SCP_vector<SCP_string> cheat;
	SCP_vector<SCP_string> cheat_anim_from;
	SCP_vector<SCP_string> cheat_anim_to;

	// minimum resolution and aspect ratio needed to display this main hall
	int min_width;
	int min_height;
	float min_aspect_ratio;

	// bitmap and mask
	SCP_string bitmap;
	SCP_string mask;

	// music
	SCP_string music_name;
	SCP_string substitute_music_name;

	// help overlay
	SCP_string help_overlay_name;
	int help_overlay_resolution_index;

	// zoom area
	int zoom_area_width;
	int zoom_area_height;

	// intercom defines -------------------

	// # of intercom sounds
	int num_random_intercom_sounds;

	// random (min/max) delays between playing intercom sounds
	SCP_vector<SCP_vector<int> > intercom_delay;

	// intercom sounds themselves
	SCP_vector<int> intercom_sounds;

	// intercom sound pan values
	SCP_vector<float> intercom_sound_pan;


	// misc animations --------------------

	// # of misc animations
	int num_misc_animations;

	// filenames of the misc animations
	SCP_vector<SCP_string> misc_anim_name;

	// Time until we will next play a given misc animation, min delay, and max delay
	SCP_vector<SCP_vector<int> > misc_anim_delay;

	// Goober5000, used in preference to the flag in generic_anim
	SCP_vector<int> misc_anim_paused;

	// Goober5000, used when we want to play one of several anims
	SCP_vector<int> misc_anim_group;

	// coords of where to play the misc anim
	SCP_vector<SCP_vector<int> > misc_anim_coords;

	// misc anim play modes (see MISC_ANIM_MODE_* above)
	SCP_vector<int> misc_anim_modes;

	// panning values for each of the misc anims
	SCP_vector<float> misc_anim_sound_pan;

	//sounds for each of the misc anims
	SCP_vector<SCP_vector<int> > misc_anim_special_sounds;

	//frame number triggers for each of the misc anims
	SCP_vector<SCP_vector<int> > misc_anim_special_trigger;

	//flags for each of the misc anim sounds
	SCP_vector<SCP_vector<int> > misc_anim_sound_flag;

	// controls the render order
	SCP_vector<bool> misc_anim_over_doors;


	// door animations --------------------

	// # of door animations
	int num_door_animations;

	// filenames of the door animations
	SCP_vector<SCP_string> door_anim_name;

	// first pair : coords of where to play a given door anim
	// second pair : center of a given door anim in windowed mode
	SCP_vector<SCP_vector<int> > door_anim_coords;

	// sounds for each region (open/close)
	SCP_vector<SCP_vector<int> > door_sounds;

	// pan values for the door sounds
	SCP_vector<float> door_sound_pan;

	// region descriptions ----------------

	// font used for the tooltips, version number, etc.
	int font;

	// action
	SCP_vector<main_hall_region> regions;
	
	bool default_readyroom;

	// num pixels shader is above/below tooltip text
	int tooltip_padding;

	// y coord of where to draw tooltip text
	int region_yval;

};

extern SCP_vector< SCP_vector<main_hall_defines> > Main_hall_defines;


// initialize the main hall proper 
void main_hall_init(const SCP_string &main_hall_name);

// parse mainhall table files
void main_hall_table_init();

// read in mainhall tables
void parse_main_hall_table(const char* filename);

// do a frame for the main hall
void main_hall_do(float frametime);

// close the main hall proper
void main_hall_close();

// start the main hall music playing
void main_hall_start_music();

// stop the main hall music
void main_hall_stop_music(bool fade);

// get the music index
int main_hall_get_music_index(int main_hall_num);

main_hall_defines* main_hall_get_pointer(const SCP_string &name_to_find);

int main_hall_get_index(const SCP_string &name_to_find);

int main_hall_get_resolution_index(int main_hall_num);

void main_hall_get_name(SCP_string &name, unsigned int index);

int main_hall_get_overlay_id();
int main_hall_get_overlay_resolution_index();

// what main hall we're on
int main_hall_id();

// Vasudan?
int main_hall_is_vasudan();

// start the ambient sounds playing in the main hall
void main_hall_start_ambient();
void main_hall_stop_ambient();
void main_hall_reset_ambient_vol();

void main_hall_do_multi_ready();

// make the vasudan main hall funny
void main_hall_vasudan_funny();

void main_hall_pause();
void main_hall_unpause();

#endif
