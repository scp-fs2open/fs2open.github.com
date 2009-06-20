/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 



#ifndef _CAMPAIGN_EDITOR_H
#define _CAMPAIGN_EDITOR_H

class dlgCampaignEditor : public wxDialog
{
	public:
		// constructor/destructors
		dlgCampaignEditor(wxWindow *parent);
		~dlgCampaignEditor();

		// event handlers
		// event functions go here

	protected:		
		// events
		DECLARE_EVENT_TABLE()

	private:
		// dialog control stuff goes here
};

#endif
