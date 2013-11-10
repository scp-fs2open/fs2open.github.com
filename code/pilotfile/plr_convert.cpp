
/* WARNING:
 *    This is magic-number central, but these numbers are set specifically
 *    to the acceptable defaults or range values that were used when the
 *    pl2/plr files were created.  Standard game defines should not be used in
 *    place of these major numbers for /any/ reason, *ever*!
 */

#include "pilotfile/pilotfile_convert.h"
#include "network/psnet2.h"
#include "cfile/cfilesystem.h"
#include "mission/missionbriefcommon.h"


// this struct isn't well packed, and is written whole to the pilot file, so
// we can't easily just get the RGBA that we need and instead must retain this
typedef struct conv_color { //-V802
	uint		screen_sig;
	ubyte		red;
	ubyte		green;
	ubyte		blue;
	ubyte		alpha;
	ubyte		ac_type;
	int		is_alphacolor;
	ubyte		raw8;
	int		alphacolor;
	int		magic;
} conv_color;


plr_data::plr_data()
{
	// not carried over, just for reference during conversion process
	version = 0;
	is_multi = 0;


	// basic flags and settings
	tips = 0;
	rank = 0;
	skill_level = 1;
	save_flags = 0;
	readyroom_listing_mode = 0;
	voice_enabled = 1;
	auto_advance = 1;
	Use_mouse_to_fly = 0;
	Mouse_sensitivity = 4;
	Joy_sensitivity = 9;
	Dead_zone_size = 10;

	// multiplayer settings/options
	net_protocol = 1;

	multi_squad_set = 2;
	multi_endgame_set = 3;
	multi_flags = 3;
	multi_respawn = 2;
	multi_max_observers = 2;
	multi_skill_level = 2;
	multi_voice_qos = 10;
	multi_voice_token_wait = 2000;
	multi_voice_record_time = 5000;
	multi_time_limit = -65536;
	multi_kill_limit = 9999;

	multi_local_flags = 5;
	multi_local_update_level = 0;

	// pilot info stuff
	memset(image_filename, 0, sizeof(image_filename));
	memset(squad_name, 0, sizeof(squad_name));
	memset(squad_filename, 0, sizeof(squad_filename));
	memset(current_campaign, 0, sizeof(current_campaign));
	memset(last_ship_flown, 0, sizeof(last_ship_flown));

	// HUD config
	hud_show_flags = -1;
	hud_show_flags2 = 31;
	hud_popup_flags = 0;
	hud_popup_flags2 = 0;
	hud_num_lines = 4;
	hud_rp_flags = 7;
	hud_rp_dist = 2;

	for (int idx = 0; idx < 39; idx++) {
		hud_colors[idx][0] = 0;
		hud_colors[idx][1] = 255;
		hud_colors[idx][2] = 0;
		hud_colors[idx][3] = 144;
	}

	// control setup
	joy_axis_map_to[0] = 0;
	joy_axis_map_to[1] = 1;
	joy_axis_map_to[2] = 3;
	joy_invert_axis[3] = -1;
	joy_invert_axis[4] = -1;

	memset(joy_invert_axis, 0, sizeof(joy_invert_axis));

	// audio
	sound_volume = 1.0f;
	music_volume = 0.5f;
	voice_volume = 0.7f;

	// detail settings
	detail_setting = 3;
	detail_nebula = 3;
	detail_distance = 3;
	detail_hardware_textures = 4;
	detail_num_debris = 4;
	detail_num_particles = 3;
	detail_num_stars = 4;
	detail_shield_effects = 4;
	detail_lighting = 4;
	detail_targetview_model = 1;
	detail_planets_suns = 1;
	detail_weapon_extras = 1;
}

plr_data::~plr_data()
{
	controls.clear();
}

void pilotfile_convert::plr_import_controls()
{
	int idx;
	config_item con;

	unsigned char num_controls = cfread_ubyte(cfp);

	if ( !num_controls ) {
		return;
	}

	// it may be less than 118, but it shouldn't be more than 118
	if (num_controls > 118) {
		throw "Data check failure in controls!";
	}

	plr->controls.reserve(num_controls);

	for (idx = 0; idx < num_controls; idx++) {
		con.key_id = cfread_short(cfp);

		if (con.key_id == 255) {
			con.key_id = -1;
		}

		con.joy_id = cfread_short(cfp);

		if (con.joy_id == 255) {
			con.joy_id = -1;
		}

		plr->controls.push_back( con );
	}
}

