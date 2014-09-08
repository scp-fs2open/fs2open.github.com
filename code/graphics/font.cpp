/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#endif

#include <stdio.h>
#include <stdarg.h>

#include "graphics/2d.h"
#include "cfile/cfile.h"
#include "graphics/font.h"
#include "palman/palman.h"
#include "io/key.h"
#include "bmpman/bmpman.h"
#include "localization/localize.h"
#include "parse/parselo.h"
#include "globalincs/systemvars.h"
#include "globalincs/def_files.h"



int Num_fonts = 0;
font Fonts[MAX_FONTS];
font *Current_font = NULL;


/**
 * Crops a string if required to force it to not exceed max_width pixels when printed.
 * Does this by dropping characters at the end of the string and adding '...' to the end.
 *
 * @param str		string to crop.  Modifies this string directly
 * @param max_str	max characters allowed in str
 * @param max_width number of pixels to limit string to (less than or equal to).
 * @return			Returns same pointer passed in for str.
 */
char *gr_force_fit_string(char *str, int max_str, int max_width)
{
	int w;

	gr_get_string_size(&w, NULL, str);
	if (w > max_width) {
		if ((int) strlen(str) > max_str - 3) {
			Assert(max_str >= 3);
			str[max_str - 3] = 0;
		}

		strcpy(str + strlen(str) - 1, "...");
		gr_get_string_size(&w, NULL, str);
		while (w > max_width) {
			Assert(strlen(str) >= 4);  // if this is hit, a bad max_width was passed in and the calling function needs fixing.
			strcpy(str + strlen(str) - 4, "...");
			gr_get_string_size(&w, NULL, str);
		}
	}

	return str;
}

/**
 * Takes the character BEFORE being offset into current font
 * @return the letter code
 */
int get_char_width(ubyte c1,ubyte c2,int *width,int *spacing)
{
	int i, letter;

	Assert ( Current_font != NULL );
	letter = c1-Current_font->first_ascii;

	if (letter<0 || letter>=Current_font->num_chars) {				//not in font, draw as space
		*width=0;
		*spacing = Current_font->w;
		return -1;
	}

	*width = Current_font->char_data[letter].byte_width;
	*spacing = Current_font->char_data[letter].spacing;

	i = Current_font->char_data[letter].kerning_entry;
	if ( i > -1)  {
		if (!(c2==0 || c2=='\n')) {
			int letter2;

			letter2 = c2-Current_font->first_ascii;

			if ((letter2>=0) && (letter2<Current_font->num_chars) ) {				//not in font, draw as space
				font_kernpair	*k = &Current_font->kern_data[i];
				while( (k->c1 == (char)letter) && (k->c2<(char)letter2) && (i<Current_font->num_kern_pairs-1) )	{
					i++;
					k++;
				}
				if ( k->c2 == (char)letter2 )	{
					*spacing += k->offset;
				}
			}
		}
	}
	return letter;
}

// NOTE: this returns an unscaled size for non-standard resolutions
int get_centered_x(const char *s, bool scaled)
{
	int w,w2,s2;

	for (w=0;*s!=0 && *s!='\n';s++) {
		get_char_width(s[0],s[1],&w2,&s2);
		w += s2;
	}

	return (((scaled ? gr_screen.clip_width : gr_screen.clip_width_unscaled) - w) / 2);
}

/**
 * Draws a character centered on x
 */
void gr_char_centered(int x, int y, char chr, int resize_mode)
{
	char str[2];
	int w, sc;

	sc = Lcl_special_chars;
	if (chr == '1')
		chr = (char)(sc + 1);

	str[0] = chr;
	str[1] = 0;
	gr_get_string_size(&w, NULL, str);
	gr_string(x - w / 2, y, str, resize_mode);
}

void gr_print_timestamp(int x, int y, fix timestamp, int resize_mode)
{
	char h[2], m[3], s[3];
	int w, c;

	int time = (int)f2fl(timestamp);  // convert to seconds

	// format the time information into strings
	sprintf(h, "%.1d", (time / 3600));
	sprintf(m, "%.2d", (time / 60) % 60);
	sprintf(s, "%.2d", time % 60);

	gr_get_string_size(&w, NULL, "0");
	gr_get_string_size(&c, NULL, ":");

	gr_string(x + w, y, ":", resize_mode);
	gr_string(x + w * 3 + c, y, ":", resize_mode);

	x += w / 2;
	gr_char_centered(x, y, h[0], resize_mode);
	x += w + c;
	gr_char_centered(x, y, m[0], resize_mode);
	x += w;
	gr_char_centered(x, y, m[1], resize_mode);
	x += w + c;
	gr_char_centered(x, y, s[0], resize_mode);
	x += w;
	gr_char_centered(x, y, s[1], resize_mode);
}

