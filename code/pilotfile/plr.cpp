
#include "globalincs/pstypes.h"
#include "pilotfile/pilotfile.h"
#include "playerman/player.h"
#include "ship/ship.h"
#include "menuui/techmenu.h"
#include "weapon/weapon.h"
#include "hud/hudconfig.h"
#include "stats/medals.h"
#include "hud/hudsquadmsg.h"
#include "gamesnd/eventmusic.h"
#include "osapi/osregistry.h"
#include "sound/audiostr.h"
#include "io/joy.h"
#include "io/mouse.h"
#include "network/multi.h"
#include "freespace2/freespace.h"
#include "playerman/managepilot.h"
#include "localization/localize.h"


void pilotfile::plr_read_flags()
{
	// tips?
	p->tips = (int)cfread_ubyte(cfp);

	// saved flags
	p->save_flags = cfread_int(cfp);

	// listing mode (single or campaign missions
	p->readyroom_listing_mode = cfread_int(cfp);

	// briefing auto-play
	p->auto_advance = cfread_int(cfp);

	// special rank setting (to avoid having to read all stats on verify)
	// will be the multi rank
	// if there's a valid CSG, this will be overwritten
	p->stats.rank = cfread_int(cfp);

	if (version > 0) 
	{
		p->player_was_multi = cfread_int(cfp);
	} else 
	{
		p->player_was_multi = 0; // Default to single player
	}

	// which language was this pilot created with
	if (version > 1) {
		cfread_string_len(p->language, sizeof(p->language), cfp);
	} else {
		// if we don't know, default to the current language setting
		lcl_get_language_name(p->language);
	}
}

void pilotfile::plr_write_flags()
{
	startSection(Section::Flags);

	// tips
	cfwrite_ubyte((unsigned char)p->tips, cfp);

	// saved flags
	cfwrite_int(p->save_flags, cfp);

	// listing mode (single or campaign missions)
	cfwrite_int(p->readyroom_listing_mode, cfp);

	// briefing auto-play
	cfwrite_int(p->auto_advance, cfp);

	// special rank setting (to avoid having to read all stats on verify)
	// should be multi only from now on
	cfwrite_int(multi_stats.rank, cfp);

	// What game mode we were in last on this pilot
	cfwrite_int(p->player_was_multi, cfp);

	// which language was this pilot created with
	cfwrite_string_len(p->language, cfp);

	endSection();
}

void pilotfile::plr_read_info()
{
	if ( !m_have_flags ) {
		throw "Info before Flags!";
	}

	// pilot image
	cfread_string_len(p->image_filename, MAX_FILENAME_LEN, cfp);

	// multi squad name
	cfread_string_len(p->m_squad_name, NAME_LENGTH, cfp);

	// squad image
	cfread_string_len(p->m_squad_filename, MAX_FILENAME_LEN, cfp);

	// active campaign
	cfread_string_len(p->current_campaign, MAX_FILENAME_LEN, cfp);
}

void pilotfile::plr_write_info()
{
	startSection(Section::Info);

	// pilot image
	cfwrite_string_len(p->image_filename, cfp);

	// multi squad name
	cfwrite_string_len(p->m_squad_name, cfp);

	// squad image
	cfwrite_string_len(p->m_squad_filename, cfp);

	// active campaign
	cfwrite_string_len(p->current_campaign, cfp);

	endSection();
}

void pilotfile::plr_read_hud()
{
	int idx;

	// flags
	HUD_config.show_flags = cfread_int(cfp);
	HUD_config.show_flags2 = cfread_int(cfp);

	HUD_config.popup_flags = cfread_int(cfp);
	HUD_config.popup_flags2 = cfread_int(cfp);

	// settings
	HUD_config.num_msg_window_lines = cfread_ubyte(cfp);

	HUD_config.rp_flags = cfread_int(cfp);
	HUD_config.rp_dist = cfread_int(cfp);

	// basic colors
	HUD_config.main_color = cfread_int(cfp);
	HUD_color_alpha = cfread_int(cfp);

	if (HUD_color_alpha < HUD_COLOR_ALPHA_USER_MIN) {
		HUD_color_alpha = HUD_COLOR_ALPHA_DEFAULT;
	}

	hud_config_set_color(HUD_config.main_color);

	// gauge-specific colors
	int num_gauges = cfread_int(cfp);

	for (idx = 0; idx < num_gauges; idx++) {
		ubyte red = cfread_ubyte(cfp);
		ubyte green = cfread_ubyte(cfp);
		ubyte blue = cfread_ubyte(cfp);
		ubyte alpha = cfread_ubyte(cfp);

		if (idx >= NUM_HUD_GAUGES) {
			continue;
		}

		HUD_config.clr[idx].red = red;
		HUD_config.clr[idx].green = green;
		HUD_config.clr[idx].blue = blue;
		HUD_config.clr[idx].alpha = alpha;
	}
}

