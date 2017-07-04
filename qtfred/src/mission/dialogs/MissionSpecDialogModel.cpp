//
//

#include "MissionSpecDialogModel.h"
#include "ui\dialogs\MissionSpecDialog.h"

#include "ship/ship.h"
#include "gamesnd/eventmusic.h"
#include "cfile/cfile.h"
#include "mission/missionmessage.h"

#include <QtWidgets>

namespace fso {
namespace fred {
namespace dialogs {

#define NO_SQUAD	"<none>"

MissionSpecDialogModel::MissionSpecDialogModel(QObject* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport) {
	initializeData();
}

void MissionSpecDialogModel::initializeData() {
	int i, box_index = 0, mission_command_persona_box_index = -1;

	_m_mission_title = The_mission.name;
	_m_designer_name = The_mission.author;
	_m_created = The_mission.created;
	_m_modified = The_mission.modified;
	_m_mission_notes = The_mission.notes;
	_m_mission_desc = The_mission.mission_desc;
	_m_red_alert = (The_mission.flags[Mission::Mission_Flags::Red_alert]) ? 1 : 0;
	_m_scramble = (The_mission.flags[Mission::Mission_Flags::Scramble]) ? 1 : 0;
	_m_full_war = Mission_all_attack;
	_m_daisy_chained_docking = (The_mission.flags[Mission::Mission_Flags::Allow_dock_trees]) ? 1 : 0;
	_m_disallow_support = (The_mission.support_ships.max_support_ships == 0) ? 1 : 0;
	_m_no_promotion = (The_mission.flags[Mission::Mission_Flags::No_promotion]) ? 1 : 0;
	_m_no_builtin_msgs = (The_mission.flags[Mission::Mission_Flags::No_builtin_msgs]) ? 1 : 0;
	_m_no_builtin_command_msgs = (The_mission.flags[Mission::Mission_Flags::No_builtin_command]) ? 1 : 0;
	_m_no_traitor = (The_mission.flags[Mission::Mission_Flags::No_traitor]) ? 1 : 0;
	_m_toggle_trails = (The_mission.flags[Mission::Mission_Flags::Toggle_ship_trails]) ? 1 : 0;
	_m_support_repairs_hull = (The_mission.flags[Mission::Mission_Flags::Support_repairs_hull]) ? 1 : 0;
	_m_beam_free_all_by_default = (The_mission.flags[Mission::Mission_Flags::Beam_free_all_by_default]) ? 1 : 0;
	_m_player_start_using_ai = (The_mission.flags[Mission::Mission_Flags::Player_start_ai]) ? 1 : 0;
	_m_no_briefing = (The_mission.flags[Mission::Mission_Flags::No_briefing]) ? 1 : 0;
	_m_no_debriefing = (The_mission.flags[Mission::Mission_Flags::Toggle_debriefing]) ? 1 : 0;
	_m_autpilot_cinematics = (The_mission.flags[Mission::Mission_Flags::Use_ap_cinematics]) ? 1 : 0;
	_m_2d_mission = (The_mission.flags[Mission::Mission_Flags::Mission_2d]) ? 1 : 0;
	_m_no_autpilot = (The_mission.flags[Mission::Mission_Flags::Deactivate_ap]) ? 1 : 0;
	_m_always_show_goals = (The_mission.flags[Mission::Mission_Flags::Always_show_goals]) ? 1 : 0;

	_m_flags = The_mission.flags;

	_m_loading_640 = The_mission.loading_screen[GR_640];
	_m_loading_1024 = The_mission.loading_screen[GR_1024];

	
	for (i = 0; i < Num_ai_profiles; i++) {
		_ai_profiles.push_back(Ai_profiles[i].profile_name);
	}

	_event_music.push_back("None");
	for (i = 0; i < Num_soundtracks; i++) {
		_event_music.push_back(Soundtracks[i].name);
	}

	_sub_event_music.push_back("None");
	for (i = 0; i < Num_soundtracks; i++) {
		_sub_event_music.push_back(Soundtracks[i].name);
	}

	for (i = 0; i < Num_personas; i++) {
		if (Personas[i].flags & PERSONA_FLAG_COMMAND) {
			_command_personas.push_back(Personas[i].name);
			if (i == The_mission.command_persona)
				mission_command_persona_box_index = box_index;
			box_index++;
		}
	}

	_command_senders.push_back(DEFAULT_COMMAND);
	for (i = 0; i < MAX_SHIPS; i++) {
		if (Ships[i].objnum >= 0)
			if (Ship_info[Ships[i].ship_info_index].is_huge_ship())
				_command_senders.push_back(Ships[i].ship_name);
	}

	// squad info
	if (strlen(The_mission.squad_name) > 0) { //-V805
		_m_squad_name = The_mission.squad_name;
		_m_squad_filename = The_mission.squad_filename;
	}
	else {
		_m_squad_name = NO_SQUAD;
		_m_squad_filename = "";
	}

	_m_type = The_mission.game_type;
	_m_ai_profile = AI_PROFILES_INDEX(The_mission.ai_profile);

	_m_event_music = Current_soundtrack_num + 1;
	_m_substitute_event_music = The_mission.substitute_event_music_name;

	_m_command_persona = mission_command_persona_box_index;
	_m_command_sender = The_mission.command_sender;

	_m_num_respawns = The_mission.num_respawns;
	_m_max_respawn_delay = The_mission.max_respawn_delay;
	_m_max_hull_repair_val = The_mission.support_ships.max_hull_repair_val;
	_m_max_subsys_repair_val = The_mission.support_ships.max_subsys_repair_val;

	_m_contrail_threshold = The_mission.contrail_threshold;
	_m_contrail_threshold_flag = (_m_contrail_threshold != CONTRAIL_THRESHOLD_DEFAULT);

	modelChanged();
}

bool MissionSpecDialogModel::apply() {
	int new_m_type, is_multi = 0, is_training = 0, is_single = 0;
	flagset<Mission::Mission_Flags> flags;

	// deal with changing the mission type.  Code is done this way since training missions
	// just override anything else.
	new_m_type = _m_type;

	if (!new_m_type) {
		auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error, "Invalid mission type", "You must select the game type: training, single, or multiplayer",
		{ DialogButton::Ok });
		if (button == DialogButton::Ok) {
			return false;
		}
	}