int gr_get_font_height()
{
	if (Current_font)	{
		return Current_font->h;
	} else {
		return 16;
	}
}

void gr_get_string_size(int *w1, int *h1, const char *text, int len)
{
	int longest_width;
	int width,spacing;
	int w, h;

	if (!Current_font)	{
		if ( w1)
			*w1 = 16;

		if ( h1 )
			*h1 = 16;

		return;
	}	

	w = 0;
	h = 0;
	longest_width = 0;

	if (text != NULL ) {
		h += Current_font->h;
		while (*text && (len>0) ) {

			// Process one or more 
			while ((*text == '\n') && (len>0) ) {
				text++;
				len--;
				if ( *text )	{
					h += Current_font->h;
				}
				w = 0;
			}

			if (*text == 0)	{
				break;
			}

			get_char_width(text[0], text[1], &width, &spacing);
			w += spacing;
			if (w > longest_width)
				longest_width = w;

			text++;
			len--;
		}
	}

	if ( h1 )
		*h1 = h;

	if ( w1 )
		*w1 = longest_width;
}


MONITOR( FontChars )

#ifdef _WIN32
HFONT MyhFont = NULL;
HDC hDibDC = NULL;

void gr_string_win(int x, int y, const char *s)
{
	int old_bitmap = gr_screen.current_bitmap; 
	gr_set_font(FONT1);
   	gr_string(x,y,s);
	gr_screen.current_bitmap = old_bitmap; 
}

void gr_get_string_size_win(int *w, int *h, const char *text)
{
	const char *ptr;
	SIZE size;

	ptr = strchr(text, '\n');

	if (MyhFont==NULL)	{
		if (w) *w = 0;
		if (h) *h = 0;
		return;
	}

	SelectObject( hDibDC, MyhFont );

	if (!ptr)	{
		GetTextExtentPoint32( hDibDC, text, strlen(text), &size);
		if (w) *w = size.cx;
		if (h) *h = size.cy;
		return;
	}

	GetTextExtentPoint32(hDibDC, text, ptr - text, &size);
	gr_get_string_size_win(w, h, ptr+1);
	if (w && (size.cx > *w) )
		*w = size.cx;

	if (h)
		*h += size.cy;
}
#endif   // ifdef _WIN32

char grx_printf_text[2048];	

void _cdecl gr_printf( int x, int y, const char * format, ... )
{
	va_list args;

	if ( !Current_font ) return;
	
	va_start(args, format);
	vsprintf(grx_printf_text,format,args);
	va_end(args);

	gr_string(x,y,grx_printf_text);
}

void _cdecl gr_printf_menu( int x, int y, const char * format, ... )
{
	va_list args;

	if ( !Current_font ) return;
	
	va_start(args, format);
	vsprintf(grx_printf_text,format,args);
	va_end(args);

	gr_string(x,y,grx_printf_text,GR_RESIZE_MENU);
}

void _cdecl gr_printf_menu_zoomed( int x, int y, const char * format, ... )
{
	va_list args;

	if ( !Current_font ) return;

	va_start(args, format);
	vsprintf(grx_printf_text,format,args);
	va_end(args);

	gr_string(x,y,grx_printf_text,GR_RESIZE_MENU_ZOOMED);
}

void _cdecl gr_printf_no_resize( int x, int y, const char * format, ... )
{
	va_list args;

	if ( !Current_font ) return;
	
	va_start(args, format);
	vsprintf(grx_printf_text,format,args);
	va_end(args);

	gr_string(x,y,grx_printf_text,GR_RESIZE_NONE);
}

void gr_font_close()
{
	font *fnt;
	int i;

	fnt = Fonts;

	for (i=0; i<Num_fonts; i++) {
		if (fnt->kern_data) {
			vm_free(fnt->kern_data);
			fnt->kern_data = NULL;
		}

		if (fnt->char_data) {
			vm_free(fnt->char_data);
			fnt->char_data = NULL;
		}

		if (fnt->pixel_data) {
			vm_free(fnt->pixel_data);
			fnt->pixel_data = NULL;
		}

		if (fnt->bm_data) {
			vm_free(fnt->bm_data);
			fnt->bm_data = NULL;
		}

		if (fnt->bm_u) {
			vm_free(fnt->bm_u);
			fnt->bm_u = NULL;
		}

		if (fnt->bm_v) {
			vm_free(fnt->bm_v);
			fnt->bm_v = NULL;
		}

		fnt++;
	}
}

/**
 * @return -1 if couldn't init font, otherwise returns the font id number.
 */
