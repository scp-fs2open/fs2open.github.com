/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "editors/dlgasteroidfieldeditor.h"
#include "frmfred2.h"
#include "base/wxfred_base.h"

#include <wx/wx.h>

// Public members:
dlgAsteroidFieldEditor::dlgAsteroidFieldEditor( wxWindow* parent, wxWindowID id )
	: fredBase::dlgAsteroidFieldEditor(parent, id)
{
}

// Protected members:
	// Handlers for dlgAsteroidFieldEditor
void dlgAsteroidFieldEditor::OnClose( wxCloseEvent& event )
{
	((frmFRED2*) GetParent())->OnChildClosed(this);
	Destroy();
}

void dlgAsteroidFieldEditor::OnEnable( wxCommandEvent& event )
{
}

void dlgAsteroidFieldEditor::OnContentType( wxCommandEvent& event )
{
}

void dlgAsteroidFieldEditor::OnMode( wxCommandEvent& event )
{
}

void dlgAsteroidFieldEditor::OnInnerBoxEnable( wxCommandEvent& event )
{
}

void dlgAsteroidFieldEditor::OnOK( wxCommandEvent& event )
{
	Close();
}

void dlgAsteroidFieldEditor::OnCancel( wxCommandEvent& event )
{
	Close();
}
