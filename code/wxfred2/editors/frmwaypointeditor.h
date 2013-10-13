#ifndef _frmWaypointEditor_H
#define _frmWaypointEditor_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxfred_base.h"

#include <wx/wx.h>

class frmWaypointEditor : public fredBase::frmWaypointEditor
{
public:
	frmWaypointEditor( wxWindow* parent, wxWindowID id );

protected:
	frmWaypointEditor( void );
	frmWaypointEditor( const frmWaypointEditor& T );

	// Handlers for frmWaypointEditor
	void OnClose( wxCloseEvent& event );

private:

};
 #endif // _frmWaypointEditor_H
