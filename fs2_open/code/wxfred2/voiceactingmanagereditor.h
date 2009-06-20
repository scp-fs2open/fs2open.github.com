/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 



#ifndef _VOICE_ACTING_MANAGER_EDITOR_H
#define _VOICE_ACTING_MANAGER_EDITOR_H

class dlgVoiceActingManagerEditor : public wxDialog
{
	public:
		// constructor/destructors
		dlgVoiceActingManagerEditor(wxWindow *parent);
		~dlgVoiceActingManagerEditor();

		// event handlers
		void OnGenerateFileNames(wxCommandEvent &WXUNUSED(event));

	protected:		
		// events
		DECLARE_EVENT_TABLE()

	private:
		// dialog control stuff goes here
};

#endif
