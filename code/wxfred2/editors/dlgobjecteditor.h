#ifndef _dlgObjectEditor_H
#define _dlgObjectEditor_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxfred_base.h"

#include <wx/wx.h>

class dlgObjectEditor : public fredBase::dlgObjectEditor
{
public:
	dlgObjectEditor( wxWindow* parent, wxWindowID id );

protected:
	dlgObjectEditor( void );
	dlgObjectEditor( const dlgObjectEditor &T);

	// Handlers for dlgObjectEditor events.
	void OnClose( wxCloseEvent& event );
	void OnPointTo( wxCommandEvent& event);
	void OnOK( wxCommandEvent& event );
	void OnCancel( wxCommandEvent& event);

private:
};
#endif	// _dlgObjectEditor_H
