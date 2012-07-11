/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#include "globalincs/pstypes.h"
#include "cmdline/cmdline.h"
#include "osapi/osapi.h"
#include "graphics/2d.h"
#include "render/3d.h"
#include "bmpman/bmpman.h"
#include "math/floating.h"
#include "palman/palman.h"
#include "globalincs/systemvars.h"
#include "graphics/grinternal.h"
#include "graphics/gropengl.h"
#include "graphics/line.h"
#include "graphics/gropenglbmpman.h"
#include "graphics/gropengllight.h"
#include "graphics/gropengltexture.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropenglstate.h"
#include "graphics/gropengltnl.h"
#include "graphics/gropenglbmpman.h"
#include "graphics/gropengldraw.h"
#include "debugconsole/timerbar.h"
#include "nebula/neb.h"
#include "graphics/gropenglshader.h"
#include "graphics/gropenglpostprocessing.h"
#include "freespace2/freespace.h"

GLuint Scene_framebuffer;
GLuint Scene_color_texture;
GLuint Scene_luminance_texture;
GLuint Scene_effect_texture;
GLuint Scene_depth_texture;
GLuint Cockpit_depth_texture;

GLuint Distortion_framebuffer;
GLuint Distortion_texture[2];
int Distortion_switch = 0;

int Scene_texture_initialized;
bool Scene_framebuffer_in_frame;

int Scene_texture_width;
int Scene_texture_height;

GLfloat Scene_texture_u_scale = 1.0f;
GLfloat Scene_texture_v_scale = 1.0f;

void gr_opengl_pixel(int x, int y, bool resize)
{
	gr_line(x, y, x, y, resize);
}

