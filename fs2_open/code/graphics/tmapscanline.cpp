/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/TmapScanline.cpp $
 * $Revision: 2.1 $
 * $Date: 2002-08-01 01:41:05 $
 * $Author: penguin $
 *
 * Routines to draw one textured mapped scanline.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:23  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
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
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 21    4/20/98 4:44p John
 * Fixed problems with black being xparent on model cache rneders.  Made
 * model cache key off of detail level setting and framerate.
 * 
 * 20    4/09/98 7:58p John
 * Cleaned up tmapper code a bit.   Put NDEBUG around some ndebug stuff.
 * Took out XPARENT flag settings in all the alpha-blended texture stuff.
 * 
 * 19    3/22/98 2:33p John
 * Took out fx_v/v_right.  Made fx_u etc get calculated in tmapper.
 * 
 * 18    3/10/98 4:19p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 17    12/10/96 10:37a John
 * Restructured texture mapper to remove some overhead from each scanline
 * setup.  This gave about a 30% improvement drawing trans01.pof, which is
 * a really complex model.  In the process, I cleaned up the scanline
 * functions and separated them into different modules for each pixel
 * depth.   
 * 
 * 16    12/02/96 4:03p John
 * made texture divide pipeline better. 2.5% speedup.
 * 
 * 15    11/26/96 6:50p John
 * Added some more hicolor primitives.  Made windowed mode run as current
 * bpp, if bpp is 8,16,or 32.
 * 
 * 14    11/18/96 9:58a John
 * Fixed warnings
 * 
 * 13    11/07/96 6:19p John
 * Added a bunch of 16bpp primitives so the game sort of runs in 16bpp
 * mode.
 * 
 * 12    11/07/96 3:49p John
 * Fixed some old 'c' inner loop code for testing.
 * 
 * 11    11/07/96 2:17p John
 * Took out the OldTmapper stuff.
 * 
 * 10    11/05/96 4:05p John
 * Added roller.  Added code to draw a distant planet.  Made bm_load
 * return -1 if invalid bitmap.
 * 
 * 9     10/31/96 7:20p John
 * Added per,tiled tmapper.  Made models tile if they use 64x64 textures.
 * 
 * 8     10/26/96 1:40p John
 * Added some now primitives to the 2d library and
 * cleaned up some old ones.
 *
 * $NoKeywords: $
 */

#include "render/3d.h"
#include "graphics/2d.h"
#include "graphics/grinternal.h"
#include "graphics/tmapper.h"
#include "graphics/tmapscanline.h"
#include "math/floating.h"
#include "palman/palman.h"
#include "math/fix.h"
#include "io/key.h"

// Needed to keep warning 4725 to stay away.  See PsTypes.h for details why.
void disable_warning_4725_stub_ts32()
{
}


extern void tmapscan_pln8_tiled_256x256();
extern void tmapscan_pln8_tiled_128x128();
extern void tmapscan_pln8_tiled_64x64();
extern void tmapscan_pln8_tiled_32x32();
extern void tmapscan_pln8_tiled_16x16();


void tmapscan_pln8_tiled()
{
	if ( (Tmap.bp->w == 256) && (Tmap.bp->h == 256) )	{
		tmapscan_pln8_tiled_256x256();
	} else if ( (Tmap.bp->w == 128) && (Tmap.bp->h == 128) )	{
		tmapscan_pln8_tiled_128x128();
	} else if ( (Tmap.bp->w == 64) && (Tmap.bp->h == 64) )	{
		tmapscan_pln8_tiled_64x64();
	} else if ( (Tmap.bp->w == 32) && (Tmap.bp->h == 32) )	{
		tmapscan_pln8_tiled_32x32();
	} else if ( (Tmap.bp->w == 16) && (Tmap.bp->h == 16) )	{
		tmapscan_pln8_tiled_16x16();
	} else {
		// argh! write another texure mapper!
		tmapscan_pln8();
	}
}


void tmapscan_write_z()
{
	int i;
	ubyte * dptr;
	uint w, dw;
	
	dptr = (ubyte *)Tmap.dest_row_data;

	w = Tmap.fx_w;
	dw = Tmap.fx_dwdx;

	uint *zbuf = (uint *)&gr_zbuffer[(uint)dptr-(uint)Tmap.pScreenBits];
	
	for (i=0; i<Tmap.loop_count; i++ )	{
		*zbuf = w;
		zbuf++;
		w += dw;
	}
}


void tmapscan_flat_gouraud_zbuffered()
{
	int i;
	ubyte * dptr,c;
	fix l, dl;
	uint w, dw;
	
	dptr = (ubyte *)Tmap.dest_row_data;
	c = gr_screen.current_color.raw8;

	w = Tmap.fx_w;
	dw = Tmap.fx_dwdx;

	l = Tmap.fx_l;
	dl = Tmap.fx_dl_dx;

	uint *zbuf = (uint *)&gr_zbuffer[(uint)dptr-(uint)Tmap.pScreenBits];
	
	for (i=0; i<Tmap.loop_count; i++ )	{
		if ( w > *zbuf )	{
			*zbuf = w;
			*dptr = gr_fade_table[(f2i(l)<<8)+c];
		}
		zbuf++;
		w += dw;
		l+=dl;
		dptr++;
	}
}

// ADAM: Change Nebula colors here:
#define NEBULA_COLORS 20

void tmapscan_nebula8()
{
	ubyte * dptr;
	int l1,l2, dldx;
	
	dptr = (ubyte *)Tmap.dest_row_data;

	float max_neb_color = i2fl(NEBULA_COLORS-1);

	l1 = (int)(Tmap.l.b*max_neb_color*256.0f);
	l2 = l1 + 256/2;		// dithering
	dldx = (int)(Tmap.deltas.b*max_neb_color*2.0f*256.0f);

		#ifdef USE_INLINE_ASM
//			memset( dptr, 31, Tmap.loop_count );
			_asm	push	eax
			_asm	push	ebx
			_asm	push	ecx
			_asm	push	edx
			_asm	push	edi

			// eax - l1
			// ebx - l2
			// ecx - count
			// edx - dldx
			// edi - dest
			_asm	mov	eax, l1
			_asm	mov	ebx, l2
			_asm	mov	edx, dldx
			_asm	mov	edi, dptr
		
			_asm	mov	ecx, Tmap.loop_count
			_asm	shr	ecx, 1
			_asm	jz		DoFinal
			_asm	pushf

		Next2Pixels:
			_asm	mov	[edi], ah
			_asm	add	eax, edx

			_asm	mov	[edi+1], bh
			_asm	add	ebx, edx
			
			_asm	add	edi, 2
			_asm	dec	ecx
			_asm	jnz	Next2Pixels

			_asm	popf
		DoFinal:
			_asm	jnc	NotDoFinal
			_asm	mov	[edi], ah
		NotDoFinal:

			_asm	pop	edi
			_asm	pop	edx
			_asm	pop	ecx
			_asm	pop	ebx
			_asm	pop	eax

		#else
			int i;
			if ( Tmap.loop_count > 1 )	{
				for (i=0; i<Tmap.loop_count/2; i++ )	{
					dptr[0] = (ubyte)((l1&0xFF00)>>8);
					l1+=dldx;
					dptr[1] = (ubyte)((l2&0xFF00)>>8);
					l2+=dldx;
					dptr+=2;
				}
			}
			if ( Tmap.loop_count & 1 )	{
				dptr[0] = (ubyte)((l1&0xFF00)>>8);
				dptr++;
			}
		#endif
}


void tmapscan_flat_gouraud()
{
	if (gr_zbuffering) {
		switch(gr_zbuffering_mode)	{
		case GR_ZBUFF_NONE:
			break;
		case GR_ZBUFF_FULL:		// both
			tmapscan_flat_gouraud_zbuffered();
			return;
		case GR_ZBUFF_WRITE:		// write only
			tmapscan_flat_gouraud_zbuffered();
			break;
		case GR_ZBUFF_READ:		// read only
			tmapscan_flat_gouraud_zbuffered();
			return;
		}
	}

	/* HARDWARE_ONLY
	if ( Current_alphacolor )	{
		ubyte *lookup = &Current_alphacolor->table.lookup[0][0];

		int i;
		ubyte * dptr;
		fix l, dl;
		
		dptr = (ubyte *)Tmap.dest_row_data;

		l = Tmap.fx_l;
		dl = Tmap.fx_dl_dx;
		
		for (i=0; i<Tmap.loop_count; i++ )	{
			*dptr = lookup[f2i(l*16)*256+*dptr];
			l+=dl;
			dptr++;
		}

	} else {
	*/
		int i;
		ubyte * dptr,c;
		fix l, dl;
		
		dptr = (ubyte *)Tmap.dest_row_data;
		c = gr_screen.current_color.raw8;

		l = Tmap.fx_l;
		dl = Tmap.fx_dl_dx;
		
		for (i=0; i<Tmap.loop_count; i++ )	{
			*dptr = gr_fade_table[f2i(l*32)*256+c];
			l+=dl;
			dptr++;
		}	
}


void tmapscan_flat8_zbuffered()
{
	int i;
	ubyte * dptr,c;

	dptr = (ubyte *)Tmap.dest_row_data;
	c = gr_screen.current_color.raw8;


	for (i=0; i<Tmap.loop_count; i++ )	{
		int tmp = (uint)dptr-Tmap.pScreenBits;
		if ( Tmap.fx_w > (int)gr_zbuffer[tmp] )	{
			gr_zbuffer[tmp] = Tmap.fx_w;
			*dptr = c;
		}
		Tmap.fx_w += Tmap.fx_dwdx;
		dptr++;
	}
}

void tmapscan_flat8()
{
	if (gr_zbuffering) {
		switch(gr_zbuffering_mode)	{
		case GR_ZBUFF_NONE:
			break;
		case GR_ZBUFF_FULL:		// both
			tmapscan_flat8_zbuffered();
			return;
		case GR_ZBUFF_WRITE:		// write only
			tmapscan_write_z();
			break;
		case GR_ZBUFF_READ:		// read only
			tmapscan_flat8_zbuffered();
			return;
		}
	}

	memset( (ubyte *)Tmap.dest_row_data, gr_screen.current_color.raw8, Tmap.loop_count );
}

void tmapscan_pln8_zbuffered();


void tmapscan_pln8_ppro()
{
	_asm {
	
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi

	// Put the FPU in low precision mode
	fstcw		Tmap.OldFPUCW					// store copy of CW
	mov		ax,Tmap.OldFPUCW				// get it in ax
	and		eax, ~0x300L
	mov		Tmap.FPUCW,ax					// store it
	fldcw		Tmap.FPUCW						// load the FPU

	mov		ecx, Tmap.loop_count		// ecx = width
	mov		edi, Tmap.dest_row_data	// edi = dest pointer

	// edi = pointer to start pixel in dest dib
	// ecx = spanwidth

	mov		eax,ecx							// eax and ecx = width
	shr		ecx,5								// ecx = width / subdivision length
	and		eax,31								// eax = width mod subdivision length
	jnz		some_left_over					// any leftover?
	dec		ecx								// no, so special case last span
	mov		eax,32								// it's 8 pixels long
some_left_over:
	mov		Tmap.Subdivisions,ecx		// store widths
	mov		Tmap.WidthModLength,eax

	// calculate ULeft and VLeft			// FPU Stack (ZL = ZLeft)
													// st0  st1  st2  st3  st4  st5  st6  st7
	fld		Tmap.l.v					// V/ZL 
	fld		Tmap.l.u					// U/ZL V/ZL 
	fld		Tmap.l.sw					// 1/ZL U/ZL V/ZL 
	fld1											// 1    1/ZL U/ZL V/ZL 
	fdiv		st,st(1)							// ZL   1/ZL U/ZL V/ZL 
	fld		st									// ZL   ZL   1/ZL U/ZL V/ZL 
	fmul		st,st(4)							// VL   ZL   1/ZL U/ZL V/ZL 
	fxch		st(1)								// ZL   VL   1/ZL U/ZL V/ZL 
	fmul		st,st(3)							// UL   VL   1/ZL U/ZL V/ZL 

	fstp		st(5)								// VL   1/ZL U/ZL V/ZL UL
	fstp		st(5)								// 1/ZL U/ZL V/ZL UL   VL

	// calculate right side OverZ terms  ; st0  st1  st2  st3  st4  st5  st6  st7

	fadd		Tmap.fl_dwdx_wide			// 1/ZR U/ZL V/ZL UL   VL
	fxch		st(1)								// U/ZL 1/ZR V/ZL UL   VL
	fadd		Tmap.fl_dudx_wide				// U/ZR 1/ZR V/ZL UL   VL
	fxch		st(2)								// V/ZL 1/ZR U/ZR UL   VL
	fadd		Tmap.fl_dvdx_wide				// V/ZR 1/ZR U/ZR UL   VL

	// calculate right side coords		// st0  st1  st2  st3  st4  st5  st6  st7

	fld1											// 1    V/ZR 1/ZR U/ZR UL   VL
	// @todo overlap this guy
	fdiv		st,st(2)							// ZR   V/ZR 1/ZR U/ZR UL   VL
	fld		st									// ZR   ZR   V/ZR 1/ZR U/ZR UL   VL
	fmul		st,st(2)							// VR   ZR   V/ZR 1/ZR U/ZR UL   VL
	fxch		st(1)								// ZR   VR   V/ZR 1/ZR U/ZR UL   VL
	fmul		st,st(4)							// UR   VR   V/ZR 1/ZR U/ZR UL   VL

	cmp		ecx,0							// check for any full spans
	jle      HandleLeftoverPixels
    
SpanLoop:

	// at this point the FPU contains	// st0  st1  st2  st3  st4  st5  st6  st7
													// UR   VR   V/ZR 1/ZR U/ZR UL   VL

	// convert left side coords

	fld     st(5)                       ; UL   UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fmul    Tmap.FixedScale            ; UL16 UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fistp   Tmap.UFixed                ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

	fld     st(6)                       ; VL   UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fmul    Tmap.FixedScale            ; VL16 UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fistp   Tmap.VFixed                ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

	// calculate deltas                  ; st0  st1  st2  st3  st4  st5  st6  st7

	fsubr   st(5),st                    ; UR   VR   V/ZR 1/ZR U/ZR dU   VL
	fxch    st(1)                       ; VR   UR   V/ZR 1/ZR U/ZR dU   VL
	fsubr   st(6),st                    ; VR   UR   V/ZR 1/ZR U/ZR dU   dV
	fxch    st(6)                       ; dV   UR   V/ZR 1/ZR U/ZR dU   VR

	fmul    Tmap.FixedScale8           ; dV8  UR   V/ZR 1/ZR U/ZR dU   VR
	fistp   Tmap.DeltaV                ; UR   V/ZR 1/ZR U/ZR dU   VR

	fxch    st(4)                       ; dU   V/ZR 1/ZR U/ZR UR   VR
	fmul    Tmap.FixedScale8           ; dU8  V/ZR 1/ZR U/ZR UR   VR
	fistp   Tmap.DeltaU                ; V/ZR 1/ZR U/ZR UR   VR

	// increment terms for next span    // st0  st1  st2  st3  st4  st5  st6  st7
	// Right terms become Left terms--->// V/ZL 1/ZL U/ZL UL   VL

	fadd    Tmap.fl_dvdx_wide				// V/ZR 1/ZL U/ZL UL   VL
	fxch    st(1)								// 1/ZL V/ZR U/ZL UL   VL
	fadd    Tmap.fl_dwdx_wide				// 1/ZR V/ZR U/ZL UL   VL
	fxch    st(2)								// U/ZL V/ZR 1/ZR UL   VL
	fadd    Tmap.fl_dudx_wide				// U/ZR V/ZR 1/ZR UL   VL
	fxch    st(2)								// 1/ZR V/ZR U/ZR UL   VL
	fxch    st(1)								// V/ZR 1/ZR U/ZR UL   VL


	// setup delta values
    
	mov     eax,Tmap.DeltaV				// get v 16.16 step
	mov     ebx,eax							// copy it
	sar     eax,16								// get v int step
	shl     ebx,16								// get v frac step
	mov     Tmap.DeltaVFrac,ebx			// store it
	imul    eax,Tmap.src_offset			// calculate texture step for v int step

	mov     ebx,Tmap.DeltaU				// get u 16.16 step
	mov     ecx,ebx							// copy it
	sar     ebx,16								// get u int step
	shl     ecx,16								// get u frac step
	mov     Tmap.DeltaUFrac,ecx			// store it
	add     eax,ebx							// calculate uint + vint step
	mov     Tmap.uv_delta[4],eax			// save whole step in non-v-carry slot
	add     eax,Tmap.src_offset			// calculate whole step + v carry
	mov     Tmap.uv_delta[0],eax			// save in v-carry slot

	// setup initial coordinates
	mov     esi,Tmap.UFixed				// get u 16.16 fixedpoint coordinate

	mov     ebx,esi							// copy it
	sar     esi,16								// get integer part
	shl     ebx,16								// get fractional part

	mov     ecx,Tmap.VFixed				// get v 16.16 fixedpoint coordinate
   
	mov     edx,ecx							// copy it
	sar     edx,16								// get integer part
	shl     ecx,16								// get fractional part
	imul    edx,Tmap.src_offset			// calc texture scanline address
	add     esi,edx							// calc texture offset
	add     esi,Tmap.pixptr				// calc address

	// set up affine registers
	mov     edx,Tmap.DeltaUFrac			// get register copy

 	mov	eax, Tmap.fx_l
	shr	eax, 8
	mov	bx, ax

	mov	ebp, Tmap.fx_dl_dx
	shl	ebp, 5	//*32
	add	Tmap.fx_l, ebp

	mov	ebp, Tmap.fx_l
	shr	ebp, 8
	sub	bp, ax
	shr	bp, 5

	mov	dx, bp

	// calculate right side coords		st0  st1  st2  st3  st4  st5  st6  st7
	fld1										// 1    V/ZR 1/ZR U/ZR UL   VL
	// This divide should happen while the pixel span is drawn.
	fdiv	st,st(2)							// ZR   V/ZR 1/ZR U/ZR UL   VL


	// 8 pixel span code
	// edi = dest dib bits at current pixel
	// esi = texture pointer at current u,v
	// eax = scratch
	// ebx = u fraction 0.32
	// ecx = v fraction 0.32
	// edx = u frac step
	// ebp = v carry scratch

	xor	eax, eax
	mov	al,[edi]								// preread the destination cache line
	xor	eax, eax
	mov	al,[esi]								// get texture pixel 0

	mov	Tmap.InnerLooper, 32/4			// Set up loop counter

InnerInnerLoop:

	push	ebx
	and	ebx, 0ff00h
	mov	eax, DWORD PTR gr_fade_table[eax+ebx]	// Get shaded pixel
	pop	ebx
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+0],al							// store pixel
	xor	eax, eax
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	push	ebx
	and	ebx, 0ff00h
	mov	eax, DWORD PTR gr_fade_table[eax+ebx]	// Get shaded pixel
	pop	ebx
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+1],al							// store pixel
	xor	eax, eax
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	push	ebx
	and	ebx, 0ff00h
	mov	eax, DWORD PTR gr_fade_table[eax+ebx]	// Get shaded pixel
	pop	ebx
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+2],al							// store pixel
	xor	eax, eax
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	push	ebx
	and	ebx, 0ff00h
	mov	eax, DWORD PTR gr_fade_table[eax+ebx]	// Get shaded pixel
	pop	ebx
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+3],al							// store pixel
	xor	eax, eax
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	add	edi, 4
	dec	Tmap.InnerLooper
	jnz	InnerInnerLoop

	// the fdiv is done, finish right	// st0  st1  st2  st3  st4  st5  st6  st7
	                                    // ZR   V/ZR 1/ZR U/ZR UL   VL

    fld     st									// ZR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(2)							// VR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fxch    st(1)								// ZR   VR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(4)							// UR   VR   V/ZR 1/ZR U/ZR UL   VL

    dec     Tmap.Subdivisions			// decrement span count
    jnz     SpanLoop							// loop back


