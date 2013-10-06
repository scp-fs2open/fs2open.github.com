#ifndef _DLGFICTIONViEWER_H
#define _DLGFICTIONViEWER_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxfred_base.h"

#include <wx/wx.h>

class dlgFictionViewer : public fredBase::dlgFictionViewer
{
public:
	dlgFictionViewer( wxWindow* parent, wxWindowID id );

protected:
	dlgFictionViewer( void );
	dlgFictionViewer( const dlgFictionViewer& T );

	// Handlers for frmBriefingEditor
	void OnClose( wxCloseEvent& event );

private:

};
#endif // _DLGFICTIONViEWER_H