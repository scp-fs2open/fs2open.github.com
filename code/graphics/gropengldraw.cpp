/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell
 * or otherwise commercially exploit the source or things you created based on the
 * source.
 *
*/

#include <algorithm>
#include "bmpman/bmpman.h"
#include "cmdline/cmdline.h"
#include "freespace2/freespace.h"
#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "graphics/grinternal.h"
#include "graphics/gropengl.h"
#include "graphics/gropenglbmpman.h"
#include "graphics/gropengldraw.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropengllight.h"
#include "graphics/gropenglpostprocessing.h"
#include "graphics/gropenglshader.h"
#include "graphics/gropengltexture.h"
#include "graphics/gropengltnl.h"
#include "graphics/line.h"
#include "lighting/lighting.h"
#include "math/floating.h"
#include "nebula/neb.h"
#include "osapi/osapi.h"
#include "palman/palman.h"
#include "render/3d.h"

GLuint Scene_framebuffer;
GLuint Scene_color_texture;
GLuint Scene_position_texture;
GLuint Scene_normal_texture;
GLuint Scene_specular_texture;
GLuint Scene_luminance_texture;
GLuint Scene_effect_texture;
GLuint Scene_depth_texture;
GLuint Cockpit_depth_texture;
GLuint Scene_stencil_buffer;

GLuint Distortion_framebuffer;
GLuint Distortion_texture[2];
int Distortion_switch = 0;

int Scene_texture_initialized;
bool Scene_framebuffer_in_frame;

bool Deferred_lighting = false;

int Scene_texture_width;
int Scene_texture_height;

GLfloat Scene_texture_u_scale = 1.0f;
GLfloat Scene_texture_v_scale = 1.0f;

GLuint deferred_light_sphere_vbo = 0;
GLuint deferred_light_sphere_ibo = 0;
GLushort deferred_light_sphere_vcount = 0;
GLuint deferred_light_sphere_icount = 0;

GLuint deferred_light_cylinder_vbo = 0;
GLuint deferred_light_cylinder_ibo = 0;
GLushort deferred_light_cylinder_vcount = 0;
GLuint deferred_light_cylinder_icount = 0;

struct opengl_vertex_bind {
	vertex_format_data::vertex_format format;

	enum type {
		POSITION,
		COLOR,
		TEXCOORD0,
		TEXCOORD1,
		NORMAL,
		ATTRIB
	};

	opengl_vertex_bind::type binding_type;

	GLint size;
	GLenum data_type;

	// used only by vertex attributes
	SCP_string attrib_name;
	GLboolean normalized;
};

static opengl_vertex_bind GL_array_binding_data[] =
{
	{ vertex_format_data::POSITION4,	opengl_vertex_bind::POSITION,	4, GL_FLOAT,			"",					GL_FALSE },
	{ vertex_format_data::POSITION3,	opengl_vertex_bind::POSITION,	3, GL_FLOAT,			"",					GL_FALSE },
	{ vertex_format_data::POSITION2,	opengl_vertex_bind::POSITION,	2, GL_FLOAT,			"",					GL_FALSE },
	{ vertex_format_data::SCREEN_POS,	opengl_vertex_bind::POSITION,	2, GL_INT,				"",					GL_FALSE },
	{ vertex_format_data::COLOR3,		opengl_vertex_bind::COLOR,		3, GL_UNSIGNED_BYTE,	"",					GL_FALSE },
	{ vertex_format_data::COLOR4,		opengl_vertex_bind::COLOR,		4, GL_UNSIGNED_BYTE,	"",					GL_FALSE },
	{ vertex_format_data::TEX_COORD,	opengl_vertex_bind::TEXCOORD0,	2, GL_FLOAT,			"",					GL_FALSE },
	{ vertex_format_data::NORMAL,		opengl_vertex_bind::NORMAL,		3, GL_FLOAT,			"",					GL_FALSE },
	{ vertex_format_data::TANGENT,		opengl_vertex_bind::TEXCOORD1,	4, GL_FLOAT,			"",					GL_FALSE },
	{ vertex_format_data::MODEL_ID,		opengl_vertex_bind::ATTRIB,		1, GL_FLOAT,			"model_id",			GL_FALSE },
	{ vertex_format_data::RADIUS,		opengl_vertex_bind::ATTRIB,		1, GL_FLOAT,			"radius",			GL_FALSE },
	{ vertex_format_data::FVEC,			opengl_vertex_bind::ATTRIB,		3, GL_FLOAT,			"fvec",				GL_FALSE },
	{ vertex_format_data::UVEC,			opengl_vertex_bind::ATTRIB,		3, GL_FLOAT,			"uvec",				GL_FALSE },
	{ vertex_format_data::INTENSITY,	opengl_vertex_bind::ATTRIB,		1, GL_FLOAT,			"intensity",		GL_FALSE }
};

void opengl_bind_vertex_component(vertex_format_data &vert_component)
{
	opengl_vertex_bind &bind_info = GL_array_binding_data[vert_component.format_type];

	switch ( bind_info.binding_type ) {
		case opengl_vertex_bind::POSITION:
		{
			GL_state.Array.EnableClientVertex();
			GL_state.Array.VertexPointer(bind_info.size, bind_info.data_type, vert_component.stride, vert_component.data_src);
			break;
		}
		case opengl_vertex_bind::TEXCOORD0:
		{
			GL_state.Array.SetActiveClientUnit(0);
			GL_state.Array.EnableClientTexture();
			GL_state.Array.TexPointer(bind_info.size, bind_info.data_type, vert_component.stride, vert_component.data_src);
			break;
		}
		case opengl_vertex_bind::TEXCOORD1:
		{
			GL_state.Array.SetActiveClientUnit(1);
			GL_state.Array.EnableClientTexture();
			GL_state.Array.TexPointer(bind_info.size, bind_info.data_type, vert_component.stride, vert_component.data_src);
			break;
		}
		case opengl_vertex_bind::COLOR:
		{
			GL_state.Array.EnableClientColor();
			GL_state.Array.ColorPointer(bind_info.size, bind_info.data_type, vert_component.stride, vert_component.data_src);
			GL_state.InvalidateColor();
			break;
		}
		case opengl_vertex_bind::NORMAL:
		{
			GL_state.Array.EnableClientNormal();
			GL_state.Array.NormalPointer(bind_info.data_type, vert_component.stride, vert_component.data_src);
			break;
		}
		case opengl_vertex_bind::ATTRIB:
		{
			// grabbing a vertex attribute is dependent on what current shader has been set. i hope no one calls opengl_bind_vertex_layout before opengl_set_current_shader
			GLint index = opengl_shader_get_attribute(bind_info.attrib_name.c_str());

			if ( index >= 0 ) {
				GL_state.Array.EnableVertexAttrib(index);
				GL_state.Array.VertexAttribPointer(index, bind_info.size, bind_info.data_type, bind_info.normalized, vert_component.stride, vert_component.data_src);
			}

			break;
		}
	}
}

void opengl_bind_vertex_layout(vertex_layout &layout)
{
	GL_state.Array.BindPointersBegin();

	uint num_vertex_bindings = layout.get_num_vertex_components();

	for ( uint i = 0; i < num_vertex_bindings; ++i ) {
		opengl_bind_vertex_component(*layout.get_vertex_component(i));
	}

	GL_state.Array.BindPointersEnd();
}

void gr_opengl_pixel(int x, int y, int resize_mode)
{
	gr_line(x, y, x, y, resize_mode);
}

void opengl_aabitmap_ex_internal(int x, int y, int w, int h, int sx, int sy, int resize_mode, bool mirror)
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

	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
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
		gr_resize_screen_posf(&x1, &y1, NULL, NULL, resize_mode);
		gr_resize_screen_posf(&x2, &y2, NULL, NULL, resize_mode);
	}

	GL_state.Color(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);

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

