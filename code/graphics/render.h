#pragma once

#include "globalincs/pstypes.h"
#include "graphics/2d.h"

void gr_flash( int r, int g, int b );

void gr_flash_alpha(int r, int g, int b, int a);


void gr_aabitmap(int x, int y, int resize_mode = GR_RESIZE_FULL, bool mirror = false);

void gr_aabitmap_ex(int x, int y, int w, int h, int sx, int sy, int resize_mode = GR_RESIZE_FULL, bool mirror = false);

void gr_bitmap_ex(int x, int y, int w, int h, int sx, int sy, int resize_mode = GR_RESIZE_FULL);
