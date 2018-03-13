#pragma once

#include <globalincs/pstypes.h>

class object;

void    object_moved(object *ptr);
bool     query_valid_object(int index);

const char *object_name(int obj);

int get_ship_from_obj(int obj);

int get_ship_from_obj(object *objp);
