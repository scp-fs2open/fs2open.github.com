/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 

//#include "mission.h"
#include "frmfred2.h"
#include "base/wxfred_base.h"

#include "wxfred2.h"

#include <globalincs/pstypes.h>

#include <wx/image.h>
#include <wx/string.h>
//#include <wx/xrc/xmlres.h>
	#include <wx/wx.h>

IMPLEMENT_APP(wxFRED2)

bool wxFRED2::OnInit()
{
	//wxXmlResource::Get()->InitAllHandlers();
	//InitXmlResource();
	//wxFREDMission* the_Mission = new wxFREDMission();
	wxChar* title = NULL;
	// Init image handlers before frmFRED2 (wxFormBuilder workaround)
	wxImage::AddHandler(new wxPNGHandler);
	frmFRED2 *frame = new frmFRED2( title, 50, 50, 800, 600 );
	SetTopWindow(frame);
	frame->Show(TRUE);

	return true;
}
