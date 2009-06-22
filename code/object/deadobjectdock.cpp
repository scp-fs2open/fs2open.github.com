/*
 * Created by Ian "Goober5000" Warfield for the FreeSpace2 Source Code Project.
 * You may not sell or otherwise commercially exploit the source or things you
 * create based on the source.
 */ 



#include "globalincs/pstypes.h"
#include "object/deadobjectdock.h"
#include "object/object.h"




// management prototypes

void dead_dock_add_instance(object *objp, int dockpoint, object *other_objp);
void dead_dock_remove_instance(object *objp, object *other_objp);
dock_instance *dead_dock_find_instance(object *objp, object *other_objp);
dock_instance *dead_dock_find_instance(object *objp, int dockpoint);



object *dock_get_first_dead_docked_object(object *objp)
{
	// are we docked?
	if (!object_is_dead_docked(objp))
		return NULL;

	return objp->dead_dock_list->docked_objp;
}

// dock management functions -------------------------------------------------------------------------------------
void dock_dead_dock_objects(object *objp1, int dockpoint1, object *objp2, int dockpoint2)
{
#ifndef NDEBUG
	if ((dead_dock_find_instance(objp1, objp2) != NULL) || (dead_dock_find_instance(objp2, objp1) != NULL))
	{
		Error(LOCATION, "Trying to dock an object that's already docked!\n");
	}

	if ((dead_dock_find_instance(objp1, dockpoint1) != NULL) || (dead_dock_find_instance(objp2, dockpoint2) != NULL))
	{
		Error(LOCATION, "Trying to dock to a dockpoint that's in use!\n");
	}
#endif

	// put objects on each others' dock lists 
	dead_dock_add_instance(objp1, dockpoint1, objp2);
	dead_dock_add_instance(objp2, dockpoint2, objp1);
}

void dock_dead_undock_objects(object *objp1, object *objp2)
{
#ifndef NDEBUG
	if ((dead_dock_find_instance(objp1, objp2) == NULL) || (dead_dock_find_instance(objp2, objp1) == NULL))
	{
		Error(LOCATION, "Trying to undock an object that isn't docked!\n");
	}
#endif

	// remove objects from each others' dock lists
	dead_dock_remove_instance(objp1, objp2);
	dead_dock_remove_instance(objp2, objp1);
}

void dead_dock_add_instance(object *objp, int dockpoint, object *other_objp)
{
	dock_instance *item;

	// create item
	item = (dock_instance *) vm_malloc(sizeof(dock_instance));
	item->dockpoint_used = dockpoint;
	item->docked_objp = other_objp;

	// prepend item to existing list
	item->next = objp->dead_dock_list;
	objp->dead_dock_list = item;
}

void dead_dock_remove_instance(object *objp, object *other_objp)
{
	int found = 0;
	dock_instance *prev_ptr, *ptr;
	
	prev_ptr = NULL;
	ptr = objp->dead_dock_list;

	// iterate until item found
	while (ptr != NULL)
	{
		// if found, exit loop
		if (ptr->docked_objp == other_objp)
		{
			found = 1;
			break;
		}

		// iterate
		prev_ptr = ptr;
		ptr = ptr->next;
	}

	// delete if found
	if (found)
	{
		// special case... found at beginning of list
		if (prev_ptr == NULL)
		{
			objp->dead_dock_list = ptr->next;
		}
		// normal case
		else
		{
			prev_ptr->next = ptr->next;
		}

		// delete it
		vm_free(ptr);
	}
}

dock_instance *dead_dock_find_instance(object *objp, object *other_objp)
{
	dock_instance *ptr = objp->dead_dock_list;

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

dock_instance *dead_dock_find_instance(object *objp, int dockpoint)
{
	dock_instance *ptr = objp->dead_dock_list;

	// iterate until item found
	while (ptr != NULL)
	{
		// if found, return it
		if (ptr->dockpoint_used == dockpoint)
			return ptr;

		// iterate
		ptr = ptr->next;
	}

	// not found
	return NULL;
}
