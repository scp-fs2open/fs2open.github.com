#ifndef _DOCFRED_H
#define _DOCFRED_H
/*
 * Created by "z64555" for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include <cfile/cfile.h>
#include <mission/missionparse.h>
#include <object/waypoint.h>

#include <wx/docview.h>
#include <wx/wx.h>

// Class responsible for handling interactions between the application (wxFRED2) and mission file.
// Document criteria:
//  Each document handles 1 mission.

class docFRED2 : public wxDocument
{
public:
	docFRED2();

	// wxDocument Handler overrides
	// Can't use the file handler methods wxWidgets provides, because FSO uses the CFILE system
	virtual bool OnSaveDocument( const wxString& filename );
	virtual bool OnOpenDocument( const wxString& filename );
	virtual bool OnNewDocument( void );
	virtual bool OnCloseDocument( void );

protected:
	virtual bool DoSaveDocument( const wxString& filename );
	virtual bool DoOpenDocument( const wxString& filename );
	virtual bool DoNewDocument( void );
	virtual bool DoCloseDocument( void );

private:
	int Save_waypoint_list( waypoint_list &wpl, CFILE *fp );
	int Save_waypoints( CFILE *fp );
	int Save_goals( CFILE *fp );
	int Save_wings( CFILE *fp );
	int Save_objects( CFILE *fp );
	int Save_players( CFILE *fp );
	int Save_briefing_info( CFILE *fp );
	int Save_plot_info( CFILE *fp );
	int Save_mission_info( CFILE *fp );

	//mission The_mission;	// The mission object for this document
};
#endif // _DOCFRED_H