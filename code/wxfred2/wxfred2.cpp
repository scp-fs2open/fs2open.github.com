/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 

//#include "mission.h"
#include "frmfred2.h"
#include "base/wxfred_base.h"

#include "wxfred2.h"

#include <globalincs/globals.h>
#include <globalincs/pstypes.h>
#include <mission/missionparse.h>

#include <wx/image.h>
#include <wx/string.h>
//#include <wx/xrc/xmlres.h>
#include <wx/wx.h>

// Referenced variables and functions from code.lib
char Fred_alt_names[MAX_SHIPS][NAME_LENGTH+1];
char Fred_callsigns[MAX_SHIPS][NAME_LENGTH+1];
int Show_cpu = 0;
int Fred_running = 1;	// z64: :V:'s cheap hack to let the codebase know that fred-specific routines should be run
float Sun_spot = 0.0f;


IMPLEMENT_APP(wxFRED2)

bool wxFRED2::OnInit()
{
	//wxXmlResource::Get()->InitAllHandlers();
	//InitXmlResource();
	//wxFREDMission* the_Mission = new wxFREDMission();
	wxChar* title = NULL;
	// Init image handlers before frmFRED2 (wxFormBuilder workaround)
	wxImage::AddHandler(new wxPNGHandler);
	frmFRED2 *Fred_gui = new frmFRED2( title, 50, 50, 800, 600 );
	SetTopWindow(Fred_gui);
	Fred_gui->Show(TRUE);

	return true;
}

// Mission Management Methods
void wxFRED2::Mission_new( void )
{
//	Mission_reset();
//	Mission_filename[0] = '\0';
//	docFREDDoc_ptr->autosave("nothing");
//	Undo_count = 0;
};

bool wxFRED2::Mission_load( const wxString infile )
{
	wxMessageBox(_T("Sorry! This feature has not been implemented yet.\nPlease encourage a developer to get right on this. :)"), _T("Unimplemented feature"), wxOK );
	return false;
}