void gr_opengl_aabitmap_ex(int x, int y, int w, int h, int sx, int sy, int resize_mode, bool mirror)
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

	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
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
	opengl_aabitmap_ex_internal(dx1, dy1, (dx2 - dx1 + 1), (dy2 - dy1 + 1), sx, sy, resize_mode, mirror);
}

void gr_opengl_aabitmap(int x, int y, int resize_mode, bool mirror)
{
	int w, h, do_resize;

	bm_get_info(gr_screen.current_bitmap, &w, &h);

	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
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
	opengl_aabitmap_ex_internal(dx1, dy1, (dx2 - dx1 + 1), (dy2 - dy1 + 1), sx, sy, resize_mode, mirror);
}

#define MAX_VERTS_PER_DRAW 120
struct v4 { GLfloat x,y,u,v; };
static v4 GL_string_render_buff[MAX_VERTS_PER_DRAW];

void gr_opengl_string(float sx, float sy, const char *s, int resize_mode)
{
	int width, spacing, letter;
	float x, y;
	bool do_resize;
	float bw, bh;
	float u0, u1, v0, v1;
	float x1, x2, y1, y2;
	float u_scale, v_scale;

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

	int buffer_offset = 0;

	int ibw, ibh;

	bm_get_info(gr_screen.current_bitmap, &ibw, &ibh);

	bw = i2fl(ibw);
	bh = i2fl(ibh);

	// set color!
	if (gr_screen.current_color.is_alphacolor) {
		GL_state.Color(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);
	} else {
		GL_state.Color(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue);
	}

//	if ( (gr_screen.custom_size && resize) || (gr_screen.rendering_to_texture != -1) ) {
	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		do_resize = true;
	} else {
		do_resize = false;
	}

	int clip_left = ((do_resize) ? gr_screen.clip_left_unscaled : gr_screen.clip_left);
	int clip_right = ((do_resize) ? gr_screen.clip_right_unscaled : gr_screen.clip_right);
	int clip_top = ((do_resize) ? gr_screen.clip_top_unscaled : gr_screen.clip_top);
	int clip_bottom = ((do_resize) ? gr_screen.clip_bottom_unscaled : gr_screen.clip_bottom);

	x = sx;
	y = sy;

	if (sx == (float)0x8000) {
		// centered
		x = (float)get_centered_x(s, !do_resize);
	} else {
		x = sx;
	}

	spacing = 0;

	GLboolean cull_face = GL_state.CullFace(GL_FALSE);

	GL_state.Array.BindArrayBuffer(0);

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION2, sizeof(v4), &GL_string_render_buff[0].x);
	vert_def.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(v4), &GL_string_render_buff[0].u);

	opengl_bind_vertex_layout(vert_def);

	// pick out letter coords, draw it, goto next letter and do the same
	while (*s)	{
		x += spacing;

		while (*s == '\n')	{
			s++;
			y += Current_font->h;

			if (sx == (float)0x8000) {
				// centered
				x = (float)get_centered_x(s, !do_resize);
			} else {
				x = sx;
			}
		}

		if (*s == 0) {
			break;
		}

		letter = get_char_width(s[0], s[1], &width, &spacing);
		s++;

		// not in font, draw as space
		if (letter < 0) {
			continue;
		}

		float xd, yd, xc, yc;
		float wc, hc;

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
			gr_resize_screen_posf( &x1, &y1, NULL, NULL, resize_mode );
			gr_resize_screen_posf( &x2, &y2, NULL, NULL, resize_mode );
		}

		u0 = u_scale * (i2fl(u+xd) / bw);
		v0 = v_scale * (i2fl(v+yd) / bh);

		u1 = u_scale * (i2fl((u+xd)+wc) / bw);
		v1 = v_scale * (i2fl((v+yd)+hc) / bh);

		if ( buffer_offset == MAX_VERTS_PER_DRAW ) {
			glDrawArrays(GL_TRIANGLES, 0, buffer_offset);
			buffer_offset = 0;
		}

		GL_string_render_buff[buffer_offset].x = (GLfloat)x1;
		GL_string_render_buff[buffer_offset].y = (GLfloat)y1;
		GL_string_render_buff[buffer_offset].u = u0;
		GL_string_render_buff[buffer_offset].v = v0;
		buffer_offset++;

		GL_string_render_buff[buffer_offset].x = (GLfloat)x1;
		GL_string_render_buff[buffer_offset].y = (GLfloat)y2;
		GL_string_render_buff[buffer_offset].u = u0;
		GL_string_render_buff[buffer_offset].v = v1;
		buffer_offset++;

		GL_string_render_buff[buffer_offset].x = (GLfloat)x2;
		GL_string_render_buff[buffer_offset].y = (GLfloat)y1;
		GL_string_render_buff[buffer_offset].u = u1;
		GL_string_render_buff[buffer_offset].v = v0;
		buffer_offset++;

		GL_string_render_buff[buffer_offset].x = (GLfloat)x1;
		GL_string_render_buff[buffer_offset].y = (GLfloat)y2;
		GL_string_render_buff[buffer_offset].u = u0;
		GL_string_render_buff[buffer_offset].v = v1;
		buffer_offset++;

		GL_string_render_buff[buffer_offset].x = (GLfloat)x2;
		GL_string_render_buff[buffer_offset].y = (GLfloat)y1;
		GL_string_render_buff[buffer_offset].u = u1;
		GL_string_render_buff[buffer_offset].v = v0;
		buffer_offset++;

		GL_string_render_buff[buffer_offset].x = (GLfloat)x2;
		GL_string_render_buff[buffer_offset].y = (GLfloat)y2;
		GL_string_render_buff[buffer_offset].u = u1;
		GL_string_render_buff[buffer_offset].v = v1;
		buffer_offset++;
	}

	if ( buffer_offset ) {
		glDrawArrays(GL_TRIANGLES, 0, buffer_offset);
	}

	GL_state.CullFace(cull_face);

	GL_CHECK_FOR_ERRORS("end of string()");
}

void gr_opengl_string(int sx, int sy, const char *s, int resize_mode)
{
	gr_opengl_string(i2fl(sx), i2fl(sy), s, resize_mode);
}

void gr_opengl_line(int x1,int y1,int x2,int y2, int resize_mode)
{
	int do_resize;
	float sx1, sy1;
	float sx2, sy2;

	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
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


	INT_CLIPLINE(x1, y1, x2, y2, clip_left, clip_top, clip_right, clip_bottom, return, ;, ;);

	sx1 = i2fl(x1 + offset_x);
	sy1 = i2fl(y1 + offset_y);
	sx2 = i2fl(x2 + offset_x);
	sy2 = i2fl(y2 + offset_y);


	if (do_resize) {
		gr_resize_screen_posf(&sx1, &sy1, NULL, NULL, resize_mode);
		gr_resize_screen_posf(&sx2, &sy2, NULL, NULL, resize_mode);
	}

	GL_state.SetTextureSource(TEXTURE_SOURCE_NONE);
	GL_state.SetAlphaBlendMode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	GL_state.SetZbufferType(ZBUFFER_TYPE_NONE);

	if ( (x1 == x2) && (y1 == y2) ) {
		gr_opengl_set_2d_matrix();

		GLfloat vert[3]= {sx1, sy1, -0.99f};
		GL_state.Color(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);

		vertex_layout vert_def;

		vert_def.add_vertex_component(vertex_format_data::POSITION3, 0, vert);

		opengl_bind_vertex_layout(vert_def);

		glDrawArrays(GL_POINTS, 0, 1);

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

	GL_state.Color(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION3, 0, line);

	opengl_bind_vertex_layout(vert_def);

	glDrawArrays(GL_LINES, 0, 2);

	GL_CHECK_FOR_ERRORS("end of opengl_line()");

	gr_opengl_end_2d_matrix();
}

void gr_opengl_line_htl(const vec3d *start, const vec3d *end)
{
	if (Cmdline_nohtl) {
		return;
	}

	gr_zbuffer_type zbuffer_state = (gr_zbuffering) ? ZBUFFER_TYPE_FULL : ZBUFFER_TYPE_NONE;
	GL_state.SetTextureSource(TEXTURE_SOURCE_NONE);
	GL_state.SetZbufferType(zbuffer_state);


    if (gr_screen.current_color.is_alphacolor) {
        GL_state.SetAlphaBlendMode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
		GL_state.Color( gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha );
	} else {
        GL_state.SetAlphaBlendMode(ALPHA_BLEND_NONE);
		GL_state.Color( gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue );
    }

	GLfloat line[6] = {
		start->xyz.x,	start->xyz.y,	start->xyz.z,
		end->xyz.x,		end->xyz.y,		end->xyz.z
	};

	GL_state.Array.BindArrayBuffer(0);
	opengl_shader_set_current();

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION3, 0, line);

	opengl_bind_vertex_layout(vert_def);

	glDrawArrays(GL_LINES, 0, 2);

	GL_CHECK_FOR_ERRORS("end of opengl_line_htl()");
}

