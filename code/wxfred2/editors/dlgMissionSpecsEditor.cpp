/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "editors/dlgMissionSpecsEditor.h"
#include "editors/dlgSoundEnvironment.h"
#include "frmFRED2.h"
#include "base/wxFRED_base.h"

#include <wx/wx.h>

// Public members:
dlgMissionSpecsEditor::dlgMissionSpecsEditor( wxWindow* parent, wxWindowID id )
	: fredBase::dlgMissionSpecsEditor(parent, id)
{
	dlgSoundEnvironment_p = new dlgSoundEnvironment(this);
}

// Protected members:
		// Handlers for dlgMissionFieldEditor
void dlgMissionSpecsEditor::OnClose( wxCloseEvent& event )
{
	((frmFRED2*) GetParent())->OnChildClosed(this);
	dlgSoundEnvironment_p->Destroy();
	Destroy();
}

void dlgMissionSpecsEditor::OnOK( wxCommandEvent& event )
{
	Close();
}

void dlgMissionSpecsEditor::OnCancel( wxCommandEvent& event )
{
	Close();
}

void dlgMissionSpecsEditor::OnSoundEnvironment( wxCommandEvent& event )
{
	dlgSoundEnvironment_p->Center();
	dlgSoundEnvironment_p->ShowModal();
}