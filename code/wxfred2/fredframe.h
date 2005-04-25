/*
 * Created by Ian "Goober5000" Warfield for the Freespace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */ 

/*
 * $Logfile: /Freespace2/code/wxFRED2/FREDFrame.h $
 * $Revision: 1.3 $
 * $Date: 2005-04-25 06:36:25 $
 * $Author: taylor $
 *
 * FRED app frame
 *
 * $Log: not supported by cvs2svn $
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
		void OnExit(wxCommandEvent &WXUNUSED(event));

	protected:		
		// events
		DECLARE_EVENT_TABLE()

	private:
		// menubar
		wxMenuBar	*myMenuBar;
};

#endif