void opengl_aabitmap_ex_internal(int x, int y, int w, int h, int sx, int sy, bool resize, bool mirror)
{
	if ( (w < 1) || (h < 1) ) {
		return;
	}

	if ( !gr_screen.current_color.is_alphacolor ) {
		return;
	}

	float u_scale, v_scale;

	GL_CHECK_FOR_ERRORS("start of aabitmap_ex_internal()");

	GL_state.SetTextureSource(TEXTURE_SOURCE_NO_FILTERING);
	GL_state.SetAlphaBlendMode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	GL_state.SetZbufferType(ZBUFFER_TYPE_NONE);

	if ( !gr_opengl_tcache_set(gr_screen.current_bitmap, TCACHE_TYPE_AABITMAP, &u_scale, &v_scale) ) {
		mprintf(("WARNING: Error setting aabitmap texture (%i)!\n", gr_screen.current_bitmap));
		return;
	}

	float u0, u1, v0, v1;
	float x1, x2, y1, y2;
	int bw, bh, do_resize;

	if ( resize && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	bm_get_info(gr_screen.current_bitmap, &bw, &bh);

	u0 = u_scale * (i2fl(sx) / i2fl(bw));
	v0 = v_scale * (i2fl(sy) / i2fl(bh));

	u1 = u_scale * (i2fl(sx+w) / i2fl(bw));
	v1 = v_scale * (i2fl(sy+h) / i2fl(bh));

	x1 = i2fl(x + ((do_resize) ? gr_screen.offset_x_unscaled : gr_screen.offset_x));
	y1 = i2fl(y + ((do_resize) ? gr_screen.offset_y_unscaled : gr_screen.offset_y));
	x2 = x1 + i2fl(w);
	y2 = y1 + i2fl(h);

	if (do_resize) {
		gr_resize_screen_posf(&x1, &y1);
		gr_resize_screen_posf(&x2, &y2);
	}

	glColor4ub(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);

	if (mirror) {
		float temp = u0;
		u0 = u1;
		u1 = temp;
	}

	GLboolean cull_face = GL_state.CullFace(GL_FALSE);

	opengl_draw_textured_quad(x1,y1,u0,v0, x2,y2,u1,v1);

	GL_state.CullFace(cull_face);

	GL_CHECK_FOR_ERRORS("end of aabitmap_ex_internal()");
}

void gr_opengl_aabitmap_ex(int x, int y, int w, int h, int sx, int sy, bool resize, bool mirror)
{
	int reclip;
#ifndef NDEBUG
	int count = 0;
#endif

	int dx1 = x;
	int dx2 = x + w - 1;
	int dy1 = y;
	int dy2 = y + h - 1;

	int bw, bh, do_resize;

	bm_get_info(gr_screen.current_bitmap, &bw, &bh);

	if ( resize && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	int clip_left = ((do_resize) ? gr_screen.clip_left_unscaled : gr_screen.clip_left);
	int clip_right = ((do_resize) ? gr_screen.clip_right_unscaled : gr_screen.clip_right);
	int clip_top = ((do_resize) ? gr_screen.clip_top_unscaled : gr_screen.clip_top);
	int clip_bottom = ((do_resize) ? gr_screen.clip_bottom_unscaled : gr_screen.clip_bottom);

	do {
		reclip = 0;

#ifndef NDEBUG
		if (count > 1) {
			Int3();
		}

		count++;
#endif
	
		if ( (dx1 > clip_right) || (dx2 < clip_left) ) {
			return;
		}

		if ( (dy1 > clip_bottom) || (dy2 < clip_top) ) {
			return;
		}

		if (dx1 < clip_left) {
			sx += clip_left - dx1;
			dx1 = clip_left;
		}

		if (dy1 < clip_top) {
			sy += clip_top - dy1;
			dy1 = clip_top;
		}

		if (dx2 > clip_right) {
			dx2 = clip_right;
		}

		if (dy2 > clip_bottom) {
			dy2 = clip_bottom;
		}


		if ( sx < 0 ) {
			dx1 -= sx;
			sx = 0;
			reclip = 1;
		}

		if ( sy < 0 ) {
			dy1 -= sy;
			sy = 0;
			reclip = 1;
		}

		w = dx2 - dx1 + 1;
		h = dy2 - dy1 + 1;

		if ( sx + w > bw ) {
			w = bw - sx;
			dx2 = dx1 + w - 1;
		}

		if ( sy + h > bh ) {
			h = bh - sy;
			dy2 = dy1 + h - 1;
		}

		if ( (w < 1) || (h < 1) ) {
			// clipped away!
			return;
		}
	} while (reclip);

	// Make sure clipping algorithm works
#ifndef NDEBUG
	Assert( w > 0 );
	Assert( h > 0 );
	Assert( w == (dx2-dx1+1) );
	Assert( h == (dy2-dy1+1) );
	Assert( sx >= 0 );
	Assert( sy >= 0 );
	Assert( sx+w <= bw );
	Assert( sy+h <= bh );
	Assert( dx2 >= dx1 );
	Assert( dy2 >= dy1 );
	Assert( (dx1 >= clip_left) && (dx1 <= clip_right) );
	Assert( (dx2 >= clip_left) && (dx2 <= clip_right) );
	Assert( (dy1 >= clip_top) && (dy1 <= clip_bottom) );
	Assert( (dy2 >= clip_top) && (dy2 <= clip_bottom) );
#endif

	// We now have dx1,dy1 and dx2,dy2 and sx, sy all set validly within clip regions.
	opengl_aabitmap_ex_internal(dx1, dy1, (dx2 - dx1 + 1), (dy2 - dy1 + 1), sx, sy, resize, mirror);
}

void gr_opengl_aabitmap(int x, int y, bool resize, bool mirror)
{
	int w, h, do_resize;

	bm_get_info(gr_screen.current_bitmap, &w, &h);

	if ( resize && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	int dx1 = x;
	int dx2 = x + w - 1;
	int dy1 = y;
	int dy2 = y + h - 1;
	int sx = 0, sy = 0;

	int clip_left = ((do_resize) ? gr_screen.clip_left_unscaled : gr_screen.clip_left);
	int clip_right = ((do_resize) ? gr_screen.clip_right_unscaled : gr_screen.clip_right);
	int clip_top = ((do_resize) ? gr_screen.clip_top_unscaled : gr_screen.clip_top);
	int clip_bottom = ((do_resize) ? gr_screen.clip_bottom_unscaled : gr_screen.clip_bottom);

	if ( (dx1 > clip_right) || (dx2 < clip_left) ) {
		return;
	}

	if ( (dy1 > clip_bottom) || (dy2 < clip_top) ) {
		return;
	}

	if (dx1 < clip_left) {
		sx = clip_left - dx1;
		dx1 = clip_left;
	}

	if (dy1 < clip_top) {
		sy = clip_top - dy1;
		dy1 = clip_top;
	}

	if (dx2 > clip_right) {
		dx2 = clip_right;
	}

	if (dy2 > clip_bottom) {
		dy2 = clip_bottom;
	}

	if ( (sx < 0) || (sy < 0) ) {
		return;
	}

	if ( (sx >= w) || (sy >= h) ) {
		return;
	}

	// Draw bitmap bm[sx,sy] into (dx1,dy1)-(dx2,dy2)
	opengl_aabitmap_ex_internal(dx1, dy1, (dx2 - dx1 + 1), (dy2 - dy1 + 1), sx, sy, resize, mirror);
}

struct v4 { GLfloat x,y,u,v; };

void gr_opengl_string(int sx, int sy, const char *s, bool resize)
{
	int width, spacing, letter;
	int x, y, do_resize;
	float bw, bh;
	float u0, u1, v0, v1;
	int x1, x2, y1, y2;
	float u_scale, v_scale;

	// conversion from quads to triangles requires six vertices per quad
	struct v4 *glVert = (struct v4*) alloca(sizeof(struct v4) * strlen(s) * 6);
	int curChar = 0;

	if ( !Current_font || (*s == 0) ) {
		return;
	}

	GL_CHECK_FOR_ERRORS("start of string()");

	gr_set_bitmap(Current_font->bitmap_id);

	GL_state.SetTextureSource(TEXTURE_SOURCE_NO_FILTERING);
	GL_state.SetAlphaBlendMode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	GL_state.SetZbufferType(ZBUFFER_TYPE_NONE);

	if ( !gr_opengl_tcache_set(gr_screen.current_bitmap, TCACHE_TYPE_AABITMAP, &u_scale, &v_scale) ) {
		return;
	}

	int ibw, ibh;

	bm_get_info(gr_screen.current_bitmap, &ibw, &ibh);

	bw = i2fl(ibw);
	bh = i2fl(ibh);

	// set color!
	if (gr_screen.current_color.is_alphacolor) {
		glColor4ub(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);
	} else {
		glColor3ub(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue);
	}

//	if ( (gr_screen.custom_size && resize) || (gr_screen.rendering_to_texture != -1) ) {
	if ( resize && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	int clip_left = ((do_resize) ? gr_screen.clip_left_unscaled : gr_screen.clip_left);
	int clip_right = ((do_resize) ? gr_screen.clip_right_unscaled : gr_screen.clip_right);
	int clip_top = ((do_resize) ? gr_screen.clip_top_unscaled : gr_screen.clip_top);
	int clip_bottom = ((do_resize) ? gr_screen.clip_bottom_unscaled : gr_screen.clip_bottom);

	x = sx;
	y = sy;

	if (sx == 0x8000) {
		// centered
		x = get_centered_x(s);
	} else {
		x = sx;
	}

	spacing = 0;

	GLboolean cull_face = GL_state.CullFace(GL_FALSE);

	// pick out letter coords, draw it, goto next letter and do the same
	while (*s)	{
		x += spacing;

		while (*s == '\n')	{
			s++;
			y += Current_font->h;

			if (sx == 0x8000) {
				// centered
				x = get_centered_x(s);
			} else {
				x = sx;
			}
		}

		if (*s == 0) {
			break;
		}

		letter = get_char_width(s[0], s[1], &width, &spacing);
		s++;

		//not in font, draw as space
		if (letter < 0) {
			continue;
		}

		int xd, yd, xc, yc;
		int wc, hc;

		// Check if this character is totally clipped
		if ( (x + width) < clip_left ) {
			continue;
		}

		if ( (y + Current_font->h) < clip_top ) {
			continue;
		}

		if (x > clip_right) {
			continue;
		}

		if (y > clip_bottom) {
			continue;
		}

		xd = yd = 0;

		if (x < clip_left) {
			xd = clip_left - x;
		}

		if (y < clip_top) {
			yd = clip_top - y;
		}

		xc = x + xd;
		yc = y + yd;

		wc = width - xd;
		hc = Current_font->h - yd;

		if ( (xc + wc) > clip_right ) {
			wc = clip_right - xc;
		}

		if ( (yc + hc) > clip_bottom ) {
			hc = clip_bottom - yc;
		}

		if ( (wc < 1) || (hc < 1) ) {
			continue;
		}

		int u = Current_font->bm_u[letter];
		int v = Current_font->bm_v[letter];

		x1 = xc + ((do_resize) ? gr_screen.offset_x_unscaled : gr_screen.offset_x);
		y1 = yc + ((do_resize) ? gr_screen.offset_y_unscaled : gr_screen.offset_y);
		x2 = x1 + wc;
		y2 = y1 + hc;

		if (do_resize) {
			gr_resize_screen_pos( &x1, &y1 );
			gr_resize_screen_pos( &x2, &y2 );
		}

		u0 = u_scale * (i2fl(u+xd) / bw);
		v0 = v_scale * (i2fl(v+yd) / bh);

		u1 = u_scale * (i2fl((u+xd)+wc) / bw);
		v1 = v_scale * (i2fl((v+yd)+hc) / bh);

		glVert[curChar*6 + 0].x = (GLfloat)x1;
		glVert[curChar*6 + 0].y = (GLfloat)y2;
		glVert[curChar*6 + 0].u = u0;
		glVert[curChar*6 + 0].v = v1;

		glVert[curChar*6 + 1].x = (GLfloat)x2;
		glVert[curChar*6 + 1].y = (GLfloat)y2;
		glVert[curChar*6 + 1].u = u1;
		glVert[curChar*6 + 1].v = v1;

		glVert[curChar*6 + 2].x = (GLfloat)x1;
		glVert[curChar*6 + 2].y = (GLfloat)y1;
		glVert[curChar*6 + 2].u = u0;
		glVert[curChar*6 + 2].v = v0;

		glVert[curChar*6 + 3] = glVert[curChar*6 + 1];
		glVert[curChar*6 + 4] = glVert[curChar*6 + 2];

		glVert[curChar*6 + 5].x = (GLfloat)x2;
		glVert[curChar*6 + 5].y = (GLfloat)y1;
		glVert[curChar*6 + 5].u = u1;
		glVert[curChar*6 + 5].v = v0;

		curChar++;
	}

	glVertexPointer(2, GL_FLOAT, sizeof(struct v4), &glVert[0].x);
	glTexCoordPointer(2, GL_FLOAT, sizeof(struct v4), &glVert[0].u);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glDrawArrays(GL_TRIANGLES, 0, curChar * 6);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	GL_state.CullFace(cull_face);

	GL_CHECK_FOR_ERRORS("end of string()");
}

void gr_opengl_line(int x1,int y1,int x2,int y2, bool resize)
{
	int do_resize, clipped = 0, swapped = 0;
	float sx1, sy1;
	float sx2, sy2;

	if ( resize && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	int clip_left = ((do_resize) ? gr_screen.clip_left_unscaled : gr_screen.clip_left);
	int clip_right = ((do_resize) ? gr_screen.clip_right_unscaled : gr_screen.clip_right);
	int clip_top = ((do_resize) ? gr_screen.clip_top_unscaled : gr_screen.clip_top);
	int clip_bottom = ((do_resize) ? gr_screen.clip_bottom_unscaled : gr_screen.clip_bottom);
	int offset_x = ((do_resize) ? gr_screen.offset_x_unscaled : gr_screen.offset_x);
	int offset_y = ((do_resize) ? gr_screen.offset_y_unscaled : gr_screen.offset_y);


	INT_CLIPLINE(x1, y1, x2, y2, clip_left, clip_top, clip_right, clip_bottom, return, clipped = 1, swapped = 1);

	sx1 = i2fl(x1 + offset_x);
	sy1 = i2fl(y1 + offset_y);
	sx2 = i2fl(x2 + offset_x);
	sy2 = i2fl(y2 + offset_y);


	if (do_resize) {
		gr_resize_screen_posf(&sx1, &sy1);
		gr_resize_screen_posf(&sx2, &sy2);
	}

	GL_state.SetTextureSource(TEXTURE_SOURCE_NONE);
	GL_state.SetAlphaBlendMode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	GL_state.SetZbufferType(ZBUFFER_TYPE_NONE);

	if ( (x1 == x2) && (y1 == y2) ) {
		gr_opengl_set_2d_matrix();
		
		GLfloat vert[3]= {sx1, sy1, -0.99f};
		glColor4ub(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);

		glVertexPointer(3, GL_FLOAT, 0, vert);
		glEnableClientState(GL_VERTEX_ARRAY);
		glDrawArrays(GL_POINTS, 0, 1);
		glDisableClientState(GL_VERTEX_ARRAY);

		GL_CHECK_FOR_ERRORS("end of opengl_line()");
		
		gr_opengl_end_2d_matrix();

		return;
	}

	if (x1 == x2) {
		if (sy1 < sy2) {
			sy2 += 0.5f;
		} else {
			sy1 += 0.5f;
		}
	} else if (y1 == y2) {
		if (sx1 < sx2) {
			sx2 += 0.5f;
		} else {
			sx1 += 0.5f;
		}
	}

	gr_opengl_set_2d_matrix();

	GLfloat line[6] = {
		sx2, sy2, -0.99f,
		sx1, sy1, -0.99f
	};

	glColor4ub(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);

	glVertexPointer(3, GL_FLOAT, 0, line);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_LINES, 0, 2);
	glDisableClientState(GL_VERTEX_ARRAY);

	GL_CHECK_FOR_ERRORS("end of opengl_line()");

	gr_opengl_end_2d_matrix();
}

void gr_opengl_line_htl(vec3d *start, vec3d *end)
{
	if (Cmdline_nohtl) {
		return;
	}

	gr_zbuffer_type zbuffer_state = (gr_zbuffering) ? ZBUFFER_TYPE_FULL : ZBUFFER_TYPE_NONE;
	GL_state.SetTextureSource(TEXTURE_SOURCE_NONE);
	GL_state.SetZbufferType(zbuffer_state);


    if (gr_screen.current_color.is_alphacolor) {
        GL_state.SetAlphaBlendMode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
		glColor4ub( gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha );
	} else {
        GL_state.SetAlphaBlendMode(ALPHA_BLEND_NONE);
		glColor3ub( gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue );
    }

	GLfloat line[6] = {
		start->xyz.x,	start->xyz.y,	start->xyz.z,
		end->xyz.x,		end->xyz.y,		end->xyz.z
	};

	glVertexPointer(3, GL_FLOAT, 0, line);

	glEnableClientState(GL_VERTEX_ARRAY);

	glDrawArrays(GL_LINES, 0, 2);

	glDisableClientState(GL_VERTEX_ARRAY);

	GL_CHECK_FOR_ERRORS("end of opengl_line_htl()");
}

void gr_opengl_aaline(vertex *v1, vertex *v2)
{
// -- AA OpenGL lines.  Looks good but they are kinda slow so this is disabled until an option is implemented - taylor
//	gr_opengl_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );
//	glEnable( GL_LINE_SMOOTH );
//	glHint( GL_LINE_SMOOTH_HINT, GL_FASTEST );
//	glLineWidth( 1.0 );

	int clipped = 0, swapped = 0;
	float x1 = v1->screen.xyw.x;
	float y1 = v1->screen.xyw.y;
	float x2 = v2->screen.xyw.x;
	float y2 = v2->screen.xyw.y;
	float sx1, sy1;
	float sx2, sy2;


	FL_CLIPLINE(x1, y1, x2, y2, (float)gr_screen.clip_left, (float)gr_screen.clip_top, (float)gr_screen.clip_right, (float)gr_screen.clip_bottom, return, clipped = 1, swapped = 1);

	sx1 = x1 + (float)gr_screen.offset_x;
	sy1 = y1 + (float)gr_screen.offset_y;
	sx2 = x2 + (float)gr_screen.offset_x;
	sy2 = y2 + (float)gr_screen.offset_y;

	GL_state.SetTextureSource(TEXTURE_SOURCE_NONE);
	GL_state.SetAlphaBlendMode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	GL_state.SetZbufferType(ZBUFFER_TYPE_NONE);

	if ( (x1 == x2) && (y1 == y2) ) {
		gr_opengl_set_2d_matrix();

		glColor4ub(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);

		GLfloat vert[3]= {sx1, sy1, -0.99f};

		glVertexPointer(3, GL_FLOAT, 0, vert);
		glEnableClientState(GL_VERTEX_ARRAY);
		glDrawArrays(GL_POINTS, 0, 1);
		glDisableClientState(GL_VERTEX_ARRAY);

		GL_CHECK_FOR_ERRORS("end of opengl_aaline()");

		gr_opengl_end_2d_matrix();

		return;
	}

	if (x1 == x2) {
		if (sy1 < sy2) {
			sy2 += 0.5f;
		} else {
			sy1 += 0.5f;
		}
	} else if (y1 == y2) {
		if (sx1 < sx2) {
			sx2 += 0.5f;
		} else {
			sx1 += 0.5f;
		}
	}

	gr_opengl_set_2d_matrix();

	glColor4ub(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);
	GLfloat line[6] = {
		sx2, sy2, -0.99f,
		sx1, sy1, -0.99f
	};

	glVertexPointer(3, GL_FLOAT, 0, line);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_LINES, 0, 2);
	glDisableClientState(GL_VERTEX_ARRAY);

	GL_CHECK_FOR_ERRORS("end of opengl_aaline()");

	gr_opengl_end_2d_matrix();

//	glDisable( GL_LINE_SMOOTH );
}

void gr_opengl_gradient(int x1, int y1, int x2, int y2, bool resize)
{
	int clipped = 0, swapped = 0;

	if ( !gr_screen.current_color.is_alphacolor ) {
		gr_opengl_line(x1, y1, x2, y2, resize);
		return;
	}

	if ( resize && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		gr_resize_screen_pos(&x1, &y1);
		gr_resize_screen_pos(&x2, &y2);
	}

	INT_CLIPLINE(x1, y1, x2, y2, gr_screen.clip_left, gr_screen.clip_top, gr_screen.clip_right, gr_screen.clip_bottom, return, clipped = 1, swapped = 1);

	GL_state.SetTextureSource(TEXTURE_SOURCE_NONE);
	GL_state.SetAlphaBlendMode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	GL_state.SetZbufferType(ZBUFFER_TYPE_NONE);

	ubyte aa = swapped ? 0 : gr_screen.current_color.alpha;
	ubyte ba = swapped ? gr_screen.current_color.alpha : 0;
	
	float sx1, sy1, sx2, sy2;
	
	sx1 = i2fl(x1 + gr_screen.offset_x);
	sy1 = i2fl(y1 + gr_screen.offset_y);
	sx2 = i2fl(x2 + gr_screen.offset_x);
	sy2 = i2fl(y2 + gr_screen.offset_y);

	if (x1 == x2) {
		if (sy1 < sy2) {
			sy2 += 0.5f;
		} else {
			sy1 += 0.5f;
		}
	} else if (y1 == y2) {
		if (sx1 < sx2) {
			sx2 += 0.5f;
		} else {
			sx1 += 0.5f;
		}
	}

	GLubyte colour[8] = {
		gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, ba,
		gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, aa
	};

	GLfloat verts[4] = {
		sx2, sy2,
		sx1, sy1
	};

	glColorPointer(4, GL_UNSIGNED_BYTE, 0, colour);
	glVertexPointer(2, GL_FLOAT, 0, verts);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glDrawArrays(GL_LINES, 0, 2);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

}

void gr_opengl_circle(int xc, int yc, int d, bool resize)
{
	int p, x, y, r;

	if ( resize && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		gr_resize_screen_pos(&xc, &yc);
	}

	r = d / 2;
	p = 3 - d;
	x = 0;
	y = r;

	// Big clip
	if ( (xc+r) < gr_screen.clip_left ) {
		return;
	}

	if ( (xc-r) > gr_screen.clip_right ) {
		return;
	}

	if ( (yc+r) < gr_screen.clip_top ) {
		return;
	}

	if ( (yc-r) > gr_screen.clip_bottom ) {
		return;
	}

	while (x < y) {
		// Draw the first octant
		gr_opengl_line(xc-y, yc-x, xc+y, yc-x, false);
		gr_opengl_line(xc-y, yc+x, xc+y, yc+x, false);

		if (p < 0) {
			p += (x << 2) + 6;
		} else {
			// Draw the second octant
			gr_opengl_line(xc-x, yc-y, xc+x, yc-y, false);
			gr_opengl_line(xc-x, yc+y, xc+x, yc+y, false);

			p += ((x - y) << 2) + 10;
			y--;
		}

		x++;
	}

	if (x == y) {
		gr_opengl_line(xc-x, yc-y, xc+x, yc-y, false);
		gr_opengl_line(xc-x, yc+y, xc+x, yc+y, false);
	}
}

void gr_opengl_curve(int xc, int yc, int r, int direction)
{
	int a, b, p;

	gr_resize_screen_pos(&xc, &yc);

	if ( (xc + r) < gr_screen.clip_left ) {
		return;
	}

	if ( (yc + r) < gr_screen.clip_top ) {
		return;
	}

	p = 3 - (2 * r);
	a = 0;
	b = r;

	switch (direction) {
		case 0: {
			yc += r;
			xc += r;

			while (a < b) {
				// Draw the first octant
				gr_opengl_line(xc - b + 1, yc - a, xc - b, yc - a, false);

				if (p < 0) {
					p += (a << 2) + 6;
				} else {
					// Draw the second octant
					gr_opengl_line(xc - a + 1, yc - b, xc - a, yc - b, false);
					p += ((a - b) << 2) + 10;
					b--;
				}

				a++;
			}

			break;
		}

		case 1: {
			yc += r;

			while (a < b) {
				// Draw the first octant
				gr_opengl_line(xc + b - 1, yc - a, xc + b, yc - a, false);

				if (p < 0) {
					p += (a << 2) + 6;
				} else {
					// Draw the second octant
					gr_opengl_line(xc + a - 1, yc - b, xc + a, yc - b, false);
					p += ((a - b) << 2) + 10;
					b--;
				}

				a++;
			}

			break;
		}

		case 2: {
			xc += r;

			while (a < b) {
				// Draw the first octant
				gr_opengl_line(xc - b + 1, yc + a, xc - b, yc + a, false);

				if (p < 0) {
					p += (a << 2) + 6;
				} else {
					// Draw the second octant
					gr_opengl_line(xc - a + 1, yc + b, xc - a, yc + b, false);
					p += ((a - b) << 2) + 10;
					b--;
				}

				a++;
			}

			break;
		}

		case 3: {
			while (a < b) {
				// Draw the first octant
				gr_opengl_line(xc + b - 1, yc + a, xc + b, yc + a, false);

				if (p < 0) {
					p += (a << 2) + 6;
				} else {
					// Draw the second octant
					gr_opengl_line(xc + a - 1, yc + b, xc + a, yc + b, false);
					p += ((a - b) << 2) + 10;
					b--;
				}

				a++;
			}

			break;
		}
	}
}

struct v6 { GLfloat x,y,z,w,u,v; };
struct c4 { GLubyte r,g,b,a; };

void opengl_draw_primitive(int nv, vertex **verts, uint flags, float u_scale, float v_scale, int r, int g, int b, int a, int override_primary = 0)
{
	GLenum gl_mode = GL_TRIANGLE_FAN;
	float sx, sy, sz, sw;
	int i,j;
	vertex *va;
	bool isNebula = false;
	bool isRamp = false;
	bool isRGB = false;
	ubyte alpha = (ubyte)a;
	struct v6 *vertPos = (struct v6*) alloca(sizeof(struct v6) * nv);
	struct c4 *vertCol = (struct c4*) alloca(sizeof(struct c4) * nv);

	GL_CHECK_FOR_ERRORS("start of draw_primitive()");

	if (flags & TMAP_FLAG_NEBULA) {
		isNebula = true;
	} else if (flags & TMAP_FLAG_GOURAUD) {
		if (flags & TMAP_FLAG_RAMP) {
			isRamp = true;
		} else if (flags & TMAP_FLAG_RGB) {
			isRGB = true;
		}
	} else {
		glColor4ub( (ubyte)r, (ubyte)g, (ubyte)b, alpha );
	}

	if (flags & TMAP_FLAG_TRISTRIP) {
		gl_mode = GL_TRIANGLE_STRIP;
	} else if (flags & TMAP_FLAG_TRILIST) {
		gl_mode = GL_TRIANGLES;
	} else if (flags & TMAP_FLAG_QUADLIST) {
		gl_mode = GL_QUADS;
	} else if (flags & TMAP_FLAG_QUADSTRIP) {
		gl_mode = GL_TRIANGLE_STRIP;
		Assert((nv % 2) == 0);
	}

	for (i = (nv - 1), j = 0; i >= 0; i--, j++) {
		va = verts[i];

		sw = 1.0f;

		if ( gr_zbuffering || (flags & TMAP_FLAG_NEBULA) ) {
			sz = (1.0f - 1.0f / (1.0f + va->world.xyz.z / 32768.0f));

		//	if ( sz > 0.98f ) {
		//		sz = 0.98f;
		//	}
		} else {
			sz = 0.99f;
		}

		sx = va->screen.xyw.x + (float)gr_screen.offset_x;
		sy = va->screen.xyw.y + (float)gr_screen.offset_y;

		if (flags & TMAP_FLAG_CORRECT) {
			sx /= va->screen.xyw.w;
			sy /= va->screen.xyw.w;
			sz /= va->screen.xyw.w;
			sw /= va->screen.xyw.w;
		}

		if (flags & TMAP_FLAG_ALPHA) {
			alpha = va->a;
		}

		if (override_primary) {
			vertCol[j].r = va->spec_r;
			vertCol[j].g = va->spec_g;
			vertCol[j].b = va->spec_b;
			vertCol[j].a = 255;
		} else {
			if (isNebula) {
				int pal = (va->b * (NEBULA_COLORS-1)) / 255;
				vertCol[j].r = gr_palette[pal*3+0];
				vertCol[j].g = gr_palette[pal*3+1];
				vertCol[j].b = gr_palette[pal*3+2];
				vertCol[j].a = alpha;
			} else if (isRamp) {
				vertCol[j].r = va->b;
				vertCol[j].g = va->b;
				vertCol[j].b = va->b;
				vertCol[j].a = alpha;
			} else if (isRGB) {
				vertCol[j].r = va->r;
				vertCol[j].g = va->g;
				vertCol[j].b = va->b;
				vertCol[j].a = alpha;
			}
		}

		if (flags & TMAP_FLAG_TEXTURED) {
			vertPos[j].u = va->texture_position.u * u_scale;
			vertPos[j].v = va->texture_position.v * v_scale;
		}

		vertPos[j].x = sx;
		vertPos[j].y = sy;
		vertPos[j].z = -sz;
		vertPos[j].w = sw;
	}

	glVertexPointer(4, GL_FLOAT, sizeof(struct v6), &vertPos[0].x);
	glEnableClientState(GL_VERTEX_ARRAY);

	if(flags & TMAP_FLAG_TEXTURED) {
		vglClientActiveTextureARB(GL_TEXTURE0);
		glTexCoordPointer(2, GL_FLOAT, sizeof(struct v6), &vertPos[0].u);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		vglClientActiveTextureARB(GL_TEXTURE1);
		glTexCoordPointer(2, GL_FLOAT, sizeof(struct v6), &vertPos[0].u);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	if(flags & (TMAP_FLAG_NEBULA | TMAP_FLAG_GOURAUD)) {
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, &vertCol[0].r);
		glEnableClientState(GL_COLOR_ARRAY);
	}

	glDrawArrays(gl_mode, 0, nv);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	vglClientActiveTextureARB(GL_TEXTURE0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	GL_CHECK_FOR_ERRORS("start of draw_primitive()");
}

void opengl_tmapper_internal(int nv, vertex **verts, uint flags, int is_scaler = 0)
{
	int i, stage = 0;
	float u_scale = 1.0f, v_scale = 1.0f;
	int alpha,tmap_type, r, g, b;

	GL_CHECK_FOR_ERRORS("start of tmapper_internal()");

	opengl_setup_render_states(r, g, b, alpha, tmap_type, flags, is_scaler);

	if (flags & TMAP_FLAG_TEXTURED) {
		if ( !gr_opengl_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, stage) ) {
			return;
		}

		stage++; // bump!

		// glowmap
		if ( Cmdline_glow && (GLOWMAP > -1) ) {
			if ( !gr_opengl_tcache_set(GLOWMAP, tmap_type, &u_scale, &v_scale, stage) ) {
				return;
			}

			opengl_set_additive_tex_env();

			stage++; // bump
		}
	}


	if ( (flags & TMAP_FLAG_PIXEL_FOG) && (Neb2_render_mode == NEB2_RENDER_POF) ) {
		int nr, ng, nb;
		int ra, ga, ba;
		ra = ga = ba = 0;
	
		for (i = (nv - 1); i >= 0; i--) {
			vertex *va = verts[i];
			float sx, sy;
                
			if (gr_screen.offset_x || gr_screen.offset_y) {
				sx = ((va->screen.xyw.x * 16.0f) + ((float)gr_screen.offset_x * 16.0f)) / 16.0f;
				sy = ((va->screen.xyw.y * 16.0f) + ((float)gr_screen.offset_y * 16.0f)) / 16.0f;
			} else {
				sx = va->screen.xyw.x;
				sy = va->screen.xyw.y;
			}

			neb2_get_pixel( (int)sx, (int)sy, &nr, &ng, &nb );

			ra += nr;
			ga += ng;
			ba += nb;
		}
		
		ra /= nv;
		ga /= nv;
		ba /= nv;

		gr_fog_set(GR_FOGMODE_FOG, ra, ga, ba);
	}

	gr_opengl_set_2d_matrix();

	opengl_draw_primitive(nv, verts, flags, u_scale, v_scale, r, g, b, alpha);

	GL_state.Texture.DisableAll();

	gr_opengl_end_2d_matrix();

	GL_CHECK_FOR_ERRORS("end of tmapper_internal()");
}

// ok we're making some assumptions for now.  mainly it must not be multitextured, lit or fogged.
void opengl_tmapper_internal3d(int nv, vertex **verts, uint flags)
{
	int alpha, tmap_type, r, g, b;
	float u_scale = 1.0f, v_scale = 1.0f;
	GLenum gl_mode = GL_TRIANGLE_FAN;
	bool isRGB = false;

	GL_CHECK_FOR_ERRORS("start of tmapper_internal3d()");

	opengl_setup_render_states(r, g, b, alpha, tmap_type, flags);

	if (flags & TMAP_FLAG_TEXTURED) {
		if ( !gr_opengl_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale) ) {
			return;
		}
	}

	GLboolean cull_face = GL_state.CullFace(GL_FALSE);

	if (flags & TMAP_FLAG_TRILIST) {
		gl_mode = GL_TRIANGLES;
	} else if (flags & TMAP_FLAG_TRISTRIP) {
		gl_mode = GL_TRIANGLE_STRIP;
	} else if (flags & TMAP_FLAG_QUADLIST) {
		gl_mode = GL_QUADS;
	} else if (flags & TMAP_FLAG_QUADSTRIP) {
		gl_mode = GL_QUAD_STRIP;
	}

	if ( (flags & TMAP_FLAG_RGB) && (flags & TMAP_FLAG_GOURAUD) ) {
		isRGB = true;
	}

	// use what opengl_setup_render_states() gives us since this works much better for nebula and transparency
	if ( !isRGB ) {
		glColor4ub( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)alpha );
	}

	SCP_vector<ubyte> colour;
	SCP_vector<float> vertvec;
	SCP_vector<float> uvcoords;

	if (isRGB)
		colour.reserve(nv * 4);
	
	vertvec.reserve(nv * 3);
	uvcoords.reserve(nv * 2);

	for (int i = 0; i < nv; i++) {
		vertex *va = verts[i];
		
		if (isRGB) {
			colour.push_back(va->r);
			colour.push_back(va->g);
			colour.push_back(va->b);
			colour.push_back(alpha);
		}

		uvcoords.push_back(va->texture_position.u);
		uvcoords.push_back(va->texture_position.v);

		vertvec.push_back(va->world.xyz.x);
		vertvec.push_back(va->world.xyz.y);
		vertvec.push_back(va->world.xyz.z);
	}

	if (isRGB) glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colour.front());

	glTexCoordPointer(2, GL_FLOAT, 0, &uvcoords.front());
	glVertexPointer(3, GL_FLOAT, 0, &vertvec.front());

	if (isRGB) glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	glDrawArrays(gl_mode, 0, nv);

	if (isRGB) glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	GL_state.CullFace(cull_face);

	GL_CHECK_FOR_ERRORS("end of tmapper_internal3d()");
}

