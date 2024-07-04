
#ifndef _GROPENGLQUERY_H
#define _GROPENGLQUERY_H
#pragma once

#include "graphics/2d.h"

int gr_opengl_create_query_object();

void gr_opengl_query_value(int obj, QueryType type);

bool gr_opengl_query_value_available(int obj);

std::uint64_t gr_opengl_get_query_value(int obj);

void gr_opengl_delete_query_object(int obj);

#endif // _GROPENGLQUERY_H
