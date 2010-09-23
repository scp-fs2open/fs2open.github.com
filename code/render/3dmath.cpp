/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "graphics/2d.h"
#include "hud/hud.h" //For HUD_offset_*
#include "render/3dinternal.h"



#define MIN_Z 0.0f

//Codes a vector.  Returns the codes of a point.
ubyte g3_code_vector(vec3d * p)
{
	ubyte cc=0;

	if (p->xyz.x > p->xyz.z)
		cc |= CC_OFF_RIGHT;

	if (p->xyz.y > p->xyz.z)
		cc |= CC_OFF_TOP;

	if (p->xyz.x < -p->xyz.z)
		cc |= CC_OFF_LEFT;

	if (p->xyz.y < -p->xyz.z)
		cc |= CC_OFF_BOT;

	if (p->xyz.z < MIN_Z )
		cc |= CC_BEHIND;

	if ( G3_user_clip )	{
		// Check if behind user plane
		if ( g3_point_behind_user_plane(p))	{
			cc |= CC_OFF_USER;
		}
	}

	return cc;
}


//code a point.  fills in the p3_codes field of the point, and returns the codes
ubyte g3_code_vertex(vertex *p)
{
	ubyte cc=0;

	if (p->x > p->z)
		cc |= CC_OFF_RIGHT;

	if (p->y > p->z)
		cc |= CC_OFF_TOP;

	if (p->x < -p->z)
		cc |= CC_OFF_LEFT;

	if (p->y < -p->z)
		cc |= CC_OFF_BOT;

	if (p->z < MIN_Z )
		cc |= CC_BEHIND;

	if ( G3_user_clip )	{
		// Check if behind user plane
		if ( g3_point_behind_user_plane((vec3d *)&p->x))	{
			cc |= CC_OFF_USER;
		}
	}

	return p->codes = cc;

}

ubyte g3_transfer_vertex(vertex *dest,vec3d *src)
{
	dest->x = src->xyz.x;
	dest->y = src->xyz.y;
	dest->z = src->xyz.z;

	dest->codes = 0;
	dest->flags |= PF_PROJECTED;

	return 0;
}


MONITOR( NumRotations )

ubyte g3_rotate_vertex(vertex *dest,vec3d *src)
{
#if 0
	vec3d tempv;
	Assert( G3_count == 1 );
	vm_vec_sub(&tempv,src,&View_position);
	vm_vec_rotate( (vec3d *)&dest->x, &tempv, &View_matrix );
	dest->flags = 0;	//not projected
	return g3_code_vertex(dest);
#else
	float tx, ty, tz, x,y,z;
	ubyte codes;

	MONITOR_INC( NumRotations, 1 );	

	tx = src->xyz.x - View_position.xyz.x;
	ty = src->xyz.y - View_position.xyz.y;
	tz = src->xyz.z - View_position.xyz.z;

	x = tx * View_matrix.vec.rvec.xyz.x;
	x += ty * View_matrix.vec.rvec.xyz.y;
	x += tz * View_matrix.vec.rvec.xyz.z;

	y = tx * View_matrix.vec.uvec.xyz.x;
	y += ty * View_matrix.vec.uvec.xyz.y;
	y += tz * View_matrix.vec.uvec.xyz.z;

	z = tx * View_matrix.vec.fvec.xyz.x;
	z += ty * View_matrix.vec.fvec.xyz.y;
	z += tz * View_matrix.vec.fvec.xyz.z;

	codes = 0;

	if (x > z)			codes |= CC_OFF_RIGHT;
	if (x < -z)			codes |= CC_OFF_LEFT;
	if (y > z)			codes |= CC_OFF_TOP;
	if (y < -z)			codes |= CC_OFF_BOT;
	if (z < MIN_Z )	codes |= CC_BEHIND;

	dest->x = x;
	dest->y = y;
	dest->z = z;

	if ( G3_user_clip )	{
		// Check if behind user plane
		if ( g3_point_behind_user_plane((vec3d *)&dest->x))	{
			codes |= CC_OFF_USER;
		}
	}

	dest->codes = codes;

	dest->flags = 0;	// not projected

//	vm_vec_copy_scale(&dest->real_pos, src,1);

	return codes;
#endif
}	


ubyte g3_rotate_faraway_vertex(vertex *dest,vec3d *src)
{	
	Assert( G3_count == 1 );

	MONITOR_INC( NumRotations, 1 );	

	vm_vec_rotate( (vec3d *)&dest->x, src, &View_matrix );
	dest->flags = 0;	//not projected
	return g3_code_vertex(dest);
}	


//rotates a point. returns codes.  does not check if already rotated
ubyte g3_rotate_vector(vec3d *dest,vec3d *src)
{
	vec3d tempv;

	Assert( G3_count == 1 );

	MONITOR_INC( NumRotations, 1 );	

	vm_vec_sub(&tempv,src,&View_position);
	vm_vec_rotate(dest,&tempv,&View_matrix);
	return g3_code_vector(dest);
}	
		
ubyte g3_project_vector(vec3d *p, float *sx, float *sy )
{
	float w;

	Assert( G3_count == 1 );

	if ( p->xyz.z <= MIN_Z ) return PF_OVERFLOW;

	w=1.0f / p->xyz.z;

	*sx = (Canvas_width + (p->xyz.x*Canvas_width*w))*0.5f;
	*sy = (Canvas_height - (p->xyz.y*Canvas_height*w))*0.5f;
	return PF_PROJECTED;
}

//projects a point. Checks for overflow.

