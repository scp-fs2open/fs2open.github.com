/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include <errno.h>


#include "playerman/managepilot.h"
#include "freespace2/freespace.h"
#include "hud/hudsquadmsg.h"
#include "sound/sound.h"
#include "gamesnd/eventmusic.h"
#include "sound/audiostr.h"
#include "osapi/osregistry.h"
#include "graphics/font.h"
#include "menuui/playermenu.h"
#include "missionui/missionshipchoice.h"
#include "hud/hudconfig.h"
#include "popup/popup.h"
#include "missionui/redalert.h"
#include "menuui/techmenu.h"
#include "io/joy.h"
#include "io/mouse.h"
#include "cutscene/cutscenes.h"
#include "missionui/missionscreencommon.h"
#include "mission/missionbriefcommon.h"
#include "mission/missioncampaign.h"
#include "mission/missionload.h"
#include "playerman/player.h"
#include "ship/ship.h"
#include "weapon/weapon.h"
#include "cfile/cfile.h"
#include "network/multi.h"



// update this when altering data that is read/written to .PLR file
#define CURRENT_PLAYER_FILE_VERSION					242
#define CURRENT_MULTI_PLAYER_FILE_VERSION			142
#define FS2_DEMO_PLAYER_FILE_VERSION				135
#define LOWEST_COMPATIBLE_PLAYER_FILE_VERSION		140	// compatible with release - Goober5000
// it used to be "demo plr files should work in final", but I guess this was cleared out
// when version 140 came along - see below

// keep track of pilot file changes here 
// version 2	: Added squad logo filename
// version 3	: Changed size of scoring struct. use ushort instead of ints for storing alltime kills by ship type
// version 4/5 : Added squadron name field
// version 6	: changed max length on a multiplayer options field
// version 130	: changed size of hud config struct
// version 133 : misc changes. new hud gauge
// version 134 : added HUD contrast toggle key
// version 135 : added tips flag  (THIS IS THE DEMO VERSION - RETAIN COMPATIBILITY FROM HERE ON OUT)
// version 136 : added intelligence flags to tech room visibility data
// version 137 : 2 new HUD gauges. 
// version 138	: new multiplayer config
// version 139 : # medals increased - added compatibility with old plr file versions
// version 140 : ships table reordered. clear out old pilot files
// HERE ONWARD ARE SCP CHANGES - MAINTAIN COMPATIBILITY WITH RELEASE -- VERSION 140
// version 141 : player-persistent variables - Goober5000

// multi player file version changes - no savefiles, need to keep everything here
// version 142 : handle new ship/weapon tables properly - multi only - taylor

// single player file version changes
// version 242 : move anything that might be campaign specific to the savefile - single only - taylor

// search for PLAYER INIT for new pilot initialization stuff. I _think_ its in the right spot for now
//#define PLR_FILE_ID	'FPSF'	// unique signiture to identify a .PLR file (FreeSpace Player File)  // FPSF appears as FSPF in file.
#define PLR_FILE_ID	0x46505346 // "FPSF" - unique signiture to identify a .PLR file (FreeSpace Player File), appears as FSPF in file.

// Current content of a .PLR file
//

// Global variables
int Player_sel_mode;

// pilot pic image list stuff ( call pilot_load_pic_list() to make these valid )
char Pilot_images_arr[MAX_PILOT_IMAGES][MAX_FILENAME_LEN];
char *Pilot_image_names[MAX_PILOT_IMAGES];
int Num_pilot_images = 0;

// squad logo list stuff (call pilot_load_squad_pic_list() to make these valid )
char Pilot_squad_images_arr[MAX_PILOT_IMAGES][MAX_FILENAME_LEN];
char *Pilot_squad_image_names[MAX_PILOT_IMAGES];
int Num_pilot_squad_images = 0;

static uint Player_file_version;

// forward declarations
void read_detail_settings(CFILE *file, int Player_file_version);
void write_detail_settings(CFILE *file);
void read_stats_block(CFILE *file, int Player_file_version, scoring_struct *stats);
void write_stats_block(CFILE *file, scoring_struct *stats, int multi);
void read_multiplayer_options(player *p,CFILE *file);
void write_multiplayer_options(player *p,CFILE *file);

// internal function to delete a player file.  Called after a pilot is obsoleted, and when a pilot is deleted
// used in barracks and player_select
//returns 0 on failure, 1 on success
int delete_pilot_file( char *pilot_name, int single )
{
	int delreturn;
	char filename[MAX_FILENAME_LEN];
	char basename[MAX_FILENAME_LEN];

	// get the player file.
	_splitpath(pilot_name, NULL, NULL, basename, NULL);

	strcpy_s( filename, basename );

	if (Player_sel_mode == PLAYER_SELECT_MODE_SINGLE){
		strcat_s( filename, NOX(".pl2") ); // we only support the new format now - taylor
		delreturn = cf_delete(filename, CF_TYPE_SINGLE_PLAYERS);
	} else {
		strcat_s( filename, NOX(".plr") ); // multi pilots use modified old format
		delreturn = cf_delete(filename, CF_TYPE_MULTI_PLAYERS);
	}

	// we must try and delete the campaign save files for a pilot as well.
	if(delreturn) {
		mission_campaign_delete_all_savefiles( basename, !single );
		return 1;
	} else {
		return 0;
	}
}

// same as delete_pilot_file() but deletes the old .plr files only
// we don't delete campaign files here though, do it in missioncampaign.cpp instead
int delete_pilot_file_old( char *pilot_name, int single )
{
	int delreturn = 0;
	char filename[MAX_FILENAME_LEN];
	char basename[MAX_FILENAME_LEN];

	// get the player file.
	_splitpath(pilot_name, NULL, NULL, basename, NULL);

	strcpy_s( filename, basename );
	strcat_s( filename, NOX(".plr") );

	// this is for single players only
	if (Player_sel_mode == PLAYER_SELECT_MODE_SINGLE){
		delreturn = cf_delete(filename, CF_TYPE_SINGLE_PLAYERS);
	}

	if (delreturn)
		return 1;

	return 0;
}

