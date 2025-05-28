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
		waypoint(const vec3d *pos);
		~waypoint();

		// accessors
		const vec3d *get_pos() const;
		int get_objnum() const;
		const waypoint_list *get_parent_list() const;
		waypoint_list *get_parent_list();
		int get_parent_list_index() const;
		int get_index() const;

		// mutators
		void set_pos(const vec3d *pos);

	private:
		vec3d m_position;
		int m_objnum;

	friend void waypoint_create_game_object(waypoint *wpt, int list_index, int wpt_index);
};

class waypoint_list
{
	public:
		waypoint_list();
		waypoint_list(const char *name);
		~waypoint_list();

		// accessors
		const char *get_name() const;
		const SCP_vector<waypoint> &get_waypoints() const;
		SCP_vector<waypoint> &get_waypoints();

		// mutators
		void set_name(const char *name);

	private:
		char m_name[NAME_LENGTH];
		SCP_vector<waypoint> m_waypoints;
};

//********************GLOBALS********************
extern SCP_vector<waypoint_list> Waypoint_lists;

// bah
extern const int INVALID_WAYPOINT_POSITION;

//********************FUNCTIONS********************
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
int find_matching_waypoint_list_index(const char *name);

// Find a waypoint with the specified name (e.g. Path:1)
waypoint *find_matching_waypoint(const char *name);

// Find a waypoint with the matching object info
waypoint *find_waypoint_with_objnum(int objnum);
waypoint_list *find_waypoint_list_with_instance(int waypoint_instance, int *waypoint_index = NULL);
waypoint *find_waypoint_with_instance(int waypoint_instance);

// Find something at the specified index
waypoint_list *find_waypoint_list_at_index(int index);
waypoint *find_waypoint_at_index(waypoint_list *list, int index);
waypoint *find_waypoint_at_indexes(int list_index, int index);
int find_index_of_waypoint_list(const waypoint_list *wp_list);
int find_index_of_waypoint(const waypoint_list *wp_list, const waypoint *wpt);

// Find a name that doesn't conflict with any current waypoint list
void waypoint_find_unique_name(char *dest_name, int start_index);

// Add a new list of waypoints.  Called from mission parsing.
void waypoint_add_list(const char *name, const SCP_vector<vec3d> &vec_list);

// Attempts to create a waypoint with the specified instance (used to calculate list and index).
// Returns the object number, or -1 on failure.  Used by scripting and FRED.
int waypoint_add(const vec3d *pos, int waypoint_instance, bool first_waypoint_in_list = false);

// Removes a waypoint, including its entire list if it's the last waypoint remaining.
void waypoint_remove(const waypoint *wpt);

#endif //_WAYPOINT_H
