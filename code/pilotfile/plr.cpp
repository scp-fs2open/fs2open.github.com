
#include "controlconfig/controlsconfig.h"
#include "controlconfig/presets.h"
#include "freespace.h"
#include "gamesnd/eventmusic.h"
#include "hud/hudconfig.h"
#include "hud/hudsquadmsg.h"
#include "io/joy.h"
#include "io/mouse.h"
#include "localization/localize.h"
#include "menuui/techmenu.h"
#include "network/multi.h"
#include "osapi/osregistry.h"
#include "parse/sexp_container.h"
#include "pilotfile/pilotfile.h"
#include "pilotfile/BinaryFileHandler.h"
#include "pilotfile/JSONFileHandler.h"
#include "playerman/managepilot.h"
#include "playerman/player.h"
#include "ship/ship.h"
#include "sound/audiostr.h"
#include "stats/medals.h"
#include "weapon/weapon.h"

namespace {
void read_multi_stats(pilot::FileHandler* handler, scoring_special_t* scoring) {
	scoring->score = handler->readInt("score");
	scoring->rank = handler->readInt("rank");
	scoring->assists = handler->readInt("assists");
	scoring->kill_count = handler->readInt("kill_count");
	scoring->kill_count_ok = handler->readInt("kill_count_ok");
	scoring->bonehead_kills = handler->readInt("bonehead_kills");

	scoring->p_shots_fired = handler->readUInt("p_shots_fired");
	scoring->p_shots_hit = handler->readUInt("p_shots_hit");
	scoring->p_bonehead_hits = handler->readUInt("p_bonehead_hits");

	scoring->s_shots_fired = handler->readUInt("s_shots_fired");
	scoring->s_shots_hit = handler->readUInt("s_shots_hit");
	scoring->s_bonehead_hits = handler->readUInt("s_bonehead_hits");

	scoring->flight_time = handler->readUInt("flight_time");
	scoring->missions_flown = handler->readUInt("missions_flown");
	scoring->last_flown = (_fs_time_t)handler->readInt("last_flown");
	scoring->last_backup = (_fs_time_t)handler->readInt("last_backup");

	// ship kills (contains ships across all mods, not just current)
	auto list_size = handler->startArrayRead("kills");
	scoring->ship_kills.reserve(list_size);

	for (size_t idx = 0; idx < list_size; idx++, handler->nextArraySection()) {
		index_list_t ilist;
		ilist.name = handler->readString("name");
		ilist.index = ship_info_lookup(ilist.name.c_str());
		ilist.val = handler->readInt("val");

		scoring->ship_kills.push_back(ilist);
	}
	handler->endArrayRead();

	// medals earned (contains medals across all mods, not just current)
	list_size = handler->startArrayRead("medals");
	scoring->medals_earned.reserve(list_size);

	for (size_t idx = 0; idx < list_size; idx++, handler->nextArraySection()) {
		index_list_t ilist;
		ilist.name = handler->readString("name");
		ilist.index = medals_info_lookup(ilist.name.c_str());
		ilist.val = handler->readInt("val");

		scoring->medals_earned.push_back(ilist);
	}
	handler->endArrayRead();
}
}

void pilotfile::plr_read_flags()
{
	// tips?
	p->tips = (int)handler->readUByte("tips");

	// saved flags
	p->save_flags = handler->readInt("save_flags");

	// listing mode (single or campaign missions
	p->readyroom_listing_mode = handler->readInt("readyroom_listing_mode");

	// briefing auto-play
	p->auto_advance = handler->readInt("auto_advance");

	// special rank setting (to avoid having to read all stats on verify)
	// will be the multi rank
	// if there's a valid CSG, this will be overwritten
	p->stats.rank = handler->readInt("multi_rank");

	if (plr_ver > 0) {
		p->player_was_multi = handler->readInt("was_multi");
	} else {
		p->player_was_multi = 0; // Default to single player
	}

	// which language was this pilot created with
	if (plr_ver > 1) {
		handler->readString("language", p->language, sizeof(p->language));
	} else {
		// if we don't know, default to the current language setting
		lcl_get_language_name(p->language);
	}
}

void pilotfile::plr_write_flags()
{
	handler->startSectionWrite(Section::Flags);

	// tips
	handler->writeUByte("tips", (unsigned char)p->tips);

	// saved flags
	handler->writeInt("save_flags", p->save_flags);

	// listing mode (single or campaign missions)
	handler->writeInt("readyroom_listing_mode", p->readyroom_listing_mode);

	// briefing auto-play
	handler->writeInt("auto_advance", p->auto_advance);

	// special rank setting (to avoid having to read all stats on verify)
	// should be multi only from now on
	handler->writeInt("multi_rank", multi_stats.rank);

	// What game mode we were in last on this pilot
	handler->writeInt("was_multi", p->player_was_multi);

	// which language was this pilot created with
	handler->writeString("language", p->language);

	handler->endSectionWrite();
}

void pilotfile::plr_read_info()
{
	// pilot image
	handler->readString("image_filename", p->image_filename, MAX_FILENAME_LEN);

	// multi squad name
	handler->readString("squad_name", p->m_squad_name, NAME_LENGTH);

	// squad image
	handler->readString("squad_filename", p->m_squad_filename, MAX_FILENAME_LEN);

	// active campaign
	handler->readString("current_campaign", p->current_campaign, MAX_FILENAME_LEN);
}

