#ifndef _DLGMISSIONSTATS_H
#define _DLGMISSIONSTATS_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxfred_base.h"

#include <wx/wx.h>

class dlgMissionStats : public fredBase::dlgMissionStats
{
public:
	dlgMissionStats( wxWindow* parent, wxWindowID id );

protected:
	dlgMissionStats( void );
	dlgMissionStats( const dlgMissionStats& T );

	// Handlers for frmBriefingEditor
	void OnClose( wxCloseEvent& event );

private:

};
#endif // _DLGMISSIONSTATS_H