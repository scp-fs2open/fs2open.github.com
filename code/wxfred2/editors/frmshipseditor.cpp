/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "editors/frmshipseditor.h"
#include "frmfred2.h"
#include "base/wxfred_base.h"

#include <wx/wx.h>

// wxFormBuilder doesn't seem to want to keep the frame style, so override it for the time being...
const long frame_style = wxCAPTION|wxCLOSE_BOX|wxFRAME_NO_TASKBAR|wxRESIZE_BORDER|wxSYSTEM_MENU;

// Public members
frmShipsEditor::frmShipsEditor( wxWindow* parent, wxWindowID id )
	: fredBase::frmShipsEditor(parent, id, _T("Edit Ship"), wxDefaultPosition, wxDefaultSize, frame_style)
{
}

// Protected members
	// Handlers for frmShipsEditor events.
void frmShipsEditor::OnClose( wxCloseEvent& event )
{
	// Tell the parent window (frmFRED2) that we closed before destroying this window
	((frmFRED2*) GetParent())->OnChildClosed( this );
	Destroy();
}
// Private members