void pilotfile::plr_write_hud()
{
	int idx;

	startSection(Section::HUD);

	// flags
	cfwrite_int(HUD_config.show_flags, cfp);
	cfwrite_int(HUD_config.show_flags2, cfp);

	cfwrite_int(HUD_config.popup_flags, cfp);
	cfwrite_int(HUD_config.popup_flags2, cfp);

	// settings
	cfwrite_ubyte(HUD_config.num_msg_window_lines, cfp);

	cfwrite_int(HUD_config.rp_flags, cfp);
	cfwrite_int(HUD_config.rp_dist, cfp);

	// basic colors
	cfwrite_int(HUD_config.main_color, cfp);
	cfwrite_int(HUD_color_alpha, cfp);

	// gauge-specific colors
	cfwrite_int(NUM_HUD_GAUGES, cfp);

	for (idx = 0; idx < NUM_HUD_GAUGES; idx++) {
		cfwrite_ubyte(HUD_config.clr[idx].red, cfp);
		cfwrite_ubyte(HUD_config.clr[idx].green, cfp);
		cfwrite_ubyte(HUD_config.clr[idx].blue, cfp);
		cfwrite_ubyte(HUD_config.clr[idx].alpha, cfp);
	}

	endSection();
}

void pilotfile::plr_read_variables()
{
	int list_size = 0;
	int idx;
	sexp_variable n_var;

	list_size = cfread_int(cfp);

	if (list_size <= 0) {
		return;
	}

	p->variables.reserve(list_size);

	for (idx = 0; idx < list_size; idx++) {
		n_var.type = cfread_int(cfp);
		cfread_string_len(n_var.text, TOKEN_LENGTH, cfp);
		cfread_string_len(n_var.variable_name, TOKEN_LENGTH, cfp);

		p->variables.push_back( n_var );
	}
}

void pilotfile::plr_write_variables()
{
	int list_size = 0;
	int idx;

	startSection(Section::Variables);

	list_size = (int)p->variables.size();

	cfwrite_int(list_size, cfp);

	for (idx = 0; idx < list_size; idx++) {
		cfwrite_int(p->variables[idx].type, cfp);
		cfwrite_string_len(p->variables[idx].text, cfp);
		cfwrite_string_len(p->variables[idx].variable_name, cfp);
	}

	endSection();
}

void pilotfile::plr_read_multiplayer()
{
	// netgame options
	p->m_server_options.squad_set = cfread_ubyte(cfp);
	p->m_server_options.endgame_set = cfread_ubyte(cfp);
	p->m_server_options.flags = cfread_int(cfp);
	p->m_server_options.respawn = cfread_uint(cfp);
	p->m_server_options.max_observers = cfread_ubyte(cfp);
	p->m_server_options.skill_level = cfread_ubyte(cfp);
	p->m_server_options.voice_qos = cfread_ubyte(cfp);
	p->m_server_options.voice_token_wait = cfread_int(cfp);
	p->m_server_options.voice_record_time = cfread_int(cfp);
	p->m_server_options.mission_time_limit = (fix)cfread_int(cfp);
	p->m_server_options.kill_limit = cfread_int(cfp);

	// local options
	p->m_local_options.flags = cfread_int(cfp);
	p->m_local_options.obj_update_level = cfread_int(cfp);

	// netgame protocol
	Multi_options_g.protocol = cfread_int(cfp);

	if (Multi_options_g.protocol == NET_VMT) {
		Multi_options_g.protocol = NET_TCP;
	}

	Assert( (Multi_options_g.protocol == NET_IPX) || (Multi_options_g.protocol == NET_TCP) );
}

