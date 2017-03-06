#pragma once

#include "globalincs/pstypes.h"
#include "graphics/2d.h"

void gr_flash( int r, int g, int b );

void gr_flash_alpha(int r, int g, int b, int a);


void gr_aabitmap(int x, int y, int resize_mode = GR_RESIZE_FULL, bool mirror = false);

void gr_aabitmap_ex(int x, int y, int w, int h, int sx, int sy, int resize_mode = GR_RESIZE_FULL, bool mirror = false);

void gr_bitmap_ex(int x, int y, int w, int h, int sx, int sy, int resize_mode = GR_RESIZE_FULL);


void gr_string(float x, float y, const char* string, int resize_mode = GR_RESIZE_FULL, int length = -1);

inline void gr_string(int x, int y, const char* string, int resize_mode = GR_RESIZE_FULL, int length = -1)
{
	gr_string(i2fl(x), i2fl(y), string, resize_mode, length);
}

// Integer line. Used to draw a fast but pixely line.
void gr_line(int x1, int y1, int x2, int y2, int resize_mode = GR_RESIZE_FULL);

// Draws an antialiased line is the current color is an
// alphacolor, otherwise just draws a fast line.  This
// gets called internally by g3_draw_line.   This assumes
// the vertex's are already clipped, so call g3_draw_line
// not this if you have two 3d points.
void gr_aaline(vertex *v1, vertex *v2);

// Draw a gradient line... x1,y1 is bright, x2,y2 is transparent.
void gr_gradient(int x1, int y1, int x2, int y2, int resize_mode = GR_RESIZE_FULL);

void gr_pixel(int x, int y, int resize_mode = GR_RESIZE_FULL);
