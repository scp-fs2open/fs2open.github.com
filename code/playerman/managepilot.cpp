/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

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
#include "mod_table/mod_table.h"


// pilot pic image list stuff ( call pilot_load_pic_list() to make these valid )
char Pilot_images_arr[MAX_PILOT_IMAGES][MAX_FILENAME_LEN];
char *Pilot_image_names[MAX_PILOT_IMAGES];
int Num_pilot_images = 0;

// squad logo list stuff (call pilot_load_squad_pic_list() to make these valid )
char Pilot_squad_images_arr[MAX_PILOT_IMAGES][MAX_FILENAME_LEN];
char *Pilot_squad_image_names[MAX_PILOT_IMAGES];
int Num_pilot_squad_images = 0;


// internal function to delete a player file.  Called after a pilot is obsoleted, and when a pilot is deleted
// used in barracks and player_select
//returns 0 on failure, 1 on success
int delete_pilot_file(char *pilot_name)
{
	int delreturn;
	char filename[MAX_FILENAME_LEN];
	char basename[MAX_FILENAME_LEN];

	// get the player file.
	_splitpath(pilot_name, NULL, NULL, basename, NULL);

	strcpy_s( filename, basename );
	strcat_s( filename, NOX(".plr") );

	delreturn = cf_delete(filename, CF_TYPE_PLAYERS);

	// we must try and delete the campaign save files for a pilot as well.
	if (delreturn) {
		mission_campaign_delete_all_savefiles(basename);
		return 1;
	} else {
		return 0;
	}
}

// this works on barracks and player_select interface screens
void init_new_pilot(player *p, int reset)
{
	int cur_speed;

	if (reset) {
		hud_set_default_hud_config(p);		// use a default hud config

		control_config_reset_defaults();		// get a default keyboard config
		player_set_pilot_defaults(p);			// set up any player struct defaults

		// set the default detail level based on tabling rather than the above method
		cur_speed = Default_detail_level;

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
		cutscene_init();
	}

	pilot_set_start_campaign(p);
}

int local_num_campaigns = 0;

int campaign_file_list_filter(char *filename)
{
	char name[NAME_LENGTH];
	char *desc = NULL;
	int type, max_players;

	if ( mission_campaign_get_info( filename, name, &type, &max_players, &desc) ) {
		if ( type == CAMPAIGN_TYPE_SINGLE) {
			Campaign_names[local_num_campaigns] = vm_strdup(name);
			local_num_campaigns++;

			return 1;
		}
	}

	if (desc != NULL)
		vm_free(desc);
 
	return 0;
}

