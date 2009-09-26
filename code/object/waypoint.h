#include "globalincs/globals.h"
#include "globalincs/pstypes.h"

#ifndef _WAYPOINT_H
#define _WAYPOINT_H

//********************DEFINES********************
#define	MAX_WAYPOINTS_PER_LIST	20
#define	MAX_WAYPOINT_LISTS		32

#define WAYPOINTLIST_INDEX(wlp) (wlp - &Waypoint_lists[0])

//********************STRUCTURES********************
// waypoint list flags bitmasks.
#define WL_MARKED	0x01

typedef struct waypoint_list {
	char		name[NAME_LENGTH];
	int		count;
	char		flags[MAX_WAYPOINTS_PER_LIST];
	vec3d	waypoints[MAX_WAYPOINTS_PER_LIST];
} waypoint_list;

//********************GLOBALS********************
extern waypoint_list Waypoint_lists[MAX_WAYPOINT_LISTS];
extern int Num_waypoint_lists;

//********************FUNCTIONS********************
//Creates initial waypoints after mission is loaded
void create_waypoints();

//Attempts to create a waypoint on waypoint_list list. Returns -1 on failure.
int waypoint_create(vec3d *pos, int list);

//Queries for waypoint with given name, returns -1 on failure
int waypoint_lookup(char *name);

//Returns Waypoint_lists[] index for a given waypoint
int waypoint_get_list(struct object *objp);

#endif //_WAYPOINT_H
