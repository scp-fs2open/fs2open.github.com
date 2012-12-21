/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#include "io/key.h"
#include "mission/missiongrid.h"
#include "math/fvi.h"
#include "render/3d.h"
#include "graphics/2d.h"


grid	Global_grid;
grid	*The_grid;
int	double_fine_gridlines = 0;

void grid_read_camera_controls( control_info * ci, float frametime )
{
	float kh;

	{
		float temp = ci->heading;
		float temp1 = ci->pitch;
		memset( ci, 0, sizeof(control_info) );
		ci->heading = temp;
		ci->pitch = temp1;
	}

	// From keyboard...
	kh = key_down_timef(KEY_PAD6) - key_down_timef(KEY_PAD4);
	if (kh == 0.0f)
		ci->heading = 0.0f;
	else if (kh > 0.0f) {
		if (ci->heading < 0.0f)
			ci->heading = 0.0f;
	} else // kh < 0
		if (ci->heading > 0.0f)
			ci->heading = 0.0f;
	ci->heading += kh;

	kh = key_down_timef(KEY_PAD8) - key_down_timef(KEY_PAD2);
	if (kh == 0.0f)
		ci->pitch = 0.0f;
	else if (kh > 0.0f) {
		if (ci->pitch < 0.0f)
			ci->pitch = 0.0f;
	} else // kh < 0
		if (ci->pitch > 0.0f)
			ci->pitch = 0.0f;
	ci->pitch += kh;

	ci->bank = (key_down_timef(KEY_PAD7) - key_down_timef(KEY_PAD9));
	ci->forward = (key_down_timef(KEY_A) - key_down_timef(KEY_Z));
	ci->sideways = (key_down_timef(KEY_PAD3) - key_down_timef(KEY_PAD1));
	ci->vertical = (key_down_timef(KEY_PADMINUS) - key_down_timef(KEY_PADPLUS));
}

//	Project the viewer's position onto the grid plane.  If more than threshold distance
//	from grid center, move grid center.
void maybe_create_new_grid(grid* gridp, vec3d *pos, matrix *orient, int force)
{
	int roundoff;
	plane	tplane;
	vec3d	gpos, tmp, c;
	float	dist_to_plane;
	float	square_size, ux, uy, uz;

	ux = tplane.A = gridp->gmatrix.vec.uvec.xyz.x;
	uy = tplane.B = gridp->gmatrix.vec.uvec.xyz.y;
	uz = tplane.C = gridp->gmatrix.vec.uvec.xyz.z;
	tplane.D = gridp->planeD;

	compute_point_on_plane(&c, &tplane, pos);
	dist_to_plane = fl_abs(vm_dist_to_plane(pos, &gridp->gmatrix.vec.uvec, &c));
	square_size = 1.0f;
	while (dist_to_plane >= 25.0f)
	{
		square_size *= 10.0f;
		dist_to_plane /= 10.0f;
	}
	
	if (fvi_ray_plane(&gpos, &gridp->center, &gridp->gmatrix.vec.uvec, pos, &orient->vec.fvec, 0.0f)<0.0f)	{
		vec3d p;
		vm_vec_scale_add(&p,pos,&orient->vec.fvec, 100.0f );
		compute_point_on_plane(&gpos, &tplane, &p );
	}

	if (vm_vec_dist(&gpos, &c) > 50.0f * square_size)
	{
		vm_vec_sub(&tmp, &gpos, &c);
		vm_vec_normalize(&tmp);
		vm_vec_scale_add(&gpos, &c, &tmp, 50.0f * square_size);
	}

	roundoff = (int) square_size * 10;
	if (!ux)
		gpos.xyz.x = fl_roundoff(gpos.xyz.x, roundoff);
	if (!uy)
		gpos.xyz.y = fl_roundoff(gpos.xyz.y, roundoff);
	if (!uz)
		gpos.xyz.z = fl_roundoff(gpos.xyz.z, roundoff);

	if ((square_size != gridp->square_size) ||
		(gpos.xyz.x != gridp->center.xyz.x) ||
		(gpos.xyz.y != gridp->center.xyz.y) ||
		(gpos.xyz.z != gridp->center.xyz.z) || force)
	{
		gridp->square_size = square_size;
		gridp->center = gpos;
		modify_grid(gridp);
	}
}

