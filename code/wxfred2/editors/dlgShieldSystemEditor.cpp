/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "editors/dlgShieldSystemEditor.h"

#include "frmFRED2.h"

#include "base/wxFRED_base.h"

#include <wx/wx.h>

// Public Members:

dlgShieldSystemEditor::dlgShieldSystemEditor( wxWindow* parent, wxWindowID id )
	: fredBase::dlgShieldSystemEditor(parent, id)
{
}

// Protected Members:
void dlgShieldSystemEditor::OnClose( wxCloseEvent& event )
{
	((frmFRED2*) GetParent())->OnChildClosed( this );
	Destroy();
}