void gr_opengl_aaline(vertex *v1, vertex *v2)
{
// -- AA OpenGL lines.  Looks good but they are kinda slow so this is disabled until an option is implemented - taylor
//	gr_opengl_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );
//	glEnable( GL_LINE_SMOOTH );
//	glHint( GL_LINE_SMOOTH_HINT, GL_FASTEST );
//	glLineWidth( 1.0 );

	float x1 = v1->screen.xyw.x;
	float y1 = v1->screen.xyw.y;
	float x2 = v2->screen.xyw.x;
	float y2 = v2->screen.xyw.y;
	float sx1, sy1;
	float sx2, sy2;


	FL_CLIPLINE(x1, y1, x2, y2, (float)gr_screen.clip_left, (float)gr_screen.clip_top, (float)gr_screen.clip_right, (float)gr_screen.clip_bottom, return, ;, ;);

	sx1 = x1 + (float)gr_screen.offset_x;
	sy1 = y1 + (float)gr_screen.offset_y;
	sx2 = x2 + (float)gr_screen.offset_x;
	sy2 = y2 + (float)gr_screen.offset_y;

	GL_state.SetTextureSource(TEXTURE_SOURCE_NONE);
	GL_state.SetAlphaBlendMode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	GL_state.SetZbufferType(ZBUFFER_TYPE_NONE);

	if ( (x1 == x2) && (y1 == y2) ) {
		gr_opengl_set_2d_matrix();

		GL_state.Color(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);

		GLfloat vert[3]= {sx1, sy1, -0.99f};

		vertex_layout vert_def;

		vert_def.add_vertex_component(vertex_format_data::POSITION3, 0, vert);

		opengl_bind_vertex_layout(vert_def);

		glDrawArrays(GL_POINTS, 0, 1);

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

	GL_state.Color(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);
	GLfloat line[6] = {
		sx2, sy2, -0.99f,
		sx1, sy1, -0.99f
	};

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION3, 0, line);

	opengl_bind_vertex_layout(vert_def);

	glDrawArrays(GL_LINES, 0, 2);

	GL_CHECK_FOR_ERRORS("end of opengl_aaline()");

	gr_opengl_end_2d_matrix();

//	glDisable( GL_LINE_SMOOTH );
}

void gr_opengl_gradient(int x1, int y1, int x2, int y2, int resize_mode)
{
	int swapped = 0;

	if ( !gr_screen.current_color.is_alphacolor ) {
		gr_opengl_line(x1, y1, x2, y2, resize_mode);
		return;
	}

	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		gr_resize_screen_pos(&x1, &y1, NULL, NULL, resize_mode);
		gr_resize_screen_pos(&x2, &y2, NULL, NULL, resize_mode);
	}

	INT_CLIPLINE(x1, y1, x2, y2, gr_screen.clip_left, gr_screen.clip_top, gr_screen.clip_right, gr_screen.clip_bottom, return, ;, swapped = 1);

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

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION2, 0, verts);
	vert_def.add_vertex_component(vertex_format_data::COLOR4, 0, colour);

	opengl_bind_vertex_layout(vert_def);

	glDrawArrays(GL_LINES, 0, 2);
}

void gr_opengl_circle(int xc, int yc, int d, int resize_mode)
{
	gr_opengl_arc(xc, yc, d / 2.0f, 0.0f, 360.0f, true, resize_mode);
}

void gr_opengl_unfilled_circle(int xc, int yc, int d, int resize_mode)
{
	int r = d / 2;
	int segments = 4 + (int)(r); // seems like a good approximation
	float theta = 2 * PI / float(segments - 1);
	float c = cosf(theta);
	float s = sinf(theta);
	float t;

	float x1 = 1.0f;
	float y1 = 0.0f;
	float x2 = x1;
	float y2 = y1;

	float linewidth;
	glGetFloatv(GL_LINE_WIDTH, &linewidth);

	float halflinewidth = linewidth / 2.0f;
	float inner_rad = r - halflinewidth;
	float outer_rad = r + halflinewidth;

	int do_resize = 0;

	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		gr_resize_screen_pos(&xc, &yc, NULL, NULL, resize_mode);
		do_resize = 1;
	}

	// Big clip
	if ( (xc+outer_rad) < gr_screen.clip_left ) {
		return;
	}

	if ( (xc-outer_rad) > gr_screen.clip_right ) {
		return;
	}

	if ( (yc+outer_rad) < gr_screen.clip_top ) {
		return;
	}

	if ( (yc-outer_rad) > gr_screen.clip_bottom ) {
		return;
	}

	int offset_x = ((do_resize) ? gr_screen.offset_x_unscaled : gr_screen.offset_x);
	int offset_y = ((do_resize) ? gr_screen.offset_y_unscaled : gr_screen.offset_y);

	GL_state.SetTextureSource(TEXTURE_SOURCE_NONE);
	GL_state.SetAlphaBlendMode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	GL_state.SetZbufferType(ZBUFFER_TYPE_NONE);

	GLfloat *circle = new GLfloat[segments * 4];

	for (int i=0; i < segments * 4; i+=4) {
		circle[i] = i2fl(xc + (x2 * outer_rad) + offset_x);
		circle[i+1] = i2fl(yc + (y2 * outer_rad) + offset_y);

		circle[i+2] = i2fl(xc + (x2 * inner_rad) + offset_x);
		circle[i+3] = i2fl(yc + (y2 * inner_rad) + offset_y);

		t = x2;
		x2 = c * x1 - s * y1;
		y2 = s * t + c * y1;

		x1 = x2;
		y1 = y2;
	}

	gr_opengl_set_2d_matrix();

	GL_state.Color(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION2, 0, circle);

	opengl_bind_vertex_layout(vert_def);

	glDrawArrays(GL_QUAD_STRIP, 0, segments * 2);

	GL_CHECK_FOR_ERRORS("end of opengl_unfilled_circle()");

	gr_opengl_end_2d_matrix();

	delete [] circle;
}

