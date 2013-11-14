/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */
#include "editors/dlgobjecteditor.h"
#include "frmfred2.h"
#include "base/wxfred_base.h"

#include <wx/wx.h>

// Public Members
dlgObjectEditor::dlgObjectEditor( wxWindow* parent, wxWindowID id )
	: fredBase::dlgObjectEditor(parent, id)
{
}

// Protected Members:
	// Handlers for dlgObjectEditor events.
void dlgObjectEditor::OnClose( wxCloseEvent& event )
{
	// Tell the parent window (frmFRED2) that we closed before destroying this window
	((frmFRED2*) GetParent())->OnChildClosed( this );
	Destroy();
}

void dlgObjectEditor::OnPointTo( wxCommandEvent& event )
{
	if( event.IsChecked() == true )
	{
		// Enable Orientation Controls
		optObject->Enable();
		cbObject->Enable();

		optLocation->Enable();
		lblLocationX->Enable();
		spnLocationX->Enable();
		lblLocationY->Enable();
		spnLocationY->Enable();
		lblLocationZ->Enable();
		spnLocationZ->Enable();
	}
	else
	{
		// Disable Orientation Controls
		optObject->Disable();
		cbObject->Disable();

		optLocation->Disable();
		lblLocationX->Disable();
		spnLocationX->Disable();
		lblLocationY->Disable();
		spnLocationY->Disable();
		lblLocationZ->Disable();
		spnLocationZ->Disable();
	}
}

void dlgObjectEditor::OnOK( wxCommandEvent& event )
{
	// TODO: Apply Changes
	Close();
}

void dlgObjectEditor::OnCancel( wxCommandEvent& event)
{
	// TODO: Restore previous state of objects
	Close();
}