//	Create a grid
//	*forward is vector pointing forward
//	*right is vector pointing right
//	*center is center point of grid
//	length is length of grid
//	width is width of grid
//	square_size is size of a grid square
//	For example:
//		*forward = (0.0, 0.0, 1.0)
//		*right   = (1.0, 0.0, 0.0)
//		*center = (0.0, 0.0, 0.0)
//		nrows = 10
//		ncols =  50.0
//		square_size = 10.0
//	will generate a grid of squares 10 long by 5 wide.
//	Each grid square will be 10.0 x 10.0 units.
//	The center of the grid will be at the global origin.
//	The grid will be parallel to the xz plane (because the normal is 0,1,0).
//	(In fact, it will be the xz plane because it is centered on the origin.)
//
//	Stuffs grid in *gridp.  If gridp == NULL, mallocs and returns a grid.
grid *create_grid(grid *gridp, vec3d *forward, vec3d *right, vec3d *center, int nrows, int ncols, float square_size)
{
	int	i, ncols2, nrows2, d = 1;
	vec3d	dfvec, drvec, cur, cur2, tvec, uvec, save, save2;

	Assert(square_size > 0.0);
	if (double_fine_gridlines)
		d = 2;

	if (gridp == NULL)
		gridp = (grid *) vm_malloc(sizeof(grid));

	Assert(gridp);

	gridp->center = *center;
	gridp->square_size = square_size;

	//	Create the plane equation.
	Assert(!IS_VEC_NULL(forward));
	Assert(!IS_VEC_NULL(right));

	vm_vec_copy_normalize(&dfvec, forward);
	vm_vec_copy_normalize(&drvec, right);

	vm_vec_cross(&uvec, &dfvec, &drvec);
	
	Assert(!IS_VEC_NULL(&uvec));

	gridp->gmatrix.vec.uvec = uvec;

	gridp->planeD = -(center->xyz.x * uvec.xyz.x + center->xyz.y * uvec.xyz.y + center->xyz.z * uvec.xyz.z);
	Assert(!_isnan(gridp->planeD));

	gridp->gmatrix.vec.fvec = dfvec;
	gridp->gmatrix.vec.rvec = drvec;

	vm_vec_scale(&dfvec, square_size);
	vm_vec_scale(&drvec, square_size);

	vm_vec_scale_add(&cur, center, &dfvec, (float) -nrows * d / 2);
	vm_vec_scale_add2(&cur, &drvec, (float) -ncols * d / 2);
	vm_vec_scale_add(&cur2, center, &dfvec, (float) -nrows * 5 / 2);
	vm_vec_scale_add2(&cur2, &drvec, (float) -ncols * 5 / 2);
	save = cur;
	save2 = cur2;

	gridp->ncols = ncols;
	gridp->nrows = nrows;
	ncols2 = ncols / 2;
	nrows2 = nrows / 2;
	Assert(ncols < MAX_GRIDLINE_POINTS && nrows < MAX_GRIDLINE_POINTS);

	// Create the points along the edges of the grid, so we can just draw lines
	// between them to form the grid.  
	for (i=0; i<=ncols*d; i++) {
		gridp->gpoints1[i] = cur;  // small, dark gridline points
		vm_vec_scale_add(&tvec, &cur, &dfvec, (float) nrows * d);
		gridp->gpoints2[i] = tvec;
		vm_vec_add2(&cur, &drvec);
	}

	for (i=0; i<=ncols2; i++) {
		gridp->gpoints5[i] = cur2;  // large, brighter gridline points
		vm_vec_scale_add(&tvec, &cur2, &dfvec, (float) nrows2 * 10);
		gridp->gpoints6[i] = tvec; 
		vm_vec_scale_add2(&cur2, &drvec, 10.0f);
	}

	cur = save;
	cur2 = save2;
	for (i=0; i<=nrows*d; i++) {
		gridp->gpoints3[i] = cur;  // small, dark gridline points
		vm_vec_scale_add(&tvec, &cur, &drvec, (float) ncols * d);
		gridp->gpoints4[i] = tvec;
		vm_vec_add2(&cur, &dfvec);
	}

	for (i=0; i<=nrows2; i++) {
		gridp->gpoints7[i] = cur2;  // large, brighter gridline points
		vm_vec_scale_add(&tvec, &cur2, &drvec, (float) ncols2 * 10);
		gridp->gpoints8[i] = tvec;
		vm_vec_scale_add2(&cur2, &dfvec, 10.0f);
	}

	return gridp;
}