void gr_opengl_arc(int xc, int yc, float r, float angle_start, float angle_end, bool fill, int resize_mode)
{
	// Ensure that angle_start < angle_end
	if (angle_end < angle_start) {
		float temp = angle_start;
		angle_start = angle_end;
		angle_end = temp;
	}

	float arc_length_ratio;
	arc_length_ratio = MIN(angle_end - angle_start, 360.0f) / 360.0f;

	int segments = 4 + (int)(r * arc_length_ratio); // seems like a good approximation
	float theta = 2 * PI / float(segments - 1) * arc_length_ratio;
	float c = cosf(theta);
	float s = sinf(theta);
	float t;

	float x1 = cosf(ANG_TO_RAD(angle_start));
	float y1 = sinf(ANG_TO_RAD(angle_start));
	float x2 = x1;
	float y2 = y1;

	float halflinewidth = 0.0f;
	float inner_rad = 0.0f; // only used if fill==false
	float outer_rad = r;

	if (!fill) {
		float linewidth;
		glGetFloatv(GL_LINE_WIDTH, &linewidth);

		halflinewidth = linewidth / 2.0f;
		inner_rad = r - halflinewidth;
		outer_rad = r + halflinewidth;
	}

	int do_resize = 0;

	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		gr_resize_screen_pos(&xc, &yc, NULL, NULL, resize_mode);
		do_resize = 1;
	}

	// Big clip
	if ( (xc+outer_rad) < gr_screen.clip_left ) {
		return;
	}

	if ( (xc-outer_rad) > gr_screen.clip_right ) {
		return;
	}

	if ( (yc+outer_rad) < gr_screen.clip_top ) {
		return;
	}

	if ( (yc-outer_rad) > gr_screen.clip_bottom ) {
		return;
	}

	int offset_x = ((do_resize) ? gr_screen.offset_x_unscaled : gr_screen.offset_x);
	int offset_y = ((do_resize) ? gr_screen.offset_y_unscaled : gr_screen.offset_y);

	GL_state.SetTextureSource(TEXTURE_SOURCE_NONE);
	GL_state.SetAlphaBlendMode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	GL_state.SetZbufferType(ZBUFFER_TYPE_NONE);

	GLfloat *arc;

	gr_opengl_set_2d_matrix();
	GL_state.Color(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha);

	if (fill) {
		arc = new GLfloat[segments * 2 + 2];

		arc[0] = i2fl(xc);
		arc[1] = i2fl(yc);

		for (int i=2; i < segments * 2 + 2; i+=2) {
			arc[i] = i2fl(xc + (x2 * outer_rad) + offset_x);
			arc[i+1] = i2fl(yc + (y2 * outer_rad) + offset_y);

			t = x2;
			x2 = c * x1 - s * y1;
			y2 = s * t + c * y1;

			x1 = x2;
			y1 = y2;
		}

		vertex_layout vert_def;
		vert_def.add_vertex_component(vertex_format_data::POSITION2, 0, arc);
		opengl_bind_vertex_layout(vert_def);

		glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 1);
	} else {
		arc = new GLfloat[segments * 4];

		for (int i=0; i < segments * 4; i+=4) {
			arc[i] = i2fl(xc + (x2 * outer_rad) + offset_x);
			arc[i+1] = i2fl(yc + (y2 * outer_rad) + offset_y);

			arc[i+2] = i2fl(xc + (x2 * inner_rad) + offset_x);
			arc[i+3] = i2fl(yc + (y2 * inner_rad) + offset_y);

			t = x2;
			x2 = c * x1 - s * y1;
			y2 = s * t + c * y1;

			x1 = x2;
			y1 = y2;
		}

		vertex_layout vert_def;
		vert_def.add_vertex_component(vertex_format_data::POSITION2, 0, arc);
		opengl_bind_vertex_layout(vert_def);

		glDrawArrays(GL_QUAD_STRIP, 0, segments * 2);
	}

	GL_CHECK_FOR_ERRORS("end of opengl_arc()");

	gr_opengl_end_2d_matrix();

	delete [] arc;
}

void gr_opengl_curve(int xc, int yc, int r, int direction, int resize_mode)
{
	int a, b, p;

	if (resize_mode != GR_RESIZE_NONE) {
		gr_resize_screen_pos(&xc, &yc, NULL, NULL, resize_mode);
	}

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
				gr_opengl_line(xc - b + 1, yc - a, xc - b, yc - a, GR_RESIZE_NONE);

				if (p < 0) {
					p += (a << 2) + 6;
				} else {
					// Draw the second octant
					gr_opengl_line(xc - a + 1, yc - b, xc - a, yc - b, GR_RESIZE_NONE);
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
				gr_opengl_line(xc + b - 1, yc - a, xc + b, yc - a, GR_RESIZE_NONE);

				if (p < 0) {
					p += (a << 2) + 6;
				} else {
					// Draw the second octant
					gr_opengl_line(xc + a - 1, yc - b, xc + a, yc - b, GR_RESIZE_NONE);
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
				gr_opengl_line(xc - b + 1, yc + a, xc - b, yc + a, GR_RESIZE_NONE);

				if (p < 0) {
					p += (a << 2) + 6;
				} else {
					// Draw the second octant
					gr_opengl_line(xc - a + 1, yc + b, xc - a, yc + b, GR_RESIZE_NONE);
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
				gr_opengl_line(xc + b - 1, yc + a, xc + b, yc + a, GR_RESIZE_NONE);

				if (p < 0) {
					p += (a << 2) + 6;
				} else {
					// Draw the second octant
					gr_opengl_line(xc + a - 1, yc + b, xc + a, yc + b, GR_RESIZE_NONE);
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
	struct v6 *vertPos = (struct v6*) vm_malloc(sizeof(struct v6) * nv);
	struct c4 *vertCol = (struct c4*) vm_malloc(sizeof(struct c4) * nv);

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
		GL_state.Color( (ubyte)r, (ubyte)g, (ubyte)b, alpha );
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

	GL_state.Array.BindArrayBuffer(0);

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION4, sizeof(struct v6), &vertPos[0].x);

	if(flags & TMAP_FLAG_TEXTURED) {
		vert_def.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(struct v6), &vertPos[0].u);
		//vert_def.add_vertex_component(vertex_format_data::TEX_COORD1, sizeof(struct v6), &vertPos[0].u);
	}

	if(flags & (TMAP_FLAG_NEBULA | TMAP_FLAG_GOURAUD)) {
		vert_def.add_vertex_component(vertex_format_data::COLOR4, 0, &vertCol[0].r);
	}

	opengl_bind_vertex_layout(vert_def);

	glDrawArrays(gl_mode, 0, nv);

	GL_CHECK_FOR_ERRORS("start of draw_primitive()");

	vm_free(vertPos);
	vm_free(vertCol);
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

	opengl_shader_set_current();

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
		GL_state.Color( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)alpha );
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
			colour.push_back((ubyte) alpha);
		}

		uvcoords.push_back(va->texture_position.u);
		uvcoords.push_back(va->texture_position.v);

		vertvec.push_back(va->world.xyz.x);
		vertvec.push_back(va->world.xyz.y);
		vertvec.push_back(va->world.xyz.z);
	}

	GL_state.Array.BindArrayBuffer(0);

	vertex_layout vert_def;

	if (isRGB) {
		vert_def.add_vertex_component(vertex_format_data::COLOR4, 0, &colour.front());
	}

	vert_def.add_vertex_component(vertex_format_data::POSITION3, 0, &vertvec.front());
	vert_def.add_vertex_component(vertex_format_data::TEX_COORD, 0, &uvcoords.front());

	opengl_bind_vertex_layout(vert_def);

	glDrawArrays(gl_mode, 0, nv);

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

	GL_state.Array.BindArrayBuffer(0);

	vertex_layout vert_def;

	if (flags & TMAP_FLAG_TEXTURED) {
		if ( !gr_opengl_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale) ) {
			return;
		}

		vert_def.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), &verts[0].texture_position.u);

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
		if (flags & TMAP_FLAG_ALPHA) {
			vert_def.add_vertex_component(vertex_format_data::COLOR4, sizeof(vertex), &verts[0].r);
		} else {
			GL_state.Color( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)alpha );
			vert_def.add_vertex_component(vertex_format_data::COLOR3, sizeof(vertex), &verts[0].r);
		}
	}
	// use what opengl_setup_render_states() gives us since this works much better for nebula and transparency
	else {
		GL_state.Color( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)alpha );
	}

	float offset_z = -0.99f;

	glPushMatrix();
	glTranslatef((float)gr_screen.offset_x, (float)gr_screen.offset_y, offset_z);

	vert_def.add_vertex_component(vertex_format_data::POSITION2, sizeof(vertex), &verts[0].screen.xyw.x);

	opengl_bind_vertex_layout(vert_def);

	gr_opengl_set_2d_matrix();

	glDrawArrays(gl_mode, 0, nverts);

	gr_opengl_end_2d_matrix();

	GL_state.Texture.DisableAll();

	if (texture_matrix_set) {
		glMatrixMode(GL_TEXTURE);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}

	glPopMatrix();

	GL_CHECK_FOR_ERRORS("end of render()");
}