void pilotfile_convert::plr_import_hud()
{
	int idx;
	conv_color c;

	plr->hud_show_flags = cfread_int(cfp);
	plr->hud_show_flags2 = cfread_int(cfp);

	plr->hud_popup_flags = cfread_int(cfp);
	plr->hud_popup_flags2 = cfread_int(cfp);

	plr->hud_num_lines = cfread_ubyte(cfp);
	plr->hud_rp_flags = cfread_int(cfp);
	plr->hud_rp_dist = cfread_int(cfp);

	for (idx = 0; idx < 39; idx++) {
		cfread(&c, sizeof(conv_color), 1, cfp);

		if ( (c.alphacolor != -1) || (c.is_alphacolor != 1) ) {
			throw "Data check failure in hud!";
		}

		plr->hud_colors[idx][0] = c.red;
		plr->hud_colors[idx][1] = c.green;
		plr->hud_colors[idx][2] = c.blue;
		plr->hud_colors[idx][3] = c.alpha;
	}
}

void pilotfile_convert::plr_import_detail()
{
	bool data_failure = false;

	plr->detail_setting = cfread_int(cfp);
	plr->detail_nebula = cfread_int(cfp);
	plr->detail_distance = cfread_int(cfp);
	plr->detail_hardware_textures = cfread_int(cfp);
	plr->detail_num_debris = cfread_int(cfp);
	plr->detail_num_particles = cfread_int(cfp);
	plr->detail_num_stars = cfread_int(cfp);
	plr->detail_shield_effects = cfread_int(cfp);
	plr->detail_lighting = cfread_int(cfp);
	plr->detail_targetview_model = cfread_int(cfp);
	plr->detail_planets_suns = cfread_int(cfp);
	plr->detail_weapon_extras = cfread_int(cfp);

	if ( (plr->detail_setting < -1) || (plr->detail_setting > 4) ) {
		data_failure = true;
	} else 	if ( (plr->detail_nebula < 0) || (plr->detail_nebula > 4) ) {
		data_failure = true;
	} else 	if ( (plr->detail_distance < 0) || (plr->detail_distance > 4) ) {
		data_failure = true;
	} else 	if ( (plr->detail_hardware_textures < 0) || (plr->detail_hardware_textures > 4) ) {
		data_failure = true;
	} else 	if ( (plr->detail_num_debris < 0) || (plr->detail_num_debris > 4) ) {
		data_failure = true;
	} else 	if ( (plr->detail_num_particles < 0) || (plr->detail_num_particles > 4) ) {
		data_failure = true;
	} else 	if ( (plr->detail_num_stars < 0) || (plr->detail_num_stars > 4) ) {
		data_failure = true;
	} else 	if ( (plr->detail_shield_effects < 0) || (plr->detail_shield_effects > 4) ) {
		data_failure = true;
	} else 	if ( (plr->detail_lighting < 0) || (plr->detail_lighting > 4) ) {
		data_failure = true;
	} else 	if ( (plr->detail_targetview_model < 0) || (plr->detail_targetview_model > 1) ) {
		data_failure = true;
	} else 	if ( (plr->detail_planets_suns < 0) || (plr->detail_planets_suns > 1) ) {
		data_failure = true;
	} else 	if ( (plr->detail_weapon_extras < 0) || (plr->detail_weapon_extras > 1) ) {
		data_failure = true;
	}

	if (data_failure) {
		throw "Data check failure in details!";
	}
}

