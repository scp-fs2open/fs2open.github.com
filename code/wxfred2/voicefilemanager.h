/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
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
