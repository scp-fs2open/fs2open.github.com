#ifndef _DLGMISSIONOBJECTIVES_H
#define _DLGMISSIONOBJECTIVES_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxfred_base.h"

#include <wx/wx.h>

class dlgMissionObjectivesEditor : public fredBase::dlgMissionObjectivesEditor
{
public:
	dlgMissionObjectivesEditor( wxWindow* parent, wxWindowID id );

protected:
	dlgMissionObjectivesEditor( void );
	dlgMissionObjectivesEditor( const dlgMissionObjectivesEditor& T );

	// Handlers for dlgMissionObjectives
	void OnClose( wxCloseEvent& event );
	void OnNewObjective( wxCommandEvent& event );
	void OnOK(wxCommandEvent& event );
	void OnCancel( wxCommandEvent& event );

private:
};
#endif // _DLGMISSIONOBJECTIVES_H
