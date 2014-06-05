/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "editors/frmTeamLoadoutEditor.h"
#include "frmFRED2.h"
#include "base/wxFRED_base.h"

#include <wx/wx.h>

// Public Members:
frmTeamLoadoutEditor::frmTeamLoadoutEditor( wxWindow* parent, wxWindowID id )
	: fredBase::frmTeamLoadoutEditor(parent, id)
{
}

// Protected Members:
	// Handlers for frmTeamLoadoutEditor
void frmTeamLoadoutEditor::OnClose( wxCloseEvent& event )
{
	((frmFRED2*) GetParent())->OnChildClosed( this );
	Destroy();
}

void frmTeamLoadoutEditor::OnOK( wxCommandEvent& event )
{
	Close();
}

void frmTeamLoadoutEditor::OnCancel( wxCommandEvent& event )
{
	Close();
}
