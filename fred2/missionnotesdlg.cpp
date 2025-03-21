/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "stdafx.h"
#include "FRED.h"
#include "FREDDoc.h"
#include "MissionNotesDlg.h"
#include "CustomWingNames.h"
#include "soundenvironmentdlg.h"
#include "Management.h"
#include "cfile/cfile.h"
#include "gamesnd/eventmusic.h"
#include "mission/missionparse.h"
#include "mission/missionmessage.h"
#include "scripting/global_hooks.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NO_SQUAD				"<none>"

// module globals to hold button information
CButton *coop, *team, *dogfight;

/////////////////////////////////////////////////////////////////////////////
// CMissionNotesDlg dialog

CMissionNotesDlg::CMissionNotesDlg(CWnd* pParent /*=NULL*/) : CDialog(CMissionNotesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMissionNotesDlg)
	m_created = _T("");
	m_modified = _T("");
	m_mission_notes = _T("");
	m_designer_name = _T("");
	m_mission_title = _T("");
	m_mission_desc = _T("");
	m_squad_filename = _T("");
	m_squad_name = _T(NO_SQUAD);
	m_loading_640 = _T("");
	m_loading_1024 = _T("");
	m_ai_profile = -1;
	m_event_music = -1;
	m_substitute_event_music = _T("");
	m_command_persona = -1;
	m_command_sender = _T("");
	m_full_war = FALSE;
	m_red_alert = FALSE;
	m_scramble = FALSE;
	m_num_respawns = 0;
	m_max_respawn_delay = -1;
	m_disallow_support = 0;
	m_no_promotion = FALSE;
	m_no_builtin_msgs = FALSE;
	m_no_builtin_command_msgs = FALSE;
	m_no_traitor = FALSE;
	m_toggle_trails = FALSE;
	m_support_repairs_hull = FALSE;
	m_beam_free_all_by_default = FALSE;
	m_player_start_using_ai = FALSE;
	m_toggle_start_chase_view = FALSE;
	m_no_briefing = FALSE;
	m_toggle_debriefing = FALSE;
	m_autpilot_cinematics = FALSE;
	m_no_autpilot = FALSE;
	m_2d_mission = FALSE;
	m_toggle_showing_goals = FALSE;
	m_end_to_mainhall = FALSE;
	m_override_hashcommand = FALSE;
	m_preload_subspace = FALSE;
	m_max_hull_repair_val = 0.0f;
	m_max_subsys_repair_val = 100.0f;
	m_contrail_threshold = CONTRAIL_THRESHOLD_DEFAULT;
	m_contrail_threshold_flag = FALSE;
	//}}AFX_DATA_INIT
}

void CMissionNotesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMissionNotesDlg)
	DDX_Control(pDX, IDC_RESPAWN_SPIN, m_respawn_spin);
	DDX_Control(pDX, IDC_MAX_RESPAWN_DELAY_SPIN, m_max_respawn_delay_spin);
	DDX_Text(pDX, IDC_CREATED, m_created);
	DDX_Text(pDX, IDC_MODIFIED, m_modified);
	DDX_Text(pDX, IDC_MISSION_NOTES, m_mission_notes);
	DDX_Text(pDX, IDC_DESIGNER_NAME, m_designer_name);
	DDX_Text(pDX, IDC_MISSION_TITLE, m_mission_title);
	DDX_Text(pDX, IDC_MISSION_DESC, m_mission_desc);
	DDX_Text(pDX, IDC_SQUAD_LOGO, m_squad_filename);
	DDX_Text(pDX, IDC_SQUAD_NAME, m_squad_name);
	DDX_Text(pDX, IDC_LOADING_SCREEN640, m_loading_640);
	DDX_Text(pDX, IDC_LOADING_SCREEN1024, m_loading_1024);
	DDX_CBIndex(pDX, IDC_AI_PROFILE, m_ai_profile);
	DDX_CBIndex(pDX, IDC_EVENT_MUSIC, m_event_music);
	DDX_Text(pDX, IDC_SUBSTITUTE_EVENT_MUSIC, m_substitute_event_music);
	DDX_CBIndex(pDX, IDC_COMMAND_PERSONA, m_command_persona);
	DDX_Text(pDX, IDC_COMMAND_SENDER, m_command_sender);
	DDX_Check(pDX, IDC_FULL_WAR, m_full_war);
	DDX_Check(pDX, IDC_RED_ALERT, m_red_alert);
	DDX_Check(pDX, IDC_SCRAMBLE, m_scramble);
	DDX_Text(pDX, IDC_RESPAWNS, m_num_respawns);
	DDV_MinMaxUInt(pDX, m_num_respawns, 0, 99);
	DDX_Text(pDX, IDC_MAX_RESPAWN_DELAY, m_max_respawn_delay);
	DDV_MinMaxInt(pDX, m_max_respawn_delay, -1, 999);
	DDX_Check(pDX, IDC_SUPPORT_ALLOWED, m_disallow_support);
	DDX_Check(pDX, IDC_NO_PROMOTION, m_no_promotion);
	DDX_Check(pDX, IDC_DISABLE_BUILTIN_MSGS, m_no_builtin_msgs);
	DDX_Check(pDX, IDC_DISABLE_BUILTIN_COMMAND_MSGS, m_no_builtin_command_msgs);
	DDX_Check(pDX, IDC_NO_TRAITOR, m_no_traitor);
	DDX_Check(pDX, IDC_SPECS_TOGGLE_TRAILS, m_toggle_trails);
	DDX_Check(pDX, IDC_SUPPORT_REPAIRS_HULL, m_support_repairs_hull);
	DDX_Check(pDX, IDC_BEAM_FREE_ALL_BY_DEFAULT, m_beam_free_all_by_default);
	DDX_Check(pDX, IDC_PLAYER_START_AI, m_player_start_using_ai);
	DDX_Check(pDX, IDC_TOGGLE_START_CHASE, m_toggle_start_chase_view);
	DDX_Check(pDX, IDC_NO_BRIEFING, m_no_briefing);
	DDX_Check(pDX, IDC_TOGGLE_DEBRIEFING, m_toggle_debriefing);
	DDX_Check(pDX, IDC_USE_AUTOPILOT_CINEMATICS, m_autpilot_cinematics);
	DDX_Check(pDX, IDC_2D_MISSION, m_2d_mission);
	DDX_Check(pDX, IDC_DEACTIVATE_AUTOPILOT, m_no_autpilot);
	DDX_Check(pDX, IDC_TOGGLE_SHOWING_GOALS, m_toggle_showing_goals);
	DDX_Check(pDX, IDC_END_TO_MAINHALL, m_end_to_mainhall);
	DDX_Check(pDX, IDC_OVERRIDE_HASHCOMMAND, m_override_hashcommand);
	DDX_Check(pDX, IDC_PRELOAD_SUBSPACE, m_preload_subspace);
	DDX_Text(pDX, IDC_MAX_HULL_REPAIR_VAL, m_max_hull_repair_val);
	DDV_MinMaxFloat(pDX, m_max_hull_repair_val, 0, 100);
	DDX_Text(pDX, IDC_MAX_SUBSYS_REPAIR_VAL, m_max_subsys_repair_val);
	DDV_MinMaxFloat(pDX, m_max_subsys_repair_val, 0, 100);
	DDX_Text(pDX, IDC_CONTRAIL_THRESHOLD, m_contrail_threshold);
	DDV_MinMaxInt(pDX, m_contrail_threshold, 0, 1000);
	DDX_Check(pDX, IDC_CONTRAIL_THRESHOLD_CHECK, m_contrail_threshold_flag);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMissionNotesDlg, CDialog)
	//{{AFX_MSG_MAP(CMissionNotesDlg)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_TRAINING, OnTraining)
	ON_BN_CLICKED(IDC_MULTI, OnMulti)
	ON_BN_CLICKED(IDC_SINGLE, OnSingle)
	ON_BN_CLICKED(IDC_SQUAD_LOGO_BUTTON, OnSquadLogo)
	ON_BN_CLICKED(IDC_LOADING_SCREEN_BUTTON640, OnLoad640)
	ON_BN_CLICKED(IDC_LOADING_SCREEN_BUTTON1024, OnLoad1024)
	ON_BN_CLICKED(IDC_CONTRAIL_THRESHOLD_CHECK, OnToggleContrailThreshold)
	ON_BN_CLICKED(IDC_CUSTOM_WING_NAMES, OnCustomWingNames)
	ON_BN_CLICKED(IDC_SOUND_ENVIRONMENT_BUTTON, OnSoundEnvironment)
	ON_BN_CLICKED(IDC_OPEN_CUSTOM_DATA, OnCustomData)
	ON_BN_CLICKED(IDC_OPEN_CUSTOM_STRINGS, OnCustomStrings)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMissionNotesDlg message handlers

