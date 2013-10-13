/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "editors/frmdebriefingeditor.h"
#include "frmfred2.h"
#include "base/wxfred_base.h"

#include <wx/wx.h>

// Public Members:

frmDebriefingEditor::frmDebriefingEditor( wxWindow* parent, wxWindowID id )
	: fredBase::frmDebriefingEditor(parent, id)
{
}

// Protected Members:
void frmDebriefingEditor::OnClose( wxCloseEvent& event )
{
	((frmFRED2*) GetParent())->OnChildClosed( this );
	Destroy();
}


