/*
 * Created by Ian "Goober5000" Warfield for the Freespace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */ 

/*
 * $Logfile: /Freespace2/code/wxFRED2/MissionSpecsEditor.cpp $
 * $Revision: 1.1 $
 * $Date: 2005-05-12 14:00:14 $
 * $Author: Goober5000 $
 *
 * Mission specs editor
 *
 * $Log: not supported by cvs2svn $
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

#include "missionspecseditor.h"
#include <wx/xrc/xmlres.h>


BEGIN_EVENT_TABLE(dlgMissionSpecsEditor, wxDialog)
END_EVENT_TABLE()


dlgMissionSpecsEditor::dlgMissionSpecsEditor(wxWindow *parent)
	: wxDialog()
{
	wxXmlResource::Get()->LoadDialog(this, parent, "dlgMissionSpecsEditor");
}

dlgMissionSpecsEditor::~dlgMissionSpecsEditor()
{}

