#ifndef _FRMTEAMLOADOUTEDITOR_H
#define _FRMTEAMLOADOUTEDITOR_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxfred_base.h"

#include <wx/wx.h>

class frmTeamLoadoutEditor : public fredBase::frmTeamLoadoutEditor
{
public:
	frmTeamLoadoutEditor( wxWindow* parent, wxWindowID id );

protected:
	frmTeamLoadoutEditor( void );
	frmTeamLoadoutEditor( const frmTeamLoadoutEditor& T );

	void OnClose( wxCloseEvent& event );
	void OnOK( wxCommandEvent& event );
	void OnCancel( wxCommandEvent& event );

private:
};
#endif //_FRMTEAMLOADOUTEDITOR_H