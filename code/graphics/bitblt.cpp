/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/Bitblt.cpp $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:22 $
 * $Author: penguin $
 *
 * Code to do software bitblt type stuff
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.2  2002/05/03 22:07:08  mharris
 * got some stuff to compile
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 3     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:48a Dave
 * 
 * 2     4/14/98 12:15p John
 * Made 16-bpp movies work.
 * 
 * 1     3/25/98 8:07p John
 * Split software renderer into Win32 and DirectX
 *
 * $NoKeywords: $
 */

#include "pstypes.h"
#include "osapi.h"
#include "2d.h"
#include "bmpman.h"
#include "key.h"
#include "floating.h"
#include "palman.h"
#include "grsoft.h"
#include "grinternal.h"

// Headers for 2d functions
#include "bitblt.h"

MONITOR( Num2dBitmaps );	

void gr8_aabitmap_ex(int x,int y,int w,int h,int sx,int sy)
{
#if 0
	int hi;
	bitmap * bmp;

	if ( w < 1 ) return;
	if ( h < 1 ) return;

	MONITOR_INC( Num2dBitmaps, 1 );	

	if ( !Current_alphacolor )	return;

	//	mprintf(( "x=%d, y=%d, w=%d, h=%d\n", x, y, w, h ));
	//	mprintf(( "sx=%d, sy=%d, bw=%d, bh=%d\n", sx, sy, bmp->w, bmp->h ));

	bmp = bm_lock( gr_screen.current_bitmap, 8, BMP_RLE|BMP_NO_PALETTE_MAP );	 

	gr_lock();

	if (bmp->flags & BMP_RLE)	{
		int * offsets = (int *)(bmp->data);
		ubyte *lookup = &Current_alphacolor->table.lookup[0][0];
			
		for (hi=0; hi < h; hi++ )    {
			ubyte * dp = GR_SCREEN_PTR(ubyte,x,y+hi);
			ubyte * sp = (ubyte *)bmp->data + offsets[sy+hi];

			int x1 = sx;
			int x2 = sx+w;
			int i = 0;			

			while(i<x1)	{ 			
				int count;

				count = int(*sp++);
				int run_span = count & 0x80;

				count = (count & (~0x80))+1;		
				if ( i+count > x1 ) {
					// This span crosses X1.. so skip some and draw some
					if ( i+count >= x2 ) {
						count = x2 - i;
					}

					if ( run_span )	{
						// RLE'd data
						ubyte c = *sp++;

						if ( c > 0 )	{					
							// We have 'count' pixels of c
							ubyte *tmp_lookup = &lookup[c<<8];
							while( count-- ) {
								if ( i >= x1 )	{
									*dp = tmp_lookup[*dp];
									dp++;
								}
								i++;
							}	
						} else {
							while( count-- ) {
								if ( i >= x1 )	{
									dp++;
								}
								i++;
							}	
						}
					} else {
						// non RLE'd data

						// We have 'count' un-rle'd pixels
						while( count-- ) {
							if ( i >= x1 )	{
								ubyte c = *sp;
								*dp = lookup[(c<<8) | *dp];
								dp++;
							}
							sp++;
							i++;
						} 	
					}

				} else {
					i += count;
					if ( run_span )	{
						// RLE'd data
						sp++; 
					} else {
						sp += count;
					}
				}
			}

			while(i<x2)	{ 			
				int count;

				count = int(*sp++);
				int run_span = count & 0x80;

				count = (count & (~0x80))+1;		
				if ( i+count >= x2 ) {
					count = x2 - i;
				}
				i += count;

				ubyte *end_ptr = dp + count;
				
				if ( count > 0 )	{
					if ( run_span )	{
						// RLE'd data
						ubyte c = *sp++;

						if ( c > 0 )	{					
							// We have 'count' pixels of c
							ubyte *tmp_lookup = &lookup[c<<8];

							while( count >= 4 )	{
								// We have to plot at least 4 pixels of constant color starting at 'dp'
								dp[0] = tmp_lookup[dp[0]];
								dp[1] = tmp_lookup[dp[1]];
								dp[2] = tmp_lookup[dp[2]];
								dp[3] = tmp_lookup[dp[3]];

								count -= 4;
								dp += 4;
							} 

							while ( count > 0 )	{
								*dp = tmp_lookup[*dp];
								dp++;
								count--;
							}
						} 
					} else {
						// non RLE'd data

						// We have 'count' un-rle'd pixels
						do {
							ubyte c = *sp++;
							*dp = lookup[(c<<8) | *dp];
							dp++;
						} while ( dp < end_ptr );
					}
				}

				dp = end_ptr;
			}

		}
	} else {
		ubyte * sptr = (ubyte *)( bmp->data + (sy*bmp->w+sx) );
		ubyte *lookup = (ubyte *)&Current_alphacolor->table.lookup[0][0];

		for (hi=0; hi<h; hi++ )	{
			int j;
			ubyte c, * sp = sptr;	
			ubyte * dp = GR_SCREEN_PTR(ubyte,x,hi+y);
			for (j=0; j<w; j++ )	{
				c = *sp++;
				if ( c > 0 ) {
					*dp = lookup[(c<<8) | *dp];
				}
				dp++;
			}
			sptr += bmp->w;
		}
	}

	gr_unlock();
	bm_unlock(gr_screen.current_bitmap);
#endif
}

