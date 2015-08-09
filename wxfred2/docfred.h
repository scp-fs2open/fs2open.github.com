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

/**
* @class Class responsible for handling interactions between the application (wxFRED2) and mission file.

* @details Each document handles 1 mission. The_Mission can be thought as the working copy of mission files, docFRED's would be used whenever wxFRED wants to read from or
* save to disk.
* Real-time Collaborative editing might be possible by sending .diff's of the document or by sending command stacks periodically to a server (and then to all clients)
*/
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
};
#endif // _DOCFRED_H