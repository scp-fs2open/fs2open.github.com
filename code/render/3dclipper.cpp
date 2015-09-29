/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "graphics/tmapper.h"
#include "render/3dinternal.h"


int free_point_num=0;

vertex temp_points[TMAP_MAX_VERTS];
vertex *free_points[TMAP_MAX_VERTS];

void init_free_points(void)
{
	int i;

	for (i=0;i<TMAP_MAX_VERTS;i++)
		free_points[i] = &temp_points[i];
}


vertex *get_temp_point()
{
	vertex *p;

	p = free_points[free_point_num++];

	p->flags = PF_TEMP_POINT;

	return p;
}


void free_temp_point(vertex *p)
{
	Assert(p->flags & PF_TEMP_POINT);

	free_points[--free_point_num] = p;

	p->flags &= ~PF_TEMP_POINT;
}


/**
 * @brief Clips an edge against one plane.
 */
vertex *clip_edge(int plane_flag,vertex *on_pnt,vertex *off_pnt, uint flags)
{
	float ratio;
	vertex *tmp;

	tmp = get_temp_point();

	if ( plane_flag & CC_OFF_USER )	{

		// Clip with user-defined plane
		vec3d w, ray_direction;
		float num,den;

		vm_vec_sub(&ray_direction,&off_pnt->world,&on_pnt->world);
			
		vm_vec_sub(&w,&on_pnt->world,&G3_user_clip_point);
	
		den = -vm_vec_dot(&G3_user_clip_normal,&ray_direction);
		if ( den == 0.0f ) {	// Ray & plane are parallel, so there is no intersection
			Int3();	// Get John
			ratio = 1.0f;
		} else {
			num =  vm_vec_dot(&G3_user_clip_normal,&w);
	
			ratio = num / den;
		}
		
		vm_vec_sub(&tmp->world, &off_pnt->world, &on_pnt->world);
		vm_vec_scale(&tmp->world, ratio);
		vm_vec_add2(&tmp->world, &on_pnt->world);

	} else {
		float a,b,kn,kd;

		//compute clipping value k = (xs-zs) / (xs-xe-zs+ze)
		//use x or y as appropriate, and negate x/y value as appropriate

		if (plane_flag & (CC_OFF_RIGHT | CC_OFF_LEFT)) {
			a = on_pnt->world.xyz.x;
			b = off_pnt->world.xyz.x;
		}
		else {
			a = on_pnt->world.xyz.y;
			b = off_pnt->world.xyz.y;
		}

		if (plane_flag & (CC_OFF_LEFT | CC_OFF_BOT)) {
			a = -a;
			b = -b;
		}

		kn = a - on_pnt->world.xyz.z;						//xs-zs
		kd = kn - b + off_pnt->world.xyz.z;				//xs-zs-xe+ze

		ratio = kn / kd;

		tmp->world.xyz.x = on_pnt->world.xyz.x + (off_pnt->world.xyz.x-on_pnt->world.xyz.x) * ratio;
		tmp->world.xyz.y = on_pnt->world.xyz.y + (off_pnt->world.xyz.y-on_pnt->world.xyz.y) * ratio;

		if (plane_flag & (CC_OFF_TOP|CC_OFF_BOT))	{
			tmp->world.xyz.z = tmp->world.xyz.y;
		} else {
			tmp->world.xyz.z = tmp->world.xyz.x;
		}

		if (plane_flag & (CC_OFF_LEFT|CC_OFF_BOT))
			tmp->world.xyz.z = -tmp->world.xyz.z;

	}

	if (flags & TMAP_FLAG_TEXTURED) {
		tmp->texture_position.u = on_pnt->texture_position.u + (off_pnt->texture_position.u-on_pnt->texture_position.u) * ratio;
		tmp->texture_position.v = on_pnt->texture_position.v + (off_pnt->texture_position.v-on_pnt->texture_position.v) * ratio;
	}

	if (flags & TMAP_FLAG_GOURAUD ) {
		if (flags & TMAP_FLAG_RAMP) {

			float on_b, off_b;

			on_b = i2fl(on_pnt->b);
			off_b = i2fl(off_pnt->b);

			tmp->b = ubyte(fl2i(on_b + (off_b-on_b) * ratio));
		}
		if (flags & TMAP_FLAG_RGB) {
			float on_r, on_b, on_g, onspec_r, onspec_g, onspec_b;
			float off_r, off_b, off_g, offspec_r, offspec_g, offspec_b;

			on_r = i2fl(on_pnt->r);
			off_r = i2fl(off_pnt->r);

			on_g = i2fl(on_pnt->g);
			off_g = i2fl(off_pnt->g);

			on_b = i2fl(on_pnt->b);
			off_b = i2fl(off_pnt->b);


			onspec_r = i2fl(on_pnt->spec_r);
			offspec_r = i2fl(off_pnt->spec_r);

			onspec_g = i2fl(on_pnt->spec_g);
			offspec_g = i2fl(off_pnt->spec_g);

			onspec_b = i2fl(on_pnt->spec_b);
			offspec_b = i2fl(off_pnt->spec_b);


			tmp->r = ubyte(fl2i(on_r + (off_r-on_r) * ratio));
			tmp->g = ubyte(fl2i(on_g + (off_g-on_g) * ratio));
			tmp->b = ubyte(fl2i(on_b + (off_b-on_b) * ratio));

			tmp->spec_r = ubyte(fl2i(onspec_r + (offspec_r-onspec_r) * ratio));
			tmp->spec_g = ubyte(fl2i(onspec_g + (offspec_g-onspec_g) * ratio));
			tmp->spec_b = ubyte(fl2i(onspec_b + (offspec_b-onspec_b) * ratio));
		}
	}
	else
	{
		tmp->spec_r=tmp->spec_g=tmp->spec_b=0;
	}

	if (flags & TMAP_FLAG_ALPHA) {

		float on_a, off_a;

		on_a = i2fl(on_pnt->a);
		off_a = i2fl(off_pnt->a);

		tmp->a = ubyte(fl2i(on_a + (off_a-on_a) * ratio));
	}

	g3_code_vertex(tmp);

	return tmp;	
}