int CMissionNotesDlg::query_modified()
{
	// the below is massively out of date
	return 1;

	/*
	if (m_mission_title != m_mission_title_orig){
		return 1;
	}
	if (m_designer_name != m_designer_name_orig){
		return 1;
	}
	if (m_mission_notes != m_mission_notes_orig){
		return 1;
	}
	if (m_mission_desc != m_mission_desc_orig){
		return 1;
	}
	if (Current_soundtrack_num != m_event_music - 1){
		return 1;
	}
	if (Mission_all_attack != m_full_war){
		return 1;
	}

	return 0;
	*/
}

void CMissionNotesDlg::OnOK()
{
	int new_m_type, is_multi = 0, is_training = 0, is_single = 0;
    flagset<Mission::Mission_Flags> flags;

	UpdateData();
	is_single = (((CButton *) GetDlgItem(IDC_SINGLE))->GetCheck() == 1);
	is_multi = (((CButton *) GetDlgItem(IDC_MULTI))->GetCheck() == 1);
	is_training = (((CButton *) GetDlgItem(IDC_TRAINING))->GetCheck() == 1);

	// deal with changing the mission type.  Code is done this way since training missions
	// just override anything else.
	new_m_type = 0;
	if (is_training) {
		new_m_type = MISSION_TYPE_TRAINING;
	} else {
		if (is_single){
			new_m_type = MISSION_TYPE_SINGLE;
		}

		if (is_multi) {
			new_m_type |= MISSION_TYPE_MULTI;
			if (coop->GetCheck()){
				new_m_type |= MISSION_TYPE_MULTI_COOP;
			} else if (team->GetCheck()){
				new_m_type |= MISSION_TYPE_MULTI_TEAMS;
			} else if(dogfight->GetCheck()){
				new_m_type |= MISSION_TYPE_MULTI_DOGFIGHT;
			} else {
				Int3();			// get allender -- multiplayer mode not set!!!
			}
		}
	}

	if (!new_m_type) {
		MessageBox("You must select the game type: training, single, or multiplayer", "Error");
		return;
	}

	MODIFY(The_mission.game_type, new_m_type );
	MODIFY(The_mission.num_respawns, (int)m_num_respawns );
	MODIFY(The_mission.max_respawn_delay, m_max_respawn_delay );
	MODIFY(The_mission.support_ships.max_support_ships, (m_disallow_support) ? 0 : -1);
	MODIFY(The_mission.support_ships.max_hull_repair_val, m_max_hull_repair_val);
	MODIFY(The_mission.support_ships.max_subsys_repair_val, m_max_subsys_repair_val);

	flags = The_mission.flags;

	// set flags for red alert
    The_mission.flags.set(Mission::Mission_Flags::Red_alert, m_red_alert != 0);

	// set flags for scramble
    The_mission.flags.set(Mission::Mission_Flags::Scramble, m_scramble != 0);

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
	if ( m_contrail_threshold_flag ) {
		The_mission.contrail_threshold = m_contrail_threshold;
	} else {
		The_mission.contrail_threshold = CONTRAIL_THRESHOLD_DEFAULT;
	}

	//set support ship repairing flags
    The_mission.flags.set(Mission::Mission_Flags::Support_repairs_hull, m_support_repairs_hull != 0);

	// set default beam free
    The_mission.flags.set(Mission::Mission_Flags::Beam_free_all_by_default, m_beam_free_all_by_default != 0);

	// set player AI by default
    The_mission.flags.set(Mission::Mission_Flags::Player_start_ai, m_player_start_using_ai != 0);

	// set player chase view
    The_mission.flags.set(Mission::Mission_Flags::Toggle_start_chase_view, m_toggle_start_chase_view != 0);

	// set briefing
    The_mission.flags.set(Mission::Mission_Flags::No_briefing, m_no_briefing != 0);

	// set debriefing
    The_mission.flags.set(Mission::Mission_Flags::Toggle_debriefing, m_toggle_debriefing != 0);

	// set autopilot cinematics
    The_mission.flags.set(Mission::Mission_Flags::Use_ap_cinematics, m_autpilot_cinematics != 0);

	// 2D mission
    The_mission.flags.set(Mission::Mission_Flags::Mission_2d, m_2d_mission != 0);
	
	// set autopilot disabled
    The_mission.flags.set(Mission::Mission_Flags::Deactivate_ap, m_no_autpilot != 0);
	
	// toggle showing the mission goals in the briefing
    The_mission.flags.set(Mission::Mission_Flags::Toggle_showing_goals, m_toggle_showing_goals != 0);

    // End to mainhall
    The_mission.flags.set(Mission::Mission_Flags::End_to_mainhall, m_end_to_mainhall != 0);

	// Override #Command
	The_mission.flags.set(Mission::Mission_Flags::Override_hashcommand, m_override_hashcommand != 0);

	// Preload Subspace Tunnel
	The_mission.flags.set(Mission::Mission_Flags::Preload_subspace, m_preload_subspace != 0);

	if ( flags != The_mission.flags ){
		set_modified();
	}

	// originally the dialog stripped out quotation marks here;
	// now it handles all special characters
	lcl_fred_replace_stuff(m_mission_title);
	lcl_fred_replace_stuff(m_designer_name);
	lcl_fred_replace_stuff(m_mission_notes);
	lcl_fred_replace_stuff(m_mission_desc);
	lcl_fred_replace_stuff(m_squad_name);

	// puts "$End Notes:" on a different line to ensure it's not interpreted as part of a comment
	pad_with_newline(m_mission_notes, NOTES_LENGTH - 1);

	string_copy(The_mission.name, m_mission_title, NAME_LENGTH - 1, 1);
	string_copy(The_mission.author, m_designer_name, true);
	string_copy(The_mission.loading_screen[GR_640], m_loading_640, NAME_LENGTH - 1, 1);
	string_copy(The_mission.loading_screen[GR_1024], m_loading_1024, NAME_LENGTH - 1, 1);
	deconvert_multiline_string(The_mission.notes, m_mission_notes, NOTES_LENGTH - 1);
	deconvert_multiline_string(The_mission.mission_desc, m_mission_desc, MISSION_DESC_LENGTH - 1);

	// copy squad stuff
	if(m_squad_name == CString(NO_SQUAD)){
		strcpy_s(The_mission.squad_name, "");
		strcpy_s(The_mission.squad_filename, "");
	} else {
		string_copy(The_mission.squad_name, m_squad_name, NAME_LENGTH - 1);
		string_copy(The_mission.squad_filename, m_squad_filename, MAX_FILENAME_LEN - 1);
	}

	The_mission.ai_profile = &Ai_profiles[m_ai_profile];

	MODIFY(Current_soundtrack_num, m_event_music - 1);
	strcpy_s(The_mission.substitute_event_music_name, m_substitute_event_music);

	MODIFY(The_mission.command_persona, (int) ((CComboBox *) GetDlgItem(IDC_COMMAND_PERSONA))->GetItemData(m_command_persona));
	if (m_command_sender.GetAt(0) == '#')
		strcpy_s(The_mission.command_sender, m_command_sender.Mid(1));
	else
		strcpy_s(The_mission.command_sender, m_command_sender);

	MODIFY(Mission_all_attack, m_full_war);
	if (query_modified()){
		set_modified();
	}

	theApp.record_window_data(&Mission_notes_wnd_data, this);

	// update the Num_teams variable accoriding to mission types
	Num_teams = 1;
	if ( (The_mission.game_type & MISSION_TYPE_MULTI) && (The_mission.game_type & MISSION_TYPE_MULTI_TEAMS) ){
		Num_teams = 2;
	}

	// This hook will allow for scripts to know when custom data or strings may be updated
	// which will then allow them to update any LuaEnums that may be related to sexps
	if (scripting::hooks::FredOnMissionSpecsSave->isActive()) {
		scripting::hooks::FredOnMissionSpecsSave->run();
	}

	CDialog::OnOK();

	FREDDoc_ptr->autosave("mission specs editor");
}

