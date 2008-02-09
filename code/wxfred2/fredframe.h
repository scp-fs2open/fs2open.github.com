/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 

/*
 * $Logfile: /Freespace2/code/wxFRED2/FREDFrame.h $
 * $Revision: 1.5 $
 * $Date: 2006-04-20 06:32:30 $
 * $Author: Goober5000 $
 *
 * FRED app frame
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2005/05/12 14:00:14  Goober5000
 * added a bunch of dialogs to wxFRED... thanks, taylor, for the GUI development :)
 * --Goober5000
 *
 * Revision 1.3  2005/04/25 06:36:25  taylor
 * newline fix
 *
 * Revision 1.2  2005/04/24 14:42:27  Goober5000
 * wxFRED now uses XRC-based resources
 * --Goober5000
 *
 * Revision 1.1  2005/03/29 13:35:53  Goober5000
 * set up the main menu options
 * --Goober5000
 *
 * Revision 1.0  2005/03/29 07:55:19  Goober5000
 * Addition to CVS repository
 *
 */

#ifndef _FREDFRAME_H
#define _FREDFRAME_H

class FREDFrame : public wxFrame
{
	public:
		// constructor/destructors
		FREDFrame(const wxChar *title, int xpos, int ypos, int width, int height);
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
