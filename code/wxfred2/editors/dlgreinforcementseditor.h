#ifndef _DLGREINFORCEMENTSEDITOR_H
#define _DLGREINFORCEMENTSEDITOR_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "editors/dlgreinforcementspicker.h"
#include "base/wxfred_base.h"

#include <wx/wx.h>

class dlgReinforcementsEditor : public fredBase::dlgReinforcementsEditor
{
public:
	dlgReinforcementsEditor( wxWindow* parent, wxWindowID id );
protected:
	dlgReinforcementsEditor( void );
	dlgReinforcementsEditor( const dlgReinforcementsEditor& T );

	// Handlers for dlgReinforcementsEditor
	void OnClose( wxCloseEvent& event );

	void OnAdd( wxCommandEvent& event );
	void OnRemove( wxCommandEvent& event );
	void OnOK( wxCommandEvent& event );
	void OnCancel( wxCommandEvent& event );

private:
	dlgReinforcementsPicker* dlgReinforcementsPicker_p;
};
#endif // _DLGREINFORCEMENTSEDITOR_H