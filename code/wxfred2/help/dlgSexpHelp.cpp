/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "help/dlgSexpHelp.h"
#include "frmFRED2.h"
#include "base/wxFRED_base.h"

#include <wx/wx.h>

// Public Members:

dlgSexpHelp::dlgSexpHelp( wxWindow* parent, wxWindowID id )
	: fredBase::dlgSexpHelp(parent, id)
{
}

// Protected Members:
void dlgSexpHelp::OnClose( wxCloseEvent& event )
{
	((frmFRED2*) GetParent())->OnChildClosed( this );
	Destroy();
}


