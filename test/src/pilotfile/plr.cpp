#include "math/vecmat.h"
#include "object/object.h"
#include "pilotfile/pilotfile.h"
#include "playerman/player.h"
#include "util/FSTestFixture.h"

#include <gtest/gtest.h>
#include <jansson.h>

class PilotPlayerFileTest: public test::FSTestFixture {
 public:
	PilotPlayerFileTest() : test::FSTestFixture(INIT_CFILE | INIT_GRAPHICS | INIT_FONTS) {
		pushModDir("pilotfile");
	}

 protected:
	void TearDown() override {
		// Delete the converted files
		cf_delete("asdf.json", CF_TYPE_PLAYERS, CF_LOCATION_ROOT_USER | CF_LOCATION_ROOT_GAME | CF_LOCATION_TYPE_ROOT);

		FSTestFixture::TearDown();
	}
};

template<typename T, size_t SIZE>
std::ostream& array_print(std::ostream& out, const T (& data)[SIZE]) {
	auto first = true;
	out << "(";
	for (auto& el : data) {
		if (!first) {
			out << ", ";
		}
		out << el;
		first = false;
	}
	out << ")";
	return out;
};

template<typename T>
std::ostream& operator<<(std::ostream& out, const SCP_vector<T>& data) {
	auto first = true;
	out << "(";
	for (auto& el : data) {
		if (!first) {
			out << ", ";
		}
		out << el;
		first = false;
	}
	out << ")";
	return out;
};

std::ostream& operator<<(std::ostream& out, const scoring_struct& data) {
	out << "flags:" << data.flags << "\n";
	out << "score:" << data.score << "\n";
	out << "rank:" << data.rank << "\n";
	out << "medal_counts:" << data.medal_counts << "\n";
	out << "kills:";
	array_print(out, data.kills) << "\n";
	out << "assists:" << data.assists << "\n";
	out << "kill_count:" << data.kill_count << "\n";
	out << "kill_count_ok:" << data.kill_count_ok << "\n";
	out << "p_shots_fired:" << data.p_shots_fired << "\n";
	out << "s_shots_fired:" << data.s_shots_fired << "\n";
	out << "p_shots_hit:" << data.p_shots_hit << "\n";
	out << "s_shots_hit:" << data.s_shots_hit << "\n";
	out << "p_bonehead_hits:" << data.p_bonehead_hits << "\n";
	out << "s_bonehead_hits:" << data.s_bonehead_hits << "\n";
	out << "bonehead_kills:" << data.bonehead_kills << "\n";
	out << "missions_flown:" << data.missions_flown << "\n";
	out << "flight_time:" << data.flight_time << "\n";
	out << "last_flown:" << data.last_flown << "\n";
	out << "last_backup:" << data.last_backup << "\n";
	out << "m_medal_earned:" << data.m_medal_earned << "\n";
	out << "m_badge_earned:" << data.m_badge_earned << "\n";
	out << "m_promotion_earned:" << data.m_promotion_earned << "\n";
	out << "m_score:" << data.m_score << "\n";
	out << "m_kills:";
	array_print(out, data.m_kills) << "\n";
	out << "m_okKills:";
	array_print(out, data.m_okKills) << "\n";
	out << "m_kill_count:" << data.m_kill_count << "\n";
	out << "m_kill_count_ok:" << data.m_kill_count_ok << "\n";
	out << "m_assists:" << data.m_assists << "\n";
	out << "mp_shots_fired:" << data.mp_shots_fired << "\n";
	out << "ms_shots_fired:" << data.ms_shots_fired << "\n";
	out << "mp_shots_hit:" << data.mp_shots_hit << "\n";
	out << "ms_shots_hit:" << data.ms_shots_hit << "\n";
	out << "mp_bonehead_hits:" << data.mp_bonehead_hits << "\n";
	out << "ms_bonehead_hits:" << data.ms_bonehead_hits << "\n";
	out << "m_bonehead_kills:" << data.m_bonehead_kills << "\n";
	out << "m_player_deaths:" << data.m_player_deaths << "\n";
	out << "m_dogfight_kills:";
	array_print(out, data.m_dogfight_kills) << "\n";


	return out;
}
std::ostream& operator<<(std::ostream& out, const control_info& data) {
	out << "pitch:" << data.pitch << "\n";
	out << "vertical:" << data.vertical << "\n";
	out << "heading:" << data.heading << "\n";
	out << "sideways:" << data.sideways << "\n";
	out << "bank:" << data.bank << "\n";
	out << "forward:" << data.forward << "\n";
	out << "forward_cruise_percent:" << data.forward_cruise_percent << "\n";
	out << "fire_primary_count:" << data.fire_primary_count << "\n";
	out << "fire_secondary_count:" << data.fire_secondary_count << "\n";
	out << "fire_countermeasure_count:" << data.fire_countermeasure_count << "\n";
	out << "fire_debug_count:" << data.fire_debug_count << "\n";
	out << "afterburner_start:" << data.afterburner_start << "\n";
	out << "afterburner_stop:" << data.afterburner_stop << "\n";

	return out;
}
std::ostream& operator<<(std::ostream& out, const button_info& data) {
	return array_print(out, data.status);
}
std::ostream& operator<<(std::ostream& out, const multi_local_options& data) {
	out << "flags:" << data.flags << "\n";
	out << "obj_update_level:" << data.obj_update_level << "\n";
	return out;
}
std::ostream& operator<<(std::ostream& out, const multi_server_options& data) {
	out << "squad_set:" << (int) data.squad_set << "\n";
	out << "endgame_set:" << (int) data.endgame_set << "\n";
	out << "flags:" << data.flags << "\n";
	out << "respawn:" << data.respawn << "\n";
	out << "max_observers:" << (int) data.max_observers << "\n";
	out << "skill_level:" << (int) data.skill_level << "\n";
	out << "voice_qos:" << (int) data.voice_qos << "\n";
	out << "voice_token_wait:" << data.voice_token_wait << "\n";
	out << "voice_record_time:" << data.voice_record_time << "\n";
	out << "mission_time_limit:" << data.mission_time_limit << "\n";
	out << "kill_limit:" << data.kill_limit << "\n";

	return out;
}
std::ostream& operator<<(std::ostream& out, const sexp_variable& data) {
	out << "type: " << data.type << "\n";
	out << "text: " << data.text << "\n";
	out << "variable_name: " << data.variable_name << "\n";

	return out;
}
std::ostream& operator<<(std::ostream& out, const htarget_list& data) {
	out << "how_added:" << data.how_added << "\n";
	out << "objp:" << OBJ_INDEX(data.objp) << "\n";

	return out;
}