void gr_opengl_tmapper(int nverts, vertex **verts, uint flags)
{
	if ( !Cmdline_nohtl && (flags & TMAP_HTL_3D_UNLIT) ) {
		opengl_tmapper_internal3d(nverts, verts, flags);
	} else {
		opengl_tmapper_internal(nverts, verts, flags);
	}
}

void opengl_render_internal(int nverts, vertex *verts, uint flags)
{
	int alpha, tmap_type, r, g, b;
	float u_scale = 1.0f, v_scale = 1.0f;
	GLenum gl_mode = GL_TRIANGLE_FAN;
	bool texture_matrix_set = false;

	GL_CHECK_FOR_ERRORS("start of render()");

	opengl_setup_render_states(r, g, b, alpha, tmap_type, flags);

	if (flags & TMAP_FLAG_TRILIST) {
		gl_mode = GL_TRIANGLES;
	} else if (flags & TMAP_FLAG_TRISTRIP) {
		gl_mode = GL_TRIANGLE_STRIP;
	} else if (flags & TMAP_FLAG_QUADLIST) {
		gl_mode = GL_QUADS;
	} else if (flags & TMAP_FLAG_QUADSTRIP) {
		gl_mode = GL_QUAD_STRIP;
	}

	if (flags & TMAP_FLAG_TEXTURED) {
		if ( !gr_opengl_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale) ) {
			return;
		}

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), &verts[0].texture_position.u);

		// adjust texture coords if needed
		if ( (u_scale != 1.0f) || (v_scale != 1.0f) ) {
			glMatrixMode(GL_TEXTURE);
			glPushMatrix();
			glScalef(u_scale, v_scale, 1.0f);

			// switch back to the default modelview mode
			glMatrixMode(GL_MODELVIEW);

			texture_matrix_set = true;
		}
	}

	if ( (flags & TMAP_FLAG_RGB) && (flags & TMAP_FLAG_GOURAUD) ) {
		glEnableClientState(GL_COLOR_ARRAY);

		if (flags & TMAP_FLAG_ALPHA) {
			glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vertex), &verts[0].r);
		} else {
			glColor4ub( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)alpha );
			glColorPointer(3, GL_UNSIGNED_BYTE, sizeof(vertex), &verts[0].r);
		}
	}
	// use what opengl_setup_render_states() gives us since this works much better for nebula and transparency
	else {
		glColor4ub( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)alpha );
	}

	float offset_z = -0.99f;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef((float)gr_screen.offset_x, (float)gr_screen.offset_y, offset_z);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, sizeof(vertex), &verts[0].screen.xyw.x);

	gr_opengl_set_2d_matrix();

	glDrawArrays(gl_mode, 0, nverts);

	gr_opengl_end_2d_matrix();

	GL_state.Texture.DisableAll();

	if (texture_matrix_set) {
		glMatrixMode(GL_TEXTURE);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	GL_CHECK_FOR_ERRORS("end of render()");
}