/**
 * @brief Clips a line to the viewing pyramid.
 */
void clip_line(vertex **p0,vertex **p1,ubyte codes_or, uint flags)
{
	int plane_flag;
	vertex *old_p1;

	for (plane_flag=1;plane_flag<=CC_OFF_USER;plane_flag<<=1)
		if (codes_or & plane_flag) {

			if ((*p0)->codes & plane_flag)
				{vertex *t=*p0; *p0=*p1; *p1=t;}	//swap!

			old_p1 = *p1;

			*p1 = clip_edge(plane_flag,*p0,*p1,flags);
			codes_or = (unsigned char)((*p0)->codes | (*p1)->codes);	//get new codes

			if (old_p1->flags & PF_TEMP_POINT)
				free_temp_point(old_p1);
		}

}

/**
 * @brief Clips a plane to the viewing pyramid.
 */
int clip_plane(int plane_flag,vertex **src,vertex **dest,int *nv,ccodes *cc,uint flags)
{
	int i;
	vertex **save_dest=dest;

	//copy first two verts to end
	src[*nv] = src[0];
	src[*nv+1] = src[1];

	cc->cc_and = 0xff;
	cc->cc_or = 0;

	for (i=1;i<=*nv;i++) {

		if (src[i]->codes & plane_flag) {				//cur point off?

			if (! (src[i-1]->codes & plane_flag)) {	//prev not off?

				*dest = clip_edge(plane_flag,src[i-1],src[i],flags);
				cc->cc_or  |= (*dest)->codes;
				cc->cc_and &= (*dest)->codes;
				dest++;
			}

			if (! (src[i+1]->codes & plane_flag)) {

				*dest = clip_edge(plane_flag,src[i+1],src[i],flags);
				cc->cc_or  |= (*dest)->codes;
				cc->cc_and &= (*dest)->codes;
				dest++;
			}

			//see if must free discarded point

			if (src[i]->flags & PF_TEMP_POINT)
				free_temp_point(src[i]);
		}
		else {			//cur not off, copy to dest buffer

			*dest++ = src[i];

			cc->cc_or  |= src[i]->codes;
			cc->cc_and &= src[i]->codes;
		}
	}

	return (dest-save_dest);
}

/**
 * @brief Clips a polygon to the viewing pyramid.
 */
vertex **clip_polygon(vertex **src,vertex **dest,int *nv,ccodes *cc,uint flags)
{
	int plane_flag;
	vertex **t;

	for (plane_flag=1;plane_flag<=CC_OFF_USER;plane_flag<<=1)

		if (cc->cc_or & plane_flag) {

			*nv = clip_plane(plane_flag,src,dest,nv,cc,flags);

			if (cc->cc_and)		//clipped away
				return dest;

			t = src; src = dest; dest = t;

		}

	return src;		//we swapped after we copied
}