HandleLeftoverPixels:

    mov     esi,Tmap.pixptr				// load texture pointer

    // edi = dest dib bits
    // esi = current texture dib bits
    // at this point the FPU contains    ; st0  st1  st2  st3  st4  st5  st6  st7
    // inv. means invalid numbers        ; inv. inv. inv. inv. inv. UL   VL

    cmp     Tmap.WidthModLength,0          ; are there remaining pixels to draw?
    jz      FPUReturn                   ; nope, pop the FPU and bail

    // convert left side coords          ; st0  st1  st2  st3  st4  st5  st6  st7

    fld     st(5)                       ; UL   inv. inv. inv. inv. inv. UL   VL
    fmul    Tmap.FixedScale                ; UL16 inv. inv. inv. inv. inv. UL   VL
    fistp   Tmap.UFixed                    ; inv. inv. inv. inv. inv. UL   VL

    fld     st(6)                       ; VL   inv. inv. inv. inv. inv. UL   VL
    fmul    Tmap.FixedScale                // VL16 inv. inv. inv. inv. inv. UL   VL
    fistp   Tmap.VFixed                    ; inv. inv. inv. inv. inv. UL   VL

    dec     Tmap.WidthModLength            ; calc how many steps to take
    jz      OnePixelSpan                ; just one, don't do deltas

    // calculate right edge coordinates  ; st0  st1  st2  st3  st4  st5  st6  st7
    // r -> R+1

    // @todo rearrange things so we don't need these two instructions
    fstp    Tmap.FloatTemp                 ; inv. inv. inv. inv. UL   VL
    fstp    Tmap.FloatTemp                 ; inv. inv. inv. UL   VL

    fld     Tmap.r.v           ; V/Zr inv. inv. inv. UL   VL
    fsub    Tmap.deltas.v             ; V/ZR inv. inv. inv. UL   VL
    fld     Tmap.r.u           ; U/Zr V/ZR inv. inv. inv. UL   VL
    fsub    Tmap.deltas.u             ; U/ZR V/ZR inv. inv. inv. UL   VL
    fld     Tmap.r.sw              ; 1/Zr U/ZR V/ZR inv. inv. inv. UL   VL
    fsub    Tmap.deltas.sw           ; 1/ZR U/ZR V/ZR inv. inv. inv. UL   VL

    fdivr   Tmap.One                       ; ZR   U/ZR V/ZR inv. inv. inv. UL   VL

    fmul    st(1),st                    ; ZR   UR   V/ZR inv. inv. inv. UL   VL
    fmulp   st(2),st                    ; UR   VR   inv. inv. inv. UL   VL

    // calculate deltas                  ; st0  st1  st2  st3  st4  st5  st6  st7

    fsubr   st(5),st                    ; UR   VR   inv. inv. inv. dU   VL
    fxch    st(1)                       ; VR   UR   inv. inv. inv. dU   VL
    fsubr   st(6),st                    ; VR   UR   inv. inv. inv. dU   dV
    fxch    st(6)                       ; dV   UR   inv. inv. inv. dU   VR

    fidiv   Tmap.WidthModLength            ; dv   UR   inv. inv. inv. dU   VR
    fmul    Tmap.FixedScale                ; dv16 UR   inv. inv. inv. dU   VR
    fistp   Tmap.DeltaV                    ; UR   inv. inv. inv. dU   VR

    fxch    st(4)                       ; dU   inv. inv. inv. UR   VR
    fidiv   Tmap.WidthModLength            ; du   inv. inv. inv. UR   VR
    fmul    Tmap.FixedScale                ; du16 inv. inv. inv. UR   VR
    fistp   Tmap.DeltaU                    ; inv. inv. inv. UR   VR

    // @todo gross!  these are to line up with the other loop
    fld     st(1)                       ; inv. inv. inv. inv. UR   VR
    fld     st(2)                       ; inv. inv. inv. inv. inv. UR   VR


	// setup delta values
	mov	eax, Tmap.DeltaV	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap.DeltaVFrac, ebx	// store it
	imul	eax, Tmap.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap.DeltaU			// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step
	mov	Tmap.uv_delta[4], eax	// save whole step in non-v-carry slot
	add	eax, Tmap.src_offset				// calc whole step + v carry
	mov	Tmap.uv_delta[0], eax	// save in v-carry slot


OnePixelSpan:

	; setup initial coordinates
	mov	esi, Tmap.UFixed			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap.VFixed			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap.pixptr			// calc address


	mov	eax, Tmap.fx_l
	shr	eax, 8
	mov	bx, ax

	mov	edx, Tmap.DeltaUFrac

	cmp	Tmap.WidthModLength, 1
	jle	NoDeltaLight

	push	ebx
	
	mov	ebx, Tmap.fx_l_right
	shr	ebx, 8
	
	sub	ebx, eax
	mov	eax, ebx
	
	mov	eax, Tmap.fx_dl_dx
	shr	eax, 8

	mov	dx, ax

	pop	ebx

NoDeltaLight:

	inc	Tmap.WidthModLength
	mov	eax,Tmap.WidthModLength
	shr	eax, 1
	jz		one_more_pix
	pushf
	mov	Tmap.WidthModLength, eax

	xor	eax, eax

	mov	al,[edi]                    // preread the destination cache line
	mov	al,[esi]

NextPixel:
	mov	ah, bh								// move lighting value into place
	mov	al, gr_fade_table[eax]			// Get shaded pixel
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+0],al							// store pixel
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	mov	ah, bh								// move lighting value into place
	mov	al, gr_fade_table[eax]			// Get shaded pixel
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+1],al							// store pixel
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	add	edi, 2
	dec	Tmap.WidthModLength
	jg		NextPixel

	popf
	jnc	FPUReturn

one_more_pix:	

	mov	al,[esi]                    // get texture pixel 2
	mov	ah, bh
	mov	al, gr_fade_table[eax]
	mov	[edi],al                  // store pixel 2


FPUReturn:

	// busy FPU registers:	// st0  st1  st2  st3  st4  st5  st6  st7
									// xxx  xxx  xxx  xxx  xxx  xxx  xxx
	ffree	st(0)
	ffree	st(1)
	ffree	st(2)
	ffree	st(3)
	ffree	st(4)
	ffree	st(5)
	ffree	st(6)

	fldcw	Tmap.OldFPUCW                  // restore the FPU

	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	pop	edx
	pop	ecx
	pop	eax
	}
}


void tmapscan_pln8_pentium()
{
	_asm {
	
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi

	// Put the FPU in low precision mode
	fstcw		Tmap.OldFPUCW					// store copy of CW
	mov		ax,Tmap.OldFPUCW				// get it in ax
	and		eax, ~0x300L
	mov		Tmap.FPUCW,ax					// store it
	fldcw		Tmap.FPUCW						// load the FPU

	mov		ecx, Tmap.loop_count		// ecx = width
	mov		edi, Tmap.dest_row_data	// edi = dest pointer

	// edi = pointer to start pixel in dest dib
	// ecx = spanwidth

	mov		eax,ecx							// eax and ecx = width
	shr		ecx,5								// ecx = width / subdivision length
	and		eax,31								// eax = width mod subdivision length
	jnz		some_left_over					// any leftover?
	dec		ecx								// no, so special case last span
	mov		eax,32								// it's 8 pixels long
some_left_over:
	mov		Tmap.Subdivisions,ecx		// store widths
	mov		Tmap.WidthModLength,eax

	// calculate ULeft and VLeft			// FPU Stack (ZL = ZLeft)
													// st0  st1  st2  st3  st4  st5  st6  st7
	fld		Tmap.l.v					// V/ZL 
	fld		Tmap.l.u					// U/ZL V/ZL 
	fld		Tmap.l.sw					// 1/ZL U/ZL V/ZL 
	fld1											// 1    1/ZL U/ZL V/ZL 
	fdiv		st,st(1)							// ZL   1/ZL U/ZL V/ZL 
	fld		st									// ZL   ZL   1/ZL U/ZL V/ZL 
	fmul		st,st(4)							// VL   ZL   1/ZL U/ZL V/ZL 
	fxch		st(1)								// ZL   VL   1/ZL U/ZL V/ZL 
	fmul		st,st(3)							// UL   VL   1/ZL U/ZL V/ZL 

	fstp		st(5)								// VL   1/ZL U/ZL V/ZL UL
	fstp		st(5)								// 1/ZL U/ZL V/ZL UL   VL

	// calculate right side OverZ terms  ; st0  st1  st2  st3  st4  st5  st6  st7

	fadd		Tmap.fl_dwdx_wide			// 1/ZR U/ZL V/ZL UL   VL
	fxch		st(1)								// U/ZL 1/ZR V/ZL UL   VL
	fadd		Tmap.fl_dudx_wide				// U/ZR 1/ZR V/ZL UL   VL
	fxch		st(2)								// V/ZL 1/ZR U/ZR UL   VL
	fadd		Tmap.fl_dvdx_wide				// V/ZR 1/ZR U/ZR UL   VL

	// calculate right side coords		// st0  st1  st2  st3  st4  st5  st6  st7

	fld1											// 1    V/ZR 1/ZR U/ZR UL   VL
	// @todo overlap this guy
	fdiv		st,st(2)							// ZR   V/ZR 1/ZR U/ZR UL   VL
	fld		st									// ZR   ZR   V/ZR 1/ZR U/ZR UL   VL
	fmul		st,st(2)							// VR   ZR   V/ZR 1/ZR U/ZR UL   VL
	fxch		st(1)								// ZR   VR   V/ZR 1/ZR U/ZR UL   VL
	fmul		st,st(4)							// UR   VR   V/ZR 1/ZR U/ZR UL   VL

	cmp		ecx,0							// check for any full spans
	jle      HandleLeftoverPixels
    
SpanLoop:

	// at this point the FPU contains	// st0  st1  st2  st3  st4  st5  st6  st7
													// UR   VR   V/ZR 1/ZR U/ZR UL   VL

	// convert left side coords

	fld     st(5)                       ; UL   UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fmul    Tmap.FixedScale            ; UL16 UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fistp   Tmap.UFixed                ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

	fld     st(6)                       ; VL   UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fmul    Tmap.FixedScale            ; VL16 UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fistp   Tmap.VFixed                ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

	// calculate deltas                  ; st0  st1  st2  st3  st4  st5  st6  st7

	fsubr   st(5),st                    ; UR   VR   V/ZR 1/ZR U/ZR dU   VL
	fxch    st(1)                       ; VR   UR   V/ZR 1/ZR U/ZR dU   VL
	fsubr   st(6),st                    ; VR   UR   V/ZR 1/ZR U/ZR dU   dV
	fxch    st(6)                       ; dV   UR   V/ZR 1/ZR U/ZR dU   VR

	fmul    Tmap.FixedScale8           ; dV8  UR   V/ZR 1/ZR U/ZR dU   VR
	fistp   Tmap.DeltaV                ; UR   V/ZR 1/ZR U/ZR dU   VR

	fxch    st(4)                       ; dU   V/ZR 1/ZR U/ZR UR   VR
	fmul    Tmap.FixedScale8           ; dU8  V/ZR 1/ZR U/ZR UR   VR
	fistp   Tmap.DeltaU                ; V/ZR 1/ZR U/ZR UR   VR

	// increment terms for next span    // st0  st1  st2  st3  st4  st5  st6  st7
	// Right terms become Left terms--->// V/ZL 1/ZL U/ZL UL   VL

	fadd    Tmap.fl_dvdx_wide				// V/ZR 1/ZL U/ZL UL   VL
	fxch    st(1)								// 1/ZL V/ZR U/ZL UL   VL
	fadd    Tmap.fl_dwdx_wide				// 1/ZR V/ZR U/ZL UL   VL
	fxch    st(2)								// U/ZL V/ZR 1/ZR UL   VL
	fadd    Tmap.fl_dudx_wide				// U/ZR V/ZR 1/ZR UL   VL
	fxch    st(2)								// 1/ZR V/ZR U/ZR UL   VL
	fxch    st(1)								// V/ZR 1/ZR U/ZR UL   VL


	// setup delta values
    
	mov     eax,Tmap.DeltaV				// get v 16.16 step
	mov     ebx,eax							// copy it
	sar     eax,16								// get v int step
	shl     ebx,16								// get v frac step
	mov     Tmap.DeltaVFrac,ebx			// store it
	imul    eax,Tmap.src_offset			// calculate texture step for v int step

	mov     ebx,Tmap.DeltaU				// get u 16.16 step
	mov     ecx,ebx							// copy it
	sar     ebx,16								// get u int step
	shl     ecx,16								// get u frac step
	mov     Tmap.DeltaUFrac,ecx			// store it
	add     eax,ebx							// calculate uint + vint step
	mov     Tmap.uv_delta[4],eax			// save whole step in non-v-carry slot
	add     eax,Tmap.src_offset			// calculate whole step + v carry
	mov     Tmap.uv_delta[0],eax			// save in v-carry slot

	// setup initial coordinates
	mov     esi,Tmap.UFixed				// get u 16.16 fixedpoint coordinate

	mov     ebx,esi							// copy it
	sar     esi,16								// get integer part
	shl     ebx,16								// get fractional part

	mov     ecx,Tmap.VFixed				// get v 16.16 fixedpoint coordinate
   
	mov     edx,ecx							// copy it
	sar     edx,16								// get integer part
	shl     ecx,16								// get fractional part
	imul    edx,Tmap.src_offset			// calc texture scanline address
	add     esi,edx							// calc texture offset
	add     esi,Tmap.pixptr				// calc address

	// set up affine registers
	mov     edx,Tmap.DeltaUFrac			// get register copy

 	mov	eax, Tmap.fx_l
	shr	eax, 8
	mov	bx, ax

	mov	ebp, Tmap.fx_dl_dx
	shl	ebp, 5	//*32
	add	Tmap.fx_l, ebp

	mov	ebp, Tmap.fx_l
	shr	ebp, 8
	sub	bp, ax
	shr	bp, 5

	mov	dx, bp

	// calculate right side coords		st0  st1  st2  st3  st4  st5  st6  st7
	fld1										// 1    V/ZR 1/ZR U/ZR UL   VL
	// This divide should happen while the pixel span is drawn.
	fdiv	st,st(2)							// ZR   V/ZR 1/ZR U/ZR UL   VL


	// 8 pixel span code
	// edi = dest dib bits at current pixel
	// esi = texture pointer at current u,v
	// eax = scratch
	// ebx = u fraction 0.32
	// ecx = v fraction 0.32
	// edx = u frac step
	// ebp = v carry scratch


	mov	al,[edi]								// preread the destination cache line

	mov	al,[esi]								// get texture pixel 0

	mov	Tmap.InnerLooper, 32/4			// Set up loop counter

InnerInnerLoop:

	mov	ah, bh								// move lighting value into place
	mov	al, gr_fade_table[eax]			// Get shaded pixel


	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+0],al							// store pixel

	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	mov	ah, bh								// move lighting value into place
	mov	al, gr_fade_table[eax]			// Get shaded pixel


	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+1],al							// store pixel

	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	mov	ah, bh								// move lighting value into place
	mov	al, gr_fade_table[eax]			// Get shaded pixel


	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+2],al							// store pixel

	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	mov	ah, bh								// move lighting value into place
	mov	al, gr_fade_table[eax]			// Get shaded pixel


	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+3],al							// store pixel

	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	add	edi, 4
	dec	Tmap.InnerLooper
	jnz	InnerInnerLoop

	// the fdiv is done, finish right	// st0  st1  st2  st3  st4  st5  st6  st7
	                                    // ZR   V/ZR 1/ZR U/ZR UL   VL

    fld     st									// ZR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(2)							// VR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fxch    st(1)								// ZR   VR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(4)							// UR   VR   V/ZR 1/ZR U/ZR UL   VL

    dec     Tmap.Subdivisions			// decrement span count
    jnz     SpanLoop							// loop back


