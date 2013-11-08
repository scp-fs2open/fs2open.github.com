/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#include "graphics/2d.h"
#include "render/3dinternal.h"
#include "globalincs/systemvars.h"
#include "io/key.h"
#include "cmdline/cmdline.h"



int Lasers = 1;
DCF_BOOL( lasers, Lasers )

float g3_draw_laser_htl(vec3d *p0,float width1,vec3d *p1,float width2, int r, int g, int b, uint tmap_flags)
{
	width1 *= 0.5f;
	width2 *= 0.5f;
	vec3d uvec, fvec, rvec, center, reye, rfvec;

	vm_vec_sub( &fvec, p0, p1 );
	vm_vec_normalize_safe( &fvec );
	vm_vec_copy_scale(&rfvec, &fvec, -1.0f);

	vm_vec_avg( &center, p0, p1 ); //needed for the return value only
	vm_vec_sub(&reye, &Eye_position, &center);
	vm_vec_normalize(&reye);

	// code intended to prevent possible null vector normalize issue - start
	if (vm_test_parallel(&reye,&fvec)){
		fvec.xyz.x = -reye.xyz.z;
		fvec.xyz.y = 0.0f;
		fvec.xyz.z = -reye.xyz.x;
	}

	if (vm_test_parallel(&reye,&rfvec)){
		fvec.xyz.x = reye.xyz.z;
		fvec.xyz.y = 0.0f;
		fvec.xyz.z = reye.xyz.x;
	}
	// code intended to prevent possible null vector normalize issue - end

	vm_vec_crossprod(&uvec,&fvec,&reye);
	vm_vec_normalize(&uvec);
	vm_vec_crossprod(&fvec,&uvec,&reye);
	vm_vec_normalize(&fvec);
	 
	//now recompute right vector, in case it wasn't entirely perpendiclar
	vm_vec_crossprod(&rvec,&uvec,&fvec);

	// Now have uvec, which is up vector and rvec which is the normal
	// of the face.

	int i;
	vec3d start, end;
	vm_vec_scale_add(&start, p0, &fvec, -width1);
	vm_vec_scale_add(&end, p1, &fvec, width2);
	vec3d vecs[4];
	vertex pts[4];
	vertex *ptlist[8] = 
	{ &pts[3], &pts[2], &pts[1], &pts[0], 
	  &pts[0], &pts[1], &pts[2], &pts[3]};

	vm_vec_scale_add( &vecs[0], &start, &uvec, width1 );
	vm_vec_scale_add( &vecs[1], &end, &uvec, width2 );
	vm_vec_scale_add( &vecs[2], &end, &uvec, -width2 );
	vm_vec_scale_add( &vecs[3], &start, &uvec, -width1 );

	for (i=0; i<4; i++ )	{
		g3_transfer_vertex( &pts[i], &vecs[i] );
	}
	ptlist[0]->texture_position.u = 0.0f;
	ptlist[0]->texture_position.v = 0.0f;
	ptlist[1]->texture_position.u = 1.0f;
	ptlist[1]->texture_position.v = 0.0f;
	ptlist[2]->texture_position.u = 1.0f;
	ptlist[2]->texture_position.v = 1.0f;
	ptlist[3]->texture_position.u = 0.0f;
	ptlist[3]->texture_position.v = 1.0f;
	ptlist[0]->r = (ubyte)r;
	ptlist[0]->g = (ubyte)g;
	ptlist[0]->b = (ubyte)b;
	ptlist[0]->a = 255;
	ptlist[1]->r = (ubyte)r;
	ptlist[1]->g = (ubyte)g;
	ptlist[1]->b = (ubyte)b;
	ptlist[1]->a = 255;
	ptlist[2]->r = (ubyte)r;
	ptlist[2]->g = (ubyte)g;
	ptlist[2]->b = (ubyte)b;
	ptlist[2]->a = 255;
	ptlist[3]->r = (ubyte)r;
	ptlist[3]->g = (ubyte)g;
	ptlist[3]->b = (ubyte)b;
	ptlist[3]->a = 255;

	gr_tmapper(4, ptlist,tmap_flags | TMAP_FLAG_RGB | TMAP_FLAG_GOURAUD | TMAP_FLAG_CORRECT);
	
	return center.xyz.z;
}