void pilotfile_convert::plr_import_stats()
{
	int idx;
	char name[35];

	if (fver >= 242) {
		return;
	}

	// read everything, but we don't need any of it ...

	cfread_int(cfp);	// score
	cfread_int(cfp);	// rank
	cfread_int(cfp);	// assists

	// medals
	for (idx = 0; idx < 18; idx++) {
		cfread_int(cfp);
	}

	// kills per ship
	int count = cfread_int(cfp);

	for (idx = 0; idx < count; idx++) {
		cfread_ushort(cfp);
		cfread_string_len(name, sizeof(name), cfp);
	}

	cfread_int(cfp);	// kill_count
	cfread_int(cfp);	// kill_count_ok

	cfread_uint(cfp);	// p_shots_fired
	cfread_uint(cfp);	// s_shots_fired
	cfread_uint(cfp);	// p_shots_hit
	cfread_uint(cfp);	// s_shots_hit

	cfread_uint(cfp);	// p_bonehead_hits
	cfread_uint(cfp);	// s_bonehead_hits
	cfread_uint(cfp);	// bonehead_kills
}

void pilotfile_convert::plr_import_loadout()
{
	int idx, j;
	int s_count, w_count;
	char name[52];

	if (fver >= 242) {
		return;
	}

	// have to read it, but don't need any of it ...

	cfread_string_len(name, sizeof(name), cfp);	// filename
	cfread_string_len(name, sizeof(name), cfp);	// last_modified

	s_count = cfread_int(cfp);	// num ships
	w_count = cfread_int(cfp);	// num weapons

	// ships
	for (idx = 0; idx < s_count; idx++) {
		cfread_int(cfp);	// count
		cfread_string_len(name, sizeof(name), cfp);	// name
	}

	// weapons
	for (idx = 0; idx < w_count; idx++) {
		cfread_int(cfp);	// count
		cfread_string_len(name, sizeof(name), cfp);	// name
	}

	// loadout info
	for (idx = 0; idx < 12; idx++) {
		cfread_int(cfp);	// ship class
		cfread_string_len(name, sizeof(name), cfp);	// ship name

		for (j = 0; j < 12; j++) {
			cfread_int(cfp);	// weapon type
			cfread_int(cfp);	// weapon count
			cfread_string_len(name, sizeof(name), cfp);	// weapon name
		}
	}
}

void pilotfile_convert::plr_import_multiplayer()
{
	plr->multi_squad_set = cfread_ubyte(cfp);
	plr->multi_endgame_set = cfread_ubyte(cfp);
	plr->multi_flags = cfread_int(cfp);
	plr->multi_respawn = cfread_uint(cfp);
	plr->multi_max_observers = cfread_ubyte(cfp);
	plr->multi_skill_level = cfread_ubyte(cfp);
	plr->multi_voice_qos = cfread_ubyte(cfp);
	plr->multi_voice_token_wait = cfread_int(cfp);
	plr->multi_voice_record_time = cfread_int(cfp);
	plr->multi_time_limit = cfread_int(cfp);
	plr->multi_kill_limit = cfread_int(cfp);

	plr->multi_local_flags = cfread_int(cfp);
	plr->multi_local_update_level = cfread_int(cfp);
}

void pilotfile_convert::plr_import_red_alert()
{
	int idx, j;
	char name[35];

	if (fver >= 242) {
		return;
	}

	// have to read it, but don't need any of it ...

	int num_slots = cfread_int(cfp);

	if ( (num_slots < 0) || (num_slots >= 32) ) {
		throw "Data check failure in red-alert!";
	}

	if ( !num_slots ) {
		return;
	}

	for (idx = 0; idx < num_slots; idx++) {
		cfread_string(name, sizeof(name) - 1, cfp);
		cfread_float(cfp);

		cfread_string_len(name, sizeof(name), cfp);

		// subsystem hits
		for (j = 0; j < 64; j++) {
			cfread_float(cfp);
		}

		// aggregate hits
		for (j = 0; j < 12; j++) {
			cfread_float(cfp);
		}

		// weapons
		for (j = 0; j < 12; j++) {
			cfread_string_len(name, sizeof(name), cfp);
			cfread_int(cfp);
		}
	}
}

void pilotfile_convert::plr_import_variables()
{
	int idx;
	sexp_variable nvar;

	int num_variables = cfread_int(cfp);

	if ( (num_variables < 0) || (num_variables >= 100) ) {
		throw "Data check failure in variables!";
	}

	plr->variables.reserve(num_variables);

	for (idx = 0; idx < num_variables; idx++) {
		nvar.type = cfread_int(cfp);
		cfread_string_len(nvar.text, sizeof(nvar.text), cfp);
		cfread_string_len(nvar.variable_name, sizeof(nvar.variable_name), cfp);

		plr->variables.push_back( nvar );
	}
}