void pilotfile::plr_write_multiplayer()
{
	startSection(Section::Multiplayer);

	// netgame options
	cfwrite_ubyte(p->m_server_options.squad_set, cfp);
	cfwrite_ubyte(p->m_server_options.endgame_set, cfp);
	cfwrite_int(p->m_server_options.flags, cfp);
	cfwrite_uint(p->m_server_options.respawn, cfp);
	cfwrite_ubyte(p->m_server_options.max_observers, cfp);
	cfwrite_ubyte(p->m_server_options.skill_level, cfp);
	cfwrite_ubyte(p->m_server_options.voice_qos, cfp);
	cfwrite_int(p->m_server_options.voice_token_wait, cfp);
	cfwrite_int(p->m_server_options.voice_record_time, cfp);
	cfwrite_int((int)p->m_server_options.mission_time_limit, cfp);
	cfwrite_int(p->m_server_options.kill_limit, cfp);

	// local options
	cfwrite_int(p->m_local_options.flags, cfp);
	cfwrite_int(p->m_local_options.obj_update_level, cfp);

	// netgame protocol
	cfwrite_int(Multi_options_g.protocol, cfp);

	endSection();
}

void pilotfile::plr_read_stats()
{
	int idx, j, list_size = 0;
	index_list_t ilist;
	char t_string[NAME_LENGTH+1];

	// global, all-time stats (used only until campaign stats are loaded)
	all_time_stats.score = cfread_int(cfp);
	all_time_stats.rank = cfread_int(cfp);
	all_time_stats.assists = cfread_int(cfp);
	all_time_stats.kill_count = cfread_int(cfp);
	all_time_stats.kill_count_ok = cfread_int(cfp);
	all_time_stats.bonehead_kills = cfread_int(cfp);

	all_time_stats.p_shots_fired = cfread_uint(cfp);
	all_time_stats.p_shots_hit = cfread_uint(cfp);
	all_time_stats.p_bonehead_hits = cfread_uint(cfp);

	all_time_stats.s_shots_fired = cfread_uint(cfp);
	all_time_stats.s_shots_hit = cfread_uint(cfp);
	all_time_stats.s_bonehead_hits = cfread_uint(cfp);

	all_time_stats.flight_time = cfread_uint(cfp);
	all_time_stats.missions_flown = cfread_uint(cfp);
	all_time_stats.last_flown = (_fs_time_t)cfread_int(cfp);
	all_time_stats.last_backup = (_fs_time_t)cfread_int(cfp);

	// ship kills (contains ships across all mods, not just current)
	list_size = cfread_int(cfp);
	all_time_stats.ship_kills.reserve(list_size);

	for (idx = 0; idx < list_size; idx++) {
		cfread_string_len(t_string, NAME_LENGTH, cfp);

		ilist.name = t_string;
		ilist.index = ship_info_lookup(t_string);
		ilist.val = cfread_int(cfp);

		all_time_stats.ship_kills.push_back(ilist);
	}

	// medals earned (contains medals across all mods, not just current)
	list_size = cfread_int(cfp);
	all_time_stats.medals_earned.reserve(list_size);

	for (idx = 0; idx < list_size; idx++) {
		cfread_string_len(t_string, NAME_LENGTH,cfp);

		ilist.name = t_string;
		ilist.index = medals_info_lookup(t_string);
		ilist.val = cfread_int(cfp);

		all_time_stats.medals_earned.push_back(ilist);
	}

	// if not in multiplayer mode then set these stats as the player stats
	if ( !(Game_mode & GM_MULTIPLAYER) ) {
		p->stats.score = all_time_stats.score;
		p->stats.rank = all_time_stats.rank;
		p->stats.assists = all_time_stats.assists;
		p->stats.kill_count = all_time_stats.kill_count;
		p->stats.kill_count_ok = all_time_stats.kill_count_ok;
		p->stats.bonehead_kills = all_time_stats.bonehead_kills;

		p->stats.p_shots_fired = all_time_stats.p_shots_fired;
		p->stats.p_shots_hit = all_time_stats.p_shots_hit;
		p->stats.p_bonehead_hits = all_time_stats.p_bonehead_hits;

		p->stats.s_shots_fired = all_time_stats.s_shots_fired;
		p->stats.s_shots_hit = all_time_stats.s_shots_hit;
		p->stats.s_bonehead_hits = all_time_stats.s_bonehead_hits;

		p->stats.flight_time = all_time_stats.flight_time;
		p->stats.missions_flown = all_time_stats.missions_flown;
		p->stats.last_flown = all_time_stats.last_flown;
		p->stats.last_backup = all_time_stats.last_backup;

		// ship kills (have to find ones that match content)
		list_size = (int)all_time_stats.ship_kills.size();
		for (idx = 0; idx < list_size; idx++) {
			j = all_time_stats.ship_kills[idx].index;

			if (j >= 0) {
				p->stats.kills[j] = all_time_stats.ship_kills[idx].val;
			}
		}

		// medals earned (have to find ones that match content)
		list_size = (int)all_time_stats.medals_earned.size();
		for (idx = 0; idx < list_size; idx++) {
			j = all_time_stats.medals_earned[idx].index;

			if (j >= 0) {
				p->stats.medal_counts[j] = all_time_stats.medals_earned[idx].val;
			}
		}
	}
}

