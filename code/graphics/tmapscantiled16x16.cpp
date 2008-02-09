/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/TmapScanTiled16x16.cpp $
 * $Revision: 2.0 $
 * $Date: 2002-06-03 04:02:23 $
 * $Author: penguin $
 *
 * Routines for drawing tiled 16x16 textues
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
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
 * 6     4/23/98 9:55a John
 * Fixed some bugs in the tiled tmapper causing bright dots to appear all
 * over models.
 * 
 * 5     3/10/98 4:19p John
 * Cleaned up graphics lib.  Took out most unused gr functions.   Made D3D
 * & Glide have popups and print screen.  Took out all >8bpp software
 * support.  Made Fred zbuffer.  Made zbuffer allocate dynamically to
 * support Fred.  Made zbuffering key off of functions rather than one
 * global variable.
 * 
 * 4     1/23/98 5:08p John
 * Took L out of vertex structure used B (blue) instead.   Took all small
 * fireballs out of fireball types and used particles instead.  Fixed some
 * debris explosion things.  Restructured fireball code.   Restructured
 * some lighting code.   Made dynamic lighting on by default. Made groups
 * of lasers only cast one light.  Made fireballs not cast light.
 * 
 * 3     12/04/97 10:38a John
 * Fixed tiled texture mappers that were swapping uvs.
 * 
 * 2     10/14/97 9:19a John
 * removed fdiv warnings.
 * 
 * 1     6/18/97 4:02p John
 * added new code for 16x16 and 32x32 tiled tmaps.
 *
 * $NoKeywords: $
 */


#include "3d.h"
#include "2d.h"
#include "grinternal.h"
#include "tmapper.h"
#include "tmapscanline.h"
#include "floating.h"
#include "palman.h"
#include "fix.h"

// Needed to keep warning 4725 to stay away.  See PsTypes.h for details why.
void disable_warning_4725_stub_tst16()
{
}

