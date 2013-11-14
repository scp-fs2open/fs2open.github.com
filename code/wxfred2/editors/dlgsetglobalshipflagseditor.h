#ifndef _DLGSETGLOBALSHIPFLAGSEDITOR_H
#define _DLGSETGLOBALSHIPFLAGSEDITOR_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxfred_base.h"

#include <wx/wx.h>

class dlgSetGlobalShipFlagsEditor : public fredBase::dlgSetGlobalShipFlagsEditor
{
public:
	dlgSetGlobalShipFlagsEditor( wxWindow* parent, wxWindowID id );

protected:
	dlgSetGlobalShipFlagsEditor( void );
	dlgSetGlobalShipFlagsEditor( const dlgSetGlobalShipFlagsEditor& T );

	// Handlers for frmBriefingEditor
	void OnClose( wxCloseEvent& event );

private:

};
#endif // _DLGSETGLOBALSHIPFLAGSEDITOR_H