void pilotfile_convert::plr_import()
{
	char name[35];
	int idx;

	unsigned int plr_id = cfread_uint(cfp);

	if (plr_id != 0x46505346) {
		throw "Invalid file signature!";
	}

	fver = cfread_uint(cfp);

	if ( (fver != 142) && (fver != 242) ) {
		throw "Unsupported file version!";
	}

	// multi flag
	plr->is_multi = (int)cfread_ubyte(cfp);

	// rank
	plr->rank = cfread_int(cfp);

	// mainhall, don't need it
	if (fver < 242) {
		cfread_ubyte(cfp);
	}

	plr->tips = cfread_int(cfp);

	if ( (plr->tips < 0) || (plr->tips > 1) ) {
		throw "Data check failure!";
	}

	cfread_string_len(plr->image_filename, sizeof(plr->image_filename), cfp);
	cfread_string_len(plr->squad_name, sizeof(plr->squad_name), cfp);
	cfread_string_len(plr->squad_filename, sizeof(plr->squad_filename), cfp);
	cfread_string_len(plr->current_campaign, sizeof(plr->current_campaign), cfp);
	cfread_string_len(plr->last_ship_flown, sizeof(plr->last_ship_flown), cfp);

	// controls
	plr_import_controls();

	// hud config
	plr_import_hud();

	// cutscenes, don't need it
	if (fver < 242) {
		cfread_int(cfp);
	}

	// volume stuff
	plr->sound_volume = cfread_float(cfp);
	plr->music_volume = cfread_float(cfp);
	plr->voice_volume = cfread_float(cfp);

	// detail settings
	plr_import_detail();

	// recent missions, don't need it
	int num_missions = cfread_int(cfp);

	for (idx = 0; idx < num_missions; idx++) {
		cfread_string_len(name, sizeof(name), cfp);
	}

	// stats, will skip if fver < 242
	plr_import_stats();

	plr->skill_level = cfread_int(cfp);

	if ( (plr->skill_level < 0) || (plr->skill_level > 4) ) {
		throw "Data check failure!";
	}

	// extra joystick stuff
	for (idx = 0; idx < 5; idx++) {
		plr->joy_axis_map_to[idx] = cfread_int(cfp);
		plr->joy_invert_axis[idx] = cfread_int(cfp);
	}

	// flags
	plr->save_flags = cfread_int(cfp);

	// loadout, will skip if fver < 242
	plr_import_loadout();

	// multiplayer
	plr_import_multiplayer();

	// two briefing related values
	plr->readyroom_listing_mode = cfread_int(cfp);
	Briefing_voice_enabled = cfread_int(cfp);

	plr->net_protocol = cfread_int(cfp);

	// protocol must be set to something
	if (plr->net_protocol == NET_NONE) {
		plr->net_protocol = NET_TCP;
	} else if ( (plr->net_protocol < 0) || (plr->net_protocol > NET_VMT) ) {
		throw "Data check failure!";
	}

	// red alert status, will skip if fver < 242 (and should be empty if multi)
	plr_import_red_alert();

	// briefing auto-advance
	plr->auto_advance = cfread_int(cfp);

	if ( (plr->auto_advance < 0) || (plr->auto_advance > 1) ) {
		throw "Data check failure!";
	}

	// some input options
	plr->Use_mouse_to_fly = cfread_int(cfp);
	plr->Mouse_sensitivity = cfread_int(cfp);
	plr->Joy_sensitivity = cfread_int(cfp);
	plr->Dead_zone_size = cfread_int(cfp);

	// variables
	plr_import_variables();


	// and... we're done! :)
}