void pilotfile::plr_write_info()
{
	handler->startSectionWrite(Section::Info);

	// pilot image
	handler->writeString("image_filename", p->image_filename);

	// multi squad name
	handler->writeString("squad_name", p->m_squad_name);

	// squad image
	handler->writeString("squad_filename", p->m_squad_filename);

	// active campaign
	handler->writeString("current_campaign", p->current_campaign);

	handler->endSectionWrite();
}

void pilotfile::plr_read_hud()
{
	int strikes = 0;
	// flags
	HUD_config.show_flags = handler->readInt("show_flags");
	HUD_config.show_flags2 = handler->readInt("show_flags2");

	HUD_config.popup_flags = handler->readInt("popup_flags");
	HUD_config.popup_flags2 = handler->readInt("popup_flags2");

	// settings
	HUD_config.num_msg_window_lines = handler->readUByte("num_msg_window_lines");

	HUD_config.rp_flags = handler->readInt("rp_flags");
	HUD_config.rp_dist = handler->readInt("rp_dist");
	if (HUD_config.rp_dist < 0 || HUD_config.rp_dist >= RR_MAX_RANGES) {
		ReleaseWarning(LOCATION, "Player file has invalid radar range %d, setting to default.\n", HUD_config.rp_dist);
		HUD_config.rp_dist = RR_INFINITY;
		strikes++;
	}

	// basic colors
	HUD_config.main_color = handler->readInt("main_color");
	if (HUD_config.main_color < 0 || HUD_config.main_color >= HUD_COLOR_SIZE) {
		ReleaseWarning(LOCATION, "Player file has invalid main color selection %i, setting to default.\n", HUD_config.main_color);
		HUD_config.main_color = HUD_COLOR_GREEN;
		strikes++;
	}

	HUD_color_alpha = handler->readInt("color_alpha");
	if (HUD_color_alpha < HUD_COLOR_ALPHA_USER_MIN || HUD_color_alpha > HUD_COLOR_ALPHA_USER_MAX) {
		ReleaseWarning(LOCATION, "Player file has invalid alpha color %i, setting to default.\n", HUD_color_alpha);
		HUD_color_alpha = HUD_COLOR_ALPHA_DEFAULT;
		strikes++;
	}

	if (strikes == 3) {
		ReleaseWarning(LOCATION, "Player file has too many hud config errors, and is likely corrupted. Please verify and save your settings in the hud config menu.");
	}

	hud_config_set_color(HUD_config.main_color);

	// gauge-specific colors
	auto num_gauges = handler->startArrayRead("hud_gauges");
	for (size_t idx = 0; idx < num_gauges; idx++, handler->nextArraySection()) {
		ubyte red = handler->readUByte("red");
		ubyte green = handler->readUByte("green");
		ubyte blue = handler->readUByte("blue");
		ubyte alpha = handler->readUByte("alpha");


		if (idx >= NUM_HUD_GAUGES) {
			continue;
		}

		HUD_config.clr[idx].red = red;
		HUD_config.clr[idx].green = green;
		HUD_config.clr[idx].blue = blue;
		HUD_config.clr[idx].alpha = alpha;
	}
	handler->endArrayRead();
}

void pilotfile::plr_write_hud()
{
	int idx;

	handler->startSectionWrite(Section::HUD);

	// flags
	handler->writeInt("show_flags", HUD_config.show_flags);
	handler->writeInt("show_flags2", HUD_config.show_flags2);

	handler->writeInt("popup_flags", HUD_config.popup_flags);
	handler->writeInt("popup_flags2", HUD_config.popup_flags2);

	// settings
	handler->writeUByte("num_msg_window_lines", HUD_config.num_msg_window_lines);

	handler->writeInt("rp_flags", HUD_config.rp_flags);
	handler->writeInt("rp_dist", HUD_config.rp_dist);

	// basic colors
	handler->writeInt("main_color", HUD_config.main_color);
	handler->writeInt("color_alpha", HUD_color_alpha);

	// gauge-specific colors
	handler->startArrayWrite("hud_gauges", NUM_HUD_GAUGES);
	for (idx = 0; idx < NUM_HUD_GAUGES; idx++) {
		handler->startSectionWrite(Section::Unnamed);

		handler->writeUByte("red", HUD_config.clr[idx].red);
		handler->writeUByte("green", HUD_config.clr[idx].green);
		handler->writeUByte("blue", HUD_config.clr[idx].blue);
		handler->writeUByte("alpha", HUD_config.clr[idx].alpha);

		handler->endSectionWrite();
	}
	handler->endArrayWrite();

	handler->endSectionWrite();
}

void pilotfile::plr_read_variables()
{
	auto list_size = handler->startArrayRead("variables");

	p->variables.reserve(list_size);
	for (size_t idx = 0; idx < list_size; idx++, handler->nextArraySection()) {
		sexp_variable n_var;
		n_var.type = handler->readInt("type");
		handler->readString("text", n_var.text, TOKEN_LENGTH);
		handler->readString("variable_name", n_var.variable_name, TOKEN_LENGTH);

		p->variables.push_back( n_var );
	}
	handler->endArrayRead();
}