void pilotfile::plr_write_stats()
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

void pilotfile::plr_read_stats_multi()
{
	int idx, j, list_size = 0;
	index_list_t ilist;
	char t_string[NAME_LENGTH+1];

	// global, all-time stats (used only until campaign stats are loaded)
	multi_stats.score = cfread_int(cfp);
	multi_stats.rank = cfread_int(cfp);
	multi_stats.assists = cfread_int(cfp);
	multi_stats.kill_count = cfread_int(cfp);
	multi_stats.kill_count_ok = cfread_int(cfp);
	multi_stats.bonehead_kills = cfread_int(cfp);

	multi_stats.p_shots_fired = cfread_uint(cfp);
	multi_stats.p_shots_hit = cfread_uint(cfp);
	multi_stats.p_bonehead_hits = cfread_uint(cfp);

	multi_stats.s_shots_fired = cfread_uint(cfp);
	multi_stats.s_shots_hit = cfread_uint(cfp);
	multi_stats.s_bonehead_hits = cfread_uint(cfp);

	multi_stats.flight_time = cfread_uint(cfp);
	multi_stats.missions_flown = cfread_uint(cfp);
	multi_stats.last_flown = (_fs_time_t)cfread_int(cfp);
	multi_stats.last_backup = (_fs_time_t)cfread_int(cfp);

	// ship kills (contains ships across all mods, not just current)
	list_size = cfread_int(cfp);
	multi_stats.ship_kills.reserve(list_size);

	for (idx = 0; idx < list_size; idx++) {
		cfread_string_len(t_string, NAME_LENGTH, cfp);

		ilist.name = t_string;
		ilist.index = ship_info_lookup(t_string);
		ilist.val = cfread_int(cfp);

		multi_stats.ship_kills.push_back(ilist);
	}

	// medals earned (contains medals across all mods, not just current)
	list_size = cfread_int(cfp);
	multi_stats.medals_earned.reserve(list_size);

	for (idx = 0; idx < list_size; idx++) {
		cfread_string_len(t_string, NAME_LENGTH,cfp);

		ilist.name = t_string;
		ilist.index = medals_info_lookup(t_string);
		ilist.val = cfread_int(cfp);

		multi_stats.medals_earned.push_back(ilist);
	}

	// if in multiplayer mode then set these stats as the player stats
	if (Game_mode & GM_MULTIPLAYER) {
		p->stats.score = multi_stats.score;
		p->stats.rank = multi_stats.rank;
		p->stats.assists = multi_stats.assists;
		p->stats.kill_count = multi_stats.kill_count;
		p->stats.kill_count_ok = multi_stats.kill_count_ok;
		p->stats.bonehead_kills = multi_stats.bonehead_kills;

		p->stats.p_shots_fired = multi_stats.p_shots_fired;
		p->stats.p_shots_hit = multi_stats.p_shots_hit;
		p->stats.p_bonehead_hits = multi_stats.p_bonehead_hits;

		p->stats.s_shots_fired = multi_stats.s_shots_fired;
		p->stats.s_shots_hit = multi_stats.s_shots_hit;
		p->stats.s_bonehead_hits = multi_stats.s_bonehead_hits;

		p->stats.flight_time = multi_stats.flight_time;
		p->stats.missions_flown = multi_stats.missions_flown;
		p->stats.last_flown = multi_stats.last_flown;
		p->stats.last_backup = multi_stats.last_backup;

		// ship kills (have to find ones that match content)
		list_size = (int)multi_stats.ship_kills.size();
		for (idx = 0; idx < list_size; idx++) {
			j = multi_stats.ship_kills[idx].index;

			if (j >= 0) {
				p->stats.kills[j] = multi_stats.ship_kills[idx].val;
			}
		}

		// medals earned (have to find ones that match content)
		list_size = (int)multi_stats.medals_earned.size();
		for (idx = 0; idx < list_size; idx++) {
			j = multi_stats.medals_earned[idx].index;

			if (j >= 0) {
				p->stats.medal_counts[j] = multi_stats.medals_earned[idx].val;
			}
		}
	}
}