// check if a pilot file is valid or not (i.e. is usable, not out of date, etc)
// used in barracks and player_select
int verify_pilot_file(char *filename, int single, int *rank)
{
	CFILE	*file;
	uint id, file_version;
	int type;
	char pname[MAX_FILENAME_LEN];

	strcpy_s(pname, filename);

	char *p = strchr( pname, '.' );
	if ( p ) *p = 0;

	/* strlen doesn't count the null but MAX_FILENAME_LEN has
	to include the null so its (32-1) = 31 < pilot_filename_length*/
	if (strlen(pname) > ((MAX_FILENAME_LEN - 1) - 1 - 4))
	{
		char popup_txt[] = 
			"Pilot name is too long\n\n"
			"Pilot file '%s' is %d characters long, which is more than %d characters.\n\n"
			"Please locate the file in %s/data/players/%s%s and shorten the name. "
			"This pilot will be excluded from the select list.";

		char popup_str[sizeof(popup_txt) + 64];
		snprintf(popup_str, sizeof(popup_str), popup_txt,
			pname, strlen(pname), (MAX_FILENAME_LEN - 1) - 1 - 4,
#if SCP_UNIX
			"~",
#else
			"<freespace dir>",
#endif
			(single)?"single":"multi",
#ifdef INF_BUILD
			"/inferno"
#else
			""
#endif
		);
		popup(PF_TITLE_BIG | PF_TITLE_RED | PF_USE_AFFIRMATIVE_ICON, 1, POPUP_OK, popup_str);
		return -1;
	}

	if (single)
		strcat_s(pname, ".pl2");
	else
		strcat_s(pname, ".plr");

	if (single){
		file = cfopen(pname, "rb", CFILE_NORMAL, CF_TYPE_SINGLE_PLAYERS);
	} else {
		file = cfopen(pname, "rb", CFILE_NORMAL, CF_TYPE_MULTI_PLAYERS);
	}

	// if we didn't fine the file try the old version
	if (!file) {
		if (single){
			strcpy_s(pname, filename);
			strcat_s(pname, ".plr");

			file = cfopen(pname, "rb", CFILE_NORMAL, CF_TYPE_SINGLE_PLAYERS);
		}

		if (!file)
			return -1;
	}

	id = cfread_uint(file);
	if (id != PLR_FILE_ID) {
		nprintf(("Warning", "Player file ('%s') has invalid signature\n", pname));
		cfclose(file);
		delete_pilot_file( pname, single );
		return -1;
	}


	// check for compatibility here
	file_version = cfread_uint(file);
/*	if (file_version < INITIAL_RELEASE_FILE_VERSION) { */
//	if (file_version != CURRENT_PLAYER_FILE_VERSION) {
	if (file_version < LOWEST_COMPATIBLE_PLAYER_FILE_VERSION) {
		nprintf(("Warning", "WARNING => Player file ('%s') is outdated and not compatible...\n", pname));

		// Goober5000 - warn them
		char warning_text[256];
		sprintf(warning_text, "ATTENTION: Detected out-of-date player file \"%s\".  If you press OK, this file will be deleted!  Back this file up now if you do not want to lose it.  Contact a coder to resolve this error.", pname);
		Error(LOCATION, warning_text);

		cfclose(file);
		delete_pilot_file( pname, single );
		return -1;
	} else if ( single && (file_version > CURRENT_PLAYER_FILE_VERSION) ) {
		nprintf(("Warning", "WARNING => Player file ('%s') is too new and not compatible...\n", pname));

		cfclose(file);
		return -1;
	} else if ( !single && (file_version > CURRENT_MULTI_PLAYER_FILE_VERSION) ) {
		nprintf(("Warning", "WARNING => Player file ('%s') is too new and not compatible...\n", pname));

		cfclose(file);
		return -1;
	}

	type = !cfread_ubyte(file);
	if (rank){
		*rank = 0;	
	}
		
	if (rank){
		*rank = cfread_int(file);
	} else {
		cfread_int(file);
	}

	cfclose(file);
	if (type != single){
		return -1;
	}

	return 0;
}

// check to see if a pilot file is going to be updated to the new pl2 format
int pilot_file_upgrade_check(char *callsign, int single)
{
	int rc;
	char pname[MAX_FILENAME_LEN];

	// not for multi
	if (!single)
		return 0;

	// we only look for old pilot files here so if it's not found then it's assumed to be upgraded already
	Assert(strlen(callsign) < MAX_FILENAME_LEN - 4);  // ensure we won't overrun the buffer
	strcpy_s( pname, callsign );
	strcat_s( pname, NOX(".plr") );

	// check if we've actually got an old file and make sure the user knows what's going to happen
	if (cf_exists(pname, CF_TYPE_SINGLE_PLAYERS)) {
		// give a popup warning about the conversion process before proceeding - taylor
		char confirm_string[300];
#ifndef _WIN32
		snprintf(confirm_string, 300, "This will cause the pilot '%s' to be converted to FSO format. The conversion process is not reversible and you will no longer be able to use this pilot in earlier versions of FreeSpace (including FS2 retail). Do you want to continue?", callsign);
#else
		// FIXME: really need to fix this crap system-wide (snprintf)
		// The Windows implementation if snprintf is not safe
		sprintf(confirm_string, "This will cause the pilot '%s' to be converted to FSO format. The conversion process is not reversible and you will no longer be able to use this pilot in earlier versions of FreeSpace (including FS2 retail). Do you want to continue?", callsign);
#endif

		rc = popup(PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON, 2, POPUP_NO, POPUP_YES, confirm_string, -1);

		if (rc != 1)
			return 1;
	}

	return 0;
}

void pilot_write_techroom_data(CFILE *file, int multi)
{
	int idx;		
	ubyte out;

	/******** MULTI ONLY ********/
	if ( !multi )
		return;

	// write the ship and weapon count
	cfwrite_int(Num_ship_classes, file);
	cfwrite_int(Num_weapon_types, file);
	cfwrite_int(Intel_info_size, file);

	// write all ship flags out
	for (idx=0; idx<Num_ship_classes; idx++) {
		out = (Ship_info[idx].flags & SIF_IN_TECH_DATABASE) ? (ubyte)1 : (ubyte)0;		
		cfwrite_ubyte(out, file);				
	}

	// write all weapon types out
	for (idx=0; idx<Num_weapon_types; idx++) {
		out = (Weapon_info[idx].wi_flags & WIF_IN_TECH_DATABASE) ? (ubyte)1 : (ubyte)0;
		cfwrite_ubyte(out, file);
	}	

	// write all intel entry flags out
	for (idx=0; idx<Intel_info_size; idx++) {
		out = (Intel_info[idx].flags & IIF_IN_TECH_DATABASE) ? (ubyte)1 : (ubyte)0;
		cfwrite_ubyte(out, file);
	}

}

void pilot_read_techroom_data(CFILE *file, int pfile_version)
{
	int idx;
	int ship_count, weapon_count, intel_count;
	ubyte in;

	if (pfile_version < 242) {
		// read in ship and weapon counts
		ship_count = cfread_int(file);
		weapon_count = cfread_int(file);
		Assert(ship_count <= MAX_SHIP_CLASSES);
		Assert(weapon_count <= MAX_WEAPON_TYPES);

		// maintain compatibility w/ demo version
		if (Player_file_version < 136) {
			// skip over all this data, because the lack of tech room in the demo
			// left this all hosed in the demo .plr files
			// this will all get initialized as if this fella was a new pilot
			for (idx=0; idx<ship_count+weapon_count; idx++) {
				in = cfread_ubyte(file);
			}

		} else {

			intel_count = cfread_int(file);
			Assert(intel_count <= MAX_INTEL_ENTRIES);

			// read all ships in
			for (idx=0; idx<ship_count; idx++) {
				in = cfread_ubyte(file);
				if (in) {
					Ship_info[idx].flags |= SIF_IN_TECH_DATABASE | SIF_IN_TECH_DATABASE_M;
				} else {
					Ship_info[idx].flags &= ~SIF_IN_TECH_DATABASE;
				}
			}

			// read all weapons in
			for (idx=0; idx<weapon_count; idx++) {
				in = cfread_ubyte(file);
				if (in) {
					Weapon_info[idx].wi_flags |= WIF_IN_TECH_DATABASE;
				} else {
					Weapon_info[idx].wi_flags &= ~WIF_IN_TECH_DATABASE;
				}	
			}
	
			// read all intel entries in
			for (idx=0; idx<intel_count; idx++) {
				in = cfread_ubyte(file);
				if (in) {
					Intel_info[idx].flags |= IIF_IN_TECH_DATABASE;
				} else {
					Intel_info[idx].flags &= ~IIF_IN_TECH_DATABASE;
				}
			}
		}
	}
}

