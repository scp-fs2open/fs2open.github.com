#include "object.h"
#include "globalincs/linklist.h"
#include "object/object.h"
#include "object/waypoint.h"
#include "object/objectdock.h"

void object_moved(object *objp)
{
    if (objp->type == OBJ_WAYPOINT)
    {
        waypoint *wpt = find_waypoint_with_instance(objp->instance);
        Assert(wpt != NULL);
        wpt->set_pos(&objp->pos);
    }

    if ((objp->type == OBJ_SHIP) || (objp->type == OBJ_START)) // do we have a ship?
    {
        // reset the already-handled flag (inefficient, but it's FRED, so who cares)
        for (int i = 0; i < MAX_OBJECTS; i++)
            Objects[i].flags.set(Object::Object_Flags::Docked_already_handled);

        // move all docked objects docked to me
        dock_move_docked_objects(objp);
    }
}

int query_valid_object(int index)
{
    int obj_found = FALSE;
    object *ptr;

    if (index < 0 || index >= MAX_OBJECTS || Objects[index].type == OBJ_NONE)
        return FALSE;

    ptr = GET_FIRST(&obj_used_list);
    while (ptr != END_OF_LIST(&obj_used_list)) {
        Assert(ptr->type != OBJ_NONE);
        if (OBJ_INDEX(ptr) == index)
            obj_found = TRUE;

        ptr = GET_NEXT(ptr);
    }

    Assert(obj_found);  // just to make sure it's in the list like it should be.
    return TRUE;
}
