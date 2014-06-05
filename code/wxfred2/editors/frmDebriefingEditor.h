#ifndef _FRMDEBRIEFINGEDITOR_H
#define _FRMDEBRIEFINGEDITOR_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxFRED_base.h"

#include <wx/wx.h>

class frmDebriefingEditor : public fredBase::frmDebriefingEditor
{
public:
	frmDebriefingEditor( wxWindow* parent, wxWindowID id );

protected:
	frmDebriefingEditor( void );
	frmDebriefingEditor( const frmDebriefingEditor& T );

	// Handlers for frmBriefingEditor
	void OnClose( wxCloseEvent& event );

private:

};
#endif // _FRMDEBRIEFINGEDITOR_H