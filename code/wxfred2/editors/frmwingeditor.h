#ifndef _frmWingEditor_H
#define _frmWingEditor_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxfred_base.h"

#include <wx/wx.h>

class frmWingEditor : public fredBase::frmWingEditor
{
public:
	frmWingEditor( wxWindow* parent, wxWindowID id );

protected:
	frmWingEditor( void );
	frmWingEditor( const frmWingEditor& T );

	// Handlers for frmWingEditor events
	void OnClose( wxCloseEvent& event );

private:
};

#endif // _frmWingEditor_H
