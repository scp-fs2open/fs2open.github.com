#ifndef _DLGVOICEACTINGMANAGER_H
#define _DLGVOICEACTINGMANAGER_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxfred_base.h"

#include <wx/wx.h>

class dlgVoiceActingManager : public fredBase::dlgVoiceActingManager
{
public:
	dlgVoiceActingManager( wxWindow* parent, wxWindowID id );

protected:
	dlgVoiceActingManager( void );
	dlgVoiceActingManager( const dlgVoiceActingManager& T );

	// Handlers for frmBriefingEditor
	void OnClose( wxCloseEvent& event );

private:

};
#endif // _DLGVOICEACTINGMANAGER_H