void CMissionNotesDlg::OnCancel()
{
	theApp.record_window_data(&Mission_notes_wnd_data, this);
	CDialog::OnCancel();
}

BOOL CMissionNotesDlg::OnInitDialog() 
{
	int i, box_index = 0, mission_command_persona_box_index = -1;
	CComboBox *box;
	CEdit *edit;

	// set up the radio box states
	coop = (CButton *)GetDlgItem(IDC_COOP);
	team = (CButton *)GetDlgItem(IDC_TEAMVTEAM);
	dogfight = (CButton *)GetDlgItem(IDC_DOGFIGHT);

	m_mission_title_orig = m_mission_title = _T(The_mission.name);
	m_designer_name_orig = m_designer_name = _T(The_mission.author.c_str());
	m_created = _T(The_mission.created);
	m_modified = _T(The_mission.modified);
	convert_multiline_string(m_mission_notes_orig, The_mission.notes);
	convert_multiline_string(m_mission_notes, The_mission.notes);
	convert_multiline_string(m_mission_desc_orig, The_mission.mission_desc);
	convert_multiline_string(m_mission_desc, The_mission.mission_desc);
	m_red_alert = (The_mission.flags[Mission::Mission_Flags::Red_alert]) ? 1 : 0;
	m_scramble = (The_mission.flags[Mission::Mission_Flags::Scramble]) ? 1 : 0;
	m_full_war = Mission_all_attack;
	m_disallow_support = (The_mission.support_ships.max_support_ships == 0) ? 1 : 0;
	m_no_promotion = (The_mission.flags[Mission::Mission_Flags::No_promotion]) ? 1 : 0;
	m_no_builtin_msgs = (The_mission.flags[Mission::Mission_Flags::No_builtin_msgs]) ? 1 : 0;
	m_no_builtin_command_msgs = (The_mission.flags[Mission::Mission_Flags::No_builtin_command]) ? 1 : 0;
	m_no_traitor = (The_mission.flags[Mission::Mission_Flags::No_traitor]) ? 1 : 0;
	m_toggle_trails = (The_mission.flags[Mission::Mission_Flags::Toggle_ship_trails]) ? 1 : 0;
	m_support_repairs_hull = (The_mission.flags[Mission::Mission_Flags::Support_repairs_hull]) ? 1 : 0;
	m_beam_free_all_by_default = (The_mission.flags[Mission::Mission_Flags::Beam_free_all_by_default]) ? 1 : 0;
	m_player_start_using_ai = (The_mission.flags[Mission::Mission_Flags::Player_start_ai]) ? 1 : 0;
	m_toggle_start_chase_view = (The_mission.flags[Mission::Mission_Flags::Toggle_start_chase_view]) ? 1 : 0;
	m_no_briefing = (The_mission.flags[Mission::Mission_Flags::No_briefing]) ? 1 : 0;
	m_toggle_debriefing = (The_mission.flags[Mission::Mission_Flags::Toggle_debriefing]) ? 1 : 0;
	m_autpilot_cinematics = (The_mission.flags[Mission::Mission_Flags::Use_ap_cinematics]) ? 1 : 0;
	m_2d_mission = (The_mission.flags[Mission::Mission_Flags::Mission_2d]) ? 1 : 0;
	m_no_autpilot = (The_mission.flags[Mission::Mission_Flags::Deactivate_ap]) ? 1 : 0;
	m_toggle_showing_goals = (The_mission.flags[Mission::Mission_Flags::Toggle_showing_goals]) ? 1 : 0;
	m_end_to_mainhall = (The_mission.flags[Mission::Mission_Flags::End_to_mainhall]) ? 1 : 0;
	m_override_hashcommand = (The_mission.flags[Mission::Mission_Flags::Override_hashcommand]) ? 1 : 0;
	m_preload_subspace = (The_mission.flags[Mission::Mission_Flags::Preload_subspace]) ? 1 : 0;

	m_loading_640=_T(The_mission.loading_screen[GR_640]);
	m_loading_1024=_T(The_mission.loading_screen[GR_1024]);

	CDialog::OnInitDialog();

	box = (CComboBox *) GetDlgItem(IDC_AI_PROFILE);
	for (i=0; i<Num_ai_profiles; i++){
		box->AddString(Ai_profiles[i].profile_name);
	}

	box = (CComboBox *) GetDlgItem(IDC_EVENT_MUSIC);
	box->AddString("None");
	for (auto &st: Soundtracks) {
		box->AddString(st.name);
	}

	box = (CComboBox *) GetDlgItem(IDC_SUBSTITUTE_EVENT_MUSIC);
	box->AddString("None");
	for (auto &st: Soundtracks) {
		box->AddString(st.name);
	}

	box = (CComboBox *) GetDlgItem(IDC_COMMAND_PERSONA);
	for (i = 0; i < (int)Personas.size(); i++) {
		if (Personas[i].flags & PERSONA_FLAG_COMMAND){
			box->AddString(Personas[i].name);
			box->SetItemData(box_index, i);
			if (i == The_mission.command_persona)
				mission_command_persona_box_index = box_index;
			box_index++;
		}
	}

	box = (CComboBox *) GetDlgItem(IDC_COMMAND_SENDER);
	box->AddString(DEFAULT_COMMAND);
	for (i=0; i<MAX_SHIPS; i++){
		if (Ships[i].objnum >= 0)
			if (Ship_info[Ships[i].ship_info_index].is_huge_ship())
				box->AddString(Ships[i].ship_name);
	}

	// squad info
	if(strlen(The_mission.squad_name) > 0){ //-V805
		m_squad_name = _T(The_mission.squad_name);
		m_squad_filename = _T(The_mission.squad_filename);
	} else {
		m_squad_name = _T(NO_SQUAD);
		m_squad_filename = _T("");
	}

	m_type = The_mission.game_type;
	m_ai_profile = AI_PROFILES_INDEX(The_mission.ai_profile);

	m_event_music = Current_soundtrack_num + 1;
	m_substitute_event_music = The_mission.substitute_event_music_name;

	m_command_persona = mission_command_persona_box_index;
	m_command_sender = The_mission.command_sender;

	// set up the game type checkboxes accoring to m_type
	if ( m_type & MISSION_TYPE_SINGLE ){
		((CButton *) GetDlgItem(IDC_SINGLE))->SetCheck(1);
	}

	// for multiplayer -- be sure to assign a default type if not already assigned.
	if ( m_type & MISSION_TYPE_MULTI ){
		((CButton *) GetDlgItem(IDC_MULTI))->SetCheck(1);
	}

	if ( m_type & MISSION_TYPE_TRAINING ){
		((CButton *) GetDlgItem(IDC_TRAINING))->SetCheck(1);
	}

	// we need to set one of these three multiplayer modes so interface looks correct
	if ( !(m_type & (MISSION_TYPE_MULTI_COOP | MISSION_TYPE_MULTI_DOGFIGHT | MISSION_TYPE_MULTI_TEAMS)) ){
		m_type |= MISSION_TYPE_MULTI_COOP;
	}

	if ( m_type & MISSION_TYPE_MULTI_COOP ){
		coop->SetCheck(1);
	} else if ( m_type & MISSION_TYPE_MULTI_TEAMS ){
		team->SetCheck(1);
	} else if ( m_type & MISSION_TYPE_MULTI_DOGFIGHT ){
		dogfight->SetCheck(1);
	} else {
		Int3();			// get allender -- multiplayer mode not set!!!
	}

	m_respawn_spin.SetRange(0, 99);
	m_max_respawn_delay_spin.SetRange(-1, 999);
	m_num_respawns = The_mission.num_respawns;
	m_max_respawn_delay = The_mission.max_respawn_delay;
	m_max_hull_repair_val = The_mission.support_ships.max_hull_repair_val;
	m_max_subsys_repair_val = The_mission.support_ships.max_subsys_repair_val;

	m_contrail_threshold = The_mission.contrail_threshold;
	m_contrail_threshold_flag = (m_contrail_threshold != CONTRAIL_THRESHOLD_DEFAULT);
	edit = (CEdit *) GetDlgItem(IDC_CONTRAIL_THRESHOLD);
	edit->SetReadOnly(!m_contrail_threshold_flag);

	set_types();
	UpdateData(FALSE);
	theApp.init_window(&Mission_notes_wnd_data, this);
	return TRUE;
}