// This assumes you have already set a color with gr_set_color or gr_set_color_fast
// and a bitmap with gr_set_bitmap.  If it is very far away, it draws the laser
// as flat-shaded using current color, else textured using current texture.
// If max_len is > 1.0, then this caps the length to be no longer than max_len pixels.
float g3_draw_laser(vec3d *headp, float head_width, vec3d *tailp, float tail_width, uint tmap_flags, float max_len )
{
	if (!Lasers) {
		return 0.0f;
	}

	if ( !Cmdline_nohtl && (tmap_flags & TMAP_HTL_3D_UNLIT) ) {
		return g3_draw_laser_htl(headp, head_width, tailp, tail_width, 255,255,255, tmap_flags | TMAP_HTL_3D_UNLIT);
	}

	float headx, heady, headr, tailx, taily, tailr;
	vertex pt1, pt2;
	float depth;

	Assert( G3_count == 1 );

	g3_rotate_vertex(&pt1,headp);

	g3_project_vertex(&pt1);
	if (pt1.flags & PF_OVERFLOW) 
		return 0.0f;

	g3_rotate_vertex(&pt2,tailp);

	g3_project_vertex(&pt2);
	if (pt2.flags & PF_OVERFLOW) 
		return 0.0f;

	if ( (pt1.codes & pt2.codes) != 0 )	{
		// Both off the same side
		return 0.0f;
	}

	headx = pt1.screen.xyw.x;
	heady = pt1.screen.xyw.y;
	headr = (head_width*Matrix_scale.xyz.x*Canv_w2*pt1.screen.xyw.w);

	tailx = pt2.screen.xyw.x;
	taily = pt2.screen.xyw.y;
	tailr = (tail_width*Matrix_scale.xyz.x*Canv_w2*pt2.screen.xyw.w);

	float len_2d = fl_sqrt( (tailx-headx)*(tailx-headx) + (taily-heady)*(taily-heady) );

	// Cap the length if needed.
	if ( (max_len > 1.0f) && (len_2d > max_len) )	{
		float ratio = max_len / len_2d;
	
		tailx = headx + ( tailx - headx ) * ratio;
		taily = heady + ( taily - heady ) * ratio;
		tailr = headr + ( tailr - headr ) * ratio;

		len_2d = fl_sqrt( (tailx-headx)*(tailx-headx) + (taily-heady)*(taily-heady) );
	}

	depth = (pt1.world.xyz.z+pt2.world.xyz.z)*0.5f;

	float max_r  = headr;
	float a;
	if ( tailr > max_r ) 
		max_r = tailr;

	if ( max_r < 1.0f )
		max_r = 1.0f;

	float mx, my, w, h1,h2;

	if ( len_2d < max_r ) {

		h1 = headr + (max_r-len_2d);
		if ( h1 > max_r ) h1 = max_r;
		h2 = tailr + (max_r-len_2d);
		if ( h2 > max_r ) h2 = max_r;

		len_2d = max_r;
		if ( fl_abs(tailx - headx) > 0.01f )	{
			a = (float)atan2( taily-heady, tailx-headx );
		} else {
			a = 0.0f;
		}

		w = len_2d;

	} else {
		a = atan2_safe( taily-heady, tailx-headx );

		w = len_2d;

		h1 = headr;
		h2 = tailr;
	}
	
	mx = (tailx+headx)/2.0f;
	my = (taily+heady)/2.0f;

	// Draw box with width 'w' and height 'h' at angle 'a' from horizontal
	// centered around mx, my

	if ( h1 < 1.0f ) h1 = 1.0f;
	if ( h2 < 1.0f ) h2 = 1.0f;

	float sa, ca;

	sa = (float)sin(a);
	ca = (float)cos(a);

	vertex v[4];
	vertex *vertlist[4] = { &v[3], &v[2], &v[1], &v[0] };
	memset(v,0,sizeof(vertex)*4);

	if ( depth < 0.0f ) depth = 0.0f;
	
	v[0].screen.xyw.x = (-w/2.0f)*ca + (-h1/2.0f)*sa + mx;
	v[0].screen.xyw.y = (-w/2.0f)*sa - (-h1/2.0f)*ca + my;
	v[0].world.xyz.z = pt1.world.xyz.z;
	v[0].screen.xyw.w = pt1.screen.xyw.w;
	v[0].texture_position.u = 0.0f;
	v[0].texture_position.v = 0.0f;
	v[0].b = 191;

	v[1].screen.xyw.x = (w/2.0f)*ca + (-h2/2.0f)*sa + mx;
	v[1].screen.xyw.y = (w/2.0f)*sa - (-h2/2.0f)*ca + my;
	v[1].world.xyz.z = pt2.world.xyz.z;
	v[1].screen.xyw.w = pt2.screen.xyw.w;
	v[1].texture_position.u = 1.0f;
	v[1].texture_position.v = 0.0f;
	v[1].b = 191;

	v[2].screen.xyw.x = (w/2.0f)*ca + (h2/2.0f)*sa + mx;
	v[2].screen.xyw.y = (w/2.0f)*sa - (h2/2.0f)*ca + my;
	v[2].world.xyz.z = pt2.world.xyz.z;
	v[2].screen.xyw.w = pt2.screen.xyw.w;
	v[2].texture_position.u = 1.0f;
	v[2].texture_position.v = 1.0f;
	v[2].b = 191;

	v[3].screen.xyw.x = (-w/2.0f)*ca + (h1/2.0f)*sa + mx;
	v[3].screen.xyw.y = (-w/2.0f)*sa - (h1/2.0f)*ca + my;
	v[3].world.xyz.z = pt1.world.xyz.z;
	v[3].screen.xyw.w = pt1.screen.xyw.w;
	v[3].texture_position.u = 0.0f;
	v[3].texture_position.v = 1.0f;
	v[3].b = 191;

	gr_tmapper(4, vertlist, tmap_flags | TMAP_FLAG_CORRECT);	

	return depth;
}


