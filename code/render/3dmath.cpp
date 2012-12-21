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

/**
 * Codes a vector.  Returns the codes of a point.
 */
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


/**
 * Code a point.  fills in the p3_codes field of the point, and returns the codes
 */
ubyte g3_code_vertex(vertex *p)
{
	ubyte cc=0;

	if (p->world.xyz.x > p->world.xyz.z)
		cc |= CC_OFF_RIGHT;

	if (p->world.xyz.y > p->world.xyz.z)
		cc |= CC_OFF_TOP;

	if (p->world.xyz.x < -p->world.xyz.z)
		cc |= CC_OFF_LEFT;

	if (p->world.xyz.y < -p->world.xyz.z)
		cc |= CC_OFF_BOT;

	if (p->world.xyz.z < MIN_Z )
		cc |= CC_BEHIND;

	if ( G3_user_clip )	{
		// Check if behind user plane
		if ( g3_point_behind_user_plane(&p->world))	{
			cc |= CC_OFF_USER;
		}
	}

	return p->codes = cc;

}

ubyte g3_transfer_vertex(vertex *dest,vec3d *src)
{
	dest->world = *src;

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
	y += tz * View_matrix.vec.uvec.xyz.z; //-V537

	z = tx * View_matrix.vec.fvec.xyz.x;
	z += ty * View_matrix.vec.fvec.xyz.y;
	z += tz * View_matrix.vec.fvec.xyz.z;

	codes = 0;

	if (x > z)			codes |= CC_OFF_RIGHT;
	if (x < -z)			codes |= CC_OFF_LEFT;
	if (y > z)			codes |= CC_OFF_TOP;
	if (y < -z)			codes |= CC_OFF_BOT;
	if (z < MIN_Z )		codes |= CC_BEHIND;

	dest->world.xyz.x = x;
	dest->world.xyz.y = y;
	dest->world.xyz.z = z;

	if ( G3_user_clip )	{
		// Check if behind user plane
		if ( g3_point_behind_user_plane(&dest->world))	{
			codes |= CC_OFF_USER;
		}
	}

	dest->codes = codes;

	dest->flags = 0;	// not projected

	return codes;
#endif
}	


ubyte g3_rotate_faraway_vertex(vertex *dest,vec3d *src)
{	
	Assert( G3_count == 1 );

	MONITOR_INC( NumRotations, 1 );	

	vm_vec_rotate( &dest->world, src, &View_matrix );
	dest->flags = 0;	//not projected
	return g3_code_vertex(dest);
}	


/**
 * Rotates a point. returns codes.  does not check if already rotated
 */
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

/**
 * Projects a point. Checks for overflow.
 */
int g3_project_vertex(vertex *p)
{
	float w;

	Assert( G3_count == 1 );

	if ( p->flags & PF_PROJECTED )
		return p->flags;

	if ( p->world.xyz.z <= MIN_Z ) {
		p->flags |= PF_OVERFLOW;
	} else {
		w = 1.0f / p->world.xyz.z;
		p->screen.xyw.x = (Canvas_width + (p->world.xyz.x*Canvas_width*w))*0.5f;
		p->screen.xyw.y = (Canvas_height - (p->world.xyz.y*Canvas_height*w))*0.5f;

		if ( w > 1.0f ) w = 1.0f;		
		
		p->screen.xyw.w = w;
		p->flags |= PF_PROJECTED;
	}
	
	return p->flags;
}


/**
 * From a 2d point, compute the vector through that point
 */
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

/**
 * From a 2d point, compute the vector through that point.
 *
 * This can be called outside of a g3_start_frame/g3_end_frame
 * pair as long g3_start_frame was previously called.
 */
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

/**
 * Calculate the depth of a point - returns the z coord of the rotated point
 */
float g3_calc_point_depth(vec3d *pnt)
{
	float q;

	q  = (pnt->xyz.x - View_position.xyz.x) * View_matrix.vec.fvec.xyz.x;
	q += (pnt->xyz.y - View_position.xyz.y) * View_matrix.vec.fvec.xyz.y;
	q += (pnt->xyz.z - View_position.xyz.z) * View_matrix.vec.fvec.xyz.z;

	return q;
}
