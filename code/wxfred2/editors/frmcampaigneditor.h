#ifndef _FRMCAMPAIGNEDITOR_H
#define _FRMCAMPAIGNEDITOR_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxfred_base.h"

#include <wx/wx.h>

class frmCampaignEditor : public fredBase::frmCampaignEditor
{
public:
	frmCampaignEditor( wxWindow* parent, wxWindowID id );

protected:
	frmCampaignEditor( void );
	frmCampaignEditor( const frmCampaignEditor& T );

	// Handlers for frmBriefingEditor
	void OnClose( wxCloseEvent& event );

private:

};
#endif // _FRMCAMPAIGNEDITOR_H