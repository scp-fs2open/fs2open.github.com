#ifndef _DLGSOUNDENVIRONMENT_H
#define _DLGSOUNDENVIRONMENT_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxfred_base.h"

#include <wx/wx.h>

class dlgSoundEnvironment : public fredBase::dlgSoundEnvironment
{
public:
	dlgSoundEnvironment( wxWindow* parent );

protected:
	dlgSoundEnvironment( void );
	dlgSoundEnvironment( const dlgSoundEnvironment& T );

	// Handlers for dlgSoundEnvironment
	void OnClose( wxCloseEvent& event );

	void OnOK( wxCommandEvent& event );
	void OnCancel( wxCommandEvent& event );
};

#endif // _DLGSOUNDENVIRONMENT_H