// write out the player ship selection
void pilot_write_loadout(CFILE *file, int multi)
{
	int i, j;
	wss_unit *slot;	

	/******** MULTI ONLY ********/
	if ( !multi )
		return;

	cfwrite_string_len(Player_loadout.filename, file);
	cfwrite_string_len(Player_loadout.last_modified, file);

	// write ship and weapon counts
	cfwrite_int(Num_ship_classes, file);
	cfwrite_int(Num_weapon_types, file);

	// write ship pool
	for ( i = 0; i < Num_ship_classes; i++ ) {
		cfwrite_int(Player_loadout.ship_pool[i], file);
		cfwrite_string_len(Ship_info[i].name, file);
	}

	// write weapons pool
	for ( i = 0; i < Num_weapon_types; i++ ) {
		cfwrite_int(Player_loadout.weapon_pool[i], file);
		cfwrite_string_len(Weapon_info[i].name, file);
	}

	// write ship loadouts
	for ( i = 0; i < MAX_WSS_SLOTS; i++ ) {
		slot = &Player_loadout.unit_data[i];
		cfwrite_int(slot->ship_class, file);
		cfwrite_string_len(Ship_info[slot->ship_class].name, file);

		for ( j = 0; j < MAX_SHIP_WEAPONS; j++ ) {
			cfwrite_int(slot->wep[j], file);
			cfwrite_int(slot->wep_count[j], file);
			cfwrite_string_len(Weapon_info[slot->wep[j]].name, file);
		}
	}
}

// read in the ship selection for the pilot
void pilot_read_loadout(CFILE *file, int pfile_version)
{
	int i, j;
	wss_unit *slot;
	int ship_count, weapon_count;
	char sw_name[NAME_LENGTH];
	int pool_count = -1;

	if (pfile_version < 242) {
		memset(Player_loadout.filename, 0, MAX_FILENAME_LEN);
		memset(Player_loadout.last_modified, 0, DATE_TIME_LENGTH);

		cfread_string_len(Player_loadout.filename, MAX_FILENAME_LEN, file);
		cfread_string_len(Player_loadout.last_modified, DATE_TIME_LENGTH, file);	

		// read in ship and weapon counts
		ship_count = cfread_int(file);
		weapon_count = cfread_int(file);
		Assert(ship_count <= MAX_SHIP_CLASSES);
		Assert(weapon_count <= MAX_WEAPON_TYPES);

		// read in ship pool
		for ( i = 0; i < ship_count; i++ ) {
			if (pfile_version >= 142) {
				pool_count = cfread_int(file);
				cfread_string_len(sw_name, NAME_LENGTH, file);
				Player_loadout.ship_pool[ship_info_lookup(sw_name)] = pool_count;
			} else {
				Player_loadout.ship_pool[i] = cfread_int(file);
			}
		}

		// read in weapons pool
		for ( i = 0; i < weapon_count; i++ ) {
			if (pfile_version >= 142) {
				pool_count = cfread_int(file);
				cfread_string_len(sw_name, NAME_LENGTH, file);
				Player_loadout.weapon_pool[weapon_info_lookup(sw_name)] = pool_count;
			} else {
				Player_loadout.weapon_pool[i] = cfread_int(file);
			}

		}

		// read in loadout info
		for ( i = 0; i < MAX_WSS_SLOTS; i++ ) {
			slot = &Player_loadout.unit_data[i];
			slot->ship_class = cfread_int(file);

			if (pfile_version >= 142) {
				cfread_string_len(sw_name, NAME_LENGTH, file);
				slot->ship_class = ship_info_lookup(sw_name);
			}

			for ( j = 0; j < MAX_SHIP_WEAPONS; j++ ) {
				slot->wep[j] = cfread_int(file);
				slot->wep_count[j] = cfread_int(file);

				if (pfile_version >= 142) {
					cfread_string_len(sw_name, NAME_LENGTH, file);
					slot->wep[j] = weapon_info_lookup(sw_name);
				}
			}
		}
	}
}

