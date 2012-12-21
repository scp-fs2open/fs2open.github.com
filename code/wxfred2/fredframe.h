/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 



#ifndef _FREDFRAME_H
#define _FREDFRAME_H

#include "mission.h"

class FREDFrame : public wxFrame
{
	public:
		// constructor/destructors
		FREDFrame(const wxChar *title, int xpos, int ypos, int width, int height, wxFREDMission* current_Mission);
		~FREDFrame();

		// event handlers
		void OnFileNew(wxCommandEvent &WXUNUSED(event));
		void OnFileOpen(wxCommandEvent &WXUNUSED(event));
		void OnFileSave(wxCommandEvent &WXUNUSED(event));
		void OnFileSaveAs(wxCommandEvent &WXUNUSED(event));
		void OnFileExit(wxCommandEvent &WXUNUSED(event));
		void OnEditorsAsteroidField(wxCommandEvent &WXUNUSED(event));
		void OnEditorsMissionSpecs(wxCommandEvent &WXUNUSED(event));
		void OnEditorsDebriefing(wxCommandEvent &WXUNUSED(event));
		void OnEditorsShieldSystem(wxCommandEvent &WXUNUSED(event));
		void OnEditorsCommandBriefing(wxCommandEvent &WXUNUSED(event));
		void OnEditorsSetGlobalShipFlags(wxCommandEvent &WXUNUSED(event));
		void OnEditorsVoiceActingManager(wxCommandEvent &WXUNUSED(event));
		void OnEditorsCampaign(wxCommandEvent &WXUNUSED(event));
		void OnHelpAboutFRED2(wxCommandEvent &WXUNUSED(event));

		wxFREDMission* the_Mission;

	protected:		
		// events
		DECLARE_EVENT_TABLE()

	private:
		// menubar
		wxMenuBar	*myMenuBar;

		// member variables
		wxString currentFilename;
};

#endif
