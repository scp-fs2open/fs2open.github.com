/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include "cfile/cfile.h"
#include "cutscene/cutscenes.h"
#include "freespace.h"
#include "gamesnd/eventmusic.h"
#include "graphics/font.h"
#include "hud/hudconfig.h"
#include "hud/hudsquadmsg.h"
#include "io/joy.h"
#include "io/mouse.h"
#include "menuui/playermenu.h"
#include "menuui/readyroom.h"
#include "menuui/techmenu.h"
#include "mission/missionbriefcommon.h"
#include "mission/missioncampaign.h"
#include "mission/missionload.h"
#include "missionui/missionscreencommon.h"
#include "missionui/missionshipchoice.h"
#include "missionui/redalert.h"
#include "mod_table/mod_table.h"
#include "network/multi.h"
#include "osapi/osregistry.h"
#include "pilotfile/pilotfile.h"
#include "playerman/managepilot.h"
#include "playerman/player.h"
#include "popup/popup.h"
#include "ship/ship.h"
#include "sound/audiostr.h"
#include "sound/sound.h"
#include "weapon/weapon.h"


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
bool delete_pilot_file(const char *pilot_name)
{
	int delreturn;
	char filename[MAX_FILENAME_LEN];
	char basename[MAX_FILENAME_LEN];

	// get the player file.
	_splitpath(pilot_name, NULL, NULL, basename, NULL);

	strcpy_s( filename, basename );
	strcat_s( filename, NOX(".plr") );

	cf_delete(filename, CF_TYPE_PLAYERS, CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);

	strcpy_s( filename, basename );
	strcat_s( filename, NOX(".json") );

	delreturn =
	    cf_delete(filename, CF_TYPE_PLAYERS, CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);

	// we must try and delete the campaign save files for a pilot as well.
	if (delreturn) {
		mission_campaign_delete_all_savefiles(basename);
		return true;
	} else {
		return false;
	}
}

// this works on barracks and player_select interface screens
void init_new_pilot(player *p, int reset)
{
	if (reset) {
		hud_set_default_hud_config(p, HC_default_preset_file); // use a default hud config

		control_config_use_preset(Control_config_presets[0]);		// get a default keyboard config
		player_set_pilot_defaults(p);			// set up any player struct defaults

		// set the default detail preset based on tabling rather than the above method
		DefaultDetailPreset cur_speed = Default_detail_preset;

		static_assert(static_cast<int>(DefaultDetailPreset::Num_detail_presets) == 4, "Code in ManagePilot assumes Num_detail_presets = 4");

		detail_level_set(cur_speed);

		Game_skill_level = game_get_default_skill_level();

		mprintf(( "Setting detail level to %d because of new pilot\n", static_cast<int>(cur_speed) ));
		Use_mouse_to_fly = true;
		Mouse_sensitivity = 4;
		if (!Using_in_game_options) {
			// This is handled by the in game options in this case
			Joy_sensitivity = 9;
			Joy_dead_zone_size = 10;
		}

		snd_set_effects_volume(Default_sound_volume);
		event_music_set_volume(Default_music_volume);
		snd_set_voice_volume(Default_voice_volume);

		p->variables.clear();
		p->containers.clear();
	}

	// unassigned squadron
	strcpy_s(p->s_squad_name, XSTR("Unassigned", 1255));
	strcpy_s(p->s_squad_filename, "");
	strcpy_s(p->m_squad_name, XSTR("Unassigned", 1255));
	strcpy_s(p->m_squad_filename, "");

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

	p->stats.init();
	Pilot.reset_stats();
	
	p->stats.score = 0;
	p->stats.rank = 0;	

	p->tips = 1;

	// set pilot language to the current language
	lcl_get_language_name(p->language);

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

	if (OnCampaignBeginHook->isActive()) {
		OnCampaignBeginHook->run(scripting::hook_param_list(scripting::hook_param("Campaign", 's', p->current_campaign)));
	}
}

int local_num_campaigns = 0;

int campaign_file_list_filter(const char *filename)
{
	SCP_string name;
	int type, max_players;

	if ( mission_campaign_get_info( filename, name, &type, &max_players) ) {
		if ( type == CAMPAIGN_TYPE_SINGLE) {
			Campaign_names[local_num_campaigns] = vm_strdup(name.c_str());
			local_num_campaigns++;
			return 1;
		}
	}

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
	font::set_font(font::FONT1);
	font::force_fit_string(p->short_callsign, CALLSIGN_LEN - 1, max_width);
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
		int random_index = Random::next(Num_pilot_images);
		Assert((random_index >= 0) && (random_index < Num_pilot_images));
		strcpy_s(p->image_filename, Pilot_images_arr[random_index]);
	}	
}