void grx_aabitmap_ex(int x,int y,int w,int h,int sx,int sy)
{
	int reclip;
	#ifndef NDEBUG
	int count = 0;
	#endif

	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;

	int bw, bh;
	bm_get_info( gr_screen.current_bitmap, &bw, &bh, NULL );

	do {
		reclip = 0;
		#ifndef NDEBUG
			if ( count > 1 ) Int3();
			count++;
		#endif
	
		if ((dx1 > gr_screen.clip_right ) || (dx2 < gr_screen.clip_left)) return;
		if ((dy1 > gr_screen.clip_bottom ) || (dy2 < gr_screen.clip_top)) return;
		if ( dx1 < gr_screen.clip_left ) { sx += gr_screen.clip_left-dx1; dx1 = gr_screen.clip_left; }
		if ( dy1 < gr_screen.clip_top ) { sy += gr_screen.clip_top-dy1; dy1 = gr_screen.clip_top; }
		if ( dx2 > gr_screen.clip_right )	{ dx2 = gr_screen.clip_right; }
		if ( dy2 > gr_screen.clip_bottom )	{ dy2 = gr_screen.clip_bottom; }

		if ( sx < 0 ) {
			dx1 -= sx;
			sx = 0;
			reclip = 1;
		}

		if ( sy < 0 ) {
			dy1 -= sy;
			sy = 0;
			reclip = 1;
		}

		w = dx2-dx1+1;
		h = dy2-dy1+1;

		if ( sx + w > bw ) {
			w = bw - sx;
			dx2 = dx1 + w - 1;
		}

		if ( sy + h > bh ) {
			h = bh - sy;
			dy2 = dy1 + h - 1;
		}

		if ( w < 1 ) return;		// clipped away!
		if ( h < 1 ) return;		// clipped away!

	} while (reclip);

	// Make sure clipping algorithm works
	#ifndef NDEBUG
		Assert( w > 0 );
		Assert( h > 0 );
		Assert( w == (dx2-dx1+1) );
		Assert( h == (dy2-dy1+1) );
		Assert( sx >= 0 );
		Assert( sy >= 0 );
		Assert( sx+w <= bw );
		Assert( sy+h <= bh );
		Assert( dx2 >= dx1 );
		Assert( dy2 >= dy1 );
		Assert( (dx1 >= gr_screen.clip_left ) && (dx1 <= gr_screen.clip_right) );
		Assert( (dx2 >= gr_screen.clip_left ) && (dx2 <= gr_screen.clip_right) );
		Assert( (dy1 >= gr_screen.clip_top ) && (dy1 <= gr_screen.clip_bottom) );
		Assert( (dy2 >= gr_screen.clip_top ) && (dy2 <= gr_screen.clip_bottom) );
	#endif

	// We now have dx1,dy1 and dx2,dy2 and sx, sy all set validly within clip regions.

	// Draw bitmap bm[sx,sy] into (dx1,dy1)-(dx2,dy2)

	switch( gr_screen.bits_per_pixel )	{
	case 8:	
		gr8_aabitmap_ex(dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy);
		break;

	default:
		Error( LOCATION, "Unsupported BPP=%d in grx_aabitmap_ex!\n", gr_screen.bytes_per_pixel );
	}


}


void gr8_bitmap_ex(int x,int y,int w,int h,int sx,int sy)
{
#if 0
	MONITOR_INC( Num2dBitmaps, 1 );	

	gr_lock();

	// Normal bitblt
	int i;
	bitmap * bmp;
	ubyte * sptr;

	bmp = bm_lock( gr_screen.current_bitmap, 8, 0 );
	sptr = (ubyte *)( bmp->data + (sy*bmp->w+sx) );

	//mprintf(( "x=%d, y=%d, w=%d, h=%d\n", x, y, w, h ));
	//mprintf(( "sx=%d, sy=%d, bw=%d, bh=%d\n", sx, sy, bmp->w, bmp->h ));

	if ( bmp->flags & BMP_XPARENT )	{
		for (i=0; i<h; i++ )	{
			int j;
			ubyte c, * sp = sptr;	
			ubyte * dp = GR_SCREEN_PTR(ubyte,x,i+y);
			for (j=0; j<w; j++ )	{
				c = *sp++;
				if (c != 255) *dp = c;
				dp++;
			}
			sptr += bmp->w;
		}
	} else {
		for (i=0; i<h; i++ )	{
			ubyte *dptr = GR_SCREEN_PTR(ubyte,x,i+y);
			memcpy( dptr, sptr, w );
			sptr += bmp->w;
		}
	}
	bm_unlock(gr_screen.current_bitmap);

	gr_unlock();
#endif
}



