#ifndef _WXFRED2_H
#define _WXFRED2_H
/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */

#include <mission/missionparse.h>
#include <object/object.h>
#include <globalincs/pstypes.h>

#include <wx/wx.h>

#include <string>
#include <vector>

typedef int MTIN_t;	// Model Type Index Number, used to create objects of the assoc. model type
typedef int OIN_t;	// Object Index Number, used to keep track of all object instances (except waypoints)
typedef int WIN_t;	// Waypoint Idex Number, used to keep track of all waypoint instances

static WIN_t WIN_new_list = -1;	// Waypoints created with this WIN also create a new wp list (with them as the first wp)

class wxFRED2 : public wxApp
{
public:
	virtual bool OnInit();

	// Mission management
	bool Mission_load( const wxString infile );
	void Mission_new( void );
	void Mission_save( void );

	// Object management
	OIN_t	Object_create( MTIN_t mtin, vec3d *pos, angles *orient, WIN_t win = -1 );
	OIN_t	Object_copy( object *objp );
	int		Object_delete( OIN_t oin );
	int		Object_delete( object *objp );

	char** Get_ship_types( void );	// Retrieves the available ship types as a 

protected:
	// Mission management
	void Mission_reset( void );	// Init's/ Resets the mission to initial conditions
	void Mission_clear( void );	// Wipes out everything, leaving a clean slate to start with
	
	// Object management
	OIN_t Player_create( void );	// Creates the default player ship at {0, 0, 0}
	OIN_t Waypoint_create( vec3d *pos, WIN_t win );	// Creates a waypoint in the same list as the given WIN, or creates a new list with the new waypoint

private:
	std::vector<std::string> ship_types;
//	mission The_mission;
};

DECLARE_APP(wxFRED2)

#endif