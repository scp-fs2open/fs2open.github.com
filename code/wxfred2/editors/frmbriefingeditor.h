#ifndef _FRMBRIEFINGEDITOR_H
#define _FRMBRIEFINGEDITOR_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxfred_base.h"

#include <wx/wx.h>

class frmBriefingEditor : public fredBase::frmBriefingEditor
{
public:
	frmBriefingEditor( wxWindow* parent, wxWindowID id );

protected:
	frmBriefingEditor( void );
	frmBriefingEditor( const frmBriefingEditor& T );

	// Handlers for frmBriefingEditor
	void OnClose( wxCloseEvent& event );

private:

};
#endif // _FRMBRIEFINGEDITOR_H