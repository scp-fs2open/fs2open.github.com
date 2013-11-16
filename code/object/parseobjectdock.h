/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 



#ifndef _PARSE_OBJECT_DOCK_H
#define _PARSE_OBJECT_DOCK_H

#include "globalincs/globals.h"

class p_object;

// info for each docked parse object
typedef struct p_dock_instance {
	p_dock_instance *next;			// next item in list

	char dockpoint_used[NAME_LENGTH];	// name of docking bay
	p_object *docked_objp;				// parse object that is docked to me
} p_dock_instance;

// class used when a function must be evaluated for all docked objects
// (it's a class because it has a constructor)
class p_dock_function_info
{
public:

	// Set this to true when the function should return early.
	bool early_return_condition;		
	
	// The following were originally unions, but it became necessary to use structs
	// for certain functions that need to maintain two or more values.
	struct {
		int			int_value;
		p_object*	objp_value;
	} parameter_variables, maintained_variables;


	// constructor to initialize everything to 0
	// (revised for the same reasons as dock_function_info)
	p_dock_function_info()
		: early_return_condition(false)
	{
		memset(&parameter_variables, 0, sizeof(parameter_variables));
		memset(&maintained_variables, 0, sizeof(maintained_variables));
	}
};

bool object_is_docked(p_object *objp);

// check whether other_objp is directly docked to objp
bool dock_check_find_direct_docked_object(p_object *objp, p_object *other_objp);

// find the object occupying objp's specified dockpoint
p_object *dock_find_object_at_dockpoint(p_object *objp, char *dockpoint);

// find objp's dockpoint being occupied by other_objp
char *dock_find_dockpoint_used_by_object(p_object *objp, p_object *other_objp);

// Überfunction for evaluating all objects that could possibly be docked to objp.  This will
// call "function" for each docked object.  The function should store its intermediate and
// return values in the p_dock_function_info class.
void dock_evaluate_all_docked_objects(p_object *objp, p_dock_function_info *infop, void (*function)(p_object *, p_dock_function_info *));

// docks all docked objects; called only from parse_create_object in missionparse.cpp
void dock_dock_docked_objects(p_object *objp);

// add objp1 and objp2 to each others' dock lists; currently only called by mission_parse_set_up_initial_docks
void dock_dock_objects(p_object *objp1, char *dockpoint1, p_object *objp2, char *dockpoint2);

// free the entire dock list without undocking anything; should only be used on object cleanup
void dock_free_dock_list(p_object *objp);

#endif	// _PARSE_OBJECT_DOCK_H
