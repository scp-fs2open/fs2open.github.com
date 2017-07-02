//
//

#include "MissionSpecDialogModel.h"

#include "ship\ship.h"
#include "gamesnd/eventmusic.h"
#include "cfile/cfile.h"
#include "mission/missionparse.h"
#include "mission/missionmessage.h"

namespace fso {
namespace fred {
namespace dialogs {

MissionSpecDialogModel::MissionSpecDialogModel(QObject* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport) {

}

bool MissionSpecDialogModel::apply() {
	/*int new_m_type, is_multi = 0, is_training = 0, is_single = 0;
	flagset<Mission::Mission_Flags> flags;

	UpdateData();
	is_single = (((CButton *)GetDlgItem(IDC_SINGLE))->GetCheck() == 1);
	is_multi = (((CButton *)GetDlgItem(IDC_MULTI))->GetCheck() == 1);
	is_training = (((CButton *)GetDlgItem(IDC_TRAINING))->GetCheck() == 1);

	// deal with changing the mission type.  Code is done this way since training missions
	// just override anything else.
	new_m_type = 0;
	if (is_training) {
		new_m_type = MISSION_TYPE_TRAINING;
	}
	else {
		if (is_single) {
			new_m_type = MISSION_TYPE_SINGLE;
		}

		if (is_multi) {
			new_m_type |= MISSION_TYPE_MULTI;
			if (coop->GetCheck()) {
				new_m_type |= MISSION_TYPE_MULTI_COOP;
			}
			else if (team->GetCheck()) {
				new_m_type |= MISSION_TYPE_MULTI_TEAMS;
			}
			else if (dogfight->GetCheck()) {
				new_m_type |= MISSION_TYPE_MULTI_DOGFIGHT;
			}
			else {
				Int3();			// get allender -- multiplayer mode not set!!!
			}
		}
	}

	if (!new_m_type) {
		MessageBox("You must select the game type: training, single, or multiplayer", "Error");
		return;
	}

	MODIFY(The_mission.game_type, new_m_type);
	MODIFY(The_mission.num_respawns, (int)m_num_respawns);
	MODIFY(The_mission.max_respawn_delay, m_max_respawn_delay);
	MODIFY(The_mission.support_ships.max_support_ships, (m_disallow_support) ? 0 : -1);
	MODIFY(The_mission.support_ships.max_hull_repair_val, m_max_hull_repair_val);
	MODIFY(The_mission.support_ships.max_subsys_repair_val, m_max_subsys_repair_val);

	flags = The_mission.flags;

	// set flags for red alert
	The_mission.flags.set(Mission::Mission_Flags::Red_alert, m_red_alert != 0);

	// set flags for scramble
	The_mission.flags.set(Mission::Mission_Flags::Scramble, m_scramble != 0);

	// set flags for dock trees
	The_mission.flags.set(Mission::Mission_Flags::Allow_dock_trees, m_daisy_chained_docking != 0);

	// set the flags for no promotion
	The_mission.flags.set(Mission::Mission_Flags::No_promotion, m_no_promotion != 0);

	// set flags for no builtin messages
	The_mission.flags.set(Mission::Mission_Flags::No_builtin_msgs, m_no_builtin_msgs != 0);

	// set flags for no builtin command messages
	The_mission.flags.set(Mission::Mission_Flags::No_builtin_command, m_no_builtin_command_msgs != 0);

	// set no traitor flags
	The_mission.flags.set(Mission::Mission_Flags::No_traitor, m_no_traitor != 0);

	//set ship trail flags
	The_mission.flags.set(Mission::Mission_Flags::Toggle_ship_trails, m_toggle_trails != 0);

	// set ship trail threshold
	if (m_contrail_threshold_flag) {
		The_mission.contrail_threshold = m_contrail_threshold;
	}
	else {
		The_mission.contrail_threshold = CONTRAIL_THRESHOLD_DEFAULT;
	}

	//set support ship repairing flags
	The_mission.flags.set(Mission::Mission_Flags::Support_repairs_hull, m_support_repairs_hull != 0);

	// set default beam free
	The_mission.flags.set(Mission::Mission_Flags::Beam_free_all_by_default, m_beam_free_all_by_default != 0);

	// set player AI by default
	The_mission.flags.set(Mission::Mission_Flags::Player_start_ai, m_player_start_using_ai != 0);

	// set briefing
	The_mission.flags.set(Mission::Mission_Flags::No_briefing, m_no_briefing != 0);

	// set debriefing
	The_mission.flags.set(Mission::Mission_Flags::Toggle_debriefing, m_no_debriefing != 0);

	// set autopilot cinematics
	The_mission.flags.set(Mission::Mission_Flags::Use_ap_cinematics, m_autpilot_cinematics != 0);

	// 2D mission
	The_mission.flags.set(Mission::Mission_Flags::Mission_2d, m_2d_mission != 0);

	// set autopilot disabled
	The_mission.flags.set(Mission::Mission_Flags::Deactivate_ap, m_no_autpilot != 0);

	// always show mission goals
	The_mission.flags.set(Mission::Mission_Flags::Always_show_goals, m_always_show_goals != 0);

	// End to mainhall
	The_mission.flags.set(Mission::Mission_Flags::End_to_mainhall, m_end_to_mainhall != 0);

	if (flags != The_mission.flags) {
		set_modified();
	}

	string_copy(The_mission.name, m_mission_title, NAME_LENGTH, 1);
	string_copy(The_mission.author, m_designer_name, NAME_LENGTH, 1);
	string_copy(The_mission.loading_screen[GR_640], m_loading_640, NAME_LENGTH, 1);
	string_copy(The_mission.loading_screen[GR_1024], m_loading_1024, NAME_LENGTH, 1);
	deconvert_multiline_string(The_mission.notes, m_mission_notes, NOTES_LENGTH);
	deconvert_multiline_string(The_mission.mission_desc, m_mission_desc, MISSION_DESC_LENGTH);

	// copy squad stuff
	if (m_squad_name == CString(NO_SQUAD)) {
		strcpy_s(The_mission.squad_name, "");
		strcpy_s(The_mission.squad_filename, "");
	}
	else {
		string_copy(The_mission.squad_name, m_squad_name, NAME_LENGTH);
		string_copy(The_mission.squad_filename, m_squad_filename, MAX_FILENAME_LEN);
	}

	The_mission.ai_profile = &Ai_profiles[m_ai_profile];

	MODIFY(Current_soundtrack_num, m_event_music - 1);
	strcpy_s(The_mission.substitute_event_music_name, m_substitute_event_music);

	MODIFY(The_mission.command_persona, (int)((CComboBox *)GetDlgItem(IDC_COMMAND_PERSONA))->GetItemData(m_command_persona));
	if (m_command_sender.GetAt(0) == '#')
		strcpy_s(The_mission.command_sender, m_command_sender.Mid(1));
	else
		strcpy_s(The_mission.command_sender, m_command_sender);

	MODIFY(Mission_all_attack, m_full_war);
	if (query_modified()) {
		set_modified();
	}

	theApp.record_window_data(&Mission_notes_wnd_data, this);

	// update the Num_teams variable accoriding to mission types
	Num_teams = 1;
	if ((The_mission.game_type & MISSION_TYPE_MULTI) && (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS)) {
		Num_teams = 2;
	}
	*/
	return true;
}

void MissionSpecDialogModel::reject() {
}

SCP_string MissionSpecDialogModel::getDesigner() {
	return designer_name;
}

void MissionSpecDialogModel::setDesigner(SCP_string designer_name) {
	this->designer_name = designer_name;
}

bool MissionSpecDialogModel::query_modified()
{
	return false;
}

}
}
}