void CMissionNotesDlg::OnClose() 
{
	int z;

	if (query_modified()) {
		z = MessageBox("Do you want to keep your changes?", "Close", MB_ICONQUESTION | MB_YESNOCANCEL);
		if (z == IDCANCEL){
			return;
		}

		if (z == IDYES) {
			OnOK();
			return;
		}
	}

	CDialog::OnClose();
}

// when training button is set, we need to disable all other buttons
void CMissionNotesDlg::OnTraining() 
{
	UpdateData(TRUE);
	set_types();
}

void CMissionNotesDlg::OnMulti() 
{
	UpdateData(TRUE);
	set_types();
}

void CMissionNotesDlg::OnSingle() 
{
	UpdateData(TRUE);
	set_types();
}

void CMissionNotesDlg::set_types()
{
	int enable = 0;

	// when training is checked, no other type is active
	if (((CButton *) GetDlgItem(IDC_MULTI))->GetCheck() == 1){
		enable = 1;
	}

	coop->EnableWindow(enable);
	team->EnableWindow(enable);
	dogfight->EnableWindow(enable);
	GetDlgItem(IDC_RESPAWNS)->EnableWindow(enable);
	GetDlgItem(IDC_RESPAWN_SPIN)->EnableWindow(enable);
	GetDlgItem(IDC_MAX_RESPAWN_DELAY)->EnableWindow(enable);
	GetDlgItem(IDC_MAX_RESPAWN_DELAY_SPIN)->EnableWindow(enable);
}

