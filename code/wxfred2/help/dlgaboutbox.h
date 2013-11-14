#ifndef _DLGABOUTBOX_H
#define _DLGABOUTBOX_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxfred_base.h"

#include <wx/wx.h>

class dlgAboutBox : public fredBase::dlgAboutBox
{
public:
	dlgAboutBox( wxWindow* parent, wxWindowID id );

protected:
	dlgAboutBox( void );
	dlgAboutBox( const dlgAboutBox& T );

	// Handlers for frmBriefingEditor
	void OnClose( wxCloseEvent& event );

private:

};
#endif // _DLGABOUTBOX_H