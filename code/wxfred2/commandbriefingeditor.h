/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 



#ifndef _COMMAND_BRIEFING_EDITOR_H
#define _COMMAND_BRIEFING_EDITOR_H

class dlgCommandBriefingEditor : public wxDialog
{
	public:
		// constructor/destructors
		dlgCommandBriefingEditor(wxWindow *parent);
		~dlgCommandBriefingEditor();

		// event handlers
		// event functions go here

	protected:		
		// events
		DECLARE_EVENT_TABLE()

	private:
		// dialog control stuff goes here
};

#endif
