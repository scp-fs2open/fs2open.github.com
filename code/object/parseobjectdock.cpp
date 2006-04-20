/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 

/*
 * $Logfile: /Freespace2/code/Object/ParseObjectDock.cpp $
 * $Revision: 1.3 $
 * $Date: 2006-04-20 06:32:23 $
 * $Author: Goober5000 $
 *
 * Implementation of new docking system for parse objects
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2005/10/31 09:12:16  Goober5000
 * remove unused commented stuff
 * --Goober5000
 *
 * Revision 1.1  2005/10/29 22:09:30  Goober5000
 * multiple ship docking implemented for initially docked ships
 * --Goober5000
 *
 */

#include "globalincs/pstypes.h"
#include "object/parseobjectdock.h"
#include "mission/missionparse.h"
#include "math/bitarray.h"




// helper prototypes

void dock_evaluate_tree(p_object *objp, p_dock_function_info *infop, void (*function)(p_object *, p_dock_function_info *), char *visited_bitstring);
void dock_dock_docked_children_tree(p_object *objp, p_object *parent_objp);


// management prototypes

void dock_add_instance(p_object *objp, char *dockpoint, p_object *other_objp);
p_dock_instance *dock_find_instance(p_object *objp, p_object *other_objp);
p_dock_instance *dock_find_instance(p_object *objp, char *dockpoint);


bool object_is_docked(p_object *objp)
{
	return (objp->dock_list != NULL);
}

p_object *dock_get_first_docked_object(p_object *objp)
{
	// are we docked?
	if (!object_is_docked(objp))
		return NULL;

	return objp->dock_list->docked_objp;
}

bool dock_check_docked_one_on_one(p_object *objp)
{
	// we must be docked
	if (!object_is_docked(objp))
		return false;
	
	// our dock list must contain only one object
	if (objp->dock_list->next != NULL)
		return false;

	// the other guy's dock list must contain only one object
	if (dock_get_first_docked_object(objp)->dock_list->next != NULL)
		return false;

	// debug check to make sure that we're docked to each other
	Assert(objp == dock_get_first_docked_object(objp)->dock_list->docked_objp);
	
	// success
	return true;
}

bool dock_check_find_direct_docked_object(p_object *objp, p_object *other_objp)
{
	return (dock_find_instance(objp, other_objp) != NULL);
}

char *dock_find_dockpoint_used_by_object(p_object *objp, p_object *other_objp)
{
	p_dock_instance *result = dock_find_instance(objp, other_objp);
	
	if (result == NULL)
		return NULL;
	else
		return result->dockpoint_used;
}

// functions to deal with all docked objects anywhere
// ---------------------------------------------------------------------------------------------------------------

// universal two functions
// -----------------------

// evaluate a certain function for all docked objects
void dock_evaluate_all_docked_objects(p_object *objp, p_dock_function_info *infop, void (*function)(p_object *, p_dock_function_info *))
{
	Assert((objp != NULL) && (infop != NULL) && (function != NULL));

	// not docked?
	if (!object_is_docked(objp))
	{
		// call the function for just the one object
		function(objp, infop);
		return;
	}

	// we only have two objects docked
	if (dock_check_docked_one_on_one(objp))
	{
		// call the function for the first object, and return if instructed
		function(objp, infop);
		if (infop->early_return_condition) return;

		// call the function for the second object, and return if instructed
		function(objp->dock_list->docked_objp, infop);
		if (infop->early_return_condition) return;
	}

	// NOTE - never treat a group of parse objects as a hub... it cuts down on bugs, and it's
	// not needed because it's not time-critical

	// we have multiple objects docked and we must treat them as a tree
	else
	{
		// create a bit array to mark the objects we checked
		char visited_bitstring[(MAX_PARSE_OBJECTS >> 3) + 1];

		// clear it
		memset(visited_bitstring, 0, (MAX_PARSE_OBJECTS >> 3) + 1);

		// start evaluating the tree
		dock_evaluate_tree(objp, infop, function, visited_bitstring);
	}
}

void dock_evaluate_tree(p_object *objp, p_dock_function_info *infop, void (*function)(p_object *, p_dock_function_info *), char *visited_bitstring)
{
	// make sure we haven't visited this object already
	if (get_bit(visited_bitstring, POBJ_INDEX(objp)))
		return;

	// mark as visited
	set_bit(visited_bitstring, POBJ_INDEX(objp));

	// call the function for this object, and return if instructed
	function(objp, infop);
	if (infop->early_return_condition) return;

	// iterate through all docked objects
	for (p_dock_instance *ptr = objp->dock_list; ptr != NULL; ptr = ptr->next)
	{
		// start another tree with the docked object as the root, and return if instructed
		dock_evaluate_tree(ptr->docked_objp, infop, function, visited_bitstring);
		if (infop->early_return_condition) return;
	}
}

