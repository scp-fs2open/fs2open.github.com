/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/Font.cpp $
 * $Revision: 2.14 $
 * $Date: 2005-07-02 19:42:15 $
 * $Author: taylor $
 *
 * source file for font stuff
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.13  2005/05/12 17:49:12  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 *
 * Revision 2.12  2005/01/31 23:27:53  taylor
 * merge with Linux/OSX tree - p0131-2
 *
 * Revision 2.11  2004/07/26 20:47:31  Kazan
 * remove MCD complete
 *
 * Revision 2.10  2004/07/17 18:46:07  taylor
 * various OGL and memory leak fixes
 *
 * Revision 2.9  2004/07/12 16:32:48  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 2.8  2004/02/20 04:29:54  bobboau
 * pluged memory leaks,
 * 3D HTL lasers (they work perfictly)
 * and posably fixed Turnsky's shinemap bug
 *
 * Revision 2.7  2004/02/14 00:18:31  randomtiger
 * Please note that from now on OGL will only run with a registry set by Launcher v4. See forum for details.
 * OK, these changes effect a lot of file, I suggest everyone updates ASAP:
 * Removal of many files from project.
 * Removal of meanless Gr_bitmap_poly variable.
 * Removal of glide, directdraw, software modules all links to them, and all code specific to those paths.
 * Removal of redundant Fred paths that arent needed for Fred OGL.
 * Have seriously tidied the graphics initialisation code and added generic non standard mode functionality.
 * Fixed many D3D non standard mode bugs and brought OGL up to the same level.
 * Removed texture section support for D3D8, voodoo 2 and 3 cards will no longer run under fs2_open in D3D, same goes for any card with a maximum texture size less than 1024.
 *
 * Revision 2.6  2004/01/24 12:47:48  randomtiger
 * Font and other small changes for Fred
 *
 * Revision 2.5  2004/01/19 00:56:09  randomtiger
 * Some more small changes for Fred OGL
 *
 * Revision 2.4  2003/03/02 05:41:52  penguin
 * Added some #ifndef NO_SOFTWARE_RENDERING
 *  - penguin
 *
 * Revision 2.3  2002/08/01 01:41:05  penguin
 * The big include file move
 *
 * Revision 2.2  2002/07/29 20:12:31  penguin
 * added #ifdef _WIN32 around windows-specific system headers
 *
 * Revision 2.1  2002/07/07 19:55:59  penguin
 * Back-port to MSVC
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.2  2002/05/16 13:43:25  mharris
 * ifdef WIN32 around (unused?) win-specific code
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 9     7/09/99 10:32p Dave
 * Command brief and red alert screens.
 * 
 * 8     12/14/98 9:21a Dan
 * Put gr8_string() back in
 * 
 * 7     12/02/98 5:47p Dave
 * Put in interface xstr code. Converted barracks screen to new format.
 * 
 * 6     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 5     11/05/98 4:18p Dave
 * First run nebula support. Beefed up localization a bit. Removed all
 * conditional compiles for foreign versions. Modified mission file
 * format.
 * 
 * 4     10/13/98 9:28a Dave
 * Started neatening up freespace.h. Many variables renamed and
 * reorganized. Added AlphaColors.[h,cpp]
 * 
 * 3     10/13/98 9:19a Andsager
 * Add localization support to cfile.  Optional parameter with cfopen that
 * looks for localized files.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 49    7/02/98 3:41p Allender
 * fix nasty assembler bug where pushf wasn't accompanied by corresponding
 * pop in certain clipping conditions -- manifested only in software and
 * usually on the PXO chat screen.
 * 
 * 48    6/18/98 10:10a Allender
 * fixed compiler warnings
 * 
 * 47    6/13/98 10:48p Lawrance
 * Changed code to utilize proper fixed-space 1 character.
 * 
 * 46    6/13/98 3:18p Hoffoss
 * NOX()ed out a bunch of strings that shouldn't be translated.
 * 
 * 45    5/25/98 10:32a John
 * Took out redundant code for font bitmap offsets that converted to a
 * float, then later on converted back to an integer.  Quite unnecessary.
 * 
 * 44    5/12/98 11:45a John
 * Fixed a bug with the length of string being passed to
 * gr_get_string_size.  The bug occurred if you had two or more'\n'''s in
 * a row.
 * 
 * 43    3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 42    3/09/98 6:06p John
 * Restructured font stuff to avoid duplicate code in Direct3D and Glide.
 * Restructured Glide to avoid redundent state setting.
 * 
 * 41    3/02/98 10:33a John
 * Fixed bug where Y clip was using X value.
 * 
 * 40    2/19/98 9:04a John
 * Fixed fonts with glide
 * 
 * 39    2/17/98 7:27p John
 * Got fonts and texturing working in Direct3D
 * 
 * 38    2/05/98 9:21p John
 * Some new Direct3D code.   Added code to monitor a ton of stuff in the
 * game.
 * 
 * 37    1/20/98 2:25p Hoffoss
 * Fixed bug where timestamp printing incorrectly increments an hour at 6
 * minutes.
 * 
 * 36    1/19/98 6:15p John
 * Fixed all my Optimized Build compiler warnings
 * 
 * 35    11/30/97 12:18p John
 * added more 24 & 32-bpp primitives
 * 
 * 34    11/29/97 2:24p John
 * fixed warnings
 * 
 * 33    11/29/97 2:06p John
 * added mode 16-bpp support
 * 
 * 32    11/10/97 2:35p Hoffoss
 * Changed timestamp printing code to utilize new fixed-space '1'
 * character.
 * 
 * 31    11/06/97 5:42p Hoffoss
 * Added support for fixed size timstamp rendering.
 * 
 * 30    11/03/97 10:08p Hoffoss
 * Changed gr_get_string_size to utilize an optional length specifier, if
 * you want to use non-null terminated strings.
 * 
 * 29    11/03/97 10:59a John
 * added support for more than one font.
 * 
 * 28    10/24/97 12:13p Hoffoss
 * Added gr_force_fit_string().
 * 
 * 27    10/19/97 12:55p John
 * new code to lock / unlock surfaces for smooth directx integration.
 * 
 * 26    10/14/97 4:50p John
 * more 16 bpp stuff.
 * 
 * 25    10/14/97 8:08a John
 * added a bunch more 16 bit support
 * 
 * 24    10/09/97 5:23p John
 * Added support for more 16-bpp functions
 * 
 * 23    9/23/97 10:53a Hoffoss
 * Fixed bug in assumptions made half the time, and not the other half.
 * 
 * 22    9/09/97 3:39p Sandeep
 * warning level 4 bugs
 * 
 * 21    8/19/97 5:46p Hoffoss
 * Changed font used in Fred, and added display to show current eye
 * position.
 * 
 * 20    6/25/97 2:35p John
 * added some functions to use the windows font for Fred.
 * 
 * 19    6/17/97 12:03p John
 * Moved color/alphacolor functions into their own module.  Made all color
 * functions be part of the low-level graphics drivers, not just the
 * grsoft.
 * 
 * 18    6/13/97 2:37p John
 * fixed a string bug that printed weird characters sometimes.
 * 
 * 17    6/11/97 6:00p John
 * sped up alpha matching a bit.
 * 
 * 16    6/11/97 5:49p John
 * Changed palette code to only recalculate alphacolors when needed, not
 * when palette changes.
 * 
 * 15    6/11/97 5:01p John
 * Fixed mission log text.  Added fast clipped font drawer.  
 * 
 * 14    6/11/97 4:11p John
 * addec function to get font height
 * 
 * 13    6/11/97 1:12p John
 * Started fixing all the text colors in the game.
 * 
 * 12    6/09/97 9:24a John
 * Changed the way fonts are set.
 * 
 * 11    6/06/97 4:41p John
 * Fixed alpha colors to be smoothly integrated into gr_set_color_fast
 * code.
 * 
 * 10    6/06/97 2:40p John
 * Made all the radar dim in/out
 * 
 * 9     6/05/97 4:53p John
 * First rev of new antialiased font stuff.
 * 
 * 8     5/12/97 12:27p John
 * Restructured Graphics Library to add support for multiple renderers.
 * 
 * 7     4/24/97 11:28a Hoffoss
 * added code to handle gr_String being called before gr_init_font ever
 * was called
 * 
 * 6     4/23/97 5:26p John
 * First rev of new debug console stuff.
 * 
 * 5     4/22/97 12:20p John
 * fixed more resource leaks
 * 
 * 4     4/22/97 10:33a John
 * fixed the 2d resource leaks that Alan found.
 * 
 * 3     4/04/97 11:21a Hoffoss
 * JOHN: Fixed invalid param that caused a trap in BoundsChecker.    
 * 
 * 2     4/01/97 9:26a Allender
 * added support for descent style fonts although they are not used in the
 * game yet
 * 
 * 1     3/31/97 9:42a Allender
 *
 * $NoKeywords: $
 */

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include "graphics/grinternal.h"
#include "graphics/2d.h"
#include "cfile/cfile.h"
#include "graphics/font.h"
#include "palman/palman.h"
#include "io/key.h"
#include "bmpman/bmpman.h"
#include "localization/localize.h"
#include "globalincs/systemvars.h"



