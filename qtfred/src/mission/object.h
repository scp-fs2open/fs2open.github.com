#pragma once
class object;

void    object_moved(object *ptr);
int     query_valid_object(int index);

const char *object_name(int obj);