HandleLeftoverPixels:

    mov     esi,Tmap.pixptr				// load texture pointer

    // edi = dest dib bits
    // esi = current texture dib bits
    // at this point the FPU contains    ; st0  st1  st2  st3  st4  st5  st6  st7
    // inv. means invalid numbers        ; inv. inv. inv. inv. inv. UL   VL

    cmp     Tmap.WidthModLength,0          ; are there remaining pixels to draw?
    jz      FPUReturn                   ; nope, pop the FPU and bail

    // convert left side coords          ; st0  st1  st2  st3  st4  st5  st6  st7

    fld     st(5)                       ; UL   inv. inv. inv. inv. inv. UL   VL
    fmul    Tmap.FixedScale                ; UL16 inv. inv. inv. inv. inv. UL   VL
    fistp   Tmap.UFixed                    ; inv. inv. inv. inv. inv. UL   VL

    fld     st(6)                       ; VL   inv. inv. inv. inv. inv. UL   VL
    fmul    Tmap.FixedScale                // VL16 inv. inv. inv. inv. inv. UL   VL
    fistp   Tmap.VFixed                    ; inv. inv. inv. inv. inv. UL   VL

    dec     Tmap.WidthModLength            ; calc how many steps to take
    jz      OnePixelSpan                ; just one, don't do deltas

    // calculate right edge coordinates  ; st0  st1  st2  st3  st4  st5  st6  st7
    // r -> R+1

    // @todo rearrange things so we don't need these two instructions
    fstp    Tmap.FloatTemp                 ; inv. inv. inv. inv. UL   VL
    fstp    Tmap.FloatTemp                 ; inv. inv. inv. UL   VL

    fld     Tmap.r.v           ; V/Zr inv. inv. inv. UL   VL
    fsub    Tmap.deltas.v             ; V/ZR inv. inv. inv. UL   VL
    fld     Tmap.r.u           ; U/Zr V/ZR inv. inv. inv. UL   VL
    fsub    Tmap.deltas.u             ; U/ZR V/ZR inv. inv. inv. UL   VL
    fld     Tmap.r.sw              ; 1/Zr U/ZR V/ZR inv. inv. inv. UL   VL
    fsub    Tmap.deltas.sw           ; 1/ZR U/ZR V/ZR inv. inv. inv. UL   VL

    fdivr   Tmap.One                       ; ZR   U/ZR V/ZR inv. inv. inv. UL   VL

    fmul    st(1),st                    ; ZR   UR   V/ZR inv. inv. inv. UL   VL
    fmulp   st(2),st                    ; UR   VR   inv. inv. inv. UL   VL

    // calculate deltas                  ; st0  st1  st2  st3  st4  st5  st6  st7

    fsubr   st(5),st                    ; UR   VR   inv. inv. inv. dU   VL
    fxch    st(1)                       ; VR   UR   inv. inv. inv. dU   VL
    fsubr   st(6),st                    ; VR   UR   inv. inv. inv. dU   dV
    fxch    st(6)                       ; dV   UR   inv. inv. inv. dU   VR

    fidiv   Tmap.WidthModLength            ; dv   UR   inv. inv. inv. dU   VR
    fmul    Tmap.FixedScale                ; dv16 UR   inv. inv. inv. dU   VR
    fistp   Tmap.DeltaV                    ; UR   inv. inv. inv. dU   VR

    fxch    st(4)                       ; dU   inv. inv. inv. UR   VR
    fidiv   Tmap.WidthModLength            ; du   inv. inv. inv. UR   VR
    fmul    Tmap.FixedScale                ; du16 inv. inv. inv. UR   VR
    fistp   Tmap.DeltaU                    ; inv. inv. inv. UR   VR

    // @todo gross!  these are to line up with the other loop
    fld     st(1)                       ; inv. inv. inv. inv. UR   VR
    fld     st(2)                       ; inv. inv. inv. inv. inv. UR   VR


	// setup delta values
	mov	eax, Tmap.DeltaV	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap.DeltaVFrac, ebx	// store it
	imul	eax, Tmap.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap.DeltaU			// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step
	mov	Tmap.uv_delta[4], eax	// save whole step in non-v-carry slot
	add	eax, Tmap.src_offset				// calc whole step + v carry
	mov	Tmap.uv_delta[0], eax	// save in v-carry slot


OnePixelSpan:

	; setup initial coordinates
	mov	esi, Tmap.UFixed			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap.VFixed			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap.pixptr			// calc address


	mov	eax, Tmap.fx_l
	shr	eax, 8
	mov	bx, ax

	mov	edx, Tmap.DeltaUFrac

	cmp	Tmap.WidthModLength, 1
	jle	NoDeltaLight

	push	ebx
	
	mov	ebx, Tmap.fx_l_right
	shr	ebx, 8
	
	sub	ebx, eax
	mov	eax, ebx
	
	mov	eax, Tmap.fx_dl_dx
	shr	eax, 8

	mov	dx, ax

	pop	ebx

NoDeltaLight:

	inc	Tmap.WidthModLength
	mov	eax,Tmap.WidthModLength
	shr	eax, 1
	jz		one_more_pix
	pushf
	mov	Tmap.WidthModLength, eax

	xor	eax, eax

	mov	al,[edi]                    // preread the destination cache line
	mov	al,[esi]

NextPixel:
	mov	ah, bh								// move lighting value into place
	mov	al, gr_fade_table[eax]			// Get shaded pixel
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+0],al							// store pixel
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	mov	ah, bh								// move lighting value into place
	mov	al, gr_fade_table[eax]			// Get shaded pixel
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+1],al							// store pixel
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	add	edi, 2
	dec	Tmap.WidthModLength
	jg		NextPixel

	popf
	jnc	FPUReturn

one_more_pix:	

	mov	al,[esi]                    // get texture pixel 2
	mov	ah, bh
	mov	al, gr_fade_table[eax]
	mov	[edi],al                  // store pixel 2


FPUReturn:

	// busy FPU registers:	// st0  st1  st2  st3  st4  st5  st6  st7
									// xxx  xxx  xxx  xxx  xxx  xxx  xxx
	ffree	st(0)
	ffree	st(1)
	ffree	st(2)
	ffree	st(3)
	ffree	st(4)
	ffree	st(5)
	ffree	st(6)

	fldcw	Tmap.OldFPUCW                  // restore the FPU

	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	pop	edx
	pop	ecx
	pop	eax
	}
}




void tmapscan_pln8()
{
	if (gr_zbuffering) {
		switch(gr_zbuffering_mode)	{
		case GR_ZBUFF_NONE:
			break;
		case GR_ZBUFF_FULL:		// both
			tmapscan_pln8_zbuffered();
			return;
		case GR_ZBUFF_WRITE:		// write only
			tmapscan_write_z();
			break;
		case GR_ZBUFF_READ:		// read only
			tmapscan_pln8_zbuffered();
			return;
		}

	}

	if ( Gr_cpu > 5 )	{
		tmapscan_pln8_ppro();
	} else {
		tmapscan_pln8_pentium();
	}
}


void tmapscan_lln8()
{
	_asm {
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi

	; setup delta values
	mov	eax, Tmap.fx_dv_dx	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap.DeltaVFrac, ebx	// store it
	imul	eax, Tmap.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap.fx_du_dx		// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step

	mov	Tmap.uv_delta[4], eax	// save whole step in non-v-carry slot
	add	eax, Tmap.src_offset				// calc whole step + v carry
	mov	Tmap.uv_delta[0], eax	// save in v-carry slot

	; setup initial coordinates
	mov	esi, Tmap.fx_u			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap.fx_v			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap.pixptr			// calc address
	
	; set edi = address of first pixel to modify
	mov	edi, Tmap.dest_row_data
	
	mov	edx, Tmap.DeltaUFrac

	mov	eax, Tmap.loop_count
	shr	eax, 2
	je		DoLeftOverPixels

	mov	Tmap.num_big_steps, eax
	and	Tmap.loop_count, 3

	mov     al,[edi]                    // preread the destination cache line
	mov     al,[esi]                    // get texture pixel 0

NextPixelBlock:

	push	eax
	mov	eax, Tmap.fx_l
	shr	eax, 8
	mov	bx, ax

	mov	ebp, Tmap.fx_dl_dx
	shl	ebp, 2	//*4
	add	Tmap.fx_l, ebp

	mov	ebp, Tmap.fx_l
	shr	ebp, 8
	sub	bp, ax
	shr	bp, 2

	mov	dx, bp
	pop	eax


    // 8 pixel span code
    // edi = dest dib bits at current pixel
    // esi = texture pointer at current u,v
    // eax = scratch
    // ebx = u fraction 0.32
    // ecx = v fraction 0.32
    // edx = u frac step
    // ebp = v carry scratch

	mov	ah, bh								// move lighting value into place
	mov	al, gr_fade_table[eax]			// Get shaded pixel
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+0],al							// store pixel
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	mov	ah, bh								// move lighting value into place
	mov	al, gr_fade_table[eax]			// Get shaded pixel
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+1],al							// store pixel
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	mov	ah, bh								// move lighting value into place
	mov	al, gr_fade_table[eax]			// Get shaded pixel
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+2],al							// store pixel
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	mov	ah, bh								// move lighting value into place
	mov	al, gr_fade_table[eax]			// Get shaded pixel
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+3],al							// store pixel
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	add	edi, 4
	dec	Tmap.num_big_steps
	jne	NextPixelBlock
	

DoLeftOverPixels:

	mov	ebp, Tmap.fx_l
	shr	ebp, 8
	mov	bx, bp

	mov	ebp, Tmap.fx_dl_dx
	shr	ebp, 8
	mov	dx, bp

	mov	eax,Tmap.loop_count
	test	eax, -1
	jz	_none_to_do
	shr	eax, 1
	je	one_more_pix
	mov	Tmap.loop_count, eax
	pushf

	xor	eax, eax

	mov	al, [edi]                    // preread the destination cache line
	mov	al, [esi]							// Get first texel

NextPixel:

	mov	ah, bh								// move lighting value into place
	mov	al, gr_fade_table[eax]			// Get shaded pixel
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+0],al							// store pixel
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	mov	ah, bh								// move lighting value into place
	mov	al, gr_fade_table[eax]			// Get shaded pixel
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+1],al							// store pixel
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	add	edi, 2
	dec	Tmap.loop_count
	jne	NextPixel

	popf
	jnc	_none_to_do

one_more_pix:	
	xor	eax, eax
	mov	al, [esi]							// Get first texel
	mov	ah, bh
	mov	al, gr_fade_table[eax]
   mov	[edi],al                  // store pixel 2

_none_to_do:	
	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	pop	edx
	pop	ecx
	pop	eax
	}
}