void opengl_render_internal3d(int nverts, vertex *verts, uint flags)
{
	int alpha, tmap_type, r, g, b;
	float u_scale = 1.0f, v_scale = 1.0f;
	GLenum gl_mode = GL_TRIANGLE_FAN;

	GL_CHECK_FOR_ERRORS("start of render3d()");

	opengl_setup_render_states(r, g, b, alpha, tmap_type, flags);

	if (flags & TMAP_FLAG_TEXTURED) {
		if ( !gr_opengl_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale) ) {
			return;
		}

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), &verts[0].texture_position.u);
	}

	GLboolean cull_face = GL_state.CullFace(GL_FALSE);
	GLboolean lighting = GL_state.Lighting(GL_FALSE);

	if (flags & TMAP_FLAG_TRILIST) {
		gl_mode = GL_TRIANGLES;
	} else if (flags & TMAP_FLAG_TRISTRIP) {
		gl_mode = GL_TRIANGLE_STRIP;
	} else if (flags & TMAP_FLAG_QUADLIST) {
		gl_mode = GL_QUADS;
	} else if (flags & TMAP_FLAG_QUADSTRIP) {
		gl_mode = GL_QUAD_STRIP;
	}

	if ( (flags & TMAP_FLAG_RGB) && (flags & TMAP_FLAG_GOURAUD) ) {
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vertex), &verts[0].r);
	}
	// use what opengl_setup_render_states() gives us since this works much better for nebula and transparency
	else {
		glColor4ub( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)alpha );
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(vertex), &verts[0].world.xyz.x);

	glDrawArrays(gl_mode, 0, nverts);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	GL_state.CullFace(cull_face);
	GL_state.Lighting(lighting);

	GL_CHECK_FOR_ERRORS("end of render3d()");
}


