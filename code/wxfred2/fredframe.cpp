/*
 * Created by Ian "Goober5000" Warfield for the Freespace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */ 

/*
 * $Logfile: /Freespace2/code/wxFRED2/FREDFrame.cpp $
 * $Revision: 1.2 $
 * $Date: 2005-04-24 14:42:27 $
 * $Author: Goober5000 $
 *
 * FRED app frame
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2005/03/29 13:35:53  Goober5000
 * set up the main menu options
 * --Goober5000
 *
 * Revision 1.0  2005/03/29 07:55:19  Goober5000
 * Addition to CVS repository
 *
 */

// precompiled header for compilers that support it
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#include "fredframe.h"
#include <wx/xrc/xmlres.h>


BEGIN_EVENT_TABLE(FREDFrame, wxFrame)
	EVT_MENU(XRCID("mnuFileExit"), FREDFrame::OnExit)
END_EVENT_TABLE()


FREDFrame::FREDFrame(const wxChar *title, int xpos, int ypos, int width, int height)
	: wxFrame(NULL, -1, title, wxPoint(xpos, ypos), wxSize(width, height))
{
	myMenuBar = wxXmlResource::Get()->LoadMenuBar(_T("FREDMenu"));
	SetMenuBar(myMenuBar);

	CreateStatusBar(5);
	SetStatusText("For Help, press F1", 0);
}

FREDFrame::~FREDFrame()
{}

void FREDFrame::OnExit(wxCommandEvent &WXUNUSED(event))
{
	Close(true);
}