// special-case functions
// ----------------------

void dock_dock_docked_objects(p_object *objp)
{
	if (!object_is_docked(objp))
		return;

	// has this object (by extension, this group of docked objects) been handled already?
	if (objp->flags2 & P2_ALREADY_HANDLED)
		return;

	Assert(objp->flags & P_SF_DOCK_LEADER);

	p_dock_function_info dfi;
	
	// start a tree with that object as the parent... do NOT use the überfunction for this,
	// because we must use a tree for the parent ancestry to work correctly

	// we don't need a bit array because P2_ALREADY_HANDLED takes care of it

	// start evaluating the tree, starting with the dock leader
	dock_dock_docked_children_tree(objp, NULL);
}

void dock_dock_docked_children_tree(p_object *objp, p_object *parent_objp)
{
	// has this object been handled already?
	if (objp->flags2 & P2_ALREADY_HANDLED)
		return;

	// mark as handled
	objp->flags2 |= P2_ALREADY_HANDLED;

	// if parent_objp exists
	if (parent_objp != NULL)
	{
		// dock this object to it
		parse_dock_one_docked_object(objp, parent_objp);
	}

	// iterate through all docked objects
	for (p_dock_instance *ptr = objp->dock_list; ptr != NULL; ptr = ptr->next)
	{
		// start another tree with the docked object as the root and this object as the parent
		dock_dock_docked_children_tree(ptr->docked_objp, objp);
	}
}
// ---------------------------------------------------------------------------------------------------------------
// end of über code block ----------------------------------------------------------------------------------------

// dock management functions -------------------------------------------------------------------------------------
void dock_dock_objects(p_object *objp1, char *dockpoint1, p_object *objp2, char *dockpoint2)
{
#ifndef NDEBUG
	if ((dock_find_instance(objp1, objp2) != NULL) || (dock_find_instance(objp2, objp1) != NULL))
	{
		Error(LOCATION, "Trying to dock an object that's already docked!\n");
	}

	if ((dock_find_instance(objp1, dockpoint1) != NULL) || (dock_find_instance(objp2, dockpoint2) != NULL))
	{
		Error(LOCATION, "Trying to dock to a dockpoint that's in use!\n");
	}
#endif

	// put objects on each others' dock lists 
	dock_add_instance(objp1, dockpoint1, objp2);
	dock_add_instance(objp2, dockpoint2, objp1);
}

// dock list functions -------------------------------------------------------------------------------------------
void dock_add_instance(p_object *objp, char *dockpoint, p_object *other_objp)
{
	p_dock_instance *item;

	// create item
	item = (p_dock_instance *) vm_malloc(sizeof(p_dock_instance));
	strcpy(item->dockpoint_used, dockpoint);
	item->docked_objp = other_objp;

	// prepend item to existing list
	item->next = objp->dock_list;
	objp->dock_list = item;
}

// this is only allowed for parse objects because we can just free the lists
// at mission close without worrying about undocking them
void dock_free_instances(p_object *objp)
{
	// delete the whole list
	while (objp->dock_list != NULL)
	{
		// save first item
		p_dock_instance *ptr = objp->dock_list;

		// remove it from the list
		objp->dock_list = ptr->next;

		// delete it
		vm_free(ptr);
	}
}

p_dock_instance *dock_find_instance(p_object *objp, p_object *other_objp)
{
	p_dock_instance *ptr = objp->dock_list;

	// iterate until item found
	while (ptr != NULL)
	{
		// if found, return it
		if (ptr->docked_objp == other_objp)
			return ptr;

		// iterate
		ptr = ptr->next;
	}

	// not found
	return NULL;
}

p_dock_instance *dock_find_instance(p_object *objp, char *dockpoint)
{
	p_dock_instance *ptr = objp->dock_list;

	// iterate until item found
	while (ptr != NULL)
	{
		// if found, return it
		if (!strcmp(ptr->dockpoint_used, dockpoint))
			return ptr;

		// iterate
		ptr = ptr->next;
	}

	// not found
	return NULL;
}