// read_pilot_file()
//
// returns 0 - file read in correctly
//        -1 - .PLR file doesn't exist or file not compatible
//        >0 - errno from fopen error
// if single == 1, look for players in the single players directory, otherwise look in the 
// multiplayers directory
int read_pilot_file(char *callsign, int single, player *p)
{
	ubyte num_ctrls;
	ubyte is_multi = 0;
	char filename[MAX_FILENAME_LEN], ship_name[NAME_LENGTH];
	CFILE	*file;
	uint id;
	int idx;
	int i, key_value;
	int pfile_upgrade = 0;
	char *ext;

	// if we're a standalone server in multiplayer, just fill in some bogus values since we don't have a pilot file
	if ((Game_mode & GM_MULTIPLAYER) && (Game_mode & GM_STANDALONE_SERVER)) {
		Player->insignia_texture = -1;
		strcpy_s(Player->callsign, NOX("Standalone"));
		strcpy_s(Player->short_callsign, NOX("Standalone"));
		return 0;
	}

	if (!p) {
		Assert((Player_num >= 0) && (Player_num < MAX_PLAYERS));
		p = &Players[Player_num];
	}

	// multi doesn't have a campaign savefile so we have to still use the old format
	// for all multi pilots.  Do use the new 142 version though for table values.
	if (single) {
		ext = NOX(".pl2");
	} else {
		ext = NOX(".plr");
	}

	//sprintf(filename, "%-.8s.plr",Players[Player_num].callsign);
	Assert(strlen(callsign) < MAX_FILENAME_LEN - 4);  // ensure we won't overrun the buffer
	strcpy_s( filename, callsign );
	strcat_s( filename, ext );

	// see comments at the beginning of function
	if (single) {
		file = cfopen(filename, "rb", CFILE_NORMAL, CF_TYPE_SINGLE_PLAYERS);
	} else {
		file = cfopen(filename, "rb", CFILE_NORMAL, CF_TYPE_MULTI_PLAYERS);
	}

	// if we couldn't open the new filetype try the old one and upgrade
	if (!file) {
		if (single) {
			strcpy_s( filename, callsign );
			strcat_s( filename, NOX(".plr") );

			file = cfopen(filename, "rb", CFILE_NORMAL, CF_TYPE_SINGLE_PLAYERS);

			pfile_upgrade = 1;
		}

		if (!file)
			return errno;
	}

	id = cfread_uint(file);
	if (id != PLR_FILE_ID) {
		Warning(LOCATION, "Player file has invalid signature");
		cfclose(file);
		return -1;
	}

	// check for compatibility here
	Player_file_version = cfread_uint(file);
	cf_set_version( file, Player_file_version );

	if (Player_file_version < LOWEST_COMPATIBLE_PLAYER_FILE_VERSION) {
		nprintf(("Warning", "WARNING => Player file is outdated and not compatible...\n"));

		// Goober5000 - warn them
		char warning_text[256];
		sprintf(warning_text, "ATTENTION: Detected out-of-date player file \"%s\".  If you press OK, this file will be deleted!  Back this file up now if you do not want to lose it.  Contact a coder to resolve this error.", filename);
		Error(LOCATION, warning_text);

		cfclose(file);
		delete_pilot_file( filename, single );
		return -1;
	}

	// read in whether he's a multiplayer or not
	is_multi = cfread_ubyte(file);
	if (is_multi){
		p->flags |= PLAYER_FLAGS_IS_MULTI;
	} else {
		p->flags &= ~PLAYER_FLAGS_IS_MULTI;	// this takes care of unsetting any leftover bits from a (possibly) previously selected pilot
	}

	// read in rank.
	cfread_int(file);  

	// get player location
	// moved to campaign file in 242 - taylor
	if ( Player_file_version < 242 ) {
		p->main_hall = cfread_ubyte(file);
	}

	// tips?
	p->tips = cfread_int(file);

	// write out the image file name
	cfread_string_len(p->image_filename, MAX_FILENAME_LEN - 1, file);

	// write out the image file name
	p->insignia_texture = -1;
	cfread_string_len(p->squad_name, NAME_LENGTH, file);

	// squad image filename
	char fname[MAX_FILENAME_LEN];
	cfread_string_len(fname, MAX_FILENAME_LEN - 1, file);
	player_set_squad_bitmap(p, fname);

	// deal with campaign stuff.  The way we store the information in the file is to first store the
	// name of the current campaign that the player is playing.  Next we store the info regarding the campaigns
	// that the player is currently playing
	memset(p->current_campaign, 0, MAX_FILENAME_LEN);
	cfread_string_len(p->current_campaign, MAX_FILENAME_LEN, file);

	// read in the ship name for last ship flown by the player
	memset(ship_name, 0, NAME_LENGTH);
	cfread_string_len(ship_name, NAME_LENGTH, file);
	p->last_ship_flown_si_index = ship_info_lookup(ship_name);
	if ( p->last_ship_flown_si_index < 0 ) {
		nprintf(("Warning","WARNING => Ship class %s not located in Ship_info[] in player file\n",ship_name));
		p->last_ship_flown_si_index = ship_info_lookup(default_player_ship);
	}

	// set all the entries in the control config arrays to -1 (undefined)
	control_config_clear();

	// ---------------------------------------------
	// read in the keyboard/joystick mapping
	// ---------------------------------------------
	num_ctrls = cfread_ubyte(file);
	for (i=0; i<num_ctrls; i++) {
		key_value = cfread_short(file);
		// NOTE: next two lines are only here for transitioning from 255 to -1 as undefined key items
		if (key_value == 255)
			key_value = -1;

		Control_config[i].key_id = (short) key_value;

		key_value = cfread_short(file);
		// NOTE: next two lines are only here for transitioning from 255 to -1 as undefined key items
		if (key_value == 255)
			key_value = -1;

		Control_config[i].joy_id = (short) key_value;
	}
	
	HUD_config.show_flags = cfread_int(file);	
	HUD_config.show_flags2 = cfread_int(file);

	HUD_config.popup_flags = cfread_int(file);
	HUD_config.popup_flags2 = cfread_int(file);

	HUD_config.num_msg_window_lines = cfread_ubyte(file);			
	HUD_config.rp_flags = cfread_int(file);
	HUD_config.rp_dist =	cfread_int(file);
	// HUD_config.color = cfread_int( file );
	// HUD_color_alpha = cfread_int( file );
	// if ( HUD_color_alpha < HUD_COLOR_ALPHA_USER_MIN ) {
		// HUD_color_alpha = HUD_COLOR_ALPHA_DEFAULT;
	// }
	// hud_config_record_color(HUD_config.color);

	// added 2 gauges with version 137
	if(Player_file_version < 137){
		for(idx=0; idx<NUM_HUD_GAUGES-2; idx++){
			cfread(&HUD_config.clr[idx], sizeof(color), 1, file);
		}

		// set the 2 new gauges to be radar color
		memcpy(&HUD_config.clr[NUM_HUD_GAUGES-2], &HUD_config.clr[HUD_RADAR], sizeof(color));
		memcpy(&HUD_config.clr[NUM_HUD_GAUGES-1], &HUD_config.clr[HUD_RADAR], sizeof(color));
	} else {
		for(idx=0; idx<NUM_HUD_GAUGES; idx++){
			cfread(&HUD_config.clr[idx], sizeof(color), 1, file);
		}
	}

	// read in the cutscenes which have been viewed
	// moved to campaign file in 242 - taylor
	if (Player_file_version < 242) {
		Cutscenes_viewable = cfread_int(file);
	}

	Master_sound_volume = cfread_float(file);
	Master_event_music_volume = cfread_float(file);
	Master_voice_volume = cfread_float(file);

	audiostream_set_volume_all(Master_voice_volume, ASF_VOICE);
	audiostream_set_volume_all(Master_event_music_volume, ASF_EVENTMUSIC);

	if ( Master_event_music_volume > 0.0f )	{
		Event_music_enabled = 1;
	} else {
		Event_music_enabled = 0;
	}

	read_detail_settings(file, Player_file_version);

	// restore list of most recently played missions
	Num_recent_missions = cfread_int( file );
	Assert(Num_recent_missions <= MAX_RECENT_MISSIONS);
	for ( i = 0; i < Num_recent_missions; i++ ) {
		char *cp;

		cfread_string_len( Recent_missions[i], MAX_FILENAME_LEN, file);
		// Remove the extension
		cp = strchr(Recent_missions[i], '.');
		if (cp)
			*cp = 0;
	}
	
	// use this block of stats from now on
	read_stats_block(file, Player_file_version, &p->stats);	

	Game_skill_level = cfread_int(file);

	for (i=0; i<NUM_JOY_AXIS_ACTIONS; i++) {
		Axis_map_to[i] = cfread_int(file);
		Invert_axis[i] = cfread_int(file);
	}

	// restore some player flags
	p->save_flags = cfread_int(file);

	// restore the most recent ship selection	
	pilot_read_loadout(file, Player_file_version);	

	// read in multiplayer options
	read_multiplayer_options(p,file);

	p->readyroom_listing_mode = cfread_int(file);
	Briefing_voice_enabled = cfread_int(file);

	// restore the default netgame protocol mode
	int protocol_temp = cfread_int(file);

	switch(protocol_temp){
	// plain TCP
	case NET_VMT:	
	case NET_TCP:
		Multi_options_g.protocol = NET_TCP;
		break;
	// IPX
	case NET_IPX:		
		Multi_options_g.protocol = NET_IPX;
		break;			
	}	

	// restore wingman status used by red alert missions
	// moved to campaign file in 242 - taylor
	if ( Player_file_version < 242 ) {
		red_alert_read_wingman_status(file, Player_file_version);
	}

	// read techroom data
	pilot_read_techroom_data(file, Player_file_version);

	// restore auto-advance pref
	Player->auto_advance = cfread_int(file);

	Use_mouse_to_fly = cfread_int(file);
	Mouse_sensitivity = cfread_int(file);
	Joy_sensitivity = cfread_int(file);
	Dead_zone_size = cfread_int(file);

	// Goober5000 - read in player-persistent variables
	// maintain compatibility with previous file versions
	if (Player_file_version >= 141)
	{
		Player->num_variables = cfread_int( file );
		for ( i = 0; i < Player->num_variables; i++ ) {
			Player->player_variables[i].type = cfread_int( file );
			cfread_string_len( Player->player_variables[i].text, TOKEN_LENGTH, file );
			cfread_string_len( Player->player_variables[i].variable_name, TOKEN_LENGTH, file );
		}
	}

	if (cfclose(file))
		return errno;

	// restore the callsign into the Player structure
	strcpy_s(p->callsign, callsign);

	// restore the truncated callsign into Player structure
	pilot_set_short_callsign(p, SHORT_CALLSIGN_PIXEL_W);
	
	// when we store the LastPlayer key, we have to mark it as being single or multiplayer, so we know where to look for him
	// (since we could have a single and a multiplayer pilot with the same callsign)
	// we'll distinguish them by putting an M and the end of the multiplayer callsign and a P at the end of a single player
	char cat[35];

	strcpy_s(cat, p->callsign);
	if (is_multi)
		strcat_s(cat, NOX("M"));
	else
		strcat_s(cat, NOX("S"));

	os_config_write_string( NULL, "LastPlayer", cat );
/*
	// if he's not a multiplayer pilot, then load in the campaign file at this point!
	if (!is_multi) {
		if (mission_campaign_load_by_name(campaign_fname)) {
			strcpy_s(campaign_fname, BUILTIN_CAMPAIGN);
			if (mission_campaign_load_by_name(campaign_fname))
				Assert(0);
		}
	}
	//Campaign.current_mission = mission_num;*/

	hud_squadmsg_save_keys();			// when new pilot read in, must save info for squadmate messaging

	// if we had to upgrade formats force a save to be sure we don't lose anything
	if (pfile_upgrade) {
		write_pilot_file(p);

		// now delete old .plr file if it exists
		delete_pilot_file_old(callsign, single);
	}

	return 0;
}

