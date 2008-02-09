/*
 * Created by Ian "Goober5000" Warfield for the Freespace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */ 

/*
 * $Logfile: /Freespace2/code/wxFRED2/VoiceFileManager.h $
 * $Revision: 1.1 $
 * $Date: 2005-05-12 14:00:15 $
 * $Author: Goober5000 $
 *
 * Voice file manager
 *
 * $Log: not supported by cvs2svn $
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