void pilotfile::plr_write_stats_multi()
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

void pilotfile::plr_read_controls()
{
	int idx, list_size, list_axis;
	short id1, id2, id3 __attribute__((__unused__));
	int axi, inv;

	list_size = (int)cfread_ushort(cfp);
	for (idx = 0; idx < list_size; idx++) {
		id1 = cfread_short(cfp);
		id2 = cfread_short(cfp);
		id3 = cfread_short(cfp);	// unused, at the moment

		if (idx < CCFG_MAX) {
			Control_config[idx].key_id = id1;
			Control_config[idx].joy_id = id2;
		}
	}

	list_axis = cfread_int(cfp);
	for (idx = 0; idx < list_axis; idx++) {
		axi = cfread_int(cfp);
		inv = cfread_int(cfp);

		if (idx < NUM_JOY_AXIS_ACTIONS) {
			Axis_map_to[idx] = axi;
			Invert_axis[idx] = inv;
		}
	}
}

void pilotfile::plr_write_controls()
{
	int idx;

	startSection(Section::Controls);

	cfwrite_ushort(CCFG_MAX, cfp);

	for (idx = 0; idx < CCFG_MAX; idx++) {
		cfwrite_short(Control_config[idx].key_id, cfp);
		cfwrite_short(Control_config[idx].joy_id, cfp);
		// placeholder? for future mouse_id?
		cfwrite_short(-1, cfp);
	}

	cfwrite_int(NUM_JOY_AXIS_ACTIONS, cfp);

	for (idx = 0; idx < NUM_JOY_AXIS_ACTIONS; idx++) {
		cfwrite_int(Axis_map_to[idx], cfp);
		cfwrite_int(Invert_axis[idx], cfp);
	}

	endSection();
}

void pilotfile::plr_read_settings()
{
	// sound/voice/music
	Master_sound_volume = cfread_float(cfp);
	Master_event_music_volume = cfread_float(cfp);
	Master_voice_volume = cfread_float(cfp);

	audiostream_set_volume_all(Master_voice_volume, ASF_VOICE);
	audiostream_set_volume_all(Master_event_music_volume, ASF_EVENTMUSIC);

	if (Master_event_music_volume > 0.0f) {
		Event_music_enabled = 1;
	} else {
		Event_music_enabled = 0;
	}

	Briefing_voice_enabled = cfread_int(cfp);

	// skill level
	Game_skill_level = cfread_int(cfp);

	// input options
	Use_mouse_to_fly = cfread_int(cfp);
	Mouse_sensitivity = cfread_int(cfp);
	Joy_sensitivity = cfread_int(cfp);
	Dead_zone_size = cfread_int(cfp);

	// detail
	Detail.setting = cfread_int(cfp);
	Detail.nebula_detail = cfread_int(cfp);
	Detail.detail_distance = cfread_int(cfp);
	Detail.hardware_textures = cfread_int(cfp);
	Detail.num_small_debris = cfread_int(cfp);
	Detail.num_particles = cfread_int(cfp);
	Detail.num_stars = cfread_int(cfp);
	Detail.shield_effects = cfread_int(cfp);
	Detail.lighting = cfread_int(cfp);
	Detail.targetview_model = cfread_int(cfp);
	Detail.planets_suns = cfread_int(cfp);
	Detail.weapon_extras = cfread_int(cfp);
}

