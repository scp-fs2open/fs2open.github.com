/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "editors/frmWaypointEditor.h"
#include "frmFRED2.h"
#include "base/wxFRED_base.h"

#include <wx/wx.h>

// Public Members:
frmWaypointEditor::frmWaypointEditor( wxWindow* parent, wxWindowID id )
	: fredBase::frmWaypointEditor(parent, id)
{
}

// Protected Members:
	// Handlers for frmWaypointEditor
void frmWaypointEditor::OnClose( wxCloseEvent& event )
{
	((frmFRED2*) GetParent())->OnChildClosed( this );
	Destroy();
}