void opengl_render_internal3d(int nverts, vertex *verts, uint flags)
{
	int alpha, tmap_type, r, g, b;
	float u_scale = 1.0f, v_scale = 1.0f;
	GLenum gl_mode = GL_TRIANGLE_FAN;

	GL_CHECK_FOR_ERRORS("start of render3d()");

	opengl_setup_render_states(r, g, b, alpha, tmap_type, flags);

	GL_state.Array.BindArrayBuffer(0);

	vertex_layout vert_def;

	if (flags & TMAP_FLAG_TEXTURED) {
		if ( !gr_opengl_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale) ) {
			return;
		}

		vert_def.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), &verts[0].texture_position.u);
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
	} else if (flags & TMAP_FLAG_LINES) {
		gl_mode = GL_LINES;
	}

	if ( (flags & TMAP_FLAG_RGB) && (flags & TMAP_FLAG_GOURAUD) ) {
		vert_def.add_vertex_component(vertex_format_data::COLOR4, sizeof(vertex), &verts[0].r);
	}
	// use what opengl_setup_render_states() gives us since this works much better for nebula and transparency
	else {
		GL_state.Color( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)alpha );
	}

	vert_def.add_vertex_component(vertex_format_data::POSITION3, sizeof(vertex), &verts[0].world.xyz.x);

	opengl_bind_vertex_layout(vert_def);

	glDrawArrays(gl_mode, 0, nverts);

	GL_state.CullFace(cull_face);
	GL_state.Lighting(lighting);

	GL_CHECK_FOR_ERRORS("end of render3d()");
}

//Used by the effects batcher to determine whether to use shaders or not
bool Use_Shaders_for_effect_rendering = true;

void gr_opengl_render_effect(int nverts, vertex *verts, float *radius_list, uint flags)
{
	int alpha, tmap_type, r, g, b;
	float u_scale = 1.0f, v_scale = 1.0f;
	GLenum gl_mode = GL_TRIANGLE_FAN;
	int zbuff = ZBUFFER_TYPE_DEFAULT;
	GL_CHECK_FOR_ERRORS("start of render3d()");

	Assert(Scene_depth_texture != 0);

	opengl_setup_render_states(r, g, b, alpha, tmap_type, flags);

	GL_state.Array.BindArrayBuffer(0);

	vertex_layout vert_def;

	if ( flags & TMAP_FLAG_TEXTURED ) {
		if ( flags & TMAP_FLAG_SOFT_QUAD ) {
			if( (flags & TMAP_FLAG_DISTORTION) || (flags & TMAP_FLAG_DISTORTION_THRUSTER) )
			{
				glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

				opengl_tnl_set_material_distortion(flags);

				zbuff = gr_zbuffer_set(GR_ZBUFF_READ);
			}
			else
			{
				opengl_tnl_set_material_soft_particle(flags);
				zbuff = gr_zbuffer_set(GR_ZBUFF_NONE);
			}

			vert_def.add_vertex_component(vertex_format_data::RADIUS, 0, radius_list);
		}

		if ( !gr_opengl_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale) ) {
			return;
		}

		vert_def.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(vertex), &verts[0].texture_position.u);
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
		vert_def.add_vertex_component(vertex_format_data::COLOR4, sizeof(vertex), &verts[0].r);
	}
	// use what opengl_setup_render_states() gives us since this works much better for nebula and transparency
	else {
		GL_state.Color( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)alpha );
	}

	vert_def.add_vertex_component(vertex_format_data::POSITION3, sizeof(vertex), &verts[0].world.xyz.x);

	opengl_bind_vertex_layout(vert_def);

	glDrawArrays(gl_mode, 0, nverts);

	opengl_shader_set_current();

	GL_state.Texture.DisableAll();

	GL_state.CullFace(cull_face);
	GL_state.Lighting(lighting);
	gr_zbuffer_set(zbuff);

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
	v[0].screen.xyw.x = clipped_x0;
	v[0].screen.xyw.y = clipped_y0;
	v[0].screen.xyw.w = va->screen.xyw.w;
	v[0].world.xyz.z = va->world.xyz.z;
	v[0].texture_position.u = clipped_u0;
	v[0].texture_position.v = clipped_v0;
	v[0].spec_r = 0;
	v[0].spec_g = 0;
	v[0].spec_b = 0;

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
void gr_opengl_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct, int resize_mode)
{
   	gr_set_bitmap(bmap1, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f - pct);
	gr_bitmap(x1, y1, resize_mode);

  	gr_set_bitmap(bmap2, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, pct);
	gr_bitmap(x2, y2, resize_mode);
}

void gr_opengl_shade(int x, int y, int w, int h, int resize_mode)
{
	if (resize_mode != GR_RESIZE_NONE) {
		gr_resize_screen_pos(&x, &y, &w, &h, resize_mode);
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

	GL_state.Color( (GLubyte)gr_screen.current_shader.r, (GLubyte)gr_screen.current_shader.g,
				(GLubyte)gr_screen.current_shader.b, (GLubyte)gr_screen.current_shader.c );

	opengl_draw_coloured_quad((GLint)x1, (GLint)y1, (GLint)x2, (GLint)y2);

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

	GL_state.Color( (GLubyte)r, (GLubyte)g, (GLubyte)b, 255 );

	opengl_draw_coloured_quad((GLint)x1, (GLint)y1, (GLint)x2, (GLint)y2);
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

	GL_state.Color( (GLubyte)r, (GLubyte)g, (GLubyte)b, (GLubyte)a );

	opengl_draw_coloured_quad((GLint)x1, (GLint)y1, (GLint)x2, (GLint)y2);
}


void gr_opengl_fade_in(int instantaneous)
{
	// Empty - DDOI
}

void gr_opengl_fade_out(int instantaneous)
{
	// Empty - DDOI
}

void opengl_bitmap_ex_internal(int x, int y, int w, int h, int sx, int sy, int resize_mode)
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

	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
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
		gr_resize_screen_posf(&x1, &y1, NULL, NULL, resize_mode);
		gr_resize_screen_posf(&x2, &y2, NULL, NULL, resize_mode);
	}

	GL_state.Color(255, 255, 255, (GLubyte)(gr_screen.current_alpha * 255));

	opengl_draw_textured_quad(x1, y1, u0, v0, x2, y2, u1, v1);
}


//these are penguins bitmap functions
void gr_opengl_bitmap_ex(int x, int y, int w, int h, int sx, int sy, int resize_mode)
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

	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
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
			sy += clip_top-dy1;
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
	opengl_bitmap_ex_internal(dx1, dy1, (dx2 - dx1 + 1), (dy2 - dy1 + 1), sx, sy, resize_mode);
}

/*void gr_opengl_bitmap(int x, int y, int resize_mode)
{
	int w, h, do_resize;

	bm_get_info( gr_screen.current_bitmap, &w, &h, NULL );

	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
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
	gr_opengl_bitmap_ex(dx1, dy1, dx2-dx1+1, dy2-dy1+1, sx, sy, resize_mode);
}*/

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
	opengl_shader_set_current();

	GL_state.Color(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue);

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

