/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 



// precompiled header for compilers that support it
#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include "missionspecseditor.h"
#include <wx/xrc/xmlres.h>
#include <wx/spinctrl.h>


BEGIN_EVENT_TABLE(dlgMissionSpecsEditor, wxDialog)
	EVT_TEXT(XRCID("txtTitle"), dlgMissionSpecsEditor::OnTitleChange)
	EVT_TEXT(XRCID("txtAuthor"), dlgMissionSpecsEditor::OnAuthorChange)
END_EVENT_TABLE()


dlgMissionSpecsEditor::dlgMissionSpecsEditor(wxWindow *parent, wxFREDMission* current_Mission)
	: wxDialog()
{
	the_Mission = current_Mission;
	wxXmlResource::Get()->LoadDialog(this, parent, "dlgMissionSpecsEditor");

	//set up our data
	XRCCTRL(*this, "txtCreated", wxStaticText)->SetLabel(the_Mission->created);
	XRCCTRL(*this, "txtModified", wxStaticText)->SetLabel(the_Mission->modified);

	if (the_Mission->game_type & MISSION_TYPE_SINGLE) {
		XRCCTRL(*this, "optSinglePlayer", wxRadioButton)->SetValue(true);
		XRCCTRL(*this, "optCooperative", wxRadioButton)->Disable();
		XRCCTRL(*this, "optTeamVsTeam", wxRadioButton)->Disable();
		XRCCTRL(*this, "optDogfight", wxRadioButton)->Disable();
	} else if (the_Mission->game_type & MISSION_TYPE_MULTI) {
		XRCCTRL(*this, "optMultiPlayer", wxRadioButton)->SetValue(true);
		XRCCTRL(*this, "optCooperative", wxRadioButton)->Enable();
		XRCCTRL(*this, "optTeamVsTeam", wxRadioButton)->Enable();
		XRCCTRL(*this, "optDogfight", wxRadioButton)->Enable();
		if (the_Mission->game_type & MISSION_TYPE_MULTI_COOP)
			XRCCTRL(*this, "optCooperative", wxRadioButton)->SetValue(true);
		else if (the_Mission->game_type & MISSION_TYPE_MULTI_TEAMS)
			XRCCTRL(*this, "optTeamVsTeam", wxRadioButton)->SetValue(true);
		else if (the_Mission->game_type & MISSION_TYPE_MULTI_DOGFIGHT)
			XRCCTRL(*this, "optDogfight", wxRadioButton)->SetValue(true);
	} else if (the_Mission->game_type & MISSION_TYPE_TRAINING) {
		XRCCTRL(*this, "optTraining", wxRadioButton)->SetValue(true);
		XRCCTRL(*this, "optCooperative", wxRadioButton)->Disable();
		XRCCTRL(*this, "optTeamVsTeam", wxRadioButton)->Disable();
		XRCCTRL(*this, "optDogfight", wxRadioButton)->Disable();
	}

	XRCCTRL(*this, "spnMaxRespawns", wxSpinCtrl)->SetValue(the_Mission->num_respawns);
	XRCCTRL(*this, "spnMaxRespawnDelay", wxSpinCtrl)->SetValue(the_Mission->max_respawn_delay);

	XRCCTRL(*this, "txtSquadronName", wxTextCtrl)->SetValue(the_Mission->squad_name);
	XRCCTRL(*this, "txtSquadronLogo", wxTextCtrl)->SetValue(the_Mission->squad_filename);

	XRCCTRL(*this, "chkDisallowSupportShips", wxCheckBox)->SetValue(the_Mission->support_ships.max_support_ships == 0);
	XRCCTRL(*this, "chkSupportShipsRepairHull", wxCheckBox)->SetValue( (the_Mission->flags & MISSION_FLAG_SUPPORT_REPAIRS_HULL) ? true : false);
	XRCCTRL(*this, "txtHullRepairCeiling", wxTextCtrl)->SetValue(wxString::Format(wxT("%f"), the_Mission->support_ships.max_hull_repair_val) );
	XRCCTRL(*this, "txtSubsystemRepair", wxTextCtrl)->SetValue(wxString::Format(wxT("%f"), the_Mission->support_ships.max_subsys_repair_val) );

	XRCCTRL(*this, "chkToggleNebula", wxCheckBox)->SetValue( (the_Mission->flags & MISSION_FLAG_TOGGLE_SHIP_TRAILS) ? true : false);
	XRCCTRL(*this, "chkMinimumTrailSpeed", wxCheckBox)->SetValue(the_Mission->contrail_threshold != CONTRAIL_THRESHOLD_DEFAULT);
	if (the_Mission->contrail_threshold != CONTRAIL_THRESHOLD_DEFAULT) {
		XRCCTRL(*this, "spnMinimumTrailSpeed", wxSpinCtrl)->Enable();
		XRCCTRL(*this, "spnMinimumTrailSpeed", wxSpinCtrl)->SetValue(the_Mission->contrail_threshold);
	}
}

dlgMissionSpecsEditor::~dlgMissionSpecsEditor()
{}

void dlgMissionSpecsEditor::OnTitleChange(wxCommandEvent &WXUNUSED(event)) {
	the_Mission->name = XRCCTRL(*this, "txtTitle", wxTextCtrl)->GetValue();
}

void dlgMissionSpecsEditor::OnAuthorChange(wxCommandEvent &WXUNUSED(event)) {
	the_Mission->author = XRCCTRL(*this, "txtDesigner", wxTextCtrl)->GetValue();
}