void gr_opengl_render_effect(int nverts, vertex *verts, float *radius_list, uint flags)
{
	int alpha, tmap_type, r, g, b;
	float u_scale = 1.0f, v_scale = 1.0f;
	GLenum gl_mode = GL_TRIANGLE_FAN;
	int attrib_index = -1;
	int zbuff = ZBUFFER_TYPE_DEFAULT;
	GL_CHECK_FOR_ERRORS("start of render3d()");

	Assert(Scene_depth_texture != 0);

	opengl_setup_render_states(r, g, b, alpha, tmap_type, flags);

	if ( flags & TMAP_FLAG_TEXTURED ) {
		if ( flags & TMAP_FLAG_SOFT_QUAD ) {
			int sdr_index;
			if( (flags & TMAP_FLAG_DISTORTION) || (flags & TMAP_FLAG_DISTORTION_THRUSTER) )
			{
				glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
				sdr_index = gr_opengl_maybe_create_shader(SDR_FLAG_SOFT_QUAD|SDR_FLAG_DISTORTION);
				opengl_shader_set_current(&GL_shader[sdr_index]);
				
				vglUniform1iARB(opengl_shader_get_uniform("frameBuffer"), 2);
				
				GL_state.Texture.SetActiveUnit(2);
				GL_state.Texture.SetTarget(GL_TEXTURE_2D);
				GL_state.Texture.Enable(Scene_effect_texture);
				GL_state.Texture.SetActiveUnit(3);
				GL_state.Texture.SetTarget(GL_TEXTURE_2D);
				if(flags & TMAP_FLAG_DISTORTION_THRUSTER)
				{
					vglUniform1iARB(opengl_shader_get_uniform("distMap"), 3);
					GL_state.Texture.Enable(Distortion_texture[!Distortion_switch]);
				}
				else
				{
					vglUniform1iARB(opengl_shader_get_uniform("distMap"), 0);
					GL_state.Texture.Disable();
				}
				zbuff = gr_zbuffer_set(GR_ZBUFF_READ);
			}
			else
			{
				sdr_index = gr_opengl_maybe_create_shader(SDR_FLAG_SOFT_QUAD);
				opengl_shader_set_current(&GL_shader[sdr_index]);
				zbuff = gr_zbuffer_set(GR_ZBUFF_NONE);
			}

			vglUniform1iARB(opengl_shader_get_uniform("baseMap"), 0);
			vglUniform1iARB(opengl_shader_get_uniform("depthMap"), 1);
			vglUniform1fARB(opengl_shader_get_uniform("window_width"), (float)gr_screen.max_w);
			vglUniform1fARB(opengl_shader_get_uniform("window_height"), (float)gr_screen.max_h);
			vglUniform1fARB(opengl_shader_get_uniform("nearZ"), Min_draw_distance);
			vglUniform1fARB(opengl_shader_get_uniform("farZ"), Max_draw_distance);

			if( !(flags & TMAP_FLAG_DISTORTION) && !(flags & TMAP_FLAG_DISTORTION_THRUSTER) ) // Only use vertex attribute with soft particles to avoid OpenGL Errors - Valathil
			{
				attrib_index = opengl_shader_get_attribute("radius_in");
				vglVertexAttribPointerARB(attrib_index, 1, GL_FLOAT, GL_FALSE, 0, radius_list);

				vglEnableVertexAttribArrayARB(attrib_index);

			}
			if(flags & TMAP_FLAG_DISTORTION_THRUSTER)
			{
				attrib_index = opengl_shader_get_attribute("offset_in");
				vglVertexAttribPointerARB(attrib_index, 1, GL_FLOAT, GL_FALSE, 0, radius_list);

				vglEnableVertexAttribArrayARB(attrib_index);
			}
			GL_state.Texture.SetActiveUnit(1);
			GL_state.Texture.SetTarget(GL_TEXTURE_2D);
			GL_state.Texture.Enable(Scene_depth_texture);
		}
		
		if ( !gr_opengl_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale) ) {
			return;
		}

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), &verts[0].texture_position.u);
	}

	GLboolean cull_face = GL_state.CullFace(GL_FALSE);
	GLboolean lighting = GL_state.Lighting(GL_FALSE);

	if (flags & TMAP_FLAG_TRILIST) {
		gl_mode = GL_TRIANGLES;
	} else if (flags & TMAP_FLAG_TRISTRIP) {
		gl_mode = GL_TRIANGLE_STRIP;
	} else if (flags & TMAP_FLAG_QUADLIST) {
		gl_mode = GL_QUADS;
	} else if (flags & TMAP_FLAG_QUADSTRIP) {
		gl_mode = GL_QUAD_STRIP;
	}

	if ( (flags & TMAP_FLAG_RGB) && (flags & TMAP_FLAG_GOURAUD) ) {
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vertex), &verts[0].r);
	}
	// use what opengl_setup_render_states() gives us since this works much better for nebula and transparency
	else {
		glColor4ub( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)alpha );
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, sizeof(vertex), &verts[0].world.xyz.x);

	glDrawArrays(gl_mode, 0, nverts);

	opengl_shader_set_current();

	GL_state.Texture.SetActiveUnit(1);
	GL_state.Texture.Disable();

	GL_state.Texture.SetActiveUnit(2);
	GL_state.Texture.Disable();

	GL_state.Texture.SetActiveUnit(3);
	GL_state.Texture.Disable();

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	if ( attrib_index >= 0 ) {
		vglDisableVertexAttribArrayARB(attrib_index);
	}

	GL_state.CullFace(cull_face);
	GL_state.Lighting(lighting);
	gr_zbuffer_set(zbuff);

	if( (flags & TMAP_FLAG_DISTORTION) || (flags & TMAP_FLAG_DISTORTION_THRUSTER) ) {
		GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
		vglDrawBuffers(2, buffers);
	}

	GL_CHECK_FOR_ERRORS("end of render3d()");
}

void gr_opengl_render(int nverts, vertex *verts, uint flags)
{
	if ( !Cmdline_nohtl && (flags & TMAP_HTL_3D_UNLIT) ) {
		opengl_render_internal3d(nverts, verts, flags);
	} else {
		opengl_render_internal(nverts, verts, flags);
	}
}


#define FIND_SCALED_NUM(x, x0, x1, y0, y1) ( ((((x) - (x0)) * ((y1) - (y0))) / ((x1) - (x0))) + (y0) )