//Generates a list of available campaigns. Sets active campaign to "freespace2.fc2", or if that is unavailable, the first campaign found
void pilot_set_start_campaign(player* p)
{
	char wild_card[10];
	int i, j, incr = 0;
	char *t = NULL;
	int rc = 0;
	char *campaign_file_list[MAX_CAMPAIGNS];

	memset(wild_card, 0, sizeof(wild_card));
	strcpy_s(wild_card, NOX("*"));
	strcat_s(wild_card, FS_CAMPAIGN_FILE_EXT);

	// set filter for cf_get_file_list() if there isn't one set already (the simroom has a special one)
	if (Get_file_list_filter == NULL)
		Get_file_list_filter = campaign_file_list_filter;

	// now get the list of all campaign names
	// NOTE: we don't do sorting here, but we assume CF_SORT_NAME, and do it manually below
	rc = cf_get_file_list(MAX_CAMPAIGNS, campaign_file_list, CF_TYPE_MISSIONS, wild_card, CF_SORT_NONE);

	for (i = 0; i < rc; i++) 
	{
		if (!stricmp(campaign_file_list[i], Default_campaign_file_name))
		{
			strcpy_s(p->current_campaign, campaign_file_list[i]);
			return;
		}
	}

	// now sort everything
	incr = local_num_campaigns / 2;

	while (incr > 0) {
		for (i = incr; i < local_num_campaigns; i++) {
			j = i - incr;
	
			while (j >= 0) {
				char *name1 = Campaign_names[j];
				char *name2 = Campaign_names[j + incr];

				// if we hit this then a coder probably did something dumb (like not needing to sort)
				if ( (name1 == NULL) || (name2 == NULL) ) {
					Int3();
					break;
				}

				if ( !strnicmp(name1, "the ", 4) )
					name1 += 4;

				if ( !strnicmp(name2, "the ", 4) )
					name2 += 4;

				if (stricmp(name1, name2) > 0) {
					// first, do filenames
					t = campaign_file_list[j];
					campaign_file_list[j] = campaign_file_list[j + incr];
					campaign_file_list[j + incr] = t;

					j -= incr;
				} else {
					break;
				}
			}
		}

		incr /= 2;
	}

	if (rc > 0)
		strcpy_s(p->current_campaign, campaign_file_list[0]);
	else
		strcpy_s(p->current_campaign, "<none>");

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
	if (p->insignia_texture > 0) {
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

void player::reset()
{
	memset(callsign, 0, sizeof(callsign));
	memset(short_callsign, 0, sizeof(short_callsign));

	short_callsign_width = 0;

	memset(image_filename, 0, sizeof(image_filename));
	memset(squad_filename, 0, sizeof(squad_filename));
	memset(squad_name, 0, sizeof(squad_name));

	memset(current_campaign, 0, sizeof(current_campaign));

	readyroom_listing_mode = 0;
	flags = 0;
	save_flags = 0;

	memset(keyed_targets, 0, sizeof(keyed_targets));
	current_hotkey_set = -1;

	lead_target_pos = vmd_zero_vector;
	lead_target_cheat = 0;
	lead_indicator_active = 0;

	lock_indicator_x = 0;
	lock_indicator_y = 0;
	lock_indicator_start_x = 0;
	lock_indicator_start_y = 0;
	lock_indicator_visible = 0;
	lock_time_to_target = 0.0f;
	lock_dist_to_target = 0.0f;

	last_ship_flown_si_index = -1;

	objnum = -1;

	memset(&bi, 0, sizeof(button_info));
	memset(&ci, 0, sizeof(control_info));

	init_scoring_element(&stats);

	friendly_hits = 0;
	friendly_damage = 0.0f;
	friendly_last_hit_time = 0;
	last_warning_message_time = 0;

	control_mode = 0;
	saved_viewer_mode = 0;

	check_warn_timestamp = -1;

	distance_warning_count = 0;
	distance_warning_time = -1;

	allow_warn_timestamp = -1;
	warn_count = 0;

	damage_this_burst = 0.0f;

	repair_sound_loop = -1;
	cargo_scan_loop = -1;

	praise_count = 0;
	allow_praise_timestamp = -1;
	praise_delay_timestamp = -1;

	ask_help_count = 0;
	allow_ask_help_timestamp = -1;

	scream_count = 0;
	allow_scream_timestamp = -1;

	low_ammo_complaint_count = 0;
	allow_ammo_timestamp = -1;

	subsys_in_view = -1;
	request_repair_timestamp = -1;

	cargo_inspect_time = -1;
	target_is_dying = -1;
	current_target_sx = 0;
	current_target_sy = 0;
	target_in_lock_cone = 0;
	locking_subsys = NULL;
	locking_subsys_parent = -1;
	locking_on_center = 0;

	killer_objtype = -1;
	killer_species = 0;
	killer_weapon_index = -1;
	memset(killer_parent_name, 0, sizeof(killer_parent_name));

	check_for_all_alone_msg = -1;

	update_dumbfire_time = -1;
	update_lock_time = -1;
	threat_flags = 0;
	auto_advance = 1;

	multi_options_set_local_defaults(&m_local_options);
	multi_options_set_netgame_defaults(&m_server_options);

	insignia_texture = -1;

	tips = 1;

	shield_penalty_stamp = 0;

	failures_this_session = 0;
	show_skip_popup = 0;

	variables.clear();

	death_message = "";

	memset(&lua_ci, 0, sizeof(control_info));
	memset(&lua_bi, 0, sizeof(button_info));
	memset(&lua_bi_full, 0, sizeof(button_info));
}
