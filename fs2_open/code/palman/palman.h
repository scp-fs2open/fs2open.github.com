/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/*
 * $Logfile: /Freespace2/code/Palman/PalMan.h $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:26:01 $
 * $Author: penguin $
 *
 * Palette Manager header file
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:12  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     3/31/99 8:24p Dave
 * Beefed up all kinds of stuff, incluging beam weapons, nebula effects
 * and background nebulae. Added per-ship non-dimming pixel colors.
 * 
 * 3     2/05/99 12:52p Dave
 * Fixed Glide nondarkening textures.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:50a Dave
 * 
 * 14    5/20/98 9:46p John
 * added code so the places in code that change half the palette don't
 * have to clear the screen.
 * 
 * 13    12/02/97 4:00p John
 * Added first rev of thruster glow, along with variable levels of
 * translucency, which retquired some restructing of palman.
 * 
 * 12    11/21/97 11:32a John
 * Added nebulas.   Fixed some warpout bugs.
 * 
 * 11    11/14/97 12:31p John
 * Fixed some DirectX bugs.  Moved the 8-16 xlat tables into Graphics
 * libs.  Made 16-bpp DirectX modes know what bitmap format they're in.
 * 
 * 10    7/16/97 5:29p John
 * added palette table caching and made scaler and liner no light tmapper
 * do alpha blending in 8 bpp mode.
 * 
 * 9     7/16/97 3:07p John
 * 
 * 8     5/21/97 11:06a Lawrance
 * added user_palette_find()
 * 
 * 7     5/12/97 12:27p John
 * Restructured Graphics Library to add support for multiple renderers.
 * 
 * 6     11/26/96 6:50p John
 * Added some more hicolor primitives.  Made windowed mode run as current
 * bpp, if bpp is 8,16,or 32.
 * 
 * 5     11/26/96 9:44a Allender
 * allow for use of different bitmap palettes
 *
 * $NoKeywords: $
 */

#ifndef _PALMAN_H
#define _PALMAN_H 

// Calculate tables for this palette.
// Assumes gr_palette is filled in.
extern void palette_update(char *name, int restrict_colors_to_upper_128 );

// Writes current tables to disk.
extern void palette_flush();

// Functions to query a palette
extern uint palette_compute_checksum( ubyte *pal );		// computes checksum of palette
extern ubyte *palette_get_blend_table(float alpha);
extern ubyte *palette_get_fade_table();

extern uint palette_find( int r, int g, int b );

// Data used to query a palette
extern ubyte gr_palette[256*3];
extern ubyte gr_fade_table[(256*34)*2];
extern uint gr_palette_checksum;

// Functions to deal with changing the palette.
// These just call gr_set_palette, which will in turn
// call palette_flush and palette_update.
extern void palette_load_table( char * filename );
extern void palette_use_bm_palette(int n);
extern void palette_restore_palette( void );

// nondarkening texture pixel colors
#define MAX_NONDARK_COLORS					10

extern int Palman_num_nondarkening_default;
extern ubyte Palman_non_darkening_default[MAX_NONDARK_COLORS][3];

extern int Palman_num_nondarkening;
extern ubyte Palman_non_darkening[MAX_NONDARK_COLORS][3];

extern int palman_is_nondarkening(int r,int g, int b);
extern void palman_load_pixels();
extern void palman_set_nondarkening(ubyte colors[MAX_NONDARK_COLORS][3], int size);

#endif