std::ostream& operator<<(std::ostream& out, const player& plr) {
	out << "callsign:" << plr.callsign << "\n";
	out << "short_callsign:" << plr.short_callsign << "\n";
	out << "short_callsign_width:" << plr.short_callsign_width << "\n";
	out << "image_filename:" << plr.image_filename << "\n";
	out << "s_squad_filename:" << plr.s_squad_filename << "\n";
	out << "s_squad_name:" << plr.s_squad_name << "\n";
	out << "m_squad_filename:" << plr.m_squad_filename << "\n";
	out << "m_squad_name:" << plr.m_squad_name << "\n";
	out << "current_campaign:" << plr.current_campaign << "\n";
	out << "readyroom_listing_mode:" << plr.readyroom_listing_mode << "\n";
	out << "flags:" << plr.flags << "\n";
	out << "save_flags:" << plr.save_flags << "\n";
	out << "keyed_targets:";
	array_print(out, plr.keyed_targets) << "\n";
	out << "current_hotkey_set:" << plr.current_hotkey_set << "\n";
	out << "lead_target_pos:" << plr.lead_target_pos << "\n";
	out << "lead_target_cheat:" << plr.lead_target_cheat << "\n";
	out << "lead_indicator_active:" << plr.lead_indicator_active << "\n";
	out << "lock_indicator_x:" << plr.lock_indicator_x << "\n";
	out << "lock_indicator_y:" << plr.lock_indicator_y << "\n";
	out << "lock_indicator_start_x:" << plr.lock_indicator_start_x << "\n";
	out << "lock_indicator_start_y:" << plr.lock_indicator_start_y << "\n";
	out << "lock_indicator_visible:" << plr.lock_indicator_visible << "\n";
	out << "lock_time_to_target:" << plr.lock_time_to_target << "\n";
	out << "lock_dist_to_target:" << plr.lock_dist_to_target << "\n";
	out << "last_ship_flown_si_index:" << plr.last_ship_flown_si_index << "\n";
	out << "objnum:" << plr.objnum << "\n";
	out << "bi:" << plr.bi << "\n";
	out << "ci:" << plr.ci << "\n";
	out << "stats:" << plr.stats << "\n";
	out << "friendly_hits:" << plr.friendly_hits << "\n";
	out << "friendly_damage:" << plr.friendly_damage << "\n";
	out << "friendly_last_hit_time:" << plr.friendly_last_hit_time << "\n";
	out << "last_warning_message_time:" << plr.last_warning_message_time << "\n";
	out << "control_mode:" << plr.control_mode << "\n";
	out << "saved_viewer_mode:" << plr.saved_viewer_mode << "\n";
	out << "check_warn_timestamp:" << plr.check_warn_timestamp << "\n";
	out << "distance_warning_count:" << plr.distance_warning_count << "\n";
	out << "distance_warning_time:" << plr.distance_warning_time << "\n";
	out << "allow_warn_timestamp:" << plr.allow_warn_timestamp << "\n";
	out << "warn_count:" << plr.warn_count << "\n";
	out << "damage_this_burst:" << plr.damage_this_burst << "\n";
	out << "repair_sound_loop:" << plr.repair_sound_loop << "\n";
	out << "cargo_scan_loop:" << plr.cargo_scan_loop << "\n";
	out << "praise_count:" << plr.praise_count << "\n";
	out << "allow_praise_timestamp:" << plr.allow_praise_timestamp << "\n";
	out << "praise_delay_timestamp:" << plr.praise_delay_timestamp << "\n";
	out << "ask_help_count:" << plr.ask_help_count << "\n";
	out << "allow_ask_help_timestamp:" << plr.allow_ask_help_timestamp << "\n";
	out << "scream_count:" << plr.scream_count << "\n";
	out << "allow_scream_timestamp:" << plr.allow_scream_timestamp << "\n";
	out << "low_ammo_complaint_count:" << plr.low_ammo_complaint_count << "\n";
	out << "allow_ammo_timestamp:" << plr.allow_ammo_timestamp << "\n";
	out << "praise_self_count:" << plr.praise_self_count << "\n";
	out << "praise_self_timestamp:" << plr.praise_self_timestamp << "\n";
	out << "subsys_in_view:" << plr.subsys_in_view << "\n";
	out << "request_repair_timestamp:" << plr.request_repair_timestamp << "\n";
	out << "cargo_inspect_time:" << plr.cargo_inspect_time << "\n";
	out << "target_is_dying:" << plr.target_is_dying << "\n";
	out << "current_target_sx:" << plr.current_target_sx << "\n";
	out << "current_target_sy:" << plr.current_target_sy << "\n";
	out << "target_in_lock_cone:" << plr.target_in_lock_cone << "\n";
	out << "locking_subsys:" << plr.locking_subsys << "\n";
	out << "locking_subsys_parent:" << plr.locking_subsys_parent << "\n";
	out << "locking_on_center:" << plr.locking_on_center << "\n";
	out << "killer_objtype:" << plr.killer_objtype << "\n";
	out << "killer_species:" << plr.killer_species << "\n";
	out << "killer_weapon_index:" << plr.killer_weapon_index << "\n";
	out << "killer_parent_name:" << plr.killer_parent_name << "\n";
	out << "check_for_all_alone_msg:" << plr.check_for_all_alone_msg << "\n";
	out << "update_dumbfire_time:" << plr.update_dumbfire_time << "\n";
	out << "update_lock_time:" << plr.update_lock_time << "\n";
	out << "threat_flags:" << plr.threat_flags << "\n";
	out << "auto_advance:" << plr.auto_advance << "\n";
	out << "m_local_options:" << plr.m_local_options << "\n";
	out << "m_server_options:" << plr.m_server_options << "\n";
	out << "insignia_texture:" << plr.insignia_texture << "\n";
	out << "tips:" << plr.tips << "\n";
	out << "shield_penalty_stamp:" << plr.shield_penalty_stamp << "\n";
	out << "failures_this_session:" << plr.failures_this_session << "\n";
	out << "show_skip_popup:" << (int) plr.show_skip_popup << "\n";
	out << "variables:" << plr.variables << "\n";
	out << "death_message:" << plr.death_message << "\n";
	out << "lua_ci:" << plr.lua_ci << "\n";
	out << "lua_bi:" << plr.lua_bi << "\n";
	out << "lua_bi_full:" << plr.lua_bi_full << "\n";
	out << "player_was_multi:" << plr.player_was_multi << "\n";
	out << "language:" << plr.language << "\n";

	return out;
}

TEST_F(PilotPlayerFileTest, binaryToJSONConversion) {
	// Init control_config stuff
	control_config_common_init();

	// Call the conversion function
	convert_pilot_files();

	// Now check if the pilot files before and after are the same
	pilotfile loader;

	// First read the binary file
	player binary_plr;
	binary_plr.reset();

	player json_plr;
	json_plr.reset();

	// Make sure that the blank pilots are the same to make sure we don't operate on uninitialized data
	ASSERT_EQ(binary_plr, json_plr);

	ASSERT_TRUE(loader.load_player("asdf", &binary_plr, true));

	// Then read the json file
	ASSERT_TRUE(loader.load_player("asdf", &json_plr, false));

	ASSERT_EQ(binary_plr, json_plr);

	// Close control_config stuff
	control_config_common_close();
}