void pilotfile::plr_write_variables()
{
	handler->startSectionWrite(Section::Variables);

	handler->startArrayWrite("variables", p->variables.size());
	for (auto& var : p->variables) {
		handler->startSectionWrite(Section::Unnamed);

		handler->writeInt("type", var.type);
		handler->writeString("text", var.text);
		handler->writeString("variable_name", var.variable_name);

		handler->endSectionWrite();
	}
	handler->endArrayWrite();

	handler->endSectionWrite();
}

void pilotfile::plr_read_containers()
{
	const size_t list_size = handler->startArrayRead("containers");

	p->containers.reserve(list_size);
	for (size_t idx = 0; idx < list_size; idx++, handler->nextArraySection()) {
		p->containers.emplace_back();
		auto& container = p->containers.back();

		container.container_name = handler->readString("container_name");

		container.type = (ContainerType)handler->readInt("type");
		container.opf_type = handler->readInt("opf_type");

		const int size = handler->readInt("size");
		Assert(size >= 0);

		if (container.is_list()) {
			for (int i = 0; i < size; ++i) {
				SCP_string data_idx_str = SCP_string("data_") + std::to_string(i);
				container.list_data.emplace_back(handler->readString(data_idx_str.c_str()));
			}
		} else if (container.is_map()) {
			for (int i = 0; i < size; ++i) {
				SCP_string key_idx_str = SCP_string("key_") + std::to_string(i);
				SCP_string data_idx_str = SCP_string("data_") + std::to_string(i);
				SCP_string key = handler->readString(key_idx_str.c_str());
				SCP_string data = handler->readString(data_idx_str.c_str());
				container.map_data.emplace(key, data);
			}
		} else {
			UNREACHABLE("Unknown container type %d", (int)container.type);
		}
	}
	handler->endArrayRead();
}

void pilotfile::plr_write_containers()
{
	handler->startSectionWrite(Section::Containers);

	handler->startArrayWrite("containers", p->containers.size());
	for (const auto& container : p->containers) {
		handler->startSectionWrite(Section::Unnamed);

		handler->writeString("container_name", container.container_name.c_str());

		handler->writeInt("type", (int)container.type);
		handler->writeInt("opf_type", container.opf_type);

		handler->writeInt("size", container.size());

		int i = 0;

		if (container.is_list()) {
			for (const auto& data : container.list_data) {
				SCP_string data_idx_str = SCP_string("data_") + std::to_string(i);
				handler->writeString(data_idx_str.c_str(), data.c_str());
				++i;
			}
		} else if (container.is_map()) {
			for (const auto& key_data : container.map_data) {
				SCP_string key_idx_str = SCP_string("key_") + std::to_string(i);
				SCP_string data_idx_str = SCP_string("data_") + std::to_string(i);
				handler->writeString(key_idx_str.c_str(), key_data.first.c_str());
				handler->writeString(data_idx_str.c_str(), key_data.second.c_str());
				++i;
			}
		} else {
			UNREACHABLE("Unknown container type %d", (int)container.type);
		}

		handler->endSectionWrite();
	}
	handler->endArrayWrite();

	handler->endSectionWrite();
}

void pilotfile::plr_read_multiplayer()
{
	// netgame options
	p->m_server_options.squad_set = handler->readUByte("squad_set");
	p->m_server_options.endgame_set = handler->readUByte("endgame_set");
	p->m_server_options.flags = handler->readInt("server_flags");
	p->m_server_options.respawn = handler->readUInt("respawn");
	p->m_server_options.max_observers = handler->readUByte("max_observers");
	p->m_server_options.skill_level = handler->readUByte("skill_level");
	p->m_server_options.voice_qos = handler->readUByte("voice_qos");
	p->m_server_options.voice_token_wait = handler->readInt("voice_token_wait");
	p->m_server_options.voice_record_time = handler->readInt("voice_record_time");
	p->m_server_options.mission_time_limit = (fix)handler->readInt("mission_time_limit");
	p->m_server_options.kill_limit = handler->readInt("kill_limit");

	// local options
	p->m_local_options.flags = handler->readInt("local_flags");
	p->m_local_options.obj_update_level = handler->readInt("obj_update_level");

	// netgame protocol
	Multi_options_g.protocol = handler->readInt("protocol");

	if (Multi_options_g.protocol == NET_VMT) {
		Multi_options_g.protocol = NET_TCP;
		Multi_options_g.pxo = 1;
	} else if (Multi_options_g.protocol != NET_TCP) {
		Multi_options_g.protocol = NET_TCP;
	}
}

