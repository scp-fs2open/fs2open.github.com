/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/Colors.cpp $
 * $Revision: 2.1 $
 * $Date: 2002-08-01 01:41:05 $
 * $Author: penguin $
 *
 * Functions to deal with colors & alphacolors
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.4  2002/05/26 14:10:29  mharris
 * More testing
 *
 * Revision 1.3  2002/05/24 16:45:06  mharris
 * Increased MAX_ALPHACOLORS (dunno why unix port affected this but...)
 *
 * Revision 1.2  2002/05/21 15:40:04  mharris
 * Added some debug mprintfs
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 7     7/08/99 10:52a Dave
 * New multiplayer interpolation scheme. Not 100% done yet, but still
 * better than the old way.
 * 
 * 6     1/14/99 12:48a Dave
 * Todo list bug fixes. Made a pass at putting briefing icons back into
 * FRED. Sort of works :(
 * 
 * 5     12/02/98 5:47p Dave
 * Put in interface xstr code. Converted barracks screen to new format.
 * 
 * 4     11/30/98 5:31p Dave
 * Fixed up Fred support for software mode.
 * 
 * 3     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 35    5/19/98 3:32p John
 * upped alpha colors
 * 
 * 34    5/13/98 10:22p John
 * Added cfile functions to read/write rle compressed blocks of data.
 * Made palman use it for .clr files.  Made alphacolors calculate on the
 * fly rather than caching to/from disk.
 * 
 * 33    4/30/98 4:53p John
 * Restructured and cleaned up cfile code.  Added capability to read off
 * of CD-ROM drive and out of multiple pack files.
 * 
 * 32    4/10/98 5:20p John
 * Changed RGB in lighting structure to be ubytes.  Removed old
 * not-necessary 24 bpp software stuff.
 * 
 * 31    4/09/98 11:04p John
 * Changed ID's in output files to make more sense.
 * 
 * 30    4/01/98 3:05p Adam
 * Reduced gamma on text.
 * 
 * 29    3/29/98 2:36p John
 * fixed bug with reloading fullscreen colros
 * 
 * 28    3/25/98 8:07p John
 * Restructured software rendering into two modules; One for windowed
 * debug mode and one for DirectX fullscreen.   
 * 
 * 27    3/24/98 4:22p John
 * fixed warning
 * 
 * 26    3/24/98 3:58p John
 * Put in (hopefully) final gamma setting code.
 * 
 * 25    3/10/98 4:18p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 24    2/07/98 7:50p John
 * Added code so that we can use the old blending type of alphacolors if
 * we want to.  Made the stars use them.
 * 
 * 23    1/13/98 10:20a John
 * Added code to support "glass" in alphacolors
 * 
 * 22    1/02/98 9:10p Lawrance
 * Big changes to how colors get set on the HUD.
 * 
 * 21    12/30/97 4:32p John
 * 
 * 20    12/02/97 3:59p John
 * Added first rev of thruster glow, along with variable levels of
 * translucency, which retquired some restructing of palman.
 * 
 * 19    11/30/97 4:26p John
 * Added 32-bpp antialiased line.   Took gamma out of alphacolor
 * calculation.
 * 
 * 18    11/29/97 2:06p John
 * added mode 16-bpp support
 * 
 * 17    11/21/97 11:32a John
 * Added nebulas.   Fixed some warpout bugs.
 * 
 * 16    11/14/97 12:30p John
 * Fixed some DirectX bugs.  Moved the 8-16 xlat tables into Graphics
 * libs.  Made 16-bpp DirectX modes know what bitmap format they're in.
 * 
 * 15    11/05/97 11:20p Lawrance
 * increase number of alpha colors to 52
 * 
 * 14    11/04/97 6:32p Hoffoss
 * Changes to hotkey screen.  Needed to add new colors for
 * Color_text_active*.
 * 
 * 13    10/31/97 10:47a John
 * upped clr version to force rebuild after changing palette code.
 * 
 * 12    10/14/97 4:50p John
 * more 16 bpp stuff.
 * 
 * 11    10/14/97 8:08a John
 * added a bunch more 16 bit support
 * 
 * 10    10/09/97 5:23p John
 * Added support for more 16-bpp functions
 * 
 * 9     10/03/97 9:10a John
 * added better antialiased line drawer
 * 
 * 8     9/20/97 8:16a John
 * Made .clr files go into the Cache directory. Replaced cfopen(name,NULL)
 * to delete a file with cf_delete.
 * 
 * 7     9/09/97 10:46a Sandeep
 * fixed warning level 4
 * 
 * 6     7/18/97 12:40p John
 * cached alphacolors to disk.  Also made cfopen be able to delete a file
 * by passing NULL for mode.
 * 
 * 5     7/16/97 3:07p John
 * 
 * 4     6/18/97 12:07p John
 * fixed some color bugs
 * 
 * 3     6/18/97 9:46a John
 * added first rev of d3d shader.   Made HUD target use medium detail
 * models.  Took out some color debug messages.
 * 
 * 2     6/17/97 7:04p John
 * added d3d support for gradients.
 * fixed some color bugs by adding screen signatures instead of watching
 * flags and palette changes.
 * 
 * 1     6/17/97 12:01p John
 *
 * $NoKeywords: $
 */

#include "globalincs/pstypes.h"
#include "graphics/2d.h"
#include "graphics/grinternal.h"
#include "graphics/colors.h"
#include "palman/palman.h"
#include "cfile/cfile.h"
#include "globalincs/systemvars.h"

//#define MAX_ALPHACOLORS 36
#define MAX_ALPHACOLORS 256

alphacolor Alphacolors[MAX_ALPHACOLORS];
static int Alphacolors_intited = 0;

alphacolor * Current_alphacolor = NULL;


void calc_alphacolor_hud_type( alphacolor * ac )
{
#ifndef HARDWARE_ONLY
	int i,j;
	int tr,tg,tb, Sr, Sg, Sb;
	ubyte * pal;
	int r, g, b, alpha;
	float falpha;

	Assert(Alphacolors_intited);

//	mprintf(( "Calculating alphacolor for %d,%d,%d,%d\n", ac->r, ac->g, ac->b, ac->alpha ));

	falpha = i2fl(ac->alpha)/255.0f;
	if ( falpha<0.0f ) falpha = 0.0f; else if ( falpha > 1.0f ) falpha = 1.0f;

	alpha = ac->alpha >> 4;
	if (alpha < 0 ) alpha = 0; else if (alpha > 15 ) alpha = 15;
	r = ac->r;
	if (r < 0 ) r = 0; else if (r > 255 ) r = 255;
	g = ac->g;
	if (g < 0 ) g = 0; else if (g > 255 ) g = 255;
	b = ac->b;
	if (b < 0 ) b = 0; else if (b > 255 ) b = 255;

	int ii[16];

	for (j=1; j<15; j++ )	{

		// JAS: Use 1.5/Gamma instead of 1/Gamma because on Adam's
		// PC a gamma of 1.2 makes text look good, but his gamma is
		// really 1.8.   1.8/1.2 = 1.5
		float factor = falpha * (float)pow(i2fl(j)/14.0f, 1.5f/Gr_gamma);
		//float factor = i2fl(j)/14.0f;

		tr = fl2i( i2fl(r) * factor );
		tg = fl2i( i2fl(g) * factor );
		tb = fl2i( i2fl(b) * factor );

		ii[j] = tr;
		if ( tg > ii[j] )	ii[j] = tg;
		if ( tb > ii[j] )	ii[j] = tb;
	}

	pal = gr_palette;

	int m = r;
	if ( g > m ) m = g;
	if ( b > m ) m = b;

	ubyte ri[256], gi[256], bi[256];

	if ( m > 0 )	{
		for (i=0; i<256; i++ )	{
			ri[i] = ubyte((i*r)/m);
			gi[i] = ubyte((i*g)/m);
			bi[i] = ubyte((i*b)/m);
		}
	} else {
		for (i=0; i<256; i++ )	{
			ri[i] = 0;
			gi[i] = 0;
			bi[i] = 0;
		}
	}

	for (i=0; i<256; i++ )	{
		Sr = pal[0];
		Sg = pal[1];
		Sb = pal[2];
		pal += 3;

		int dst_intensity = Sr;
		if ( Sg > dst_intensity ) dst_intensity = Sg;
		if ( Sb > dst_intensity ) dst_intensity = Sb;

		ac->table.lookup[0][i] = (unsigned char)i;

		for (j=1; j<15; j++ )	{

			int tmp_i = max( ii[j], dst_intensity );

			ac->table.lookup[j][i] = (unsigned char)palette_find(ri[tmp_i],gi[tmp_i],bi[tmp_i]);
		}

		float di = (i2fl(Sr)*.30f+i2fl(Sg)*0.60f+i2fl(Sb)*.10f)/255.0f;
		float factor = 0.0f + di*0.75f;

		tr = fl2i( factor*i2fl(r)*falpha );
		tg = fl2i( factor*i2fl(g)*falpha );
		tb = fl2i( factor*i2fl(b)*falpha );

		if ( tr > 255 ) tr = 255; else if ( tr < 0 ) tr = 0;
		if ( tg > 255 ) tg = 255; else if ( tg < 0 ) tg = 0; 
		if ( tb > 255 ) tb = 255; else if ( tb < 0 ) tb = 0;

		ac->table.lookup[15][i] = (unsigned char)palette_find(tr,tg,tb);
		//ac->table.lookup[15][i] = (unsigned char)palette_find(255,0,0);

	}
#endif
}

// Old way to calculate alpha colors

void calc_alphacolor_blend_type( alphacolor * ac )
{
#ifndef HARDWARE_ONLY
	int i,j;
	int tr,tg,tb, Sr, Sg, Sb;
	int Dr, Dg, Db;
	ubyte * pal;
	int r, g, b, alpha;

	Assert(Alphacolors_intited);

//	mprintf(( "Calculating alphacolor for %d,%d,%d,%d\n", ac->r, ac->g, ac->b, ac->alpha ));

	alpha = ac->alpha >> 4;
	if (alpha < 0 ) alpha = 0; else if (alpha > 15 ) alpha = 15;
	r = ac->r;
	if (r < 0 ) r = 0; else if (r > 255 ) r = 255;
	g = ac->g;
	if (g < 0 ) g = 0; else if (g > 255 ) g = 255;
	b = ac->b;
	if (b < 0 ) b = 0; else if (b > 255 ) b = 255;

	int gamma_j1[16];

	for (j=1; j<16; j++ )	{
		// JAS: Use 1.5/Gamma instead of 1/Gamma because on Adam's
		// PC a gamma of 1.2 makes text look good, but his gamma is
		// really 1.8.   1.8/1.2 = 1.5
		gamma_j1[j] = (int)((pow(i2fl(j)/15.0f, 1.5f/Gr_gamma)*16.0f) + 0.5);
	}

	pal = gr_palette;

	for (i=0; i<256; i++ )	{
		Sr = pal[0];
		Sg = pal[1];
		Sb = pal[2];
		pal += 3;
	
		Dr = ( Sr*(16-alpha) + (r*alpha) ) >> 4;
		Dg = ( Sg*(16-alpha) + (g*alpha) ) >> 4;
		Db = ( Sb*(16-alpha) + (b*alpha) ) >> 4;

		ac->table.lookup[0][i] = (unsigned char)i;

		for (j=1; j<16; j++ )	{

			int j1 = gamma_j1[j];

			tr = ( Sr*(16-j1) + (Dr*j1) ) >> 4;
			tg = ( Sg*(16-j1) + (Dg*j1) ) >> 4;
			tb = ( Sb*(16-j1) + (Db*j1) ) >> 4;

			if ( tr > 255 ) tr = 255; else if ( tr < 0 ) tr = 0;
			if ( tg > 255 ) tg = 255; else if ( tg < 0 ) tg = 0; 
			if ( tb > 255 ) tb = 255; else if ( tb < 0 ) tb = 0;

			ac->table.lookup[j][i] = (unsigned char)palette_find(tr,tg,tb);
		}
	}
#endif
}

void calc_alphacolor( alphacolor * ac )
{
	switch(ac->type)	{
	case AC_TYPE_HUD:
		calc_alphacolor_hud_type(ac);
		break;
	case AC_TYPE_BLEND:
		calc_alphacolor_blend_type(ac);
		break;
	default:
		Int3();		// Passing an invalid type of alphacolor!
	}
}

void grx_init_alphacolors()
{
	int i;
	
	Alphacolors_intited = 1;

	for (i=0; i<MAX_ALPHACOLORS; i++ )	{
		Alphacolors[i].used = 0;
		Alphacolors[i].clr = NULL;
	}

	Current_alphacolor = NULL;
}



void grx_init_alphacolor( color *clr, int r, int g, int b, int alpha, int type )
{
	int n;
	alphacolor *ac;
	
	if(Game_mode & GM_STANDALONE_SERVER){
		return;
	}

	if (!Alphacolors_intited) return;

	if ( alpha < 0 ) alpha = 0;
	if ( alpha > 255 ) alpha = 255;

	n = -1;
	if ( (clr->magic == 0xAC01) && (clr->is_alphacolor) )	{
		if ( (clr->alphacolor >= 0) && (clr->alphacolor < MAX_ALPHACOLORS))	{
			if ( Alphacolors[clr->alphacolor].used && (Alphacolors[clr->alphacolor].clr==clr) )	{
				n = clr->alphacolor;
			}
		}
	}

	int changed = 0;

	if ( n==-1 )	{
		for (n=0; n<MAX_ALPHACOLORS; n++ )	{
			if (!Alphacolors[n].used) {
				mprintf(("Creating alphacolor #%d %02x/%02x/%02x/%02x\n",
							n, r, g, b, alpha));
				break;
			}
		}
		if ( n == MAX_ALPHACOLORS )	
			Error( LOCATION, "Out of alphacolors!\n" );
	} else {
		changed = 1;
	}


	// Create the alphacolor
	ac = &Alphacolors[n];

	if ( changed && (ac->r!=r || ac->g!=g || ac->b!=b || ac->alpha!=alpha || ac->type != type ) )	{
		// we're changing the color, so delete the old cache file
		//mprintf(( "Changing ac from %d,%d,%d,%d to %d,%d,%d,%d\n", ac->r, ac->g, ac->b, ac->alpha, r, g, b, alpha ));
		//ac_delete_cached(ac);
	}

	ac->used = 1;
	ac->r = r;
	ac->g = g;
	ac->b = b;
	ac->alpha = alpha;
	ac->type = type;
	ac->clr=clr;
	
	calc_alphacolor(ac);

	grx_init_color( clr, r, g, b );

	// Link the alphacolor to the color
	clr->alpha = (unsigned char)alpha;
	clr->ac_type = (ubyte)type;
	clr->alphacolor = n;
	clr->is_alphacolor = 1;
}


void grx_get_color( int * r, int * g, int * b )
{
	if (r) *r = gr_screen.current_color.red;
	if (g) *g = gr_screen.current_color.green;
	if (b) *b = gr_screen.current_color.blue;
}

void grx_init_color( color * dst, int r, int g, int b )
{
	dst->screen_sig = gr_screen.signature;
	dst->red = (unsigned char)r;
	dst->green = (unsigned char)g;
	dst->blue = (unsigned char)b;
	dst->alpha = 255;
	dst->ac_type = AC_TYPE_NONE;
	dst->is_alphacolor = 0;
	dst->alphacolor = -1;
	dst->magic = 0xAC01;

	dst->raw8 = (unsigned char)palette_find( r, g, b );
}

void grx_set_color_fast( color * dst )
{
//  	mprintf(("grx_set_color_fast: %02x/%02x/%02x/%02x (isalpha=%d, magic=%04x)\n",
//  				dst->red, dst->green, dst->blue, dst->alpha, 
//  				dst->is_alphacolor, dst->magic));

	if ( dst->magic != 0xAC01 ) return;

	if ( dst->screen_sig != gr_screen.signature )	{
		if ( dst->is_alphacolor )	{
			grx_init_alphacolor( dst, dst->red, dst->green, dst->blue, dst->alpha, dst->ac_type );
		} else {
			grx_init_color( dst, dst->red, dst->green, dst->blue );
		}
	}

	gr_screen.current_color = *dst;
	
	if ( dst->is_alphacolor )	{
		Assert( dst->alphacolor > -1 );
		Assert( dst->alphacolor <= MAX_ALPHACOLORS );


///FIXME--THIS IS TEMP ONLY
///FIXME--THIS IS TEMP ONLY
///FIXME--THIS IS TEMP ONLY
//		Assert( Alphacolors[dst->alphacolor].used );
///////////////////////



		// Current_alphacolor = &Alphacolors[dst->alphacolor];
		Current_alphacolor = NULL;
	} else {
		Current_alphacolor = NULL;
	}
}


void grx_set_color( int r, int g, int b )
{
//	mprintf(("grx_set_color: %02x/%02x/%02x\n", r, g, b));

	Assert((r >= 0) && (r < 256));
	Assert((g >= 0) && (g < 256));
	Assert((b >= 0) && (b < 256));

//	if ( r!=0 || g!=0 || b!=0 )	{
//		mprintf(( "Setcolor: %d,%d,%d\n", r,g,b ));
//	}
	grx_init_color( &gr_screen.current_color, r, g, b );
	Current_alphacolor = NULL;
}

void calc_alphacolor_hud_type_old( alphacolor_old * ac )
{
	int i,j;
	int tr,tg,tb, Sr, Sg, Sb;
	ubyte * pal;
	int r, g, b, alpha;
	float falpha;

	// Assert(Alphacolors_intited);

//	mprintf(( "Calculating alphacolor for %d,%d,%d,%d\n", ac->r, ac->g, ac->b, ac->alpha ));

	falpha = i2fl(ac->alpha)/255.0f;
	if ( falpha<0.0f ) falpha = 0.0f; else if ( falpha > 1.0f ) falpha = 1.0f;

	alpha = ac->alpha >> 4;
	if (alpha < 0 ) alpha = 0; else if (alpha > 15 ) alpha = 15;
	r = ac->r;
	if (r < 0 ) r = 0; else if (r > 255 ) r = 255;
	g = ac->g;
	if (g < 0 ) g = 0; else if (g > 255 ) g = 255;
	b = ac->b;
	if (b < 0 ) b = 0; else if (b > 255 ) b = 255;

	int ii[16];

	for (j=1; j<15; j++ )	{

		// JAS: Use 1.5/Gamma instead of 1/Gamma because on Adam's
		// PC a gamma of 1.2 makes text look good, but his gamma is
		// really 1.8.   1.8/1.2 = 1.5
		float factor = falpha * (float)pow(i2fl(j)/14.0f, 1.5f/Gr_gamma);
		//float factor = i2fl(j)/14.0f;

		tr = fl2i( i2fl(r) * factor );
		tg = fl2i( i2fl(g) * factor );
		tb = fl2i( i2fl(b) * factor );

		ii[j] = tr;
		if ( tg > ii[j] )	ii[j] = tg;
		if ( tb > ii[j] )	ii[j] = tb;
	}

	pal = gr_palette;

	int m = r;
	if ( g > m ) m = g;
	if ( b > m ) m = b;

	ubyte ri[256], gi[256], bi[256];

	if ( m > 0 )	{
		for (i=0; i<256; i++ )	{
			ri[i] = ubyte((i*r)/m);
			gi[i] = ubyte((i*g)/m);
			bi[i] = ubyte((i*b)/m);
		}
	} else {
		for (i=0; i<256; i++ )	{
			ri[i] = 0;
			gi[i] = 0;
			bi[i] = 0;
		}
	}

	for (i=0; i<256; i++ )	{
		Sr = pal[0];
		Sg = pal[1];
		Sb = pal[2];
		pal += 3;

		int dst_intensity = Sr;
		if ( Sg > dst_intensity ) dst_intensity = Sg;
		if ( Sb > dst_intensity ) dst_intensity = Sb;

		ac->table.lookup[0][i] = (unsigned char)i;

		for (j=1; j<15; j++ )	{

			int tmp_i = max( ii[j], dst_intensity );

			ac->table.lookup[j][i] = (unsigned char)palette_find(ri[tmp_i],gi[tmp_i],bi[tmp_i]);
		}

		float di = (i2fl(Sr)*.30f+i2fl(Sg)*0.60f+i2fl(Sb)*.10f)/255.0f;
		float factor = 0.0f + di*0.75f;

		tr = fl2i( factor*i2fl(r)*falpha );
		tg = fl2i( factor*i2fl(g)*falpha );
		tb = fl2i( factor*i2fl(b)*falpha );

		if ( tr > 255 ) tr = 255; else if ( tr < 0 ) tr = 0;
		if ( tg > 255 ) tg = 255; else if ( tg < 0 ) tg = 0; 
		if ( tb > 255 ) tb = 255; else if ( tb < 0 ) tb = 0;

		ac->table.lookup[15][i] = (unsigned char)palette_find(tr,tg,tb);
		//ac->table.lookup[15][i] = (unsigned char)palette_find(255,0,0);
	}
}

void calc_alphacolor_old(alphacolor_old *ac)
{
	Assert(Fred_running);
	calc_alphacolor_hud_type_old(ac);
}
