#ifndef _DLGSEXPHELP_H
#define _DLGSEXPHELP_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxFRED_base.h"

#include <wx/wx.h>

class dlgSexpHelp : public fredBase::dlgSexpHelp
{
public:
	dlgSexpHelp( wxWindow* parent, wxWindowID id );

protected:
	dlgSexpHelp( void );
	dlgSexpHelp( const dlgSexpHelp& T );

	// Handlers for frmBriefingEditor
	void OnClose( wxCloseEvent& event );

private:

};
#endif // _DLGSEXPHELP_H