void pilotfile::plr_write_multiplayer()
{
	handler->startSectionWrite(Section::Multiplayer);

	// netgame options
	handler->writeUByte("squad_set", p->m_server_options.squad_set);
	handler->writeUByte("endgame_set", p->m_server_options.endgame_set);
	handler->writeInt("server_flags", p->m_server_options.flags);
	handler->writeUInt("respawn", p->m_server_options.respawn);
	handler->writeUByte("max_observers", p->m_server_options.max_observers);
	handler->writeUByte("skill_level", p->m_server_options.skill_level);
	handler->writeUByte("voice_qos", p->m_server_options.voice_qos);
	handler->writeInt("voice_token_wait", p->m_server_options.voice_token_wait);
	handler->writeInt("voice_record_time", p->m_server_options.voice_record_time);
	handler->writeInt("mission_time_limit", (int)p->m_server_options.mission_time_limit);
	handler->writeInt("kill_limit", p->m_server_options.kill_limit);

	// local options
	handler->writeInt("local_flags", p->m_local_options.flags);
	handler->writeInt("obj_update_level", p->m_local_options.obj_update_level);

	// netgame protocol
	if (Multi_options_g.pxo == 1) {
		const int protocol = NET_VMT;
		handler->writeInt("protocol", protocol);
	} else {
		handler->writeInt("protocol", Multi_options_g.protocol);
	}

	handler->endSectionWrite();
}

void pilotfile::plr_read_stats()
{
	// global, all-time stats (used only until campaign stats are loaded)
	read_multi_stats(handler.get(), &all_time_stats);

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
		auto list_size = all_time_stats.ship_kills.size();
		for (size_t idx = 0; idx < list_size; idx++) {
			auto j = all_time_stats.ship_kills[idx].index;

			if (j >= 0) {
				p->stats.kills[j] = all_time_stats.ship_kills[idx].val;
			}
		}

		// medals earned (have to find ones that match content)
		list_size = all_time_stats.medals_earned.size();
		for (size_t idx = 0; idx < list_size; idx++) {
			auto j = all_time_stats.medals_earned[idx].index;

			if (j >= 0) {
				p->stats.medal_counts[j] = all_time_stats.medals_earned[idx].val;
			}
		}
	}
}

void pilotfile::plr_write_stats()
{
	handler->startSectionWrite(Section::Scoring);

	// global, all-time stats
	handler->writeInt("score", all_time_stats.score);
	handler->writeInt("rank", all_time_stats.rank);
	handler->writeInt("assists", all_time_stats.assists);
	handler->writeInt("kill_count", all_time_stats.kill_count);
	handler->writeInt("kill_count_ok", all_time_stats.kill_count_ok);
	handler->writeInt("bonehead_kills", all_time_stats.bonehead_kills);

	handler->writeUInt("p_shots_fired", all_time_stats.p_shots_fired);
	handler->writeUInt("p_shots_hit", all_time_stats.p_shots_hit);
	handler->writeUInt("p_bonehead_hits", all_time_stats.p_bonehead_hits);

	handler->writeUInt("s_shots_fired", all_time_stats.s_shots_fired);
	handler->writeUInt("s_shots_hit", all_time_stats.s_shots_hit);
	handler->writeUInt("s_bonehead_hits", all_time_stats.s_bonehead_hits);

	handler->writeUInt("flight_time", all_time_stats.flight_time);
	handler->writeUInt("missions_flown", all_time_stats.missions_flown);
	handler->writeInt("last_flown", (int)all_time_stats.last_flown);
	handler->writeInt("last_backup", (int)all_time_stats.last_backup);

	// ship kills (contains ships across all mods, not just current)
	handler->startArrayWrite("kills", all_time_stats.ship_kills.size());
	for (auto& kill : all_time_stats.ship_kills) {
		handler->startSectionWrite(Section::Unnamed);
		handler->writeString("name", kill.name.c_str());
		handler->writeInt("val", kill.val);
		handler->endSectionWrite();
	}
	handler->endArrayWrite();

	// medals earned (contains medals across all mods, not just current)
	handler->startArrayWrite("medals", all_time_stats.medals_earned.size());
	for (auto& medal : all_time_stats.medals_earned) {
		handler->startSectionWrite(Section::Unnamed);
		handler->writeString("name", medal.name.c_str());
		handler->writeInt("val", medal.val);
		handler->endSectionWrite();
	}
	handler->endArrayWrite();

	handler->endSectionWrite();
}

void pilotfile::plr_read_stats_multi()
{
	// global, all-time stats (used only until campaign stats are loaded)
	read_multi_stats(handler.get(), &multi_stats);

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
		auto list_size = multi_stats.ship_kills.size();
		for (size_t idx = 0; idx < list_size; idx++) {
			auto j = multi_stats.ship_kills[idx].index;

			if (j >= 0) {
				p->stats.kills[j] = multi_stats.ship_kills[idx].val;
			}
		}

		// medals earned (have to find ones that match content)
		list_size = (int)multi_stats.medals_earned.size();
		for (size_t idx = 0; idx < list_size; idx++) {
			auto j = multi_stats.medals_earned[idx].index;

			if (j >= 0) {
				p->stats.medal_counts[j] = multi_stats.medals_earned[idx].val;
			}
		}
	}
}

