#ifndef _DLGEVENTSEDITOR_H
#define _DLGEVENTSEDITOR_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

 // z64: This dialog could do with a good redesign...

#include "base/wxfred_base.h"

#include <wx/wx.h>

class dlgEventsEditor : public fredBase::dlgEventsEditor
{
public:
	dlgEventsEditor( wxWindow* parent, wxWindowID id );

protected:
	dlgEventsEditor( void );
	dlgEventsEditor( const dlgEventsEditor& T );

	// Handlers for dlgEventsEditor
	void OnClose( wxCloseEvent& event );

	void OnOK( wxCommandEvent& event );
	void OnCancel( wxCommandEvent& event);
	
private:

};

#endif // _DLGEVENTSEDITOR_H
