#ifndef _DLGASTEROIDFIELDEDITOR_H
#define _DLGASTEROIDFIELDEDITOR_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxfred_base.h"

#include <wx/wx.h>

class dlgAsteroidFieldEditor : public fredBase::dlgAsteroidFieldEditor
{
public:
	dlgAsteroidFieldEditor( wxWindow* parent, wxWindowID id );

protected:
	dlgAsteroidFieldEditor( void );
	dlgAsteroidFieldEditor( const dlgAsteroidFieldEditor& T );

	// Handlers for dlgAsteroidFieldEditor
	void OnClose( wxCloseEvent& event );

	void OnEnable( wxCommandEvent& event );
	void OnContentType( wxCommandEvent& event );
	void OnMode( wxCommandEvent& event );
	void OnInnerBoxEnable( wxCommandEvent& event );
	void OnOK( wxCommandEvent& event );
	void OnCancel( wxCommandEvent& event );

private:
};
#endif // _DLGASTEROIDFIELDEDITOR_H