void read_stats_block(CFILE *file, int pfile_version, scoring_struct *stats)
{
	int i, total;
	int k_count;
	char kname[NAME_LENGTH];

	init_scoring_element(stats);

	if (pfile_version < 242) {
		stats->score = cfread_int(file);
		stats->rank = cfread_int(file);
		stats->assists = cfread_int(file);

		if (pfile_version < 139) {
			// support for FS2_DEMO pilots that still have FS1 medal info in the .plr files
			for (i=0; i < NUM_MEDALS_FS1; i++) {
				total = cfread_int(file);			// dummy read
			}
		} else {
			// read the usual way
			for (i=0; i < MAX_MEDALS; i++) {
				stats->medals[i] = cfread_int(file);
			}
		}

		total = cfread_int(file);
		if (total > MAX_SHIP_CLASSES){
			Warning(LOCATION, "Some ship kill information will be lost due to MAX_SHIP_CLASSES decrease");
		}

		for (i=0; i<total && i<MAX_SHIP_CLASSES; i++){
			if (pfile_version >= 142) {
				k_count = cfread_ushort(file);
				cfread_string_len(kname, NAME_LENGTH, file);
				stats->kills[ship_info_lookup(kname)] = k_count;
			} else {
				stats->kills[i] = cfread_ushort(file);
			}
		}

		stats->kill_count = cfread_int(file);
		stats->kill_count_ok = cfread_int(file);
	
		stats->p_shots_fired = cfread_uint(file);
		stats->s_shots_fired = cfread_uint(file);
		stats->p_shots_hit = cfread_uint(file);
		stats->s_shots_hit = cfread_uint(file);
	
		stats->p_bonehead_hits = cfread_uint(file);
		stats->s_bonehead_hits = cfread_uint(file);
		stats->bonehead_kills = cfread_uint(file);
	}
}

// grab the various detail settings
void read_detail_settings(CFILE *file, int pfile_version)
{
	// mass read the Detail struct
	cfread( &Detail, sizeof(detail_levels), 1, file );

	// swap, swap, swap
	Detail.setting = INTEL_INT(Detail.setting); //-V570
	Detail.nebula_detail = INTEL_INT(Detail.nebula_detail); //-V570
	Detail.detail_distance = INTEL_INT(Detail.detail_distance); //-V570
	Detail.hardware_textures = INTEL_INT(Detail.hardware_textures); //-V570
	Detail.num_small_debris = INTEL_INT(Detail.num_small_debris); //-V570
	Detail.num_particles = INTEL_INT(Detail.num_particles); //-V570
	Detail.num_stars = INTEL_INT(Detail.num_stars); //-V570
	Detail.shield_effects = INTEL_INT(Detail.shield_effects); //-V570
	Detail.lighting = INTEL_INT(Detail.lighting); //-V570
	Detail.targetview_model = INTEL_INT(Detail.targetview_model); //-V570
	Detail.planets_suns = INTEL_INT(Detail.planets_suns); //-V570
	Detail.weapon_extras = INTEL_INT(Detail.weapon_extras); //-V570
}

