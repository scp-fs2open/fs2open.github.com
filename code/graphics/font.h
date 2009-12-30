/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _FONT_H
#define _FONT_H

#include "globalincs/pstypes.h"

#define MAX_FONTS 5

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
extern void gr_stuff_first_font(char *first_font, size_t first_font_size);
extern int gr_get_current_fontnum();
extern int gr_get_fontnum(char *filename);
extern void gr_set_font(int fontnum);
extern void gr8_string(int x,int y,char * text);

void gr_print_timestamp(int x, int y, int timestamp);
char *gr_force_fit_string(char *str, int max_str, int max_width);
void gr_font_init();
void gr_font_close();

extern font *Current_font;
extern int get_char_width(ubyte c1,ubyte c2,int *width,int *spacing);
extern int get_centered_x(char *s);

#endif
