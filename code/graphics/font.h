/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/Font.h $
 * $Revision: 1.1 $
 * $Date: 2002-06-03 03:25:57 $
 * $Author: penguin $
 *
 * header file for font stuff
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 4     7/09/99 10:32p Dave
 * Command brief and red alert screens.
 * 
 * 3     10/13/98 9:28a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 14    5/25/98 10:32a John
 * Took out redundant code for font bitmap offsets that converted to a
 * float, then later on converted back to an integer.  Quite unnecessary.
 * 
 * 13    3/25/98 8:07p John
 * Restructured software rendering into two modules; One for windowed
 * debug mode and one for DirectX fullscreen.   
 * 
 * 12    3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 11    3/09/98 6:06p John
 * Restructured font stuff to avoid duplicate code in Direct3D and Glide.
 * Restructured Glide to avoid redundent state setting.
 * 
 * 10    2/19/98 9:04a John
 * Fixed fonts with glide
 * 
 * 9     2/17/98 7:27p John
 * Got fonts and texturing working in Direct3D
 * 
 * 8     11/30/97 12:18p John
 * added more 24 & 32-bpp primitives
 * 
 * 7     11/06/97 5:42p Hoffoss
 * Added support for fixed size timstamp rendering.
 * 
 * 6     11/03/97 10:59a John
 * added support for more than one font.
 * 
 * 5     10/24/97 12:13p Hoffoss
 * Added gr_force_fit_string().
 * 
 * 4     10/09/97 5:23p John
 * Added support for more 16-bpp functions
 * 
 * 3     6/05/97 4:53p John
 * First rev of new antialiased font stuff.
 * 
 * 2     4/22/97 10:33a John
 * fixed the 2d resource leaks that Alan found.
 * 
 * 1     3/31/97 9:42a Allender
 *
 * $NoKeywords: $
 */

#ifndef _FONT_H
#define _FONT_H

#define MAX_FONTS 3

#define FONT_VERSION 0
#define WIDEST_DIGIT	"4"  // the widest number character
#define WIDEST_CHAR	"W"  // the widest character

typedef struct font_char {
	int				spacing;
	int				byte_width;
	int				offset;
	short				kerning_entry;
	short				user_data;
} font_char;

typedef struct font_kernpair {
	char				c1,c2;
	signed char		offset;
} font_kernpair;


typedef struct font {
	char				filename[MAX_FILENAME_LEN];
	int				id;			// Should be 'VFNT'
	int				version;			// font version
	int				num_chars;
	int				first_ascii;
	int				w;
	int				h;
	int				num_kern_pairs;
	int				kern_data_size;
	int				char_data_size;
	int				pixel_data_size;
	font_kernpair	*kern_data;
	font_char		*char_data;
	ubyte				*pixel_data;

	// Data for 3d cards
	int				bitmap_id;			// A bitmap representing the font data
	int				bm_w, bm_h;			// Bitmap width and height
	ubyte				*bm_data;			// The actual font data
	int				*bm_u;				// U offset of each character
	int				*bm_v;				// V offset of each character

} font;

extern int Num_fonts;
extern font Fonts[MAX_FONTS];
extern font *Current_font;

#define FONT1				0				// font01.vf
#define FONT2				1				// font02.vf
#define FONT3				2				// font03.vf

// extern definitions for basic font functions
extern void grx_set_font(int fontnum);
extern void gr8_string(int x,int y,char * text);

void gr_print_timestamp(int x, int y, int timestamp);
char *gr_force_fit_string(char *str, int max_str, int max_width);
void gr_font_init();
void gr_font_close();

extern font * Current_font;
extern int get_char_width(ubyte c1,ubyte c2,int *width,int *spacing);
extern int get_centered_x(char *s);

#endif