// Will write the pilot file in the most current format
//
// if single == 1, save into the single players directory, else save into the multiplayers directory
int write_pilot_file_core(player *p)
{
	char filename[MAX_FILENAME_LEN + 1];
   int i, si_index, idx;
	ubyte is_multi;
	CFILE *file;

	// never save a pilot file for the standalone server in multiplayer
	if ((Game_mode & GM_MULTIPLAYER) && (Game_mode & GM_STANDALONE_SERVER)) {
		return 0;
	}

	if (!p) {
		Assert((Player_num >= 0) && (Player_num < MAX_PLAYERS));
		p = &Players[Player_num];
	}

	i = strlen(p->callsign);
	if (i == 0)
		return 0;	//	This means there is no player, probably meaning he was deleted and game exited from same screen.

	Assert((i > 0) && (i <= MAX_FILENAME_LEN - 4));  // ensure we won't overrun the buffer
	strcpy_s( filename, p->callsign);

	// determine if this pilot is a multiplayer pilot or not
	if (p->flags & PLAYER_FLAGS_IS_MULTI){
		is_multi = 1;
	} else {
		is_multi = 0;
	}

	// see above
	if ( !is_multi ){
		strcat_s( filename, NOX(".pl2") ); // support only new format on save - taylor
		file = cfopen(filename, "wb", CFILE_NORMAL, CF_TYPE_SINGLE_PLAYERS);
	} else {
		strcat_s( filename, NOX(".plr") );
		file = cfopen(filename, "wb", CFILE_NORMAL, CF_TYPE_MULTI_PLAYERS);
	}

	if (!file){
		return errno;
	}

	// Write out player's info
	cfwrite_uint(PLR_FILE_ID, file);
	if ( !is_multi )
		cfwrite_uint(CURRENT_PLAYER_FILE_VERSION, file);
	else
		cfwrite_uint(CURRENT_MULTI_PLAYER_FILE_VERSION, file);

	cfwrite_ubyte(is_multi, file);
	cfwrite_int(p->stats.rank, file);

	if ( is_multi ) {
		cfwrite_ubyte((ubyte) p->main_hall, file);
	}

	cfwrite_int(p->tips, file);

	// write out the image file name
	cfwrite_string_len(p->image_filename, file);

	// write out the image file name
	cfwrite_string_len(p->squad_name, file);
	cfwrite_string_len(p->squad_filename, file);

	// write out the name of the player's active campaign.
	cfwrite_string_len(p->current_campaign, file);	

	// write the ship name for last ship flown by the player
	si_index = p->last_ship_flown_si_index;
	if((si_index < 0) || (si_index >= Num_ship_classes)){
		si_index = 0;
	}

	cfwrite_string_len(Ship_info[si_index].name, file);

	// ---------------------------------------------
	// write the keyboard/joystick configuration
	// ---------------------------------------------
	cfwrite_ubyte( CCFG_MAX, file );
	for (i=0; i<CCFG_MAX; i++) {
		cfwrite_short( Control_config[i].key_id, file );
		cfwrite_short( Control_config[i].joy_id, file );
	}		

	// if this hud is an observer, the player ended the last mission as an observer, so we should
	// restore his "real" ship HUD.
	HUD_CONFIG_TYPE hc_temp;
	hc_temp.show_flags = 0;
	int stored_observer = 0;
	if ( HUD_config.is_observer ){
		// if we're in mission, copy the HUD we're currently using
		if(Game_mode & GM_IN_MISSION){			
			memcpy(&hc_temp,&HUD_config,sizeof(HUD_CONFIG_TYPE));
			stored_observer = 1;
		}

		hud_config_restore();				
	}

	// write the hud configuration
	cfwrite_int(HUD_config.show_flags, file);	
	cfwrite_int(HUD_config.show_flags2, file);	
	cfwrite_int(HUD_config.popup_flags, file);	
	cfwrite_int(HUD_config.popup_flags2, file);	
	cfwrite_ubyte( (ubyte) HUD_config.num_msg_window_lines, file );
	cfwrite_int( HUD_config.rp_flags, file );
	cfwrite_int( HUD_config.rp_dist, file );
	// cfwrite_int( HUD_config.color, file );
	// cfwrite_int( HUD_color_alpha, file );
	for(idx=0; idx<NUM_HUD_GAUGES; idx++){
		cfwrite(&HUD_config.clr[idx], sizeof(color), 1, file);
	}

	// restore the HUD we backed up
	if( (Game_mode & GM_IN_MISSION) && stored_observer ){		
		memcpy(&HUD_config,&hc_temp,sizeof(HUD_CONFIG_TYPE));
	}

	// write the cutscenes which have been viewed
	if (is_multi)
		cfwrite_int(Cutscenes_viewable, file);

	// store the digital sound fx volume, and music volume
	cfwrite_float(Master_sound_volume, file);
	cfwrite_float(Master_event_music_volume, file);
	cfwrite_float(Master_voice_volume, file);

	write_detail_settings(file);

	// store list of most recently played missions
	cfwrite_int(Num_recent_missions, file);
	for (i=0; i<Num_recent_missions; i++) {
		cfwrite_string_len(Recent_missions[i], file);
	}

	// write the player stats
	write_stats_block(file, &p->stats, is_multi);	
	cfwrite_int(Game_skill_level, file);

	for (i=0; i<NUM_JOY_AXIS_ACTIONS; i++) {
		cfwrite_int(Axis_map_to[i], file);
		cfwrite_int(Invert_axis[i], file);
	}

	// store some player flags
	cfwrite_int(Player->save_flags, file);

	// store ship selection for most recent mission
	pilot_write_loadout(file, is_multi);

	// read in multiplayer options	
	write_multiplayer_options(p, file);

	cfwrite_int(p->readyroom_listing_mode, file);
	cfwrite_int(Briefing_voice_enabled, file);

	// store the default netgame protocol mode for this pilot
	if (Multi_options_g.protocol == NET_TCP) {		
		cfwrite_int(NET_TCP, file);		
	} else {
		cfwrite_int(NET_IPX, file);
	}	

	if ( is_multi ) {
		red_alert_write_wingman_status(file);
	}

	pilot_write_techroom_data(file, is_multi);

	// store auto-advance pref
	cfwrite_int(Player->auto_advance, file);

	cfwrite_int(Use_mouse_to_fly, file);
	cfwrite_int(Mouse_sensitivity, file);
	cfwrite_int(Joy_sensitivity, file);
	cfwrite_int(Dead_zone_size, file);

	// write out player-persistent variables - Goober5000
	cfwrite_int( Player->num_variables, file );
	for ( i = 0; i < Player->num_variables; i++ ) {
		cfwrite_int( Player->player_variables[i].type, file );
		cfwrite_string_len( Player->player_variables[i].text, file );
		cfwrite_string_len( Player->player_variables[i].variable_name, file );
	}

	if (!cfclose(file))
		return 0;

	return errno;
}

int write_pilot_file(player *the_player)
{
	int pilot_write_rval;
	do {
		// write_pilot_file_core returns 0 if ok, non-zero for error
		pilot_write_rval = write_pilot_file_core(the_player);

		// check with user if write not successful
		if (pilot_write_rval) {
			int popup_rval = popup(PF_TITLE_RED | PF_TITLE_BIG, 3, XSTR( "&Retry", 41), XSTR( "&Ignore", 42), XSTR( "&Quit Game", 43),
				XSTR( "Warning\nFailed to save pilot file.  You may be out of disk space.  If so, you should press Alt-Tab, free up some disk space, then come back and choose retry.\n", 44) );

			// quit game popup return value (2)
			if (popup_rval == 2) {
				exit(1);
			}

			// _ignore_ popup return value (1) - don't save the file
			if (popup_rval) {
				return pilot_write_rval;
			}

			// retry popup return value (0) - try again 
		}

	} while (pilot_write_rval);

	// write successful
	return 0;
}