void pilotfile::plr_write_stats_multi()
{
	handler->startSectionWrite(Section::ScoringMulti);

	// global, all-time stats
	handler->writeInt("score", multi_stats.score);
	handler->writeInt("rank", multi_stats.rank);
	handler->writeInt("assists", multi_stats.assists);
	handler->writeInt("kill_count", multi_stats.kill_count);
	handler->writeInt("kill_count_ok", multi_stats.kill_count_ok);
	handler->writeInt("bonehead_kills", multi_stats.bonehead_kills);

	handler->writeUInt("p_shots_fired", multi_stats.p_shots_fired);
	handler->writeUInt("p_shots_hit", multi_stats.p_shots_hit);
	handler->writeUInt("p_bonehead_hits", multi_stats.p_bonehead_hits);

	handler->writeUInt("s_shots_fired", multi_stats.s_shots_fired);
	handler->writeUInt("s_shots_hit", multi_stats.s_shots_hit);
	handler->writeUInt("s_bonehead_hits", multi_stats.s_bonehead_hits);

	handler->writeUInt("flight_time", multi_stats.flight_time);
	handler->writeUInt("missions_flown", multi_stats.missions_flown);
	handler->writeInt("last_flown", (int)multi_stats.last_flown);
	handler->writeInt("last_backup", (int)multi_stats.last_backup);

	// ship kills (contains medals across all mods, not just current)
	handler->startArrayWrite("kills", multi_stats.ship_kills.size());
	for (auto& kill : multi_stats.ship_kills) {
		handler->startSectionWrite(Section::Unnamed);
		handler->writeString("name", kill.name.c_str());
		handler->writeInt("val", kill.val);
		handler->endSectionWrite();
	}
	handler->endArrayWrite();

	// medals earned (contains medals across all mods, not just current)
	handler->startArrayWrite("medals", multi_stats.medals_earned.size());
	for (auto& medal : multi_stats.medals_earned) {
		handler->startSectionWrite(Section::Unnamed);
		handler->writeString("name", medal.name.c_str());
		handler->writeInt("val", medal.val);
		handler->endSectionWrite();
	}
	handler->endArrayWrite();

	handler->endSectionWrite();
}

void pilotfile::plr_read_controls()
{
	if (plr_ver < 4) {
		// PLR < 4
		short id1, id2;
		int axi, inv;

		// Set up preset name, we'll populate the rest of the preset's data later
		CC_preset preset;
		preset.name = filename;

		// strip off extension
		auto n = preset.name.find_last_of('.');
		preset.name.resize(n);
		
		// Load in the bindings to Control_config
		// ...First the digital controls
		auto list_size = handler->startArrayRead("controls", true);
		for (size_t idx = 0; idx < list_size; idx++, handler->nextArraySection()) {
			id1 = handler->readShort("key");
			id2 = handler->readShort("joystick");
			handler->readShort("mouse");

			if (idx < Control_config.size()) {
				// Force the bindings into Control_config
				Control_config[idx].take(CC_bind(CID_KEYBOARD, id1), 0);
				Control_config[idx].take(CC_bind(CID_JOY0, id2), 1);
			}
		}
		handler->endArrayRead();

		// ...Then the analog controls
		auto list_axis = handler->startArrayRead("axes");
		for (size_t idx = 0; idx < list_axis; idx++, handler->nextArraySection()) {
			axi = handler->readInt("axis_map");
			inv = handler->readInt("invert_axis");

			if (idx < NUM_JOY_AXIS_ACTIONS) {
				Control_config[idx + JOY_AXIS_BEGIN].take(CC_bind(CID_JOY0, static_cast<short>(axi), CCF_AXIS), 0);
				Control_config[idx + JOY_AXIS_BEGIN].first.invert(inv != 0);
				Control_config[idx + JOY_AXIS_BEGIN].second.invert(inv != 0);
			}
		}
		handler->endArrayRead();

		// Check that these bindings are in a preset.  If it is not, create a new preset file
		auto it = control_config_get_current_preset();
		if (it == Control_config_presets.end()) {
			std::copy(Control_config.begin(), Control_config.end(), std::back_inserter(preset.bindings));
			Control_config_presets.push_back(preset);

			// Try to save the preset file
			if (!save_preset_file(preset, false)) {
				Warning(LOCATION, "Could not save controls preset file (%s)", preset.name.c_str());
			}
		}
		return;

	} else {
		// read PLR >= 4
		SCP_string buf = handler->readString("preset");

		auto it = std::find_if(Control_config_presets.begin(), Control_config_presets.end(),
							   [buf](const CC_preset& preset) { return preset.name == buf; });

		if (it == Control_config_presets.end()) {
			Assertion(!Control_config_presets.empty(), "[PLR] Error reading Controls! Control_config_presets empty! Get a coder!");

			// Couldn't find the preset, use defaults
			ReleaseWarning(LOCATION, "Could not find preset %s, using defaults\n", buf.c_str());
			it = Control_config_presets.begin();
		}

		control_config_use_preset(*it);
		return;
	}
}