	The_mission.game_type = new_m_type;
	The_mission.num_respawns = _m_num_respawns;
	The_mission.max_respawn_delay = _m_max_respawn_delay;
	The_mission.support_ships.max_support_ships = (_m_disallow_support) ? 0 : -1;
	The_mission.support_ships.max_hull_repair_val = _m_max_hull_repair_val;
	The_mission.support_ships.max_subsys_repair_val = _m_max_subsys_repair_val;

	flags = The_mission.flags;

	/*// set flags for red alert
	The_mission.flags.set(Mission::Mission_Flags::Red_alert, _m_red_alert != 0);

	// set flags for scramble
	The_mission.flags.set(Mission::Mission_Flags::Scramble, _m_scramble != 0);

	// set flags for dock trees
	The_mission.flags.set(Mission::Mission_Flags::Allow_dock_trees, _m_daisy_chained_docking != 0);

	// set the flags for no promotion
	The_mission.flags.set(Mission::Mission_Flags::No_promotion, _m_no_promotion != 0);

	// set flags for no builtin messages
	The_mission.flags.set(Mission::Mission_Flags::No_builtin_msgs, _m_no_builtin_msgs != 0);

	// set flags for no builtin command messages
	The_mission.flags.set(Mission::Mission_Flags::No_builtin_command, _m_no_builtin_command_msgs != 0);

	// set no traitor flags
	The_mission.flags.set(Mission::Mission_Flags::No_traitor, _m_no_traitor != 0);

	//set ship trail flags
	The_mission.flags.set(Mission::Mission_Flags::Toggle_ship_trails, _m_toggle_trails != 0);
	*/
	// set ship trail threshold
	if (_m_contrail_threshold_flag) {
		The_mission.contrail_threshold = _m_contrail_threshold;
	} else {
		The_mission.contrail_threshold = CONTRAIL_THRESHOLD_DEFAULT;
	}
	/*
	//set support ship repairing flags
	The_mission.flags.set(Mission::Mission_Flags::Support_repairs_hull, _m_support_repairs_hull != 0);

	// set default beam free
	The_mission.flags.set(Mission::Mission_Flags::Beam_free_all_by_default, _m_beam_free_all_by_default != 0);

	// set player AI by default
	The_mission.flags.set(Mission::Mission_Flags::Player_start_ai, _m_player_start_using_ai != 0);

	// set briefing
	The_mission.flags.set(Mission::Mission_Flags::No_briefing, _m_no_briefing != 0);

	// set debriefing
	The_mission.flags.set(Mission::Mission_Flags::Toggle_debriefing, _m_no_debriefing != 0);

	// set autopilot cinematics
	The_mission.flags.set(Mission::Mission_Flags::Use_ap_cinematics, _m_autpilot_cinematics != 0);

	// 2D mission
	The_mission.flags.set(Mission::Mission_Flags::Mission_2d, _m_2d_mission != 0);

	// set autopilot disabled
	The_mission.flags.set(Mission::Mission_Flags::Deactivate_ap, _m_no_autpilot != 0);

	// always show mission goals
	The_mission.flags.set(Mission::Mission_Flags::Always_show_goals, _m_always_show_goals != 0);

	// End to mainhall
	The_mission.flags.set(Mission::Mission_Flags::End_to_mainhall, _m_end_to_mainhall != 0);*/