void gr_opengl_scaler(vertex *va, vertex *vb, bool bw_bitmap = false)
{
	float x0, y0, x1, y1;
	float u0, v0, u1, v1;
	float clipped_x0, clipped_y0, clipped_x1, clipped_y1;
	float clipped_u0, clipped_v0, clipped_u1, clipped_v1;
	float xmin, xmax, ymin, ymax;
	int dx0, dy0, dx1, dy1;

	x0 = va->screen.xyw.x;
	y0 = va->screen.xyw.y;
	x1 = vb->screen.xyw.x;
	y1 = vb->screen.xyw.y;

	xmin = i2fl(gr_screen.clip_left);
	ymin = i2fl(gr_screen.clip_top);
	xmax = i2fl(gr_screen.clip_right);
	ymax = i2fl(gr_screen.clip_bottom);

	u0 = va->texture_position.u; v0 = va->texture_position.v;
	u1 = vb->texture_position.u; v1 = vb->texture_position.v;

	// Check for obviously offscreen bitmaps...
	if ( (y1 <= y0) || (x1 <= x0) ) {
		return;
	}

	if ( (x1 < xmin ) || (x0 > xmax) ) {
		return;
	}

	if ( (y1 < ymin ) || (y0 > ymax) ) {
		return;
	}

	clipped_u0 = u0;
	clipped_v0 = v0;
	clipped_u1 = u1;
	clipped_v1 = v1;

	clipped_x0 = x0;
	clipped_y0 = y0;
	clipped_x1 = x1;
	clipped_y1 = y1;

	// Clip the left, moving u0 right as necessary
	if (x0 < xmin) {
		clipped_u0 = FIND_SCALED_NUM(xmin, x0, x1, u0, u1);
		clipped_x0 = xmin;
	}

	// Clip the right, moving u1 left as necessary
	if (x1 > xmax) {
		clipped_u1 = FIND_SCALED_NUM(xmax, x0, x1, u0, u1);
		clipped_x1 = xmax;
	}

	// Clip the top, moving v0 down as necessary
	if (y0 < ymin) {
		clipped_v0 = FIND_SCALED_NUM(ymin, y0, y1, v0, v1);
		clipped_y0 = ymin;
	}

	// Clip the bottom, moving v1 up as necessary
	if (y1 > ymax) {
		clipped_v1 = FIND_SCALED_NUM(ymax, y0, y1, v0, v1);
		clipped_y1 = ymax;
	}
	
	dx0 = fl2i(clipped_x0);
	dx1 = fl2i(clipped_x1);
	dy0 = fl2i(clipped_y0);
	dy1 = fl2i(clipped_y1);

	if ( (dx1 <= dx0) || (dy1 <= dy0) ) {
		return;
	}


	vertex v[4];
	vertex *vl[4];

	vl[0] = &v[0];	
	v->screen.xyw.x = clipped_x0;
	v->screen.xyw.y = clipped_y0;
	v->screen.xyw.w = va->screen.xyw.w;
	v->world.xyz.z = va->world.xyz.z;
	v->texture_position.u = clipped_u0;
	v->texture_position.v = clipped_v0;
	v->spec_r = 0;
	v->spec_g = 0;
	v->spec_b = 0;

	vl[1] = &v[1];	
	v[1].screen.xyw.x = clipped_x1;
	v[1].screen.xyw.y = clipped_y0;
	v[1].screen.xyw.w = va->screen.xyw.w;
	v[1].world.xyz.z = va->world.xyz.z;
	v[1].texture_position.u = clipped_u1;
	v[1].texture_position.v = clipped_v0;
	v[1].spec_r = 0;
	v[1].spec_g = 0;
	v[1].spec_b = 0;

	vl[2] = &v[2];	
	v[2].screen.xyw.x = clipped_x1;
	v[2].screen.xyw.y = clipped_y1;
	v[2].screen.xyw.w = va->screen.xyw.w;
	v[2].world.xyz.z = va->world.xyz.z;
	v[2].texture_position.u = clipped_u1;
	v[2].texture_position.v = clipped_v1;
	v[2].spec_r = 0;
	v[2].spec_g = 0;
	v[2].spec_b = 0;

	vl[3] = &v[3];	
	v[3].screen.xyw.x = clipped_x0;
	v[3].screen.xyw.y = clipped_y1;
	v[3].screen.xyw.w = va->screen.xyw.w;
	v[3].world.xyz.z = va->world.xyz.z;
	v[3].texture_position.u = clipped_u0;
	v[3].texture_position.v = clipped_v1;
	v[3].spec_r = 0;
	v[3].spec_g = 0;
	v[3].spec_b = 0;

	if (!bw_bitmap)
	{
		opengl_tmapper_internal( 4, vl, TMAP_FLAG_TEXTURED, 1 );
	}
	else
	{
		opengl_tmapper_internal( 4, vl,( TMAP_FLAG_TEXTURED | TMAP_FLAG_BW_TEXTURE ), 1 );
	}
}


// cross fade
void gr_opengl_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct)
{
   	gr_set_bitmap(bmap1, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f - pct);
	gr_bitmap(x1, y1);

  	gr_set_bitmap(bmap2, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, pct);
	gr_bitmap(x2, y2);
}

void gr_opengl_shade(int x, int y, int w, int h, bool resize)
{
	if (resize) {
		gr_resize_screen_pos(&x, &y);
		gr_resize_screen_pos(&w, &h);
	}

	int x1 = (gr_screen.offset_x + x);
	int y1 = (gr_screen.offset_y + y);
	int x2 = (gr_screen.offset_x + w);
	int y2 = (gr_screen.offset_y + h);

	if ( (x1 >= gr_screen.max_w) || (y1 >= gr_screen.max_h) ) {
		return;
	}

	CLAMP(x2, x1+1, gr_screen.max_w);
	CLAMP(y2, y1+1, gr_screen.max_h);

	GL_state.SetTextureSource(TEXTURE_SOURCE_NONE);
	GL_state.SetAlphaBlendMode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	GL_state.SetZbufferType(ZBUFFER_TYPE_NONE);

	gr_opengl_set_2d_matrix();

	glColor4ub( (GLubyte)gr_screen.current_shader.r, (GLubyte)gr_screen.current_shader.g,
				(GLubyte)gr_screen.current_shader.b, (GLubyte)gr_screen.current_shader.c );

	opengl_draw_coloured_quad(x1, y1, x2, y2);

	gr_opengl_end_2d_matrix();
}

void gr_opengl_flash(int r, int g, int b)
{
	if ( !(r || g || b) ) {
		return;
	}

	CLAMP(r, 0, 255);
	CLAMP(g, 0, 255);
	CLAMP(b, 0, 255);

	GL_state.SetTextureSource(TEXTURE_SOURCE_NONE);
	GL_state.SetAlphaBlendMode(ALPHA_BLEND_ALPHA_ADDITIVE);
	GL_state.SetZbufferType(ZBUFFER_TYPE_NONE);

	int x1 = (gr_screen.clip_left + gr_screen.offset_x);
	int y1 = (gr_screen.clip_top + gr_screen.offset_y);
	int x2 = (gr_screen.clip_right + gr_screen.offset_x) + 1;
	int y2 = (gr_screen.clip_bottom + gr_screen.offset_y) + 1;

	glColor4ub( (GLubyte)r, (GLubyte)g, (GLubyte)b, 255 );

	opengl_draw_coloured_quad(x1, y1, x2, y2);
}

void gr_opengl_flash_alpha(int r, int g, int b, int a)
{
	if ( !(r || g || b || a) ) {
		return;
	}

	CLAMP(r, 0, 255);
	CLAMP(g, 0, 255);
	CLAMP(b, 0, 255);
	CLAMP(a, 0, 255);

	GL_state.SetTextureSource(TEXTURE_SOURCE_NONE);
	GL_state.SetAlphaBlendMode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	GL_state.SetZbufferType(ZBUFFER_TYPE_NONE);

	int x1 = (gr_screen.clip_left + gr_screen.offset_x);
	int y1 = (gr_screen.clip_top + gr_screen.offset_y);
	int x2 = (gr_screen.clip_right + gr_screen.offset_x) + 1;
	int y2 = (gr_screen.clip_bottom + gr_screen.offset_y) + 1;

	glColor4ub( (GLubyte)r, (GLubyte)g, (GLubyte)b, (GLubyte)a );

	opengl_draw_coloured_quad(x1, y1, x2, y2);
}


void gr_opengl_fade_in(int instantaneous)
{
	// Empty - DDOI
}

void gr_opengl_fade_out(int instantaneous)
{
	// Empty - DDOI
}

