#ifndef _DLGSHIELDSYSTEMEDITOR_H
#define _DLGSHIELDSYSTEMEDITOR_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxfred_base.h"

#include <wx/wx.h>

class dlgShieldSystemEditor : public fredBase::dlgShieldSystemEditor
{
public:
	dlgShieldSystemEditor( wxWindow* parent, wxWindowID id );

protected:
	dlgShieldSystemEditor( void );
	dlgShieldSystemEditor( const dlgShieldSystemEditor& T );

	// Handlers for frmBriefingEditor
	void OnClose( wxCloseEvent& event );

private:

};
#endif // _DLGSHIELDSYSTEMEDITOR_H