void pilotfile::plr_write_settings()
{
	startSection(Section::Settings);

	// sound/voice/music
	cfwrite_float(Master_sound_volume, cfp);
	cfwrite_float(Master_event_music_volume, cfp);
	cfwrite_float(Master_voice_volume, cfp);

	cfwrite_int(Briefing_voice_enabled, cfp);

	// skill level
	cfwrite_int(Game_skill_level, cfp);

	// input options
	cfwrite_int(Use_mouse_to_fly, cfp);
	cfwrite_int(Mouse_sensitivity, cfp);
	cfwrite_int(Joy_sensitivity, cfp);
	cfwrite_int(Dead_zone_size, cfp);

	// detail
	cfwrite_int(Detail.setting, cfp);
	cfwrite_int(Detail.nebula_detail, cfp);
	cfwrite_int(Detail.detail_distance, cfp);
	cfwrite_int(Detail.hardware_textures, cfp);
	cfwrite_int(Detail.num_small_debris, cfp);
	cfwrite_int(Detail.num_particles, cfp);
	cfwrite_int(Detail.num_stars, cfp);
	cfwrite_int(Detail.shield_effects, cfp);
	cfwrite_int(Detail.lighting, cfp);
	cfwrite_int(Detail.targetview_model, cfp);
	cfwrite_int(Detail.planets_suns, cfp);
	cfwrite_int(Detail.weapon_extras, cfp);

	endSection();
}

void pilotfile::plr_reset_data()
{
	// internals
	m_have_flags = false;
	m_have_info = false;

	m_data_invalid = false;

	// set all the entries in the control config arrays to -1 (undefined)
	control_config_clear();

	// init stats
	p->stats.init();

	// reset scoring lists
	all_time_stats.ship_kills.clear();
	all_time_stats.medals_earned.clear();

	multi_stats.ship_kills.clear();
	multi_stats.medals_earned.clear();

	// clear variables
	p->variables.clear();

	// reset techroom to defaults (CSG will override this, multi will use defaults)
	tech_reset_to_default();
}

void pilotfile::plr_close()
{
	if (cfp) {
		cfclose(cfp);
		cfp = NULL;
	}

	p = NULL;
	filename = "";

	ship_list.clear();
	weapon_list.clear();
	intel_list.clear();
	medals_list.clear();

	m_have_flags = false;
	m_have_info = false;
}

