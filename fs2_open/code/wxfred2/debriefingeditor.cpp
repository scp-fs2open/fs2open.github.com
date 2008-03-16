/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 

/*
 * $Logfile: /Freespace2/code/wxFRED2/DebriefingEditor.cpp $
 * $Revision: 1.2 $
 * $Date: 2006-04-20 06:32:30 $
 * $Author: Goober5000 $
 *
 * Debriefing editor
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2005/05/12 14:00:14  Goober5000
 * added a bunch of dialogs to wxFRED... thanks, taylor, for the GUI development :)
 * --Goober5000
 *
 * Revision 1.0  2005/05/12 07:30:00  Goober5000
 * Addition to CVS repository
 *
 */

// precompiled header for compilers that support it
#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include "debriefingeditor.h"
#include <wx/xrc/xmlres.h>


BEGIN_EVENT_TABLE(dlgDebriefingEditor, wxDialog)
END_EVENT_TABLE()


dlgDebriefingEditor::dlgDebriefingEditor(wxWindow *parent)
	: wxDialog()
{
	wxXmlResource::Get()->LoadDialog(this, parent, "dlgDebriefingEditor");
}

dlgDebriefingEditor::~dlgDebriefingEditor()
{}

