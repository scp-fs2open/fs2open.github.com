/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 



#ifndef _DEBRIEFING_EDITOR_H
#define _DEBRIEFING_EDITOR_H

class dlgDebriefingEditor : public wxDialog
{
	public:
		// constructor/destructors
		dlgDebriefingEditor(wxWindow *parent);
		~dlgDebriefingEditor();

		// event handlers
		// event functions go here

	protected:		
		// events
		DECLARE_EVENT_TABLE()

	private:
		// dialog control stuff goes here
};

#endif