void opengl_bitmap_ex_internal(int x, int y, int w, int h, int sx, int sy, bool resize)
{
	if ( (w < 1) || (h < 1) ) {
		return;
	}

	float u_scale, v_scale;
	float u0, u1, v0, v1;
	float x1, x2, y1, y2;
	int bw, bh, do_resize;

	GL_state.SetTextureSource(TEXTURE_SOURCE_NO_FILTERING);
	GL_state.SetAlphaBlendMode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	GL_state.SetZbufferType(ZBUFFER_TYPE_NONE);

	if ( !gr_opengl_tcache_set(gr_screen.current_bitmap, TCACHE_TYPE_INTERFACE, &u_scale, &v_scale) ) {
		return;
	}

	if ( resize && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	bm_get_info(gr_screen.current_bitmap, &bw, &bh);

	u0 = u_scale * (i2fl(sx) / i2fl(bw));
	v0 = v_scale * (i2fl(sy) / i2fl(bh));

	u1 = u_scale * (i2fl(sx+w) / i2fl(bw));
	v1 = v_scale * (i2fl(sy+h) / i2fl(bh));

	x1 = i2fl(x + ((do_resize) ? gr_screen.offset_x_unscaled : gr_screen.offset_x));
	y1 = i2fl(y + ((do_resize) ? gr_screen.offset_y_unscaled : gr_screen.offset_y));
	x2 = x1 + i2fl(w);
	y2 = y1 + i2fl(h);

	if (do_resize) {
		gr_resize_screen_posf(&x1, &y1);
		gr_resize_screen_posf(&x2, &y2);
	}

	glColor4f(1.0f, 1.0f, 1.0f, gr_screen.current_alpha);

	opengl_draw_textured_quad(x1, y1, u0, v0, x2, y2, u1, v1);
}


//these are penguins bitmap functions
void gr_opengl_bitmap_ex(int x, int y, int w, int h, int sx, int sy, bool resize)
{
	int reclip;
#ifndef NDEBUG
	int count = 0;
#endif

	int dx1 = x;
	int dx2 = x + w - 1;
	int dy1 = y;
	int dy2 = y + h - 1;

	int bw, bh, do_resize;

	bm_get_info(gr_screen.current_bitmap, &bw, &bh);

	if ( resize && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	int clip_left = ((do_resize) ? gr_screen.clip_left_unscaled : gr_screen.clip_left);
	int clip_right = ((do_resize) ? gr_screen.clip_right_unscaled : gr_screen.clip_right);
	int clip_top = ((do_resize) ? gr_screen.clip_top_unscaled : gr_screen.clip_top);
	int clip_bottom = ((do_resize) ? gr_screen.clip_bottom_unscaled : gr_screen.clip_bottom);

	do {
		reclip = 0;

#ifndef NDEBUG
		if (count > 1) {
			Int3();
		}

		count++;
#endif
	
		if ( (dx1 > clip_right) || (dx2 < clip_left) ) {
			return;
		}

		if ( (dy1 > clip_bottom) || (dy2 < clip_top) ) {
			return;
		}

		if ( dx1 < clip_left ) {
			sx += clip_left-dx1;
			dx1 = clip_left;
		}

		if ( dy1 < clip_top ) {
			sy += clip_top-dx1;
			dy1 = clip_top;
		}

		if ( dx2 > clip_right ) {
			dx2 = clip_right;
		}

		if ( dy2 > clip_bottom ) {
			dy2 = clip_bottom;
		}

		if ( sx < 0 ) {
			dx1 -= sx;
			sx = 0;
			reclip = 1;
		}

		if ( sy < 0 ) {
			dy1 -= sy;
			sy = 0;
			reclip = 1;
		}

		w = dx2 - dx1 + 1;
		h = dy2 - dy1 + 1;

		if ( (sx + w) > bw ) {
			w = bw - sx;
			dx2 = dx1 + w - 1;
		}

		if ( (sy + h) > bh ) {
			h = bh - sy;
			dy2 = dy1 + h - 1;
		}

		if ( (w < 1) || (h < 1) ) {
			// clipped away!
			return;
		}
	} while (reclip);

	// Make sure clipping algorithm works
#ifndef NDEBUG
	Assert( w > 0 );
	Assert( h > 0 );
	Assert( w == (dx2 - dx1 + 1) );
	Assert( h == (dy2 - dy1 + 1) );
	Assert( sx >= 0 );
	Assert( sy >= 0 );
	Assert( (sx + w) <= bw );
	Assert( (sy + h) <= bh );
	Assert( dx2 >= dx1 );
	Assert( dy2 >= dy1 );
	Assert( (dx1 >= clip_left) && (dx1 <= clip_right) );
	Assert( (dx2 >= clip_left) && (dx2 <= clip_right) );
	Assert( (dy1 >= clip_top) && (dy1 <= clip_bottom) );
	Assert( (dy2 >= clip_top) && (dy2 <= clip_bottom) );
#endif

	// We now have dx1,dy1 and dx2,dy2 and sx, sy all set validly within clip regions.
	opengl_bitmap_ex_internal(dx1, dy1, (dx2 - dx1 + 1), (dy2 - dy1 + 1), sx, sy, resize);
}

/*void gr_opengl_bitmap(int x, int y, bool resize)
{
	int w, h, do_resize;

	bm_get_info( gr_screen.current_bitmap, &w, &h, NULL );

	if ( resize && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		do_resize = 1;
	} else {
		do_resize = 0;
	}

	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;
	int sx=0, sy=0;

	int clip_left = ((do_resize) ? gr_screen.clip_left_unscaled : gr_screen.clip_left);
	int clip_right = ((do_resize) ? gr_screen.clip_right_unscaled : gr_screen.clip_right);
	int clip_top = ((do_resize) ? gr_screen.clip_top_unscaled : gr_screen.clip_top);
	int clip_bottom = ((do_resize) ? gr_screen.clip_bottom_unscaled : gr_screen.clip_bottom);

	if ( (dx1 > clip_right) || (dx2 < clip_left) ) return;
	if ( (dy1 > clip_bottom) || (dy2 < clip_top) ) return;
	if ( dx1 < clip_left ) { sx = clip_left-dx1; dx1 = clip_left; }
	if ( dy1 < clip_top ) { sy = clip_top-dy1; dy1 = clip_top; }
	if ( dx2 > clip_right )	{ dx2 = clip_right; }
	if ( dy2 > clip_bottom ) { dy2 = clip_bottom; }

	if ( sx < 0 ) return;
	if ( sy < 0 ) return;
	if ( sx >= w ) return;
	if ( sy >= h ) return;

	// Draw bitmap bm[sx,sy] into (dx1,dy1)-(dx2,dy2)
	gr_opengl_bitmap_ex(dx1, dy1, dx2-dx1+1, dy2-dy1+1, sx, sy, resize);
}*/

void opengl_render_timer_bar(int colour, float x, float y, float w, float h)
{
	static float pre_set_colours[MAX_NUM_TIMERBARS][3] = {
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.2f, 1.0f, 0.8f }, 
		{ 1.0f, 0.0f, 8.0f }, 
		{ 1.0f, 0.0f, 1.0f },
		{ 1.0f, 0.2f, 0.2f },
		{ 1.0f, 1.0f, 1.0f }
	};

	static float max_fw = (float)gr_screen.max_w; 
	static float max_fh = (float)gr_screen.max_h; 

	x *= max_fw;
	y *= max_fh;
	w *= max_fw;
	h *= max_fh;

	y += 5;

	GL_state.SetTextureSource(TEXTURE_SOURCE_NONE);
	GL_state.SetAlphaBlendMode(ALPHA_BLEND_NONE);
	GL_state.SetZbufferType(ZBUFFER_TYPE_NONE);

	glColor3fv(pre_set_colours[colour]);

	opengl_draw_coloured_quad(x, y, x+w, y+h);
}

void gr_opengl_sphere_htl(float rad)
{
	if (Cmdline_nohtl) {
		return;
	}

	GLUquadricObj *quad = NULL;

	// FIXME: before this is used in anything other than FRED2 we need to make this creation/deletion 
	// stuff global so that it's not so slow (it can be reused for multiple quadratic objects)
	quad = gluNewQuadric();

	if (quad == NULL) {
		Int3();
		return;
	}

	GL_state.SetTextureSource(TEXTURE_SOURCE_NONE);
	GL_state.SetAlphaBlendMode(ALPHA_BLEND_NONE);
	GL_state.SetZbufferType(ZBUFFER_TYPE_FULL);

	glColor3ub(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue);

	// FIXME: opengl_check_for_errors() needs to be modified to work with this at
	// some point but for now I just don't care so it does nothing
	gluQuadricCallback(quad, GLU_ERROR, NULL);

	// FIXME: maybe support fill/wireframe with a future flag?
	gluQuadricDrawStyle(quad, GLU_FILL);

	// assuming unlit spheres, otherwise use GLU_SMOOTH so that it looks better
	gluQuadricNormals(quad, GLU_NONE);

	// we could set the slices/stacks at some point in the future but just use 16 now since it looks ok
	gluSphere(quad, (GLdouble)rad, 16, 16);

	// FIXME: I just heard this scream "Globalize Me!!".  It was really scary.  I even cried.
	gluDeleteQuadric(quad);
}


void gr_opengl_draw_line_list(colored_vector *lines, int num)
{
	if (Cmdline_nohtl) {
		return;
	}
}
extern int opengl_check_framebuffer();
void opengl_setup_scene_textures()
{
	Scene_texture_initialized = 0;

	if ( !Use_GLSL || Cmdline_no_fbo || !Is_Extension_Enabled(OGL_EXT_FRAMEBUFFER_OBJECT) ) {
		Cmdline_postprocess = 0;
		Cmdline_softparticles = 0;

		Scene_color_texture = 0;
		Scene_effect_texture = 0;
		Scene_depth_texture = 0;
		return;
	}

	// for ease of use we require support for non-power-of-2 textures in one
	// form or another:
	//    - the NPOT extension
	//    - GL version 2.0+ (which should work for non-reporting ATI cards since we don't use mipmaps)
	if ( !(Is_Extension_Enabled(OGL_ARB_TEXTURE_NON_POWER_OF_TWO) || (GL_version >= 20)) ) {
		Cmdline_postprocess = 0;
		Cmdline_softparticles = 0;

		Scene_color_texture = 0;
		Scene_effect_texture = 0;
		Scene_depth_texture = 0;
		return;
	}

	// clamp size, if needed
	Scene_texture_width = gr_screen.max_w;
	Scene_texture_height = gr_screen.max_h;

	if ( Scene_texture_width > GL_max_renderbuffer_size ) {
		Scene_texture_width = GL_max_renderbuffer_size;
	}

	if ( Scene_texture_height > GL_max_renderbuffer_size) {
		Scene_texture_height = GL_max_renderbuffer_size;
	}

	// create framebuffer
	vglGenFramebuffersEXT(1, &Scene_framebuffer);
	vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, Scene_framebuffer);

	// setup main render texture
	glGenTextures(1, &Scene_color_texture);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_color_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Scene_texture_width, Scene_texture_height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

	vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, Scene_color_texture, 0);

	//Set up luminance texture (used as input for FXAA)
	glGenTextures(1, &Scene_luminance_texture);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_luminance_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Scene_texture_width, Scene_texture_height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

	// setup effect texture

	glGenTextures(1, &Scene_effect_texture);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_effect_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Scene_texture_width, Scene_texture_height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

	vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, Scene_effect_texture, 0);
	
	// setup cockpit depth texture
	glGenTextures(1, &Cockpit_depth_texture);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Cockpit_depth_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, Scene_texture_width, Scene_texture_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, Cockpit_depth_texture, 0);
	gr_zbuffer_set(GR_ZBUFF_FULL);
	glClear(GL_DEPTH_BUFFER_BIT);

	// setup main depth texture
	glGenTextures(1, &Scene_depth_texture);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_depth_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, Scene_texture_width, Scene_texture_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, Scene_depth_texture, 0);

	if ( opengl_check_framebuffer() ) {
		vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		vglDeleteFramebuffersEXT(1, &Scene_framebuffer);
		Scene_framebuffer = 0;

		GL_state.Texture.Disable();

		glDeleteTextures(1, &Scene_color_texture);
		Scene_color_texture = 0;

		glDeleteTextures(1, &Scene_effect_texture);
		Scene_effect_texture = 0;

		glDeleteTextures(1, &Scene_depth_texture);
		Scene_depth_texture = 0;

		glDeleteTextures(1, &Scene_luminance_texture);
		Scene_luminance_texture = 0;

		//glDeleteTextures(1, &Scene_fxaa_output_texture);
		//Scene_fxaa_output_texture = 0;

		Cmdline_postprocess = 0;
		Cmdline_softparticles = 0;
		return;
	}

	//Setup thruster distortion framebuffer
	vglGenFramebuffersEXT(1, &Distortion_framebuffer);
	vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, Distortion_framebuffer);

	glGenTextures(2, Distortion_texture);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Distortion_texture[0]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 32, 32, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

	GL_state.Texture.Enable(Distortion_texture[1]);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 32, 32, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

	vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, Distortion_texture[0], 0);
	
	
	
	if ( opengl_check_framebuffer() ) {
		vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		vglDeleteFramebuffersEXT(1, &Distortion_framebuffer);
		Distortion_framebuffer = 0;

		GL_state.Texture.Disable();

		glDeleteTextures(2, Distortion_texture);
		Scene_color_texture = 0;
		return;
	}

	if ( opengl_check_for_errors("post_init_framebuffer()") ) {
		Scene_color_texture = 0;
		Scene_depth_texture = 0;

		Cmdline_postprocess = 0;
		Cmdline_softparticles = 0;
		return;
	}

	vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	Scene_texture_initialized = 1;
	Scene_framebuffer_in_frame = false;
}