void tmapscan_lna8_zbuffered_ppro()
{
	_asm {
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi

	; setup delta values
	mov	eax, Tmap.fx_dv_dx	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap.DeltaVFrac, ebx	// store it
	imul	eax, Tmap.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap.fx_du_dx		// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step

	mov	Tmap.uv_delta[4], eax	// save whole step in non-v-carry slot
	add	eax, Tmap.src_offset				// calc whole step + v carry
	mov	Tmap.uv_delta[0], eax	// save in v-carry slot

	; setup initial coordinates
	mov	esi, Tmap.fx_u			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap.fx_v			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap.pixptr			// calc address
	
	; set edi = address of first pixel to modify
	mov	edi, Tmap.dest_row_data
	
	xor	eax, eax
	mov	al,[edi]							// get the destination pixel

	mov	ebp, Tmap.fx_w
	mov	edx, gr_zbuffer
	mov	eax, edi
	sub	eax, Tmap.pScreenBits
	shl	eax, 2
	add	edx, eax

	mov	eax, Tmap.loop_count

	shr	eax, 2
	je		DoLeftOverPixels

	mov	Tmap.num_big_steps, eax
	and	Tmap.loop_count, 3

NextPixelBlock:

	// 8 pixel span code
	// eax = scratch
	// ebx = u fraction 0.32
	// ecx = v fraction 0.32
	// edx = zbuffer pointer
	// edi = dest dib bits at current pixel
	// esi = texture pointer at current u,v
	// ebp = zvalue
	// esp = stack

	cmp	ebp, [edx+4*0]						// Compare the Z depth of this pixel with zbuffer
	jle	Skip0a								// If pixel is covered, skip drawing
//	mov	[edx+4*0], ebp						// Write new Z value

	// Get pixel and blend it
	push	ebx
	xor	ebx, ebx
	mov	bl, [edi+0]
	xor	eax, eax									// Clear all bits of EAX.  This avoids a partial register stall on Pentium Pros
	mov	ah, [esi]								// Get texel into AL
	add	eax, Tmap.BlendLookup
	mov	eax, [eax+ebx]		// Lookup pixel in lighting table
	pop	ebx

	mov	[edi+0],al							// store pixel
Skip0a:
	add	ebp,Tmap.fx_dwdx					// increment z value
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	eax,eax								// get -1 if carry
	add	ebx,Tmap.DeltaUFrac				// increment u fraction
	adc	esi,Tmap.uv_delta[4*eax+4]		// add in step ints & carries


	cmp	ebp, [edx+4*1]						// Compare the Z depth of this pixel with zbuffer
	jle	Skip1a								// If pixel is covered, skip drawing
//	mov	[edx+4*1], ebp						// Write new Z value

	// Get pixel and blend it
	push	ebx
	xor	ebx, ebx
	mov	bl, [edi+1]
	xor	eax, eax									// Clear all bits of EAX.  This avoids a partial register stall on Pentium Pros
	mov	ah, [esi]								// Get texel into AL
	add	eax, Tmap.BlendLookup
	mov	eax, [eax+ebx]		// Lookup pixel in lighting table
	pop	ebx

	mov	[edi+1],al							// store pixel
Skip1a:
	add	ebp, Tmap.fx_dwdx
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	eax,eax								// get -1 if carry
	add	ebx,Tmap.DeltaUFrac				// increment u fraction
	adc	esi,Tmap.uv_delta[4*eax+4]		// add in step ints & carries

	cmp	ebp, [edx+4*2]						// Compare the Z depth of this pixel with zbuffer
	jle	Skip2a								// If pixel is covered, skip drawing
//	mov	[edx+4*2], ebp						// Write new Z value

	push	ebx
	xor	ebx, ebx
	mov	bl, [edi+2]
	xor	eax, eax									// Clear all bits of EAX.  This avoids a partial register stall on Pentium Pros
	mov	ah, [esi]								// Get texel into AL
	add	eax, Tmap.BlendLookup
	mov	eax, [eax+ebx]		// Lookup pixel in lighting table
	pop	ebx

	mov	[edi+2],al							// store pixel
Skip2a:
	add	ebp, Tmap.fx_dwdx
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	eax,eax								// get -1 if carry
	add	ebx,Tmap.DeltaUFrac				// increment u fraction
	adc	esi,Tmap.uv_delta[4*eax+4]		// add in step ints & carries

	cmp	ebp, [edx+4*3]						// Compare the Z depth of this pixel with zbuffer
	jle	Skip3a								// If pixel is covered, skip drawing
//	mov	[edx+4*3], ebp						// Write new Z value

	push	ebx
	xor	ebx, ebx
	mov	bl, [edi+3]
	xor	eax, eax									// Clear all bits of EAX.  This avoids a partial register stall on Pentium Pros
	mov	ah, [esi]								// Get texel into AL
	add	eax, Tmap.BlendLookup
	mov	eax, [eax+ebx]		// Lookup pixel in lighting table
	pop	ebx

	mov	[edi+3],al							// store pixel
Skip3a:
	add	ebp, Tmap.fx_dwdx
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	eax,eax								// get -1 if carry
	add	ebx,Tmap.DeltaUFrac				// increment u fraction
	adc	esi,Tmap.uv_delta[4*eax+4]		// add in step ints & carries

	add	edx, 16
	add	edi, 4
	dec	Tmap.num_big_steps
	jne	NextPixelBlock
	

DoLeftOverPixels:

	mov	eax,Tmap.loop_count
	test	eax, -1
	jz	_none_to_do
	shr	eax, 1
	je	one_more_pix
	mov	Tmap.loop_count, eax
	pushf

	xor	eax, eax
	mov	al,[edi]							// get the destination pixel

NextPixel:

	cmp	ebp, [edx+4*0]						// Compare the Z depth of this pixel with zbuffer
	jle	Skip0b								// If pixel is covered, skip drawing
//	mov	[edx+4*0], ebp						// Write new Z value
	mov	al,[edi+0]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	and	eax, 0ffffh
	add	eax, Tmap.BlendLookup
	mov	al, [eax]							// blend them
	mov	[edi+0],al							// store pixel
Skip0b:
	add	ebp, Tmap.fx_dwdx
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	eax,eax								// get -1 if carry
	add	ebx,Tmap.DeltaUFrac				// increment u fraction
	adc	esi,Tmap.uv_delta[4*eax+4]		// add in step ints & carries

	cmp	ebp, [edx+4*1]						// Compare the Z depth of this pixel with zbuffer
	jle	Skip1b								// If pixel is covered, skip drawing
//	mov	[edx+4*1], ebp						// Write new Z value
	mov	al,[edi+1]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	and	eax, 0ffffh
	add	eax, Tmap.BlendLookup
	mov	al, [eax]							// blend them
	mov	[edi+1],al							// store pixel
Skip1b:
	add	ebp, Tmap.fx_dwdx
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	eax,eax								// get -1 if carry
	add	ebx,Tmap.DeltaUFrac				// increment u fraction
	adc	esi,Tmap.uv_delta[4*eax+4]		// add in step ints & carries

	add	edi, 2
	add	edx, 8
	dec	Tmap.loop_count
	jne	NextPixel

	popf
	jnc	_none_to_do

one_more_pix:	
	cmp	ebp, [edx]						// Compare the Z depth of this pixel with zbuffer
	jle	Skip0c								// If pixel is covered, skip drawing
//	mov	[edx], ebp						// Write new Z value
	mov	al,[edi]							// get the destination pixel
	mov	ah,[esi]							// get texture pixel 0
	and	eax, 0ffffh
	add	eax, Tmap.BlendLookup
	mov	al, [eax]							// blend them
	mov	[edi],al							// store pixel
Skip0c:

_none_to_do:	
	pop	edi
  	pop	esi
	pop	ebp
	pop	ebx
	pop	edx
	pop	ecx
	pop	eax
	}	
}

void tmapscan_lna8_zbuffered_pentium()
{
	_asm {
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi

	; setup delta values
	mov	eax, Tmap.fx_dv_dx	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap.DeltaVFrac, ebx	// store it
	imul	eax, Tmap.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap.fx_du_dx		// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step

	mov	Tmap.uv_delta[4], eax	// save whole step in non-v-carry slot
	add	eax, Tmap.src_offset				// calc whole step + v carry
	mov	Tmap.uv_delta[0], eax	// save in v-carry slot

	; setup initial coordinates
	mov	esi, Tmap.fx_u			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap.fx_v			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap.pixptr			// calc address
	
	; set edi = address of first pixel to modify
	mov	edi, Tmap.dest_row_data
	
	xor	eax, eax
	mov	al,[edi]							// get the destination pixel

	mov	ebp, Tmap.fx_w
	mov	edx, gr_zbuffer
	mov	eax, edi
	sub	eax, Tmap.pScreenBits
	shl	eax, 2
	add	edx, eax

	mov	eax, Tmap.loop_count

	shr	eax, 2
	je		DoLeftOverPixels

	mov	Tmap.num_big_steps, eax
	and	Tmap.loop_count, 3

NextPixelBlock:

	// 8 pixel span code
	// eax = scratch
	// ebx = u fraction 0.32
	// ecx = v fraction 0.32
	// edx = zbuffer pointer
	// edi = dest dib bits at current pixel
	// esi = texture pointer at current u,v
	// ebp = zvalue
	// esp = stack

	cmp	ebp, [edx+4*0]						// Compare the Z depth of this pixel with zbuffer
	jle	Skip0a								// If pixel is covered, skip drawing
//	mov	[edx+4*0], ebp						// Write new Z value
	mov	al,[edi+0]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	and	eax, 0ffffh




	add	eax, Tmap.BlendLookup
	mov	al, [eax]							// blend them


	mov	[edi+0],al							// store pixel
Skip0a:
	add	ebp,Tmap.fx_dwdx					// increment z value
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	eax,eax								// get -1 if carry
	add	ebx,Tmap.DeltaUFrac				// increment u fraction
	adc	esi,Tmap.uv_delta[4*eax+4]		// add in step ints & carries


	cmp	ebp, [edx+4*1]						// Compare the Z depth of this pixel with zbuffer
	jle	Skip1a								// If pixel is covered, skip drawing
//	mov	[edx+4*1], ebp						// Write new Z value
	mov	al,[edi+1]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	and	eax, 0ffffh




	add	eax, Tmap.BlendLookup
	mov	al, [eax]							// blend them


	mov	[edi+1],al							// store pixel
Skip1a:
	add	ebp, Tmap.fx_dwdx
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	eax,eax								// get -1 if carry
	add	ebx,Tmap.DeltaUFrac				// increment u fraction
	adc	esi,Tmap.uv_delta[4*eax+4]		// add in step ints & carries

	cmp	ebp, [edx+4*2]						// Compare the Z depth of this pixel with zbuffer
	jle	Skip2a								// If pixel is covered, skip drawing
//	mov	[edx+4*2], ebp						// Write new Z value
	mov	al,[edi+2]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	and	eax, 0ffffh



	add	eax, Tmap.BlendLookup
	mov	al, [eax]							// blend them


	mov	[edi+2],al							// store pixel
Skip2a:
	add	ebp, Tmap.fx_dwdx
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	eax,eax								// get -1 if carry
	add	ebx,Tmap.DeltaUFrac				// increment u fraction
	adc	esi,Tmap.uv_delta[4*eax+4]		// add in step ints & carries

	cmp	ebp, [edx+4*3]						// Compare the Z depth of this pixel with zbuffer
	jle	Skip3a								// If pixel is covered, skip drawing
//	mov	[edx+4*3], ebp						// Write new Z value
	mov	al,[edi+3]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	and	eax, 0ffffh



	add	eax, Tmap.BlendLookup
	mov	al, [eax]							// blend them


	mov	[edi+3],al							// store pixel
Skip3a:
	add	ebp, Tmap.fx_dwdx
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	eax,eax								// get -1 if carry
	add	ebx,Tmap.DeltaUFrac				// increment u fraction
	adc	esi,Tmap.uv_delta[4*eax+4]		// add in step ints & carries

	add	edx, 16
	add	edi, 4
	dec	Tmap.num_big_steps
	jne	NextPixelBlock
	

DoLeftOverPixels:

	mov	eax,Tmap.loop_count
	test	eax, -1
	jz	_none_to_do
	shr	eax, 1
	je	one_more_pix
	mov	Tmap.loop_count, eax
	pushf

	xor	eax, eax
	mov	al,[edi]							// get the destination pixel

NextPixel:

	cmp	ebp, [edx+4*0]						// Compare the Z depth of this pixel with zbuffer
	jle	Skip0b								// If pixel is covered, skip drawing
//	mov	[edx+4*0], ebp						// Write new Z value
	mov	al,[edi+0]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	and	eax, 0ffffh
	add	eax, Tmap.BlendLookup
	mov	al, [eax]							// blend them
	mov	[edi+0],al							// store pixel
Skip0b:
	add	ebp, Tmap.fx_dwdx
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	eax,eax								// get -1 if carry
	add	ebx,Tmap.DeltaUFrac				// increment u fraction
	adc	esi,Tmap.uv_delta[4*eax+4]		// add in step ints & carries

	cmp	ebp, [edx+4*1]						// Compare the Z depth of this pixel with zbuffer
	jle	Skip1b								// If pixel is covered, skip drawing
//	mov	[edx+4*1], ebp						// Write new Z value
	mov	al,[edi+1]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	and	eax, 0ffffh
	add	eax, Tmap.BlendLookup
	mov	al, [eax]							// blend them
	mov	[edi+1],al							// store pixel
Skip1b:
	add	ebp, Tmap.fx_dwdx
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	eax,eax								// get -1 if carry
	add	ebx,Tmap.DeltaUFrac				// increment u fraction
	adc	esi,Tmap.uv_delta[4*eax+4]		// add in step ints & carries

	add	edi, 2
	add	edx, 8
	dec	Tmap.loop_count
	jne	NextPixel

	popf
	jnc	_none_to_do

one_more_pix:	
	cmp	ebp, [edx]						// Compare the Z depth of this pixel with zbuffer
	jle	Skip0c								// If pixel is covered, skip drawing
//	mov	[edx], ebp						// Write new Z value
	mov	al,[edi]							// get the destination pixel
	mov	ah,[esi]							// get texture pixel 0
	and	eax, 0ffffh
	add	eax, Tmap.BlendLookup
	mov	al, [eax]							// blend them
	mov	[edi],al							// store pixel
Skip0c:

_none_to_do:	
	pop	edi
  	pop	esi
	pop	ebp
	pop	ebx
	pop	edx
	pop	ecx
	pop	eax
	}
}

void tmapscan_lna8_zbuffered()
{
	if ( Gr_cpu > 5 )	{
		tmapscan_lna8_zbuffered_ppro();
	} else {
		tmapscan_lna8_zbuffered_pentium();
	}
}



extern float Tmap_clipped_left;

void tmapscan_lna8()
{
	if (gr_zbuffering) {
		switch(gr_zbuffering_mode)	{
		case GR_ZBUFF_NONE:
			break;
		case GR_ZBUFF_FULL:		// both
		case GR_ZBUFF_WRITE:		// write only
		case GR_ZBUFF_READ:		// read only
			tmapscan_lna8_zbuffered();
			return;
		}

	}
	
	_asm {
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi

	; setup delta values
	mov	eax, Tmap.fx_dv_dx	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap.DeltaVFrac, ebx	// store it
	imul	eax, Tmap.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap.fx_du_dx		// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step

	mov	Tmap.uv_delta[4], eax	// save whole step in non-v-carry slot
	add	eax, Tmap.src_offset				// calc whole step + v carry
	mov	Tmap.uv_delta[0], eax	// save in v-carry slot

	; setup initial coordinates
	mov	esi, Tmap.fx_u			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap.fx_v			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap.pixptr			// calc address
	
	; set edi = address of first pixel to modify
	mov	edi, Tmap.dest_row_data
	
	mov	edx, Tmap.DeltaUFrac

	xor	eax, eax
	mov	al,[edi]							// get the destination pixel

	mov	ebp, Tmap.loop_count

	shr	ebp, 2
	je		DoLeftOverPixels

	mov	Tmap.num_big_steps, ebp
	and	Tmap.loop_count, 3

	mov	eax, 0

NextPixelBlock:

    // 8 pixel span code
    // edi = dest dib bits at current pixel
    // esi = texture pointer at current u,v
    // eax = scratch
    // ebx = u fraction 0.32
    // ecx = v fraction 0.32
    // edx = u frac step
    // ebp = v carry scratch


	xor	eax, eax
	mov	al,[edi+0]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries
	add	eax, Tmap.BlendLookup
	mov	al, [eax]							// blend them
	mov	[edi+0],al							// store pixel

	xor	eax, eax
	mov	al,[edi+1]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries
	add	eax, Tmap.BlendLookup
	mov	al, [eax]							// blend them
	mov	[edi+1],al							// store pixel

	xor	eax, eax
	mov	al,[edi+2]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries
	add	eax, Tmap.BlendLookup
	mov	al, [eax]							// blend them
	mov	[edi+2],al							// store pixel

	xor	eax, eax
	mov	al,[edi+3]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries
	add	eax, Tmap.BlendLookup
	mov	al, [eax]							// blend them
	mov	[edi+3],al							// store pixel

	add	edi, 4
	dec	Tmap.num_big_steps
	jne	NextPixelBlock
	

DoLeftOverPixels:

	mov	ebp,Tmap.loop_count
	test	ebp, -1
	jz	_none_to_do
	shr	ebp, 1
	je	one_more_pix
	mov	Tmap.loop_count, ebp
	pushf

	mov	al,[edi]							// get the destination pixel

NextPixel:

	xor	eax, eax
	mov	al,[edi+0]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries
	add	eax, Tmap.BlendLookup
	mov	al, [eax]							// blend them
	mov	[edi+0],al							// store pixel

	xor	eax, eax
	mov	al,[edi+1]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries
	add	eax, Tmap.BlendLookup
	mov	al, [eax]							// blend them
	mov	[edi+1],al							// store pixel

	add	edi, 2
	dec	Tmap.loop_count
	jne	NextPixel

	popf
	jnc	_none_to_do

one_more_pix:	
	mov	eax, 0
	mov	al,[edi]							// get the destination pixel
	mov	ah,[esi]							// get texture pixel 0
	add	eax, Tmap.BlendLookup
	mov	al, [eax]							// blend them
	mov	[edi],al							// store pixel

_none_to_do:	
	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	pop	edx
	pop	ecx
	pop	eax
	}
}

// HACKED IN SYSTEM FOR DOING MODEL CACHING
int Tmap_scan_read = 0;		// 0 = normal mapper, 1=read, 2=write

