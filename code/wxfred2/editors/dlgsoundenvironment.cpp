/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "editors/dlgsoundenvironment.h"
#include "base/wxfred_base.h"

#include <wx/wx.h>

// Public members:
dlgSoundEnvironment::dlgSoundEnvironment( wxWindow* parent )
	: fredBase::dlgSoundEnvironment(parent)
{
}

// Protected members:
	// Handlers for dlgSoundEnvironment:
void dlgSoundEnvironment::OnClose( wxCloseEvent& event )
{
	Hide();
}

void dlgSoundEnvironment::OnOK( wxCommandEvent& event )
{
	Hide();
}

void dlgSoundEnvironment::OnCancel( wxCommandEvent& event )
{
	Hide();
}

// Private members: