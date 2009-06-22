/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef HUD_BRACKETS
#define HUD_BRACKETS

void hud_init_brackets();
void draw_bounding_brackets(int x1, int y1, int x2, int y2, int w_correction, int h_correction, float distance=0.0f, int target_objnum=-1);
void draw_bounding_brackets_subobject();
void draw_brackets_square(int x1, int y1, int x2, int y2, bool resize = true);
void draw_brackets_diamond(int x1, int y1, int x2, int y2);
void draw_brackets_square_quick(int x1, int y1, int x2, int y2, int thick=0);
void draw_brackets_diamond_quick(int x1, int y1, int x2, int y2, int thick=0);
void draw_brackets_dashed_square_quick(int x1, int y1, int x2, int y2);

#endif