// HACKED IN SYSTEM FOR DOING MODEL CACHING
void tmapscan_lnn8_read()
{
	Tmap.fx_u = fl2f(Tmap.l.u);
	Tmap.fx_v = fl2f(Tmap.l.v);
	Tmap.fx_du_dx = fl2f(Tmap.deltas.u);
	Tmap.fx_dv_dx = fl2f(Tmap.deltas.v);

/*
	int i;

	ubyte * src = (ubyte *)Tmap.pixptr;
	ubyte * dst = (ubyte *)Tmap.dest_row_data;
	
	for (i=0; i<Tmap.loop_count; i++ )	{
		int u,v;
		u = f2i(Tmap.fx_u);
		v = f2i(Tmap.fx_v);
		
		src[u+v*Tmap.src_offset] = *dst++;
						
		Tmap.fx_u += Tmap.fx_du_dx;
		Tmap.fx_v += Tmap.fx_dv_dx;
	}
*/

	_asm {
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi

	; setup delta values
	mov	eax, Tmap.fx_dv_dx	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap.DeltaVFrac, ebx	// store it
	imul	eax, Tmap.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap.fx_du_dx		// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step

	mov	Tmap.uv_delta[4], eax	// save whole step in non-v-carry slot
	add	eax, Tmap.src_offset				// calc whole step + v carry
	mov	Tmap.uv_delta[0], eax	// save in v-carry slot

	; setup initial coordinates
	mov	esi, Tmap.fx_u			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap.fx_v			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap.pixptr			// calc address
	
	; set edi = address of first pixel to modify
	mov	edi, Tmap.dest_row_data
	
	mov	edx, Tmap.DeltaUFrac

	mov     al,[edi]                    // preread the destination cache line

	mov	ebp, Tmap.loop_count

	shr	ebp, 2
	je		DoLeftOverPixels

	mov	Tmap.num_big_steps, ebp
	and	Tmap.loop_count, 3


NextPixelBlock:

    // 8 pixel span code
    // edi = dest dib bits at current pixel
    // esi = texture pointer at current u,v
    // eax = scratch
    // ebx = u fraction 0.32
    // ecx = v fraction 0.32
    // edx = u frac step
    // ebp = v carry scratch

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	al,[edi+0]							// get texture pixel
	mov	[esi],al								// store pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	al,[edi+1]							// get texture pixel
	mov	[esi],al								// store pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	al,[edi+2]							// get texture pixel
	mov	[esi],al								// store pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	al,[edi+3]							// get texture pixel
	mov	[esi],al								// store pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries

	add	edi, 4
	dec	Tmap.num_big_steps
	jne	NextPixelBlock
	

DoLeftOverPixels:

	mov	ebp,Tmap.loop_count
	test	ebp, -1
	jz	_none_to_do
	shr	ebp, 1
	je	one_more_pix
	mov	Tmap.loop_count, ebp
	pushf

	xor	eax, eax

	mov	al, [edi]                    // preread the destination cache line

NextPixel:

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	al,[edi+0]							// get texture pixel
	mov	[esi],al								// store pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	al,[edi+1]							// get texture pixel
	mov	[esi],al								// store pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries

	add	edi, 2
	dec	Tmap.loop_count
	jne	NextPixel

	popf
	jnc	_none_to_do

one_more_pix:	
	mov	al,[edi]								// get texture pixel
   mov	[esi],al                  // store pixel 2

_none_to_do:	
	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	pop	edx
	pop	ecx
	pop	eax
	}
}


// HACKED IN SYSTEM FOR DOING MODEL CACHING
void tmapscan_lnn8_write()
{
	Tmap.fx_u = fl2f(Tmap.l.u);
	Tmap.fx_v = fl2f(Tmap.l.v);
	Tmap.fx_du_dx = fl2f(Tmap.deltas.u);
	Tmap.fx_dv_dx = fl2f(Tmap.deltas.v);

/*
	int i;

	ubyte * src = (ubyte *)Tmap.pixptr;
	ubyte * dst = (ubyte *)Tmap.dest_row_data;
	
	for (i=0; i<Tmap.loop_count; i++ )	{
		int u,v;
		u = f2i(Tmap.fx_u);
		v = f2i(Tmap.fx_v);

		ubyte c = src[u+v*Tmap.src_offset];
		if ( c != 0 )	{
			*dst = c;
		}
		dst++;
					
		Tmap.fx_u += Tmap.fx_du_dx;
		Tmap.fx_v += Tmap.fx_dv_dx;
	}
*/

	_asm {
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi

	; setup delta values
	mov	eax, Tmap.fx_dv_dx	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap.DeltaVFrac, ebx	// store it
	imul	eax, Tmap.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap.fx_du_dx		// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step

	mov	Tmap.uv_delta[4], eax	// save whole step in non-v-carry slot
	add	eax, Tmap.src_offset				// calc whole step + v carry
	mov	Tmap.uv_delta[0], eax	// save in v-carry slot

	; setup initial coordinates
	mov	esi, Tmap.fx_u			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap.fx_v			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap.pixptr			// calc address
	
	; set edi = address of first pixel to modify
	mov	edi, Tmap.dest_row_data
	
	mov	edx, Tmap.DeltaUFrac

	mov     al,[edi]                    // preread the destination cache line

	mov	ebp, Tmap.loop_count

	shr	ebp, 2
	je		DoLeftOverPixels

	mov	Tmap.num_big_steps, ebp
	and	Tmap.loop_count, 3


NextPixelBlock:

    // 8 pixel span code
    // edi = dest dib bits at current pixel
    // esi = texture pointer at current u,v
    // eax = scratch
    // ebx = u fraction 0.32
    // ecx = v fraction 0.32
    // edx = u frac step
    // ebp = v carry scratch

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries
	cmp	al, 255
	je		Skip0
	mov	[edi+0],al							// store pixel
Skip0:

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries
	cmp	al, 255
	je		Skip1
	mov	[edi+1],al							// store pixel
Skip1:

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries
	cmp	al, 255
	je		Skip2
	mov	[edi+2],al							// store pixel
Skip2:

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries
	cmp	al, 255
	je		Skip3
	mov	[edi+3],al							// store pixel
Skip3:

	add	edi, 4
	dec	Tmap.num_big_steps
	jne	NextPixelBlock
	

DoLeftOverPixels:

	mov	ebp,Tmap.loop_count
	test	ebp, -1
	jz	_none_to_do
	shr	ebp, 1
	je	one_more_pix
	mov	Tmap.loop_count, ebp
	pushf

	xor	eax, eax

	mov	al, [edi]                    // preread the destination cache line

NextPixel:

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries
	cmp	al, 255
	je		Skip0a
	mov	[edi+0],al							// store pixel
Skip0a:

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries
	cmp	al, 255
	je		Skip1a
	mov	[edi+1],al							// store pixel
Skip1a:

	add	edi, 2
	dec	Tmap.loop_count
	jne	NextPixel

	popf
	jnc	_none_to_do

one_more_pix:	
	mov	al,[esi]								// get texture pixel
	cmp	al, 255
	je		Skip0b
	mov	[edi],al							// store pixel
Skip0b:

_none_to_do:	
	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	pop	edx
	pop	ecx
	pop	eax
	}
}

void tmapscan_lnn8()
{
	// HACKED IN SYSTEM FOR DOING MODEL CACHING
	if ( Tmap_scan_read==1 )	{
		tmapscan_lnn8_read();
		return;
	} else if ( Tmap_scan_read==2 ) {
		tmapscan_lnn8_write();
		//tmapscan_lnt8();
		return;
	}

	if ( gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER )		{
		tmapscan_lna8();
		return;
	}

	_asm {
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi

	; setup delta values
	mov	eax, Tmap.fx_dv_dx	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap.DeltaVFrac, ebx	// store it
	imul	eax, Tmap.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap.fx_du_dx		// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step

	mov	Tmap.uv_delta[4], eax	// save whole step in non-v-carry slot
	add	eax, Tmap.src_offset				// calc whole step + v carry
	mov	Tmap.uv_delta[0], eax	// save in v-carry slot

	; setup initial coordinates
	mov	esi, Tmap.fx_u			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap.fx_v			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap.pixptr			// calc address
	
	; set edi = address of first pixel to modify
	mov	edi, Tmap.dest_row_data
	
	mov	edx, Tmap.DeltaUFrac

	mov     al,[edi]                    // preread the destination cache line
	mov     al,[esi]                    // get texture pixel 0

	mov	ebp, Tmap.loop_count

	shr	ebp, 2
	je		DoLeftOverPixels

	mov	Tmap.num_big_steps, ebp
	and	Tmap.loop_count, 3


NextPixelBlock:

    // 8 pixel span code
    // edi = dest dib bits at current pixel
    // esi = texture pointer at current u,v
    // eax = scratch
    // ebx = u fraction 0.32
    // ecx = v fraction 0.32
    // edx = u frac step
    // ebp = v carry scratch

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+0],al							// store pixel
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+1],al							// store pixel
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+2],al							// store pixel
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+3],al							// store pixel
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	add	edi, 4
	dec	Tmap.num_big_steps
	jne	NextPixelBlock
	

DoLeftOverPixels:

	mov	ebp,Tmap.loop_count
	test	ebp, -1
	jz	_none_to_do
	shr	ebp, 1
	je	one_more_pix
	mov	Tmap.loop_count, ebp
	pushf

	xor	eax, eax

	mov	al, [edi]                    // preread the destination cache line
	mov	al, [esi]							// Get first texel

NextPixel:

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+0],al							// store pixel
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	mov	[edi+1],al							// store pixel
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	add	edi, 2
	dec	Tmap.loop_count
	jne	NextPixel

	popf
	jnc	_none_to_do

one_more_pix:	
   mov     [edi],al                  // store pixel 2

_none_to_do:	
	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	pop	edx
	pop	ecx
	pop	eax
	}
}

void tmapscan_lnt8()
{
	if ( gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER )		{
		tmapscan_lna8();
		return;
	}

	_asm {
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi

	; setup delta values
	mov	eax, Tmap.fx_dv_dx	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap.DeltaVFrac, ebx	// store it
	imul	eax, Tmap.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap.fx_du_dx		// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step

	mov	Tmap.uv_delta[4], eax	// save whole step in non-v-carry slot
	add	eax, Tmap.src_offset				// calc whole step + v carry
	mov	Tmap.uv_delta[0], eax	// save in v-carry slot

	; setup initial coordinates
	mov	esi, Tmap.fx_u			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap.fx_v			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap.pixptr			// calc address
	
	; set edi = address of first pixel to modify
	mov	edi, Tmap.dest_row_data
	
	mov	edx, Tmap.DeltaUFrac

	mov     al,[edi]                    // preread the destination cache line
	mov     al,[esi]                    // get texture pixel 0

	mov	ebp, Tmap.loop_count

	shr	ebp, 2
	je		DoLeftOverPixels

	mov	Tmap.num_big_steps, ebp
	and	Tmap.loop_count, 3

NextPixelBlock:

    // 8 pixel span code
    // edi = dest dib bits at current pixel
    // esi = texture pointer at current u,v
    // eax = scratch
    // ebx = u fraction 0.32
    // ecx = v fraction 0.32
    // edx = u frac step
    // ebp = v carry scratch

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	cmp	al, 255
	je		skip0
	mov	[edi+0],al							// store pixel
skip0:
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	cmp	al, 255
	je		skip1
	mov	[edi+1],al							// store pixel
skip1:
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	cmp	al, 255
	je		skip2
	mov	[edi+2],al							// store pixel
skip2:
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	cmp	al, 255
	je		skip3
	mov	[edi+3],al							// store pixel
skip3:
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	add	edi, 4
	dec	Tmap.num_big_steps
	jne	NextPixelBlock
	

DoLeftOverPixels:

	mov	ebp,Tmap.loop_count
	test	ebp, -1
	jz	_none_to_do
	shr	ebp, 1
	je	one_more_pix
	mov	Tmap.loop_count, ebp
	pushf

	xor	eax, eax

	mov	al, [edi]                    // preread the destination cache line
	mov	al, [esi]							// Get first texel

NextPixel:

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	cmp	al, 255
	je		skipa0
	mov	[edi+0],al							// store pixel
skipa0:
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	cmp	al, 255
	je		skipa1
	mov	[edi+1],al							// store pixel
skipa1:
	mov	al,[esi]								// get texture pixel
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]	// add in step ints & carries

	add	edi, 2
	dec	Tmap.loop_count
	jne	NextPixel

	popf
	jnc	_none_to_do

one_more_pix:	
	cmp	al, 255
	je		skipb0
	mov	[edi],al							// store pixel
skipb0:

_none_to_do:	
	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	pop	edx
	pop	ecx
	pop	eax
	}
}



