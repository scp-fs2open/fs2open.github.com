/*
 * Created by Ian "Goober5000" Warfield for the Freespace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */ 

/*
 * $Logfile: /Freespace2/code/wxFRED2/wxFRED2.cpp $
 * $Revision: 1.1 $
 * $Date: 2005-03-29 07:55:19 $
 * $Author: Goober5000 $
 *
 * New cross-platform version of FRED2
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.0  2005/03/28 22:15:00  Goober5000
 * Addition to CVS repository
 *
 */

// precompiled header for compilers that support it
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "wxFRED2.h"

IMPLEMENT_APP(wxFRED2)

bool wxFRED2::OnInit()
{
	wxFrame *frame = new wxFrame((wxFrame*) NULL, -1, "Hello World");
	frame->Show(TRUE);
	SetTopWindow(frame);

	return true;
}