#pragma once

#include <globalincs/pstypes.h>

class object;

void    object_moved(object *ptr);
// Calls object_moved() for each marked object, so that their docked partners move with them.
// Returns the number of marked objects.
int     marked_objects_moved();
bool     query_valid_object(int index);

const char *object_name(int obj);

int get_ship_from_obj(int obj);

int get_ship_from_obj(object *objp);