/*
 * pick a random squad image for the passed player
 * sets single & multi squad pic to the same image
 *
 * @param p	pointer to player
 */
void pilot_set_random_squad_pic(player *p)
{	
	// if there are no available pilot pics, set the image filename to null
	if (Num_pilot_squad_images <= 0) {
		player_set_squad_bitmap(p, "", true);
		player_set_squad_bitmap(p, "", false);
	} else {
		// pick a random name from the list
		int random_index = Random::next(Num_pilot_squad_images);
		Assert((random_index >= 0) && (random_index < Num_pilot_squad_images));
		player_set_squad_bitmap(p, Pilot_squad_images_arr[random_index], true);
		player_set_squad_bitmap(p, Pilot_squad_images_arr[random_index], false);
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
void player_set_squad_bitmap(player *p, const char *fname, bool ismulti)
{
	// sanity check
	if(p == NULL){
		return;
	}

	char *squad_pic_p;
	if (ismulti) {
		squad_pic_p = p->m_squad_filename;
	} else {
		squad_pic_p = p->s_squad_filename;
	}

	// if he has another bitmap already - unload it
	if (p->insignia_texture > 0) {
		bm_release(p->insignia_texture);
	}

	p->insignia_texture = -1;

	// try and set the new one
	if (fname != squad_pic_p) {
		strncpy(squad_pic_p, fname, MAX_FILENAME_LEN);
	}

	if (squad_pic_p[0] != '\0') {
		p->insignia_texture = bm_load_duplicate(fname);

		// lock is as a transparent texture
		if (p->insignia_texture != -1) {
			bm_lock(p->insignia_texture, 16, BMP_TEX_XPARENT);
			bm_unlock(p->insignia_texture);
		}
	}
}

// set squadron
void player_set_squad(player *p, char *squad_name)
{
	// sanity check
	if(p == NULL){
		return;
	}

	if (Game_mode & GM_MULTIPLAYER) {
		strcpy_s(p->m_squad_name, squad_name);
	} else {
		strcpy_s(p->s_squad_name, squad_name);
	}
}

void player::reset()
{
	memset(callsign, 0, sizeof(callsign));
	memset(short_callsign, 0, sizeof(short_callsign));
	short_callsign_width = 0;

	memset(image_filename, 0, sizeof(image_filename));
	memset(s_squad_filename, 0, sizeof(s_squad_filename));
	memset(s_squad_name, 0, sizeof(s_squad_name));
	memset(m_squad_filename, 0, sizeof(m_squad_filename));
	memset(m_squad_name, 0, sizeof(m_squad_name));

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

	memset(&bi, 0, sizeof(bi));
	memset(&ci, 0, sizeof(ci));

	stats.init();
	// only reset Pilotfile stats if we're resetting Player
	// remember: multi has many Players...
	if (Player == this) {
		Pilot.reset_stats();
	}

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

	repair_sound_loop = sound_handle::invalid();
	cargo_scan_loop   = sound_handle::invalid();

	praise_count = 0;
	allow_praise_timestamp = -1;
	praise_delay_timestamp = -1;

	ask_help_count = 0;
	allow_ask_help_timestamp = -1;

	scream_count = 0;
	allow_scream_timestamp = -1;

	low_ammo_complaint_count = 0;
	allow_ammo_timestamp = -1;

	praise_self_count = 0;
	praise_self_timestamp = -1;

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

	shield_penalty_stamp = -1;

	failures_this_session = 0;
	show_skip_popup = 0;

	variables.clear();
	containers.clear();

	death_message = "";

	memset(&lua_ci, 0, sizeof(lua_ci));
	memset(&lua_bi, 0, sizeof(lua_bi));
	memset(&lua_bi_full, 0, sizeof(lua_bi_full));

	player_was_multi = 0;
	memset(language, 0, sizeof(language));
}

void player::assign(const player *other)
{
	int i;

	strcpy_s(callsign, other->callsign);
	strcpy_s(short_callsign, other->short_callsign);
	short_callsign_width = other->short_callsign_width;

	strcpy_s(image_filename, other->image_filename);
	strcpy_s(s_squad_filename, other->s_squad_filename);
	strcpy_s(s_squad_name, other->s_squad_name);
	strcpy_s(m_squad_filename, other->m_squad_filename);
	strcpy_s(m_squad_name, other->m_squad_name);

	strcpy_s(current_campaign, other->current_campaign);
	readyroom_listing_mode = other->readyroom_listing_mode;

	flags = other->flags;
	save_flags = other->save_flags;

	memcpy(keyed_targets, other->keyed_targets, sizeof(keyed_targets));
	// make sure we correctly set the pointers
	for (i = 0; i < MAX_KEYED_TARGETS; i++)
	{
		if (other->keyed_targets[i].next == NULL)
			keyed_targets[i].next = NULL;
		else
		{
			size_t index = (other->keyed_targets[i].next - &other->keyed_targets[0]);
			keyed_targets[i].next = &keyed_targets[index];
		}

		if (other->keyed_targets[i].prev == NULL)
			keyed_targets[i].prev = NULL;
		else
		{
			size_t index = (other->keyed_targets[i].prev - &other->keyed_targets[0]);
			keyed_targets[i].prev = &keyed_targets[index];
		}
	}
	current_hotkey_set = other->current_hotkey_set;

	lead_target_pos = other->lead_target_pos;
	lead_target_cheat = other->lead_target_cheat;
	lead_indicator_active = other->lead_indicator_active;

	lock_indicator_x = other->lock_indicator_x;
	lock_indicator_y = other->lock_indicator_y;
	lock_indicator_start_x = other->lock_indicator_start_x;
	lock_indicator_start_y = other->lock_indicator_start_y;
	lock_indicator_visible = other->lock_indicator_visible;
	lock_time_to_target = other->lock_time_to_target;
	lock_dist_to_target = other->lock_dist_to_target;

	last_ship_flown_si_index = other->last_ship_flown_si_index;

	// this one might be dicey
	objnum = other->objnum;

	memcpy(&bi, &other->bi, sizeof(button_info));
	memcpy(&ci, &other->ci, sizeof(control_info));

	stats.assign(other->stats);

	friendly_hits = other->friendly_hits;
	friendly_damage = other->friendly_damage;
	friendly_last_hit_time = other->friendly_last_hit_time;
	last_warning_message_time = other->last_warning_message_time;

	control_mode = other->control_mode;
	saved_viewer_mode = other->saved_viewer_mode;

	check_warn_timestamp = other->check_warn_timestamp;

	distance_warning_count = other->distance_warning_count;
	distance_warning_time = other->distance_warning_time;

	allow_warn_timestamp = other->allow_warn_timestamp;
	warn_count = other->warn_count;
	damage_this_burst = other->damage_this_burst;

	repair_sound_loop = other->repair_sound_loop;
	cargo_scan_loop = other->cargo_scan_loop;

	praise_count = other->praise_count;
	allow_praise_timestamp = other->allow_praise_timestamp;
	praise_delay_timestamp = other->praise_delay_timestamp;

	ask_help_count = other->ask_help_count;
	allow_ask_help_timestamp = other->allow_ask_help_timestamp;

	scream_count = other->scream_count;
	allow_scream_timestamp = other->allow_scream_timestamp;

	low_ammo_complaint_count = other->low_ammo_complaint_count;
	allow_ammo_timestamp = other->allow_ammo_timestamp;

	praise_self_count = other->praise_self_count;
	praise_self_timestamp = other->praise_self_timestamp;

	subsys_in_view = other->subsys_in_view;
	request_repair_timestamp = other->request_repair_timestamp;

	cargo_inspect_time = other->cargo_inspect_time;
	target_is_dying = other->target_is_dying;
	current_target_sx = other->current_target_sx;
	current_target_sy = other->current_target_sy;
	target_in_lock_cone = other->target_in_lock_cone;
	locking_subsys = other->locking_subsys;
	locking_subsys_parent = other->locking_subsys_parent;
	locking_on_center = other->locking_on_center;

	killer_objtype = other->killer_objtype;
	killer_species = other->killer_species;
	killer_weapon_index = other->killer_weapon_index;
	strcpy_s(killer_parent_name, other->killer_parent_name);

	check_for_all_alone_msg = other->check_for_all_alone_msg;

	update_dumbfire_time = other->update_dumbfire_time;
	update_lock_time = other->update_lock_time;
	threat_flags = other->threat_flags;
	auto_advance = other->auto_advance;

	memcpy(&m_local_options, &other->m_local_options, sizeof(multi_local_options));
	memcpy(&m_server_options, &other->m_server_options, sizeof(multi_server_options));

	insignia_texture = other->insignia_texture;

	tips = other->tips;

	shield_penalty_stamp = other->shield_penalty_stamp;

	failures_this_session = other->failures_this_session;
	show_skip_popup = other->show_skip_popup;

	variables.clear();
	variables.reserve(other->variables.size());
	for (SCP_vector<sexp_variable>::const_iterator ii = other->variables.begin(); ii != other->variables.end(); ++ii)
	{
		sexp_variable temp;
		memcpy(&temp, &(*ii), sizeof(sexp_variable));
		variables.push_back(temp);
	}

	containers = other->containers;

	death_message = other->death_message;

	memcpy(&lua_ci, &other->lua_ci, sizeof(control_info));
	memcpy(&lua_bi, &other->lua_bi, sizeof(button_info));
	memcpy(&lua_bi_full, &other->lua_bi_full, sizeof(button_info));

	player_was_multi = other->player_was_multi;
	strcpy_s(language, other->language);

	Assertion(*this == *other, "Equality comparison failed after pilot assignment!");
}

template<typename T, size_t N>
bool array_compare(const T (&left)[N], const T (&right)[N]) {
	auto left_el = std::begin(left);
	auto right_el = std::begin(right);

	auto left_end = std::end(left);
	auto right_end = std::end(right);

	for (; left_el != left_end && right_el != right_end; ++left_el, ++right_el) {
		if (!(*left_el == *right_el)) {
			return false;
		}
	}

	return true;
}

bool operator==(const htarget_list& left, const htarget_list& right) {
	return left.how_added == right.how_added && obj_compare(left.objp, right.objp);
}

bool operator==(const button_info& left, const button_info& right) {
	return array_compare(left.status, right.status);
}

bool operator==(const control_info& left, const control_info& right) {
	return left.pitch == right.pitch && left.vertical == right.vertical && left.heading == right.heading
		&& left.sideways == right.sideways && left.bank == right.bank && left.forward == right.forward
		&& left.forward_cruise_percent == right.forward_cruise_percent
		&& left.fire_primary_count == right.fire_primary_count
		&& left.fire_secondary_count == right.fire_secondary_count
		&& left.fire_countermeasure_count == right.fire_countermeasure_count
		&& left.fire_debug_count == right.fire_debug_count && left.afterburner_start == right.afterburner_start
		&& left.afterburner_stop == right.afterburner_stop;
}

bool operator==(const multi_local_options& left, const multi_local_options& right) {
	return left.flags == right.flags && left.obj_update_level == right.obj_update_level;
}

bool operator==(const multi_server_options& left, const multi_server_options& right) {
	return left.squad_set == right.squad_set && left.endgame_set == right.endgame_set && left.flags == right.flags
		&& left.respawn == right.respawn && left.max_observers == right.max_observers
		&& left.skill_level == right.skill_level && left.voice_qos == right.voice_qos
		&& left.voice_token_wait == right.voice_token_wait && left.voice_record_time == right.voice_record_time
		&& left.mission_time_limit == right.mission_time_limit && left.kill_limit == right.kill_limit;
}

bool operator==(const sexp_variable& left, const sexp_variable& right) {
	return left.type == right.type && !strcmp(left.text, right.text)
		&& !strcmp(left.variable_name, right.variable_name);
}

bool operator==(const player& lhs, const player& rhs) {
	return !strcmp(lhs.callsign, rhs.callsign) && !strcmp(lhs.short_callsign, rhs.short_callsign)
		&& lhs.short_callsign_width == rhs.short_callsign_width && !strcmp(lhs.image_filename, rhs.image_filename)
		&& !strcmp(lhs.s_squad_filename, rhs.s_squad_filename) && !strcmp(lhs.s_squad_name, rhs.s_squad_name)
		&& !strcmp(lhs.m_squad_filename, rhs.m_squad_filename) && !strcmp(lhs.m_squad_name, rhs.m_squad_name)
		&& !strcmp(lhs.current_campaign, rhs.current_campaign) && lhs.readyroom_listing_mode == rhs.readyroom_listing_mode
		&& lhs.flags == rhs.flags && lhs.save_flags == rhs.save_flags && array_compare(lhs.keyed_targets, rhs.keyed_targets)
		&& lhs.current_hotkey_set == rhs.current_hotkey_set && lhs.lead_target_pos == rhs.lead_target_pos
		&& lhs.lead_target_cheat == rhs.lead_target_cheat && lhs.lead_indicator_active == rhs.lead_indicator_active
		&& lhs.lock_indicator_x == rhs.lock_indicator_x && lhs.lock_indicator_y == rhs.lock_indicator_y
		&& lhs.lock_indicator_start_x == rhs.lock_indicator_start_x
		&& lhs.lock_indicator_start_y == rhs.lock_indicator_start_y
		&& lhs.lock_indicator_visible == rhs.lock_indicator_visible
		&& lhs.lock_time_to_target == rhs.lock_time_to_target && lhs.lock_dist_to_target == rhs.lock_dist_to_target
		&& lhs.last_ship_flown_si_index == rhs.last_ship_flown_si_index && lhs.objnum == rhs.objnum && lhs.bi == rhs.bi
		&& lhs.ci == rhs.ci && lhs.stats == rhs.stats && lhs.friendly_hits == rhs.friendly_hits
		&& lhs.friendly_damage == rhs.friendly_damage && lhs.friendly_last_hit_time == rhs.friendly_last_hit_time
		&& lhs.last_warning_message_time == rhs.last_warning_message_time && lhs.control_mode == rhs.control_mode
		&& lhs.saved_viewer_mode == rhs.saved_viewer_mode && lhs.check_warn_timestamp == rhs.check_warn_timestamp
		&& lhs.distance_warning_count == rhs.distance_warning_count
		&& lhs.distance_warning_time == rhs.distance_warning_time
		&& lhs.allow_warn_timestamp == rhs.allow_warn_timestamp && lhs.warn_count == rhs.warn_count
		&& lhs.damage_this_burst == rhs.damage_this_burst && lhs.repair_sound_loop == rhs.repair_sound_loop
		&& lhs.cargo_scan_loop == rhs.cargo_scan_loop && lhs.praise_count == rhs.praise_count
		&& lhs.allow_praise_timestamp == rhs.allow_praise_timestamp
		&& lhs.praise_delay_timestamp == rhs.praise_delay_timestamp && lhs.ask_help_count == rhs.ask_help_count
		&& lhs.allow_ask_help_timestamp == rhs.allow_ask_help_timestamp && lhs.scream_count == rhs.scream_count
		&& lhs.allow_scream_timestamp == rhs.allow_scream_timestamp
		&& lhs.low_ammo_complaint_count == rhs.low_ammo_complaint_count
		&& lhs.allow_ammo_timestamp == rhs.allow_ammo_timestamp && lhs.praise_self_count == rhs.praise_self_count
		&& lhs.praise_self_timestamp == rhs.praise_self_timestamp && lhs.subsys_in_view == rhs.subsys_in_view
		&& lhs.request_repair_timestamp == rhs.request_repair_timestamp
		&& lhs.cargo_inspect_time == rhs.cargo_inspect_time && lhs.target_is_dying == rhs.target_is_dying
		&& lhs.current_target_sx == rhs.current_target_sx && lhs.current_target_sy == rhs.current_target_sy
		&& lhs.target_in_lock_cone == rhs.target_in_lock_cone && lhs.locking_subsys == rhs.locking_subsys
		&& lhs.locking_subsys_parent == rhs.locking_subsys_parent && lhs.locking_on_center == rhs.locking_on_center
		&& lhs.killer_objtype == rhs.killer_objtype && lhs.killer_species == rhs.killer_species
		&& lhs.killer_weapon_index == rhs.killer_weapon_index && !strcmp(lhs.killer_parent_name, rhs.killer_parent_name)
		&& lhs.check_for_all_alone_msg == rhs.check_for_all_alone_msg
		&& lhs.update_dumbfire_time == rhs.update_dumbfire_time && lhs.update_lock_time == rhs.update_lock_time
		&& lhs.threat_flags == rhs.threat_flags && lhs.auto_advance == rhs.auto_advance
		&& lhs.m_local_options == rhs.m_local_options && lhs.m_server_options == rhs.m_server_options
		&& lhs.insignia_texture == rhs.insignia_texture && lhs.tips == rhs.tips
		&& lhs.shield_penalty_stamp == rhs.shield_penalty_stamp
		&& lhs.failures_this_session == rhs.failures_this_session && lhs.show_skip_popup == rhs.show_skip_popup
		&& lhs.variables == rhs.variables && lhs.containers == rhs.containers && lhs.death_message == rhs.death_message
		&& lhs.lua_ci == rhs.lua_ci && lhs.lua_bi == rhs.lua_bi && lhs.lua_bi_full == rhs.lua_bi_full
		&& lhs.player_was_multi == rhs.player_was_multi && !strcmp(lhs.language, rhs.language);
}
bool operator!=(const player& lhs, const player& rhs) {
	return !(rhs == lhs);
}