void gr_opengl_deferred_light_sphere_init(int rings, int segments) // Generate a VBO of a sphere of radius 1.0f, based on code at http://www.ogre3d.org/tikiwiki/ManualSphereMeshes
{
	unsigned int nVertex = (rings + 1) * (segments+1) * 3;
	unsigned int nIndex = deferred_light_sphere_icount = 6 * rings * (segments + 1);
	float *Vertices = (float*)vm_malloc(sizeof(float) * nVertex);
	float *pVertex = Vertices;
	ushort *Indices = (ushort*)vm_malloc(sizeof(ushort) * nIndex);
	ushort *pIndex = Indices;

	float fDeltaRingAngle = (PI / rings);
	float fDeltaSegAngle = (2.0f * PI / segments);
	unsigned short wVerticeIndex = 0 ;

	// Generate the group of rings for the sphere
	for( int ring = 0; ring <= rings; ring++ ) {
		float r0 = sinf (ring * fDeltaRingAngle);
		float y0 = cosf (ring * fDeltaRingAngle);

		// Generate the group of segments for the current ring
		for(int seg = 0; seg <= segments; seg++) {
			float x0 = r0 * sinf(seg * fDeltaSegAngle);
			float z0 = r0 * cosf(seg * fDeltaSegAngle);

			// Add one vertex to the strip which makes up the sphere
			*pVertex++ = x0;
			*pVertex++ = y0;
			*pVertex++ = z0;

			if (ring != rings) {
				// each vertex (except the last) has six indices pointing to it
				*pIndex++ = wVerticeIndex + (ushort)segments + 1;
				*pIndex++ = wVerticeIndex;
				*pIndex++ = wVerticeIndex + (ushort)segments;
				*pIndex++ = wVerticeIndex + (ushort)segments + 1;
				*pIndex++ = wVerticeIndex + 1;
				*pIndex++ = wVerticeIndex;
				wVerticeIndex ++;
			}
		}; // end for seg
	} // end for ring

	deferred_light_sphere_vcount = wVerticeIndex;

	glGetError();

	vglGenBuffersARB(1, &deferred_light_sphere_vbo);

	// make sure we have one
	if (deferred_light_sphere_vbo) {
		vglBindBufferARB(GL_ARRAY_BUFFER_ARB, deferred_light_sphere_vbo);
		vglBufferDataARB(GL_ARRAY_BUFFER_ARB, nVertex * sizeof(float), Vertices, GL_STATIC_DRAW_ARB);

		// just in case
		if ( opengl_check_for_errors() ) {
			vglDeleteBuffersARB(1, &deferred_light_sphere_vbo);
			deferred_light_sphere_vbo = 0;
			return;
		}

		vglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

		vm_free(Vertices);
		Vertices = NULL;
	}

	vglGenBuffersARB(1, &deferred_light_sphere_ibo);

	// make sure we have one
	if (deferred_light_sphere_ibo) {
		vglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, deferred_light_sphere_ibo);
		vglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, nIndex * sizeof(ushort), Indices, GL_STATIC_DRAW_ARB);

		// just in case
		if ( opengl_check_for_errors() ) {
			vglDeleteBuffersARB(1, &deferred_light_sphere_ibo);
			deferred_light_sphere_ibo = 0;
			return;
		}

		vglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

		vm_free(Indices);
		Indices = NULL;
	}

}

void gr_opengl_draw_deferred_light_sphere(vec3d *position, float rad, bool clearStencil = true)
{
	if (Cmdline_nohtl) {
		return;
	}

	g3_start_instance_matrix(position, &vmd_identity_matrix, true);

	GL_state.Uniform.setUniform3f("scale", rad, rad, rad);

	GL_state.Array.BindArrayBuffer(deferred_light_sphere_vbo);
	GL_state.Array.BindElementBuffer(deferred_light_sphere_ibo);

	vertex_layout vertex_declare;

	vertex_declare.add_vertex_component(vertex_format_data::POSITION3, 0, 0);

	opengl_bind_vertex_layout(vertex_declare);

	vglDrawRangeElements(GL_TRIANGLES, 0, deferred_light_sphere_vcount, deferred_light_sphere_icount, GL_UNSIGNED_SHORT, 0);

	g3_done_instance(true);
}

void gr_opengl_deferred_light_cylinder_init(int segments) // Generate a VBO of a cylinder of radius and height 1.0f, based on code at http://www.ogre3d.org/tikiwiki/ManualSphereMeshes
{
	unsigned int nVertex = (segments + 1) * 2 * 3 + 6; // Can someone verify this?
	unsigned int nIndex = deferred_light_cylinder_icount = 12 * (segments + 1) - 6; //This too
	float *Vertices = (float*)vm_malloc(sizeof(float) * nVertex);
	float *pVertex = Vertices;
	ushort *Indices = (ushort*)vm_malloc(sizeof(ushort) * nIndex);
	ushort *pIndex = Indices;

	float fDeltaSegAngle = (2.0f * PI / segments);
	unsigned short wVerticeIndex = 0 ;

	*pVertex++ = 0.0f;
	*pVertex++ = 0.0f;
	*pVertex++ = 0.0f;
	wVerticeIndex ++;
	*pVertex++ = 0.0f;
	*pVertex++ = 0.0f;
	*pVertex++ = 1.0f;
	wVerticeIndex ++;

	for( int ring = 0; ring <= 1; ring++ ) {
		float z0 = (float)ring;

		// Generate the group of segments for the current ring
		for(int seg = 0; seg <= segments; seg++) {
			float x0 = sinf(seg * fDeltaSegAngle);
			float y0 = cosf(seg * fDeltaSegAngle);

			// Add one vertex to the strip which makes up the cylinder
			*pVertex++ = x0;
			*pVertex++ = y0;
			*pVertex++ = z0;

			if (!ring) {
				*pIndex++ = wVerticeIndex + (ushort)segments + 1;
				*pIndex++ = wVerticeIndex;
				*pIndex++ = wVerticeIndex + (ushort)segments;
				*pIndex++ = wVerticeIndex + (ushort)segments + 1;
				*pIndex++ = wVerticeIndex + 1;
				*pIndex++ = wVerticeIndex;
				if(seg != segments)
				{
					*pIndex++ = wVerticeIndex + 1;
					*pIndex++ = wVerticeIndex;
					*pIndex++ = 0;
				}
				wVerticeIndex ++;
			}
			else
			{
				if(seg != segments)
				{
					*pIndex++ = wVerticeIndex + 1;
					*pIndex++ = wVerticeIndex;
					*pIndex++ = 1;
					wVerticeIndex ++;
				}
			}
		}; // end for seg
	} // end for ring

	deferred_light_cylinder_vcount = wVerticeIndex;

	glGetError();

	vglGenBuffersARB(1, &deferred_light_cylinder_vbo);

	// make sure we have one
	if (deferred_light_cylinder_vbo) {
		vglBindBufferARB(GL_ARRAY_BUFFER_ARB, deferred_light_cylinder_vbo);
		vglBufferDataARB(GL_ARRAY_BUFFER_ARB, nVertex * sizeof(float), Vertices, GL_STATIC_DRAW_ARB);

		// just in case
		if ( opengl_check_for_errors() ) {
			vglDeleteBuffersARB(1, &deferred_light_cylinder_vbo);
			deferred_light_cylinder_vbo = 0;
			return;
		}

		vglBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

		vm_free(Vertices);
		Vertices = NULL;
	}

	vglGenBuffersARB(1, &deferred_light_cylinder_ibo);

	// make sure we have one
	if (deferred_light_cylinder_ibo) {
		vglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, deferred_light_cylinder_ibo);
		vglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, nIndex * sizeof(ushort), Indices, GL_STATIC_DRAW_ARB);

		// just in case
		if ( opengl_check_for_errors() ) {
			vglDeleteBuffersARB(1, &deferred_light_cylinder_ibo);
			deferred_light_cylinder_ibo = 0;
			return;
		}

		vglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);

		vm_free(Indices);
		Indices = NULL;
	}

}