void tmapscan_pln8_zbuffered_tiled_16x16()
{
	Tmap.fx_l = fl2f(Tmap.l.b*32.0); 
	Tmap.fx_l_right = fl2f(Tmap.r.b*32.0); 
	Tmap.fx_dl_dx = fl2f(Tmap.deltas.b*32.0);

	if ( Tmap.fx_dl_dx < 0 )	{
		Tmap.fx_dl_dx = -Tmap.fx_dl_dx;
		Tmap.fx_l = (67*F1_0)-Tmap.fx_l;
		Tmap.fx_l_right = (67*F1_0)-Tmap.fx_l_right;
//		Assert( Tmap.fx_l > 31*F1_0 );
//		Assert( Tmap.fx_l < 66*F1_0 );
//		Assert( Tmap.fx_dl_dx >= 0 );
//		Assert( Tmap.fx_dl_dx < 31*F1_0 );
	}

	Tmap.fl_dudx_wide = Tmap.deltas.u*32.0f;
	Tmap.fl_dvdx_wide = Tmap.deltas.v*32.0f;
	Tmap.fl_dwdx_wide = Tmap.deltas.sw*32.0f;

	Tmap.fx_w = fl2i(Tmap.l.sw * GR_Z_RANGE)+gr_zoffset;
	Tmap.fx_dwdx = fl2i(Tmap.deltas.sw * GR_Z_RANGE);

//	Assert(Tmap.fx_w < 65536 );
//	Assert(Tmap.fx_w >= 0 );
//	Assert(Tmap.fx_w+Tmap.fx_dwdx*Tmap.loop_count < 65536 );
//	Assert(Tmap.fx_w+Tmap.fx_dwdx*Tmap.loop_count >= 0 );

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

	mov	Tmap.InnerLooper, 32/4			// Set up loop counter

	mov	edx, gr_zbuffer

	mov	eax, edi
	sub	eax, Tmap.pScreenBits
	shl	eax, 2
	add	edx, eax

// Make ESI =  DV:DU in 5:11,5:11 format
	mov	eax, Tmap.DeltaV
	shr	eax, 4
	mov	esi, Tmap.DeltaU
	shl	esi, 12
	mov	si, ax
	mov	Tmap.DeltaUFrac, esi
		
// Make ECX = V:U in 5:11,5:11 format
	mov	eax, Tmap.VFixed
	shr	eax, 4
	mov	ecx, Tmap.UFixed
	shl	ecx, 12
	mov	cx, ax

	mov	esi, Tmap.fx_w

	// eax = tmp
	// ebx = light
	// ecx = V:U in 8.6:10.8
	// edx = zbuffer pointer
	// esi = z
	// edi = screen data
	// ebp = dl_dx


InnerInnerLoop:

		// pixel 0
		cmp	esi, [edx+0]					// Compare the Z depth of this pixel with zbuffer
		jle	Skip0								// If pixel is covered, skip drawing

		mov	[edx+0], esi					// Write z

		mov	eax, ecx							// EAX = V.VF:U.UF in 6.10:6.10
		shr	ax, 12								// EAX = V:U in 6.10:16.0
		rol	eax, 4							// EAX = V:U in 0.0:6:6
		and	eax, 0ffh						// clear upper bits
		add	eax, Tmap.pixptr		// EAX = (V*64)+U + Pixptr

		mov	al, [eax]			
		mov	ah, bh	
		and	eax, 0ffffh						// clear upper bits
		mov	al, gr_fade_table[eax]
		mov	[edi+0], al
Skip0:
		add	ecx, Tmap.DeltaUFrac
		add	esi, Tmap.fx_dwdx
		add	ebx, ebp 

		// pixel 1
		cmp	esi, [edx+4]					// Compare the Z depth of this pixel with zbuffer
		jle	Skip1								// If pixel is covered, skip drawing

		mov	[edx+4], esi					// Write z

		mov	eax, ecx							// EAX = V.VF:U.UF in 6.10:6.10
		shr	ax, 12								// EAX = V:U in 6.10:16.0
		rol	eax, 4							// EAX = V:U in 0.0:6:6
		and	eax, 0ffh						// clear upper bits
		add	eax, Tmap.pixptr		// EAX = (V*64)+U + Pixptr

		mov	al, [eax]			
		mov	ah, bh	
		and	eax, 0ffffh						// clear upper bits
		mov	al, gr_fade_table[eax]
		mov	[edi+1], al
Skip1:
		add	ecx, Tmap.DeltaUFrac
		add	esi, Tmap.fx_dwdx
		add	ebx, ebp 

		// pixel 2
		cmp	esi, [edx+8]					// Compare the Z depth of this pixel with zbuffer
		jle	Skip2								// If pixel is covered, skip drawing

		mov	[edx+8], esi					// Write z

		mov	eax, ecx							// EAX = V.VF:U.UF in 6.10:6.10
		shr	ax, 12								// EAX = V:U in 6.10:16.0
		rol	eax, 4							// EAX = V:U in 0.0:6:6
		and	eax, 0ffh						// clear upper bits
		add	eax, Tmap.pixptr		// EAX = (V*64)+U + Pixptr

		mov	al, [eax]			
		mov	ah, bh	
		and	eax, 0ffffh						// clear upper bits
		mov	al, gr_fade_table[eax]
		mov	[edi+2], al
Skip2:
		add	ecx, Tmap.DeltaUFrac
		add	esi, Tmap.fx_dwdx
		add	ebx, ebp 

		// pixel 3
		cmp	esi, [edx+12]					// Compare the Z depth of this pixel with zbuffer
		jle	Skip3								// If pixel is covered, skip drawing

		mov	[edx+12], esi					// Write z

		mov	eax, ecx							// EAX = V.VF:U.UF in 6.10:6.10
		shr	ax, 12								// EAX = V:U in 6.10:16.0
		rol	eax, 4							// EAX = V:U in 0.0:6:6
		and	eax, 0ffh						// clear upper bits
		add	eax, Tmap.pixptr		// EAX = (V*64)+U + Pixptr

		mov	al, [eax]			
		mov	ah, bh	
		and	eax, 0ffffh						// clear upper bits
		mov	al, gr_fade_table[eax]
		mov	[edi+3], al
Skip3:
		add	ecx, Tmap.DeltaUFrac
		add	esi, Tmap.fx_dwdx
		add	ebx, ebp 


	add	edi, 4
	add	edx, 16
	dec	Tmap.InnerLooper
	jnz	InnerInnerLoop

	mov	Tmap.fx_w, esi

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

	push	ebx
	
	mov	ebx, Tmap.fx_l_right
	shr	ebx, 8
	
	sub	ebx, eax
	mov	eax, ebx
	
	mov	eax, Tmap.fx_dl_dx
	shr	eax, 8

	mov	bp, ax

	pop	ebx


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

// Make ESI =  DV:DU in 6:10,6:10 format
	mov	eax, Tmap.DeltaV
	shr	eax, 4
	mov	esi, Tmap.DeltaU
	shl	esi, 12
	mov	si, ax
	mov	Tmap.DeltaUFrac, esi
		
// Make ECX = V:U in 6:10,6:10 format
	mov	eax, Tmap.VFixed
	shr	eax, 4
	mov	ecx, Tmap.UFixed
	shl	ecx, 12
	mov	cx, ax

	mov	esi, Tmap.fx_w

	// eax = tmp
	// ebx = light
	// ecx = V:U in 8.6:10.8
	// edx = zbuffer pointer
	// esi = z
	// edi = screen data
	// ebp = dl_dx



NextPixel:
		// pixel 0
		cmp	esi, [edx+0]					// Compare the Z depth of this pixel with zbuffer
		jle	Skip0a								// If pixel is covered, skip drawing

		mov	[edx+0], esi					// Write z

		mov	eax, ecx							// EAX = V.VF:U.UF in 6.10:6.10
		shr	ax, 12								// EAX = V:U in 6.10:16.0
		rol	eax, 4							// EAX = V:U in 0.0:6:6
		and	eax, 0ffh						// clear upper bits
		add	eax, Tmap.pixptr		// EAX = (V*64)+U + Pixptr

		mov	al, [eax]			
		mov	ah, bh	
		and	eax, 0ffffh						// clear upper bits
		mov	al, gr_fade_table[eax]
		mov	[edi+0], al
Skip0a:
		add	ecx, Tmap.DeltaUFrac
		add	esi, Tmap.fx_dwdx
		add	ebx, ebp 

		// pixel 1
		cmp	esi, [edx+4]					// Compare the Z depth of this pixel with zbuffer
		jle	Skip1a								// If pixel is covered, skip drawing

		mov	[edx+4], esi					// Write z

		mov	eax, ecx							// EAX = V.VF:U.UF in 6.10:6.10
		shr	ax, 12								// EAX = V:U in 6.10:16.0
		rol	eax, 4							// EAX = V:U in 0.0:6:6
		and	eax, 0ffh						// clear upper bits
		add	eax, Tmap.pixptr		// EAX = (V*64)+U + Pixptr

		mov	al, [eax]			
		mov	ah, bh	
		and	eax, 0ffffh						// clear upper bits
		mov	al, gr_fade_table[eax]
		mov	[edi+1], al
Skip1a:
		add	ecx, Tmap.DeltaUFrac
		add	esi, Tmap.fx_dwdx
		add	ebx, ebp 



	add	edi, 2
	add	edx, 8
	dec	Tmap.WidthModLength
	jg		NextPixel

	popf
	jnc	FPUReturn

one_more_pix:	
		// pixel 0
		cmp	esi, [edx+0]					// Compare the Z depth of this pixel with zbuffer
		jle	Skip0b								// If pixel is covered, skip drawing

		mov	[edx+0], esi					// Write z

		mov	eax, ecx							// EAX = V.VF:U.UF in 6.10:6.10
		shr	ax, 12								// EAX = V:U in 6.10:16.0
		rol	eax, 4							// EAX = V:U in 0.0:6:6
		and	eax, 0ffh						// clear upper bits
		add	eax, Tmap.pixptr		// EAX = (V*64)+U + Pixptr

		mov	al, [eax]			
		mov	ah, bh	
		and	eax, 0ffffh						// clear upper bits
		mov	al, gr_fade_table[eax]
		mov	[edi+0], al
Skip0b:
		add	ecx, Tmap.DeltaUFrac
		add	esi, Tmap.fx_dwdx
		add	ebx, ebp 


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

void tmapscan_pln8_tiled_16x16()
{
	if (gr_zbuffering) {
		switch(gr_zbuffering_mode)	{
		case GR_ZBUFF_NONE:
			break;
		case GR_ZBUFF_FULL:		// both
			tmapscan_pln8_zbuffered_tiled_16x16();
			return;
		case GR_ZBUFF_WRITE:		// write only
			tmapscan_pln8_zbuffered_tiled_16x16();
			break;
		case GR_ZBUFF_READ:		// read only
			tmapscan_pln8_zbuffered_tiled_16x16();
			return;
		}
	}

	Tmap.fx_l = fl2f(Tmap.l.b*32.0); 
	Tmap.fx_l_right = fl2f(Tmap.r.b*32.0); 
	Tmap.fx_dl_dx = fl2f(Tmap.deltas.b*32.0);

	if ( Tmap.fx_dl_dx < 0 )	{
		Tmap.fx_dl_dx = -Tmap.fx_dl_dx;
		Tmap.fx_l = (67*F1_0)-Tmap.fx_l;
		Tmap.fx_l_right = (67*F1_0)-Tmap.fx_l_right;
//		Assert( Tmap.fx_l > 31*F1_0 );
//		Assert( Tmap.fx_l < 66*F1_0 );
//		Assert( Tmap.fx_dl_dx >= 0 );
//		Assert( Tmap.fx_dl_dx < 31*F1_0 );
	}

	Tmap.fl_dudx_wide = Tmap.deltas.u*32.0f;
	Tmap.fl_dvdx_wide = Tmap.deltas.v*32.0f;
	Tmap.fl_dwdx_wide = Tmap.deltas.sw*32.0f;

	Tmap.fx_w = fl2i(Tmap.l.sw * GR_Z_RANGE)+gr_zoffset;
	Tmap.fx_dwdx = fl2i(Tmap.deltas.sw * GR_Z_RANGE);

//	Assert(Tmap.fx_w < 65536 );
//	Assert(Tmap.fx_w >= 0 );
//	Assert(Tmap.fx_w+Tmap.fx_dwdx*Tmap.loop_count < 65536 );
//	Assert(Tmap.fx_w+Tmap.fx_dwdx*Tmap.loop_count >= 0 );

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

	mov	Tmap.InnerLooper, 32/4			// Set up loop counter

	mov	edx, gr_zbuffer

	mov	eax, edi
	sub	eax, Tmap.pScreenBits
	shl	eax, 2
	add	edx, eax

// Make ESI =  DV:DU in 6:10,6:10 format
	mov	eax, Tmap.DeltaV
	shr	eax, 4
	mov	esi, Tmap.DeltaU
	shl	esi, 12
	mov	si, ax
	mov	Tmap.DeltaUFrac, esi
		
// Make ECX = V:U in 6:10,6:10 format
	mov	eax, Tmap.VFixed
	shr	eax, 4
	mov	ecx, Tmap.UFixed
	shl	ecx, 12
	mov	cx, ax


	// eax = tmp
	// ebx = light
	// ecx = V:U in 8.6:10.8
	// edx = zbuffer pointer
	// esi = z
	// edi = screen data
	// ebp = dl_dx


InnerInnerLoop:

		// pixel 0
		mov	eax, ecx							// EAX = V.VF:U.UF in 6.10:6.10
		shr	ax, 12								// EAX = V:U in 6.10:16.0
		rol	eax, 4							// EAX = V:U in 0.0:6:6
		and	eax, 0ffh						// clear upper bits
		add	eax, Tmap.pixptr		// EAX = (V*64)+U + Pixptr

		mov	al, [eax]			
		mov	ah, bh	
		and	eax, 0ffffh						// clear upper bits
		mov	al, gr_fade_table[eax]
		mov	[edi+0], al
		add	ecx, Tmap.DeltaUFrac
		add	ebx, ebp 

		// pixel 1
		mov	eax, ecx							// EAX = V.VF:U.UF in 6.10:6.10
		shr	ax, 12								// EAX = V:U in 6.10:16.0
		rol	eax, 4							// EAX = V:U in 0.0:6:6
		and	eax, 0ffh						// clear upper bits
		add	eax, Tmap.pixptr		// EAX = (V*64)+U + Pixptr

		mov	al, [eax]			
		mov	ah, bh	
		and	eax, 0ffffh						// clear upper bits
		mov	al, gr_fade_table[eax]
		mov	[edi+1], al
		add	ecx, Tmap.DeltaUFrac
		add	ebx, ebp 

		// pixel 2
		mov	eax, ecx							// EAX = V.VF:U.UF in 6.10:6.10
		shr	ax, 12								// EAX = V:U in 6.10:16.0
		rol	eax, 4							// EAX = V:U in 0.0:6:6
		and	eax, 0ffh						// clear upper bits
		add	eax, Tmap.pixptr		// EAX = (V*64)+U + Pixptr

		mov	al, [eax]			
		mov	ah, bh	
		and	eax, 0ffffh						// clear upper bits
		mov	al, gr_fade_table[eax]
		mov	[edi+2], al
		add	ecx, Tmap.DeltaUFrac
		add	ebx, ebp 

		// pixel 3
		mov	eax, ecx							// EAX = V.VF:U.UF in 6.10:6.10
		shr	ax, 12								// EAX = V:U in 6.10:16.0
		rol	eax, 4							// EAX = V:U in 0.0:6:6
		and	eax, 0ffh						// clear upper bits
		add	eax, Tmap.pixptr		// EAX = (V*64)+U + Pixptr

		mov	al, [eax]			
		mov	ah, bh	
		and	eax, 0ffffh						// clear upper bits
		mov	al, gr_fade_table[eax]
		mov	[edi+3], al
		add	ecx, Tmap.DeltaUFrac
		add	ebx, ebp 


	add	edi, 4
	dec	Tmap.InnerLooper
	jnz	InnerInnerLoop

	mov	Tmap.fx_w, esi

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

	push	ebx
	
	mov	ebx, Tmap.fx_l_right
	shr	ebx, 8
	
	sub	ebx, eax
	mov	eax, ebx
	
	mov	eax, Tmap.fx_dl_dx
	shr	eax, 8

	mov	bp, ax

	pop	ebx


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

// Make ESI =  DV:DU in 6:10,6:10 format
	mov	eax, Tmap.DeltaV
	shr	eax, 4
	mov	esi, Tmap.DeltaU
	shl	esi, 12
	mov	si, ax
	mov	Tmap.DeltaUFrac, esi
		
// Make ECX = V:U in 6:10,6:10 format
	mov	eax, Tmap.VFixed
	shr	eax, 4
	mov	ecx, Tmap.UFixed
	shl	ecx, 12
	mov	cx, ax

	mov	esi, Tmap.fx_w

	// eax = tmp
	// ebx = light
	// ecx = V:U in 8.6:10.8
	// edx = zbuffer pointer
	// esi = z
	// edi = screen data
	// ebp = dl_dx



NextPixel:
		// pixel 0
		mov	eax, ecx							// EAX = V.VF:U.UF in 6.10:6.10
		shr	ax, 12								// EAX = V:U in 6.10:16.0
		rol	eax, 4							// EAX = V:U in 0.0:6:6
		and	eax, 0ffh						// clear upper bits
		add	eax, Tmap.pixptr		// EAX = (V*64)+U + Pixptr

		mov	al, [eax]			
		mov	ah, bh	
		and	eax, 0ffffh						// clear upper bits
		mov	al, gr_fade_table[eax]
		mov	[edi+0], al
		add	ecx, Tmap.DeltaUFrac
		add	ebx, ebp 

		// pixel 1
		mov	eax, ecx							// EAX = V.VF:U.UF in 6.10:6.10
		shr	ax, 12								// EAX = V:U in 6.10:16.0
		rol	eax, 4							// EAX = V:U in 0.0:6:6
		and	eax, 0ffh						// clear upper bits
		add	eax, Tmap.pixptr		// EAX = (V*64)+U + Pixptr

		mov	al, [eax]			
		mov	ah, bh	
		and	eax, 0ffffh						// clear upper bits
		mov	al, gr_fade_table[eax]
		mov	[edi+1], al
		add	ecx, Tmap.DeltaUFrac
		add	ebx, ebp 



	add	edi, 2
	add	edx, 8
	dec	Tmap.WidthModLength
	jg		NextPixel

	popf
	jnc	FPUReturn

one_more_pix:	
		// pixel 0
		mov	eax, ecx							// EAX = V.VF:U.UF in 6.10:6.10
		shr	ax, 12								// EAX = V:U in 6.10:16.0
		rol	eax, 4							// EAX = V:U in 0.0:6:6
		and	eax, 0ffh						// clear upper bits
		add	eax, Tmap.pixptr		// EAX = (V*64)+U + Pixptr

		mov	al, [eax]			
		mov	ah, bh	
		and	eax, 0ffffh						// clear upper bits
		mov	al, gr_fade_table[eax]
		mov	[edi+0], al
		add	ecx, Tmap.DeltaUFrac
		add	ebx, ebp 


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
