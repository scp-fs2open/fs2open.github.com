/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/UI/UIDRAW.cpp $
 * $Revision: 2.3 $
 * $Date: 2004-07-26 20:47:55 $
 * $Author: Kazan $
 *
 * Routines to draw UI sort of stuff.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.2  2004/07/12 16:33:08  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.1  2002/08/01 01:41:10  penguin
 * The big include file move
 *
 * Revision 2.0  2002/06/03 04:02:29  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:13  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     10/13/98 9:29a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 2     10/07/98 10:54a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:51a Dave
 * 
 * 4     3/10/98 4:19p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 3     2/03/98 4:21p Hoffoss
 * Made UI controls draw white text when disabled.
 * 
 * 2     6/11/97 1:13p John
 * Started fixing all the text colors in the game.
 * 
 * 1     11/14/96 6:55p John
 *
 * $NoKeywords: $
 */

#include "ui/uidefs.h"
#include "ui/ui.h"
#include "globalincs/alphacolors.h"



void ui_hline(int x1, int x2, int y )
{
	gr_line(x1,y,x2,y);
}

void ui_vline(int y1, int y2, int x )
{
	gr_line(x,y1,x,y2);
}

void ui_string_centered( int x, int y, char * s )
{
	int height, width;

	gr_get_string_size( &width, &height, s );

	//baseline = height-grd_curcanv->cv_font->ft_baseline;

	gr_string(x-((width-1)/2), y-((height-1)/2), s );
}


void ui_draw_shad( int x1, int y1, int x2, int y2, color * c1, color *c2 )
{
	gr_set_color_fast( c1 );

	ui_hline( x1+0, x2-1, y1+0 );
	ui_vline( y1+1, y2+0, x1+0 );

	gr_set_color_fast( c2 );
	ui_hline( x1+1, x2, y2-0 );
	ui_vline( y1+0, y2-1, x2-0 );
}

void ui_draw_frame( int x1, int y1, int x2, int y2 )
{
	ui_draw_shad( x1+0, y1+0, x2-0, y2-0, &CBRIGHT, &CGRAY );
	ui_draw_shad( x1+1, y1+1, x2-1, y2-1, &CBRIGHT, &CGRAY );

	ui_draw_shad( x1+2, y1+2, x2-2, y2-2, &CWHITE, &CWHITE );
	ui_draw_shad( x1+3, y1+3, x2-3, y2-3, &CWHITE, &CWHITE );
	ui_draw_shad( x1+4, y1+4, x2-4, y2-4, &CWHITE, &CWHITE );
	ui_draw_shad( x1+5, y1+5, x2-5, y2-5, &CWHITE, &CWHITE );

	ui_draw_shad( x1+6, y1+6, x2-6, y2-6, &CGRAY, &CBRIGHT );
	ui_draw_shad( x1+7, y1+7, x2-7, y2-7, &CGRAY, &CBRIGHT );
}

void ui_rect( int x1, int y1, int x2, int y2 )
{
	gr_rect( x1, y1, x2-x1+1, y2-y1+1 );
}



void ui_draw_box_out( int x1, int y1, int x2, int y2 )
{

	gr_set_color_fast( &CWHITE );
	gr_rect( x1+2, y1+2, (x2-2)-(x1+2)+1, (y2-2)-(y1+2)+1 );

	ui_draw_shad( x1+0, y1+0, x2-0, y2-0, &CBRIGHT, &CGRAY );
	ui_draw_shad( x1+1, y1+1, x2-1, y2-1, &CBRIGHT, &CGRAY );

}

void ui_draw_box_in( int x1, int y1, int x2, int y2 )
{

	gr_set_color_fast( &CWHITE );
	gr_rect( x1+2, y1+2, (x2-2)-(x1+2)+1, (y2-2)-(y1+2)+1 );

	ui_draw_shad( x1+0, y1+0, x2-0, y2-0, &CGRAY, &CBRIGHT );
	ui_draw_shad( x1+1, y1+1, x2-1, y2-1, &CGRAY, &CBRIGHT );
}


void ui_draw_line_in( int x1, int y1, int x2, int y2 )
{
	gr_set_color_fast( &CGRAY );
	ui_hline( x1, x2, y1 );
	ui_hline( x1, x2-1, y2-1 );
	ui_vline( y1+1, y2-2, x1 );
	ui_vline( y1+1, y2-2, x2-1 );

	gr_set_color_fast( &CBRIGHT );
	ui_hline( x1+1, x2-1, y1+1 );
	ui_hline( x1, x2, y2 );
	ui_vline( y1+2, y2-2, x1+1 );
	ui_vline( y1+1, y2-1, x2 );
}

void ui_draw_sunken_border( int x1, int y1, int x2, int y2 )
{

	gr_set_color_fast( &CGRAY );
	ui_hline( x1-1, x2+1, y1-1);
	ui_vline( y1-1, y2+1, x1-1);

	gr_set_color_fast( &CBRIGHT );
	ui_hline( x1-1, x2+1, y2+1);
	ui_vline( y1, y2+1, x2+1);
}