void opengl_scene_texture_shutdown()
{
	if ( !Scene_texture_initialized ) {
		return;
	}

	if ( Scene_color_texture ) {
		glDeleteTextures(1, &Scene_color_texture);
		Scene_color_texture = 0;
	}

	if ( Scene_effect_texture ) {
		glDeleteTextures(1, &Scene_effect_texture);
		Scene_effect_texture = 0;
	}

	if ( Scene_depth_texture ) {
		glDeleteTextures(1, &Scene_depth_texture);
		Scene_depth_texture = 0;
	}

	if ( Scene_framebuffer ) {
		vglDeleteFramebuffersEXT(1, &Scene_framebuffer);
		Scene_framebuffer = 0;
	}

	if ( Distortion_texture ) {
		glDeleteTextures(2, Distortion_texture);
		Distortion_texture[0] = 0;
		Distortion_texture[1] = 0;
	}

	if ( Distortion_framebuffer ) {
		vglDeleteFramebuffersEXT(1, &Distortion_framebuffer);
		Distortion_framebuffer = 0;
	}

	Scene_texture_initialized = 0;
	Scene_framebuffer_in_frame = false;
}

void gr_opengl_scene_texture_begin()
{
	if ( !Scene_texture_initialized ) {
		return;
	}

	if ( Scene_framebuffer_in_frame ) {
		return;
	}

	vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, Scene_framebuffer);

	if (GL_rendering_to_texture)
	{
		Scene_texture_u_scale = i2fl(gr_screen.max_w) / i2fl(Scene_texture_width);
		Scene_texture_v_scale = i2fl(gr_screen.max_h) / i2fl(Scene_texture_height);

		CLAMP(Scene_texture_u_scale, 0.0f, 1.0f);
		CLAMP(Scene_texture_v_scale, 0.0f, 1.0f);
	}
	else
	{
		Scene_texture_u_scale = 1.0f;
		Scene_texture_v_scale = 1.0f;
	}

	GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
	vglDrawBuffers(2, buffers);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Scene_framebuffer_in_frame = true;
}

float time_buffer = 0.0f;
void gr_opengl_scene_texture_end()
{
	if ( !Scene_framebuffer_in_frame ) {
		return;
	}
	
	time_buffer+=flFrametime;
	if(time_buffer>0.03f)
	{
		gr_opengl_update_distortion();
		time_buffer = 0.0f;
	}
	
	if ( Cmdline_postprocess && !PostProcessing_override ) {
		gr_post_process_end();
	} else {
		GLboolean depth = GL_state.DepthTest(GL_FALSE);
		GLboolean depth_mask = GL_state.DepthMask(GL_FALSE);
		GLboolean light = GL_state.Lighting(GL_FALSE);
		GLboolean blend = GL_state.Blend(GL_FALSE);
		GLboolean cull = GL_state.CullFace(GL_FALSE);

		vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, opengl_get_rtt_framebuffer());

		GL_state.Texture.SetActiveUnit(0);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D);
		GL_state.Texture.Enable(Scene_color_texture);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		if (GL_rendering_to_texture)
		{
			GLfloat vertices[8] = {
				0.0f, (float)gr_screen.max_h,
				(float)gr_screen.max_w, (float)gr_screen.max_h,
				(float)gr_screen.max_w, 0.0f,
				0.0f, 0.0f
			};

			GLfloat uvcoords[8] = {
				Scene_texture_u_scale, 0.0f,
				0.0f, 0.0f,
				0.0f, Scene_texture_v_scale,
				Scene_texture_u_scale, Scene_texture_v_scale
			};

			glVertexPointer(2, GL_FLOAT, 0, vertices);
			glTexCoordPointer(2, GL_FLOAT, 0, uvcoords);

			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		else
		{
			GLfloat vertices[8] = {
				0.0f, (float)gr_screen.max_h,
				(float)gr_screen.max_w, (float)gr_screen.max_h,
				(float)gr_screen.max_w, 0.0f,
				0.0f, 0.0f
			};

			GLfloat uvcoords[8] = {
				0.0f, 0.0f,
				Scene_texture_u_scale, 0.0f,
				Scene_texture_u_scale, Scene_texture_v_scale,
				0.0f, Scene_texture_v_scale
			};

			glVertexPointer(2, GL_FLOAT, 0, vertices);
			glTexCoordPointer(2, GL_FLOAT, 0, uvcoords);

			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		
			glDrawArrays(GL_QUADS, 0, 4);
			
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}

		GL_state.Texture.SetActiveUnit(0);
		GL_state.Texture.Disable();

		// reset state
		GL_state.DepthTest(depth);
		GL_state.DepthMask(depth_mask);
		GL_state.Lighting(light);
		GL_state.Blend(blend);
		GL_state.CullFace(cull);
	}

	// Reset the UV scale values
	
	Scene_texture_u_scale = 1.0f;
	Scene_texture_v_scale = 1.0f;

	Scene_framebuffer_in_frame = false;
}

void gr_opengl_update_distortion()
{
	GLboolean depth = GL_state.DepthTest(GL_FALSE);
	GLboolean depth_mask = GL_state.DepthMask(GL_FALSE);
	GLboolean light = GL_state.Lighting(GL_FALSE);
	GLboolean blend = GL_state.Blend(GL_FALSE);
	GLboolean cull = GL_state.CullFace(GL_FALSE);

	opengl_shader_set_current();
	vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, Distortion_framebuffer);
	vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, Distortion_texture[!Distortion_switch], 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

	glViewport(0,0,32,32);
	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Distortion_texture[Distortion_switch]);
	glClearColor(0.5f, 0.5f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	GLfloat texcoord[8] = {
		0.0f, 0.0f,
		0.96875f, 0.0f,
		0.96875f, 1.0f,
		0.0f, 1.0f
	};

	GLfloat vertices[8] = {
		0.03f*(float)gr_screen.max_w,(float)gr_screen.max_h,
		(float)gr_screen.max_w, (float)gr_screen.max_h,
		(float)gr_screen.max_w, 0.0f,
		0.03f*(float)gr_screen.max_w, 0.0f
	};

	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoord);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		
	glDrawArrays(GL_QUADS, 0, 4);
			
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	GL_state.Texture.Disable();

	SCP_vector<ubyte> colours;
	SCP_vector<GLfloat> vertex;
	colours.reserve(33 * 4);
	vertex.reserve(33 * 2);
	for(int i = 0; i < 33; i++)
	{
		colours.push_back(rand()%256);
		colours.push_back(rand()%256);
		colours.push_back(255);
		colours.push_back(255);

		vertex.push_back(0.04f);
		vertex.push_back((float)gr_screen.max_h*0.03125f*i);
	}

	glVertexPointer(2, GL_FLOAT, 0, &vertex.front());
	glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colours.front());

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
		
	glDrawArrays(GL_POINTS, 0, 33);
			
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	Distortion_switch = !Distortion_switch;

	// reset state
	vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, Scene_framebuffer);

	glViewport(0,0,gr_screen.max_w,gr_screen.max_h);

	GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
	vglDrawBuffers(2, buffers);

	GL_state.DepthTest(depth);
	GL_state.DepthMask(depth_mask);
	GL_state.Lighting(light);
	GL_state.Blend(blend);
	GL_state.CullFace(cull);
}