void write_stats_block(CFILE *file,scoring_struct *stats, int multi)
{

	int i;
	int total;

	if ( !multi )
		return;

	cfwrite_int(stats->score, file);
	cfwrite_int(stats->rank, file);
	cfwrite_int(stats->assists, file);

	//WMC - pilot files don't support different numbers of medals
	Assert(Num_medals == MAX_MEDALS);
	for (i=0; i<Num_medals; i++){
		cfwrite_int(stats->medals[i], file);
	}

	for(;i<MAX_MEDALS;i++){
		cfwrite_int(0, file);
	}

	total = MAX_SHIP_CLASSES;
	while (total && !stats->kills[total - 1]){  // find last used element
		total--;
	}

	cfwrite_int(total, file);
	for (i=0; i<total; i++){
		cfwrite_ushort((ushort)stats->kills[i], file);
		cfwrite_string_len(Ship_info[i].name, file);
	}

	cfwrite_int(stats->kill_count,file);
	cfwrite_int(stats->kill_count_ok,file);

	cfwrite_uint(stats->p_shots_fired,file);
	cfwrite_uint(stats->s_shots_fired,file);
	cfwrite_uint(stats->p_shots_hit,file);
	cfwrite_uint(stats->s_shots_hit,file);
	cfwrite_uint(stats->p_bonehead_hits,file);
	cfwrite_uint(stats->s_bonehead_hits,file);
	cfwrite_uint(stats->bonehead_kills,file);
}

// write the various detail settings
void write_detail_settings(CFILE *file)
{
	// we still need sane values in the Detail struct so create
	// a temporary one to value swap and write to file
	detail_levels Detail_tmp;
	memset(&Detail_tmp, 0, sizeof(detail_levels));
	memcpy(&Detail_tmp, &Detail, sizeof(detail_levels));

	// swap, swap, swap - on big-endian this will convert back to little-endian
	Detail_tmp.setting = INTEL_INT(Detail_tmp.setting); //-V570
	Detail_tmp.nebula_detail = INTEL_INT(Detail_tmp.nebula_detail); //-V570
	Detail_tmp.detail_distance = INTEL_INT(Detail_tmp.detail_distance); //-V570
	Detail_tmp.hardware_textures = INTEL_INT(Detail_tmp.hardware_textures); //-V570
	Detail_tmp.num_small_debris = INTEL_INT(Detail_tmp.num_small_debris); //-V570
	Detail_tmp.num_particles = INTEL_INT(Detail_tmp.num_particles); //-V570
	Detail_tmp.num_stars = INTEL_INT(Detail_tmp.num_stars); //-V570
	Detail_tmp.shield_effects = INTEL_INT(Detail_tmp.shield_effects); //-V570
	Detail_tmp.lighting = INTEL_INT(Detail_tmp.lighting); //-V570
	Detail_tmp.targetview_model = INTEL_INT(Detail_tmp.targetview_model); //-V570
	Detail_tmp.planets_suns = INTEL_INT(Detail_tmp.planets_suns); //-V570
	Detail_tmp.weapon_extras = INTEL_INT(Detail_tmp.weapon_extras); //-V570

	cfwrite( &Detail_tmp, sizeof(detail_levels), 1, file );
}

// write multiplayer information
void write_multiplayer_options(player *p,CFILE *file)
{	
	// write the netgame options
	cfwrite_ubyte(p->m_server_options.squad_set,file);
	cfwrite_ubyte(p->m_server_options.endgame_set,file);	
	cfwrite_int(p->m_server_options.flags,file);
	cfwrite_uint(p->m_server_options.respawn,file);
	cfwrite_ubyte(p->m_server_options.max_observers,file);
	cfwrite_ubyte(p->m_server_options.skill_level,file);
	cfwrite_ubyte(p->m_server_options.voice_qos,file);
	cfwrite_int(p->m_server_options.voice_token_wait,file);
	cfwrite_int(p->m_server_options.voice_record_time,file);
	cfwrite(&p->m_server_options.mission_time_limit,sizeof(fix),1,file);
	cfwrite_int(p->m_server_options.kill_limit,file);

	// write the local options
	cfwrite_int(p->m_local_options.flags,file);
	cfwrite_int(p->m_local_options.obj_update_level,file);
}

// read multiplayer options
void read_multiplayer_options(player *p,CFILE *file)
{
	// read the netgame options
	p->m_server_options.squad_set = cfread_ubyte(file);
	p->m_server_options.endgame_set = cfread_ubyte(file);	
	p->m_server_options.flags = cfread_int(file);
	p->m_server_options.respawn = cfread_uint(file);
	p->m_server_options.max_observers = cfread_ubyte(file);
	p->m_server_options.skill_level = cfread_ubyte(file);
	p->m_server_options.voice_qos = cfread_ubyte(file);
	p->m_server_options.voice_token_wait = cfread_int(file);
	p->m_server_options.voice_record_time = cfread_int(file);
	cfread(&p->m_server_options.mission_time_limit,sizeof(fix),1,file);
	p->m_server_options.kill_limit = cfread_int(file);

	// read the local options
	p->m_local_options.flags = cfread_int(file);
	p->m_local_options.obj_update_level = cfread_int(file);
}

// run thorough an open file (at the beginning) and see if this pilot is a multiplayer pilot
int is_pilot_multi(CFILE *fp)
{
	uint id,file_version;
	ubyte is_multi;
	
	id = cfread_uint(fp);
	if (id != PLR_FILE_ID) {
		Warning(LOCATION, "Player file has invalid signature");
		cfclose(fp);
		return -1;
	}

	// check for compatibility here
	file_version = cfread_uint(fp);
	if (file_version < LOWEST_COMPATIBLE_PLAYER_FILE_VERSION) {
		nprintf(("Warning", "WARNING => Player file is outdated and not compatible...\n"));
		cfclose(fp);
		return -1;
	}

	// read in whether he's a multiplayer or not
	is_multi = cfread_ubyte(fp);
	if (is_multi)
		return 1;

	return 0;
}

int is_pilot_multi(player *p)
{
	return (p->flags & PLAYER_FLAGS_IS_MULTI) ? 1 : 0;
}

// this works on barracks and player_select interface screens
void init_new_pilot(player *p, int reset)
{
	int cur_speed;

	if (reset) {
		hud_set_default_hud_config(p);		// use a default hud config

		// in the demo, load up the hardcoded hcf file
#ifdef FS2_DEMO
		hud_config_color_load("hud_1.hcf");
#else
		hud_config_color_load("hud_3.hcf");
#endif

		control_config_reset_defaults();		// get a default keyboard config
		player_set_pilot_defaults(p);			// set up any player struct defaults

		cur_speed = os_config_read_uint(NULL, NOX("ComputerSpeed"), 2 );
		if ( cur_speed < 0 )	{
			cur_speed = 0;
		} else if ( cur_speed >= NUM_DEFAULT_DETAIL_LEVELS )	{
			cur_speed = NUM_DEFAULT_DETAIL_LEVELS-1;
		}	
		// always set to high
		// DKA: 8/4/99 USE speed from registry
		// cur_speed = NUM_DEFAULT_DETAIL_LEVELS-2;

#if NUM_DEFAULT_DETAIL_LEVELS != 4
#error Code in ManagePilot assumes NUM_DEFAULT_DETAIL_LEVELS = 4
#endif

		detail_level_set(cur_speed);

		Game_skill_level = game_get_default_skill_level();

		mprintf(( "Setting detail level to %d because of new pilot\n", cur_speed ));
		Use_mouse_to_fly = 1;
		Mouse_sensitivity = 4;
		Joy_sensitivity = 9;
		Dead_zone_size = 10;
	}

	// unassigned squadron
	strcpy_s(p->squad_name, XSTR("Unassigned", 1255));
	strcpy_s(p->squad_filename, "");

	// set him to be a single player pilot by default (the actual creation routines will change this if necessary)
	p->flags &= ~PLAYER_FLAGS_IS_MULTI;

	// effectively sets the length return by strlen() to 0	
	Campaign.filename[0] = 0;
	p->main_hall = 0;

	// pick a random pilot image for this guy
	if (reset){
		pilot_set_random_pic(p);
		p->insignia_texture = -1;
		pilot_set_random_squad_pic(p);
	}

	init_scoring_element(&p->stats);	
	
	p->stats.score = 0;
	p->stats.rank = RANK_ENSIGN;	

	p->tips = 1;

	Multi_options_g.protocol = NET_TCP;	

	// initialize default multiplayer options
	multi_options_set_netgame_defaults(&p->m_server_options);
	multi_options_set_local_defaults(&p->m_local_options);

	Player_loadout.filename[0] = 0;

	// reset the cutscenes which can be viewed
	if ( reset ){
		Cutscenes_viewable = INTRO_CUTSCENE_FLAG;
	}
}