// Draw a laser shaped 3d looking thing using vertex coloring (useful for things like colored laser glows)
// If max_len is > 1.0, then this caps the length to be no longer than max_len pixels
float g3_draw_laser_rgb(vec3d *headp, float head_width, vec3d *tailp, float tail_width, int r, int g, int b, uint tmap_flags, float max_len )
{
	if (!Lasers){
		return 0.0f;
	}
	if((!Cmdline_nohtl)  && tmap_flags & TMAP_HTL_3D_UNLIT){
		return g3_draw_laser_htl(headp,head_width,tailp,tail_width,r,g,b,tmap_flags | TMAP_HTL_3D_UNLIT);
	}
	float headx, heady, headr, tailx, taily, tailr;
	vertex pt1, pt2;
	float depth;

	Assert( G3_count == 1 );

	g3_rotate_vertex(&pt1,headp);

	g3_project_vertex(&pt1);
	if (pt1.flags & PF_OVERFLOW) 
		return 0.0f;

	g3_rotate_vertex(&pt2,tailp);

	g3_project_vertex(&pt2);
	if (pt2.flags & PF_OVERFLOW) 
		return 0.0f;

	if ( (pt1.codes & pt2.codes) != 0 )	{
		// Both off the same side
		return 0.0f;
	}

	headx = pt1.screen.xyw.x;
	heady = pt1.screen.xyw.y;
	headr = (head_width*Matrix_scale.xyz.x*Canv_w2*pt1.screen.xyw.w);

	tailx = pt2.screen.xyw.x;
	taily = pt2.screen.xyw.y;
	tailr = (tail_width*Matrix_scale.xyz.x*Canv_w2*pt2.screen.xyw.w);

	float len_2d = fl_sqrt( (tailx-headx)*(tailx-headx) + (taily-heady)*(taily-heady) );

	// Cap the length if needed.
	if ( (max_len > 1.0f) && (len_2d > max_len) )	{
		float ratio = max_len / len_2d;
	
		tailx = headx + ( tailx - headx ) * ratio;
		taily = heady + ( taily - heady ) * ratio;
		tailr = headr + ( tailr - headr ) * ratio;

		len_2d = fl_sqrt( (tailx-headx)*(tailx-headx) + (taily-heady)*(taily-heady) );
	}

	depth = (pt1.world.xyz.z+pt2.world.xyz.z)*0.5f;

	float max_r  = headr;
	float a;
	if ( tailr > max_r ) 
		max_r = tailr;

	if ( max_r < 1.0f )
		max_r = 1.0f;

	float mx, my, w, h1,h2;

	if ( len_2d < max_r ) {

		h1 = headr + (max_r-len_2d);
		if ( h1 > max_r ) h1 = max_r;
		h2 = tailr + (max_r-len_2d);
		if ( h2 > max_r ) h2 = max_r;

		len_2d = max_r;
		if ( fl_abs(tailx - headx) > 0.01f )	{
			a = (float)atan2( taily-heady, tailx-headx );
		} else {
			a = 0.0f;
		}

		w = len_2d;

	} else {
		a = atan2_safe( taily-heady, tailx-headx );

		w = len_2d;

		h1 = headr;
		h2 = tailr;
	}
	
	mx = (tailx+headx)/2.0f;
	my = (taily+heady)/2.0f;

	// Draw box with width 'w' and height 'h' at angle 'a' from horizontal
	// centered around mx, my

	if ( h1 < 1.0f ) h1 = 1.0f;
	if ( h2 < 1.0f ) h2 = 1.0f;

	float sa, ca;

	sa = (float)sin(a);
	ca = (float)cos(a);

	vertex v[4];
	vertex *vertlist[4] = { &v[3], &v[2], &v[1], &v[0] };
	memset(v,0,sizeof(vertex)*4);

	if ( depth < 0.0f ) depth = 0.0f;
	
	v[0].screen.xyw.x = (-w/2.0f)*ca + (-h1/2.0f)*sa + mx;
	v[0].screen.xyw.y = (-w/2.0f)*sa - (-h1/2.0f)*ca + my;
	v[0].world.xyz.z = pt1.world.xyz.z;
	v[0].screen.xyw.w = pt1.screen.xyw.w;
	v[0].texture_position.u = 0.0f;
	v[0].texture_position.v = 0.0f;
	v[0].r = (ubyte)r;
	v[0].g = (ubyte)g;
	v[0].b = (ubyte)b;
	v[0].a = 255;

	v[1].screen.xyw.x = (w/2.0f)*ca + (-h2/2.0f)*sa + mx;
	v[1].screen.xyw.y = (w/2.0f)*sa - (-h2/2.0f)*ca + my;
	v[1].world.xyz.z = pt2.world.xyz.z;
	v[1].screen.xyw.w = pt2.screen.xyw.w;
	v[1].texture_position.u = 1.0f;
	v[1].texture_position.v = 0.0f;
	v[1].r = (ubyte)r;
	v[1].g = (ubyte)g;
	v[1].b = (ubyte)b;
	v[1].a = 255;

	v[2].screen.xyw.x = (w/2.0f)*ca + (h2/2.0f)*sa + mx;
	v[2].screen.xyw.y = (w/2.0f)*sa - (h2/2.0f)*ca + my;
	v[2].world.xyz.z = pt2.world.xyz.z;
	v[2].screen.xyw.w = pt2.screen.xyw.w;
	v[2].texture_position.u = 1.0f;
	v[2].texture_position.v = 1.0f;
	v[2].r = (ubyte)r;
	v[2].g = (ubyte)g;
	v[2].b = (ubyte)b;
	v[2].a = 255;

	v[3].screen.xyw.x = (-w/2.0f)*ca + (h1/2.0f)*sa + mx;
	v[3].screen.xyw.y = (-w/2.0f)*sa - (h1/2.0f)*ca + my;
	v[3].world.xyz.z = pt1.world.xyz.z;
	v[3].screen.xyw.w = pt1.screen.xyw.w;
	v[3].texture_position.u = 0.0f;
	v[3].texture_position.v = 1.0f;
	v[3].r = (ubyte)r;
	v[3].g = (ubyte)g;
	v[3].b = (ubyte)b;
	v[3].a = 255;
	
	gr_tmapper(4, vertlist, tmap_flags | TMAP_FLAG_RGB | TMAP_FLAG_GOURAUD | TMAP_FLAG_CORRECT);

	return depth;
}