void tmapscan_pln8_zbuffered_ppro()
{
	_asm {
	
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi

	// Put the FPU in low precision mode
	fstcw		Tmap.OldFPUCW					// store copy of CW
	mov		ax,Tmap.OldFPUCW				// get it in ax
	and		eax, ~0x300L
	mov		Tmap.FPUCW,ax					// store it
	fldcw		Tmap.FPUCW						// load the FPU


	mov		ecx, Tmap.loop_count		// ecx = width
	mov		edi, Tmap.dest_row_data	// edi = dest pointer

	// edi = pointer to start pixel in dest dib
	// ecx = spanwidth

	mov		eax,ecx							// eax and ecx = width
	shr		ecx,5								// ecx = width / subdivision length
	and		eax,31								// eax = width mod subdivision length
	jnz		some_left_over					// any leftover?
	dec		ecx								// no, so special case last span
	mov		eax,32								// it's 8 pixels long
some_left_over:
	mov		Tmap.Subdivisions,ecx		// store widths
	mov		Tmap.WidthModLength,eax

	// calculate ULeft and VLeft			// FPU Stack (ZL = ZLeft)
													// st0  st1  st2  st3  st4  st5  st6  st7
	fld		Tmap.l.v					// V/ZL 
	fld		Tmap.l.u					// U/ZL V/ZL 
	fld		Tmap.l.sw					// 1/ZL U/ZL V/ZL 
	fld1											// 1    1/ZL U/ZL V/ZL 
	fdiv		st,st(1)							// ZL   1/ZL U/ZL V/ZL 
	fld		st									// ZL   ZL   1/ZL U/ZL V/ZL 
	fmul		st,st(4)							// VL   ZL   1/ZL U/ZL V/ZL 
	fxch		st(1)								// ZL   VL   1/ZL U/ZL V/ZL 
	fmul		st,st(3)							// UL   VL   1/ZL U/ZL V/ZL 

	fstp		st(5)								// VL   1/ZL U/ZL V/ZL UL
	fstp		st(5)								// 1/ZL U/ZL V/ZL UL   VL

	// calculate right side OverZ terms  ; st0  st1  st2  st3  st4  st5  st6  st7

	fadd		Tmap.fl_dwdx_wide			// 1/ZR U/ZL V/ZL UL   VL
	fxch		st(1)								// U/ZL 1/ZR V/ZL UL   VL
	fadd		Tmap.fl_dudx_wide				// U/ZR 1/ZR V/ZL UL   VL
	fxch		st(2)								// V/ZL 1/ZR U/ZR UL   VL
	fadd		Tmap.fl_dvdx_wide				// V/ZR 1/ZR U/ZR UL   VL

	// calculate right side coords		// st0  st1  st2  st3  st4  st5  st6  st7

	fld1											// 1    V/ZR 1/ZR U/ZR UL   VL
	// @todo overlap this guy
	fdiv		st,st(2)							// ZR   V/ZR 1/ZR U/ZR UL   VL
	fld		st									// ZR   ZR   V/ZR 1/ZR U/ZR UL   VL
	fmul		st,st(2)							// VR   ZR   V/ZR 1/ZR U/ZR UL   VL
	fxch		st(1)								// ZR   VR   V/ZR 1/ZR U/ZR UL   VL
	fmul		st,st(4)							// UR   VR   V/ZR 1/ZR U/ZR UL   VL

	cmp		ecx,0							// check for any full spans
	jle      HandleLeftoverPixels
    
SpanLoop:

	// at this point the FPU contains	// st0  st1  st2  st3  st4  st5  st6  st7
													// UR   VR   V/ZR 1/ZR U/ZR UL   VL

	// convert left side coords

	fld     st(5)                       ; UL   UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fmul    Tmap.FixedScale            ; UL16 UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fistp   Tmap.UFixed                ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

	fld     st(6)                       ; VL   UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fmul    Tmap.FixedScale            ; VL16 UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fistp   Tmap.VFixed                ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

	// calculate deltas                  ; st0  st1  st2  st3  st4  st5  st6  st7

	fsubr   st(5),st                    ; UR   VR   V/ZR 1/ZR U/ZR dU   VL
	fxch    st(1)                       ; VR   UR   V/ZR 1/ZR U/ZR dU   VL
	fsubr   st(6),st                    ; VR   UR   V/ZR 1/ZR U/ZR dU   dV
	fxch    st(6)                       ; dV   UR   V/ZR 1/ZR U/ZR dU   VR

	fmul    Tmap.FixedScale8           ; dV8  UR   V/ZR 1/ZR U/ZR dU   VR
	fistp   Tmap.DeltaV                ; UR   V/ZR 1/ZR U/ZR dU   VR

	fxch    st(4)                       ; dU   V/ZR 1/ZR U/ZR UR   VR
	fmul    Tmap.FixedScale8           ; dU8  V/ZR 1/ZR U/ZR UR   VR
	fistp   Tmap.DeltaU                ; V/ZR 1/ZR U/ZR UR   VR

	// increment terms for next span    // st0  st1  st2  st3  st4  st5  st6  st7
	// Right terms become Left terms--->// V/ZL 1/ZL U/ZL UL   VL

	fadd    Tmap.fl_dvdx_wide				// V/ZR 1/ZL U/ZL UL   VL
	fxch    st(1)								// 1/ZL V/ZR U/ZL UL   VL
	fadd    Tmap.fl_dwdx_wide				// 1/ZR V/ZR U/ZL UL   VL
	fxch    st(2)								// U/ZL V/ZR 1/ZR UL   VL
	fadd    Tmap.fl_dudx_wide				// U/ZR V/ZR 1/ZR UL   VL
	fxch    st(2)								// 1/ZR V/ZR U/ZR UL   VL
	fxch    st(1)								// V/ZR 1/ZR U/ZR UL   VL


	// setup delta values
    
	mov     eax,Tmap.DeltaV				// get v 16.16 step
	mov     ebx,eax							// copy it
	sar     eax,16								// get v int step
	shl     ebx,16								// get v frac step
	mov     Tmap.DeltaVFrac,ebx			// store it
	imul    eax,Tmap.src_offset			// calculate texture step for v int step

	mov     ebx,Tmap.DeltaU				// get u 16.16 step
	mov     ecx,ebx							// copy it
	sar     ebx,16								// get u int step
	shl     ecx,16								// get u frac step
	mov     Tmap.DeltaUFrac,ecx			// store it
	add     eax,ebx							// calculate uint + vint step
	mov     Tmap.uv_delta[4],eax			// save whole step in non-v-carry slot
	add     eax,Tmap.src_offset			// calculate whole step + v carry
	mov     Tmap.uv_delta[0],eax			// save in v-carry slot

	// setup initial coordinates
	mov     esi,Tmap.UFixed				// get u 16.16 fixedpoint coordinate

	mov     ebx,esi							// copy it
	sar     esi,16								// get integer part
	shl     ebx,16								// get fractional part

	mov     ecx,Tmap.VFixed				// get v 16.16 fixedpoint coordinate
   
	mov     edx,ecx							// copy it
	sar     edx,16								// get integer part
	shl     ecx,16								// get fractional part
	imul    edx,Tmap.src_offset			// calc texture scanline address
	add     esi,edx							// calc texture offset
	add     esi,Tmap.pixptr				// calc address

	// set up affine registers

	mov	eax, Tmap.fx_l
	shr	eax, 8
	mov	bx, ax

	mov	ebp, Tmap.fx_dl_dx
	shl	ebp, 5	//*32
	add	Tmap.fx_l, ebp

	mov	ebp, Tmap.fx_l
	shr	ebp, 8
	sub	bp, ax
	shr	bp, 5

	mov	edx, Tmap.DeltaUFrac
	mov	dx, bp
	mov	Tmap.DeltaUFrac, edx


	// calculate right side coords		st0  st1  st2  st3  st4  st5  st6  st7
	fld1										// 1    V/ZR 1/ZR U/ZR UL   VL
	// This divide should happen while the pixel span is drawn.
	fdiv	st,st(2)							// ZR   V/ZR 1/ZR U/ZR UL   VL


	// 8 pixel span code
	// edi = dest dib bits at current pixel
	// esi = texture pointer at current u,v
	// eax = scratch
	// ebx = u fraction 0.32
	// ecx = v fraction 0.32
	// edx = u frac step
	// ebp = v carry scratch

	mov	al,[edi]								// preread the destination cache line
	mov	al,[esi]								// get texture pixel 0

	mov	Tmap.InnerLooper, 32/4			// Set up loop counter

	mov	ebp, Tmap.fx_w


	mov	eax, edi
	sub	eax, Tmap.pScreenBits
	mov	edx, gr_zbuffer
	shl	eax, 2
	add	edx, eax

InnerInnerLoop:

			// Pixel 0
			cmp	ebp, [edx+0]							// Compare the Z depth of this pixel with zbuffer
			jle	Skip0									// If pixel is covered, skip drawing
			mov	[edx+0], ebp							// Write new Z value
	
			// Get pixel and light it
			push	ebx
			xor	eax, eax									// Clear all bits of EAX.  This avoids a partial register stall on Pentium Pros
			mov	al, [esi]								// Get texel into AL
			and	ebx, 0ff00h								// Clear out fractional part of EBX
			mov	eax, DWORD PTR gr_fade_table[eax+ebx]		// Lookup pixel in lighting table
			pop	ebx

			mov	[edi+0],al								// Write new pixel

Skip0:	add	ecx,Tmap.DeltaVFrac
			sbb	eax,eax
			add	ebp,Tmap.fx_dwdx
			add	ebx,Tmap.DeltaUFrac
			adc	esi,Tmap.uv_delta[4*eax+4]


			// Pixel 1
			cmp	ebp, [edx+4]							// Compare the Z depth of this pixel with zbuffer
			jle	Skip1								// If pixel is covered, skip drawing
			mov	[edx+4], ebp							// Write new Z value
	
			// Get pixel and light it
			push	ebx
			xor	eax, eax									// Clear all bits of EAX.  This avoids a partial register stall on Pentium Pros
			mov	al, [esi]								// Get texel into AL
			and	ebx, 0ff00h								// Clear out fractional part of EBX
			mov	eax, DWORD PTR gr_fade_table[eax+ebx]		// Lookup pixel in lighting table
			pop	ebx

			mov	[edi+1],al								// Write new pixel

Skip1:	add	ecx,Tmap.DeltaVFrac
			sbb	eax,eax
			add	ebp,Tmap.fx_dwdx
			add	ebx,Tmap.DeltaUFrac
			adc	esi,Tmap.uv_delta[4*eax+4]


			// Pixel 2
			cmp	ebp, [edx+8]							// Compare the Z depth of this pixel with zbuffer
			jle	Skip2										// If pixel is covered, skip drawing
			mov	[edx+8], ebp							// Write new Z value
	
			// Get pixel and light it
			push	ebx
			xor	eax, eax									// Clear all bits of EAX.  This avoids a partial register stall on Pentium Pros
			mov	al, [esi]								// Get texel into AL
			and	ebx, 0ff00h								// Clear out fractional part of EBX
			mov	eax, DWORD PTR gr_fade_table[eax+ebx]		// Lookup pixel in lighting table
			pop	ebx

			mov	[edi+2],al								// Write new pixel

Skip2:	add	ecx,Tmap.DeltaVFrac
			sbb	eax,eax
			add	ebp,Tmap.fx_dwdx
			add	ebx,Tmap.DeltaUFrac
			adc	esi,Tmap.uv_delta[4*eax+4]


			// Pixel 3
			cmp	ebp, [edx+12]							// Compare the Z depth of this pixel with zbuffer
			jle	Skip3									// If pixel is covered, skip drawing
			mov	[edx+12], ebp							// Write new Z value
	
			// Get pixel and light it
			push	ebx
			xor	eax, eax									// Clear all bits of EAX.  This avoids a partial register stall on Pentium Pros
			mov	al, [esi]								// Get texel into AL
			and	ebx, 0ff00h								// Clear out fractional part of EBX
			mov	eax, DWORD PTR gr_fade_table[eax+ebx]		// Lookup pixel in lighting table
			pop	ebx

			mov	[edi+3],al								// Write new pixel

Skip3:	add	ecx,Tmap.DeltaVFrac
			sbb	eax,eax
			add	ebp,Tmap.fx_dwdx
			add	ebx,Tmap.DeltaUFrac
			adc	esi,Tmap.uv_delta[4*eax+4]



	add	edi, 4
	add	edx, 16
	dec	Tmap.InnerLooper
	jnz	InnerInnerLoop

	mov	Tmap.fx_w, ebp

	// the fdiv is done, finish right	// st0  st1  st2  st3  st4  st5  st6  st7
	                                    // ZR   V/ZR 1/ZR U/ZR UL   VL

    fld     st									// ZR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(2)							// VR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fxch    st(1)								// ZR   VR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(4)							// UR   VR   V/ZR 1/ZR U/ZR UL   VL

    dec     Tmap.Subdivisions			// decrement span count
    jnz     SpanLoop							// loop back


HandleLeftoverPixels:

    mov     esi,Tmap.pixptr				// load texture pointer

    // edi = dest dib bits
    // esi = current texture dib bits
    // at this point the FPU contains    ; st0  st1  st2  st3  st4  st5  st6  st7
    // inv. means invalid numbers        ; inv. inv. inv. inv. inv. UL   VL

    cmp     Tmap.WidthModLength,0          ; are there remaining pixels to draw?
    jz      FPUReturn                   ; nope, pop the FPU and bail

    // convert left side coords          ; st0  st1  st2  st3  st4  st5  st6  st7

    fld     st(5)                       ; UL   inv. inv. inv. inv. inv. UL   VL
    fmul    Tmap.FixedScale                ; UL16 inv. inv. inv. inv. inv. UL   VL
    fistp   Tmap.UFixed                    ; inv. inv. inv. inv. inv. UL   VL

    fld     st(6)                       ; VL   inv. inv. inv. inv. inv. UL   VL
    fmul    Tmap.FixedScale                // VL16 inv. inv. inv. inv. inv. UL   VL
    fistp   Tmap.VFixed                    ; inv. inv. inv. inv. inv. UL   VL

    dec     Tmap.WidthModLength            ; calc how many steps to take
    jz      OnePixelSpan                ; just one, don't do deltas

    // calculate right edge coordinates  ; st0  st1  st2  st3  st4  st5  st6  st7
    // r -> R+1

    // @todo rearrange things so we don't need these two instructions
    fstp    Tmap.FloatTemp                 ; inv. inv. inv. inv. UL   VL
    fstp    Tmap.FloatTemp                 ; inv. inv. inv. UL   VL

    fld     Tmap.r.v           ; V/Zr inv. inv. inv. UL   VL
    fsub    Tmap.deltas.v             ; V/ZR inv. inv. inv. UL   VL
    fld     Tmap.r.u           ; U/Zr V/ZR inv. inv. inv. UL   VL
    fsub    Tmap.deltas.u             ; U/ZR V/ZR inv. inv. inv. UL   VL
    fld     Tmap.r.sw              ; 1/Zr U/ZR V/ZR inv. inv. inv. UL   VL
    fsub    Tmap.deltas.sw           ; 1/ZR U/ZR V/ZR inv. inv. inv. UL   VL

    fdivr   Tmap.One                       ; ZR   U/ZR V/ZR inv. inv. inv. UL   VL

    fmul    st(1),st                    ; ZR   UR   V/ZR inv. inv. inv. UL   VL
    fmulp   st(2),st                    ; UR   VR   inv. inv. inv. UL   VL

    // calculate deltas                  ; st0  st1  st2  st3  st4  st5  st6  st7

    fsubr   st(5),st                    ; UR   VR   inv. inv. inv. dU   VL
    fxch    st(1)                       ; VR   UR   inv. inv. inv. dU   VL
    fsubr   st(6),st                    ; VR   UR   inv. inv. inv. dU   dV
    fxch    st(6)                       ; dV   UR   inv. inv. inv. dU   VR

    fidiv   Tmap.WidthModLength            ; dv   UR   inv. inv. inv. dU   VR
    fmul    Tmap.FixedScale                ; dv16 UR   inv. inv. inv. dU   VR
    fistp   Tmap.DeltaV                    ; UR   inv. inv. inv. dU   VR

    fxch    st(4)                       ; dU   inv. inv. inv. UR   VR
    fidiv   Tmap.WidthModLength            ; du   inv. inv. inv. UR   VR
    fmul    Tmap.FixedScale                ; du16 inv. inv. inv. UR   VR
    fistp   Tmap.DeltaU                    ; inv. inv. inv. UR   VR

    // @todo gross!  these are to line up with the other loop
    fld     st(1)                       ; inv. inv. inv. inv. UR   VR
    fld     st(2)                       ; inv. inv. inv. inv. inv. UR   VR


	// setup delta values
	mov	eax, Tmap.DeltaV	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap.DeltaVFrac, ebx	// store it
	imul	eax, Tmap.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap.DeltaU			// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step
	mov	Tmap.uv_delta[4], eax	// save whole step in non-v-carry slot
	add	eax, Tmap.src_offset				// calc whole step + v carry
	mov	Tmap.uv_delta[0], eax	// save in v-carry slot


OnePixelSpan:

	; setup initial coordinates
	mov	esi, Tmap.UFixed			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap.VFixed			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap.pixptr			// calc address


	mov	eax, Tmap.fx_l
	shr	eax, 8
	mov	bx, ax

//	mov	edx, Tmap.DeltaUFrac

	cmp	Tmap.WidthModLength, 1
	jle	NoDeltaLight

	push	ebx
	
	mov	ebx, Tmap.fx_l_right
	shr	ebx, 8
	
	sub	ebx, eax
	mov	eax, ebx
	
	mov	eax, Tmap.fx_dl_dx
	shr	eax, 8

	mov	edx, Tmap.DeltaUFrac
	mov	dx, ax
	mov	Tmap.DeltaUFrac, edx

	pop	ebx

NoDeltaLight:

	mov	ebp, Tmap.fx_w

	mov	eax, edi
	sub	eax, Tmap.pScreenBits
	shl	eax, 2
	mov	edx, gr_zbuffer
	add	edx, eax

	inc	Tmap.WidthModLength
	mov	eax,Tmap.WidthModLength
	shr	eax, 1
	jz		one_more_pix
	pushf
	mov	Tmap.WidthModLength, eax

	xor	eax, eax

	mov	al,[edi]                    // preread the destination cache line
	mov	al,[esi]



NextPixel:
			// Pixel 0
			cmp	ebp, [edx+0]							// Compare the Z depth of this pixel with zbuffer
			jle	Skip0a									// If pixel is covered, skip drawing
			mov	[edx+0], ebp							// Write new Z value

			// Get pixel and light it
			push	ebx
			xor	eax, eax									// Clear all bits of EAX.  This avoids a partial register stall on Pentium Pros
			mov	al, [esi]								// Get texel into AL
			and	ebx, 0ff00h								// Clear out fractional part of EBX
			mov	eax, DWORD PTR gr_fade_table[eax+ebx]		// Lookup pixel in lighting table
			pop	ebx

			mov	[edi+0],al								// Write new pixel

Skip0a:	add	ecx,Tmap.DeltaVFrac
			sbb	eax,eax

			//add	edx, 4								// Go to next
			add	ebp,Tmap.fx_dwdx

			add	ebx,Tmap.DeltaUFrac

			adc	esi,Tmap.uv_delta[4*eax+4]


			// Pixel 1
			cmp	ebp, [edx+4]							// Compare the Z depth of this pixel with zbuffer
			jle	Skip1a									// If pixel is covered, skip drawing
			mov	[edx+4], ebp							// Write new Z value

			// Get pixel and light it
			push	ebx
			xor	eax, eax									// Clear all bits of EAX.  This avoids a partial register stall on Pentium Pros
			mov	al, [esi]								// Get texel into AL
			and	ebx, 0ff00h								// Clear out fractional part of EBX
			mov	eax, DWORD PTR gr_fade_table[eax+ebx]		// Lookup pixel in lighting table
			pop	ebx

			mov	[edi+1],al								// Write new pixel

Skip1a:	add	ecx,Tmap.DeltaVFrac
			sbb	eax,eax

			//add	edx, 4								// Go to next
			add	ebp,Tmap.fx_dwdx

			add	ebx,Tmap.DeltaUFrac

			adc	esi,Tmap.uv_delta[4*eax+4]


	add	edi, 2
	add	edx, 8
	dec	Tmap.WidthModLength
	jg		NextPixel

	popf
	jnc	FPUReturn

one_more_pix:	

			cmp	ebp, [edx+0]							// Compare the Z depth of this pixel with zbuffer
			jle	Skip0c									// If pixel is covered, skip drawing
			mov	[edx+0], ebp							// Write new Z value

			// Get pixel and light it
			push	ebx
			xor	eax, eax									// Clear all bits of EAX.  This avoids a partial register stall on Pentium Pros
			mov	al, [esi]								// Get texel into AL
			and	ebx, 0ff00h								// Clear out fractional part of EBX
			mov	eax, DWORD PTR gr_fade_table[eax+ebx]		// Lookup pixel in lighting table
			pop	ebx

			mov	[edi+0],al								// Write new pixel

Skip0c:	

FPUReturn:

	// busy FPU registers:	// st0  st1  st2  st3  st4  st5  st6  st7
									// xxx  xxx  xxx  xxx  xxx  xxx  xxx
	ffree	st(0)
	ffree	st(1)
	ffree	st(2)
	ffree	st(3)
	ffree	st(4)
	ffree	st(5)
	ffree	st(6)

	fldcw	Tmap.OldFPUCW                  // restore the FPU

	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	pop	edx
	pop	ecx
	pop	eax
	}
}

