#ifndef _DLGMISSIONSPECSEDITOR_H
#define _DLGMISSIONSPECSEDITOR_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "editors/dlgsoundenvironment.h"
#include "base/wxfred_base.h"

#include <wx/wx.h>

class dlgMissionSpecsEditor : public fredBase::dlgMissionSpecsEditor
{
public:
	dlgMissionSpecsEditor( wxWindow* parent, wxWindowID id );

protected:
	dlgMissionSpecsEditor( void );
	dlgMissionSpecsEditor( const dlgMissionSpecsEditor& T );

	// Handlers for dlgMissionFieldEditor
	void OnClose( wxCloseEvent& event );

	void OnOK( wxCommandEvent& event );
	void OnCancel( wxCommandEvent& event );
	void OnSoundEnvironment( wxCommandEvent& event );

private:
	dlgSoundEnvironment* dlgSoundEnvironment_p;
};
#endif	// _DLGMISSIONSPECSEDITOR_H