bool pilotfile::load_player(const char *callsign, player *_p)
{
	// if we're a standalone server in multiplayer, just fill in some bogus values
	// since we don't have a pilot file
	if ( (Game_mode & GM_MULTIPLAYER) && (Game_mode & GM_STANDALONE_SERVER) ) {
		Player->insignia_texture = -1;
		strcpy_s(Player->callsign, NOX("Standalone"));
		strcpy_s(Player->short_callsign, NOX("Standalone"));

		return true;
	}

	// set player ptr first thing
	p = _p;

	if ( !p ) {
		Assert( (Player_num >= 0) && (Player_num < MAX_PLAYERS) );
		p = &Players[Player_num];
	}

	filename = callsign;
	filename += ".plr";

	if ( filename.size() == 4 ) {
		mprintf(("PLR => Invalid filename '%s'!\n", filename.c_str()));
		return false;
	}

	cfp = cfopen((char*)filename.c_str(), "rb", CFILE_NORMAL, CF_TYPE_PLAYERS);

	if ( !cfp ) {
		mprintf(("PLR => Unable to open '%s' for reading!\n", filename.c_str()));
		return false;
	}

	unsigned int plr_id = cfread_uint(cfp);

	if (plr_id != PLR_FILE_ID) {
		mprintf(("PLR => Invalid header id for '%s'!\n", filename.c_str()));
		plr_close();
		return false;
	}

	// version, now used
	version = cfread_ubyte(cfp);

	mprintf(("PLR => Loading '%s' with version %d...\n", filename.c_str(), version));

	plr_reset_data();

	// the point of all this: read in the PLR contents
	while ( !cfeof(cfp) ) {
		ushort section_id = cfread_ushort(cfp);
		uint section_size = cfread_uint(cfp);

		size_t start_pos = cftell(cfp);

		// safety, to help protect against long reads
		cf_set_max_read_len(cfp, section_size);

		try {
			switch (section_id) {
				case Section::Flags:
					mprintf(("PLR => Parsing:  Flags...\n"));
					m_have_flags = true;
					plr_read_flags();
					break;

				case Section::Info:
					mprintf(("PLR => Parsing:  Info...\n"));
					m_have_info = true;
					plr_read_info();
					break;

				case Section::Variables:
					mprintf(("PLR => Parsing:  Variables...\n"));
					plr_read_variables();
					break;

				case Section::HUD:
					mprintf(("PLR => Parsing:  HUD...\n"));
					plr_read_hud();
					break;

				case Section::Scoring:
					mprintf(("PLR => Parsing:  Scoring...\n"));
					plr_read_stats();
					break;

				case Section::ScoringMulti:
					mprintf(("PLR => Parsing:  ScoringMulti...\n"));
					plr_read_stats_multi();
					break;

				case Section::Multiplayer:
					mprintf(("PLR => Parsing:  Multiplayer...\n"));
					plr_read_multiplayer();
					break;

				case Section::Controls:
					mprintf(("PLR => Parsing:  Controls...\n"));
					plr_read_controls();
					break;

				case Section::Settings:
					mprintf(("PLR => Parsing:  Settings...\n"));
					plr_read_settings();
					break;

				default:
					mprintf(("PLR => Skipping unknown section 0x%04x!\n", section_id));
					break;
			}
		} catch (cfile::max_read_length &msg) {
			// read to max section size, move to next section, discarding
			// extra/unknown data
			mprintf(("PLR => (0x%04x) %s\n", section_id, msg.what()));
		} catch (const char *err) {
			mprintf(("PLR => ERROR: %s\n", err));
			plr_close();
			return false;
		}

		// reset safety catch
		cf_set_max_read_len(cfp, 0);

		// skip to next section (if not already there)
		size_t offset_pos = (start_pos + section_size) - cftell(cfp);

		if (offset_pos) {
			cfseek(cfp, offset_pos, CF_SEEK_CUR);
			mprintf(("PLR => WARNING: Advancing to the next section. %i bytes were skipped!\n", offset_pos));
		}
	}

	// restore the callsign into the Player structure
	strcpy_s(p->callsign, callsign);

	// restore the truncated callsign into Player structure
	pilot_set_short_callsign(p, SHORT_CALLSIGN_PIXEL_W);

	player_set_squad_bitmap(p, p->m_squad_filename, true);

	hud_squadmsg_save_keys();

	// set last pilot
	os_config_write_string(NULL, "LastPlayer", (char*)callsign);

	mprintf(("PLR => Loading complete!\n"));

	// cleanup and return
	plr_close();

	return true;
}

bool pilotfile::save_player(player *_p)
{
	// never save a pilot file for the standalone server in multiplayer
	if ( (Game_mode & GM_MULTIPLAYER) && (Game_mode & GM_STANDALONE_SERVER) ) {
		return false;
	}

	// set player ptr first thing
	p = _p;

	if ( !p ) {
		Assert( (Player_num >= 0) && (Player_num < MAX_PLAYERS) );
		p = &Players[Player_num];
	}

	if ( !strlen(p->callsign) ) {
		return false;
	}

	filename = p->callsign;
	filename += ".plr";

	if ( filename.size() == 4 ) {
		mprintf(("PLR => Invalid filename '%s'!\n", filename.c_str()));
		return false;
	}

	// open it, hopefully...
	cfp = cfopen((char*)filename.c_str(), "wb", CFILE_NORMAL, CF_TYPE_PLAYERS);

	if ( !cfp ) {
		mprintf(("PLR => Unable to open '%s' for saving!\n", filename.c_str()));
		return false;
	}

	// header and version
	cfwrite_int(PLR_FILE_ID, cfp);
	cfwrite_ubyte(PLR_VERSION, cfp);

	mprintf(("PLR => Saving '%s' with version %d...\n", filename.c_str(), (int)PLR_VERSION));

	// flags and info sections go first
	mprintf(("PLR => Saving:  Flags...\n"));
	plr_write_flags();
	mprintf(("PLR => Saving:  Info...\n"));
	plr_write_info();

	// everything else is next, not order specific
	mprintf(("PLR => Saving:  Scoring...\n"));
	plr_write_stats();
	mprintf(("PLR => Saving:  ScoringMulti...\n"));
	plr_write_stats_multi();
	mprintf(("PLR => Saving:  HUD...\n"));
	plr_write_hud();
	mprintf(("PLR => Saving:  Variables...\n"));
	plr_write_variables();
	mprintf(("PLR => Saving:  Multiplayer...\n"));
	plr_write_multiplayer();
	mprintf(("PLR => Saving:  Controls...\n"));
	plr_write_controls();
	mprintf(("PLR => Saving:  Settings...\n"));
	plr_write_settings();

	// Done!
	mprintf(("PLR => Saving complete!\n"));

	plr_close();

	return true;
}