void tmapscan_pln8_zbuffered_pentium()
{
	_asm {
	
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi

	// Put the FPU in low precision mode
	fstcw		Tmap.OldFPUCW					// store copy of CW
	mov		ax,Tmap.OldFPUCW				// get it in ax
	and		eax, ~0x300L
	mov		Tmap.FPUCW,ax					// store it
	fldcw		Tmap.FPUCW						// load the FPU


	mov		ecx, Tmap.loop_count		// ecx = width
	mov		edi, Tmap.dest_row_data	// edi = dest pointer

	// edi = pointer to start pixel in dest dib
	// ecx = spanwidth

	mov		eax,ecx							// eax and ecx = width
	shr		ecx,5								// ecx = width / subdivision length
	and		eax,31								// eax = width mod subdivision length
	jnz		some_left_over					// any leftover?
	dec		ecx								// no, so special case last span
	mov		eax,32								// it's 8 pixels long
some_left_over:
	mov		Tmap.Subdivisions,ecx		// store widths
	mov		Tmap.WidthModLength,eax

	// calculate ULeft and VLeft			// FPU Stack (ZL = ZLeft)
													// st0  st1  st2  st3  st4  st5  st6  st7
	fld		Tmap.l.v					// V/ZL 
	fld		Tmap.l.u					// U/ZL V/ZL 
	fld		Tmap.l.sw					// 1/ZL U/ZL V/ZL 
	fld1											// 1    1/ZL U/ZL V/ZL 
	fdiv		st,st(1)							// ZL   1/ZL U/ZL V/ZL 
	fld		st									// ZL   ZL   1/ZL U/ZL V/ZL 
	fmul		st,st(4)							// VL   ZL   1/ZL U/ZL V/ZL 
	fxch		st(1)								// ZL   VL   1/ZL U/ZL V/ZL 
	fmul		st,st(3)							// UL   VL   1/ZL U/ZL V/ZL 

	fstp		st(5)								// VL   1/ZL U/ZL V/ZL UL
	fstp		st(5)								// 1/ZL U/ZL V/ZL UL   VL

	// calculate right side OverZ terms  ; st0  st1  st2  st3  st4  st5  st6  st7

	fadd		Tmap.fl_dwdx_wide			// 1/ZR U/ZL V/ZL UL   VL
	fxch		st(1)								// U/ZL 1/ZR V/ZL UL   VL
	fadd		Tmap.fl_dudx_wide				// U/ZR 1/ZR V/ZL UL   VL
	fxch		st(2)								// V/ZL 1/ZR U/ZR UL   VL
	fadd		Tmap.fl_dvdx_wide				// V/ZR 1/ZR U/ZR UL   VL

	// calculate right side coords		// st0  st1  st2  st3  st4  st5  st6  st7

	fld1											// 1    V/ZR 1/ZR U/ZR UL   VL
	// @todo overlap this guy
	fdiv		st,st(2)							// ZR   V/ZR 1/ZR U/ZR UL   VL
	fld		st									// ZR   ZR   V/ZR 1/ZR U/ZR UL   VL
	fmul		st,st(2)							// VR   ZR   V/ZR 1/ZR U/ZR UL   VL
	fxch		st(1)								// ZR   VR   V/ZR 1/ZR U/ZR UL   VL
	fmul		st,st(4)							// UR   VR   V/ZR 1/ZR U/ZR UL   VL

	cmp		ecx,0							// check for any full spans
	jle      HandleLeftoverPixels
    
SpanLoop:

	// at this point the FPU contains	// st0  st1  st2  st3  st4  st5  st6  st7
													// UR   VR   V/ZR 1/ZR U/ZR UL   VL

	// convert left side coords

	fld     st(5)                       ; UL   UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fmul    Tmap.FixedScale            ; UL16 UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fistp   Tmap.UFixed                ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

	fld     st(6)                       ; VL   UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fmul    Tmap.FixedScale            ; VL16 UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fistp   Tmap.VFixed                ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

	// calculate deltas                  ; st0  st1  st2  st3  st4  st5  st6  st7

	fsubr   st(5),st                    ; UR   VR   V/ZR 1/ZR U/ZR dU   VL
	fxch    st(1)                       ; VR   UR   V/ZR 1/ZR U/ZR dU   VL
	fsubr   st(6),st                    ; VR   UR   V/ZR 1/ZR U/ZR dU   dV
	fxch    st(6)                       ; dV   UR   V/ZR 1/ZR U/ZR dU   VR

	fmul    Tmap.FixedScale8           ; dV8  UR   V/ZR 1/ZR U/ZR dU   VR
	fistp   Tmap.DeltaV                ; UR   V/ZR 1/ZR U/ZR dU   VR

	fxch    st(4)                       ; dU   V/ZR 1/ZR U/ZR UR   VR
	fmul    Tmap.FixedScale8           ; dU8  V/ZR 1/ZR U/ZR UR   VR
	fistp   Tmap.DeltaU                ; V/ZR 1/ZR U/ZR UR   VR

	// increment terms for next span    // st0  st1  st2  st3  st4  st5  st6  st7
	// Right terms become Left terms--->// V/ZL 1/ZL U/ZL UL   VL

	fadd    Tmap.fl_dvdx_wide				// V/ZR 1/ZL U/ZL UL   VL
	fxch    st(1)								// 1/ZL V/ZR U/ZL UL   VL
	fadd    Tmap.fl_dwdx_wide				// 1/ZR V/ZR U/ZL UL   VL
	fxch    st(2)								// U/ZL V/ZR 1/ZR UL   VL
	fadd    Tmap.fl_dudx_wide				// U/ZR V/ZR 1/ZR UL   VL
	fxch    st(2)								// 1/ZR V/ZR U/ZR UL   VL
	fxch    st(1)								// V/ZR 1/ZR U/ZR UL   VL


	// setup delta values
    
	mov     eax,Tmap.DeltaV				// get v 16.16 step
	mov     ebx,eax							// copy it
	sar     eax,16								// get v int step
	shl     ebx,16								// get v frac step
	mov     Tmap.DeltaVFrac,ebx			// store it
	imul    eax,Tmap.src_offset			// calculate texture step for v int step

	mov     ebx,Tmap.DeltaU				// get u 16.16 step
	mov     ecx,ebx							// copy it
	sar     ebx,16								// get u int step
	shl     ecx,16								// get u frac step
	mov     Tmap.DeltaUFrac,ecx			// store it
	add     eax,ebx							// calculate uint + vint step
	mov     Tmap.uv_delta[4],eax			// save whole step in non-v-carry slot
	add     eax,Tmap.src_offset			// calculate whole step + v carry
	mov     Tmap.uv_delta[0],eax			// save in v-carry slot

	// setup initial coordinates
	mov     esi,Tmap.UFixed				// get u 16.16 fixedpoint coordinate

	mov     ebx,esi							// copy it
	sar     esi,16								// get integer part
	shl     ebx,16								// get fractional part

	mov     ecx,Tmap.VFixed				// get v 16.16 fixedpoint coordinate
   
	mov     edx,ecx							// copy it
	sar     edx,16								// get integer part
	shl     ecx,16								// get fractional part
	imul    edx,Tmap.src_offset			// calc texture scanline address
	add     esi,edx							// calc texture offset
	add     esi,Tmap.pixptr				// calc address

	// set up affine registers

	mov	eax, Tmap.fx_l
	shr	eax, 8
	mov	bx, ax

	mov	ebp, Tmap.fx_dl_dx
	shl	ebp, 5	//*32
	add	Tmap.fx_l, ebp

	mov	ebp, Tmap.fx_l
	shr	ebp, 8
	sub	bp, ax
	shr	bp, 5

	mov	edx, Tmap.DeltaUFrac
	mov	dx, bp
	mov	Tmap.DeltaUFrac, edx


	// calculate right side coords		st0  st1  st2  st3  st4  st5  st6  st7
	fld1										// 1    V/ZR 1/ZR U/ZR UL   VL
	// This divide should happen while the pixel span is drawn.
	fdiv	st,st(2)							// ZR   V/ZR 1/ZR U/ZR UL   VL


	// 8 pixel span code
	// edi = dest dib bits at current pixel
	// esi = texture pointer at current u,v
	// eax = scratch
	// ebx = u fraction 0.32
	// ecx = v fraction 0.32
	// edx = u frac step
	// ebp = v carry scratch

	mov	al,[edi]								// preread the destination cache line
	mov	al,[esi]								// get texture pixel 0

	mov	Tmap.InnerLooper, 32/4			// Set up loop counter

	mov	ebp, Tmap.fx_w

	mov	edx, gr_zbuffer

	mov	eax, edi
	sub	eax, Tmap.pScreenBits
	shl	eax, 2
	add	edx, eax

InnerInnerLoop:

			// Pixel 0
			mov	eax, ebx								// Get lighting value from BH into AH
			and	eax, 0ffffh;						// Clear upper bits of EAX

			cmp	ebp, [edx+0]							// Compare the Z depth of this pixel with zbuffer
			mov	al, [esi]							// Get texel into AL
			jle	Skip0									// If pixel is covered, skip drawing

			mov	[edx+0], ebp							// Write new Z value

			mov	al, gr_fade_table[eax]			// Lookup pixel in lighting table
			mov	[edi+0],al								// Write new pixel

Skip0:	add	ecx,Tmap.DeltaVFrac
			sbb	eax,eax

			//add	edx, 4								// Go to next
			add	ebp,Tmap.fx_dwdx

			add	ebx,Tmap.DeltaUFrac

			adc	esi,Tmap.uv_delta[4*eax+4]


			// Pixel 1
			mov	eax, ebx								// Get lighting value from BH into AH
			and	eax, 0ffffh;						// Clear upper bits of EAX

			cmp	ebp, [edx+4]							// Compare the Z depth of this pixel with zbuffer
			mov	al, [esi]							// Get texel into AL
			jle	Skip1									// If pixel is covered, skip drawing


			mov	[edx+4], ebp							// Write new Z value

			mov	al, gr_fade_table[eax]			// Lookup pixel in lighting table
			mov	[edi+1],al								// Write new pixel

Skip1:	add	ecx,Tmap.DeltaVFrac
			sbb	eax,eax

			//add	edx, 4								// Go to next
			add	ebp,Tmap.fx_dwdx

			add	ebx,Tmap.DeltaUFrac

			adc	esi,Tmap.uv_delta[4*eax+4]

			// Pixel 2

			mov	eax, ebx								// Get lighting value from BH into AH
			and	eax, 0ffffh;						// Clear upper bits of EAX


			cmp	ebp, [edx+8]							// Compare the Z depth of this pixel with zbuffer
			mov	al, [esi]							// Get texel into AL
			jle	Skip2									// If pixel is covered, skip drawing


			mov	[edx+8], ebp							// Write new Z value

			mov	al, gr_fade_table[eax]			// Lookup pixel in lighting table
			mov	[edi+2],al								// Write new pixel

Skip2:	add	ecx,Tmap.DeltaVFrac
			sbb	eax,eax

			//add	edx, 4								// Go to next
			add	ebp,Tmap.fx_dwdx

			add	ebx,Tmap.DeltaUFrac

			adc	esi,Tmap.uv_delta[4*eax+4]

			// Pixel 3
			mov	eax, ebx								// Get lighting value from BH into AH
			and	eax, 0ffffh;						// Clear upper bits of EAX


			cmp	ebp, [edx+12]							// Compare the Z depth of this pixel with zbuffer
			mov	al, [esi]							// Get texel into AL
			jle	Skip3									// If pixel is covered, skip drawing


			mov	[edx+12], ebp							// Write new Z value

			mov	al, gr_fade_table[eax]			// Lookup pixel in lighting table
			mov	[edi+3],al								// Write new pixel

Skip3:	add	ecx,Tmap.DeltaVFrac
			sbb	eax,eax

			//add	edx, 4								// Go to next
			add	ebp,Tmap.fx_dwdx

			add	ebx,Tmap.DeltaUFrac

			adc	esi,Tmap.uv_delta[4*eax+4]


	add	edi, 4
	add	edx, 16
	dec	Tmap.InnerLooper
	jnz	InnerInnerLoop

	mov	Tmap.fx_w, ebp

	// the fdiv is done, finish right	// st0  st1  st2  st3  st4  st5  st6  st7
	                                    // ZR   V/ZR 1/ZR U/ZR UL   VL

    fld     st									// ZR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(2)							// VR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fxch    st(1)								// ZR   VR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(4)							// UR   VR   V/ZR 1/ZR U/ZR UL   VL

    dec     Tmap.Subdivisions			// decrement span count
    jnz     SpanLoop							// loop back


HandleLeftoverPixels:

    mov     esi,Tmap.pixptr				// load texture pointer

    // edi = dest dib bits
    // esi = current texture dib bits
    // at this point the FPU contains    ; st0  st1  st2  st3  st4  st5  st6  st7
    // inv. means invalid numbers        ; inv. inv. inv. inv. inv. UL   VL

    cmp     Tmap.WidthModLength,0          ; are there remaining pixels to draw?
    jz      FPUReturn                   ; nope, pop the FPU and bail

    // convert left side coords          ; st0  st1  st2  st3  st4  st5  st6  st7

    fld     st(5)                       ; UL   inv. inv. inv. inv. inv. UL   VL
    fmul    Tmap.FixedScale                ; UL16 inv. inv. inv. inv. inv. UL   VL
    fistp   Tmap.UFixed                    ; inv. inv. inv. inv. inv. UL   VL

    fld     st(6)                       ; VL   inv. inv. inv. inv. inv. UL   VL
    fmul    Tmap.FixedScale                // VL16 inv. inv. inv. inv. inv. UL   VL
    fistp   Tmap.VFixed                    ; inv. inv. inv. inv. inv. UL   VL

    dec     Tmap.WidthModLength            ; calc how many steps to take
    jz      OnePixelSpan                ; just one, don't do deltas

    // calculate right edge coordinates  ; st0  st1  st2  st3  st4  st5  st6  st7
    // r -> R+1

    // @todo rearrange things so we don't need these two instructions
    fstp    Tmap.FloatTemp                 ; inv. inv. inv. inv. UL   VL
    fstp    Tmap.FloatTemp                 ; inv. inv. inv. UL   VL

    fld     Tmap.r.v           ; V/Zr inv. inv. inv. UL   VL
    fsub    Tmap.deltas.v             ; V/ZR inv. inv. inv. UL   VL
    fld     Tmap.r.u           ; U/Zr V/ZR inv. inv. inv. UL   VL
    fsub    Tmap.deltas.u             ; U/ZR V/ZR inv. inv. inv. UL   VL
    fld     Tmap.r.sw              ; 1/Zr U/ZR V/ZR inv. inv. inv. UL   VL
    fsub    Tmap.deltas.sw           ; 1/ZR U/ZR V/ZR inv. inv. inv. UL   VL

    fdivr   Tmap.One                       ; ZR   U/ZR V/ZR inv. inv. inv. UL   VL

    fmul    st(1),st                    ; ZR   UR   V/ZR inv. inv. inv. UL   VL
    fmulp   st(2),st                    ; UR   VR   inv. inv. inv. UL   VL

    // calculate deltas                  ; st0  st1  st2  st3  st4  st5  st6  st7

    fsubr   st(5),st                    ; UR   VR   inv. inv. inv. dU   VL
    fxch    st(1)                       ; VR   UR   inv. inv. inv. dU   VL
    fsubr   st(6),st                    ; VR   UR   inv. inv. inv. dU   dV
    fxch    st(6)                       ; dV   UR   inv. inv. inv. dU   VR

    fidiv   Tmap.WidthModLength            ; dv   UR   inv. inv. inv. dU   VR
    fmul    Tmap.FixedScale                ; dv16 UR   inv. inv. inv. dU   VR
    fistp   Tmap.DeltaV                    ; UR   inv. inv. inv. dU   VR

    fxch    st(4)                       ; dU   inv. inv. inv. UR   VR
    fidiv   Tmap.WidthModLength            ; du   inv. inv. inv. UR   VR
    fmul    Tmap.FixedScale                ; du16 inv. inv. inv. UR   VR
    fistp   Tmap.DeltaU                    ; inv. inv. inv. UR   VR

    // @todo gross!  these are to line up with the other loop
    fld     st(1)                       ; inv. inv. inv. inv. UR   VR
    fld     st(2)                       ; inv. inv. inv. inv. inv. UR   VR


	// setup delta values
	mov	eax, Tmap.DeltaV	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap.DeltaVFrac, ebx	// store it
	imul	eax, Tmap.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap.DeltaU			// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step
	mov	Tmap.uv_delta[4], eax	// save whole step in non-v-carry slot
	add	eax, Tmap.src_offset				// calc whole step + v carry
	mov	Tmap.uv_delta[0], eax	// save in v-carry slot


OnePixelSpan:

	; setup initial coordinates
	mov	esi, Tmap.UFixed			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap.VFixed			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap.pixptr			// calc address


	mov	eax, Tmap.fx_l
	shr	eax, 8
	mov	bx, ax

//	mov	edx, Tmap.DeltaUFrac

	cmp	Tmap.WidthModLength, 1
	jle	NoDeltaLight

	push	ebx
	
	mov	ebx, Tmap.fx_l_right
	shr	ebx, 8
	
	sub	ebx, eax
	mov	eax, ebx
	
	mov	eax, Tmap.fx_dl_dx
	shr	eax, 8

	mov	edx, Tmap.DeltaUFrac
	mov	dx, ax
	mov	Tmap.DeltaUFrac, edx

	pop	ebx

NoDeltaLight:

	mov	ebp, Tmap.fx_w

	mov	eax, edi
	sub	eax, Tmap.pScreenBits
	mov	edx, gr_zbuffer
	shl	eax, 2
	add	edx, eax

	inc	Tmap.WidthModLength
	mov	eax,Tmap.WidthModLength
	shr	eax, 1
	jz		one_more_pix
	pushf
	mov	Tmap.WidthModLength, eax

	xor	eax, eax

	mov	al,[edi]                    // preread the destination cache line
	mov	al,[esi]



NextPixel:
			// Pixel 0
			mov	eax, ebx								// Get lighting value from BH into AH
			and	eax, 0ffffh;						// Clear upper bits of EAX

			cmp	ebp, [edx+0]							// Compare the Z depth of this pixel with zbuffer
			mov	al, [esi]							// Get texel into AL
			jle	Skip0a									// If pixel is covered, skip drawing


			mov	[edx+0], ebp							// Write new Z value

			mov	al, gr_fade_table[eax]			// Lookup pixel in lighting table
			mov	[edi+0],al								// Write new pixel

Skip0a:	add	ecx,Tmap.DeltaVFrac
			sbb	eax,eax

			//add	edx, 4								// Go to next
			add	ebp,Tmap.fx_dwdx

			add	ebx,Tmap.DeltaUFrac

			adc	esi,Tmap.uv_delta[4*eax+4]


			// Pixel 1
			mov	eax, ebx								// Get lighting value from BH into AH
			and	eax, 0ffffh;						// Clear upper bits of EAX


			cmp	ebp, [edx+4]							// Compare the Z depth of this pixel with zbuffer
			mov	al, [esi]							// Get texel into AL
			jle	Skip1a									// If pixel is covered, skip drawing

			mov	[edx+4], ebp							// Write new Z value

			mov	al, gr_fade_table[eax]			// Lookup pixel in lighting table
			mov	[edi+1],al								// Write new pixel

Skip1a:	add	ecx,Tmap.DeltaVFrac
			sbb	eax,eax

			//add	edx, 4								// Go to next
			add	ebp,Tmap.fx_dwdx

			add	ebx,Tmap.DeltaUFrac

			adc	esi,Tmap.uv_delta[4*eax+4]


	add	edi, 2
	add	edx, 8
	dec	Tmap.WidthModLength
	jg		NextPixel

	popf
	jnc	FPUReturn

one_more_pix:	

			mov	eax, ebx								// Get lighting value from BH into AH
			and	eax, 0ffffh;						// Clear upper bits of EAX

			cmp	ebp, [edx+0]							// Compare the Z depth of this pixel with zbuffer
			mov	al, [esi]							// Get texel into AL
			jle	Skip0c									// If pixel is covered, skip drawing

			mov	al, gr_fade_table[eax]			// Lookup pixel in lighting table

			mov	[edx+0], ebp							// Write new Z value

			mov	[edi+0],al								// Write new pixel

Skip0c:	

FPUReturn:

	// busy FPU registers:	// st0  st1  st2  st3  st4  st5  st6  st7
									// xxx  xxx  xxx  xxx  xxx  xxx  xxx
	ffree	st(0)
	ffree	st(1)
	ffree	st(2)
	ffree	st(3)
	ffree	st(4)
	ffree	st(5)
	ffree	st(6)

	fldcw	Tmap.OldFPUCW                  // restore the FPU

	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	pop	edx
	pop	ecx
	pop	eax
	}
}

