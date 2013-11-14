/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "editors/dlgreinforcementspicker.h"
#include "base/wxfred_base.h"

#include <wx/wx.h>

// Public Members:
dlgReinforcementsPicker::dlgReinforcementsPicker( wxWindow* parent )
	: fredBase::dlgReinforcementsPicker(parent)
{
}

// Protected Members:
	// Handlers for dlgReinforcementsPicker
void dlgReinforcementsPicker::OnClose( wxCloseEvent& event )
{
	Destroy();
}

void dlgReinforcementsPicker::OnOK( wxCommandEvent& event )
{
	Hide();
}

void dlgReinforcementsPicker::OnCancel( wxCommandEvent& event )
{
	Hide();
}

// Private Members:
