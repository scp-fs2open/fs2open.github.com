#ifndef _WXFRED2_H
#define _WXFRED2_H
/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include <wx/wx.h>


class wxFRED2 : public wxApp
{
	public:
		virtual bool OnInit();
		void LoadMission();
		void NewMission();
		void SaveMission();
};

DECLARE_APP(wxFRED2)

#endif