void CMissionNotesDlg::OnSquadLogo()
{	
	int z;
	char *Logo_ext =	"Image Files (*.dds, *.pcx)|*.dds;*.pcx|"
						"DDS Files (*.dds)|*.dds|"
						"PCX Files (*.pcx)|*.pcx|"
						"All Files (*.*)|*.*|"
						"|";

	//phreak 05/05/2003
	//this needs to be here or else the data in the mission notes dialog will revert 
	//to what it was before it was opened.
	UpdateData(TRUE);

	// get list of squad images
	z = cfile_push_chdir(CF_TYPE_SQUAD_IMAGES);
	CFileDialog dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR, Logo_ext);

	// if we have a result
	if (dlg.DoModal() == IDOK) {
		m_squad_filename = dlg.GetFileName();		
	} else {
		m_squad_filename = _T("");
	}
	UpdateData(FALSE);		

	// restore directory
	if (!z){
		cfile_pop_dir();
	}	
}

char *Load_screen_ext =	"Image Files (*.dds, *.pcx, *.jpg, *.jpeg, *.tga, *.png)|*.dds;*.pcx;*.jpg;*.jpeg;*.tga;*.png|"
						"DDS Files (*.dds)|*.dds|"
						"PCX Files (*.pcx)|*.pcx|"
						"JPG Files (*.jpg; *.jpeg)|*.jpg;*.jpeg|"
						"TGA Files (*.tga)|*.tga|"
						"PNG Files (*.png)|*.png|"
						"All Files (*.*)|*.*|"
						"|";