int gr_create_font(char * typeface)
{
	CFILE *fp;
	font *fnt;
	int n, fontnum;

	fnt = Fonts;
	n = -1;
	for (fontnum=0; fontnum<Num_fonts; fontnum++ )	{
		if (fnt->id != 0 )	{
			if ( !_strnicmp( fnt->filename, typeface, MAX_FILENAME_LEN ) )	{
				return fontnum;
			}
		} else {
			if ( n < 0 )	{
				n = fontnum;
			}				
		}
		fnt++;
	}

	if ( fontnum == MAX_FONTS )	{
		Warning( LOCATION, "Too many fonts!\nSee John, or change MAX_FONTS in Graphics\\Font.h\n" );
		return -1;
	}

	if ( fontnum == Num_fonts )	{
		Num_fonts++;
	}
	
	bool localize = true;

	fp = cfopen( typeface, "rb", CFILE_NORMAL, CF_TYPE_ANY, localize );
	if ( fp == NULL ) return -1;

	strcpy_s( fnt->filename, typeface );
	cfread( &fnt->id, 4, 1, fp );
	cfread( &fnt->version, sizeof(int), 1, fp );
	cfread( &fnt->num_chars, sizeof(int), 1, fp );
	cfread( &fnt->first_ascii, sizeof(int), 1, fp );
	cfread( &fnt->w, sizeof(int), 1, fp );
	cfread( &fnt->h, sizeof(int), 1, fp );
	cfread( &fnt->num_kern_pairs, sizeof(int), 1, fp );
	cfread( &fnt->kern_data_size, sizeof(int), 1, fp );
	cfread( &fnt->char_data_size, sizeof(int), 1, fp );
	cfread( &fnt->pixel_data_size, sizeof(int), 1, fp );

	fnt->id = INTEL_SHORT( fnt->id ); //-V570
	fnt->version = INTEL_INT( fnt->version ); //-V570
	fnt->num_chars = INTEL_INT( fnt->num_chars ); //-V570
	fnt->first_ascii = INTEL_INT( fnt->first_ascii ); //-V570
	fnt->w = INTEL_INT( fnt->w ); //-V570
	fnt->h = INTEL_INT( fnt->h ); //-V570
	fnt->num_kern_pairs = INTEL_INT( fnt->num_kern_pairs ); //-V570
	fnt->kern_data_size = INTEL_INT( fnt->kern_data_size ); //-V570
	fnt->char_data_size = INTEL_INT( fnt->char_data_size ); //-V570
	fnt->pixel_data_size = INTEL_INT( fnt->pixel_data_size ); //-V570

	if ( fnt->kern_data_size )	{
		fnt->kern_data = (font_kernpair *)vm_malloc( fnt->kern_data_size );
		Assert(fnt->kern_data!=NULL);
		cfread( fnt->kern_data, fnt->kern_data_size, 1, fp );
	} else {
		fnt->kern_data = NULL;
	}
	if ( fnt->char_data_size )	{
		fnt->char_data = (font_char *)vm_malloc( fnt->char_data_size );
		Assert( fnt->char_data != NULL );
		cfread( fnt->char_data, fnt->char_data_size, 1, fp );

		for (int i=0; i<fnt->num_chars; i++) {
			fnt->char_data[i].spacing = INTEL_INT( fnt->char_data[i].spacing ); //-V570
			fnt->char_data[i].byte_width = INTEL_INT( fnt->char_data[i].byte_width ); //-V570
			fnt->char_data[i].offset = INTEL_INT( fnt->char_data[i].offset ); //-V570
			fnt->char_data[i].kerning_entry = INTEL_INT( fnt->char_data[i].kerning_entry ); //-V570
			fnt->char_data[i].user_data = INTEL_SHORT( fnt->char_data[i].user_data ); //-V570
		}
	} else {
		fnt->char_data = NULL;
	}
	if ( fnt->pixel_data_size )	{
		fnt->pixel_data = (ubyte *)vm_malloc( fnt->pixel_data_size );
		Assert(fnt->pixel_data!=NULL);
		cfread( fnt->pixel_data, fnt->pixel_data_size, 1, fp );
	} else {
		fnt->pixel_data = NULL;
	}
	cfclose(fp);

	// Create a bitmap for hardware cards.
	// JAS:  Try to squeeze this into the smallest square power of two texture.
	// This should probably be done at font generation time, not here.
	int w, h;
	if ( fnt->pixel_data_size*4 < 64*64 ) {
		w = h = 64;
	} else if ( fnt->pixel_data_size*4 < 128*128 ) {
		w = h = 128;
	} else if ( fnt->pixel_data_size*4 < 256*256 ) {
		w = h = 256;
	} else if ( fnt->pixel_data_size*4 < 512*512 ) {
		w = h = 512;
	} else {
		w = h = 1024;
	}

	fnt->bm_w = w;
	fnt->bm_h = h;
	fnt->bm_data = (ubyte *)vm_malloc(fnt->bm_w*fnt->bm_h);
	fnt->bm_u = (int *)vm_malloc(sizeof(int)*fnt->num_chars);
	fnt->bm_v = (int *)vm_malloc(sizeof(int)*fnt->num_chars);

	memset( fnt->bm_data, 0, fnt->bm_w * fnt->bm_h );

	int i,x,y;
	x = y = 0;
	for (i=0; i<fnt->num_chars; i++ )	{
		ubyte * ubp;
		int x1, y1;
		ubp = &fnt->pixel_data[fnt->char_data[i].offset];
		if ( x + fnt->char_data[i].byte_width >= fnt->bm_w )	{
			x = 0;
			y += fnt->h + 2;
			if ( y+fnt->h > fnt->bm_h ) {
				Error( LOCATION, "Font too big!\n" );
			}
		}
		fnt->bm_u[i] = x;
		fnt->bm_v[i] = y;

		for( y1=0; y1<fnt->h; y1++ )	{
			for (x1=0; x1<fnt->char_data[i].byte_width; x1++ )	{
				uint c = *ubp++;
				if ( c > 14 ) c = 14;
				fnt->bm_data[(x+x1)+(y+y1)*fnt->bm_w] = (unsigned char)(c);	
			}
		}
		x += fnt->char_data[i].byte_width + 2;
	}

	fnt->bitmap_id = bm_create( 8, fnt->bm_w, fnt->bm_h, fnt->bm_data, BMP_AABITMAP );

	return fontnum;
}

