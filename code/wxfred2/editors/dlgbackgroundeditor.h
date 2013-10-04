#ifndef _dlgBackgroundEditor_H
#define _dlgBackgroundEditor_H
/*
 * Created by Ian "Goober5000" Warfield and "z64555" for the FreeSpace2 Source 
 * Code Project.
 *
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include "base/wxfred_base.h"

#include <wx/wx.h>

class dlgBackgroundEditor : public fredBase::dlgBackgroundEditor
{
public:
	dlgBackgroundEditor( wxWindow* parent, wxWindowID id );

protected:
	void OnClose( wxCloseEvent& event );

private:
};
#endif // _dlgBackgroundEditor_H