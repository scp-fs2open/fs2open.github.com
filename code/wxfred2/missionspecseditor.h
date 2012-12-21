/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 



#ifndef _MISSION_SPECS_EDITOR_H
#define _MISSION_SPECS_EDITOR_H

#include "mission.h"


class dlgMissionSpecsEditor : public wxDialog
{
	public:
		wxFREDMission* the_Mission;

		// constructor/destructors
		dlgMissionSpecsEditor(wxWindow *parent, wxFREDMission* current_Mission);
		~dlgMissionSpecsEditor();

		// event handlers
		// event functions go here
		void OnTitleChange(wxCommandEvent &WXUNUSED(event));
		void OnAuthorChange(wxCommandEvent &WXUNUSED(event));

	protected:		
		// events
		DECLARE_EVENT_TABLE()

	private:
		// dialog control stuff goes here
};

#endif
