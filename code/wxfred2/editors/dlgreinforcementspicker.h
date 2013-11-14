#ifndef _DLGREINFORCEMENTSPICKER_H
#define _DLGREINFORCEMENTSPICKER_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxfred_base.h"

#include <wx/wx.h>

class dlgReinforcementsPicker : public fredBase::dlgReinforcementsPicker
{
public:
	dlgReinforcementsPicker( wxWindow* parent );

protected:
	dlgReinforcementsPicker( void );
	dlgReinforcementsPicker( const dlgReinforcementsPicker& T );

	// Handlers for dlgReinforcementsPicker
	void OnClose( wxCloseEvent& event );

	void OnOK( wxCommandEvent& event );
	void OnCancel( wxCommandEvent& event );

private:
};

#endif