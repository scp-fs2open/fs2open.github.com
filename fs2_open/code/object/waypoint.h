#include "globalincs/globals.h"
#include "globalincs/pstypes.h"

#ifndef _WAYPOINT_H
#define _WAYPOINT_H

//********************CLASSES********************
class waypoint_list;

class waypoint
{
	public:
		waypoint();
		waypoint(vec3d *pos, waypoint_list *parent_list);
		~waypoint();

		// accessors
		vec3d *get_pos();
		int get_objnum();
		waypoint_list *get_parent_list();

		// mutators
		void set_pos(vec3d *pos);

	private:
		vec3d pos;
		int objnum;
		waypoint_list *parent_list;

	friend void waypoint_create_game_object(waypoint *wpt, int list_index, int wpt_index);
};

class waypoint_list
{
	public:
		waypoint_list();
		waypoint_list(const char *name);
		~waypoint_list();

		// accessors
		char *get_name();
		SCP_list<waypoint> &get_waypoints();

		// mutators
		void set_name(const char *name);

	private:
		char name[NAME_LENGTH];
		SCP_list<waypoint> waypoints;
};

//********************GLOBALS********************
extern SCP_list<waypoint_list> Waypoint_lists;

// bah
extern const SCP_list<waypoint>::iterator INVALID_WAYPOINT_POSITION;

//********************FUNCTIONS********************
void waypoint_parse_init();
void waypoint_level_close();

// Translate between object instance and list information
int calc_waypoint_instance(int waypoint_list_index, int waypoint_index);
void calc_waypoint_indexes(int waypoint_instance, int &waypoint_list_index, int &waypoint_index);
int calc_waypoint_list_index(int waypoint_instance);
int calc_waypoint_index(int waypoint_instance);

// Creates initial waypoints after mission is loaded
void waypoint_create_game_objects();

// Find a waypoint list with the specified name
waypoint_list *find_matching_waypoint_list(const char *name);

// Find a waypoint with the specified name (e.g. Path:1)
waypoint *find_matching_waypoint(const char *name);

// Find a waypoint with the matching object info
waypoint *find_waypoint_with_objnum(int objnum);
waypoint_list *find_waypoint_list_with_instance(int waypoint_instance, int *waypoint_index = NULL);
waypoint *find_waypoint_with_instance(int waypoint_instance);

// Find something at the specified index
waypoint_list *find_waypoint_list_at_index(int index);
waypoint *find_waypoint_at_index(waypoint_list *list, int index);
int find_index_of_waypoint_list(waypoint_list *wp_list);
int find_index_of_waypoint(waypoint *wpt);

// Find a name that doesn't conflict with any current waypoint list
void waypoint_find_unique_name(char *dest_name, int start_index);

// Add a new list of waypoints.  Called from mission parsing.
void waypoint_add_list(const char *name, SCP_vector<vec3d> &vec_list);

// Attempts to create a waypoint with the specified instance (used to calculate list and index).
// Returns the object number, or -1 on failure.  Used by scripting and FRED.
int waypoint_add(vec3d *pos, int waypoint_instance);

// Removes a waypoint, including its entire list if it's the last waypoint remaining.
void waypoint_remove(waypoint *wpt);

#endif //_WAYPOINT_H
