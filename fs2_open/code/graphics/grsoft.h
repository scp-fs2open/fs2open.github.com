/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrSoft.h $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:23 $
 * $Author: penguin $
 *
 * Include file for our software renderer
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 2     3/25/98 8:07p John
 * Restructured software rendering into two modules; One for windowed
 * debug mode and one for DirectX fullscreen.   
 * 
 * 1     5/12/97 12:14p John
 *
 * $NoKeywords: $
 */

#ifndef _GRSOFT_H
#define _GRSOFT_H

void gr_soft_init();
void gr_soft_cleanup();


// Functions/variables common between grsoft and grdirectdraw
extern int Grx_mouse_saved;
void grx_save_mouse_area(int x, int y, int w, int h );
void grx_restore_mouse_area();
void grx_print_screen(char * filename);
int gr8_save_screen();
void gr8_restore_screen(int id);
void gr8_free_screen(int id);
void gr8_dump_frame_start(int first_frame, int frames_between_dumps);
void gr8_dump_frame();
void gr8_dump_frame_stop();
void gr8_set_gamma(float gamma);

// bitmap functions
void grx_bitmap(int x, int y);
void grx_bitmap_ex(int x, int y, int w, int h, int sx, int sy);

#endif