void pilotfile::plr_write_controls()
{
	handler->startSectionWrite(Section::Controls);

	// Save the default bindings to prevent crash. See github issue 3902
	auto& Control_config_default = Control_config_presets[0].bindings;
	handler->startArrayWrite("controls", JOY_AXIS_BEGIN, true);
	for (size_t i = 0; i < JOY_AXIS_BEGIN; i++) {
		auto& item = Control_config_default[i];
		handler->startSectionWrite(Section::Unnamed);

		handler->writeShort("key", item.get_btn(CID_KEYBOARD));
		handler->writeShort("joystick", item.get_btn(CID_JOY0));
		handler->writeShort("mouse", -1);

		handler->endSectionWrite();
	}
	handler->endArrayWrite();

	handler->startArrayWrite("axes", NUM_JOY_AXIS_ACTIONS);
	for (size_t idx = JOY_AXIS_BEGIN; idx < JOY_AXIS_END; idx++) {
		auto& item = Control_config_default[idx];

		handler->startSectionWrite(Section::Unnamed);

		// Combine mouse and joy0 to joy0 axis.  Needed because controlsconfigdefaults.tbl may change the defaults.
		int joy = static_cast<int>(item.get_btn(CID_JOY0));
		int mouse = static_cast<int>(item.get_btn(CID_MOUSE));
		if (joy >= 0) {
			handler->writeInt("axis_map", joy);

		} else if (mouse >= 0) {
			handler->writeInt("axis_map", mouse);

		} else {
			handler->writeInt("axis_map", -1);
		}
		
		handler->writeInt("invert_axis", (item.first.is_inverted() || item.second.is_inverted()) ? 1 : 0);

		handler->endSectionWrite();
	}
	handler->endArrayWrite();
	// End issue 3902


	// As of PLR v4, control bindings are saved outside of the PLR file into PST files.
	// Save the currently selected preset
	auto it = control_config_get_current_preset();

	if (it == Control_config_presets.end()) {
		// No current preset selected. what? Might be a new player...
		Assertion(!Control_config_presets.empty(), "[PLR] Error saving controls! Control_config_presets empty! Get a coder!");

		// Just bash it to defaults
		it = Control_config_presets.begin();
	}

	handler->writeString("preset", it->name.c_str());

	handler->endSectionWrite(); // Section::controls
}

void pilotfile::plr_read_settings()
{
	// sound/voice/music
	if (!Using_in_game_options) {
		snd_set_effects_volume(handler->readFloat("master_sound_volume"));
		event_music_set_volume(handler->readFloat("master_event_music_volume"));
		snd_set_voice_volume(handler->readFloat("aster_voice_volume"));

		Briefing_voice_enabled = handler->readInt("briefing_voice_enabled") != 0;
	} else {
		// The values are set by the in-game menu but we still need to read the int from the file to maintain the
		// correct offset
		handler->readFloat("master_sound_volume");
		handler->readFloat("master_event_music_volume");
		handler->readFloat("aster_voice_volume");

		handler->readInt("briefing_voice_enabled");
	}

	// skill level
	Game_skill_level = handler->readInt("game_skill_level");

	// input options
	if (!Using_in_game_options) {
		Use_mouse_to_fly   = handler->readInt("use_mouse_to_fly") != 0;
		Mouse_sensitivity  = handler->readInt("mouse_sensitivity");
		Joy_sensitivity    = handler->readInt("joy_sensitivity");
		Joy_dead_zone_size = handler->readInt("joy_dead_zone_size");

		// detail
		Detail.setting           = handler->readInt("setting");
		Detail.nebula_detail     = handler->readInt("nebula_detail");
		Detail.detail_distance   = handler->readInt("detail_distance");
		Detail.hardware_textures = handler->readInt("hardware_textures");
		Detail.num_small_debris  = handler->readInt("num_small_debris");
		Detail.num_particles     = handler->readInt("num_particles");
		Detail.num_stars         = handler->readInt("num_stars");
		Detail.shield_effects    = handler->readInt("shield_effects");
		Detail.lighting          = handler->readInt("lighting");
		Detail.targetview_model  = handler->readInt("targetview_model");
		Detail.planets_suns      = handler->readInt("planets_suns");
		Detail.weapon_extras     = handler->readInt("weapon_extras");
	} else {
		// The values are set by the in-game menu but we still need to read the int from the file to maintain the correct offset
		handler->readInt("use_mouse_to_fly");
		handler->readInt("mouse_sensitivity");
		handler->readInt("joy_sensitivity");
		handler->readInt("joy_dead_zone_size");

		// detail
		handler->readInt("setting");
		handler->readInt("nebula_detail");
		handler->readInt("detail_distance");
		handler->readInt("hardware_textures");
		handler->readInt("num_small_debris");
		handler->readInt("num_particles");
		handler->readInt("num_stars");
		handler->readInt("shield_effects");
		handler->readInt("lighting");
		handler->readInt("targetview_model");
		handler->readInt("planets_suns");
		handler->readInt("weapon_extras");
	}
}

void pilotfile::plr_write_settings()
{
	handler->startSectionWrite(Section::Settings);

	// sound/voice/music
	handler->writeFloat("master_sound_volume", Master_sound_volume);
	handler->writeFloat("master_event_music_volume", Master_event_music_volume);
	handler->writeFloat("aster_voice_volume", Master_voice_volume);

	handler->writeInt("briefing_voice_enabled", Briefing_voice_enabled ? 1 : 0);

	// skill level
	handler->writeInt("game_skill_level", Game_skill_level);

	// input options
	handler->writeInt("use_mouse_to_fly", Use_mouse_to_fly);
	handler->writeInt("mouse_sensitivity", Mouse_sensitivity);
	handler->writeInt("joy_sensitivity", Joy_sensitivity);
	handler->writeInt("joy_dead_zone_size", Joy_dead_zone_size);

	// detail
	handler->writeInt("setting", Detail.setting);
	handler->writeInt("nebula_detail", Detail.nebula_detail);
	handler->writeInt("detail_distance", Detail.detail_distance);
	handler->writeInt("hardware_textures", Detail.hardware_textures);
	handler->writeInt("num_small_debris", Detail.num_small_debris);
	handler->writeInt("num_particles", Detail.num_particles);
	handler->writeInt("num_stars", Detail.num_stars);
	handler->writeInt("shield_effects", Detail.shield_effects);
	handler->writeInt("lighting", Detail.lighting);
	handler->writeInt("targetview_model", Detail.targetview_model);
	handler->writeInt("planets_suns", Detail.planets_suns);
	handler->writeInt("weapon_extras", Detail.weapon_extras);

	handler->endSectionWrite();
}