void gr_opengl_draw_deferred_light_cylinder(vec3d *position,matrix *orient, float rad, float length, bool clearStencil = true)
{
	if (Cmdline_nohtl) {
		return;
	}

	g3_start_instance_matrix(position, orient, true);

	GL_state.Uniform.setUniform3f("scale", rad, rad, length);

	GL_state.Array.BindArrayBuffer(deferred_light_cylinder_vbo);
	GL_state.Array.BindElementBuffer(deferred_light_cylinder_ibo);

	vertex_layout vertex_declare;

	vertex_declare.add_vertex_component(vertex_format_data::POSITION3, 0, 0);

	opengl_bind_vertex_layout(vertex_declare);

	vglDrawRangeElements(GL_TRIANGLES, 0, deferred_light_cylinder_vcount, deferred_light_cylinder_icount, GL_UNSIGNED_SHORT, 0);

	g3_done_instance(true);
}

void gr_opengl_draw_line_list(const colored_vector *lines, int num)
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
		Cmdline_fb_explosions = 0;

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

	// setup position render texture
	glGenTextures(1, &Scene_position_texture);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_position_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, Scene_texture_width, Scene_texture_height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

	vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, Scene_position_texture, 0);

	// setup normal render texture
	glGenTextures(1, &Scene_normal_texture);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_normal_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, Scene_texture_width, Scene_texture_height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

	vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, Scene_normal_texture, 0);

	// setup specular render texture
	glGenTextures(1, &Scene_specular_texture);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_specular_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Scene_texture_width, Scene_texture_height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

	vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_2D, Scene_specular_texture, 0);

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

	vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT4_EXT, GL_TEXTURE_2D, Scene_effect_texture, 0);

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

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, Scene_texture_width, Scene_texture_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
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
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, Scene_texture_width, Scene_texture_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Scene_depth_texture, 0);

	//setup main stencil buffer
	vglGenRenderbuffersEXT(1, &Scene_stencil_buffer);
    vglBindRenderbufferEXT(GL_RENDERBUFFER, Scene_stencil_buffer);
    vglRenderbufferStorageEXT(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_EXT, Scene_texture_width, Scene_texture_height);
	//vglFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, Scene_stencil_buffer);

	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);

	if ( opengl_check_framebuffer() ) {
		vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		vglDeleteFramebuffersEXT(1, &Scene_framebuffer);
		Scene_framebuffer = 0;

		GL_state.Texture.Disable();

		glDeleteTextures(1, &Scene_color_texture);
		Scene_color_texture = 0;

		glDeleteTextures(1, &Scene_position_texture);
		Scene_position_texture = 0;

		glDeleteTextures(1, &Scene_normal_texture);
		Scene_normal_texture = 0;

		glDeleteTextures(1, &Scene_specular_texture);
		Scene_specular_texture = 0;

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
		Distortion_texture[0] = 0;
		Distortion_texture[1] = 0;
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

	if ( Scene_position_texture ) {
		glDeleteTextures(1, &Scene_position_texture);
		Scene_position_texture = 0;
	}

	if ( Scene_normal_texture ) {
		glDeleteTextures(1, &Scene_normal_texture);
		Scene_normal_texture = 0;

	}

	if ( Scene_specular_texture ) {
		glDeleteTextures(1, &Scene_specular_texture);
		Scene_specular_texture = 0;
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

	glDeleteTextures(2, Distortion_texture);
	Distortion_texture[0] = 0;
	Distortion_texture[1] = 0;

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

	if ( Cmdline_no_deferred_lighting ) {
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	} else {
		GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT };
		vglDrawBuffers(4, buffers);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		opengl_clear_deferred_buffers();

		glDrawBuffer(GL_COLOR_ATTACHMENT0);
	}

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
		GLboolean lighting = GL_state.Lighting(GL_FALSE);
		GLboolean blend = GL_state.Blend(GL_FALSE);
		GLboolean cull = GL_state.CullFace(GL_FALSE);

		vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, opengl_get_rtt_framebuffer());

		GL_state.Texture.SetActiveUnit(0);
		GL_state.Texture.SetTarget(GL_TEXTURE_2D);
		GL_state.Texture.Enable(Scene_color_texture);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		GL_state.Color(255, 255, 255, 255);

		GL_state.Array.BindArrayBuffer(0);

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

			vertex_layout vert_def;

			vert_def.add_vertex_component(vertex_format_data::POSITION2, 0, vertices);
			vert_def.add_vertex_component(vertex_format_data::TEX_COORD, 0, uvcoords);

			opengl_bind_vertex_layout(vert_def);

			glDrawArrays(GL_QUADS, 0, 4);
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

			vertex_layout vert_def;

			vert_def.add_vertex_component(vertex_format_data::POSITION2, 0, vertices);
			vert_def.add_vertex_component(vertex_format_data::TEX_COORD, 0, uvcoords);

			opengl_bind_vertex_layout(vert_def);

			glDrawArrays(GL_QUADS, 0, 4);
		}

		GL_state.Texture.SetActiveUnit(0);
		GL_state.Texture.Disable();

		// reset state
		GL_state.DepthTest(depth);
		GL_state.DepthMask(depth_mask);
		GL_state.Lighting(lighting);
		GL_state.Blend(blend);
		GL_state.CullFace(cull);
	}

	// Reset the UV scale values

	Scene_texture_u_scale = 1.0f;
	Scene_texture_v_scale = 1.0f;

	Scene_framebuffer_in_frame = false;
}

void gr_opengl_copy_effect_texture()
{
	if ( !Scene_framebuffer_in_frame ) {
		return;
	}

	glDrawBuffer(GL_COLOR_ATTACHMENT4_EXT);
	vglBlitFramebufferEXT(0, 0, gr_screen.max_w, gr_screen.max_h, 0, 0, gr_screen.max_w, gr_screen.max_h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
}

void opengl_clear_deferred_buffers()
{
	GLboolean depth = GL_state.DepthTest(GL_FALSE);
	GLboolean depth_mask = GL_state.DepthMask(GL_FALSE);
	GLboolean lighting = GL_state.Lighting(GL_FALSE);
	GLboolean blend = GL_state.Blend(GL_FALSE);
	GLboolean cull = GL_state.CullFace(GL_FALSE);

	opengl_shader_set_current( gr_opengl_maybe_create_shader(SDR_TYPE_DEFERRED_CLEAR, 0) );

	opengl_draw_textured_quad(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);

	opengl_shader_set_current();

	GL_state.DepthTest(depth);
	GL_state.DepthMask(depth_mask);
	GL_state.Lighting(lighting);
	GL_state.Blend(blend);
	GL_state.CullFace(cull);
}

void gr_opengl_deferred_lighting_begin()
{
	if (Use_GLSL < 2 || Cmdline_no_deferred_lighting)
		return;

	Deferred_lighting = true;

	GLenum buffers[] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT };
	vglDrawBuffers(4, buffers);
}

void gr_opengl_deferred_lighting_end()
{
	if(!Deferred_lighting)
		return;
	Deferred_lighting = false;
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
}

extern light Lights[MAX_LIGHTS];
extern int Num_lights;
extern float GL_light_color[];
extern float static_point_factor;
extern float static_light_factor;
extern float static_tube_factor;