void pilotfile_convert::plr_export_flags()
{
	startSection(Section::Flags);

	// tips
	cfwrite_ubyte((unsigned char)plr->tips, cfp);

	// saved flags
	cfwrite_int(plr->save_flags, cfp);

	// listing mode (single or campaign missions)
	cfwrite_int(plr->readyroom_listing_mode, cfp);

	// briefing auto-play
	cfwrite_int(plr->auto_advance, cfp);

	// special rank setting (to avoid having to read all stats on verify)
	cfwrite_int(plr->rank, cfp);

	// What game mode we were in last on this pilot
	cfwrite_int(plr->is_multi, cfp);

	endSection();
}

void pilotfile_convert::plr_export_info()
{
	startSection(Section::Info);

	// pilot image
	cfwrite_string_len(plr->image_filename, cfp);

	// squad name
	cfwrite_string_len(plr->squad_name, cfp);

	// squad image
	cfwrite_string_len(plr->squad_filename, cfp);

	// active campaign
	cfwrite_string_len(plr->current_campaign, cfp);

	endSection();
}

void pilotfile_convert::plr_export_stats()
{
	int idx, list_size = 0;

	startSection(Section::Scoring);

	// global, all-time stats
	cfwrite_int(all_time_stats.score, cfp);
	cfwrite_int(all_time_stats.rank, cfp);
	cfwrite_int(all_time_stats.assists, cfp);
	cfwrite_int(all_time_stats.kill_count, cfp);
	cfwrite_int(all_time_stats.kill_count_ok, cfp);
	cfwrite_int(all_time_stats.bonehead_kills, cfp);

	cfwrite_uint(all_time_stats.p_shots_fired, cfp);
	cfwrite_uint(all_time_stats.p_shots_hit, cfp);
	cfwrite_uint(all_time_stats.p_bonehead_hits, cfp);

	cfwrite_uint(all_time_stats.s_shots_fired, cfp);
	cfwrite_uint(all_time_stats.s_shots_hit, cfp);
	cfwrite_uint(all_time_stats.s_bonehead_hits, cfp);

	cfwrite_uint(all_time_stats.flight_time, cfp);
	cfwrite_uint(all_time_stats.missions_flown, cfp);
	cfwrite_int((int)all_time_stats.last_flown, cfp);
	cfwrite_int((int)all_time_stats.last_backup, cfp);

	// ship kills (contains ships across all mods, not just current)
	list_size = (int)all_time_stats.ship_kills.size();
	cfwrite_int(list_size, cfp);

	for (idx = 0; idx < list_size; idx++) {
		cfwrite_string_len(all_time_stats.ship_kills[idx].name.c_str(), cfp);
		cfwrite_int(all_time_stats.ship_kills[idx].val, cfp);
	}

	// medals earned (contains medals across all mods, not just current)
	list_size = (int)all_time_stats.medals_earned.size();
	cfwrite_int(list_size, cfp);

	for (idx = 0; idx < list_size; idx++) {
		cfwrite_string_len(all_time_stats.medals_earned[idx].name.c_str(), cfp);
		cfwrite_int(all_time_stats.medals_earned[idx].val, cfp);
	}

	endSection();
}

void pilotfile_convert::plr_export_stats_multi()
{
	int idx, list_size = 0;

	startSection(Section::ScoringMulti);

	// global, all-time stats
	cfwrite_int(multi_stats.score, cfp);
	cfwrite_int(multi_stats.rank, cfp);
	cfwrite_int(multi_stats.assists, cfp);
	cfwrite_int(multi_stats.kill_count, cfp);
	cfwrite_int(multi_stats.kill_count_ok, cfp);
	cfwrite_int(multi_stats.bonehead_kills, cfp);

	cfwrite_uint(multi_stats.p_shots_fired, cfp);
	cfwrite_uint(multi_stats.p_shots_hit, cfp);
	cfwrite_uint(multi_stats.p_bonehead_hits, cfp);

	cfwrite_uint(multi_stats.s_shots_fired, cfp);
	cfwrite_uint(multi_stats.s_shots_hit, cfp);
	cfwrite_uint(multi_stats.s_bonehead_hits, cfp);

	cfwrite_uint(multi_stats.flight_time, cfp);
	cfwrite_uint(multi_stats.missions_flown, cfp);
	cfwrite_int((int)multi_stats.last_flown, cfp);
	cfwrite_int((int)multi_stats.last_backup, cfp);

	// ship kills (contains medals across all mods, not just current)
	list_size = (int)multi_stats.ship_kills.size();
	cfwrite_int(list_size, cfp);

	for (idx = 0; idx < list_size; idx++) {
		cfwrite_string_len(multi_stats.ship_kills[idx].name.c_str(), cfp);
		cfwrite_int(multi_stats.ship_kills[idx].val, cfp);
	}

	// medals earned (contains medals across all mods, not just current)
	list_size = (int)multi_stats.medals_earned.size();
	cfwrite_int(list_size, cfp);

	for (idx = 0; idx < list_size; idx++) {
		cfwrite_string_len(multi_stats.medals_earned[idx].name.c_str(), cfp);
		cfwrite_int(multi_stats.medals_earned[idx].val, cfp);
	}

	endSection();
}