	The_mission.flags = _m_flags;

	if (flags != The_mission.flags) {
		// Doc has been changed
	}

	strncpy(The_mission.name, _m_mission_title.c_str(), NAME_LENGTH-1);
	strncpy(The_mission.author, _m_designer_name.c_str(), NAME_LENGTH-1);
	strncpy(The_mission.loading_screen[GR_640], _m_loading_640.c_str(), NAME_LENGTH-1);
	strncpy(The_mission.loading_screen[GR_1024], _m_loading_1024.c_str(), NAME_LENGTH-1);
	strncpy(The_mission.notes, _m_mission_notes.c_str(), NOTES_LENGTH);
	strncpy(The_mission.mission_desc, _m_mission_desc.c_str(), MISSION_DESC_LENGTH);

	// copy squad stuff
	if (_m_squad_name == NO_SQUAD) {
		strcpy_s(The_mission.squad_name, "");
		strcpy_s(The_mission.squad_filename, "");
	} else {
		strncpy(The_mission.squad_name, _m_squad_name.c_str(), NAME_LENGTH);
		strncpy(The_mission.squad_filename, _m_squad_filename.c_str(), MAX_FILENAME_LEN);
	}

	The_mission.ai_profile = &Ai_profiles[_m_ai_profile];

	Current_soundtrack_num = _m_event_music - 1;
	strcpy_s(The_mission.substitute_event_music_name, _m_substitute_event_music.c_str());

	The_mission.command_persona = _m_command_persona;
	if (_m_command_sender.at(0) == '#')
		strcpy_s(The_mission.command_sender, _m_command_sender.substr(1).c_str());
	else
		strcpy_s(The_mission.command_sender, _m_command_sender.c_str());

	Mission_all_attack = (int)_m_full_war;

	// update the Nu_m_teams variable accoriding to mission types
	Num_teams = 1;
	if ((The_mission.game_type & MISSION_TYPE_MULTI) && (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS)) {
		Num_teams = 2;
	}

	return true;
}

void MissionSpecDialogModel::reject() {

}

SCP_string MissionSpecDialogModel::getMissionTitle() {
	return _m_mission_title;
}

void MissionSpecDialogModel::setMissionTitle(SCP_string m_mission_title){
	this->_m_mission_title = m_mission_title;
}

SCP_string MissionSpecDialogModel::getDesigner() {
	return _m_designer_name;
}

void MissionSpecDialogModel::setDesigner(SCP_string m_designer_name) {
	this->_m_designer_name = m_designer_name;
	modelChanged();
}

const SCP_vector<SCP_string>& MissionSpecDialogModel::getAIProfiles() const {
	return _ai_profiles;
}

void MissionSpecDialogModel::setMissionType(int m_type) {
	if (m_type & (MISSION_TYPE_SINGLE |
		MISSION_TYPE_MULTI |
		MISSION_TYPE_TRAINING |
		MISSION_TYPE_MULTI_COOP |
		MISSION_TYPE_MULTI_DOGFIGHT |
		MISSION_TYPE_MULTI_TEAMS)) {
		_m_type = m_type;
		modelChanged();
	} else {
		auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Error, "Invalid mission type", "You must select the game type: training, single, or multiplayer",
		{ DialogButton::Ok });
		if (button == DialogButton::Ok) {
			return;
		}
	}
}

int MissionSpecDialogModel::getMissionType() {
	return _m_type;
}

void MissionSpecDialogModel::setMissionFlag(Mission::Mission_Flags flag, bool enabled) {
	_m_flags.set(flag, enabled);
	modelChanged();
}

const flagset<Mission::Mission_Flags>& MissionSpecDialogModel::getMissionFlags() const {
	return _m_flags;
}

void MissionSpecDialogModel::setMissionFullWar(bool enabled) {
	_m_full_war = enabled;
	_m_flags.set(Mission::Mission_Flags::All_attack, enabled);
	modelChanged();
}

bool MissionSpecDialogModel::query_modified() {
	return true;
}

}
}
}