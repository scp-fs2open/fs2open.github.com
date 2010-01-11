#include "globalincs/linklist.h"
#include "object/waypoint.h"
#include "object/object.h"

//********************GLOBALS********************
waypoint_list Waypoint_lists[MAX_WAYPOINT_LISTS];
int	Num_waypoint_lists = 0;

//********************FUNCTIONS********************
// done immediately after mission load; originally found in aicode.cpp
void create_waypoints()
{
	int	i, j, z;

	for (j=0; j<Num_waypoint_lists; j++) {
		for (i=0; i<Waypoint_lists[j].count; i++) {
			z = obj_create(OBJ_WAYPOINT, -1, j * 65536 + i, NULL,
				&Waypoint_lists[j].waypoints[i], 0.0f, OF_RENDERS);
		}
	}
}

int waypoint_query_path_name_duplicate(int list)
{
	int i;

	for (i=0; i<Num_waypoint_lists; i++)
		if (i != list)
			if (!stricmp(Waypoint_lists[i].name, Waypoint_lists[list].name))
				return 1;

	return 0;
}

void waypoint_get_unique_path_name(int list)
{
	int i = 1;

	sprintf(Waypoint_lists[list].name, "Waypoint path %d", list + 1);
	while (waypoint_query_path_name_duplicate(list)) {
		sprintf(Waypoint_lists[list].name, "Waypoint path U%d", i++);
	}
}

int waypoint_create(vec3d *pos, int list)
{
	int i, obj, index = 0;
	object *ptr;

	if (list == -1) {  // find a new list to start.
		for (list=0; list<MAX_WAYPOINT_LISTS; list++){
			if (!Waypoint_lists[list].count) {
				waypoint_get_unique_path_name(list);
				break;
			}
		}
	} else {
		index = (list & 0xffff) + 1;
		list /= 65536;
	}

	if (list == MAX_WAYPOINT_LISTS) {
		/*
		Fred_main_wnd->MessageBox("Unable to create new waypoint path.  You\n"
			"have reached the maximum limit.", NULL, MB_OK | MB_ICONEXCLAMATION);
			*/
		Warning(LOCATION, "Unable to create new waypoint path.  You have reached the maximum limit.");
		return -1;
	}

	Assert((list >= 0) && (list < MAX_WAYPOINT_LISTS));  // illegal index or out of lists.
	if (Waypoint_lists[list].count >= MAX_WAYPOINTS_PER_LIST) {
		/*Fred_main_wnd->MessageBox("Unable to create new waypoint.  You have\n"
			"reached the maximum limit on waypoints per list.", NULL, MB_OK | MB_ICONEXCLAMATION);*/
		Warning(LOCATION, "Unable to create new waypoint.  You have reached the maximum limit on waypoints per list.");
		return -1;
	}

	if (Waypoint_lists[list].count > index) {
		i = Waypoint_lists[list].count;
		while (i > index) {
			Waypoint_lists[list].waypoints[i] = Waypoint_lists[list].waypoints[i - 1];
			Waypoint_lists[list].flags[i] = Waypoint_lists[list].flags[i - 1];
			i--;
		}
	}

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		Assert(ptr->type != OBJ_NONE);
		if (ptr->type == OBJ_WAYPOINT) {
			i = ptr->instance;
			if ((i / 65536 == list) && ((i & 0xffff) >= index)){
				ptr->instance++;
			}
		}

		ptr = GET_NEXT(ptr);
	}

	Waypoint_lists[list].count++;
	Waypoint_lists[list].flags[index] = 0;
	Waypoint_lists[list].waypoints[index] = *pos;
	if (list >= Num_waypoint_lists){
		Num_waypoint_lists = list + 1;
	}

	obj = obj_create(OBJ_WAYPOINT, -1, list * 65536 + index, NULL, pos, 0.0f, OF_RENDERS);
	return obj;
}

// return object index of waypoint or -1 if no such waypoint
int waypoint_lookup(char *name)
{
	char buf[128];
	int i;
	object *ptr;

	if (name == NULL)
		return -1;

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->type == OBJ_WAYPOINT) {
			i = ptr->instance;
			sprintf(buf, "%s:%d", Waypoint_lists[i / 65536].name, (i & 0xffff) + 1);
			if ( !stricmp(buf, name) )
				return OBJ_INDEX(ptr);
		}

		ptr = GET_NEXT(ptr);
	}

	return -1;
}

//Return waypoint list index
int waypoint_get_list(object *objp)
{
	if(objp->type != OBJ_WAYPOINT)
		return -1;

	return (objp->instance/65536);
}