void tmapscan_pln8_zbuffered()
{
	if ( Gr_cpu	> 5 )	{
		// Pentium Pro optimized code.
		tmapscan_pln8_zbuffered_ppro();
	} else {
		tmapscan_pln8_zbuffered_pentium();
	}
}

void tmapscan_lnaa8_zbuffered()
{
#ifndef HARDWARE_ONLY
	Tmap.lookup = (uint)&Current_alphacolor->table.lookup[0][0];

	_asm {
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi

	; setup delta values
	mov	eax, Tmap.fx_dv_dx	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap.DeltaVFrac, ebx	// store it
	imul	eax, Tmap.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap.fx_du_dx		// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step

	mov	Tmap.uv_delta[4], eax	// save whole step in non-v-carry slot
	add	eax, Tmap.src_offset				// calc whole step + v carry
	mov	Tmap.uv_delta[0], eax	// save in v-carry slot

	; setup initial coordinates
	mov	esi, Tmap.fx_u			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap.fx_v			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap.pixptr			// calc address
	
	; set edi = address of first pixel to modify
	mov	edi, Tmap.dest_row_data
	
	xor	eax, eax
	mov	al,[edi]							// get the destination pixel

	mov	ebp, Tmap.fx_w
	mov	edx, gr_zbuffer
	mov	eax, edi
	sub	eax, Tmap.pScreenBits
	shl	eax, 2
	add	edx, eax

	mov	eax, Tmap.loop_count

	shr	eax, 2
	je		DoLeftOverPixels

	mov	Tmap.num_big_steps, eax
	and	Tmap.loop_count, 3

NextPixelBlock:

    // 8 pixel span code
    // edi = dest dib bits at current pixel
    // esi = texture pointer at current u,v
    // eax = scratch
    // ebx = u fraction 0.32
    // ecx = v fraction 0.32
    // edx = u frac step
    // ebp = v carry scratch

	cmp	ebp, [edx+4*0]						// Compare the Z depth of this pixel with zbuffer
	jle	Skip0a								// If pixel is covered, skip drawing
//	mov	[edx+4*0], ebp						// Write new Z value
	mov	al,[edi+0]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	and	eax, 0ffffh
	add	eax, Tmap.lookup
	mov	al, [eax]			// blend them
	mov	[edi+0],al							// store pixel
Skip0a:
	add	ebp, Tmap.fx_dwdx
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	eax,eax								// get -1 if carry
	add	ebx,Tmap.DeltaUFrac				// increment u fraction
	adc	esi,Tmap.uv_delta[4*eax+4]		// add in step ints & carries


	cmp	ebp, [edx+4*1]						// Compare the Z depth of this pixel with zbuffer
	jle	Skip1a								// If pixel is covered, skip drawing
//	mov	[edx+4*1], ebp						// Write new Z value
	mov	al,[edi+1]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	and	eax, 0ffffh
	add	eax, Tmap.lookup
	mov	al, [eax]			// blend them
	mov	[edi+1],al							// store pixel
Skip1a:
	add	ebp, Tmap.fx_dwdx
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	eax,eax								// get -1 if carry
	add	ebx,Tmap.DeltaUFrac				// increment u fraction
	adc	esi,Tmap.uv_delta[4*eax+4]		// add in step ints & carries

	cmp	ebp, [edx+4*2]						// Compare the Z depth of this pixel with zbuffer
	jle	Skip2a								// If pixel is covered, skip drawing
//	mov	[edx+4*2], ebp						// Write new Z value
	mov	al,[edi+2]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	and	eax, 0ffffh
	add	eax, Tmap.lookup
	mov	al, [eax]			// blend them
	mov	[edi+2],al							// store pixel
Skip2a:
	add	ebp, Tmap.fx_dwdx
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	eax,eax								// get -1 if carry
	add	ebx,Tmap.DeltaUFrac				// increment u fraction
	adc	esi,Tmap.uv_delta[4*eax+4]		// add in step ints & carries

	cmp	ebp, [edx+4*3]						// Compare the Z depth of this pixel with zbuffer
	jle	Skip3a								// If pixel is covered, skip drawing
//	mov	[edx+4*3], ebp						// Write new Z value
	mov	al,[edi+3]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	and	eax, 0ffffh
	add	eax, Tmap.lookup
	mov	al, [eax]			// blend them
	mov	[edi+3],al							// store pixel
Skip3a:
	add	ebp, Tmap.fx_dwdx
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	eax,eax								// get -1 if carry
	add	ebx,Tmap.DeltaUFrac				// increment u fraction
	adc	esi,Tmap.uv_delta[4*eax+4]		// add in step ints & carries

	add	edx, 16
	add	edi, 4
	dec	Tmap.num_big_steps
	jne	NextPixelBlock
	

DoLeftOverPixels:

	mov	eax,Tmap.loop_count
	test	eax, -1
	jz	_none_to_do
	shr	eax, 1
	je	one_more_pix
	mov	Tmap.loop_count, eax
	pushf

	xor	eax, eax
	mov	al,[edi]							// get the destination pixel

NextPixel:

	cmp	ebp, [edx+4*0]						// Compare the Z depth of this pixel with zbuffer
	jle	Skip0b								// If pixel is covered, skip drawing
//	mov	[edx+4*0], ebp						// Write new Z value
	mov	al,[edi+0]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	and	eax, 0ffffh
	add	eax, Tmap.lookup
	mov	al, [eax]			// blend them
	mov	[edi+0],al							// store pixel
Skip0b:
	add	ebp, Tmap.fx_dwdx
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	eax,eax								// get -1 if carry
	add	ebx,Tmap.DeltaUFrac				// increment u fraction
	adc	esi,Tmap.uv_delta[4*eax+4]		// add in step ints & carries

	cmp	ebp, [edx+4*1]						// Compare the Z depth of this pixel with zbuffer
	jle	Skip1b								// If pixel is covered, skip drawing
//	mov	[edx+4*1], ebp						// Write new Z value
	mov	al,[edi+1]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	and	eax, 0ffffh
	add	eax, Tmap.lookup
	mov	al, [eax]			// blend them
	mov	[edi+1],al							// store pixel
Skip1b:
	add	ebp, Tmap.fx_dwdx
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	eax,eax								// get -1 if carry
	add	ebx,Tmap.DeltaUFrac				// increment u fraction
	adc	esi,Tmap.uv_delta[4*eax+4]		// add in step ints & carries

	add	edi, 2
	add	edx, 8
	dec	Tmap.loop_count
	jne	NextPixel

	popf
	jnc	_none_to_do

one_more_pix:	
	cmp	ebp, [edx]						// Compare the Z depth of this pixel with zbuffer
	jle	Skip0c								// If pixel is covered, skip drawing
//	mov	[edx], ebp						// Write new Z value
	mov	al,[edi]							// get the destination pixel
	mov	ah,[esi]							// get texture pixel 0
	and	eax, 0ffffh
	add	eax, Tmap.lookup
	mov	al, [eax]			// blend them
	mov	[edi],al							// store pixel
Skip0c:

_none_to_do:	
	pop	edi
  	pop	esi
	pop	ebp
	pop	ebx
	pop	edx
	pop	ecx
	pop	eax
	}
#else
	Int3();
#endif
}

void tmapscan_lnaa8()
{
#ifndef HARDWARE_ONLY
	if (gr_zbuffering) {
		switch(gr_zbuffering_mode)	{
		case GR_ZBUFF_NONE:
			break;
		case GR_ZBUFF_FULL:		// both
		case GR_ZBUFF_WRITE:		// write only
		case GR_ZBUFF_READ:		// read only
			tmapscan_lnaa8_zbuffered();
			return;
		}

	}

	Tmap.lookup = (uint)&Current_alphacolor->table.lookup[0][0];

	_asm {
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi

	; setup delta values
	mov	eax, Tmap.fx_dv_dx	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap.DeltaVFrac, ebx	// store it
	imul	eax, Tmap.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap.fx_du_dx		// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step

	mov	Tmap.uv_delta[4], eax	// save whole step in non-v-carry slot
	add	eax, Tmap.src_offset				// calc whole step + v carry
	mov	Tmap.uv_delta[0], eax	// save in v-carry slot

	; setup initial coordinates
	mov	esi, Tmap.fx_u			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap.fx_v			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap.pixptr			// calc address
	
	; set edi = address of first pixel to modify
	mov	edi, Tmap.dest_row_data
	
	mov	edx, Tmap.DeltaUFrac

	xor	eax, eax
	mov	al,[edi]							// get the destination pixel

	mov	ebp, Tmap.loop_count

	shr	ebp, 2
	je		DoLeftOverPixels

	mov	Tmap.num_big_steps, ebp
	and	Tmap.loop_count, 3


NextPixelBlock:

    // 8 pixel span code
    // edi = dest dib bits at current pixel
    // esi = texture pointer at current u,v
    // eax = scratch
    // ebx = u fraction 0.32
    // ecx = v fraction 0.32
    // edx = u frac step
    // ebp = v carry scratch


	mov	al,[edi+0]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries
	add	eax, Tmap.lookup
	mov	al, [eax]			// blend them
	mov	[edi+0],al							// store pixel

	mov	al,[edi+1]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries
	add	eax, Tmap.lookup
	mov	al, [eax]			// blend them
	mov	[edi+1],al							// store pixel

	mov	al,[edi+2]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries
	add	eax, Tmap.lookup
	mov	al, [eax]			// blend them
	mov	[edi+2],al							// store pixel

	mov	al,[edi+3]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries
	add	eax, Tmap.lookup
	mov	al, [eax]			// blend them
	mov	[edi+3],al							// store pixel

	add	edi, 4
	dec	Tmap.num_big_steps
	jne	NextPixelBlock
	

DoLeftOverPixels:

	mov	ebp,Tmap.loop_count
	test	ebp, -1
	jz	_none_to_do
	shr	ebp, 1
	je	one_more_pix
	mov	Tmap.loop_count, ebp
	pushf

	xor	eax, eax
	mov	al,[edi]							// get the destination pixel

NextPixel:

	mov	al,[edi+0]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries
	add	eax, Tmap.lookup
	mov	al, [eax]			// blend them
	mov	[edi+0],al							// store pixel

	mov	al,[edi+1]							// get the destination pixel
	mov	ah,[esi]								// get texture pixel 0
	add	ecx,Tmap.DeltaVFrac				// increment v fraction
	sbb	ebp,ebp								// get -1 if carry
	add	ebx,edx								// increment u fraction
	adc	esi,Tmap.uv_delta[4*ebp+4]		// add in step ints & carries
	add	eax, Tmap.lookup
	mov	al, [eax]			// blend them
	mov	[edi+1],al							// store pixel

	add	edi, 2
	dec	Tmap.loop_count
	jne	NextPixel

	popf
	jnc	_none_to_do

one_more_pix:	
	mov	al,[edi]							// get the destination pixel
	mov	ah,[esi]							// get texture pixel 0
	add	eax, Tmap.lookup
	mov	al, [eax]			// blend them
	mov	[edi],al							// store pixel

_none_to_do:	
	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	pop	edx
	pop	ecx
	pop	eax
	}
#else
	Int3();
#endif
}