int g3_project_vertex(vertex *p)
{
	float w;

	Assert( G3_count == 1 );

	if ( p->flags & PF_PROJECTED )
		return p->flags;

	//if ( p->z < MIN_Z ) {
	if ( p->z <= MIN_Z ) {
		p->flags |= PF_OVERFLOW;
	} else {
		// w = (p->z == 0.0f) ? 100.0f : 1.0f / p->z;
		w = 1.0f / p->z;
		p->sx = (Canvas_width + (p->x*Canvas_width*w))*0.5f;
		p->sy = (Canvas_height - (p->y*Canvas_height*w))*0.5f;

		if ( w > 1.0f ) w = 1.0f;		
		
		p->sw = w;
		p->flags |= PF_PROJECTED;
	}
	
	return p->flags;
}


//from a 2d point, compute the vector through that point
void g3_point_to_vec(vec3d *v,int sx,int sy)
{
	vec3d	tempv;

	Assert( G3_count == 1 );

	tempv.xyz.x =  ((float)sx - Canv_w2) / Canv_w2;
	tempv.xyz.y = -((float)sy - Canv_h2) / Canv_h2;
	tempv.xyz.z = 1.0f;

	tempv.xyz.x = tempv.xyz.x * Matrix_scale.xyz.z / Matrix_scale.xyz.x;
	tempv.xyz.y = tempv.xyz.y * Matrix_scale.xyz.z / Matrix_scale.xyz.y;

	vm_vec_normalize(&tempv);
	vm_vec_unrotate(v, &tempv, &Unscaled_matrix);
}

//from a 2d point, compute the vector through that point.
// This can be called outside of a g3_start_frame/g3_end_frame
// pair as long g3_start_frame was previously called.
void g3_point_to_vec_delayed(vec3d *v,int sx,int sy)
{
	vec3d	tempv;

	tempv.xyz.x =  ((float)sx - Canv_w2) / Canv_w2;
	tempv.xyz.y = -((float)sy - Canv_h2) / Canv_h2;
	tempv.xyz.z = 1.0f;

	tempv.xyz.x = tempv.xyz.x * Matrix_scale.xyz.z / Matrix_scale.xyz.x;
	tempv.xyz.y = tempv.xyz.y * Matrix_scale.xyz.z / Matrix_scale.xyz.y;

	vm_vec_normalize(&tempv);
	vm_vec_unrotate(v, &tempv, &Unscaled_matrix);
}

vec3d *g3_rotate_delta_vec(vec3d *dest,vec3d *src)
{
	Assert( G3_count == 1 );
	return vm_vec_rotate(dest,src,&View_matrix);
}

//	vms_vector tempv;
//
//	tempv.xyz.x =  fixmuldiv(fixdiv((sx<<16) - Canv_w2,Canv_w2),Matrix_scale.xyz.z,Matrix_scale.xyz.x);
//	tempv.xyz.y = -fixmuldiv(fixdiv((sy<<16) - Canv_h2,Canv_h2),Matrix_scale.xyz.z,Matrix_scale.xyz.y);
//	tempv.xyz.z = f1_0;
//
//	vm_vec_normalize(&tempv);
//
//	vm_vec_unrotate(v, &tempv, &Unscaled_matrix);

/*

//from a 2d point, compute the vector through that point
void g3_point_2_vec(vec3d *v,int sx,int sy)
{
	vec3d tempv;
	matrix tempm;

	tempv.xyz.x =  fixmuldiv(fixdiv((sx<<16) - Canv_w2,Canv_w2),Matrix_scale.xyz.z,Matrix_scale.xyz.x);
	tempv.xyz.y = -fixmuldiv(fixdiv((sy<<16) - Canv_h2,Canv_h2),Matrix_scale.xyz.z,Matrix_scale.xyz.y);
	tempv.xyz.z = f1_0;

	vm_vec_normalize(&tempv);

	vm_vec_unrotate(v, &tempv, &Unscaled_matrix);
}

//delta rotation functions
vms_vector *g3_rotate_delta_x(vms_vector *dest,fix dx)
{
	dest->x = fixmul(View_matrix.vec.rvec.xyz.x,dx);
	dest->y = fixmul(View_matrix.vec.uvec.xyz.x,dx);
	dest->z = fixmul(View_matrix.vec.fvec.xyz.x,dx);

	return dest;
}

vms_vector *g3_rotate_delta_y(vms_vector *dest,fix dy)
{
	dest->x = fixmul(View_matrix.vec.rvec.xyz.y,dy);
	dest->y = fixmul(View_matrix.vec.uvec.xyz.y,dy);
	dest->z = fixmul(View_matrix.vec.fvec.xyz.y,dy);

	return dest;
}

vms_vector *g3_rotate_delta_z(vms_vector *dest,fix dz)
{
	dest->x = fixmul(View_matrix.vec.rvec.xyz.z,dz);
	dest->y = fixmul(View_matrix.vec.uvec.xyz.z,dz);
	dest->z = fixmul(View_matrix.vec.fvec.xyz.z,dz);

	return dest;
}



ubyte g3_add_delta_vec(g3s_point *dest,g3s_point *src,vms_vector *deltav)
{
	vm_vec_add(&dest->p3_vec,&src->p3_vec,deltav);

	dest->p3_flags = 0;		//not projected

	return g3_code_point(dest);
}
*/

// calculate the depth of a point - returns the z coord of the rotated point
float g3_calc_point_depth(vec3d *pnt)
{
	float q;

	q  = (pnt->xyz.x - View_position.xyz.x) * View_matrix.vec.fvec.xyz.x;
	q += (pnt->xyz.y - View_position.xyz.y) * View_matrix.vec.fvec.xyz.y;
	q += (pnt->xyz.z - View_position.xyz.z) * View_matrix.vec.fvec.xyz.z;

	return q;
}
