/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "mission/missiongrid.h"

#define BRIEFING_LOOKAT_POINT_ID	99999

extern int	Aa_gridlines;
extern int	player_start1;
extern int	Editing_mode;
extern int	Control_mode;
extern int	Show_grid;
extern int	Show_grid_positions;
extern int	Show_coordinates;
extern int	Show_outlines;
extern int	Single_axis_constraint;
extern int	Show_distances;
extern int	Universal_heading;
extern int	Flying_controls_mode;
extern int	Group_rotate;
extern int	Show_horizon;
extern int	Lookat_mode;
extern int	True_rw, True_rh;
extern int	Fixed_briefing_size;
extern vec3d	Constraint, Anticonstraint;
extern vec3d	Tp1, Tp2;  // test points
extern physics_info view_physics;
extern vec3d view_pos, eye_pos;
extern matrix view_orient, eye_orient;

void fred_render_init();
void generate_starfield();
void move_mouse(int btn, int mdx, int mdy);
void game_do_frame();
void render_frame();
void level_controlled();
void verticalize_controlled();
