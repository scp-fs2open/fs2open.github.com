/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 



#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include "wxfred2.h"
#include <wx/xrc/xmlres.h>
#include "fredframe.h"
#include "wxfred_xrc.h"
#include "mission.h"

IMPLEMENT_APP(wxFRED2)

bool wxFRED2::OnInit()
{
	wxXmlResource::Get()->InitAllHandlers();
	InitXmlResource();
	wxFREDMission* the_Mission = new wxFREDMission();
	FREDFrame *frame = new FREDFrame(_T("Untitled - FRED2_OPEN 3.6.5 - FreeSpace 2 Mission Editor"), 50, 50, 800, 600, the_Mission);
	SetTopWindow(frame);
	frame->Show(TRUE);

	return true;
}