void pilotfile::plr_reset_data(bool reset_all)
{
	// internals
	m_have_flags = false;
	m_have_info = false;

	m_data_invalid = false;

	// if we aren't reloading all data (such as just a verify) then skip the rest
	if ( !reset_all ) {
		return;
	}

	Assertion(p != nullptr, "player pointer is null during data reset!");

	// set all the entries in the control config arrays to -1 (undefined)
	control_config_clear();

	// init stats
	p->stats.init();

	// reset scoring lists
	scoring_special_t blank_score;

	all_time_stats = blank_score;
	multi_stats = blank_score;

	// clear variables
	p->variables.clear();

	// clear containers
	p->containers.clear();

	// reset techroom to defaults (CSG will override this, multi will use defaults)
	tech_reset_to_default();
}

void pilotfile::plr_close()
{
	if (cfp) {
		cfclose(cfp);
		cfp = NULL;
	}
	if (handler) {
		handler.reset();
	}

	p = NULL;
	filename = "";

	ship_list.clear();
	weapon_list.clear();
	intel_list.clear();
	medals_list.clear();

	m_have_flags = false;
	m_have_info = false;

	plr_ver = PLR_VERSION_INVALID;
}

bool pilotfile::load_player(const char* callsign, player* _p, bool force_binary)
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
	if (force_binary) {
		// Caller want to read the binary file
		filename += ".plr";
	} else {
		// The default is the JSON file
		filename += ".json";
	}

	if ( filename.size() == 4 ) {
		mprintf(("PLR => Invalid filename '%s'!\n", filename.c_str()));
		return false;
	}

	auto fp = cfopen(filename.c_str(), "rb", CFILE_NORMAL, CF_TYPE_PLAYERS, false,
	                 CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);
	if ( !fp ) {
		mprintf(("PLR => Unable to open '%s' for reading!\n", filename.c_str()));
		return false;
	}

	if (force_binary) {
		handler.reset(new pilot::BinaryFileHandler(fp));
	} else {
		try {
			handler.reset(new pilot::JSONFileHandler(fp, true));
		} catch (const std::exception& e) {
			mprintf(("PLR => Failed to parse JSON: %s\n", e.what()));
			return false;
		}
	}

	unsigned int plr_id = handler->readUInt("signature");

	if (plr_id != PLR_FILE_ID) {
		mprintf(("PLR => Invalid header id for '%s'!\n", filename.c_str()));
		plr_close();
		return false;
	}

	// version, now used
	plr_ver = handler->readUByte("version");

	mprintf(("PLR => Loading '%s' with version %d...\n", filename.c_str(), plr_ver));

	//true resets everything, false sets up file verify.
	plr_reset_data(true);

	// the point of all this: read in the PLR contents
	handler->beginSectionRead();
	while (handler->hasMoreSections()) {
		auto section_id = handler->nextSection();
		try {
			switch (section_id) {
				case Section::Flags:
					mprintf(("PLR => Parsing:  Flags...\n"));
					plr_read_flags();
					break;

				case Section::Info:
					mprintf(("PLR => Parsing:  Info...\n"));
					plr_read_info();
					break;

				case Section::Variables:
					mprintf(("PLR => Parsing:  Variables...\n"));
					plr_read_variables();
					break;

				case Section::Containers:
					mprintf(("PLR => Parsing:  Containers...\n"));
					plr_read_containers();
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

				case Section::Invalid:
					plr_close();
					return false;

				default:
					mprintf(("PLR => Skipping unknown section 0x%04x!\n", (uint32_t)section_id));
					break;
			}
		} catch (const cfile::max_read_length &msg) {
			// read to max section size, move to next section, discarding
			// extra/unknown data
			mprintf(("PLR => (0x%04x) %s\n", (uint32_t)section_id, msg.what()));
		} catch (const char *err) {
			mprintf(("PLR => ERROR: %s\n", err));
			plr_close();
			return false;
		}
	}
	handler->endSectionRead();

	// restore the callsign into the Player structure
	strcpy_s(p->callsign, callsign);

	// restore the truncated callsign into Player structure
	pilot_set_short_callsign(p, SHORT_CALLSIGN_PIXEL_W);

	player_set_squad_bitmap(p, p->m_squad_filename, true);

	hud_squadmsg_save_keys();

	// Flags to signal the main UI the state of the loaded player file
	// Do these here after player_read_flags() so they don't get trashed!
	if (plr_ver < 4) {
		p->save_flags |= PLAYER_FLAGS_PLR_VER_PRE_CONTROLS5;
	}

	if (plr_ver < PLR_VERSION) {
		p->flags |= PLAYER_FLAGS_PLR_VER_IS_LOWER;

	} else if (plr_ver > PLR_VERSION) {
		p->flags |= PLAYER_FLAGS_PLR_VER_IS_HIGHER;
	}

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
	if ( filename.empty() ) {
		mprintf(("PLR => Invalid filename '%s'!\n", filename.c_str()));
		return false;
	}

	// We always write the player file as JSON now
	filename += ".json";

	// open it, hopefully...
	auto fp = cfopen(filename.c_str(), "wb", CFILE_NORMAL, CF_TYPE_PLAYERS, false,
	                 CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);

	if ( !fp ) {
		mprintf(("PLR => Unable to open '%s' for saving!\n", filename.c_str()));
		return false;
	}

	try {
		handler.reset(new pilot::JSONFileHandler(fp, false));
	} catch (const std::exception& e) {
		mprintf(("PLR => Failed to parse JSON: %s\n", e.what()));
		return false;
	}

	// header and version
	handler->writeInt("signature", PLR_FILE_ID);
	handler->writeUByte("version", PLR_VERSION);

	mprintf(("PLR => Saving '%s' with version %d...\n", filename.c_str(), (int)PLR_VERSION));

	handler->beginWritingSections();

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
	mprintf(("PLR => Saving:  Containers...\n"));
	plr_write_containers();
	mprintf(("PLR => Saving:  Multiplayer...\n"));
	plr_write_multiplayer();
	mprintf(("PLR => Saving:  Controls...\n"));
	plr_write_controls();
	mprintf(("PLR => Saving:  Settings...\n"));
	plr_write_settings();

	handler->endWritingSections();

	handler->flush();

	// Done!
	mprintf(("PLR => Saving complete!\n"));

	plr_close();

	return true;
}