void pilotfile_convert::plr_export_hud()
{
	int idx;

	startSection(Section::HUD);

	// flags
	cfwrite_int(plr->hud_show_flags, cfp);
	cfwrite_int(plr->hud_show_flags2, cfp);

	cfwrite_int(plr->hud_popup_flags, cfp);
	cfwrite_int(plr->hud_popup_flags2, cfp);

	// settings
	cfwrite_ubyte(plr->hud_num_lines, cfp);

	cfwrite_int(plr->hud_rp_flags, cfp);
	cfwrite_int(plr->hud_rp_dist, cfp);

	// basic colors
	cfwrite_int(0, cfp);	// color
	cfwrite_int(8, cfp);	// alpha

	// gauge-specific colors
	cfwrite_int(39, cfp);

	for (idx = 0; idx < 39; idx++) {
		cfwrite_ubyte(plr->hud_colors[idx][0], cfp);
		cfwrite_ubyte(plr->hud_colors[idx][1], cfp);
		cfwrite_ubyte(plr->hud_colors[idx][2], cfp);
		cfwrite_ubyte(plr->hud_colors[idx][3], cfp);
	}

	endSection();
}

void pilotfile_convert::plr_export_variables()
{
	int list_size = 0;
	int idx;

	startSection(Section::Variables);

	list_size = (int)plr->variables.size();

	cfwrite_int(list_size, cfp);

	for (idx = 0; idx < list_size; idx++) {
		cfwrite_int(plr->variables[idx].type, cfp);
		cfwrite_string_len(plr->variables[idx].text, cfp);
		cfwrite_string_len(plr->variables[idx].variable_name, cfp);
	}

	endSection();
}

void pilotfile_convert::plr_export_multiplayer()
{
	startSection(Section::Multiplayer);

	// netgame options
	cfwrite_ubyte(plr->multi_squad_set, cfp);
	cfwrite_ubyte(plr->multi_endgame_set, cfp);
	cfwrite_int(plr->multi_flags, cfp);
	cfwrite_uint(plr->multi_respawn, cfp);
	cfwrite_ubyte(plr->multi_max_observers, cfp);
	cfwrite_ubyte(plr->multi_skill_level, cfp);
	cfwrite_ubyte(plr->multi_voice_qos, cfp);
	cfwrite_int(plr->multi_voice_token_wait, cfp);
	cfwrite_int(plr->multi_voice_record_time, cfp);
	cfwrite_int(plr->multi_time_limit, cfp);
	cfwrite_int(plr->multi_kill_limit, cfp);

	// local options
	cfwrite_int(plr->multi_local_flags, cfp);
	cfwrite_int(plr->multi_local_update_level, cfp);

	// netgame protocol
	cfwrite_int(plr->net_protocol, cfp);

	endSection();
}

void pilotfile_convert::plr_export_controls()
{
	unsigned int idx;

	startSection(Section::Controls);

	cfwrite_ushort((unsigned short)plr->controls.size(), cfp);

	for (idx = 0; idx < plr->controls.size(); idx++) {
		cfwrite_short(plr->controls[idx].key_id, cfp);
		cfwrite_short(plr->controls[idx].joy_id, cfp);
		// placeholder? for future mouse_id?
		cfwrite_short(-1, cfp);
	}

	// extra joystick stuff
	cfwrite_int(MAX_JOY_AXES_CONV, cfp);
	for (idx = 0; idx < MAX_JOY_AXES_CONV; idx++) {
		cfwrite_int(plr->joy_axis_map_to[idx], cfp);
		cfwrite_int(plr->joy_invert_axis[idx], cfp);
	}

	endSection();
}

