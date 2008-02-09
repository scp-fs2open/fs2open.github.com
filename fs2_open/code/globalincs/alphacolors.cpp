/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#include "graphics/2d.h"
#include "globalincs/alphacolors.h"

// -----------------------------------------------------------------------------------
// ALPHA DEFINES/VARS
//

color Color_black, Color_grey, Color_blue, Color_bright_blue, Color_violet_gray;
color Color_green, Color_bright_green, Color_bright_white, Color_white;
color Color_red, Color_bright_red, Color_yellow, Color_bright_yellow, Color_dim_red;
color Color_ui_light_green, Color_ui_green;
color Color_ui_light_pink, Color_ui_pink;

// netplayer colors
color *Color_netplayer[12] = {
	&Color_white,
	&Color_bright_white,
	&Color_bright_blue,
	&Color_red,
	&Color_bright_red,
	&Color_blue,
	&Color_bright_green,
	&Color_bright_blue,
	&Color_yellow,
	&Color_bright_yellow,
	&Color_ui_green,
	&Color_ui_pink
};


// -----------------------------------------------------------------------------------
// ALPHA FUNCTIONS
//

// initialize all alpha colors (call at startup)
void alpha_colors_init()
{
	// See the variable declarations above for color usage
	gr_init_alphacolor( &Color_blue, 93, 93, 128, 255 );
	gr_init_alphacolor( &Color_bright_blue, 185, 185, 255, 255 );

	gr_init_alphacolor( &Color_green, 0, 120, 0, 255 );
	gr_init_alphacolor( &Color_bright_green, 50, 190, 50, 255 );

	gr_init_alphacolor( &Color_black, 0, 0, 0, 255 );
	gr_init_alphacolor( &Color_grey, 50, 50, 50, 255 );
	//gr_init_alphacolor( &Color_white, 185, 185, 185, 255 );
	gr_init_alphacolor( &Color_white, 105, 105, 105, 255 );
	gr_init_alphacolor( &Color_bright_white, 255, 255, 255, 255 );

	gr_init_alphacolor( &Color_violet_gray, 160, 144, 160, 255 );

	gr_init_alphacolor( &Color_dim_red, 80, 6, 6, 255 );
	gr_init_alphacolor( &Color_red, 126, 6, 6, 255 );
	gr_init_alphacolor( &Color_bright_red, 200, 0, 0, 255 );

	gr_init_alphacolor( &Color_yellow, 113, 184, 124, 255 );
	gr_init_alphacolor( &Color_bright_yellow, 162, 210, 162, 255 );

	gr_init_alphacolor( &Color_ui_light_green, 161, 184, 161, 255 );
	gr_init_alphacolor( &Color_ui_green, 190, 228, 190, 255 );

	gr_init_alphacolor( &Color_ui_light_pink, 184, 161, 161, 255 );
	gr_init_alphacolor( &Color_ui_pink, 228, 190, 190, 255 );
}