bool pilotfile::verify(const char *fname, int *rank, char *valid_language, int* flags)
{
	player t_plr;

	// set player ptr first thing
	p = &t_plr;

	filename = fname;

	if ( filename.size() == 4 ) {
		mprintf(("PLR => Invalid filename '%s'!\n", filename.c_str()));
		return false;
	}

	auto fp = cfopen(filename.c_str(), "rb", CFILE_NORMAL, CF_TYPE_PLAYERS, false,
	                 CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);

	if ( !fp ) {
		mprintf(("PLR => Unable to open '%s'!\n", filename.c_str()));
		return false;
	}

	try {
		handler.reset(new pilot::JSONFileHandler(fp, true));
	} catch (const std::exception& e) {
		mprintf(("PLR => Failed to parse JSON: %s\n", e.what()));
		return false;
	}

	unsigned int plr_id = handler->readUInt("signature");

	if (plr_id != PLR_FILE_ID) {
		mprintf(("PLR => Invalid header id for '%s'!\n", filename.c_str()));
		plr_close();
		return false;
	}

	// version, now used
	plr_ver = handler->readUByte("version");

	mprintf(("PLR => Verifying '%s' with version %d...\n", filename.c_str(), plr_ver));

	// true resets everything, false sets up file verify.
	plr_reset_data(false);

	bool have_flags = false;
	bool have_info = false;
	// the point of all this: read in the PLR contents
	handler->beginSectionRead();
	while (!(have_flags && have_info) && handler->hasMoreSections()) {
		auto section_id = handler->nextSection();
		try {
			switch (section_id) {
				case Section::Flags:
					mprintf(("PLR => Parsing:  Flags...\n"));
					have_flags = true;
					plr_read_flags();
					break;

				// now reading the Info section to get the campaign
				// and be able to lookup the campaign rank
				case Section::Info:
					mprintf(("PLR => Parsing:  Info...\n"));
					have_info = true;
					plr_read_info();
					break;

				case Section::Invalid:
					plr_close();
					return false;

				default:
					break;
			}
		} catch (cfile::max_read_length &msg) {
			// read to max section size, move to next section, discarding
			// extra/unknown data
			mprintf(("PLR => (0x%04x) %s\n", (uint32_t)section_id, msg.what()));
		} catch (const char *err) {
			mprintf(("PLR => ERROR: %s\n", err));
			plr_close();
			return false;
		}
	}
	handler->endSectionRead();

	// Flags to signal the main UI the state of the loaded player file
	// Do these here after player_read_flags() so they don't get trashed!
	if (plr_ver < 4) {
		p->save_flags |= PLAYER_FLAGS_PLR_VER_PRE_CONTROLS5;
	}

	if (plr_ver < PLR_VERSION) {
		p->flags |= PLAYER_FLAGS_PLR_VER_IS_LOWER;

	} else if (plr_ver > PLR_VERSION) {
		p->flags |= PLAYER_FLAGS_PLR_VER_IS_HIGHER;
	}

	if (valid_language) {
		strcpy(valid_language, p->language);
	}

	// need to cleanup early to ensure everything is OK for use in the CSG next
	// also means we can't use *p from now on, use t_plr instead for a few vars

	// Save any player flags, if caller wants them
	if (flags != nullptr) {
		*flags = p->flags;
	}

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