void pilot_set_short_callsign(player *p, int max_width)
{
	strcpy_s(p->short_callsign, p->callsign);
	gr_set_font(FONT1);
	gr_force_fit_string(p->short_callsign, CALLSIGN_LEN - 1, max_width);
	gr_get_string_size( &(p->short_callsign_width), NULL, p->short_callsign );
}

// pick a random image for the passed player
void pilot_set_random_pic(player *p)
{
	// if there are no available pilot pics, set the image filename to null
	if (Num_pilot_images <= 0) {
		strcpy_s(p->image_filename, "");
	} else {
		// pick a random name from the list
		int random_index = rand() % Num_pilot_images;
		Assert((random_index >= 0) && (random_index < Num_pilot_images));
		strcpy_s(p->image_filename, Pilot_images_arr[random_index]);
	}	
}

// pick a random image for the passed player
void pilot_set_random_squad_pic(player *p)
{	
	// if there are no available pilot pics, set the image filename to null
	if (Num_pilot_squad_images <= 0) {
		player_set_squad_bitmap(p, "");
		// strcpy_s(p->squad_filename, "");		
	} else {
		// pick a random name from the list
		int random_index = rand() % Num_pilot_squad_images;		
		Assert((random_index >= 0) && (random_index < Num_pilot_squad_images));
		player_set_squad_bitmap(p, Pilot_squad_images_arr[random_index]); 
		// strcpy_s(p->squad_filename, Pilot_squad_images_arr[random_index]);
	}	
}

// format a pilot's callsign into a "personal" form - ie, adding a 's or just an ' as appropriate
void pilot_format_callsign_personal(char *in_callsign,char *out_callsign)
{
	// don't do anything if we've got invalid strings
	if((in_callsign == NULL) || (out_callsign == NULL)){
		return;
	}

	// copy the original string
	strcpy(out_callsign,in_callsign);

	// tack on the appropriate postfix
	if(in_callsign[strlen(in_callsign) - 1] == 's'){		
		strcat(out_callsign,XSTR( "\'", 45));
	} else {
		strcat(out_callsign,XSTR( "\'s", 46));
	}
}

// throw up a popup asking the user to verify the overwrite of an existing pilot name
// 1 == ok to overwrite, 0 == not ok
int pilot_verify_overwrite()
{
	return popup(PF_USE_AFFIRMATIVE_ICON | PF_USE_NEGATIVE_ICON | PF_TITLE_RED | PF_TITLE_BIG, 2, XSTR( "&No", 47), XSTR( "&Yes", 48), XSTR( "Warning\nA duplicate pilot exists\nOverwrite?", 49));
}

extern int Skip_packfile_search;  // located in CFileList.cpp


// load up the list of pilot image filenames (do this at game startup as well as barracks startup)
void pilot_load_pic_list()
{
	Num_pilot_images = 0;
	
	// load pilot images from the player images directory
	Num_pilot_images = cf_get_file_list_preallocated(MAX_PILOT_IMAGES, Pilot_images_arr, Pilot_image_names, CF_TYPE_PLAYER_IMAGES, NOX("*.pcx"));

	// sort all filenames
	cf_sort_filenames(Num_pilot_images, Pilot_image_names, CF_SORT_NAME);
}

// load up the list of pilot squad filenames
void pilot_load_squad_pic_list()
{
	Num_pilot_squad_images = 0;
	
	// load pilot images from the player images directory
	Num_pilot_squad_images = cf_get_file_list_preallocated(MAX_PILOT_IMAGES, Pilot_squad_images_arr, Pilot_squad_image_names, CF_TYPE_SQUAD_IMAGES, NOX("*.pcx"));

	// sort all filenames
	cf_sort_filenames(Num_pilot_squad_images, Pilot_squad_image_names, CF_SORT_NAME);
}

// will attempt to load an insignia bitmap and set it as active for the player
void player_set_squad_bitmap(player *p, char *fname)
{
	// sanity check
	if(p == NULL){
		return;
	}

	// if he has another bitmap already - unload it
	if (p->insignia_texture >= 0) {
		bm_release(p->insignia_texture);
	}

	p->insignia_texture = -1;

	// try and set the new one
	if (fname != p->squad_filename) {
		strncpy(p->squad_filename, fname, MAX_FILENAME_LEN);
	}

	if (p->squad_filename[0] != '\0') {
		p->insignia_texture = bm_load_duplicate(fname);
		
		// lock is as a transparent texture
		if (p->insignia_texture != -1) {
			bm_lock(p->insignia_texture, 16, BMP_TEX_XPARENT);
			bm_unlock(p->insignia_texture);
		}
	}

	/*
	flen = strlen(filename);
	elen = strlen(ext);
	Assert(flen < MAX_PATH_LEN);
	strcpy_s(path, filename);
	if ((flen < 4) || stricmp(path + flen - elen, ext)) {
		Assert(flen + elen < MAX_PATH_LEN);
		strcat_s(path, ext);
	}
	*/
}

// set squadron
void player_set_squad(player *p, char *squad_name)
{
	// sanity check
	if(p == NULL){
		return;
	}

	strncpy(p->squad_name, squad_name, NAME_LENGTH+1);
}

DCF(pilot,"Changes pilot stats. (Like reset campaign)" )
{
	if (Dc_command) {
		dc_get_arg(ARG_STRING);
		if (!strcmp(Dc_arg, NOX("reset"))) {
			if (strlen(Campaign.filename)) {
				mission_campaign_savefile_delete(Campaign.filename);
				mission_campaign_load(Campaign.filename);
			}

		} else {
			Dc_help = 1;  // print usage, not stats
		}
	}

	if (Dc_help) {
		dc_printf( "Usage: pilot keyword\nWhere keyword can be in the following forms:\n" );
		dc_printf( "pilot reset			Resets campaign stats.\n" );
		Dc_status = 0;	// don't print status if help is printed.  Too messy.
	}

	if (Dc_status) {
		// no stats
	}
}