void gr_opengl_deferred_lighting_finish()
{
	if ( Use_GLSL < 2 || Cmdline_no_deferred_lighting ) {
		return;
	}

	GL_state.SetAlphaBlendMode( ALPHA_BLEND_ADDITIVE);
	gr_zbuffer_set(GR_ZBUFF_NONE);

	//GL_state.DepthFunc(GL_GREATER);
	//GL_state.DepthMask(GL_FALSE);

	opengl_shader_set_current( gr_opengl_maybe_create_shader(SDR_TYPE_DEFERRED_LIGHTING, 0) );

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_color_texture);

	GL_state.Texture.SetActiveUnit(1);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_normal_texture);

	GL_state.Texture.SetActiveUnit(2);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_position_texture);

	GL_state.Texture.SetActiveUnit(3);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_specular_texture);

	vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Scene_luminance_texture, 0);
	vglFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, Scene_stencil_buffer);
	vglFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, Scene_stencil_buffer);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	light lights_copy[MAX_LIGHTS];
	memcpy(lights_copy, Lights, MAX_LIGHTS * sizeof(light));

	std::sort(lights_copy, lights_copy+Num_lights, light_compare_by_type);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
	glStencilMask(0xFF);

	for(int i = 0; i < Num_lights; ++i)
	{
		light *l = &lights_copy[i];
		GL_state.Uniform.setUniformi( "lightType", 0 );
		switch(l->type)
		{
			case LT_CONE:
				GL_state.Uniform.setUniformi( "lightType", 2 );
				GL_state.Uniform.setUniformi( "dualCone", l->dual_cone );
				GL_state.Uniform.setUniformf( "coneAngle", l->cone_angle );
				GL_state.Uniform.setUniformf( "coneInnerAngle", l->cone_inner_angle );
				GL_state.Uniform.setUniform3f( "coneDir", l->vec2.xyz.x, l->vec2.xyz.y, l->vec2.xyz.z);
			case LT_POINT:
				GL_state.Uniform.setUniform3f( "diffuseLightColor", l->r * l->intensity * static_point_factor, l->g * l->intensity * static_point_factor, l->b * l->intensity * static_point_factor );
				GL_state.Uniform.setUniform3f( "specLightColor", l->spec_r * l->intensity * static_point_factor, l->spec_g * l->intensity * static_point_factor, l->spec_b * l->intensity * static_point_factor );
				GL_state.Uniform.setUniformf( "lightRadius", MAX(l->rada, l->radb) * 1.25f );

				/*float dist;
				vec3d a;

				vm_vec_sub(&a, &Eye_position, &l->vec);
				dist = vm_vec_mag(&a);*/

				gr_opengl_draw_deferred_light_sphere(&l->vec, MAX(l->rada, l->radb) * 1.28f);
				break;
			case LT_TUBE:
				GL_state.Uniform.setUniform3f( "diffuseLightColor", l->r * l->intensity * static_tube_factor, l->g * l->intensity * static_tube_factor, l->b * l->intensity * static_tube_factor );
				GL_state.Uniform.setUniform3f( "specLightColor", l->spec_r * l->intensity * static_tube_factor, l->spec_g * l->intensity * static_tube_factor, l->spec_b * l->intensity * static_tube_factor );
				GL_state.Uniform.setUniformf( "lightRadius", l->radb * 1.5f );
				GL_state.Uniform.setUniformi( "lightType", 1 );

				vec3d a, b;
				matrix orient;
				float length, dist;

				vm_vec_sub(&a, &l->vec, &l->vec2);
				vm_vector_2_matrix(&orient, &a, NULL, NULL);
				length = vm_vec_mag(&a);
				int pos = vm_vec_dist_to_line(&Eye_position, &l->vec, &l->vec2, &b, &dist);
				if(pos == -1)
				{
					vm_vec_sub(&a, &Eye_position, &l->vec);
					dist = vm_vec_mag(&a);
				}
				else if (pos == 1)
				{
					vm_vec_sub(&a, &Eye_position, &l->vec2);
					dist = vm_vec_mag(&a);
				}

				glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
				gr_opengl_draw_deferred_light_cylinder(&l->vec2, &orient, l->radb * 1.53f, length);
				GL_state.Uniform.setUniformi( "lightType", 0 );
				gr_opengl_draw_deferred_light_sphere(&l->vec, l->radb * 1.53f, false);
				gr_opengl_draw_deferred_light_sphere(&l->vec2, l->radb * 1.53f, false);
				glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
				break;
		}
	}

	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	glClear(GL_STENCIL_BUFFER_BIT);
	glDisable(GL_STENCIL_TEST);

	vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Scene_color_texture, 0);
	vglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Scene_depth_texture, 0);
	vglFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);

	gr_end_view_matrix();
	gr_end_proj_matrix();

	GLboolean depth = GL_state.DepthTest(GL_FALSE);
	GLboolean depth_mask = GL_state.DepthMask(GL_FALSE);
	GLboolean lighting = GL_state.Lighting(GL_FALSE);
	GLboolean blend = GL_state.Blend(GL_FALSE);
	GLboolean cull = GL_state.CullFace(GL_FALSE);

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

	opengl_shader_set_current();

	GL_state.Array.BindArrayBuffer(0);
	GL_state.Array.BindElementBuffer(0);

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION2, 0, vertices);
	vert_def.add_vertex_component(vertex_format_data::TEX_COORD, 0, uvcoords);

	opengl_bind_vertex_layout(vert_def);

	GL_state.Texture.DisableAll();

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(Scene_luminance_texture);

	GL_state.SetAlphaBlendMode( ALPHA_BLEND_ADDITIVE );

	glDrawArrays(GL_QUADS, 0, 4);

	gr_set_proj_matrix(Proj_fov, gr_screen.clip_aspect, Min_draw_distance, Max_draw_distance);
	gr_set_view_matrix(&Eye_position, &Eye_matrix);

	GL_state.Texture.DisableAll();

	// reset state
	GL_state.DepthTest(depth);
	GL_state.DepthMask(depth_mask);
	GL_state.Lighting(lighting);
	GL_state.Blend(blend);
	GL_state.CullFace(cull);

	GL_state.SetAlphaBlendMode( ALPHA_BLEND_NONE );

	gr_clear_states();
}

void gr_opengl_update_distortion()
{
	GLboolean depth = GL_state.DepthTest(GL_FALSE);
	GLboolean depth_mask = GL_state.DepthMask(GL_FALSE);
	GLboolean lighting = GL_state.Lighting(GL_FALSE);
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
	GL_state.Color(255, 255, 255, 255);

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

	GL_state.Array.BindArrayBuffer(0);

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION2, 0, vertices);
	vert_def.add_vertex_component(vertex_format_data::TEX_COORD, 0, texcoord);

	opengl_bind_vertex_layout(vert_def);

	glDrawArrays(GL_QUADS, 0, 4);

	GL_state.Texture.Disable();

	SCP_vector<ubyte> colours;
	SCP_vector<GLfloat> distortion_vertex;
	colours.reserve(33 * 4);
	distortion_vertex.reserve(33 * 2);
	for(int i = 0; i < 33; i++)
	{
		colours.push_back((ubyte) rand()%256);
		colours.push_back((ubyte) rand()%256);
		colours.push_back(255);
		colours.push_back(255);

		distortion_vertex.push_back(0.04f);
		distortion_vertex.push_back((float)gr_screen.max_h*0.03125f*i);
	}

	vert_def = vertex_layout();

	vert_def.add_vertex_component(vertex_format_data::POSITION2, 0, &distortion_vertex.front());
	vert_def.add_vertex_component(vertex_format_data::COLOR4, 0, &colours.front());

	opengl_bind_vertex_layout(vert_def);

	glDrawArrays(GL_POINTS, 0, 33);

	Distortion_switch = !Distortion_switch;

	// reset state
	vglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, Scene_framebuffer);

	glViewport(0,0,gr_screen.max_w,gr_screen.max_h);

	GL_state.DepthTest(depth);
	GL_state.DepthMask(depth_mask);
	GL_state.Lighting(lighting);
	GL_state.Blend(blend);
	GL_state.CullFace(cull);
}

void opengl_draw(vertex_layout vertex_binding, GLenum prim_type, int count, int vbuffer_handle)
{

}
