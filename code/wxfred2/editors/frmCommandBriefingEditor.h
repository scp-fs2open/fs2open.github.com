#ifndef _FRMCOMMANDBRIEFINGEDITOR_H
#define _FRMCOMMANDBRIEFINGEDITOR_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxFRED_base.h"

#include <wx/wx.h>

class frmCommandBriefingEditor : public fredBase::frmCommandBriefingEditor
{
public:
	frmCommandBriefingEditor( wxWindow* parent, wxWindowID id );

protected:
	frmCommandBriefingEditor( void );
	frmCommandBriefingEditor( const frmCommandBriefingEditor& T );

	// Handlers for frmBriefingEditor
	void OnClose( wxCloseEvent& event );

private:

};
#endif // _FRMCOMMANDBRIEFINGEDITOR_H