bool pilotfile::verify(const char *fname, int *rank, char *valid_language)
{
	player t_plr;

	// set player ptr first thing
	p = &t_plr;

	filename = fname;

	if ( filename.size() == 4 ) {
		mprintf(("PLR => Invalid filename '%s'!\n", filename.c_str()));
		return false;
	}

	cfp = cfopen((char*)filename.c_str(), "rb", CFILE_NORMAL, CF_TYPE_PLAYERS);

	if ( !cfp ) {
		mprintf(("PLR => Unable to open '%s'!\n", filename.c_str()));
		return false;
	}

	unsigned int plr_id = cfread_uint(cfp);

	if (plr_id != PLR_FILE_ID) {
		mprintf(("PLR => Invalid header id for '%s'!\n", filename.c_str()));
		plr_close();
		return false;
	}

	// version, now used
	version = cfread_ubyte(cfp);

	mprintf(("PLR => Verifying '%s' with version %d...\n", filename.c_str(), (int)version));

	// the point of all this: read in the PLR contents
	while ( !(m_have_flags && m_have_info) && !cfeof(cfp) ) {
		ushort section_id = cfread_ushort(cfp);
		uint section_size = cfread_uint(cfp);

		size_t start_pos = cftell(cfp);

		// safety, to help protect against long reads
		cf_set_max_read_len(cfp, section_size);

		try {
			switch (section_id) {
				case Section::Flags:
					mprintf(("PLR => Parsing:  Flags...\n"));
					m_have_flags = true;
					plr_read_flags();
					break;

				// now reading the Info section to get the campaign
				// and be able to lookup the campaign rank
				case Section::Info:
					mprintf(("PLR => Parsing:  Info...\n"));
					m_have_info = true;
					plr_read_info();
					break;

				default:
					break;
			}
		} catch (cfile::max_read_length &msg) {
			// read to max section size, move to next section, discarding
			// extra/unknown data
			mprintf(("PLR => (0x%04x) %s\n", section_id, msg.what()));
		} catch (const char *err) {
			mprintf(("PLR => ERROR: %s\n", err));
			plr_close();
			return false;
		}

		// reset safety catch
		cf_set_max_read_len(cfp, 0);

		// skip to next section (if not already there)
		size_t offset_pos = (start_pos + section_size) - cftell(cfp);

		if (offset_pos) {
			mprintf(("PLR => Warning: (0x%04x) Short read, information may have been lost!\n", section_id));
			cfseek(cfp, offset_pos, CF_SEEK_CUR);
		}
	}

	if (valid_language) {
		strncpy(valid_language, p->language, sizeof(p->language));
	}

	// need to cleanup early to ensure everything is OK for use in the CSG next
	// also means we can't use *p from now on, use t_plr instead for a few vars
	plr_close();

	if (rank) {
		// maybe get the rank from the CSG
		if ( !(Game_mode & GM_MULTIPLAYER) ) {
			// build the csg filename
			// since filename/fname was validated above, perform less safety checks here
			filename = fname;
			filename = filename.replace(filename.find_last_of('.')+1,filename.npos, t_plr.current_campaign);
			filename.append(".csg");

			if (!this->get_csg_rank(rank)) {
				// if we failed to get the csg rank, default to multi rank
				*rank = t_plr.stats.rank;
			}
		} else {
			// if the CSG isn't valid, or for multi, use this rank
			*rank = t_plr.stats.rank;
		}
	}

	mprintf(("PLR => Verifying complete!\n"));

	return true;
}

