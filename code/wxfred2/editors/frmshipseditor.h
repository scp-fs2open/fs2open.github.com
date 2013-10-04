#ifndef _FRMSHIPSEDITOR_H
#define _FRMSHIPSEDITOR_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxfred_base.h"

#include <wx/wx.h>

class frmShipsEditor : public fredBase::frmShipsEditor
{
public:
	frmShipsEditor( wxWindow* parent, wxWindowID id );

protected:
	frmShipsEditor( void );
	frmShipsEditor( const frmShipsEditor &T);

	// Handlers for frmShipsEditor events.
	void OnClose( wxCloseEvent& event );

private:
};
#endif	// _FRMSHIPSEDITOR_H
