/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 

/*
 * $Logfile: /Freespace2/code/wxFRED2/VoiceFileManager.h $
 * $Revision: 1.2 $
 * $Date: 2006-04-20 06:32:30 $
 * $Author: Goober5000 $
 *
 * Voice file manager
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2005/05/12 14:00:15  Goober5000
 * added a bunch of dialogs to wxFRED... thanks, taylor, for the GUI development :)
 * --Goober5000
 *
 * Revision 1.0  2005/05/12 07:30:00  Goober5000
 * Addition to CVS repository
 *
 */

#ifndef _VOICE_FILE_MANAGER_H
#define _VOICE_FILE_MANAGER_H

class dlgVoiceFileManager : public wxDialog
{
	public:
		// constructor/destructors
		dlgVoiceFileManager(wxWindow *parent);
		~dlgVoiceFileManager();

		// event handlers
		// event functions go here

	protected:		
		// events
		DECLARE_EVENT_TABLE()

	private:
		// dialog control stuff goes here
};

#endif