//	Create a nice grid -- centered at origin, 10x10, 10.0 size squares, in xz plane.
grid *create_default_grid(void)
{
	grid	*rgrid;
	vec3d	fvec, rvec, cvec;

	vm_vec_make(&fvec, 0.0f, 0.0f, 1.0f);
	vm_vec_make(&rvec, 1.0f, 0.0f, 0.0f);
	vm_vec_make(&cvec, 0.0f, 0.0f, 0.0f);

	rgrid = create_grid(&Global_grid, &fvec, &rvec, &cvec, 100, 100, 5.0f);

	physics_init(&rgrid->physics);
	return rgrid;
}

//	Rotate and project points and draw a line.
void rpd_line(vec3d *v0, vec3d *v1)
{
	vertex	tv0, tv1;

   	g3_rotate_vertex(&tv0, v0);
  	g3_rotate_vertex(&tv1, v1);
  	g3_draw_line(&tv0, &tv1);
}


void modify_grid(grid *gridp)
{
	create_grid(gridp, &gridp->gmatrix.vec.fvec, &gridp->gmatrix.vec.rvec, &gridp->center,
		gridp->nrows, gridp->ncols, gridp->square_size);
}

void grid_render_elevation_line(vec3d *pos, grid* gridp)
{
	vec3d	gpos;	//	Location of point on grid.
	vec3d	tpos;
	float		dxz;
	plane		tplane;
	vec3d	*gv;
	
	tplane.A = gridp->gmatrix.vec.uvec.xyz.x;
	tplane.B = gridp->gmatrix.vec.uvec.xyz.y;
	tplane.C = gridp->gmatrix.vec.uvec.xyz.z;
	tplane.D = gridp->planeD;

	compute_point_on_plane(&gpos, &tplane, pos);

	dxz = vm_vec_dist(pos, &gpos)/8.0f;

	gv = &gridp->gmatrix.vec.uvec;
	if (gv->xyz.x * pos->xyz.x + gv->xyz.y * pos->xyz.y + gv->xyz.z * pos->xyz.z < -gridp->planeD)
		gr_set_color(127, 127, 127);
	else
		gr_set_color(255, 255, 255);   // white

	rpd_line(&gpos, pos);	//	Line from grid to object center.

	tpos = gpos;

	vm_vec_scale_add2(&gpos, &gridp->gmatrix.vec.rvec, -dxz/2);
	vm_vec_scale_add2(&gpos, &gridp->gmatrix.vec.fvec, -dxz/2);
	
	vm_vec_scale_add2(&tpos, &gridp->gmatrix.vec.rvec, dxz/2);
	vm_vec_scale_add2(&tpos, &gridp->gmatrix.vec.fvec, dxz/2);
	
	rpd_line(&gpos, &tpos);

	vm_vec_scale_add2(&gpos, &gridp->gmatrix.vec.rvec, dxz);
	vm_vec_scale_add2(&tpos, &gridp->gmatrix.vec.rvec, -dxz);

	rpd_line(&gpos, &tpos);
}
