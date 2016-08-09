#ifndef _EDITOR_H
#define _EDITOR_H
/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
 */

#include "physics/physics.h"
#include "render/3d.h"
#include "mission/missiongrid.h"


#define MAX_GRID_POINTS		1000

void create_object(int objnum, vec3d *pos);

void test_form_wing(int x);

int select_object(int cx, int cy);

void game_init();

matrix Grid_gmatrix;

vec3d Grid_center;

int Show_stars;

void rpd_line(vec3d *v0, vec3d *v1);

#endif	// _EDITOR_H