void CMissionNotesDlg::OnLoad1024()
{
	int z;

	UpdateData(TRUE);

	// get list of
	z = cfile_push_chdir(CF_TYPE_DATA);
	CFileDialog dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR, Load_screen_ext);

	// if we have a result
	if (dlg.DoModal() == IDOK) {
		m_loading_1024 = dlg.GetFileName();		
	} else {
		m_loading_1024 = _T("");
	}
	UpdateData(FALSE);		

	// restore directory
	if (!z){
		cfile_pop_dir();
	}	
}

void CMissionNotesDlg::OnLoad640()
{
	int z;

	UpdateData(TRUE);

	// get list of
	z = cfile_push_chdir(CF_TYPE_DATA);
	CFileDialog dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR, Load_screen_ext);

	// if we have a result
	if (dlg.DoModal() == IDOK) {
		m_loading_640 = dlg.GetFileName();		
	} else {
		m_loading_640 = _T("");
	}
	UpdateData(FALSE);		

	// restore directory
	if (!z){
		cfile_pop_dir();
	}	
}



void CMissionNotesDlg::OnEnChangeLoadingScreen641()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CMissionNotesDlg::OnToggleContrailThreshold()
{
	CEdit *edit;

	UpdateData(TRUE);

	// set threshold textbox as read-only or not
	edit = (CEdit *) GetDlgItem(IDC_CONTRAIL_THRESHOLD);
	edit->SetReadOnly(!m_contrail_threshold_flag);

	// if read-only, reset to default
	if ( !m_contrail_threshold_flag )
	{
		m_contrail_threshold = CONTRAIL_THRESHOLD_DEFAULT;
	}

	UpdateData(FALSE);
}

void CMissionNotesDlg::OnCustomWingNames() 
{
	UpdateData(TRUE);

	CustomWingNames dlg;
	dlg.DoModal();

	UpdateData(FALSE);	
}

void CMissionNotesDlg::OnSoundEnvironment()
{
	UpdateData(TRUE);

	SoundEnvironment dlg;
	dlg.DoModal();

	UpdateData(FALSE);
}

void CMissionNotesDlg::OnCustomData()
{
	UpdateData(TRUE);

	CustomDataDlg dlg;
	dlg.DoModal();

	UpdateData(FALSE);
}


void CMissionNotesDlg::OnCustomStrings()
{
	UpdateData(TRUE);

	CustomStringsDlg dlg;
	dlg.DoModal();

	UpdateData(FALSE);
}