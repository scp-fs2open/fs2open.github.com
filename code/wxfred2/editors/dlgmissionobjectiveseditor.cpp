/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "editors/dlgmissionobjectiveseditor.h"
#include "frmfred2.h"
#include "base/wxfred_base.h"

#include <wx/wx.h>

// Public Members:
dlgMissionObjectivesEditor::dlgMissionObjectivesEditor( wxWindow* parent, wxWindowID id )
	: fredBase::dlgMissionObjectivesEditor(parent, id)
{
}

// Protected Members:
	// Handlers for dlgMissionObjectives
void dlgMissionObjectivesEditor::OnClose( wxCloseEvent& event )
{
	((frmFRED2*) GetParent())->OnChildClosed( this );
	Destroy();
}

void dlgMissionObjectivesEditor::OnNewObjective( wxCommandEvent& event )
{
}

void dlgMissionObjectivesEditor::OnOK( wxCommandEvent& event )
{
	// TODO: Save changes in the editor
	Close();
}

void dlgMissionObjectivesEditor::OnCancel( wxCommandEvent& event )
{
	// TODO: Restore mission objectives state before this window was created
	Close();
}
