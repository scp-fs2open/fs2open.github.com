/*
 * Created by Ian "Goober5000" Warfield for the Freespace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */ 

/*
 * $Logfile: /Freespace2/code/wxFRED2/wxFRED2.cpp $
 * $Revision: 1.7 $
 * $Date: 2005-04-25 12:14:21 $
 * $Author: Goober5000 $
 *
 * New cross-platform version of FRED2
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.6  2005/04/24 14:42:27  Goober5000
 * wxFRED now uses XRC-based resources
 * --Goober5000
 *
 * Revision 1.5  2005/04/13 22:17:27  Goober5000
 * removed spurious folder
 * --Goober5000
 *
 * Revision 1.4  2005/03/30 16:06:26  taylor
 * a little Linux happiness
 *
 * Revision 1.3  2005/03/30 01:23:32  Goober5000
 * added header back in, with a small tweak for the .cpp
 * --Goober5000
 *
 * Revision 1.2  2005/03/29 13:35:53  Goober5000
 * set up the main menu options
 * --Goober5000
 *
 * Revision 1.1  2005/03/29 07:55:19  Goober5000
 * whew... bare-bones "hello world" wxFRED base finally implemented!
 * --Goober5000
 *
 * Revision 1.0  2005/03/28 22:15:00  Goober5000
 * Addition to CVS repository
 *
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

IMPLEMENT_APP(wxFRED2)

bool wxFRED2::OnInit()
{
	wxXmlResource::Get()->InitAllHandlers();
	InitXmlResource();

	FREDFrame *frame = new FREDFrame(_T("Untitled - FRED2_OPEN 3.6.5 - FreeSpace 2 Mission Editor"), 50, 50, 800, 600);
	SetTopWindow(frame);
	frame->Show(TRUE);

	return true;
}
