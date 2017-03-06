
#include "graphics/render.h"
#include "graphics/material.h"

void gr_flash(int r, int g, int b) {
	gr_flash_alpha(r, g, b, 255);
}

void gr_flash_alpha(int r, int g, int b, int a) {
	if ( !(r || g || b || a) ) {
		return;
	}

	CLAMP(r, 0, 255);
	CLAMP(g, 0, 255);
	CLAMP(b, 0, 255);
	CLAMP(a, 0, 255);

	int x1 = (gr_screen.clip_left + gr_screen.offset_x);
	int y1 = (gr_screen.clip_top + gr_screen.offset_y);
	int x2 = (gr_screen.clip_right + gr_screen.offset_x) + 1;
	int y2 = (gr_screen.clip_bottom + gr_screen.offset_y) + 1;

	material render_material;
	// By default materials use the immediate mode shader which is fine for us
	render_material.set_color(r, g, b, 255);
	render_material.set_blend_mode(ALPHA_BLEND_ADDITIVE);
	render_material.set_depth_mode(ZBUFFER_TYPE_NONE);

	int glVertices[8] = {
		x1, y1,
		x1, y2,
		x2, y1,
		x2, y2
	};

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::SCREEN_POS, 0, 0);

	gr_render_primitives_2d_immediate(&render_material, PRIM_TYPE_TRISTRIP, &vert_def, 4, glVertices, sizeof(int) * 8);
}

static void draw_textured_quad(material* mat,
	float x1, float y1, float u1, float v1,
	float x2, float y2, float u2, float v2 )
{
	GR_DEBUG_SCOPE("Draw textured quad");

	float glVertices[4][4] = {
		{ x1, y1, u1, v1 },
		{ x1, y2, u1, v2 },
		{ x2, y1, u2, v1 },
		{ x2, y2, u2, v2 }
	};

	vertex_layout vert_def;

	vert_def.add_vertex_component(vertex_format_data::POSITION2, sizeof(float) * 4, 0);
	vert_def.add_vertex_component(vertex_format_data::TEX_COORD, sizeof(float) * 4, sizeof(float) * 2);

	gr_render_primitives_immediate(mat, PRIM_TYPE_TRISTRIP, &vert_def, 4, glVertices, sizeof(float) * 4 * 4);
}

static void bitmap_ex_internal(int x, int y, int w, int h, int sx, int sy, int resize_mode,
							   bool aabitmap, bool mirror, color* clr)
{
	if ( (w < 1) || (h < 1) ) {
		return;
	}

	if ( aabitmap && !gr_screen.current_color.is_alphacolor ) {
		return;
	}

	float u0, u1, v0, v1;
	float x1, x2, y1, y2;
	int bw, bh;

	bool do_resize;
	if ( resize_mode != GR_RESIZE_NONE && (gr_screen.custom_size || (gr_screen.rendering_to_texture != -1)) ) {
		do_resize = true;
	} else {
		do_resize = false;
	}

	bm_get_info(gr_screen.current_bitmap, &bw, &bh);

	u0 = (i2fl(sx) / i2fl(bw));
	v0 = (i2fl(sy) / i2fl(bh));

	u1 = (i2fl(sx+w) / i2fl(bw));
	v1 = (i2fl(sy+h) / i2fl(bh));

	x1 = i2fl(x + ((do_resize) ? gr_screen.offset_x_unscaled : gr_screen.offset_x));
	y1 = i2fl(y + ((do_resize) ? gr_screen.offset_y_unscaled : gr_screen.offset_y));
	x2 = x1 + i2fl(w);
	y2 = y1 + i2fl(h);

	if (do_resize) {
		gr_resize_screen_posf(&x1, &y1, NULL, NULL, resize_mode);
		gr_resize_screen_posf(&x2, &y2, NULL, NULL, resize_mode);
	}

	if (mirror) {
		float temp = u0;
		u0 = u1;
		u1 = temp;
	}

	material render_mat;
	render_mat.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);
	render_mat.set_depth_mode(ZBUFFER_TYPE_NONE);
	render_mat.set_texture_map(TM_BASE_TYPE, gr_screen.current_bitmap);
	render_mat.set_color(clr->red, clr->green, clr->blue, clr->alpha);
	render_mat.set_cull_mode(false);

	if (aabitmap) {
		render_mat.set_texture_type(material::TEX_TYPE_AABITMAP);
	} else {
		if ( bm_has_alpha_channel(gr_screen.current_bitmap) ) {
			render_mat.set_texture_type(material::TEX_TYPE_XPARENT);
		} else {
			render_mat.set_texture_type(material::TEX_TYPE_NORMAL);
		}
	}

	draw_textured_quad(&render_mat, x1, y1, u0, v0, x2, y2, u1, v1);
}

void gr_aabitmap(int x, int y, int resize_mode, bool mirror) {

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
	bitmap_ex_internal(dx1, dy1, (dx2 - dx1 + 1), (dy2 - dy1 + 1), sx, sy, resize_mode, true, mirror, &gr_screen.current_color);
}
void gr_aabitmap_ex(int x, int y, int w, int h, int sx, int sy, int resize_mode, bool mirror) {

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
	bitmap_ex_internal(dx1, dy1, (dx2 - dx1 + 1), (dy2 - dy1 + 1), sx, sy, resize_mode, true, mirror, &gr_screen.current_color);
}
//these are penguins bitmap functions
void gr_bitmap_ex(int x, int y, int w, int h, int sx, int sy, int resize_mode) {
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

	color clr;
	gr_init_alphacolor(&clr, 255, 255, 255, fl2i(gr_screen.current_alpha * 255.0f));

	// We now have dx1,dy1 and dx2,dy2 and sx, sy all set validly within clip regions.
	bitmap_ex_internal(dx1, dy1, (dx2 - dx1 + 1), (dy2 - dy1 + 1), sx, sy, resize_mode, false, false, &clr);
}



