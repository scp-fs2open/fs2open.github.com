//
//

#if defined(_MSC_VER) && _MSC_VER <= 1920
	// work around MSVC 2015 and 2017 compiler bug
	// https://bugreports.qt.io/browse/QTBUG-72073
	#define QT_NO_FLOAT16_OPERATORS
#endif

#include "MissionSpecDialogModel.h"
#include "ui/dialogs/MissionSpecDialog.h"

#include "cfile/cfile.h"
#include "mission/missionmessage.h"

#include <QtWidgets>

namespace fso {
namespace fred {
namespace dialogs {

MissionSpecDialogModel::MissionSpecDialogModel(QObject* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport) {
	initializeData();
}

void MissionSpecDialogModel::initializeData() {
	_m_mission_title = The_mission.name;
	_m_designer_name = The_mission.author;
	_m_created = The_mission.created;
	_m_modified = The_mission.modified;
	_m_mission_notes = The_mission.notes;
	_m_mission_desc = The_mission.mission_desc;

	_m_full_war = Mission_all_attack;
	_m_disallow_support = (The_mission.support_ships.max_support_ships == 0);

	_m_flags = The_mission.flags;

	_m_loading_640 = The_mission.loading_screen[GR_640];
	_m_loading_1024 = The_mission.loading_screen[GR_1024];

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

	_m_command_persona = The_mission.command_persona;
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
	int new_m_type;

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
	
	// Copy mission flags
	The_mission.flags = _m_flags;

	// set ship trail threshold
	if (_m_contrail_threshold_flag) {
		The_mission.contrail_threshold = _m_contrail_threshold;
	} else {
		The_mission.contrail_threshold = CONTRAIL_THRESHOLD_DEFAULT;
	}
	
	//if there's a odd number of quotation marks, the mission won't parse
	//If there are an even number, nothing after the first one appears
	//So just get rid of them
	Editor::strip_quotation_marks(_m_mission_title);
	Editor::strip_quotation_marks(_m_designer_name);
	Editor::strip_quotation_marks(_m_mission_notes);
	Editor::strip_quotation_marks(_m_mission_desc);
	Editor::strip_quotation_marks(_m_squad_name);

	// puts "$End Notes:" on a different line to ensure it's not interpreted as part of a comment
	Editor::pad_with_newline(_m_mission_notes, NOTES_LENGTH - 1);

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

	// update the Num_teams variable accoriding to mission types
	Num_teams = 1;
	if ((The_mission.game_type & MISSION_TYPE_MULTI) && (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS)) {
		Num_teams = 2;
	}

	return true;
}

void MissionSpecDialogModel::reject() {

}

void MissionSpecDialogModel::setMissionTitle(const SCP_string& m_mission_title) {
	modify(_m_mission_title, m_mission_title);
}

SCP_string MissionSpecDialogModel::getMissionTitle() {
	return _m_mission_title;
}

void MissionSpecDialogModel::setDesigner(const SCP_string& m_designer_name) {
	modify(_m_designer_name, m_designer_name);
}

SCP_string MissionSpecDialogModel::getDesigner() {
	return _m_designer_name;
}

SCP_string MissionSpecDialogModel::getCreatedTime() {
	return _m_created;
}

SCP_string MissionSpecDialogModel::getModifiedTime() {
	return _m_modified;
}

void MissionSpecDialogModel::setMissionType(int m_type) {
	if (m_type & (MISSION_TYPE_SINGLE |
		MISSION_TYPE_MULTI |
		MISSION_TYPE_TRAINING |
		MISSION_TYPE_MULTI_COOP |
		MISSION_TYPE_MULTI_DOGFIGHT |
		MISSION_TYPE_MULTI_TEAMS)) {
		modify(_m_type, m_type);
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

void MissionSpecDialogModel::setNumRespawns(uint m_num_respawns) {
	modify(_m_num_respawns, m_num_respawns);
}

uint MissionSpecDialogModel::getNumRespawns() {
	return _m_num_respawns;
}

void MissionSpecDialogModel::setMaxRespawnDelay(int m_max_respawn_delay) {
	modify(_m_max_respawn_delay, m_max_respawn_delay);
}

int MissionSpecDialogModel::getMaxRespawnDelay() {
	return _m_max_respawn_delay;
}

void MissionSpecDialogModel::setSquadronName(const SCP_string& m_squad_name) {
	modify(_m_squad_name, m_squad_name);
}

SCP_string MissionSpecDialogModel::getSquadronName() {
	return _m_squad_name;
}

void MissionSpecDialogModel::setSquadronLogo(const SCP_string& m_squad_filename) {
	modify(_m_squad_filename, m_squad_filename);
}

SCP_string MissionSpecDialogModel::getSquadronLogo() {
	return _m_squad_filename;
}

void MissionSpecDialogModel::setLowResLoadingScreen(const SCP_string& m_loading_640) {
	modify(_m_loading_640, m_loading_640);
}

SCP_string MissionSpecDialogModel::getLowResLoadingScren() {
	return _m_loading_640;
}

void MissionSpecDialogModel::setHighResLoadingScreen(const SCP_string& m_loading_1024) {
	modify(_m_loading_1024, m_loading_1024);
}

SCP_string MissionSpecDialogModel::getHighResLoadingScren() {
	return _m_loading_1024;
}

void MissionSpecDialogModel::setDisallowSupport(bool m_disallow_support) {
	modify(_m_disallow_support, m_disallow_support);
}

bool MissionSpecDialogModel::getDisallowSupport() {
	return _m_disallow_support;
}

void MissionSpecDialogModel::setHullRepairMax(float m_max_hull_repair_val) {
	modify(_m_max_hull_repair_val, m_max_hull_repair_val);
}

int MissionSpecDialogModel::getHullRepairMax() {
	return _m_max_hull_repair_val;
}

void MissionSpecDialogModel::setSubsysRepairMax(float m_max_subsys_repair_val){
	modify(_m_max_subsys_repair_val, m_max_subsys_repair_val);
}

int MissionSpecDialogModel::getSubsysRepairMax() {
	return _m_max_subsys_repair_val;
}

void MissionSpecDialogModel::setTrailThresholdFlag(bool m_contrail_threshold_flag) {
	modify(_m_contrail_threshold_flag, m_contrail_threshold_flag);
}

bool MissionSpecDialogModel::getTrailThresholdFlag() {
	return _m_contrail_threshold_flag;
}

void MissionSpecDialogModel::setTrailDisplaySpeed(int m_contrail_threshold) {
	modify(_m_contrail_threshold, m_contrail_threshold);
}

int MissionSpecDialogModel::getTrailDisplaySpeed() {
	return _m_contrail_threshold;
}

void MissionSpecDialogModel::setCommandSender(const SCP_string& m_command_sender) {
	modify(_m_command_sender, m_command_sender);
}

SCP_string MissionSpecDialogModel::getCommandSender() {
	return _m_command_sender;
}

void MissionSpecDialogModel::setCommandPersona(int m_command_persona) {
	modify(_m_command_persona, m_command_persona);
}

int MissionSpecDialogModel::getCommandPersona() {
	return _m_command_persona;
}

void MissionSpecDialogModel::setEventMusic(int m_event_music) {
	modify(_m_event_music, m_event_music);
}

int MissionSpecDialogModel::getEventMusic() {
	return _m_event_music;
}

void MissionSpecDialogModel::setSubEventMusic(const SCP_string& m_substitute_event_music) {
	modify(_m_substitute_event_music, m_substitute_event_music);
}

SCP_string MissionSpecDialogModel::getSubEventMusic() {
	return _m_substitute_event_music;
}

void MissionSpecDialogModel::setMissionFlag(Mission::Mission_Flags flag, bool enabled) {
	if (_m_flags[flag] != enabled) {
		_m_flags.set(flag, enabled);
		set_modified();
		modelChanged();
	}
}

const flagset<Mission::Mission_Flags>& MissionSpecDialogModel::getMissionFlags() const {
	return _m_flags;
}

void MissionSpecDialogModel::setMissionFullWar(bool enabled) {
	if (_m_full_war != enabled) {
		_m_full_war = enabled;
		_m_flags.set(Mission::Mission_Flags::All_attack, enabled);
		set_modified();
		modelChanged();
	}
}

void MissionSpecDialogModel::setAIProfileIndex(int m_ai_profile) {
	modify(_m_ai_profile, m_ai_profile);
}

int MissionSpecDialogModel::getAIProfileIndex() const {
	return _m_ai_profile;
}

void MissionSpecDialogModel::setMissionDescText(const SCP_string& m_mission_desc) {
	modify(_m_mission_desc, m_mission_desc.substr(0, MIN(MISSION_DESC_LENGTH, m_mission_desc.length())));
}

SCP_string MissionSpecDialogModel::getMissionDescText() {
	return _m_mission_desc;
}

void MissionSpecDialogModel::setDesignerNoteText(const SCP_string& m_mission_notes) {
	modify(_m_mission_notes, m_mission_notes.substr(0, MIN(NOTES_LENGTH, m_mission_notes.length())));
}

SCP_string MissionSpecDialogModel::getDesignerNoteText() {
	return _m_mission_notes;
}

void MissionSpecDialogModel::set_modified() {
	if (!_modified) {
		_modified = true;
	}
}

bool MissionSpecDialogModel::query_modified() {
	return _modified;
}

}
}
}