void grx_bitmap_ex(int x,int y,int w,int h,int sx,int sy)
{
	int reclip;
	#ifndef NDEBUG
	int count = 0;
	#endif

	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;

	int bw, bh;
	bm_get_info( gr_screen.current_bitmap, &bw, &bh, NULL );

	do {
		reclip = 0;
		#ifndef NDEBUG
			if ( count > 1 ) Int3();
			count++;
		#endif
	
		if ((dx1 > gr_screen.clip_right ) || (dx2 < gr_screen.clip_left)) return;
		if ((dy1 > gr_screen.clip_bottom ) || (dy2 < gr_screen.clip_top)) return;
		if ( dx1 < gr_screen.clip_left ) { sx += gr_screen.clip_left-dx1; dx1 = gr_screen.clip_left; }
		if ( dy1 < gr_screen.clip_top ) { sy += gr_screen.clip_top-dy1; dy1 = gr_screen.clip_top; }
		if ( dx2 > gr_screen.clip_right )	{ dx2 = gr_screen.clip_right; }
		if ( dy2 > gr_screen.clip_bottom )	{ dy2 = gr_screen.clip_bottom; }

		if ( sx < 0 ) {
			dx1 -= sx;
			sx = 0;
			reclip = 1;
		}

		if ( sy < 0 ) {
			dy1 -= sy;
			sy = 0;
			reclip = 1;
		}

		w = dx2-dx1+1;
		h = dy2-dy1+1;

		if ( sx + w > bw ) {
			w = bw - sx;
			dx2 = dx1 + w - 1;
		}

		if ( sy + h > bh ) {
			h = bh - sy;
			dy2 = dy1 + h - 1;
		}

		if ( w < 1 ) return;		// clipped away!
		if ( h < 1 ) return;		// clipped away!

	} while (reclip);

	// Make sure clipping algorithm works
	#ifndef NDEBUG
		Assert( w > 0 );
		Assert( h > 0 );
		Assert( w == (dx2-dx1+1) );
		Assert( h == (dy2-dy1+1) );
		Assert( sx >= 0 );
		Assert( sy >= 0 );
		Assert( sx+w <= bw );
		Assert( sy+h <= bh );
		Assert( dx2 >= dx1 );
		Assert( dy2 >= dy1 );
		Assert( (dx1 >= gr_screen.clip_left ) && (dx1 <= gr_screen.clip_right) );
		Assert( (dx2 >= gr_screen.clip_left ) && (dx2 <= gr_screen.clip_right) );
		Assert( (dy1 >= gr_screen.clip_top ) && (dy1 <= gr_screen.clip_bottom) );
		Assert( (dy2 >= gr_screen.clip_top ) && (dy2 <= gr_screen.clip_bottom) );
	#endif

	// We now have dx1,dy1 and dx2,dy2 and sx, sy all set validly within clip regions.

	// Draw bitmap bm[sx,sy] into (dx1,dy1)-(dx2,dy2)

	gr8_bitmap_ex(dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy);
}



void grx_bitmap(int x,int y)
{
	int w, h;

	bm_get_info( gr_screen.current_bitmap, &w, &h, NULL );
	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;
	int sx=0, sy=0;

	if ((dx1 > gr_screen.clip_right ) || (dx2 < gr_screen.clip_left)) return;
	if ((dy1 > gr_screen.clip_bottom ) || (dy2 < gr_screen.clip_top)) return;
	if ( dx1 < gr_screen.clip_left ) { sx = gr_screen.clip_left-dx1; dx1 = gr_screen.clip_left; }
	if ( dy1 < gr_screen.clip_top ) { sy = gr_screen.clip_top-dy1; dy1 = gr_screen.clip_top; }
	if ( dx2 > gr_screen.clip_right )	{ dx2 = gr_screen.clip_right; }
	if ( dy2 > gr_screen.clip_bottom )	{ dy2 = gr_screen.clip_bottom; }

	if ( sx < 0 ) return;
	if ( sy < 0 ) return;
	if ( sx >= w ) return;
	if ( sy >= h ) return;

	// Draw bitmap bm[sx,sy] into (dx1,dy1)-(dx2,dy2)

	gr8_bitmap_ex(dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy);
}

void grx_aabitmap(int x,int y)
{
	int w, h;

	bm_get_info( gr_screen.current_bitmap, &w, &h, NULL );
	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;
	int sx=0, sy=0;

	if ((dx1 > gr_screen.clip_right ) || (dx2 < gr_screen.clip_left)) return;
	if ((dy1 > gr_screen.clip_bottom ) || (dy2 < gr_screen.clip_top)) return;
	if ( dx1 < gr_screen.clip_left ) { sx = gr_screen.clip_left-dx1; dx1 = gr_screen.clip_left; }
	if ( dy1 < gr_screen.clip_top ) { sy = gr_screen.clip_top-dy1; dy1 = gr_screen.clip_top; }
	if ( dx2 > gr_screen.clip_right )	{ dx2 = gr_screen.clip_right; }
	if ( dy2 > gr_screen.clip_bottom )	{ dy2 = gr_screen.clip_bottom; }

	if ( sx < 0 ) return;
	if ( sy < 0 ) return;
	if ( sx >= w ) return;
	if ( sy >= h ) return;

	// Draw bitmap bm[sx,sy] into (dx1,dy1)-(dx2,dy2)

	gr8_aabitmap_ex(dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy);
}