void pilotfile_convert::plr_export_settings()
{
	startSection(Section::Settings);

	// sound/voice/music
	cfwrite_float(plr->sound_volume, cfp);
	cfwrite_float(plr->music_volume, cfp);
	cfwrite_float(plr->voice_volume, cfp);

	cfwrite_int(plr->voice_enabled, cfp);

	// skill level
	cfwrite_int(plr->skill_level, cfp);

	// input options
	cfwrite_int(plr->Use_mouse_to_fly, cfp);
	cfwrite_int(plr->Mouse_sensitivity, cfp);
	cfwrite_int(plr->Joy_sensitivity, cfp);
	cfwrite_int(plr->Dead_zone_size, cfp);

	// detail
	cfwrite_int(plr->detail_setting, cfp);
	cfwrite_int(plr->detail_nebula, cfp);
	cfwrite_int(plr->detail_distance, cfp);
	cfwrite_int(plr->detail_hardware_textures, cfp);
	cfwrite_int(plr->detail_num_debris, cfp);
	cfwrite_int(plr->detail_num_particles, cfp);
	cfwrite_int(plr->detail_num_stars, cfp);
	cfwrite_int(plr->detail_shield_effects, cfp);
	cfwrite_int(plr->detail_lighting, cfp);
	cfwrite_int(plr->detail_targetview_model, cfp);
	cfwrite_int(plr->detail_planets_suns, cfp);
	cfwrite_int(plr->detail_weapon_extras, cfp);

	endSection();
}

void pilotfile_convert::plr_export()
{
	Assert( cfp != NULL );

	// header and version
	cfwrite_int(PLR_FILE_ID, cfp);
	cfwrite_ubyte(PLR_VERSION, cfp);

	// flags and info sections go first
	plr_export_flags();
	plr_export_info();

	// everything else is next, not order specific
	plr_export_stats();
	plr_export_stats_multi();
	plr_export_hud();
	plr_export_variables();
	plr_export_multiplayer();
	plr_export_controls();
	plr_export_settings();


	// and... we're done! :)
}

bool pilotfile_convert::plr_convert(const char *fname, bool inferno)
{
	Assert( fname != NULL );

	SCP_string filename;
	bool rval = true;


	if (plr == NULL) {
		plr = new(std::nothrow) plr_data;
	}

	if (plr == NULL) {
		return false;
	}

	filename.reserve(200);

	cf_create_default_path_string(filename, CF_TYPE_SINGLE_PLAYERS, (inferno) ? const_cast<char*>("inferno") : NULL);

	if (inferno) {
		filename.append(DIR_SEPARATOR_STR);
	}

	filename.append(fname);
	filename.append(".pl2");

	mprintf(("  PL2 => Converting '%s'...\n", filename.c_str()));

	cfp = cfopen(const_cast<char*>(filename.c_str()), "rb", CFILE_NORMAL);

	if ( !cfp ) {
		mprintf(("  PL2 => Unable to open for import!\n", fname));
		return false;
	}

	try {
		plr_import();
	} catch (const char *err) {
		mprintf((  "  PL2 => Import ERROR: %s\n", err));
		rval = false;
	}

	cfclose(cfp);
	cfp = NULL;

	if ( !rval ) {
		return false;
	}

	filename.assign(fname);
	filename.append(".plr");

	cfp = cfopen(const_cast<char*>(filename.c_str()), "wb", CFILE_NORMAL, CF_TYPE_PLAYERS);

	if ( !cfp ) {
		mprintf(("  PLR => Unable to open for export!\n", fname));
		return false;
	}

	try {
		plr_export();
	} catch (const char *err) {
		mprintf(("  PLR => Export ERROR: %s\n", err));
		rval = false;
	}

	cfclose(cfp);
	cfp = NULL;

	if (rval) {
		mprintf(("  PLR => Conversion complete!\n"));
	}

	return rval;
}