int gr_get_current_fontnum()
{
	if (Current_font == NULL) {
		return -1;
	} else {
		return (Current_font - &Fonts[0]);
	}
}

int gr_get_fontnum(char *filename)
{
	int i;
	for(i = 0; i < Num_fonts; i++)
	{
		if(!strextcmp(Fonts[i].filename, filename))
			return i;
	}

	return -1;
}

void gr_set_font(int fontnum)
{
	if ( fontnum < 0 ) {
		Current_font = NULL;
		Lcl_special_chars = 0;
		return;
	}

	if ( fontnum >= 0 && fontnum < Num_fonts) {
		Current_font = &Fonts[fontnum];
		Lcl_special_chars = lcl_get_font_index(fontnum);
	}
}

void parse_fonts_tbl(char *only_parse_first_font, size_t only_parse_first_font_size)
{
	int rval;
	char *filename;
	
	// choose file name
	// (this can be done within the function, as opposed to being passed as a parameter,
	// because fonts.tbl doesn't have a modular counterpart)
	if ( cf_exists_full("fonts.tbl", CF_TYPE_TABLES) ) {
		filename = "fonts.tbl";
	} else {
		filename = NULL;
	}

	if ((rval = setjmp(parse_abort)) != 0) {
		mprintf(("TABLES: Unable to parse '%s'!  Error code = %i.\n", (filename) ? filename : NOX("<default fonts.tbl>"), rval));
		return;
	}

	if (filename != NULL) {
		read_file_text(filename, CF_TYPE_TABLES);
	} else {
		read_file_text_from_array(defaults_get_file("fonts.tbl"));
	}

	reset_parse();		

	// start parsing
	required_string("#Fonts");

	// read fonts
	while (required_string_either("#End","$Font:")) {
		char font_filename[MAX_FILENAME_LEN];

		// grab font
		required_string("$Font:");
		stuff_string(font_filename, F_NAME, MAX_FILENAME_LEN);

		// if we only need the first font, copy it and bail
		if (only_parse_first_font != NULL) {
			strcpy_s(only_parse_first_font, only_parse_first_font_size, font_filename);
			return;
		}

		// create font
		int font_id = gr_create_font(font_filename);
		if (font_id < 0) {
			Warning(LOCATION, "Could not create font from typeface '%s'!", font_filename);
		}
	}

	// done parsing
	required_string("#End");

	// double check
	if (Num_fonts < 3) {
		Error(LOCATION, "There must be at least three fonts in %s!", (filename) ? filename : NOX("<default fonts.tbl>"));
	}
}

void gr_stuff_first_font(char *first_font, size_t first_font_size )
{
	parse_fonts_tbl( first_font, first_font_size );
}

void gr_font_init()
{
	parse_fonts_tbl( NULL, 0 );
	gr_set_font(0);
}
