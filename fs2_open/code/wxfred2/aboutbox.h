/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 



#ifndef _ABOUT_BOX_H
#define _ABOUT_BOX_H

class dlgAboutBox : public wxDialog
{
	public:
		// constructor/destructors
		dlgAboutBox(wxWindow *parent);
		~dlgAboutBox();

		// event handlers
		// event functions go here

	protected:		
		// events
		DECLARE_EVENT_TABLE()

	private:
		// dialog control stuff goes here
};

#endif
