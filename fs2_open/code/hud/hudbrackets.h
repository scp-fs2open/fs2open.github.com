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

#include "hud/hud.h"

void hud_init_brackets();
void draw_bounding_brackets(int x1, int y1, int x2, int y2, int w_correction, int h_correction, float distance=0.0f, int target_objnum=-1);
void draw_bounding_brackets_subobject();
void draw_brackets_square(int x1, int y1, int x2, int y2, bool resize = true);
void draw_brackets_diamond(int x1, int y1, int x2, int y2);
void draw_brackets_square_quick(int x1, int y1, int x2, int y2, int thick=0);
void draw_brackets_diamond_quick(int x1, int y1, int x2, int y2, int thick=0);
void draw_brackets_dashed_square_quick(int x1, int y1, int x2, int y2);

int draw_subsys_brackets(ship_subsys* subsys, int min_width, int min_height, bool draw = true, bool set_color = true, int* draw_coords = NULL);

class HudGaugeBrackets: public HudGauge
{
protected:
	int attacking_dot;

	int Min_target_box_width;
	int Min_target_box_height;
	int Min_subtarget_box_width;
	int Min_subtarget_box_height;
public:
	HudGaugeBrackets();
	void initMinTargetBoxSizes(int w, int h);
	void initMinSubTargetBoxSizes(int w, int h);
	void initBitmaps(char *fname);
	void render(float frametime);
	void renderObjectBrackets(object *targetp, color *clr, int w_correction, int h_correction, int flags);
	void renderNavBrackets(vec3d* nav_pos, vertex* nav_point, color* clr, char* string);
	void renderBoundingBrackets(int x1, int y1, int x2, int y2, int w_correction, int h_correction, float distance, int target_objnum, int flags);
	void renderBoundingBracketsSubobject();
	void renderDistance(int x, int y, float distance);
};

#endif