int Num_fonts = 0;
font Fonts[MAX_FONTS];
font *Current_font = NULL;


// crops a string if required to force it to not exceed max_width pixels when printed.
// Does this by dropping characters at the end of the string and adding '...' to the end.
//    str = string to crop.  Modifies this string directly
//    max_str = max characters allowed in str
//    max_width = number of pixels to limit string to (less than or equal to).
//    Returns: returns same pointer passed in for str.
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

//takes the character BEFORE being offset into current font
// Returns the letter code
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
				while( (k->c1 == (char)letter) && (k->c2<(char)letter2) && (i<Current_font->num_kern_pairs) )	{
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
int get_centered_x(char *s)
{
	int w,w2,s2;

	for (w=0;*s!=0 && *s!='\n';s++) {
		get_char_width(s[0],s[1],&w2,&s2);
		w += s2;
	}

	return ((gr_screen.clip_width_unscaled - w) / 2);
}

// draws a character centered on x
void gr_char_centered(int x, int y, char chr)
{
	char str[2];
	int w, sc;

	sc = Lcl_special_chars;
	if (chr == '1')
		chr = (char)(sc + 1);

	str[0] = chr;
	str[1] = 0;
	gr_get_string_size(&w, NULL, str);
	gr_string(x - w / 2, y, str);
}

void gr_print_timestamp(int x, int y, int timestamp)
{
	char h[2], m[3], s[3];
	int w, c;

	// format the time information into strings
	sprintf(h, "%.1d", (timestamp / 3600000) % 10);
	sprintf(m, "%.2d", (timestamp / 60000) % 60);
	sprintf(s, "%.2d", (timestamp / 1000) % 60);

	gr_get_string_size(&w, NULL, "0");
	gr_get_string_size(&c, NULL, ":");

	gr_string(x + w, y, ":");
	gr_string(x + w * 3 + c, y, ":");

	x += w / 2;
	gr_char_centered(x, y, h[0]);
	x += w + c;
	gr_char_centered(x, y, m[0]);
	x += w;
	gr_char_centered(x, y, m[1]);
	x += w + c;
	gr_char_centered(x, y, s[0]);
	x += w;
	gr_char_centered(x, y, s[1]);
}

int gr_get_font_height()
{
	if (Current_font)	{
		return Current_font->h;
	} else {
		return 16;
	}
}

void gr_get_string_size(int *w1, int *h1, char *text, int len)
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


MONITOR( FontChars );	


/*

void gr8_char(int x,int y,int letter)
{
	font_char *ch;
	
	ch = &Current_font->char_data[letter];

	gr_aabitmap_ex( x, y, ch->byte_width, Current_font->h, Current_font->u[letter], Current_font->v[letter] );

//	mprintf(( "String = %s\n", text ));
}


void gr8_string( int sx, int sy, char *s )
{
	int width, spacing, letter;
	int x, y;

	if ( !Current_font ) return;
	if ( !s ) return;
	
	gr_set_bitmap(Current_font->bitmap);

	x = sx;
	y = sy;

	if (sx==0x8000) {			//centered
		x = get_centered_x(s);
	} else {
		x = sx;
	}

	while (*s)	{
		while (*s== '\n' )	{
			s++;
			y += Current_font->h;
			if (sx==0x8000) {			//centered
				x = get_centered_x(s);
			} else {
				x = sx;
			}
		}
		if (*s == 0 ) break;

		letter = get_char_width(s[0],s[1],&width,&spacing);

		if (letter<0) {	//not in font, draw as space
			x += spacing;
			s++;
			continue;
		}
		gr8_char( x, y, letter );
	
		x += spacing;
		s++;
	}
}
*/
/*
void gr8_string(int sx, int sy, char *s )
{
	int row,width, spacing, letter;
	int x, y;

	if ( !Current_font ) return;
	if ( !s ) return;

	x = sx;
	y = sy;

	if (sx==0x8000) {			//centered
		x = get_centered_x(s);
	} else {
		x = sx;
	}

	spacing = 0;

	gr_lock();


	while (*s)	{
		MONITOR_INC( FontChars, 1 );	

		x += spacing;
		while (*s== '\n' )	{
			s++;
			y += Current_font->h;
			if (sx==0x8000) {			//centered
				x = get_centered_x(s);
			} else {
				x = sx;
			}
		}
		if (*s == 0 ) break;

		letter = get_char_width(s[0],s[1],&width,&spacing);
		s++;

		//If not in font, draw as space
		if (letter<0) continue;

		int xd, yd, xc, yc;
		int wc, hc;

		// Check if this character is totally clipped
		if ( x + width < gr_screen.clip_left ) continue;
		if ( y + Current_font->h < gr_screen.clip_top ) continue;
		if ( x > gr_screen.clip_right ) continue;
		if ( y > gr_screen.clip_bottom ) continue;

		xd = yd = 0;
		if ( x < gr_screen.clip_left ) xd = gr_screen.clip_left - x;
		if ( y < gr_screen.clip_top ) yd = gr_screen.clip_top - y;
		xc = x+xd;
		yc = y+yd;

		wc = width - xd; hc = Current_font->h - yd;
		if ( xc + wc > gr_screen.clip_right ) wc = gr_screen.clip_right - xc;
		if ( yc + hc > gr_screen.clip_bottom ) hc = gr_screen.clip_bottom - yc;

		if ( wc < 1 ) continue;
		if ( hc < 1 ) continue;

		ubyte *fp = Current_font->pixel_data + Current_font->char_data[letter].offset + xd + yd*width;
		ubyte *dptr = GR_SCREEN_PTR(ubyte, xc, yc);			

#ifndef HARDWARE_ONLY
		if ( Current_alphacolor )	{
			for (row=0; row<hc; row++)	{
				#ifdef USE_INLINE_ASM
					ubyte *lookup = &Current_alphacolor->table.lookup[0][0];
						_asm mov edx, lookup
						_asm xor eax, eax
						_asm mov ecx, wc
						_asm xor ebx, ebx
						_asm mov edi, dptr
						_asm mov esi, fp
						_asm shr ecx, 1
						_asm jz  OnlyOne
						_asm pushf
					InnerFontLoop:
						_asm mov al, [edi]
						_asm mov bl, [edi+1]
						_asm add edi,2

						_asm mov ah, [esi]
						_asm mov bh, [esi+1]
						_asm add esi,2

						_asm mov al, [edx+eax]
						_asm mov ah, [edx+ebx]

						_asm mov [edi-2], ax

						_asm dec ecx
						_asm jnz InnerFontLoop

						_asm popf
						_asm jnc NotOdd

					OnlyOne:
						_asm mov al, [edi]
						_asm mov ah, [esi]
						_asm mov al, [edx+eax]
						_asm mov [edi], al

					NotOdd:
					dptr += gr_screen.rowsize;
					fp += width;
				#else
					int i;
					for (i=0; i< wc; i++ )	{
						*dptr++ = Current_alphacolor->table.lookup[*fp++][*dptr];
					}
					fp += width - wc;
					dptr += gr_screen.rowsize - wc;
				#endif
			}
		} else {		// No alpha color
#endif
			for (row=0; row<hc; row++)	{
				int i;
				for (i=0; i< wc; i++ )	{
					if (*fp > 5 )
						*dptr = gr_screen.current_color.raw8;
					dptr++;
					fp++;
				}
				fp += width - wc;
				dptr += gr_screen.rowsize - wc;
			}
		// }
	}
	gr_unlock();
}
*/

#ifdef _WIN32
HFONT MyhFont = NULL;
HDC hDibDC = NULL;

void gr_string_win(int x, int y, char *s)
{
	int old_bitmap = gr_screen.current_bitmap; 
	gr_set_font(FONT1);
   	gr_string(x,y,s);
	gr_screen.current_bitmap = old_bitmap; 
}

void gr_get_string_size_win(int *w, int *h, char *text)
{
	char *ptr;
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

void _cdecl gr_printf( int x, int y, char * format, ... )
{
	va_list args;

	if ( !Current_font ) return;
	
	va_start(args, format);
	vsprintf(grx_printf_text,format,args);
	va_end(args);

	gr_string(x,y,grx_printf_text);
}

void _cdecl gr_printf_no_resize( int x, int y, char * format, ... )
{
	va_list args;

	if ( !Current_font ) return;
	
	va_start(args, format);
	vsprintf(grx_printf_text,format,args);
	va_end(args);

	gr_string(x,y,grx_printf_text,false);
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

// Returns -1 if couldn't init font, otherwise returns the
// font id number.
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

	if ( fontnum==MAX_FONTS )	{
		Error( LOCATION, "Too many fonts!\nSee John, or change MAX_FONTS in Graphics\\Font.h\n" );
	}

	if ( fontnum == Num_fonts )	{
		Num_fonts++;
	}
	
	bool localize = true;

	fp = cfopen( typeface, "rb", CFILE_NORMAL, CF_TYPE_ANY, localize );
	if ( fp == NULL ) return -1;

	strncpy( fnt->filename, typeface, MAX_FILENAME_LEN );
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

	fnt->id = INTEL_SHORT( fnt->id );
	fnt->version = INTEL_INT( fnt->version );
	fnt->num_chars = INTEL_INT( fnt->num_chars );
	fnt->first_ascii = INTEL_INT( fnt->first_ascii );
	fnt->w = INTEL_INT( fnt->w );
	fnt->h = INTEL_INT( fnt->h );
	fnt->num_kern_pairs = INTEL_INT( fnt->num_kern_pairs );
	fnt->kern_data_size = INTEL_INT( fnt->kern_data_size );
	fnt->char_data_size = INTEL_INT( fnt->char_data_size );
	fnt->pixel_data_size = INTEL_INT( fnt->pixel_data_size );

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
			fnt->char_data[i].spacing = INTEL_INT( fnt->char_data[i].spacing );
			fnt->char_data[i].byte_width = INTEL_INT( fnt->char_data[i].byte_width );
			fnt->char_data[i].offset = INTEL_INT( fnt->char_data[i].offset );
			fnt->char_data[i].kerning_entry = INTEL_INT( fnt->char_data[i].kerning_entry );
			fnt->char_data[i].user_data = INTEL_SHORT( fnt->char_data[i].user_data );
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
	if ( fnt->pixel_data_size < 64*64 )	{
		w = h = 64;
	} else if ( fnt->pixel_data_size < 128*128 )	{
		w = h = 128;
	} else {
		w = h = 256;
	}

	fnt->bm_w = w;
	fnt->bm_h = h;
	fnt->bm_data = (ubyte *)vm_malloc(fnt->bm_w*fnt->bm_h);
	fnt->bm_u = (int *)vm_malloc(sizeof(int)*fnt->num_chars);
	fnt->bm_v = (int *)vm_malloc(sizeof(int)*fnt->num_chars);

	int i,x,y;
	x = y = 0;
	for (i=0; i<fnt->num_chars; i++ )	{
		ubyte * fp;
		int x1, y1;
		fp = &fnt->pixel_data[fnt->char_data[i].offset];
		if ( x + fnt->char_data[i].byte_width >= fnt->bm_w )	{
			x = 0;
			y += fnt->h;
			if ( y+fnt->h > fnt->bm_h ) {
				Error( LOCATION, "Font too big!\n" );
			}
		}
		fnt->bm_u[i] = x;
		fnt->bm_v[i] = y;

		for( y1=0; y1<fnt->h; y1++ )	{
			for (x1=0; x1<fnt->char_data[i].byte_width; x1++ )	{
				uint c = *fp++;
				if ( c > 14 ) c = 14;
				fnt->bm_data[(x+x1)+(y+y1)*fnt->bm_w] = (unsigned char)(c);	
			}
		}
		x += fnt->char_data[i].byte_width;
	}

	fnt->bitmap_id = bm_create( 8, fnt->bm_w, fnt->bm_h, fnt->bm_data, BMP_AABITMAP );

	return fontnum;
}

void grx_set_font(int fontnum)
{
	if ( fontnum < 0 ) {
		Current_font = NULL;
		return;
	}

	if ( fontnum >= 0 ) {
		Current_font = &Fonts[fontnum];
	}
}

void gr_font_init()
{
	gr_init_font( NOX("font01.vf") );
	gr_init_font( NOX("font02.vf") );
	gr_init_font( NOX("font03.vf") );
}

// Returns -1 if couldn't init font, otherwise returns the
// font id number.
int gr_init_font(char * typeface)
{
	int Loaded_fontnum;

	Loaded_fontnum = gr_create_font(typeface);

	Assert( Loaded_fontnum > -1 );

	gr_set_font( Loaded_fontnum );

	return Loaded_fontnum;
}
