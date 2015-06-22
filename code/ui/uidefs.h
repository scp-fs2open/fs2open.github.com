/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _UIDEFS_H
#define _UIDEFS_H

#include "freespace2/freespace.h"
#include "globalincs/pstypes.h"
#include "io/key.h"
#include "io/mouse.h"

#define CBLACK Color_black
#define CGREEN Color_green
#define CBRIGHT_GREEN Color_bright_green
#define CGRAY Color_grey
#define CDARK_GRAY Color_bright_white  // since gray doesn't work with our current color system..
#define CWHITE Color_white
#define CBRIGHT Color_bright_white

#define BORDER_WIDTH 8

void ui_hline(int x1, int x2, int y );
void ui_vline(int y1, int y2, int x );
void ui_string_centered( int x, int y, char * s );
void ui_draw_shad( int x1, int y1, int x2, int y2, int r1, int g1, int b1, int r2, int g2, int b2 );
void ui_draw_frame( int x1, int y1, int x2, int y2 );
void ui_rect( int x1, int y1, int x2, int y2 );
void ui_draw_box_out( int x1, int y1, int x2, int y2 );
void ui_draw_box_in( int x1, int y1, int x2, int y2 );
void ui_draw_line_in( int x1, int y1, int x2, int y2 );
void ui_draw_sunken_border( int x1, int y1, int x2, int y2 );

#define BUTTON_PRESSED          1
#define BUTTON_RELEASED         2
#define BUTTON_JUST_PRESSED     4
#define BUTTON_JUST_RELEASED    8
#define BUTTON_DOUBLE_CLICKED   16

#define B1_PRESSED          (ui_mouse.b1_status & BUTTON_PRESSED)
#define B1_RELEASED         (ui_mouse.b1_status & BUTTON_RELEASED)
#define B1_JUST_PRESSED     (ui_mouse.b1_status & BUTTON_JUST_PRESSED)
#define B1_JUST_RELEASED    (ui_mouse.b1_status & BUTTON_JUST_RELEASED)
#define B1_DOUBLE_CLICKED   (ui_mouse.b1_status & BUTTON_DOUBLE_CLICKED)

#define B2_PRESSED          (ui_mouse.b2_status & BUTTON_PRESSED)
#define B2_RELEASED         (ui_mouse.b2_status & BUTTON_RELEASED)
#define B2_JUST_PRESSED     (ui_mouse.b2_status & BUTTON_JUST_PRESSED)
#define B2_JUST_RELEASED    (ui_mouse.b2_status & BUTTON_JUST_RELEASED)

typedef struct UI_MOUSE	{
	int	x, y;
	int	dx, dy;
	int	b1_status;
	int	b1_last_status;
	int	b1_time_lastpressed;
	int	b2_status;
	int	b2_last_status;
	int	b2_time_lastpressed;
	int	timestamp;
} UI_MOUSE;

extern UI_MOUSE ui_mouse;
extern void ui_mouse_process();


#define Middle(x) ((x)/2)

#endif
