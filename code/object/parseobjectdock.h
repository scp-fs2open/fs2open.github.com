/*
 * Created by Ian "Goober5000" Warfield for the Freespace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 *
 */ 

/*
 * $Logfile: /Freespace2/code/Object/ParseObjectDock.h $
 * $Revision: 1.1 $
 * $Date: 2005-10-29 22:09:30 $
 * $Author: Goober5000 $
 *
 * New docking system for parse objects, used only on mission load
 *
 * $Log: not supported by cvs2svn $
 */

#ifndef _PARSE_OBJECT_DOCK_H
#define _PARSE_OBJECT_DOCK_H

#include "globalincs/globals.h"

struct p_object;

// info for each docked parse object
typedef struct p_dock_instance {
	p_dock_instance *next;			// next item in list

	char dockpoint_used[NAME_LENGTH];	// name of docking bay
	p_object *docked_objp;				// parse object that is docked to me
} p_dock_instance;

// struct used when a function must be evaluated for all docked objects
typedef struct p_dock_function_info {

	// Set this to true when the function should return early.
	bool early_return_condition;		
	
	// The following were originally unions, but it became necessary to use structs
	// for certain functions that need to maintain two or more values.
	struct {
//		bool		bool_value;
//		char		char_value;
		int			int_value;
//		float		float_value;
//		double		double_value;

		p_object*	objp_value;
//		vec3d*		vecp_value;
//		vec3d*		vecp_value2;
	} parameter_variables, maintained_variables;


	// constructor to initialize everything to 0
	p_dock_function_info()
	{
		memset(this, 0, sizeof(p_dock_function_info));
	}

} p_dock_function_info;

bool object_is_docked(p_object *objp);

// get the first object in objp's dock list
//p_object *dock_get_first_docked_object(p_object *objp);

// check whether objp is part of a docked pair
//bool dock_check_docked_one_on_one(p_object *objp);
/*
// count objects directly docked to objp
int dock_count_direct_docked_objects(p_object *objp);

// count objects directly or indirectly docked with objp
int dock_count_total_docked_objects(p_object *objp);
*/
// check whether other_objp is directly docked to objp
bool dock_check_find_direct_docked_object(p_object *objp, p_object *other_objp);
/*
// check whether other_objp is directly or indirectly docked to objp
bool dock_check_find_docked_object(p_object *objp, p_object *other_objp);

// find the object occupying objp's specified dockpoint
p_object *dock_find_object_at_dockpoint(p_object *objp, int dockpoint);
*/
// find objp's dockpoint being occupied by other_objp
char *dock_find_dockpoint_used_by_object(p_object *objp, p_object *other_objp);

// calculate the center of all docked objects (returned in dest)
//void dock_calc_docked_center(vec3d *dest, p_object *objp);

// Überfunction for evaluating all objects that could possibly be docked to objp.  This will
// call "function" for each docked object.  The function should store its intermediate and
// return values in the dock_function_info struct.
void dock_evaluate_all_docked_objects(p_object *objp, p_dock_function_info *infop, void (*function)(p_object *, p_dock_function_info *));

// docks all docked objects; called only from parse_create_object in missionparse.cpp
void dock_dock_docked_objects(p_object *objp);

// add objp1 and objp2 to each others' dock lists; currently only called by mission_parse_set_up_initial_docks
void dock_dock_objects(p_object *objp1, char *dockpoint1, p_object *objp2, char *dockpoint2);

// free the entire dock list without undocking anything; called only from mission_parse_close in missionparse.cpp
void dock_free_instances(p_object *objp);

#endif	// _PARSE_OBJECT_DOCK_H
