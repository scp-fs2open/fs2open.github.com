/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/TmapGenericScans.cpp $
 * $Revision: 2.1 $
 * $Date: 2002-08-01 01:41:05 $
 * $Author: penguin $
 *
 * Some code for generic scanlines.  This isn't used, it is just
 * basically a dump area for inner loops I was experimenting with.
 * this entire file is #ifdef 0'd out.
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:23  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 7     4/24/97 4:45p John
 * Added tiled texture mappers for 64x64, 128x128, and 256x256 textures.
 * 
 * 6     4/24/97 3:01p John
 * added code to not crash on non-256x256 textures.
 * 
 * 5     3/14/97 3:55p John
 * Made tiled tmapper not always be zbuffered.
 * 
 * 4     3/13/97 10:32a John
 * Added code for tiled 256x256 textures in certain models.
 * 
 * 3     3/10/97 5:20p John
 * Differentiated between Gouraud and Flat shading.  Since we only do flat
 * shading as of now, we don't need to interpolate L in the outer loop.
 * This should save a few percent.
 * 
 * 2     12/10/96 10:37a John
 * Restructured texture mapper to remove some overhead from each scanline
 * setup.  This gave about a 30% improvement drawing trans01.pof, which is
 * a really complex model.  In the process, I cleaned up the scanline
 * functions and separated them into different modules for each pixel
 * depth.   
 *
 * $NoKeywords: $
 */

#include "render/3d.h"
#include "graphics/2d.h"
#include "graphics/tmapper.h"
#include "graphics/tmapscanline.h"
#include "math/floating.h"
#include "palman/palman.h"
#include "math/fix.h"

#pragma warning(disable:4410)
 


#if 0
#include "render/3d.h"
#include "graphics/2d.h"
#include "graphics/tmapper.h"
#include "graphics/tmapscanline.h"
#include "math/floating.h"
#include "palman/palman.h"
#include "math/fix.h"

#pragma warning(disable:4410)

// These must be global because I use them in assembly
// code that uses the EBP register, so the variables 
// can't be accessed off the stack.
int _fx_u, _fx_v, _fx_w, _fx_l;
int _fx_u_right, _fx_v_right, _fx_w_right;
int _fx_du, _fx_dv, _fx_dw, _fx_dl;
uint _fx_destptr,_fx_srcptr, light_table;
int V0, U0, DU1, DV1, DZ1;
int _loop_count,num_big_steps;
int num_left_over;

int rgbtable_inited = 0;
uint rgbtable1[512];
uint rgbtable2[512];
uint rgbtable3[512];

void rgbtable_init()
{
	int i,v;
	rgbtable_inited = 1;
	for (i=0; i<512; i++ )	{
		v = i - 128;
		if ( v < 0 ) v = 0;
		else if ( v > 255 ) v = 255;
		rgbtable1[i] = v;
		rgbtable2[i] = v<<8;
		rgbtable3[i] = v<<16;
	}
}


void asm_tmap_scanline_lln();
void asm_tmap_scanline_lln_tiled();

void tmapscan_lln8( int lx, int rx, int y, vertex *p, vertex *dp,  vertex * rp,uint flags )
{
	Tmap1.dest_row_data = GR_SCREEN_PTR(ubyte,lx,y);
	Tmap1.loop_count = rx - lx;
	Tmap1.pixptr = (unsigned char *)tmap_bitmap->data;
	Tmap1.bp = tmap_bitmap;
	Tmap1.src_offset = tmap_bitmap->w;

	Tmap1.fx_u = fl2f(p->u);
	Tmap1.fx_v = fl2f(p->v);
	Tmap1.fx_l = fl2f(p->l*32.0); 
	Tmap1.fx_dl_dx = fl2f(dp->l*32.0);
	Tmap1.fx_du_dx = fl2f(dp->u);
	Tmap1.fx_dv_dx = fl2f(dp->v);
	Tmap1.fx_u_right = fl2f(rp->u);
	Tmap1.fx_v_right = fl2f(rp->v);

	asm_tmap_scanline_lln();
}

extern void asm_tmap_scanline_lnt();

void tmapscan_lnt8( int lx, int rx, int y, vertex *p, vertex *dp,  vertex * rp,uint flags )
{
	Tmap1.dest_row_data = GR_SCREEN_PTR(ubyte,lx,y);
	Tmap1.loop_count = rx - lx;
	Tmap1.pixptr = (unsigned char *)tmap_bitmap->data;
	Tmap1.bp = tmap_bitmap;
	Tmap1.src_offset = tmap_bitmap->w;

	Tmap1.fx_u = fl2f(p->u);
	Tmap1.fx_v = fl2f(p->v);
	Tmap1.fx_du_dx = fl2f(dp->u);
	Tmap1.fx_dv_dx = fl2f(dp->v);
	Tmap1.fx_u_right = fl2f(rp->u);
	Tmap1.fx_v_right = fl2f(rp->v);

	asm_tmap_scanline_lnt();
}

extern void asm_tmap_scanline_lnn();

void tmapscan_lnn8( int lx, int rx, int y, vertex *p, vertex *dp,  vertex * rp,uint flags )
{
	Tmap1.dest_row_data = GR_SCREEN_PTR(ubyte,lx,y);
	Tmap1.loop_count = rx - lx;
	Tmap1.pixptr = (unsigned char *)tmap_bitmap->data;
	Tmap1.bp = tmap_bitmap;
	Tmap1.src_offset = tmap_bitmap->w;

	Tmap1.fx_u = fl2f(p->u);
	Tmap1.fx_v = fl2f(p->v);
	Tmap1.fx_du_dx = fl2f(dp->u);
	Tmap1.fx_dv_dx = fl2f(dp->v);
	Tmap1.fx_u_right = fl2f(rp->u);
	Tmap1.fx_v_right = fl2f(rp->v);

	asm_tmap_scanline_lnn();
}


void tmapscan_lln8_tiled( int lx, int rx, int y, vertex *p, vertex *dp,  vertex * rp,uint flags )
{
	Tmap1.dest_row_data = GR_SCREEN_PTR(ubyte,lx,y);
	Tmap1.loop_count = rx - lx;
	Tmap1.fx_u = fl2f(p->u);
	Tmap1.fx_v = fl2f(p->v);
	Tmap1.fx_l = fl2f(p->l*32.0); 
	Tmap1.fx_du_dx = fl2f(dp->u);
	Tmap1.fx_dv_dx = fl2f(dp->v);
	Tmap1.fx_dl_dx = fl2f(dp->l*32.0);
	Tmap1.fx_u_right = fl2f(rp->u);
	Tmap1.fx_v_right = fl2f(rp->v);
	Tmap1.pixptr = (unsigned char *)tmap_bitmap->data;
	Tmap1.bp = tmap_bitmap;
	Tmap1.src_offset = tmap_bitmap->w;

	Tmap1.BitmapWidth = tmap_bitmap->w;
	Tmap1.BitmapHeight = tmap_bitmap->h;


//	asm_tmap_scanline_lln_tiled();


}



void c_tmap_scanline_per_sub_new();

void tmapscan_pln8( int lx, int rx, int y, vertex *p, vertex *dp,  vertex * rp,uint flags )
{
	Tmap1.dest_row_data = GR_SCREEN_PTR(ubyte,lx,y);
	Tmap1.loop_count = rx - lx;
	Tmap1.fx_l = fl2f(p->l*32.0); 
	Tmap1.fx_dl_dx = fl2f(dp->l*32.0);

   Tmap1.UOverZ = p->u;
	Tmap1.VOverZ = p->v;
	Tmap1.OneOverZ = p->sw;

	Tmap1.dUOverZdX8 = dp->u*32.0f;
	Tmap1.dVOverZdX8 = dp->v*32.0f;
	Tmap1.dOneOverZdX8 = dp->sw*32.0f;

	Tmap1.dUOverZdX = dp->u;
	Tmap1.dVOverZdX = dp->v;
	Tmap1.dOneOverZdX = dp->sw;

   Tmap1.RightUOverZ = rp->u;
	Tmap1.RightVOverZ = rp->v;
	Tmap1.RightOneOverZ = rp->sw;

	if ( Tmap1.fx_dl_dx < 0 )	{
		Tmap1.fx_dl_dx = -Tmap1.fx_dl_dx;
		Tmap1.fx_l = (67*F1_0)-Tmap1.fx_l;
		Tmap1.fx_l_right = (67*F1_0)-Tmap1.fx_l_right;
//		return;
//		Assert( Tmap1.fx_l > 31*F1_0 );
//		Assert( Tmap1.fx_l < 66*F1_0 );
//		Assert( Tmap1.fx_dl_dx >= 0 );
//		Assert( Tmap1.fx_dl_dx < 31*F1_0 );
	}

//	return;


	if (0) {
			ubyte *dest, c;
			int x;
			fix l, dldx;

			l = Tmap1.fx_l;
			dldx = Tmap1.fx_dl_dx;
			dest = Tmap1.dest_row_data;

			for (x=Tmap1.loop_count; x >= 0; x-- ) {
				//*dest++ = gr_fade_table[ ((l>>8)&(0xff00)) + 35 ];
				c = *dest;
				*dest++ = c+1;
				l += dldx;
			}
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


	// put the FPU in 32 bit mode
	// @todo move this out of here!

	fstcw		Tmap1.OldFPUCW					// store copy of CW
	mov		ax,Tmap1.OldFPUCW				// get it in ax
	and		eax, ~0x300L
	mov		Tmap1.FPUCW,ax					// store it
	fldcw		Tmap1.FPUCW						// load the FPU

	mov		ecx, Tmap1.loop_count		// ecx = width
	inc		ecx
	mov		edi, Tmap1.dest_row_data	// edi = dest pointer

	// edi = pointer to start pixel in dest dib
	// ecx = spanwidth

	mov		eax,ecx							// eax and ecx = width
	shr		ecx,5								// ecx = width / subdivision length
	and		eax,31								// eax = width mod subdivision length
	jnz		some_left_over					// any leftover?
//	jmp		Return
	dec		ecx								// no, so special case last span
	mov		eax,32								// it's 8 pixels long
some_left_over:
	mov		Tmap1.Subdivisions,ecx		// store widths
	mov		Tmap1.WidthModLength,eax
    
//    mov     ebx,pLeft                   ; get left edge pointer
//    mov     edx,pGradients              ; get gradients pointer

	// calculate ULeft and VLeft			// FPU Stack (ZL = ZLeft)
													// st0  st1  st2  st3  st4  st5  st6  st7
	fld		Tmap1.VOverZ					// V/ZL 
	fld		Tmap1.UOverZ					// U/ZL V/ZL 
	fld		Tmap1.OneOverZ					// 1/ZL U/ZL V/ZL 
	fld1											// 1    1/ZL U/ZL V/ZL 
	fdiv		st,st(1)							// ZL   1/ZL U/ZL V/ZL 
	fld		st									// ZL   ZL   1/ZL U/ZL V/ZL 
	fmul		st,st(4)							// VL   ZL   1/ZL U/ZL V/ZL 
	fxch		st(1)								// ZL   VL   1/ZL U/ZL V/ZL 
	fmul		st,st(3)							// UL   VL   1/ZL U/ZL V/ZL 

	fstp		st(5)								// VL   1/ZL U/ZL V/ZL UL
	fstp		st(5)								// 1/ZL U/ZL V/ZL UL   VL

	// calculate right side OverZ terms  ; st0  st1  st2  st3  st4  st5  st6  st7

	fadd		Tmap1.dOneOverZdX8			// 1/ZR U/ZL V/ZL UL   VL
	fxch		st(1)								// U/ZL 1/ZR V/ZL UL   VL
	fadd		Tmap1.dUOverZdX8				// U/ZR 1/ZR V/ZL UL   VL
	fxch		st(2)								// V/ZL 1/ZR U/ZR UL   VL
	fadd		Tmap1.dVOverZdX8				// V/ZR 1/ZR U/ZR UL   VL

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
	fmul    Tmap1.FixedScale            ; UL16 UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fistp   Tmap1.UFixed                ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

	fld     st(6)                       ; VL   UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fmul    Tmap1.FixedScale            ; VL16 UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fistp   Tmap1.VFixed                ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

	// calculate deltas                  ; st0  st1  st2  st3  st4  st5  st6  st7

	fsubr   st(5),st                    ; UR   VR   V/ZR 1/ZR U/ZR dU   VL
	fxch    st(1)                       ; VR   UR   V/ZR 1/ZR U/ZR dU   VL
	fsubr   st(6),st                    ; VR   UR   V/ZR 1/ZR U/ZR dU   dV
	fxch    st(6)                       ; dV   UR   V/ZR 1/ZR U/ZR dU   VR

	fmul    Tmap1.FixedScale8           ; dV8  UR   V/ZR 1/ZR U/ZR dU   VR
	fistp   Tmap1.DeltaV                ; UR   V/ZR 1/ZR U/ZR dU   VR

	fxch    st(4)                       ; dU   V/ZR 1/ZR U/ZR UR   VR
	fmul    Tmap1.FixedScale8           ; dU8  V/ZR 1/ZR U/ZR UR   VR
	fistp   Tmap1.DeltaU                ; V/ZR 1/ZR U/ZR UR   VR

	// increment terms for next span     ; st0  st1  st2  st3  st4  st5  st6  st7
	// Right terms become Left terms---->; V/ZL 1/ZL U/ZL UL   VL

	fadd    Tmap1.dVOverZdX8            ; V/ZR 1/ZL U/ZL UL   VL
	fxch    st(1)                       ; 1/ZL V/ZR U/ZL UL   VL
	fadd    Tmap1.dOneOverZdX8          ; 1/ZR V/ZR U/ZL UL   VL
	fxch    st(2)                       ; U/ZL V/ZR 1/ZR UL   VL
	fadd    Tmap1.dUOverZdX8            ; U/ZR V/ZR 1/ZR UL   VL
	fxch    st(2)                       ; 1/ZR V/ZR U/ZR UL   VL
	fxch    st(1)                       ; V/ZR 1/ZR U/ZR UL   VL


    ; set up affine registers

    ; setup delta values
    
    mov     eax,Tmap1.DeltaV                ; get v 16.16 step
    mov     ebx,eax                     ; copy it
    sar     eax,16                      ; get v int step
    shl     ebx,16                      ; get v frac step
    mov     Tmap1.DeltaVFrac,ebx            ; store it
    imul    eax,Tmap1.src_offset      ; calculate texture step for v int step

    mov     ebx,Tmap1.DeltaU                ; get u 16.16 step
    mov     ecx,ebx                     ; copy it
    sar     ebx,16                      ; get u int step
    shl     ecx,16                      ; get u frac step
    mov     Tmap1.DeltaUFrac,ecx            ; store it
    add     eax,ebx                     ; calculate uint + vint step
    mov     Tmap1.UVintVfracStepVNoCarry,eax; save whole step in non-v-carry slot
    add     eax,Tmap1.src_offset      ; calculate whole step + v carry
    mov     Tmap1.UVintVfracStepVCarry,eax  ; save in v-carry slot

; setup initial coordinates
    mov     esi,Tmap1.UFixed                ; get u 16.16 fixedpoint coordinate
   
    mov     ebx,esi                     ; copy it
    sar     esi,16                      ; get integer part
    shl     ebx,16                      ; get fractional part
    
    mov     ecx,Tmap1.VFixed                ; get v 16.16 fixedpoint coordinate
   
    mov     edx,ecx                     ; copy it
    sar     edx,16                      ; get integer part
    shl     ecx,16                      ; get fractional part
    imul    edx,Tmap1.src_offset      ; calc texture scanline address
    add     esi,edx                     ; calc texture offset
    add     esi,Tmap1.pixptr          ; calc address

	mov     edx,Tmap1.DeltaUFrac            ; get register copy

 	mov	eax, Tmap1.fx_l
	shr	eax, 8
	mov	bx, ax

	mov	ebp, Tmap1.fx_dl_dx
	shl	ebp, 5	//*32
	add	Tmap1.fx_l, ebp

	mov	ebp, Tmap1.fx_l
	shr	ebp, 8
	sub	bp, ax
	shr	bp, 5

	mov	dx, bp


	; calculate right side coords       ; st0  st1  st2  st3  st4  st5  st6  st7

	fld1                                ; 1    V/ZR 1/ZR U/ZR UL   VL
	fdiv    st,st(2)                    ; ZR   V/ZR 1/ZR U/ZR UL   VL


    // 8 pixel span code
    // edi = dest dib bits at current pixel
    // esi = texture pointer at current u,v
    // eax = scratch
    // ebx = u fraction 0.32
    // ecx = v fraction 0.32
    // edx = u frac step
    // ebp = v carry scratch

    mov     al,[edi]                    // preread the destination cache line


    mov     al,[esi]                    // get texture pixel 0
    mov		ah, bh
    mov		al, gr_fade_table[eax]

    add     ecx,Tmap1.DeltaVFrac            // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
//	mov al, 0	// Uncomment this line to show divisions
    mov     [edi+0],al                  // store pixel 0

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 1
    mov		ah, bh
    mov		al, gr_fade_table[eax]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+1],al                  // store pixel 1

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 2
    mov		ah, bh
    mov		al, gr_fade_table[eax]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+2],al                  // store pixel 2

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 3
    mov		ah, bh
    mov		al, gr_fade_table[eax]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+3],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+4],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+5],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+6],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+7],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+8],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+9],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+10],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+11],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+12],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+13],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+14],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+15],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+16],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+17],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+18],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+19],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+20],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+21],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+22],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+23],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+24],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+25],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction



    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+26],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+27],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+28],al                  // store pixel 4

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 5
    mov		ah, bh
    mov		al, gr_fade_table[eax]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+29],al                  // store pixel 5

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 6
    mov		ah, bh
    mov		al, gr_fade_table[eax]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+30],al                  // store pixel 6

    add     ebx,edx                     // increment u fraction

    mov     al,[esi]                    // get texture pixel 7
    mov		ah, bh
    mov		al, gr_fade_table[eax]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries

    mov     [edi+31],al                 // store pixel 7


    
    ; ************** Okay to Access Stack Frame ****************
    ; ************** Okay to Access Stack Frame ****************
    ; ************** Okay to Access Stack Frame ****************


    ; the fdiv is done, finish right    ; st0  st1  st2  st3  st4  st5  st6  st7
                                        ; ZR   V/ZR 1/ZR U/ZR UL   VL

    fld     st                          ; ZR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(2)                    ; VR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fxch    st(1)                       ; ZR   VR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(4)                    ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

    add     edi,32                       ; increment to next span
    dec     Tmap1.Subdivisions              ; decrement span count
    jnz     SpanLoop                    ; loop back

	// save new lighting values
//	xor	eax, eax
//	mov	ax, bx
//	mov	Tmap1.fx_l, eax

//	xor	eax, eax
//	mov	ax, dx
//	mov	Tmap1.fx_dl_dx, eax

HandleLeftoverPixels:
//	jmp	FPUReturn

    mov     esi,Tmap1.pixptr          ; load texture pointer

    ; edi = dest dib bits
    ; esi = current texture dib bits
    ; at this point the FPU contains    ; st0  st1  st2  st3  st4  st5  st6  st7
    ; inv. means invalid numbers        ; inv. inv. inv. inv. inv. UL   VL

    cmp     Tmap1.WidthModLength,0          ; are there remaining pixels to draw?
    jz      FPUReturn                   ; nope, pop the FPU and bail

    ; convert left side coords          ; st0  st1  st2  st3  st4  st5  st6  st7

    fld     st(5)                       ; UL   inv. inv. inv. inv. inv. UL   VL
    fmul    Tmap1.FixedScale                ; UL16 inv. inv. inv. inv. inv. UL   VL
    fistp   Tmap1.UFixed                    ; inv. inv. inv. inv. inv. UL   VL

    fld     st(6)                       ; VL   inv. inv. inv. inv. inv. UL   VL
    fmul    Tmap1.FixedScale                // VL16 inv. inv. inv. inv. inv. UL   VL
    fistp   Tmap1.VFixed                    ; inv. inv. inv. inv. inv. UL   VL

    dec     Tmap1.WidthModLength            ; calc how many steps to take
    jz      OnePixelSpan                ; just one, don't do deltas

    ; calculate right edge coordinates  ; st0  st1  st2  st3  st4  st5  st6  st7
    ; r -> R+1

    ; @todo rearrange things so we don't need these two instructions
    fstp    Tmap1.FloatTemp                 ; inv. inv. inv. inv. UL   VL
    fstp    Tmap1.FloatTemp                 ; inv. inv. inv. UL   VL

    fld     Tmap1.RightVOverZ           ; V/Zr inv. inv. inv. UL   VL
    fsub    Tmap1.dVOverZdX             ; V/ZR inv. inv. inv. UL   VL
    fld     Tmap1.RightUOverZ           ; U/Zr V/ZR inv. inv. inv. UL   VL
    fsub    Tmap1.dUOverZdX             ; U/ZR V/ZR inv. inv. inv. UL   VL
    fld     Tmap1.RightOneOverZ              ; 1/Zr U/ZR V/ZR inv. inv. inv. UL   VL
    fsub    Tmap1.dOneOverZdX           ; 1/ZR U/ZR V/ZR inv. inv. inv. UL   VL

    fdivr   Tmap1.One                       ; ZR   U/ZR V/ZR inv. inv. inv. UL   VL

    fmul    st(1),st                    ; ZR   UR   V/ZR inv. inv. inv. UL   VL
    fmulp   st(2),st                    ; UR   VR   inv. inv. inv. UL   VL

    ; calculate deltas                  ; st0  st1  st2  st3  st4  st5  st6  st7

    fsubr   st(5),st                    ; UR   VR   inv. inv. inv. dU   VL
    fxch    st(1)                       ; VR   UR   inv. inv. inv. dU   VL
    fsubr   st(6),st                    ; VR   UR   inv. inv. inv. dU   dV
    fxch    st(6)                       ; dV   UR   inv. inv. inv. dU   VR

    fidiv   Tmap1.WidthModLength            ; dv   UR   inv. inv. inv. dU   VR
    fmul    Tmap1.FixedScale                ; dv16 UR   inv. inv. inv. dU   VR
    fistp   Tmap1.DeltaV                    ; UR   inv. inv. inv. dU   VR

    fxch    st(4)                       ; dU   inv. inv. inv. UR   VR
    fidiv   Tmap1.WidthModLength            ; du   inv. inv. inv. UR   VR
    fmul    Tmap1.FixedScale                ; du16 inv. inv. inv. UR   VR
    fistp   Tmap1.DeltaU                    ; inv. inv. inv. UR   VR

    ; @todo gross!  these are to line up with the other loop
    fld     st(1)                       ; inv. inv. inv. inv. UR   VR
    fld     st(2)                       ; inv. inv. inv. inv. inv. UR   VR

//jmp OldWay


	; setup delta values
	mov	eax, Tmap1.DeltaV	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap1.DeltaVFrac, ebx	// store it
	imul	eax, Tmap1.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap1.DeltaU			// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap1.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step
	mov	Tmap1.UVintVfracStepVNoCarry, eax	// save whole step in non-v-carry slot
	add	eax, Tmap1.src_offset				// calc whole step + v carry
	mov	Tmap1.UVintVfracStepVCarry, eax	// save in v-carry slot



OnePixelSpan:

/*
; check coordinate ranges
	mov	eax, Tmap1.UFixed
	cmp	eax, Tmap1.MinUFixed
	jge	UNotTooSmall_2
	mov	eax, Tmap1.MinUFixed
	mov	Tmap1.UFixed, eax
	jmp	CheckV_2
UNotTooSmall_2:	
	cmp	eax, Tmap1.MaxUFixed
	jle	CheckV_2
	mov	eax, Tmap1.MaxUFixed
	mov	Tmap1.UFixed, eax
CheckV_2:
	mov	eax, Tmap1.VFixed
	cmp	eax, Tmap1.MinVFixed
	jge	VNotTooSmall_2
	mov	eax, Tmap1.MinVFixed
	mov	Tmap1.VFixed, eax
	jmp	DoneCheck_2
VNotTooSmall_2:	
	cmp	eax, Tmap1.MaxVFixed
	jle	DoneCheck_2
	mov	eax, Tmap1.MaxVFixed
	mov	Tmap1.VFixed, eax
DoneCheck_2:
*/




	; setup initial coordinates
	mov	esi, Tmap1.UFixed			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap1.VFixed			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap1.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap1.pixptr			// calc address
	
	; set edi = address of first pixel to modify
;	mov	edi, Tmap1.dest_row_data




	mov	eax, Tmap1.fx_l
	shr	eax, 8
	mov	bx, ax

	mov	edx, Tmap1.DeltaUFrac

	cmp	Tmap1.WidthModLength, 1
	jle	NoDeltaLight

	push	ebx
	
	mov	ebx, Tmap1.fx_l_right
	shr	ebx, 8
	
	sub	ebx, eax
	mov	eax, ebx
	
#if 0
	// slow but maybe better
	push	edx
	cdq
	mov	ebx, Tmap1.WidthModLength 
	dec	ebx
	idiv	ebx
	pop	edx
#else
	mov	eax, Tmap1.fx_dl_dx
	shr	eax, 8
#endif

	mov	dx, ax

	pop	ebx

NoDeltaLight:

	inc	Tmap1.WidthModLength
	mov	eax,Tmap1.WidthModLength
	shr	eax, 1
	jz		one_more_pix
	pushf
	mov	Tmap1.WidthModLength, eax

	xor	eax, eax

    mov     al,[edi]                    // preread the destination cache line

NextPixel:
    mov     al,[esi]                    // get texture pixel 0
    mov		ah, bh
    mov		al, gr_fade_table[eax]

    add     ecx,Tmap1.DeltaVFrac            // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    mov     [edi+0],al                  // store pixel 0

    add     ecx,Tmap1.DeltaVFrac        // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 1
    mov		ah, bh
    mov		al, gr_fade_table[eax]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    mov     [edi+1],al                  // store pixel 1

	add	edi, 2
	dec	Tmap1.WidthModLength
	jg		NextPixel

	popf
	jnc	FPUReturn

one_more_pix:	

    mov     al,[esi]                    // get texture pixel 2
    mov		ah, bh
    mov		al, gr_fade_table[eax]
    mov     [edi],al                  // store pixel 2













/* 
OldWay:		// This is 6% slower than above

    mov     ebx,Tmap1.UFixed                ; get starting coordinates
    mov     ecx,Tmap1.VFixed                ; for span
 
    ; leftover pixels loop
    ; edi = dest dib bits
    ; esi = texture dib bits

    ; ebx = u 16.16
    ; ecx = v 16.16


    mov     eax,ecx                     ; copy v
    sar     eax,16                      ; int(v)
    imul    eax,Tmap1.src_offset      ; scan offset
    mov     edx,ebx                     ; copy u
    sar     edx,16                      ; int(u)
    add     eax,edx                     ; texture offset
    mov     al,[esi+eax]                ; get source pixel
mov al, 0
    mov     [edi],al                    ; store it
    inc     edi
    add     ebx,Tmap1.DeltaU                  ; increment u coordinate
    add     ecx,Tmap1.DeltaV                  ; increment v coordinate

    dec     Tmap1.WidthModLength            ; decrement loop count
    jl     FPUReturn                ; finish up
	 

LeftoverLoop:
    mov     eax,ecx                     ; copy v
    sar     eax,16                      ; int(v)
    imul    eax,Tmap1.src_offset      ; scan offset
    mov     edx,ebx                     ; copy u
    sar     edx,16                      ; int(u)
    add     eax,edx                     ; texture offset
    mov     al,[esi+eax]                ; get source pixel
    mov     [edi],al                    ; store it
    inc     edi
    add     ebx,Tmap1.DeltaU                  ; increment u coordinate
    add     ecx,Tmap1.DeltaV                  ; increment v coordinate

    dec     Tmap1.WidthModLength            ; decrement loop count
    jge     LeftoverLoop                ; finish up
*/

FPUReturn:

    ; busy FPU registers:               ; st0  st1  st2  st3  st4  st5  st6  st7
                                        ; xxx  xxx  xxx  xxx  xxx  xxx  xxx
    ffree   st(0)
    ffree   st(1)
    ffree   st(2)
    ffree   st(3)
    ffree   st(4)
    ffree   st(5)
    ffree   st(6)

    fldcw   Tmap1.OldFPUCW                  // restore the FPU

	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	pop	edx
	pop	ecx
	pop	eax
	}


}


void tmapscan_lln8_old( int lx, int rx, int y, vertex *p, vertex *dp,  vertex * rp,uint flags )
{
	_fx_srcptr = (uint)tmap_bitmap->data;
	_fx_destptr = (uint)GR_SCREEN_PTR(ubyte,lx,y);
	_loop_count = rx - lx;
	_fx_u = fl2f(p->u*64.0f);
	_fx_v = fl2f(p->v*64.0f);
	_fx_l = fl2f(p->l*32.0+1.0);
	_fx_du = fl2f(dp->u*64.0f);
	_fx_dv = fl2f(dp->v*64.0f);
	_fx_dl = fl2f(dp->l*32.0);
	light_table = (uint)&gr_fade_table[0];

	_asm {
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi
	
	; set edi = address of first pixel to modify
	mov	edi, _fx_destptr


	mov	eax, _fx_v
	shr	eax, 6
	mov	edx, _fx_u
	shl	edx, 10
	mov	dx, ax		; EDX=U:V in 6.10 format

	mov	eax, _fx_dv
	shr	eax, 6
	mov	esi, _fx_du
	shl	esi, 10
	mov	si, ax		; ESI=DU:DV in 6.10 format

	mov	ebx, _fx_l
	sar	ebx, 8
	mov	ebp, _fx_dl
	sar	ebp, 8

	mov	ecx, _fx_srcptr

	mov	eax, _loop_count
	inc	eax
	mov	_loop_count, eax

	shr	eax, 3
	je		DoLeftOverPixels

	mov	num_big_steps, eax
	and	_loop_count, 7

NextPixelBlock:
		; pixel 0
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+0], al

		; pixel 1
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+1], al

		; pixel 2
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+2], al

		; pixel 3
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+3], al

		; pixel 4
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+4], al

		; pixel 5
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+5], al

		; pixel 6
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+6], al

		; pixel 7
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+7], al

	add	edi, 8
	dec	num_big_steps
	jne	NextPixelBlock
	

DoLeftOverPixels:
	mov	eax,_loop_count
	test	eax, -1
	jz	_none_to_do
	shr	eax, 1
	je	one_more_pix
	mov	_loop_count, eax
	pushf


NextPixel:
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+0], al

		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+1], al


	add	edi, 2
	dec	_loop_count
	jne	NextPixel

	popf
	jnc	_none_to_do

one_more_pix:	
	mov	eax, edx	
	shr	ax, 10
	rol	eax, 6
	and	eax, 0ffffh
	mov	al, [ecx+eax]			
	mov	ah, bh	    	
	mov	al, gr_fade_table[eax]
	mov	[edi], al

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


void tmapscan_flat16( int lx, int rx, int y, vertex *p, vertex *dp,  vertex * rp,uint flags )
{
	int i;
	ushort *pDestBits;

	pDestBits = GR_SCREEN_PTR(ushort,lx,y);
	
	for (i=0; i<(rx-lx+1); i++ )
		*pDestBits++ = gr_screen.current_color.raw16;
}

float tmap_max_z = 0.0f;

void tmapscan_lln8_z( int lx, int rx, int y, vertex *p, vertex *dp,  vertex * rp,uint flags )
{
	int count;
	ubyte *pDestBits, tmp;
	float u, dudx, v, dvdx, l, dldx;
	float z, dzdx;

	pDestBits = GR_SCREEN_PTR(ubyte,lx,y);

	ubyte * cdata = (ubyte *)tmap_bitmap->data;

	u = p->u;
	v = p->v;
	l = p->l*32.0f;
	z = p->nz;
	dudx = dp->u;
	dvdx = dp->v;
	dldx = dp->l*32.0f;
	dzdx = dp->nz;

	for ( count = rx - lx + 1 ; count > 0; count-- )	{
		if ( z < tmap_max_z )	{
			tmp = cdata[fl2i(v)*tmap_bitmap->w+fl2i(u)];
			*pDestBits = gr_fade_table[ fl2i(l)*256+tmp ];
		}
		pDestBits++;
		u += dudx;
		v += dvdx;
		l += dldx;
		z += dzdx;
	}
}


void tmapscan_generic8( int lx, int rx, int y, vertex *p, vertex *dp,  vertex * rp,uint flags )
{
	int count;
	ubyte *pDestBits, tmp;
	int u, dudx, v, dvdx, w, dwdx, l, dldx;

	pDestBits = GR_SCREEN_PTR(ubyte,lx,y);

	if ( Tmap1.flags & TMAP_FLAG_TEXTURED )	{
		ubyte * cdata = (ubyte *)tmap_bitmap->data;
		if ( flags & TMAP_FLAG_RAMP ) {
			if ( Tmap1.flags & TMAP_FLAG_CORRECT )	{
				float fu, fv, fw, fdu, fdv, fdw;

				tmapscan_pln8( lx, rx, y, p, dp,  rp,Tmap1.flags );
				return;


				fu = p->u;
				fv = p->v;
				fw = p->sw;
				l = fl2f(p->l*32.0f);
				
				fdu = dp->u;
				fdv = dp->v;
				fdw = dp->sw;
				dldx = fl2f(dp->l*32.0f);
						
				for ( count = rx - lx + 1 ; count > 0; count-- )	{
					u = fl2i(fu/fw);
					v = fl2i(fv/fw);
					tmp = cdata[v*tmap_bitmap->w+u];
					*pDestBits++ = tmp; //gr_fade_table[ (l>>16)*256+tmp ];
					//tmp = *pDestBits;
					//*pDestBits++ = tmp+1;
					fu += fdu;
					fv += fdv;
					fw += fdw;
					l += dldx;
				}

			} else {
#if 1
				tmapscan_lln8( lx, rx, y, p, dp, rp, flags );
#else
				u = fl2f(p->u*64.0f);
				v = fl2f(p->v*64.0f);
				l = fl2f(p->l*32.0f);
				dudx = fl2f(dp->u*64.0f);
				dvdx = fl2f(dp->v*64.0f);
				dldx = fl2f(dp->l*32.0f);

				for ( count = rx - lx + 1 ; count > 0; count-- )	{
					
					//tmp = cdata[((v>>16)&63)*64+((u>>16)&63)];
					//*pDestBits++ = ;//gr_fade_table[ (l>>16)*256+tmp ];
					(*pDestBits)++;
					pDestBits++;
					u += dudx;
					v += dvdx;
					l += dldx;
				}
#endif
			}
		} else {
			if ( flags & TMAP_FLAG_CORRECT )	{
				u = fl2f(p->u*64.0f);
				v = fl2f(p->v*64.0f);
				w = fl2f(p->sw*16.0f);

				dudx = fl2f(dp->u*64.0f);
				dvdx = fl2f(dp->v*64.0f);
				dwdx = fl2f(dp->sw*16.0f);
						
				for ( count = rx - lx + 1 ; count > 0; count-- )	{
					tmp = cdata[((v/w)&63)*64+((u/w)&63)];
					*pDestBits++ = tmp;
					u += dudx;
					v += dvdx;
					w += dwdx;
				}
			} else {
				u = fl2f(p->u*64.0f);
				v = fl2f(p->v*64.0f);
				dudx = fl2f(dp->u*64.0f);
				dvdx = fl2f(dp->v*64.0f);

				for ( count = rx - lx + 1 ; count > 0; count-- )	{
					tmp = cdata[((v>>16)&63)*64+((u>>16)&63)];
					*pDestBits++ = tmp;
					u += dudx;
					v += dvdx;
				}
			}
		}
	} else {
		if ( Tmap1.flags & TMAP_FLAG_RAMP ) {
				l = fl2f(p->l*32.0f);
				dldx = fl2f(dp->l*32.0f);
						
				for ( count = rx - lx + 1 ; count > 0; count-- )	{
					*pDestBits++ = gr_fade_table[ (l>>16)*256+gr_screen.current_color.raw8 ];
					l += dldx;
				}
		} else {
			memset( pDestBits, gr_screen.current_color.raw8, (rx-lx+1) );
		}
	}
}

uint testpixel;
uint fsave_area[64];

unsigned __int64 packrgb( int r, int g, int b )
{
	unsigned __int64 tmp;
	unsigned int *tmps;

	tmp = 0;

	tmps = (unsigned int *)&r;
	tmp |= *tmps & 0xFFFF;
	tmp <<= 16;

	tmps = (unsigned int *)&g;
	tmp |= *tmps & 0xFFFF;
	tmp <<= 16;

	tmps = (unsigned int *)&b;
	tmp |= *tmps & 0xFFFF;

	return tmp;
}



void tmapscan_generic( int lx, int rx, int y, vertex *p, vertex *dp,  vertex * rp,uint flags )
{
	int count;
	uint *pDestBits, tmp, tmp1;
	int u, dudx, v, dvdx, w, dwdx;
	int r, g, b, dr, dg, db;

	if ( !rgbtable_inited )
		rgbtable_init();

	pDestBits = GR_SCREEN_PTR(uint,lx,y);

	if ( Tmap1.flags & TMAP_FLAG_TEXTURED )	{
		uint * cdata = (uint *)tmap_bitmap->data;

		if ( Tmap1.flags & TMAP_FLAG_GOURAUD ) {
			if ( Tmap1.flags & TMAP_FLAG_CORRECT )	{
				u = fl2f(p->u*64.0f);
				v = fl2f(p->v*64.0f);
				w = fl2f(p->sw);

				r = fl2f(p->r*255.0f);
				g = fl2f(p->g*255.0f);
				b = fl2f(p->b*255.0f);

				dr = fl2f(dp->r*255.0f);
				dg = fl2f(dp->g*255.0f);
				db = fl2f(dp->b*255.0f);

				dudx = fl2f(dp->u*64.0f);
				dvdx = fl2f(dp->v*64.0f);
				dwdx = fl2f(dp->sw);
					
				for ( count = rx - lx + 1 ; count > 0; count-- )	{
					tmp = cdata[((v/w)&63)*64+((u/w)&63)];
					tmp1 = rgbtable1[ (tmp & 0xFF)+ (b>>16) ];
					tmp1 |= rgbtable2[ ((tmp>>8) & 0xFF)+ (g>>16) ];
					tmp1 |= rgbtable3[ ((tmp>>16) & 0xFF)+ (r>>16) ];
					*pDestBits++ = tmp1;
					u += dudx;
					v += dvdx;
					w += dwdx;
					r += dr;
					g += dg;
					b += db;
				}
			} else {
				// MMX!!!
				__int64 light, deltalight;

				u = fl2f(p->u*64.0f);
				v = fl2f(p->v*64.0f);
				dudx = fl2f(dp->u*64.0f);
				dvdx = fl2f(dp->v*64.0f);

#if 0
				r = fl2f(p->r*255.0f)>>8;
				g = fl2f(p->g*255.0f)>>8;
				b = fl2f(p->b*255.0f)>>8;

				dr = fl2f(dp->r*255.0f)>>8;
				dg = fl2f(dp->g*255.0f)>>8;
				db = fl2f(dp->b*255.0f)>>8;
#else
				r = fl2f(p->r)>>7;
				g = fl2f(p->g)>>7;
				b = fl2f(p->b)>>7;

				dr = fl2f(dp->r)>>7;
				dg = fl2f(dp->g)>>7;
				db = fl2f(dp->b)>>7;

				//r = 256*2;
				//g = 256*2;
				//b = 256*2;
				//dr = dg = db = 0;
#endif
				
				light = packrgb( r, g, b );
				deltalight = packrgb( dr, dg, db );

				_asm fstenv fsave_area
				_asm movq	mm3, light
				_asm movq	mm4, deltalight
				_asm pxor	mm2, mm2			; mm0 = 0

				for ( count = rx - lx + 1 ; count > 0; count-- )	{
					testpixel = cdata[((v>>16)&63)*64+((u>>16)&63)];

					_asm punpcklbw	mm2, testpixel	; mm0 = 8.8,8.8, 8.8 rgb
					_asm pmulhw		mm2, mm3			;
					_asm paddsw		mm3, mm4			; light += deltalight
					_asm packuswb	mm2, mm2			;mm2 is who cares
					_asm movd		testpixel, mm2	; load tmp
					_asm pxor		mm2, mm2			; mm0 = 0

					*pDestBits++ = testpixel;
					u += dudx;
					v += dvdx;
				}
				_asm emms
				_asm frstor fsave_area
			}
		} else {
			if ( Tmap1.flags & TMAP_FLAG_CORRECT )	{
				u = fl2f(p->u*64.0f);
				v = fl2f(p->v*64.0f);
				w = fl2f(p->sw);
				dudx = fl2f(dp->u*64.0f);
				dvdx = fl2f(dp->v*64.0f);
				dwdx = fl2f(dp->sw);
					
				for ( count = rx - lx + 1 ; count > 0; count-- )	{
					*pDestBits++ = cdata[((v/w)&63)*64+((u/w)&63)];
					u += dudx;
					v += dvdx;
					w += dwdx;
				}
			} else {
				u = fl2f(p->u*64.0f);
				v = fl2f(p->v*64.0f);
				dudx = fl2f(dp->u*64.0f);
				dvdx = fl2f(dp->v*64.0f);
				
				for ( count = rx - lx + 1 ; count > 0; count-- )	{
					*pDestBits++ = cdata[((v>>16)&63)*64+((u>>16)&63)];
					u += dudx;
					v += dvdx;
				}
			}
		}
	} else if ( Tmap1.flags & TMAP_FLAG_GOURAUD ) {

		r = fl2f(p->r*255.0f);
		g = fl2f(p->g*255.0f);
		b = fl2f(p->b*255.0f);

		dr = fl2f(dp->r*255.0f);
		dg = fl2f(dp->g*255.0f);
		db = fl2f(dp->b*255.0f);
		
		for ( count = rx - lx + 1 ; count > 0; count-- )	{
			*pDestBits++ = (r&0xFF0000)|((g>>8)&0xFF00)|(b>>16);
			r += dr;
			g += dg;
			b += db;
			//*pDestBits++ = 100;
		}
	} else {
		memset( pDestBits, gr_screen.current_color.raw32, (rx-lx+1)*4 );
	}
}

void tmapscan_flat( int lx, int rx, int y, vertex *p, vertex *dp,  vertex * rp,uint flags )
{
	int w;
	uint *pDestBits;
	
	pDestBits = GR_SCREEN_PTR(uint,lx,y);
	w = (rx-lx+1);
#ifdef USE_INLINE_ASM
	_asm	mov	eax, gr_screen.current_color.raw32
	_asm	mov	ecx, w
	_asm	mov	edi, pDestBits
	_asm	cld
	_asm	rep	stosd
#else
	for (i=0; i<w; i++ )	{
		*pDestBits++ = gr_screen.current_color.raw32;
	}
#endif
}

float zbuffer[640*480];

void zbuffer_clear()
{
	int i;
	for (i=0; i<640*480; i++ )
		zbuffer[i] = 10000.0f;
}

void tmapscan_flat_z( int lx, int rx, int y, vertex *p, vertex *dp,  vertex * rp,uint flags )
{
	int w;
	uint *pDestBits;
	float * tz;
	float z, dz;

	tz = &zbuffer[y*640+lx];
	pDestBits = GR_SCREEN_PTR(uint,lx,y);
	w = (rx-lx+1);
	z = p->z;
	dz = dp->z;
//#ifdef USE_INLINE_ASM
#if 0
	_asm	mov	eax, gr_screen.current_color.raw32
	_asm	mov	ecx, w
	_asm	mov	edi, pDestBits
	_asm	cld
	_asm	rep	stosd
#else
	{ int i;
		for (i=0; i<w; i++ )	{
			if ( z < *tz )	{
				*tz = z;
				*pDestBits = gr_screen.current_color.raw32;
			}
			pDestBits++;
			tz++;
			z += dz;
		}
	}
#endif
}



#define NBITS 4

uint fsave_area1[64];

void tmapscan_pln( int lx, int rx, int y, vertex *p, vertex *dp,  vertex * rp,uint flags )
{
	__int64 light, deltalight;
	int r, g, b, dr, dg, db;
	_fx_srcptr = (uint)tmap_bitmap->data;
	_fx_destptr = (uint)GR_SCREEN_PTR(uint,lx,y);
	_loop_count = rx - lx;
	_fx_u = fl2f(p->u*64.0f);
	_fx_v = fl2f(p->v*64.0f);
	_fx_w = fl2f(p->sw*16.0);
	_fx_du = fl2f(dp->u*64.0f);
	_fx_dv = fl2f(dp->v*64.0f);
	_fx_dw = fl2f(dp->sw*16.0);

	_fx_u_right = fl2f(rp->u*64.0f);
	_fx_v_right = fl2f(rp->v*64.0f); 
	_fx_w_right = fl2f(rp->sw*16.0);

	r = fl2f(p->r)>>7;
	g = fl2f(p->g)>>7;
	b = fl2f(p->b)>>7;

	dr = fl2f(dp->r)>>7;
	dg = fl2f(dp->g)>>7;
	db = fl2f(dp->b)>>7;

	light = ((__int64)r<<32)|((__int64)g<<16)|(__int64)b;
	deltalight = ((__int64)dr<<32)|((__int64)dg<<16)|(__int64)db;

	_asm fstenv fsave_area1
	_asm movq	mm3, light
	_asm movq	mm4, deltalight
	
	_asm {
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi
	
	mov	ebx,_fx_u
	mov	ebp,_fx_v
	mov	ecx,_fx_w
	mov	edi,_fx_destptr

; compute initial v coordinate
	mov	eax,ebp	; get v
	mov	edx,eax
	sar	edx,16
	shl	eax,16
	idiv	ecx	; eax = (v/z)
	mov	V0, eax

; compute initial u coordinate
	mov	eax,ebx	; get u
	mov	edx,eax
	sar	edx,16
	shl	eax,16
	idiv	ecx	; eax = (v/z)
	mov	U0, eax

	mov	ecx, _fx_w

; find number of subdivisions
	mov	eax, _loop_count
	inc	eax
	mov	esi, eax
	and	esi, 15
	sar	eax, NBITS
	mov	num_left_over, esi
	jz	DoEndPixels 	;there are no 2^NBITS chunks, do divide/pixel for whole scanline
	mov	_loop_count, eax

; Set deltas to NPIXS pixel increments
	mov	eax, _fx_du
	shl	eax, NBITS
	mov	DU1, eax
	mov	eax, _fx_dv
	shl	eax, NBITS
	mov	DV1, eax
	mov	eax, _fx_dw
	shl	eax, NBITS
	mov	DZ1, eax

	align	4
TopOfLoop4:
	add	ebx, DU1
	add	ebp, DV1
	add	ecx, DZ1

; Done with ebx, ebp, ecx until next iteration
	push	ebx
	push	ecx
	push	ebp
	push	edi


; Find fixed U1		
	mov	eax, ebx
	mov	edx,eax
	sar	edx,16
	shl	eax,16
	idiv	ecx	; eax = (v/z)
	mov	ebx, eax	; ebx = U1 until pop's

; Find fixed V1		
	mov	eax, ebp
	mov	edx,eax
	sar	edx,16
	shl	eax,16
	idiv	ecx	; eax = (v/z)
	mov	ebp, eax	; ebx = V1 until pop's

; Get last correct U,Vs
	mov	ecx, U0	; ecx = U0 until pop's
	mov	edi, V0	; edi = V0 until pop's

; Make ESI =  V0:U0 in 6:10,6:10 format
	mov	eax, edi
	shr	eax, 6
	mov	esi, ecx
	shl	esi, 10
	mov	si, ax
		
; Make EDX = DV:DU in 6:10,6:10 format
	mov	eax, ebp
	sub	eax, edi
	sar	eax, NBITS+6
	mov	edx, ebx
	sub	edx, ecx
	shl	edx, 10-NBITS	; EDX = V1-V0/ 4 in 6:10 int:frac
	mov	dx, ax	; put delta u in low word

; Save the U1 and V1 so we don't have to divide on the next iteration
	mov	U0, ebx
	mov	V0, ebp

	pop	edi	; Restore EDI before using it
		
	mov	ecx, _fx_srcptr
	mov	ebx, 1 << NBITS

PixelRun:
	mov			eax, esi
	shr			ax, 10
	rol			eax, 6
	and			eax, 0ffffh
	add			esi, edx
	movd			mm1, [eax*4+ecx]	
	pxor			mm2, mm2	; mm2 = 0
	punpcklbw	mm2, mm1	; mm0 = 8.8,8.8, 8.8 rgb
	pmulhw		mm2, mm3
	paddsw		mm3, mm4	; light += deltalight
	packuswb		mm2, mm2	;mm2 is who cares
	movd			[edi], mm2	; load tmp
	add			edi, 4
	dec			ebx
	jnz			PixelRun
	
	pop	ebp
	pop	ecx
	pop	ebx
	dec	_loop_count
	jnz	TopOfLoop4

;EndOfLoop4:

	test	num_left_over, -1
	je	_none_to_do

	cmp	num_left_over, 4
	ja	DoEndPixels

	; If less than 4, then just keep interpolating without
	; calculating a new DU:DV.
	mov	ecx, _fx_srcptr
	jmp	FinishOff

; ----------------------------------------- Start of LeftOver Pixels ------------------------------------------
DoEndPixels:

	push	edi
	mov	ecx, _fx_w_right

; Find fixed U1		
	mov	eax, _fx_u_right
	mov	edx,eax
	sar	edx,16
	shl	eax,16
	idiv	ecx	; eax = (v/z)
	mov	ebx, eax	; ebx = U1 until pop's

; Find fixed V1		
	mov	eax, _fx_v_right
	mov	edx,eax
	sar	edx,16
	shl	eax,16
	idiv	ecx	; eax = (v/z)
	mov	ebp, eax	; ebp = V1 until pop's

	mov	ecx, U0	; ecx = U0 until pop's
	mov	edi, V0	; edi = V0 until pop's

; Make EDX = DV:DU in 6:10,6:10 format
	mov	eax, ebx
	sub	eax, ecx
	mov	edx, eax	; These two lines are faster than cdq
	sar	edx, 31		
	idiv	num_left_over	; eax = (v1-v0)/num_left_over
	shl	eax, 16-6	; go from 16.16 to 6.10, and move into high 16 bits
	mov	esi, eax	; esi = dvdx<<16

	mov	eax, ebp
	sub	eax, edi
	mov	edx, eax	; These two lines are faster than cdq
	sar	edx, 31		
	idiv	num_left_over	; eax = (u1-u0)/num_left_over
	sar	eax, 6		; go from 16.16 to 6.10 (ax=dvdx in 6.10)
	mov	si, ax		; esi = dvdx:dudx
	mov	edx, esi

; Make ESI =  V0:U0 in 6:10,6:10 format
	mov	eax, edi
	shr	eax, 6
	mov	esi, ecx
	shl	esi, 10
	mov	si, ax
	
	pop	edi	; Restore EDI before using it
		
; LIGHTING CODE
	mov	ecx, _fx_srcptr

FinishOff:
	mov	eax, esi
	shr	ax, 10
	rol	eax, 6
	and	eax, 0ffffh
	add	esi, edx
;	mov eax, [eax*4+ecx]
;	mov [edi],eax		
	movd			mm1, [eax*4+ecx]	
	pxor			mm2, mm2	; mm2 = 0
	punpcklbw	mm2, mm1	; mm0 = 8.8,8.8, 8.8 rgb
	pmulhw		mm2, mm3
	paddsw		mm3, mm4	; light += deltalight
	packuswb		mm2, mm2	;mm2 is who cares
	movd			[edi], mm2	; load tmp
	add	edi, 4
	
	dec	num_left_over
	jnz	FinishOff


_none_to_do:	
	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	pop	edx
	pop	ecx
	pop	eax
	}	// end asm

	_asm emms
	_asm frstor fsave_area1
}


void tmapscan_lln( int lx, int rx, int y, vertex *p, vertex *dp,  vertex * rp,uint flags )
{
	__int64 light, deltalight;
	int r, g, b, dr, dg, db;
	_fx_srcptr = (uint)tmap_bitmap->data;
	_fx_destptr = (uint)GR_SCREEN_PTR(uint,lx,y);
	_loop_count = rx - lx;
	_fx_u = fl2f(p->u*64.0f);
	_fx_v = fl2f(p->v*64.0f);
	_fx_du = fl2f(dp->u*64.0f);
	_fx_dv = fl2f(dp->v*64.0f);

	r = fl2f(p->r)>>7;
	g = fl2f(p->g)>>7;
	b = fl2f(p->b)>>7;

	dr = fl2f(dp->r)>>7;
	dg = fl2f(dp->g)>>7;
	db = fl2f(dp->b)>>7;

	light = ((__int64)r<<32)|((__int64)g<<16)|(__int64)b;
	deltalight = ((__int64)dr<<32)|((__int64)dg<<16)|(__int64)db;

	_asm fstenv fsave_area1
	_asm movq	mm3, light
	_asm movq	mm4, deltalight
	
	_asm {
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi
	
	mov	ebx,_fx_u
	mov	ebp,_fx_v
	mov	edi,_fx_destptr

; find number of subdivisions
	mov	eax, _loop_count
	inc	eax
	jz		none_to_do
	mov	_loop_count, eax

; Make ESI =  V0:U0 in 6:10,6:10 format
	mov	eax, edi
	shr	eax, 6
	mov	esi, ecx
	shl	esi, 10
	mov	si, ax
		
; Make EDX = DV:DU in 6:10,6:10 format
	mov	eax, ebp
	sub	eax, edi
	sar	eax, NBITS+6
	mov	edx, ebx
	sub	edx, ecx
	shl	edx, 10-NBITS	; EDX = V1-V0/ 4 in 6:10 int:frac
	mov	dx, ax	; put delta u in low word

	mov	ecx, _fx_srcptr
	mov	ebx, _loop_count

PixelRun:
	mov			eax, esi
	shr			ax, 10
	rol			eax, 6
	and			eax, 0ffffh
	add			esi, edx
	movd			mm1, [eax*4+ecx]	
	pxor			mm2, mm2	; mm2 = 0
	punpcklbw	mm2, mm1	; mm0 = 8.8,8.8, 8.8 rgb
	pmulhw		mm2, mm3
	paddsw		mm3, mm4	; light += deltalight
	packuswb		mm2, mm2	;mm2 is who cares
	movd			[edi], mm2	; load tmp
	add			edi, 4
	dec			ebx
	jnz			PixelRun

none_to_do:

	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	pop	edx
	pop	ecx
	pop	eax
	}	// end asm

	_asm emms
	_asm frstor fsave_area1
}




void tmapscan_pln8_tiled( int lx, int rx, int y, vertex *p, vertex *dp,  vertex * rp,uint flags )
{
	Tmap1.dest_row_data = GR_SCREEN_PTR(ubyte,lx,y);
	Tmap1.loop_count = rx - lx;
	Tmap1.fx_u = fl2f(p->u);
	Tmap1.fx_v = fl2f(p->v);
	Tmap1.fx_du_dx = fl2f(dp->u);
	Tmap1.fx_dv_dx = fl2f(dp->v);

	Tmap1.fx_l = fl2f(p->l*32.0); 
	Tmap1.fx_dl_dx = fl2f(dp->l*32.0);

	Tmap1.fx_u_right = fl2f(rp->u);
	Tmap1.fx_v_right = fl2f(rp->v);
	Tmap1.pixptr = (unsigned char *)tmap_bitmap->data;
	Tmap1.bp = tmap_bitmap;
	Tmap1.src_offset = tmap_bitmap->w;


	Tmap1.FixedScale = 65536.0f;
	Tmap1.FixedScale8 =	2048.0f;	//8192.0f;	// 2^16 / 8
	Tmap1.One = 1.0f;


   Tmap1.UOverZ = p->u;
	Tmap1.VOverZ = p->v;
	Tmap1.OneOverZ = p->sw;

	Tmap1.dUOverZdX8 = dp->u*32.0f;
	Tmap1.dVOverZdX8 = dp->v*32.0f;
	Tmap1.dOneOverZdX8 = dp->sw*32.0f;

	Tmap1.dUOverZdX = dp->u;
	Tmap1.dVOverZdX = dp->v;
	Tmap1.dOneOverZdX = dp->sw;

   Tmap1.RightUOverZ = rp->u;
	Tmap1.RightVOverZ = rp->v;
	Tmap1.RightOneOverZ = rp->sw;

	Tmap1.BitmapWidth = Tmap1.bp->w;
	Tmap1.BitmapHeight = Tmap1.bp->h;

	if (Tmap1.BitmapWidth!=64) return;
	if (Tmap1.BitmapHeight!=64) return;



	if ( Tmap1.fx_dl_dx < 0 )	{
		Tmap1.fx_dl_dx = -Tmap1.fx_dl_dx;
		Tmap1.fx_l = (67*F1_0)-Tmap1.fx_l;
		Tmap1.fx_l_right = (67*F1_0)-Tmap1.fx_l_right;
	}


	_asm {
	
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi


	// put the FPU in 32 bit mode
	// @todo move this out of here!

	fstcw		Tmap1.OldFPUCW					// store copy of CW
	mov		ax,Tmap1.OldFPUCW				// get it in ax
//hh	and		eax,NOT 1100000000y			// 24 bit precision
	and		eax, ~0x300L
	mov		Tmap1.FPUCW,ax					// store it
	fldcw		Tmap1.FPUCW						// load the FPU

	mov		ecx, Tmap1.loop_count		// ecx = width
	inc		ecx
	mov		edi, Tmap1.dest_row_data	// edi = dest pointer

	// edi = pointer to start pixel in dest dib
	// ecx = spanwidth

	mov		eax,ecx							// eax and ecx = width
	shr		ecx,5								// ecx = width / subdivision length
	and		eax,31								// eax = width mod subdivision length
	jnz		some_left_over					// any leftover?
//	jmp		Return
	dec		ecx								// no, so special case last span
	mov		eax,32								// it's 8 pixels long
some_left_over:
	mov		Tmap1.Subdivisions,ecx		// store widths
	mov		Tmap1.WidthModLength,eax
    
//    mov     ebx,pLeft                   ; get left edge pointer
//    mov     edx,pGradients              ; get gradients pointer

	// calculate ULeft and VLeft			// FPU Stack (ZL = ZLeft)
													// st0  st1  st2  st3  st4  st5  st6  st7
	fld		Tmap1.VOverZ					// V/ZL 
	fld		Tmap1.UOverZ					// U/ZL V/ZL 
	fld		Tmap1.OneOverZ					// 1/ZL U/ZL V/ZL 
	fld1											// 1    1/ZL U/ZL V/ZL 
	fdiv		st,st(1)							// ZL   1/ZL U/ZL V/ZL 
	fld		st									// ZL   ZL   1/ZL U/ZL V/ZL 
	fmul		st,st(4)							// VL   ZL   1/ZL U/ZL V/ZL 
	fxch		st(1)								// ZL   VL   1/ZL U/ZL V/ZL 
	fmul		st,st(3)							// UL   VL   1/ZL U/ZL V/ZL 

	fstp		st(5)								// VL   1/ZL U/ZL V/ZL UL
	fstp		st(5)								// 1/ZL U/ZL V/ZL UL   VL

	// calculate right side OverZ terms  ; st0  st1  st2  st3  st4  st5  st6  st7

	fadd		Tmap1.dOneOverZdX8			// 1/ZR U/ZL V/ZL UL   VL
	fxch		st(1)								// U/ZL 1/ZR V/ZL UL   VL
	fadd		Tmap1.dUOverZdX8				// U/ZR 1/ZR V/ZL UL   VL
	fxch		st(2)								// V/ZL 1/ZR U/ZR UL   VL
	fadd		Tmap1.dVOverZdX8				// V/ZR 1/ZR U/ZR UL   VL

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
	fmul    Tmap1.FixedScale            ; UL16 UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fistp   Tmap1.UFixed                ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

	fld     st(6)                       ; VL   UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fmul    Tmap1.FixedScale            ; VL16 UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fistp   Tmap1.VFixed                ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

	// calculate deltas                  ; st0  st1  st2  st3  st4  st5  st6  st7

	fsubr   st(5),st                    ; UR   VR   V/ZR 1/ZR U/ZR dU   VL
	fxch    st(1)                       ; VR   UR   V/ZR 1/ZR U/ZR dU   VL
	fsubr   st(6),st                    ; VR   UR   V/ZR 1/ZR U/ZR dU   dV
	fxch    st(6)                       ; dV   UR   V/ZR 1/ZR U/ZR dU   VR

	fmul    Tmap1.FixedScale8           ; dV8  UR   V/ZR 1/ZR U/ZR dU   VR
	fistp   Tmap1.DeltaV                ; UR   V/ZR 1/ZR U/ZR dU   VR

	fxch    st(4)                       ; dU   V/ZR 1/ZR U/ZR UR   VR
	fmul    Tmap1.FixedScale8           ; dU8  V/ZR 1/ZR U/ZR UR   VR
	fistp   Tmap1.DeltaU                ; V/ZR 1/ZR U/ZR UR   VR

	// increment terms for next span     ; st0  st1  st2  st3  st4  st5  st6  st7
	// Right terms become Left terms---->; V/ZL 1/ZL U/ZL UL   VL

	fadd    Tmap1.dVOverZdX8            ; V/ZR 1/ZL U/ZL UL   VL
	fxch    st(1)                       ; 1/ZL V/ZR U/ZL UL   VL
	fadd    Tmap1.dOneOverZdX8          ; 1/ZR V/ZR U/ZL UL   VL
	fxch    st(2)                       ; U/ZL V/ZR 1/ZR UL   VL
	fadd    Tmap1.dUOverZdX8            ; U/ZR V/ZR 1/ZR UL   VL
	fxch    st(2)                       ; 1/ZR V/ZR U/ZR UL   VL
	fxch    st(1)                       ; V/ZR 1/ZR U/ZR UL   VL

	; calculate right side coords       ; st0  st1  st2  st3  st4  st5  st6  st7

	fld1                                ; 1    V/ZR 1/ZR U/ZR UL   VL
	fdiv    st,st(2)                    ; ZR   V/ZR 1/ZR U/ZR UL   VL


    ; ************** Can't Access Stack Frame ******************
    ; ************** Can't Access Stack Frame ******************
    ; ************** Can't Access Stack Frame ******************

    // 8 pixel span code
    // edi = dest dib bits at current pixel
    // esi = texture pointer at current u,v
    // eax = scratch
    // ebx = u fraction 0.32
    // ecx = v fraction 0.32
    // edx = u frac step
    // ebp = v carry scratch

	//need to have:
	// eax = ?
	// ebx = l in 24.8
	// ecx = source pixels
	// edx = u v  in 6.10 6.10
	// esi = du dv in 6.10 6.10
	// edi = dest pixels
	// ebp = dldx in 24.8


 	mov	eax, Tmap1.fx_l
	shr	eax, 8
	mov	ebx, eax

	mov	ebp, Tmap1.fx_dl_dx
	shl	ebp, 5	//*32
	add	Tmap1.fx_l, ebp

	mov	ebp, Tmap1.fx_l
	shr	ebp, 8
	sub	ebp, eax
	shr	ebp, 5

	mov	ecx, Tmap1.pixptr	// ecx = source pixels

; Make ESI =  DV:DU in 6:10,6:10 format
	mov	eax, Tmap1.DeltaU
	shr	eax, 6
	mov	esi, Tmap1.DeltaV
	shl	esi, 10
	mov	si, ax
		
; Make EDX = DV:DU in 6:10,6:10 format

	mov	eax, Tmap1.UFixed
	shr	eax, 6
	mov	edx, Tmap1.VFixed
	shl	edx, 10
	mov	dx, ax

	; Draw 32 pixels

		; pixel 0
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+0], al

		; pixel 1
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+1], al

		; pixel 2
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+2], al

		; pixel 3
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+3], al

		; pixel 4
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+4], al

		; pixel 5
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+5], al

		; pixel 6
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+6], al

		; pixel 7
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+7], al

		; pixel 8
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+8], al

		; pixel 9
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+9], al

		; pixel 10
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+10], al

		; pixel 11
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+11], al

		; pixel 12
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+12], al

		; pixel 13
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+13], al

		; pixel 14
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+14], al

		; pixel 15
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+15], al

		; pixel 16
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+16], al

		; pixel 17
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+17], al

		; pixel 18
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+18], al

		; pixel 19
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+19], al

		; pixel 20
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+20], al

		; pixel 21
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+21], al

		; pixel 22
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+22], al

		; pixel 23
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+23], al

		; pixel 24
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+24], al

		; pixel 25
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+25], al

		; pixel 26
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+26], al

		; pixel 27
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+27], al

		; pixel 28
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+28], al

		; pixel 29
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+29], al

		; pixel 30
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+30], al

		; pixel 31
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+31], al

    
    ; ************** Okay to Access Stack Frame ****************
    ; ************** Okay to Access Stack Frame ****************
    ; ************** Okay to Access Stack Frame ****************


    ; the fdiv is done, finish right    ; st0  st1  st2  st3  st4  st5  st6  st7
                                        ; ZR   V/ZR 1/ZR U/ZR UL   VL

    fld     st                          ; ZR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(2)                    ; VR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fxch    st(1)                       ; ZR   VR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(4)                    ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

    add     edi,32                       ; increment to next span
    dec     Tmap1.Subdivisions              ; decrement span count
    jnz     SpanLoop                    ; loop back

HandleLeftoverPixels:

    mov     esi,Tmap1.pixptr          ; load texture pointer

    ; edi = dest dib bits
    ; esi = current texture dib bits
    ; at this point the FPU contains    ; st0  st1  st2  st3  st4  st5  st6  st7
    ; inv. means invalid numbers        ; inv. inv. inv. inv. inv. UL   VL

    cmp     Tmap1.WidthModLength,0          ; are there remaining pixels to draw?
    jz      FPUReturn                   ; nope, pop the FPU and bail

    ; convert left side coords          ; st0  st1  st2  st3  st4  st5  st6  st7

    fld     st(5)                       ; UL   inv. inv. inv. inv. inv. UL   VL
    fmul    Tmap1.FixedScale                ; UL16 inv. inv. inv. inv. inv. UL   VL
    fistp   Tmap1.UFixed                    ; inv. inv. inv. inv. inv. UL   VL

    fld     st(6)                       ; VL   inv. inv. inv. inv. inv. UL   VL
    fmul    Tmap1.FixedScale                // VL16 inv. inv. inv. inv. inv. UL   VL
    fistp   Tmap1.VFixed                    ; inv. inv. inv. inv. inv. UL   VL

    dec     Tmap1.WidthModLength            ; calc how many steps to take
    jz      OnePixelSpan                ; just one, don't do deltas

    ; calculate right edge coordinates  ; st0  st1  st2  st3  st4  st5  st6  st7
    ; r -> R+1

    ; @todo rearrange things so we don't need these two instructions
    fstp    Tmap1.FloatTemp                 ; inv. inv. inv. inv. UL   VL
    fstp    Tmap1.FloatTemp                 ; inv. inv. inv. UL   VL

    fld     Tmap1.RightVOverZ           ; V/Zr inv. inv. inv. UL   VL
    fsub    Tmap1.dVOverZdX             ; V/ZR inv. inv. inv. UL   VL
    fld     Tmap1.RightUOverZ           ; U/Zr V/ZR inv. inv. inv. UL   VL
    fsub    Tmap1.dUOverZdX             ; U/ZR V/ZR inv. inv. inv. UL   VL
    fld     Tmap1.RightOneOverZ              ; 1/Zr U/ZR V/ZR inv. inv. inv. UL   VL
    fsub    Tmap1.dOneOverZdX           ; 1/ZR U/ZR V/ZR inv. inv. inv. UL   VL

    fdivr   Tmap1.One                       ; ZR   U/ZR V/ZR inv. inv. inv. UL   VL

    fmul    st(1),st                    ; ZR   UR   V/ZR inv. inv. inv. UL   VL
    fmulp   st(2),st                    ; UR   VR   inv. inv. inv. UL   VL

    ; calculate deltas                  ; st0  st1  st2  st3  st4  st5  st6  st7

    fsubr   st(5),st                    ; UR   VR   inv. inv. inv. dU   VL
    fxch    st(1)                       ; VR   UR   inv. inv. inv. dU   VL
    fsubr   st(6),st                    ; VR   UR   inv. inv. inv. dU   dV
    fxch    st(6)                       ; dV   UR   inv. inv. inv. dU   VR

    fidiv   Tmap1.WidthModLength            ; dv   UR   inv. inv. inv. dU   VR
    fmul    Tmap1.FixedScale                ; dv16 UR   inv. inv. inv. dU   VR
    fistp   Tmap1.DeltaV                    ; UR   inv. inv. inv. dU   VR

    fxch    st(4)                       ; dU   inv. inv. inv. UR   VR
    fidiv   Tmap1.WidthModLength            ; du   inv. inv. inv. UR   VR
    fmul    Tmap1.FixedScale                ; du16 inv. inv. inv. UR   VR
    fistp   Tmap1.DeltaU                    ; inv. inv. inv. UR   VR

    ; @todo gross!  these are to line up with the other loop
    fld     st(1)                       ; inv. inv. inv. inv. UR   VR
    fld     st(2)                       ; inv. inv. inv. inv. inv. UR   VR


OnePixelSpan:
 	mov	eax, Tmap1.fx_l
	shr	eax, 8
	mov	ebx, eax

	mov	ebp, Tmap1.fx_dl_dx
	shl	ebp, 5	//*32
	add	Tmap1.fx_l, ebp

	mov	ebp, Tmap1.fx_l
	shr	ebp, 8
	sub	ebp, eax
	shr	ebp, 5


; Make ESI =  DV:DU in 6:10,6:10 format
	mov	eax, Tmap1.DeltaU
	shr	eax, 6
	mov	esi, Tmap1.DeltaV
	shl	esi, 10
	mov	si, ax
		
; Make EDX = DV:DU in 6:10,6:10 format

	mov	eax, Tmap1.UFixed
	shr	eax, 6
	mov	edx, Tmap1.VFixed
	shl	edx, 10
	mov	dx, ax

	mov	ecx, Tmap1.pixptr	// ecx = source pixels

	inc	Tmap1.WidthModLength
	mov	eax,Tmap1.WidthModLength
	shr	eax, 1
	jz		one_more_pix
	pushf
	mov	Tmap1.WidthModLength, eax


NextPixel:

	; Draw two pixels

		; pixel 0
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+0], al

		; pixel 1
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+1], al


	add	edi, 2
	dec	Tmap1.WidthModLength
	jg		NextPixel

	popf
	jnc	FPUReturn

one_more_pix:	

	; Draw one pixel
	; pixel 0
	mov	eax, edx	
	shr	ax, 10
	rol	eax, 6
	and	eax, 0ffffh
	add	edx, esi
	mov	al, [ecx+eax]			
	mov	ah, bh	    	
	add	ebx, ebp
	mov	al, gr_fade_table[eax]
	mov	[edi+0], al

FPUReturn:

    ; busy FPU registers:               ; st0  st1  st2  st3  st4  st5  st6  st7
                                        ; xxx  xxx  xxx  xxx  xxx  xxx  xxx
    ffree   st(0)
    ffree   st(1)
    ffree   st(2)
    ffree   st(3)
    ffree   st(4)
    ffree   st(5)
    ffree   st(6)

//Return:

    fldcw   Tmap1.OldFPUCW                  // restore the FPU

	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	pop	edx
	pop	ecx
	pop	eax
	}


}

void c_tmap_scanline_flat()
{
	switch( gr_screen.bits_per_pixel )	{
	case 8:
		#if 1
			memset( Tmap1.dest_row_data, gr_screen.current_color.raw8, Tmap1.loop_count );
		#else
			ubyte *dest;
			int x;

			dest = Tmap1.dest_row_data;

			for (x=Tmap1.loop_count; x >= 0; x-- ) {
				//(*dest)++;dest++;
				*dest++ = Tmap1.tmap_flat_color;
			}
		#endif
		break;
	case 15:
	case 16:
		_asm	mov	ecx, Tmap1.loop_count
		_asm	mov	ax, gr_screen.current_color.raw16;
		_asm	mov	edi, Tmap1.dest_row_data16
		_asm	cld
		_asm	rep	stosw
		break;
	case 24:
		_asm	mov	ecx, Tmap1.loop_count
		_asm	mov	ax, gr_screen.current_color.raw16;
		_asm	mov	edi, Tmap1.dest_row_data16
		_asm	cld
		_asm	rep	stosw
		break;
	case 32:
		_asm	mov	ecx, Tmap1.loop_count
		_asm	mov	eax, gr_screen.current_color.raw32;
		_asm	mov	edi, Tmap1.dest_row_data32
		_asm	cld
		_asm	rep	stosd
		break;
	}

}

void c_tmap_scanline_shaded()
{
	int fade;
	ubyte *dest;
	int x;

	dest = Tmap1.dest_row_data;

	fade = Tmap1.tmap_flat_shade_value<<8;
	for (x=Tmap1.loop_count; x >= 0; x-- ) {
		*dest++ = gr_fade_table[ fade |(*dest)];
	}
}

void c_tmap_scanline_lin_nolight()
{
	ubyte *dest;
	uint c;
	int x;
	fix u,v,dudx, dvdx;

	u = Tmap1.fx_u;
	v = Tmap1.fx_v*64;
	dudx = Tmap1.fx_du_dx; 
	dvdx = Tmap1.fx_dv_dx*64; 

	dest = Tmap1.dest_row_data;

	if (!Tmap1.Transparency_on)	{
		for (x=Tmap1.loop_count; x >= 0; x-- ) {
			*dest++ = (uint)Tmap1.pixptr[ (f2i(v)&(64*63)) + (f2i(u)&63) ];
			u += dudx;
			v += dvdx;
		}
	} else {
		for (x=Tmap1.loop_count; x >= 0; x-- ) {
			c = (uint)Tmap1.pixptr[ (f2i(v)&(64*63)) + (f2i(u)&63) ];
			if ( c!=255)
				*dest = c;
			dest++;
			u += dudx;
			v += dvdx;
		}
	}
}


void c_tmap_scanline_lin()
{

}



void c_tmap_scanline_per_nolight()
{
	ubyte *dest;
	uint c;
	int x;
	fix u,v,z,dudx, dvdx, dzdx;

	u = Tmap1.fx_u;
	v = Tmap1.fx_v*64;
	z = Tmap1.fx_z;
	dudx = Tmap1.fx_du_dx; 
	dvdx = Tmap1.fx_dv_dx*64; 
	dzdx = Tmap1.fx_dz_dx;

	dest = Tmap1.dest_row_data;

	if (!Tmap1.Transparency_on)	{
		for (x=Tmap1.loop_count; x >= 0; x-- ) {
			*dest++ = (uint)Tmap1.pixptr[ ( (v/z)&(64*63) ) + ((u/z)&63) ];
			u += dudx;
			v += dvdx;
			z += dzdx;
		}
	} else {
		for (x=Tmap1.loop_count; x >= 0; x-- ) {
			c = (uint)Tmap1.pixptr[ ( (v/z)&(64*63) ) + ((u/z)&63) ];
			if ( c!=255)
				*dest = c;
			dest++;
			u += dudx;
			v += dvdx;
			z += dzdx;
		}
	}
}

void c_tmap_scanline_per1()
{
	ubyte *dest;
	uint c;
	int x;
	fix u,v,z,l,dudx, dvdx, dzdx, dldx;

	u = Tmap1.fx_u;
	v = Tmap1.fx_v*64;
	z = Tmap1.fx_z;
	dudx = Tmap1.fx_du_dx; 
	dvdx = Tmap1.fx_dv_dx*64; 
	dzdx = Tmap1.fx_dz_dx;

	l = Tmap1.fx_l;
	dldx = Tmap1.fx_dl_dx;
	dest = Tmap1.dest_row_data;

	if (!Tmap1.Transparency_on)	{
		for (x=Tmap1.loop_count; x >= 0; x-- ) {
			*dest++ = gr_fade_table[ (l&(0xff00)) + (uint)Tmap1.pixptr[ ( (v/z)&(64*63) ) + ((u/z)&63) ] ];
			l += dldx;
			u += dudx;
			v += dvdx;
			z += dzdx;
		}
	} else {
		for (x=Tmap1.loop_count; x >= 0; x-- ) {
			c = (uint)Tmap1.pixptr[ ( (v/z)&(64*63) ) + ((u/z)&63) ];
			if ( c!=255)
				*dest = gr_fade_table[ (l&(0xff00)) + c ];
			dest++;
			l += dldx;
			u += dudx;
			v += dvdx;
			z += dzdx;
		}
	}
}

#define zonk 1

void c_tmap_scanline_editor()
{
	ubyte *dest;
	uint c;
	int x;
	fix u,v,z,dudx, dvdx, dzdx;

	u = Tmap1.fx_u;
	v = Tmap1.fx_v*64;
	z = Tmap1.fx_z;
	dudx = Tmap1.fx_du_dx; 
	dvdx = Tmap1.fx_dv_dx*64; 
	dzdx = Tmap1.fx_dz_dx;

	dest = Tmap1.dest_row_data;

	if (!Tmap1.Transparency_on)	{
		for (x=Tmap1.loop_count; x >= 0; x-- ) {
			*dest++ = zonk;
			//(uint)pixptr[ ( (v/z)&(64*63) ) + ((u/z)&63) ];
			u += dudx;
			v += dvdx;
			z += dzdx;
		}
	} else {
		for (x=Tmap1.loop_count; x >= 0; x-- ) {
			c = (uint)Tmap1.pixptr[ ( (v/z)&(64*63) ) + ((u/z)&63) ];
			if ( c!=255)
				*dest = zonk;
			dest++;
			u += dudx;
			v += dvdx;
			z += dzdx;
		}
	}
}

void asm_tmap_scanline_lln_tiled()
{
	if ( Tmap1.BitmapWidth != 64 ) return;
	if ( Tmap1.BitmapHeight != 64 ) return;

	_asm {
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi
	
	; set edi = address of first pixel to modify
	mov	edi, Tmap1.dest_row_data

	mov	eax, Tmap1.fx_v
	shr	eax, 6
	mov	edx, Tmap1.fx_u
	shl	edx, 10
	mov	dx, ax		; EDX=U:V in 6.10 format

	mov	eax, Tmap1.fx_dv_dx
	shr	eax, 6
	mov	esi, Tmap1.fx_du_dx
	shl	esi, 10
	mov	si, ax		; ESI=DU:DV in 6.10 format

	mov	ebx, Tmap1.fx_l
	sar	ebx, 8
	mov	ebp, Tmap1.fx_dl_dx
	sar	ebp, 8

	mov	ecx, Tmap1.pixptr

	mov	eax, Tmap1.loop_count
	inc	eax
	mov	Tmap1.loop_count, eax

	shr	eax, 3
	je		DoLeftOverPixels

	mov	Tmap1.num_big_steps, eax
	and	Tmap1.loop_count, 7

NextPixelBlock:
		; pixel 0
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+0], al

		; pixel 1
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+1], al

		; pixel 2
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+2], al

		; pixel 3
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+3], al

		; pixel 4
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+4], al

		; pixel 5
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+5], al

		; pixel 6
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+6], al

		; pixel 7
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+7], al

	add	edi, 8
	dec	Tmap1.num_big_steps
	jne	NextPixelBlock
	

DoLeftOverPixels:
	mov	eax,Tmap1.loop_count
	test	eax, -1
	jz	_none_to_do
	shr	eax, 1
	je	one_more_pix
	mov	Tmap1.loop_count, eax
	pushf


NextPixel:
		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+0], al

		mov	eax, edx	
		shr	ax, 10
		rol	eax, 6
		and	eax, 0ffffh
		add	edx, esi
		mov	al, [ecx+eax]			
		mov	ah, bh	    	
		add	ebx, ebp
		mov	al, gr_fade_table[eax]
		mov	[edi+1], al


	add	edi, 2
	dec	Tmap1.loop_count
	jne	NextPixel

	popf
	jnc	_none_to_do

one_more_pix:	
	mov	eax, edx	
	shr	ax, 10
	rol	eax, 6
	and	eax, 0ffffh
	mov	al, [ecx+eax]			
	mov	ah, bh	    	
	mov	al, gr_fade_table[eax]
	mov	[edi], al

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

void asm_tmap_scanline_lln32();

void asm_tmap_scanline_lln()
{
	int end;

//	return;
	if ( Tmap1.tmap_flags & TMAP_FLAG_TILED )	{
		asm_tmap_scanline_lln_tiled();
		return;
	}

	end = f2i(Tmap1.fx_u);
	if ( end >= Tmap1.bp->w ) return;
	
	end = f2i(Tmap1.fx_v);
	if ( end >= Tmap1.bp->h ) return;

	end = f2i(Tmap1.fx_u_right);
	if ( end >= Tmap1.bp->w ) return;

	end = f2i(Tmap1.fx_v_right);
	if ( end >= Tmap1.bp->h ) return;

	if ( Tmap1.fx_dl_dx < 0 )	{
		Tmap1.fx_dl_dx = -Tmap1.fx_dl_dx;
		Tmap1.fx_l = (67*F1_0)-Tmap1.fx_l;
		Tmap1.fx_l_right = (67*F1_0)-Tmap1.fx_l_right;
//		return;
//		Assert( Tmap1.fx_l > 31*F1_0 );
//		Assert( Tmap1.fx_l < 66*F1_0 );
//		Assert( Tmap1.fx_dl_dx >= 0 );
//		Assert( Tmap1.fx_dl_dx < 31*F1_0 );
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
	mov	eax, Tmap1.fx_dv_dx	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap1.DeltaVFrac, ebx	// store it
	imul	eax, Tmap1.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap1.fx_du_dx		// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap1.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step
	mov	Tmap1.UVintVfracStepVNoCarry, eax	// save whole step in non-v-carry slot
	add	eax, Tmap1.src_offset				// calc whole step + v carry
	mov	Tmap1.UVintVfracStepVCarry, eax	// save in v-carry slot

	; setup initial coordinates
	mov	esi, Tmap1.fx_u			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap1.fx_v			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap1.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap1.pixptr			// calc address
	
	; set edi = address of first pixel to modify
	mov	edi, Tmap1.dest_row_data
	
	mov	edx, Tmap1.DeltaUFrac

	mov	eax, Tmap1.loop_count
	inc	eax
	mov	Tmap1.loop_count, eax

	shr	eax, 3
	je		DoLeftOverPixels

	mov	Tmap1.num_big_steps, eax
	and	Tmap1.loop_count, 7


NextPixelBlock:

	mov	eax, Tmap1.fx_l
	shr	eax, 8
	mov	bx, ax

	mov	ebp, Tmap1.fx_dl_dx
	shl	ebp, 3	//*32
	add	Tmap1.fx_l, ebp

	mov	ebp, Tmap1.fx_l
	shr	ebp, 8
	sub	bp, ax
	shr	bp, 3

	mov	dx, bp


    // 8 pixel span code
    // edi = dest dib bits at current pixel
    // esi = texture pointer at current u,v
    // eax = scratch
    // ebx = u fraction 0.32
    // ecx = v fraction 0.32
    // edx = u frac step
    // ebp = v carry scratch

    mov     al,[edi]                    // preread the destination cache line

    mov     al,[esi]                    // get texture pixel 0
    mov		ah, bh
    mov		al, gr_fade_table[eax]

    add     ecx,Tmap1.DeltaVFrac            // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+0],al                  // store pixel 0

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 1
    mov		ah, bh
    mov		al, gr_fade_table[eax]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+1],al                  // store pixel 1

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 2
    mov		ah, bh
    mov		al, gr_fade_table[eax]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+2],al                  // store pixel 2

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 3
    mov		ah, bh
    mov		al, gr_fade_table[eax]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+3],al                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		al, gr_fade_table[eax]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+4],al                  // store pixel 4

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 5
    mov		ah, bh
    mov		al, gr_fade_table[eax]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+5],al                  // store pixel 5

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 6
    mov		ah, bh
    mov		al, gr_fade_table[eax]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+6],al                  // store pixel 6

    add     ebx,edx                     // increment u fraction

    mov     al,[esi]                    // get texture pixel 7
    mov		ah, bh
    mov		al, gr_fade_table[eax]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries

    mov     [edi+7],al                 // store pixel 7
 
		; end
	

	add	edi, 8
	dec	Tmap1.num_big_steps
	jne	NextPixelBlock
	

DoLeftOverPixels:

	mov	eax,Tmap1.loop_count
	test	eax, -1
	jz	_none_to_do
	shr	eax, 1
	je	one_more_pix
	mov	Tmap1.loop_count, eax
	pushf

	xor	eax, eax


	mov	eax, Tmap1.fx_l
	shr	eax, 8
	mov	bx, ax

	mov	ebp, Tmap1.fx_dl_dx
	shr	ebp, 8
	mov	dx, bp

    mov     al,[edi]                    // preread the destination cache line
//    add     ebx,edx                     // increment u fraction

NextPixel:

    mov     al,[esi]                    // get texture pixel 0
    mov		ah, bh
    mov		al, gr_fade_table[eax]

    add     ecx,Tmap1.DeltaVFrac            // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    mov     [edi+0],al                  // store pixel 0

    add     ecx,Tmap1.DeltaVFrac        // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 1
    mov		ah, bh
    mov		al, gr_fade_table[eax]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    mov     [edi+1],al                  // store pixel 1

	add	edi, 2
	dec	Tmap1.loop_count
	jne	NextPixel

	popf
	jnc	_none_to_do

one_more_pix:	

    mov     al,[esi]                    // get texture pixel 2
    mov		ah, bh
    mov		al, gr_fade_table[eax]
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


void asm_tmap_scanline_lln32()
{
	int end;

	end = f2i(Tmap1.fx_u);
	if ( end >= Tmap1.bp->w ) return;
	
	end = f2i(Tmap1.fx_v);
	if ( end >= Tmap1.bp->h ) return;

	end = f2i(Tmap1.fx_u_right);
	if ( end >= Tmap1.bp->w ) return;

	end = f2i(Tmap1.fx_v_right);
	if ( end >= Tmap1.bp->h ) return;

	_asm {
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi

	; setup delta values
	mov	eax, Tmap1.fx_dv_dx	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap1.DeltaVFrac, ebx	// store it
	imul	eax, Tmap1.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap1.fx_du_dx		// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap1.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step
	mov	Tmap1.UVintVfracStepVNoCarry, eax	// save whole step in non-v-carry slot
	add	eax, Tmap1.src_offset				// calc whole step + v carry
	mov	Tmap1.UVintVfracStepVCarry, eax	// save in v-carry slot

	; setup initial coordinates
	mov	esi, Tmap1.fx_u			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap1.fx_v			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap1.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap1.pixptr			// calc address
	
	; set edi = address of first pixel to modify
	mov	edi, Tmap1.dest_row_data32
	
	mov	edx, Tmap1.DeltaUFrac

	mov	eax, Tmap1.fx_l	// use bx and dx to do lighting
	mov	bx, ax
	mov	eax, Tmap1.fx_dl_dx	// use bx and dx to do lighting
	mov	dx, ax

	mov	eax, Tmap1.loop_count
	inc	eax
	mov	Tmap1.loop_count, eax

	shr	eax, 3
	je		DoLeftOverPixels

	mov	Tmap1.num_big_steps, eax
	and	Tmap1.loop_count, 7


NextPixelBlock:

    // 8 pixel span code
    // edi = dest dib bits at current pixel
    // esi = texture pointer at current u,v
    // eax = scratch
    // ebx = u fraction 0.32
    // ecx = v fraction 0.32
    // edx = u frac step
    // ebp = v carry scratch

    mov     al,[edi]                    // preread the destination cache line

    mov     al,[esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]

    add     ecx,Tmap1.DeltaVFrac            // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+0],eax                  // store pixel 0

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 1
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+4],al                  // store pixel 1

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 2
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+8],eax                  // store pixel 2

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 3
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+12],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+16],eax                  // store pixel 4

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 5
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+20],eax                  // store pixel 5

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 6
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+24],eax                  // store pixel 6

    add     ebx,edx                     // increment u fraction

    mov     al,[esi]                    // get texture pixel 7
    mov		ah, bh
    mov		eax, gr_fade_table32[eax]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries

    mov     [edi+28],eax                 // store pixel 7
 
		; end
	

	add	edi, 8*4
	dec	Tmap1.num_big_steps
	jne	NextPixelBlock
	

DoLeftOverPixels:

	mov	eax,Tmap1.loop_count
	test	eax, -1
	jz	_none_to_do
	shr	eax, 1
	je	one_more_pix
	mov	Tmap1.loop_count, eax
	pushf

	xor	eax, eax

NextPixel:
    mov     al,[edi]                    // preread the destination cache line

    mov     al,[esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]

    add     ecx,Tmap1.DeltaVFrac            // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    mov     [edi+0],eax                  // store pixel 0

    add     ecx,Tmap1.DeltaVFrac        // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 1
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    mov     [edi+1],al                  // store pixel 1

	add	edi, 2*4
	dec	Tmap1.loop_count
	jne	NextPixel

	popf
	jnc	_none_to_do

one_more_pix:	

    mov     al,[esi]                    // get texture pixel 2
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]
    mov     [edi],eax                  // store pixel 2

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

void asm_tmap_scanline_lnt()
{
	int end;

	end = f2i(Tmap1.fx_u);
	if ( end >= Tmap1.bp->w ) return;
	
	end = f2i(Tmap1.fx_v);
	if ( end >= Tmap1.bp->h ) return;

	end = f2i(Tmap1.fx_u_right);
	if ( end >= Tmap1.bp->w ) return;

	end = f2i(Tmap1.fx_v_right);
	if ( end >= Tmap1.bp->h ) return;


	_asm {
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi

	; setup delta values
	mov	eax, Tmap1.fx_dv_dx	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap1.DeltaVFrac, ebx	// store it
	imul	eax, Tmap1.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap1.fx_du_dx		// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap1.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step
	mov	Tmap1.UVintVfracStepVNoCarry, eax	// save whole step in non-v-carry slot
	add	eax, Tmap1.src_offset				// calc whole step + v carry
	mov	Tmap1.UVintVfracStepVCarry, eax	// save in v-carry slot

	; setup initial coordinates
	mov	esi, Tmap1.fx_u			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap1.fx_v			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap1.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap1.pixptr			// calc address
	
	; set edi = address of first pixel to modify
	mov	edi, Tmap1.dest_row_data
	
	mov	edx, Tmap1.DeltaUFrac

	mov	eax, Tmap1.loop_count
	inc	eax
	mov	Tmap1.loop_count, eax

	shr	eax, 3
	je		DoLeftOverPixels

	mov	Tmap1.num_big_steps, eax
	and	Tmap1.loop_count, 7


NextPixelBlock:

    // 8 pixel span code
    // edi = dest dib bits at current pixel
    // esi = texture pointer at current u,v
    // eax = scratch
    // ebx = u fraction 0.32
    // ecx = v fraction 0.32
    // edx = u frac step
    // ebp = v carry scratch

    mov     al,[edi]                    // preread the destination cache line

    mov     al,[esi]                    // get texture pixel 0

    add     ecx,Tmap1.DeltaVFrac            // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
	 cmp		al, 255
	 je		skip0
    mov     [edi+0],al                  // store pixel 0
skip0:

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 1

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
	 cmp		al, 255
	 je		skip1
    mov     [edi+1],al                  // store pixel 0
skip1:

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 2

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
	 cmp		al, 255
	 je		skip2
    mov     [edi+2],al                  // store pixel 0
skip2:

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 3

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
	 cmp		al, 255
	 je		skip3
    mov     [edi+3],al                  // store pixel 0
skip3:

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
	 cmp		al, 255
	 je		skip4
    mov     [edi+4],al                  // store pixel 0
skip4:

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 5

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
	 cmp		al, 255
	 je		skip5
    mov     [edi+5],al                  // store pixel 0
skip5:

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 6

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
	 cmp		al, 255
	 je		skip6
    mov     [edi+6],al                  // store pixel 0
skip6:

    add     ebx,edx                     // increment u fraction

    mov     al,[esi]                    // get texture pixel 7

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries

	 cmp		al, 255
	 je		skip7
    mov     [edi+7],al                  // store pixel 0
skip7:
 
		; end
	

	add	edi, 8
	dec	Tmap1.num_big_steps
	jne	NextPixelBlock
	

DoLeftOverPixels:

	mov	eax,Tmap1.loop_count
	test	eax, -1
	jz	_none_to_do
	shr	eax, 1
	je	one_more_pix
	mov	Tmap1.loop_count, eax
	pushf

	xor	eax, eax

    mov     al,[edi]                    // preread the destination cache line
//    add     ebx,edx                     // increment u fraction

NextPixel:

    mov     al,[esi]                    // get texture pixel 0

    add     ecx,Tmap1.DeltaVFrac            // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
	 cmp		al, 255
	 je		skipA0
    mov     [edi+0],al                  // store pixel 0
skipA0:

    add     ecx,Tmap1.DeltaVFrac        // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 1

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
	 cmp		al, 255
	 je		skipA1
    mov     [edi+1],al                  // store pixel 0
skipA1:

	add	edi, 2
	dec	Tmap1.loop_count
	jne	NextPixel

	popf
	jnc	_none_to_do

one_more_pix:	

    mov     al,[esi]                    // get texture pixel 2
	 cmp		al, 255
	 je		skipB
    mov     [edi],al                  // store pixel 0
skipB:

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


void asm_tmap_scanline_lnn()
{
	int end;

	end = f2i(Tmap1.fx_u);
	if ( end >= Tmap1.bp->w ) return;
	
	end = f2i(Tmap1.fx_v);
	if ( end >= Tmap1.bp->h ) return;

	end = f2i(Tmap1.fx_u_right);
	if ( end >= Tmap1.bp->w ) return;

	end = f2i(Tmap1.fx_v_right);
	if ( end >= Tmap1.bp->h ) return;


	_asm {
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi

	; setup delta values
	mov	eax, Tmap1.fx_dv_dx	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap1.DeltaVFrac, ebx	// store it
	imul	eax, Tmap1.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap1.fx_du_dx		// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap1.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step
	mov	Tmap1.UVintVfracStepVNoCarry, eax	// save whole step in non-v-carry slot
	add	eax, Tmap1.src_offset				// calc whole step + v carry
	mov	Tmap1.UVintVfracStepVCarry, eax	// save in v-carry slot

	; setup initial coordinates
	mov	esi, Tmap1.fx_u			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap1.fx_v			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap1.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap1.pixptr			// calc address
	
	; set edi = address of first pixel to modify
	mov	edi, Tmap1.dest_row_data
	
	mov	edx, Tmap1.DeltaUFrac

	mov	eax, Tmap1.loop_count
	inc	eax
	mov	Tmap1.loop_count, eax

	shr	eax, 3
	je		DoLeftOverPixels

	mov	Tmap1.num_big_steps, eax
	and	Tmap1.loop_count, 7


NextPixelBlock:

    // 8 pixel span code
    // edi = dest dib bits at current pixel
    // esi = texture pointer at current u,v
    // eax = scratch
    // ebx = u fraction 0.32
    // ecx = v fraction 0.32
    // edx = u frac step
    // ebp = v carry scratch

    mov     al,[edi]                    // preread the destination cache line

    mov     al,[esi]                    // get texture pixel 0

    add     ecx,Tmap1.DeltaVFrac            // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+0],al                  // store pixel 0

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 1

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+1],al                  // store pixel 0

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 2

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+2],al                  // store pixel 0

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 3

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+3],al                  // store pixel 0

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+4],al                  // store pixel 0

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 5

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+5],al                  // store pixel 0

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 6

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+6],al                  // store pixel 0

    add     ebx,edx                     // increment u fraction

    mov     al,[esi]                    // get texture pixel 7

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries

    mov     [edi+7],al                  // store pixel 0
 
		; end
	

	add	edi, 8
	dec	Tmap1.num_big_steps
	jne	NextPixelBlock
	

DoLeftOverPixels:

	mov	eax,Tmap1.loop_count
	test	eax, -1
	jz	_none_to_do
	shr	eax, 1
	je	one_more_pix
	mov	Tmap1.loop_count, eax
	pushf

	xor	eax, eax

    mov     al,[edi]                    // preread the destination cache line
//    add     ebx,edx                     // increment u fraction

NextPixel:

    mov     al,[esi]                    // get texture pixel 0

    add     ecx,Tmap1.DeltaVFrac            // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    mov     [edi+0],al                  // store pixel 0

    add     ecx,Tmap1.DeltaVFrac        // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 1

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    mov     [edi+1],al                  // store pixel 0

	add	edi, 2
	dec	Tmap1.loop_count
	jne	NextPixel

	popf
	jnc	_none_to_do

one_more_pix:	

    mov     al,[esi]                    // get texture pixel 2
    mov     [edi],al                  // store pixel 0

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

void tmapscan_pln16( int lx, int rx, int y, vertex *p, vertex *dp,  vertex * rp,uint flags )
{
	Tmap1.dest_row_data = (ubyte *)GR_SCREEN_PTR(ushort,lx,y);
	Tmap1.loop_count = rx - lx;
	Tmap1.fx_u = fl2f(p->u);
	Tmap1.fx_v = fl2f(p->v);
	Tmap1.fx_du_dx = fl2f(dp->u);
	Tmap1.fx_dv_dx = fl2f(dp->v);

	Tmap1.fx_l = fl2f(p->l*32.0); 
	Tmap1.fx_dl_dx = fl2f(dp->l*32.0);

	Tmap1.fx_u_right = fl2f(rp->u);
	Tmap1.fx_v_right = fl2f(rp->v);
	Tmap1.pixptr = (unsigned char *)tmap_bitmap->data;
	Tmap1.bp = tmap_bitmap;
	Tmap1.src_offset = tmap_bitmap->w;


	Tmap1.FixedScale = 65536.0f;
	Tmap1.FixedScale8 =	2048.0f;	//8192.0f;	// 2^16 / 8
	Tmap1.One = 1.0f;


   Tmap1.UOverZ = p->u;
	Tmap1.VOverZ = p->v;
	Tmap1.OneOverZ = p->sw;

	Tmap1.dUOverZdX8 = dp->u*32.0f;
	Tmap1.dVOverZdX8 = dp->v*32.0f;
	Tmap1.dOneOverZdX8 = dp->sw*32.0f;

	Tmap1.dUOverZdX = dp->u;
	Tmap1.dVOverZdX = dp->v;
	Tmap1.dOneOverZdX = dp->sw;

   Tmap1.RightUOverZ = rp->u;
	Tmap1.RightVOverZ = rp->v;
	Tmap1.RightOneOverZ = rp->sw;



	Tmap1.BitmapWidth = Tmap1.bp->w;
	Tmap1.BitmapHeight = Tmap1.bp->h;


	if ( Tmap1.fx_dl_dx < 0 )	{
		Tmap1.fx_dl_dx = -Tmap1.fx_dl_dx;
		Tmap1.fx_l = (67*F1_0)-Tmap1.fx_l;
		Tmap1.fx_l_right = (67*F1_0)-Tmap1.fx_l_right;
//		return;
//		Assert( Tmap1.fx_l > 31*F1_0 );
//		Assert( Tmap1.fx_l < 66*F1_0 );
//		Assert( Tmap1.fx_dl_dx >= 0 );
//		Assert( Tmap1.fx_dl_dx < 31*F1_0 );
	}

//	return;



	_asm {
	
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi


	// put the FPU in 32 bit mode
	// @todo move this out of here!

	fstcw		Tmap1.OldFPUCW					// store copy of CW
	mov		ax,Tmap1.OldFPUCW				// get it in ax
//hh	and		eax,NOT 1100000000y			// 24 bit precision
	and		eax, ~0x300L
	mov		Tmap1.FPUCW,ax					// store it
	fldcw		Tmap1.FPUCW						// load the FPU

	mov		ecx, Tmap1.loop_count		// ecx = width
	inc		ecx
	mov		edi, Tmap1.dest_row_data	// edi = dest pointer

	// edi = pointer to start pixel in dest dib
	// ecx = spanwidth

	mov		eax,ecx							// eax and ecx = width
	shr		ecx,5								// ecx = width / subdivision length
	and		eax,31								// eax = width mod subdivision length
	jnz		some_left_over					// any leftover?
//	jmp		Return
	dec		ecx								// no, so special case last span
	mov		eax,32								// it's 8 pixels long
some_left_over:
	mov		Tmap1.Subdivisions,ecx		// store widths
	mov		Tmap1.WidthModLength,eax
    
//    mov     ebx,pLeft                   ; get left edge pointer
//    mov     edx,pGradients              ; get gradients pointer

	// calculate ULeft and VLeft			// FPU Stack (ZL = ZLeft)
													// st0  st1  st2  st3  st4  st5  st6  st7
	fld		Tmap1.VOverZ					// V/ZL 
	fld		Tmap1.UOverZ					// U/ZL V/ZL 
	fld		Tmap1.OneOverZ					// 1/ZL U/ZL V/ZL 
	fld1											// 1    1/ZL U/ZL V/ZL 
	fdiv		st,st(1)							// ZL   1/ZL U/ZL V/ZL 
	fld		st									// ZL   ZL   1/ZL U/ZL V/ZL 
	fmul		st,st(4)							// VL   ZL   1/ZL U/ZL V/ZL 
	fxch		st(1)								// ZL   VL   1/ZL U/ZL V/ZL 
	fmul		st,st(3)							// UL   VL   1/ZL U/ZL V/ZL 

	fstp		st(5)								// VL   1/ZL U/ZL V/ZL UL
	fstp		st(5)								// 1/ZL U/ZL V/ZL UL   VL

	// calculate right side OverZ terms  ; st0  st1  st2  st3  st4  st5  st6  st7

	fadd		Tmap1.dOneOverZdX8			// 1/ZR U/ZL V/ZL UL   VL
	fxch		st(1)								// U/ZL 1/ZR V/ZL UL   VL
	fadd		Tmap1.dUOverZdX8				// U/ZR 1/ZR V/ZL UL   VL
	fxch		st(2)								// V/ZL 1/ZR U/ZR UL   VL
	fadd		Tmap1.dVOverZdX8				// V/ZR 1/ZR U/ZR UL   VL

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
	fmul    Tmap1.FixedScale            ; UL16 UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fistp   Tmap1.UFixed                ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

	fld     st(6)                       ; VL   UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fmul    Tmap1.FixedScale            ; VL16 UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fistp   Tmap1.VFixed                ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

	// calculate deltas                  ; st0  st1  st2  st3  st4  st5  st6  st7

	fsubr   st(5),st                    ; UR   VR   V/ZR 1/ZR U/ZR dU   VL
	fxch    st(1)                       ; VR   UR   V/ZR 1/ZR U/ZR dU   VL
	fsubr   st(6),st                    ; VR   UR   V/ZR 1/ZR U/ZR dU   dV
	fxch    st(6)                       ; dV   UR   V/ZR 1/ZR U/ZR dU   VR

	fmul    Tmap1.FixedScale8           ; dV8  UR   V/ZR 1/ZR U/ZR dU   VR
	fistp   Tmap1.DeltaV                ; UR   V/ZR 1/ZR U/ZR dU   VR

	fxch    st(4)                       ; dU   V/ZR 1/ZR U/ZR UR   VR
	fmul    Tmap1.FixedScale8           ; dU8  V/ZR 1/ZR U/ZR UR   VR
	fistp   Tmap1.DeltaU                ; V/ZR 1/ZR U/ZR UR   VR

	// increment terms for next span     ; st0  st1  st2  st3  st4  st5  st6  st7
	// Right terms become Left terms---->; V/ZL 1/ZL U/ZL UL   VL

	fadd    Tmap1.dVOverZdX8            ; V/ZR 1/ZL U/ZL UL   VL
	fxch    st(1)                       ; 1/ZL V/ZR U/ZL UL   VL
	fadd    Tmap1.dOneOverZdX8          ; 1/ZR V/ZR U/ZL UL   VL
	fxch    st(2)                       ; U/ZL V/ZR 1/ZR UL   VL
	fadd    Tmap1.dUOverZdX8            ; U/ZR V/ZR 1/ZR UL   VL
	fxch    st(2)                       ; 1/ZR V/ZR U/ZR UL   VL
	fxch    st(1)                       ; V/ZR 1/ZR U/ZR UL   VL

	; calculate right side coords       ; st0  st1  st2  st3  st4  st5  st6  st7

	fld1                                ; 1    V/ZR 1/ZR U/ZR UL   VL
	fdiv    st,st(2)                    ; ZR   V/ZR 1/ZR U/ZR UL   VL


    ; set up affine registers

    ; setup delta values
    
    mov     eax,Tmap1.DeltaV                ; get v 16.16 step
    mov     ebx,eax                     ; copy it
    sar     eax,16                      ; get v int step
    shl     ebx,16                      ; get v frac step
    mov     Tmap1.DeltaVFrac,ebx            ; store it
    imul    eax,Tmap1.src_offset      ; calculate texture step for v int step

    mov     ebx,Tmap1.DeltaU                ; get u 16.16 step
    mov     ecx,ebx                     ; copy it
    sar     ebx,16                      ; get u int step
    shl     ecx,16                      ; get u frac step
    mov     Tmap1.DeltaUFrac,ecx            ; store it
    add     eax,ebx                     ; calculate uint + vint step
    mov     Tmap1.UVintVfracStepVNoCarry,eax; save whole step in non-v-carry slot
    add     eax,Tmap1.src_offset      ; calculate whole step + v carry
    mov     Tmap1.UVintVfracStepVCarry,eax  ; save in v-carry slot


/*
; check coordinate ranges
	mov	eax, Tmap1.UFixed
	cmp	eax, Tmap1.MinUFixed
	jge	UNotTooSmall_1
	mov	eax, Tmap1.MinUFixed
	mov	Tmap1.UFixed, eax
	jmp	CheckV_1
UNotTooSmall_1:	
	cmp	eax, Tmap1.MaxUFixed
	jle	CheckV_1
	mov	eax, Tmap1.MaxUFixed
	mov	Tmap1.UFixed, eax
CheckV_1:
	mov	eax, Tmap1.VFixed
	cmp	eax, Tmap1.MinVFixed
	jge	VNotTooSmall_1
	mov	eax, Tmap1.MinVFixed
	mov	Tmap1.VFixed, eax
	jmp	DoneCheck_1
VNotTooSmall_1:	
	cmp	eax, Tmap1.MaxVFixed
	jle	DoneCheck_1
	mov	eax, Tmap1.MaxVFixed
	mov	Tmap1.VFixed, eax
DoneCheck_1:
*/
    
; setup initial coordinates
    mov     esi,Tmap1.UFixed                ; get u 16.16 fixedpoint coordinate
   
    mov     ebx,esi                     ; copy it
    sar     esi,16                      ; get integer part
    shl     ebx,16                      ; get fractional part
    
    mov     ecx,Tmap1.VFixed                ; get v 16.16 fixedpoint coordinate
   
    mov     edx,ecx                     ; copy it
    sar     edx,16                      ; get integer part
    shl     ecx,16                      ; get fractional part
    imul    edx,Tmap1.src_offset      ; calc texture scanline address
    add     esi,edx                     ; calc texture offset
    add     esi,Tmap1.pixptr          ; calc address

    mov     edx,Tmap1.DeltaUFrac            ; get register copy

 	mov	eax, Tmap1.fx_l
	shr	eax, 8
	mov	bx, ax

	mov	ebp, Tmap1.fx_dl_dx
	shl	ebp, 5	//*32
	add	Tmap1.fx_l, ebp

	mov	ebp, Tmap1.fx_l
	shr	ebp, 8
	sub	bp, ax
	shr	bp, 5

	mov	dx, bp

//	add	Tmap1.fx_l, eax


//	mov	eax, Tmap1.fx_l	// use bx and dx to do lighting
//mov eax, 31*256
//	mov	bx, ax
//	mov	eax, Tmap1.fx_dl_dx	// use bx and dx to do lighting
//mov eax, 0
//	mov	dx, ax



    ; ************** Can't Access Stack Frame ******************
    ; ************** Can't Access Stack Frame ******************
    ; ************** Can't Access Stack Frame ******************

    // 8 pixel span code
    // edi = dest dib bits at current pixel
    // esi = texture pointer at current u,v
    // eax = scratch
    // ebx = u fraction 0.32
    // ecx = v fraction 0.32
    // edx = u frac step
    // ebp = v carry scratch

    mov     al,[edi]                    // preread the destination cache line


    mov     al,[esi]                    // get texture pixel 0
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]

    add     ecx,Tmap1.DeltaVFrac            // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
//	mov al, 0	// Uncomment this line to show divisions
    mov     [edi+0],ax                  // store pixel 0

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 1
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+2],ax                  // store pixel 1

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 2
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+4],ax                  // store pixel 2

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 3
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+6],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+8],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+10],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+12],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+14],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+16],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+18],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+20],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+22],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+24],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+26],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+28],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+30],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+32],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+34],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+36],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+38],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+40],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+42],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+44],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+46],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+48],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+50],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction



    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+52],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+54],ax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 4
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+56],ax                  // store pixel 4

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 5
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+58],ax                  // store pixel 5

    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 6
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+60],ax                  // store pixel 6

    add     ebx,edx                     // increment u fraction

    mov     al,[esi]                    // get texture pixel 7
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries

    mov     [edi+62],ax                 // store pixel 7


    
    ; ************** Okay to Access Stack Frame ****************
    ; ************** Okay to Access Stack Frame ****************
    ; ************** Okay to Access Stack Frame ****************


    ; the fdiv is done, finish right    ; st0  st1  st2  st3  st4  st5  st6  st7
                                        ; ZR   V/ZR 1/ZR U/ZR UL   VL

    fld     st                          ; ZR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(2)                    ; VR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fxch    st(1)                       ; ZR   VR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(4)                    ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

    add     edi,64                       ; increment to next span
    dec     Tmap1.Subdivisions              ; decrement span count
    jnz     SpanLoop                    ; loop back

	// save new lighting values
//	xor	eax, eax
//	mov	ax, bx
//	mov	Tmap1.fx_l, eax

//	xor	eax, eax
//	mov	ax, dx
//	mov	Tmap1.fx_dl_dx, eax

HandleLeftoverPixels:
//	jmp	FPUReturn

    mov     esi,Tmap1.pixptr          ; load texture pointer

    ; edi = dest dib bits
    ; esi = current texture dib bits
    ; at this point the FPU contains    ; st0  st1  st2  st3  st4  st5  st6  st7
    ; inv. means invalid numbers        ; inv. inv. inv. inv. inv. UL   VL

    cmp     Tmap1.WidthModLength,0          ; are there remaining pixels to draw?
    jz      FPUReturn                   ; nope, pop the FPU and bail

    ; convert left side coords          ; st0  st1  st2  st3  st4  st5  st6  st7

    fld     st(5)                       ; UL   inv. inv. inv. inv. inv. UL   VL
    fmul    Tmap1.FixedScale                ; UL16 inv. inv. inv. inv. inv. UL   VL
    fistp   Tmap1.UFixed                    ; inv. inv. inv. inv. inv. UL   VL

    fld     st(6)                       ; VL   inv. inv. inv. inv. inv. UL   VL
    fmul    Tmap1.FixedScale                // VL16 inv. inv. inv. inv. inv. UL   VL
    fistp   Tmap1.VFixed                    ; inv. inv. inv. inv. inv. UL   VL

    dec     Tmap1.WidthModLength            ; calc how many steps to take
    jz      OnePixelSpan                ; just one, don't do deltas

    ; calculate right edge coordinates  ; st0  st1  st2  st3  st4  st5  st6  st7
    ; r -> R+1

    ; @todo rearrange things so we don't need these two instructions
    fstp    Tmap1.FloatTemp                 ; inv. inv. inv. inv. UL   VL
    fstp    Tmap1.FloatTemp                 ; inv. inv. inv. UL   VL

    fld     Tmap1.RightVOverZ           ; V/Zr inv. inv. inv. UL   VL
    fsub    Tmap1.dVOverZdX             ; V/ZR inv. inv. inv. UL   VL
    fld     Tmap1.RightUOverZ           ; U/Zr V/ZR inv. inv. inv. UL   VL
    fsub    Tmap1.dUOverZdX             ; U/ZR V/ZR inv. inv. inv. UL   VL
    fld     Tmap1.RightOneOverZ              ; 1/Zr U/ZR V/ZR inv. inv. inv. UL   VL
    fsub    Tmap1.dOneOverZdX           ; 1/ZR U/ZR V/ZR inv. inv. inv. UL   VL

    fdivr   Tmap1.One                       ; ZR   U/ZR V/ZR inv. inv. inv. UL   VL

    fmul    st(1),st                    ; ZR   UR   V/ZR inv. inv. inv. UL   VL
    fmulp   st(2),st                    ; UR   VR   inv. inv. inv. UL   VL

    ; calculate deltas                  ; st0  st1  st2  st3  st4  st5  st6  st7

    fsubr   st(5),st                    ; UR   VR   inv. inv. inv. dU   VL
    fxch    st(1)                       ; VR   UR   inv. inv. inv. dU   VL
    fsubr   st(6),st                    ; VR   UR   inv. inv. inv. dU   dV
    fxch    st(6)                       ; dV   UR   inv. inv. inv. dU   VR

    fidiv   Tmap1.WidthModLength            ; dv   UR   inv. inv. inv. dU   VR
    fmul    Tmap1.FixedScale                ; dv16 UR   inv. inv. inv. dU   VR
    fistp   Tmap1.DeltaV                    ; UR   inv. inv. inv. dU   VR

    fxch    st(4)                       ; dU   inv. inv. inv. UR   VR
    fidiv   Tmap1.WidthModLength            ; du   inv. inv. inv. UR   VR
    fmul    Tmap1.FixedScale                ; du16 inv. inv. inv. UR   VR
    fistp   Tmap1.DeltaU                    ; inv. inv. inv. UR   VR

    ; @todo gross!  these are to line up with the other loop
    fld     st(1)                       ; inv. inv. inv. inv. UR   VR
    fld     st(2)                       ; inv. inv. inv. inv. inv. UR   VR

//jmp OldWay


	; setup delta values
	mov	eax, Tmap1.DeltaV	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap1.DeltaVFrac, ebx	// store it
	imul	eax, Tmap1.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap1.DeltaU			// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap1.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step
	mov	Tmap1.UVintVfracStepVNoCarry, eax	// save whole step in non-v-carry slot
	add	eax, Tmap1.src_offset				// calc whole step + v carry
	mov	Tmap1.UVintVfracStepVCarry, eax	// save in v-carry slot



OnePixelSpan:

/*
; check coordinate ranges
	mov	eax, Tmap1.UFixed
	cmp	eax, Tmap1.MinUFixed
	jge	UNotTooSmall_2
	mov	eax, Tmap1.MinUFixed
	mov	Tmap1.UFixed, eax
	jmp	CheckV_2
UNotTooSmall_2:	
	cmp	eax, Tmap1.MaxUFixed
	jle	CheckV_2
	mov	eax, Tmap1.MaxUFixed
	mov	Tmap1.UFixed, eax
CheckV_2:
	mov	eax, Tmap1.VFixed
	cmp	eax, Tmap1.MinVFixed
	jge	VNotTooSmall_2
	mov	eax, Tmap1.MinVFixed
	mov	Tmap1.VFixed, eax
	jmp	DoneCheck_2
VNotTooSmall_2:	
	cmp	eax, Tmap1.MaxVFixed
	jle	DoneCheck_2
	mov	eax, Tmap1.MaxVFixed
	mov	Tmap1.VFixed, eax
DoneCheck_2:
*/




	; setup initial coordinates
	mov	esi, Tmap1.UFixed			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap1.VFixed			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap1.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap1.pixptr			// calc address
	
	; set edi = address of first pixel to modify
;	mov	edi, Tmap1.dest_row_data




	mov	eax, Tmap1.fx_l
	shr	eax, 8
	mov	bx, ax

	mov	edx, Tmap1.DeltaUFrac

	cmp	Tmap1.WidthModLength, 1
	jle	NoDeltaLight

	push	ebx
	
	mov	ebx, Tmap1.fx_l_right
	shr	ebx, 8
	
	sub	ebx, eax
	mov	eax, ebx
	
#if 0
	// slow but maybe better
	push	edx
	cdq
	mov	ebx, Tmap1.WidthModLength 
	dec	ebx
	idiv	ebx
	pop	edx
#else
	mov	eax, Tmap1.fx_dl_dx
	shr	eax, 8
#endif

	mov	dx, ax

	pop	ebx

NoDeltaLight:

	inc	Tmap1.WidthModLength
	mov	eax,Tmap1.WidthModLength
	shr	eax, 1
	jz		one_more_pix
	pushf
	mov	Tmap1.WidthModLength, eax

	xor	eax, eax

    mov     al,[edi]                    // preread the destination cache line

NextPixel:
    mov     al,[esi]                    // get texture pixel 0
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]

    add     ecx,Tmap1.DeltaVFrac            // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    mov     [edi+0],ax                  // store pixel 0

    add     ecx,Tmap1.DeltaVFrac        // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction
    mov     al,[esi]                    // get texture pixel 1
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    mov     [edi+2],ax                  // store pixel 1

	add	edi, 4
	dec	Tmap1.WidthModLength
	jg		NextPixel

	popf
	jnc	FPUReturn

one_more_pix:	

    mov     al,[esi]                    // get texture pixel 2
    mov		ah, bh
    mov		ax, gr_fade_table16[eax*2]
    mov     [edi],ax                  // store pixel 2













/* 
OldWay:		// This is 6% slower than above

    mov     ebx,Tmap1.UFixed                ; get starting coordinates
    mov     ecx,Tmap1.VFixed                ; for span
 
    ; leftover pixels loop
    ; edi = dest dib bits
    ; esi = texture dib bits

    ; ebx = u 16.16
    ; ecx = v 16.16


    mov     eax,ecx                     ; copy v
    sar     eax,16                      ; int(v)
    imul    eax,Tmap1.src_offset      ; scan offset
    mov     edx,ebx                     ; copy u
    sar     edx,16                      ; int(u)
    add     eax,edx                     ; texture offset
    mov     al,[esi+eax]                ; get source pixel
mov al, 0
    mov     [edi],al                    ; store it
    inc     edi
    add     ebx,Tmap1.DeltaU                  ; increment u coordinate
    add     ecx,Tmap1.DeltaV                  ; increment v coordinate

    dec     Tmap1.WidthModLength            ; decrement loop count
    jl     FPUReturn                ; finish up
	 

LeftoverLoop:
    mov     eax,ecx                     ; copy v
    sar     eax,16                      ; int(v)
    imul    eax,Tmap1.src_offset      ; scan offset
    mov     edx,ebx                     ; copy u
    sar     edx,16                      ; int(u)
    add     eax,edx                     ; texture offset
    mov     al,[esi+eax]                ; get source pixel
    mov     [edi],al                    ; store it
    inc     edi
    add     ebx,Tmap1.DeltaU                  ; increment u coordinate
    add     ecx,Tmap1.DeltaV                  ; increment v coordinate

    dec     Tmap1.WidthModLength            ; decrement loop count
    jge     LeftoverLoop                ; finish up
*/

FPUReturn:

    ; busy FPU registers:               ; st0  st1  st2  st3  st4  st5  st6  st7
                                        ; xxx  xxx  xxx  xxx  xxx  xxx  xxx
    ffree   st(0)
    ffree   st(1)
    ffree   st(2)
    ffree   st(3)
    ffree   st(4)
    ffree   st(5)
    ffree   st(6)

//Return:

    fldcw   Tmap1.OldFPUCW                  // restore the FPU

	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	pop	edx
	pop	ecx
	pop	eax
	}


}




void tmapscan_lnn16( int lx, int rx, int y, vertex *p, vertex *dp,  vertex * rp,uint flags )
{
	Tmap1.dest_row_data = (ubyte *)GR_SCREEN_PTR(ushort,lx,y);
	Tmap1.loop_count = rx - lx;
	Tmap1.pixptr = (unsigned char *)tmap_bitmap->data;
	Tmap1.bp = tmap_bitmap;
	Tmap1.src_offset = tmap_bitmap->w;

	Tmap1.fx_u = fl2f(p->u);
	Tmap1.fx_v = fl2f(p->v);
	Tmap1.fx_du_dx = fl2f(dp->u);
	Tmap1.fx_dv_dx = fl2f(dp->v);
	Tmap1.fx_u_right = fl2f(rp->u);
	Tmap1.fx_v_right = fl2f(rp->v);

	int end;

	end = f2i(Tmap1.fx_u);
	if ( end >= Tmap1.bp->w ) return;
	
	end = f2i(Tmap1.fx_v);
	if ( end >= Tmap1.bp->h ) return;

	end = f2i(Tmap1.fx_u_right);
	if ( end >= Tmap1.bp->w ) return;

	end = f2i(Tmap1.fx_v_right);
	if ( end >= Tmap1.bp->h ) return;


	_asm {
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi

	; setup delta values
	mov	eax, Tmap1.fx_dv_dx	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap1.DeltaVFrac, ebx	// store it
	imul	eax, Tmap1.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap1.fx_du_dx		// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap1.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step
	mov	Tmap1.UVintVfracStepVNoCarry, eax	// save whole step in non-v-carry slot
	add	eax, Tmap1.src_offset				// calc whole step + v carry
	mov	Tmap1.UVintVfracStepVCarry, eax	// save in v-carry slot

	; setup initial coordinates
	mov	esi, Tmap1.fx_u			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap1.fx_v			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap1.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap1.pixptr			// calc address
	
	; set edi = address of first pixel to modify
	mov	edi, Tmap1.dest_row_data
	
	mov	edx, Tmap1.DeltaUFrac

	mov	eax, Tmap1.loop_count
	inc	eax
	mov	Tmap1.loop_count, eax

	shr	eax, 3
	je		DoLeftOverPixels

	mov	Tmap1.num_big_steps, eax
	and	Tmap1.loop_count, 7

	xor	eax, eax

NextPixelBlock:

    // 8 pixel span code
    // edi = dest dib bits at current pixel
    // esi = texture pointer at current u,v
    // eax = scratch
    // ebx = u fraction 0.32
    // ecx = v fraction 0.32
    // edx = u frac step
    // ebp = v carry scratch

    mov     al,[edi]                    // preread the destination cache line

    movzx   eax,byte ptr [esi]                    // get texture pixel 0

    add     ecx,Tmap1.DeltaVFrac            // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
	 mov		ax, palman_8_16_xlat[eax*2]
    mov     [edi+0],ax                  // store pixel 0

    add     ebx,edx                     // increment u fraction
	  movzx   eax,byte ptr [esi]                    // get texture pixel 0

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
	 mov		ax, palman_8_16_xlat[eax*2]
    mov     [edi+2],ax                  // store pixel 0

    add     ebx,edx                     // increment u fraction
	  movzx   eax,byte ptr [esi]                    // get texture pixel 0

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
	 mov		ax, palman_8_16_xlat[eax*2]
    mov     [edi+4],ax                  // store pixel 0

    add     ebx,edx                     // increment u fraction
	  movzx   eax,byte ptr [esi]                    // get texture pixel 0

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
	 mov		ax, palman_8_16_xlat[eax*2]
    mov     [edi+6],ax                  // store pixel 0

    add     ebx,edx                     // increment u fraction
	  movzx   eax,byte ptr [esi]                    // get texture pixel 0

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
	 mov		ax, palman_8_16_xlat[eax*2]
    mov     [edi+8],ax                  // store pixel 0

    add     ebx,edx                     // increment u fraction
	  movzx   eax,byte ptr [esi]                    // get texture pixel 0

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
	 mov		ax, palman_8_16_xlat[eax*2]
    mov     [edi+10],ax                  // store pixel 0

    add     ebx,edx                     // increment u fraction
	  movzx   eax,byte ptr [esi]                    // get texture pixel 0

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
	 mov		ax, palman_8_16_xlat[eax*2]
    mov     [edi+12],ax                  // store pixel 0

    add     ebx,edx                     // increment u fraction

	  movzx   eax,byte ptr [esi]                    // get texture pixel 0

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries

	 mov		ax, palman_8_16_xlat[eax*2]
    mov     [edi+14],ax                  // store pixel 0
 
		; end
	

	add	edi, 16
	dec	Tmap1.num_big_steps
	jne	NextPixelBlock
	

DoLeftOverPixels:

	mov	eax,Tmap1.loop_count
	test	eax, -1
	jz	_none_to_do
	shr	eax, 1
	je	one_more_pix
	mov	Tmap1.loop_count, eax
	pushf

	xor	eax, eax

    mov     al,[edi]                    // preread the destination cache line
//    add     ebx,edx                     // increment u fraction

NextPixel:

	  movzx   eax,byte ptr [esi]                    // get texture pixel 0

    add     ecx,Tmap1.DeltaVFrac            // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
	 mov		ax, palman_8_16_xlat[eax*2]
    mov     [edi+0],ax                  // store pixel 0

    add     ecx,Tmap1.DeltaVFrac        // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction
	  movzx   eax,byte ptr [esi]                    // get texture pixel 0

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
	 mov		ax, palman_8_16_xlat[eax*2]
    mov     [edi+2],ax                  // store pixel 0

	add	edi, 4
	dec	Tmap1.loop_count
	jne	NextPixel

	popf
	jnc	_none_to_do

one_more_pix:	

	  movzx   eax,byte ptr [esi]                    // get texture pixel 0
	 mov		ax, palman_8_16_xlat[eax*2]
    mov     [edi],ax                  // store pixel 0

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



void tmapscan_lnn32( int lx, int rx, int y, vertex *p, vertex *dp,  vertex * rp,uint flags )
{
	Tmap1.dest_row_data = (ubyte *)GR_SCREEN_PTR(uint,lx,y);
	Tmap1.loop_count = rx - lx;
	Tmap1.pixptr = (unsigned char *)tmap_bitmap->data;
	Tmap1.bp = tmap_bitmap;
	Tmap1.src_offset = tmap_bitmap->w;

	Tmap1.fx_u = fl2f(p->u);
	Tmap1.fx_v = fl2f(p->v);
	Tmap1.fx_du_dx = fl2f(dp->u);
	Tmap1.fx_dv_dx = fl2f(dp->v);
	Tmap1.fx_u_right = fl2f(rp->u);
	Tmap1.fx_v_right = fl2f(rp->v);

	int end;

	end = f2i(Tmap1.fx_u);
	if ( end >= Tmap1.bp->w ) return;
	
	end = f2i(Tmap1.fx_v);
	if ( end >= Tmap1.bp->h ) return;

	end = f2i(Tmap1.fx_u_right);
	if ( end >= Tmap1.bp->w ) return;

	end = f2i(Tmap1.fx_v_right);
	if ( end >= Tmap1.bp->h ) return;


	_asm {
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi

	; setup delta values
	mov	eax, Tmap1.fx_dv_dx	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap1.DeltaVFrac, ebx	// store it
	imul	eax, Tmap1.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap1.fx_du_dx		// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap1.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step
	mov	Tmap1.UVintVfracStepVNoCarry, eax	// save whole step in non-v-carry slot
	add	eax, Tmap1.src_offset				// calc whole step + v carry
	mov	Tmap1.UVintVfracStepVCarry, eax	// save in v-carry slot

	; setup initial coordinates
	mov	esi, Tmap1.fx_u			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap1.fx_v			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap1.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap1.pixptr			// calc address
	
	; set edi = address of first pixel to modify
	mov	edi, Tmap1.dest_row_data
	
	mov	edx, Tmap1.DeltaUFrac

	mov	eax, Tmap1.loop_count
	inc	eax
	mov	Tmap1.loop_count, eax

	shr	eax, 3
	je		DoLeftOverPixels

	mov	Tmap1.num_big_steps, eax
	and	Tmap1.loop_count, 7

	xor	eax, eax

NextPixelBlock:

    // 8 pixel span code
    // edi = dest dib bits at current pixel
    // esi = texture pointer at current u,v
    // eax = scratch
    // ebx = u fraction 0.32
    // ecx = v fraction 0.32
    // edx = u frac step
    // ebp = v carry scratch

    mov     al,[edi]                    // preread the destination cache line

    movzx   eax,byte ptr [esi]                    // get texture pixel 0

    add     ecx,Tmap1.DeltaVFrac            // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
	 mov		eax, palman_8_32_xlat[eax*4]
    mov     [edi+0],eax                  // store pixel 0

    add     ebx,edx                     // increment u fraction
	  movzx   eax,byte ptr [esi]                    // get texture pixel 0

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
	 mov		eax, palman_8_32_xlat[eax*4]
    mov     [edi+4],eax                  // store pixel 0

    add     ebx,edx                     // increment u fraction
	  movzx   eax,byte ptr [esi]                    // get texture pixel 0

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
	 mov		eax, palman_8_32_xlat[eax*4]
    mov     [edi+8],eax                  // store pixel 0

    add     ebx,edx                     // increment u fraction
	  movzx   eax,byte ptr [esi]                    // get texture pixel 0

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
	 mov		eax, palman_8_32_xlat[eax*4]
    mov     [edi+12],eax                  // store pixel 0

    add     ebx,edx                     // increment u fraction
	  movzx   eax,byte ptr [esi]                    // get texture pixel 0

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
	 mov		eax, palman_8_32_xlat[eax*4]
    mov     [edi+16],eax                  // store pixel 0

    add     ebx,edx                     // increment u fraction
	  movzx   eax,byte ptr [esi]                    // get texture pixel 0

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
	 mov		eax, palman_8_32_xlat[eax*4]
    mov     [edi+20],eax                  // store pixel 0

    add     ebx,edx                     // increment u fraction
	  movzx   eax,byte ptr [esi]                    // get texture pixel 0

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
	 mov		eax, palman_8_32_xlat[eax*4]
    mov     [edi+24],eax                  // store pixel 0

    add     ebx,edx                     // increment u fraction

	  movzx   eax,byte ptr [esi]                    // get texture pixel 0

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries

	 mov		eax, palman_8_32_xlat[eax*4]
    mov     [edi+28],eax                  // store pixel 0
 
		; end
	

	add	edi, 32
	dec	Tmap1.num_big_steps
	jne	NextPixelBlock
	

DoLeftOverPixels:

	mov	eax,Tmap1.loop_count
	test	eax, -1
	jz	_none_to_do
	shr	eax, 1
	je	one_more_pix
	mov	Tmap1.loop_count, eax
	pushf

	xor	eax, eax

    mov     al,[edi]                    // preread the destination cache line
//    add     ebx,edx                     // increment u fraction

NextPixel:

	  movzx   eax,byte ptr [esi]                    // get texture pixel 0

    add     ecx,Tmap1.DeltaVFrac            // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
	 mov		eax, palman_8_32_xlat[eax*4]
    mov     [edi+0],eax                  // store pixel 0

    add     ecx,Tmap1.DeltaVFrac        // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction
	  movzx   eax,byte ptr [esi]                    // get texture pixel 0

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
	 mov		eax, palman_8_32_xlat[eax*4]
    mov     [edi+4],eax                  // store pixel 0

	add	edi, 8
	dec	Tmap1.loop_count
	jne	NextPixel

	popf
	jnc	_none_to_do

one_more_pix:	

	  movzx   eax,byte ptr [esi]                    // get texture pixel 0
	 mov		eax, palman_8_32_xlat[eax*4]
    mov     [edi],eax                  // store pixel 0

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


void tmapscan_pln32( int lx, int rx, int y, vertex *p, vertex *dp,  vertex * rp,uint flags )
{
	Tmap1.dest_row_data = (ubyte *)GR_SCREEN_PTR(uint,lx,y);
	Tmap1.loop_count = rx - lx;
	Tmap1.fx_u = fl2f(p->u);
	Tmap1.fx_v = fl2f(p->v);
	Tmap1.fx_du_dx = fl2f(dp->u);
	Tmap1.fx_dv_dx = fl2f(dp->v);

	Tmap1.fx_l = fl2f(p->l*32.0); 
	Tmap1.fx_dl_dx = fl2f(dp->l*32.0);

	Tmap1.fx_u_right = fl2f(rp->u);
	Tmap1.fx_v_right = fl2f(rp->v);
	Tmap1.pixptr = (unsigned char *)tmap_bitmap->data;
	Tmap1.bp = tmap_bitmap;
	Tmap1.src_offset = tmap_bitmap->w;


	Tmap1.FixedScale = 65536.0f;
	Tmap1.FixedScale8 =	2048.0f;	//8192.0f;	// 2^16 / 8
	Tmap1.One = 1.0f;


   Tmap1.UOverZ = p->u;
	Tmap1.VOverZ = p->v;
	Tmap1.OneOverZ = p->sw;

	Tmap1.dUOverZdX8 = dp->u*32.0f;
	Tmap1.dVOverZdX8 = dp->v*32.0f;
	Tmap1.dOneOverZdX8 = dp->sw*32.0f;

	Tmap1.dUOverZdX = dp->u;
	Tmap1.dVOverZdX = dp->v;
	Tmap1.dOneOverZdX = dp->sw;

   Tmap1.RightUOverZ = rp->u;
	Tmap1.RightVOverZ = rp->v;
	Tmap1.RightOneOverZ = rp->sw;


	Tmap1.BitmapWidth = Tmap1.bp->w;
	Tmap1.BitmapHeight = Tmap1.bp->h;


	if ( Tmap1.fx_dl_dx < 0 )	{
		Tmap1.fx_dl_dx = -Tmap1.fx_dl_dx;
		Tmap1.fx_l = (67*F1_0)-Tmap1.fx_l;
		Tmap1.fx_l_right = (67*F1_0)-Tmap1.fx_l_right;
//		return;
//		Assert( Tmap1.fx_l > 31*F1_0 );
//		Assert( Tmap1.fx_l < 66*F1_0 );
//		Assert( Tmap1.fx_dl_dx >= 0 );
//		Assert( Tmap1.fx_dl_dx < 31*F1_0 );
	}

//	return;



	_asm {
	
	push	eax
	push	ecx
	push	edx
	push	ebx
	push	ebp
	push	esi
	push	edi


	// put the FPU in 32 bit mode
	// @todo move this out of here!

	fstcw		Tmap1.OldFPUCW					// store copy of CW
	mov		ax,Tmap1.OldFPUCW				// get it in ax
//hh	and		eax,NOT 1100000000y			// 24 bit precision
	and		eax, ~0x300L
	mov		Tmap1.FPUCW,ax					// store it
	fldcw		Tmap1.FPUCW						// load the FPU

	mov		ecx, Tmap1.loop_count		// ecx = width
	inc		ecx
	mov		edi, Tmap1.dest_row_data	// edi = dest pointer

	// edi = pointer to start pixel in dest dib
	// ecx = spanwidth

	mov		eax,ecx							// eax and ecx = width
	shr		ecx,5								// ecx = width / subdivision length
	and		eax,31								// eax = width mod subdivision length
	jnz		some_left_over					// any leftover?
//	jmp		Return
	dec		ecx								// no, so special case last span
	mov		eax,32								// it's 8 pixels long
some_left_over:
	mov		Tmap1.Subdivisions,ecx		// store widths
	mov		Tmap1.WidthModLength,eax
    
//    mov     ebx,pLeft                   ; get left edge pointer
//    mov     edx,pGradients              ; get gradients pointer

	// calculate ULeft and VLeft			// FPU Stack (ZL = ZLeft)
													// st0  st1  st2  st3  st4  st5  st6  st7
	fld		Tmap1.VOverZ					// V/ZL 
	fld		Tmap1.UOverZ					// U/ZL V/ZL 
	fld		Tmap1.OneOverZ					// 1/ZL U/ZL V/ZL 
	fld1											// 1    1/ZL U/ZL V/ZL 
	fdiv		st,st(1)							// ZL   1/ZL U/ZL V/ZL 
	fld		st									// ZL   ZL   1/ZL U/ZL V/ZL 
	fmul		st,st(4)							// VL   ZL   1/ZL U/ZL V/ZL 
	fxch		st(1)								// ZL   VL   1/ZL U/ZL V/ZL 
	fmul		st,st(3)							// UL   VL   1/ZL U/ZL V/ZL 

	fstp		st(5)								// VL   1/ZL U/ZL V/ZL UL
	fstp		st(5)								// 1/ZL U/ZL V/ZL UL   VL

	// calculate right side OverZ terms  ; st0  st1  st2  st3  st4  st5  st6  st7

	fadd		Tmap1.dOneOverZdX8			// 1/ZR U/ZL V/ZL UL   VL
	fxch		st(1)								// U/ZL 1/ZR V/ZL UL   VL
	fadd		Tmap1.dUOverZdX8				// U/ZR 1/ZR V/ZL UL   VL
	fxch		st(2)								// V/ZL 1/ZR U/ZR UL   VL
	fadd		Tmap1.dVOverZdX8				// V/ZR 1/ZR U/ZR UL   VL

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
	fmul    Tmap1.FixedScale            ; UL16 UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fistp   Tmap1.UFixed                ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

	fld     st(6)                       ; VL   UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fmul    Tmap1.FixedScale            ; VL16 UR   VR   V/ZR 1/ZR U/ZR UL   VL
	fistp   Tmap1.VFixed                ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

	// calculate deltas                  ; st0  st1  st2  st3  st4  st5  st6  st7

	fsubr   st(5),st                    ; UR   VR   V/ZR 1/ZR U/ZR dU   VL
	fxch    st(1)                       ; VR   UR   V/ZR 1/ZR U/ZR dU   VL
	fsubr   st(6),st                    ; VR   UR   V/ZR 1/ZR U/ZR dU   dV
	fxch    st(6)                       ; dV   UR   V/ZR 1/ZR U/ZR dU   VR

	fmul    Tmap1.FixedScale8           ; dV8  UR   V/ZR 1/ZR U/ZR dU   VR
	fistp   Tmap1.DeltaV                ; UR   V/ZR 1/ZR U/ZR dU   VR

	fxch    st(4)                       ; dU   V/ZR 1/ZR U/ZR UR   VR
	fmul    Tmap1.FixedScale8           ; dU8  V/ZR 1/ZR U/ZR UR   VR
	fistp   Tmap1.DeltaU                ; V/ZR 1/ZR U/ZR UR   VR

	// increment terms for next span     ; st0  st1  st2  st3  st4  st5  st6  st7
	// Right terms become Left terms---->; V/ZL 1/ZL U/ZL UL   VL

	fadd    Tmap1.dVOverZdX8            ; V/ZR 1/ZL U/ZL UL   VL
	fxch    st(1)                       ; 1/ZL V/ZR U/ZL UL   VL
	fadd    Tmap1.dOneOverZdX8          ; 1/ZR V/ZR U/ZL UL   VL
	fxch    st(2)                       ; U/ZL V/ZR 1/ZR UL   VL
	fadd    Tmap1.dUOverZdX8            ; U/ZR V/ZR 1/ZR UL   VL
	fxch    st(2)                       ; 1/ZR V/ZR U/ZR UL   VL
	fxch    st(1)                       ; V/ZR 1/ZR U/ZR UL   VL

	; calculate right side coords       ; st0  st1  st2  st3  st4  st5  st6  st7

	fld1                                ; 1    V/ZR 1/ZR U/ZR UL   VL
	fdiv    st,st(2)                    ; ZR   V/ZR 1/ZR U/ZR UL   VL


    ; set up affine registers

    ; setup delta values
    
    mov     eax,Tmap1.DeltaV                ; get v 16.16 step
    mov     ebx,eax                     ; copy it
    sar     eax,16                      ; get v int step
    shl     ebx,16                      ; get v frac step
    mov     Tmap1.DeltaVFrac,ebx            ; store it
    imul    eax,Tmap1.src_offset      ; calculate texture step for v int step

    mov     ebx,Tmap1.DeltaU                ; get u 16.16 step
    mov     ecx,ebx                     ; copy it
    sar     ebx,16                      ; get u int step
    shl     ecx,16                      ; get u frac step
    mov     Tmap1.DeltaUFrac,ecx            ; store it
    add     eax,ebx                     ; calculate uint + vint step
    mov     Tmap1.UVintVfracStepVNoCarry,eax; save whole step in non-v-carry slot
    add     eax,Tmap1.src_offset      ; calculate whole step + v carry
    mov     Tmap1.UVintVfracStepVCarry,eax  ; save in v-carry slot


/*
; check coordinate ranges
	mov	eax, Tmap1.UFixed
	cmp	eax, Tmap1.MinUFixed
	jge	UNotTooSmall_1
	mov	eax, Tmap1.MinUFixed
	mov	Tmap1.UFixed, eax
	jmp	CheckV_1
UNotTooSmall_1:	
	cmp	eax, Tmap1.MaxUFixed
	jle	CheckV_1
	mov	eax, Tmap1.MaxUFixed
	mov	Tmap1.UFixed, eax
CheckV_1:
	mov	eax, Tmap1.VFixed
	cmp	eax, Tmap1.MinVFixed
	jge	VNotTooSmall_1
	mov	eax, Tmap1.MinVFixed
	mov	Tmap1.VFixed, eax
	jmp	DoneCheck_1
VNotTooSmall_1:	
	cmp	eax, Tmap1.MaxVFixed
	jle	DoneCheck_1
	mov	eax, Tmap1.MaxVFixed
	mov	Tmap1.VFixed, eax
DoneCheck_1:
*/
    
; setup initial coordinates
    mov     esi,Tmap1.UFixed                ; get u 16.16 fixedpoint coordinate
   
    mov     ebx,esi                     ; copy it
    sar     esi,16                      ; get integer part
    shl     ebx,16                      ; get fractional part
    
    mov     ecx,Tmap1.VFixed                ; get v 16.16 fixedpoint coordinate
   
    mov     edx,ecx                     ; copy it
    sar     edx,16                      ; get integer part
    shl     ecx,16                      ; get fractional part
    imul    edx,Tmap1.src_offset      ; calc texture scanline address
    add     esi,edx                     ; calc texture offset
    add     esi,Tmap1.pixptr          ; calc address

    mov     edx,Tmap1.DeltaUFrac            ; get register copy

 	mov	eax, Tmap1.fx_l
	shr	eax, 8
	mov	bx, ax

	mov	ebp, Tmap1.fx_dl_dx
	shl	ebp, 5	//*32
	add	Tmap1.fx_l, ebp

	mov	ebp, Tmap1.fx_l
	shr	ebp, 8
	sub	bp, ax
	shr	bp, 5

	mov	dx, bp

//	add	Tmap1.fx_l, eax


//	mov	eax, Tmap1.fx_l	// use bx and dx to do lighting
//mov eax, 31*256
//	mov	bx, ax
//	mov	eax, Tmap1.fx_dl_dx	// use bx and dx to do lighting
//mov eax, 0
//	mov	dx, ax



    ; ************** Can't Access Stack Frame ******************
    ; ************** Can't Access Stack Frame ******************
    ; ************** Can't Access Stack Frame ******************

    // 8 pixel span code
    // edi = dest dib bits at current pixel
    // esi = texture pointer at current u,v
    // eax = scratch
    // ebx = u fraction 0.32
    // ecx = v fraction 0.32
    // edx = u frac step
    // ebp = v carry scratch

    mov     al,[edi]                    // preread the destination cache line


    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]

    add     ecx,Tmap1.DeltaVFrac            // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
//	mov al, 0	// Uncomment this line to show divisions
    mov     [edi+0],eax                  // store pixel 0

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+4],eax                  // store pixel 1

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+8],eax                  // store pixel 2

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+12],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+16],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+20],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+24],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+28],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+32],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+36],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+40],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+44],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+48],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+52],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+56],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+60],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+64],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+68],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+72],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+76],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+80],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+84],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+88],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+92],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+96],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+100],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction



    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+104],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]


    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+108],eax                  // store pixel 3

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+112],eax                  // store pixel 4

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+116],eax                  // store pixel 5

    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    add     ecx,Tmap1.DeltaVFrac            // increment v fraction

    sbb     ebp,ebp                     // get -1 if carry
    mov     [edi+120],eax                  // store pixel 6

    add     ebx,edx                     // increment u fraction

    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries

    mov     [edi+124],eax                 // store pixel 7


    
    ; ************** Okay to Access Stack Frame ****************
    ; ************** Okay to Access Stack Frame ****************
    ; ************** Okay to Access Stack Frame ****************


    ; the fdiv is done, finish right    ; st0  st1  st2  st3  st4  st5  st6  st7
                                        ; ZR   V/ZR 1/ZR U/ZR UL   VL

    fld     st                          ; ZR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(2)                    ; VR   ZR   V/ZR 1/ZR U/ZR UL   VL
    fxch    st(1)                       ; ZR   VR   V/ZR 1/ZR U/ZR UL   VL
    fmul    st,st(4)                    ; UR   VR   V/ZR 1/ZR U/ZR UL   VL

    add     edi,128                       ; increment to next span
    dec     Tmap1.Subdivisions              ; decrement span count
    jnz     SpanLoop                    ; loop back

	// save new lighting values
//	xor	eax, eax
//	mov	ax, bx
//	mov	Tmap1.fx_l, eax

//	xor	eax, eax
//	mov	ax, dx
//	mov	Tmap1.fx_dl_dx, eax

HandleLeftoverPixels:
//	jmp	FPUReturn

    mov     esi,Tmap1.pixptr          ; load texture pointer

    ; edi = dest dib bits
    ; esi = current texture dib bits
    ; at this point the FPU contains    ; st0  st1  st2  st3  st4  st5  st6  st7
    ; inv. means invalid numbers        ; inv. inv. inv. inv. inv. UL   VL

    cmp     Tmap1.WidthModLength,0          ; are there remaining pixels to draw?
    jz      FPUReturn                   ; nope, pop the FPU and bail

    ; convert left side coords          ; st0  st1  st2  st3  st4  st5  st6  st7

    fld     st(5)                       ; UL   inv. inv. inv. inv. inv. UL   VL
    fmul    Tmap1.FixedScale                ; UL16 inv. inv. inv. inv. inv. UL   VL
    fistp   Tmap1.UFixed                    ; inv. inv. inv. inv. inv. UL   VL

    fld     st(6)                       ; VL   inv. inv. inv. inv. inv. UL   VL
    fmul    Tmap1.FixedScale                // VL16 inv. inv. inv. inv. inv. UL   VL
    fistp   Tmap1.VFixed                    ; inv. inv. inv. inv. inv. UL   VL

    dec     Tmap1.WidthModLength            ; calc how many steps to take
    jz      OnePixelSpan                ; just one, don't do deltas

    ; calculate right edge coordinates  ; st0  st1  st2  st3  st4  st5  st6  st7
    ; r -> R+1

    ; @todo rearrange things so we don't need these two instructions
    fstp    Tmap1.FloatTemp                 ; inv. inv. inv. inv. UL   VL
    fstp    Tmap1.FloatTemp                 ; inv. inv. inv. UL   VL

    fld     Tmap1.RightVOverZ           ; V/Zr inv. inv. inv. UL   VL
    fsub    Tmap1.dVOverZdX             ; V/ZR inv. inv. inv. UL   VL
    fld     Tmap1.RightUOverZ           ; U/Zr V/ZR inv. inv. inv. UL   VL
    fsub    Tmap1.dUOverZdX             ; U/ZR V/ZR inv. inv. inv. UL   VL
    fld     Tmap1.RightOneOverZ              ; 1/Zr U/ZR V/ZR inv. inv. inv. UL   VL
    fsub    Tmap1.dOneOverZdX           ; 1/ZR U/ZR V/ZR inv. inv. inv. UL   VL

    fdivr   Tmap1.One                       ; ZR   U/ZR V/ZR inv. inv. inv. UL   VL

    fmul    st(1),st                    ; ZR   UR   V/ZR inv. inv. inv. UL   VL
    fmulp   st(2),st                    ; UR   VR   inv. inv. inv. UL   VL

    ; calculate deltas                  ; st0  st1  st2  st3  st4  st5  st6  st7

    fsubr   st(5),st                    ; UR   VR   inv. inv. inv. dU   VL
    fxch    st(1)                       ; VR   UR   inv. inv. inv. dU   VL
    fsubr   st(6),st                    ; VR   UR   inv. inv. inv. dU   dV
    fxch    st(6)                       ; dV   UR   inv. inv. inv. dU   VR

    fidiv   Tmap1.WidthModLength            ; dv   UR   inv. inv. inv. dU   VR
    fmul    Tmap1.FixedScale                ; dv16 UR   inv. inv. inv. dU   VR
    fistp   Tmap1.DeltaV                    ; UR   inv. inv. inv. dU   VR

    fxch    st(4)                       ; dU   inv. inv. inv. UR   VR
    fidiv   Tmap1.WidthModLength            ; du   inv. inv. inv. UR   VR
    fmul    Tmap1.FixedScale                ; du16 inv. inv. inv. UR   VR
    fistp   Tmap1.DeltaU                    ; inv. inv. inv. UR   VR

    ; @todo gross!  these are to line up with the other loop
    fld     st(1)                       ; inv. inv. inv. inv. UR   VR
    fld     st(2)                       ; inv. inv. inv. inv. inv. UR   VR

//jmp OldWay


	; setup delta values
	mov	eax, Tmap1.DeltaV	// get v 16.16 step
	mov	ebx, eax						// copy it
	sar	eax, 16						// get v int step
	shl	ebx, 16						// get v frac step
	mov	Tmap1.DeltaVFrac, ebx	// store it
	imul	eax, Tmap1.src_offset	// calc texture step for v int step
	
	mov	ebx, Tmap1.DeltaU			// get u 16.16 step
	mov	ecx, ebx						// copy it
	sar	ebx, 16						// get the u int step
	shl	ecx, 16						// get the u frac step
	mov	Tmap1.DeltaUFrac, ecx			// store it
	add	eax, ebx						// calc uint + vint step
	mov	Tmap1.UVintVfracStepVNoCarry, eax	// save whole step in non-v-carry slot
	add	eax, Tmap1.src_offset				// calc whole step + v carry
	mov	Tmap1.UVintVfracStepVCarry, eax	// save in v-carry slot



OnePixelSpan:

/*
; check coordinate ranges
	mov	eax, Tmap1.UFixed
	cmp	eax, Tmap1.MinUFixed
	jge	UNotTooSmall_2
	mov	eax, Tmap1.MinUFixed
	mov	Tmap1.UFixed, eax
	jmp	CheckV_2
UNotTooSmall_2:	
	cmp	eax, Tmap1.MaxUFixed
	jle	CheckV_2
	mov	eax, Tmap1.MaxUFixed
	mov	Tmap1.UFixed, eax
CheckV_2:
	mov	eax, Tmap1.VFixed
	cmp	eax, Tmap1.MinVFixed
	jge	VNotTooSmall_2
	mov	eax, Tmap1.MinVFixed
	mov	Tmap1.VFixed, eax
	jmp	DoneCheck_2
VNotTooSmall_2:	
	cmp	eax, Tmap1.MaxVFixed
	jle	DoneCheck_2
	mov	eax, Tmap1.MaxVFixed
	mov	Tmap1.VFixed, eax
DoneCheck_2:
*/




	; setup initial coordinates
	mov	esi, Tmap1.UFixed			// get u 16.16
	mov	ebx, esi						// copy it
	sar	esi, 16						// get integer part
	shl	ebx, 16						// get fractional part

	mov	ecx, Tmap1.VFixed			// get v 16.16 
	mov	edx, ecx						// copy it
	sar	edx, 16						// get integer part
	shl	ecx, 16						// get fractional part
	imul	edx, Tmap1.src_offset		// calc texture scanline address
	add	esi, edx							// calc texture offset
	add	esi, Tmap1.pixptr			// calc address
	
	; set edi = address of first pixel to modify
;	mov	edi, Tmap1.dest_row_data




	mov	eax, Tmap1.fx_l
	shr	eax, 8
	mov	bx, ax

	mov	edx, Tmap1.DeltaUFrac

	cmp	Tmap1.WidthModLength, 1
	jle	NoDeltaLight

	push	ebx
	
	mov	ebx, Tmap1.fx_l_right
	shr	ebx, 8
	
	sub	ebx, eax
	mov	eax, ebx
	
#if 0
	// slow but maybe better
	push	edx
	cdq
	mov	ebx, Tmap1.WidthModLength 
	dec	ebx
	idiv	ebx
	pop	edx
#else
	mov	eax, Tmap1.fx_dl_dx
	shr	eax, 8
#endif

	mov	dx, ax

	pop	ebx

NoDeltaLight:

	inc	Tmap1.WidthModLength
	mov	eax,Tmap1.WidthModLength
	shr	eax, 1
	jz		one_more_pix
	pushf
	mov	Tmap1.WidthModLength, eax

	xor	eax, eax

    mov     al,[edi]                    // preread the destination cache line

NextPixel:
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]

    add     ecx,Tmap1.DeltaVFrac            // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction
    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    mov     [edi+0],eax                  // store pixel 0

    add     ecx,Tmap1.DeltaVFrac        // increment v fraction
    sbb     ebp,ebp                     // get -1 if carry
    add     ebx,edx                     // increment u fraction
    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]

    adc     esi,UVintVfracStep[4*ebp]  // add in step ints & carries
    mov     [edi+4],eax                  // store pixel 1

	add	edi, 8
	dec	Tmap1.WidthModLength
	jg		NextPixel

	popf
	jnc	FPUReturn

one_more_pix:	

    movzx   eax,byte ptr [esi]                    // get texture pixel 0
    mov		ah, bh
    mov		eax, gr_fade_table32[eax*4]
    mov     [edi],eax                  // store pixel 2













/* 
OldWay:		// This is 6% slower than above

    mov     ebx,Tmap1.UFixed                ; get starting coordinates
    mov     ecx,Tmap1.VFixed                ; for span
 
    ; leftover pixels loop
    ; edi = dest dib bits
    ; esi = texture dib bits

    ; ebx = u 16.16
    ; ecx = v 16.16


    mov     eax,ecx                     ; copy v
    sar     eax,16                      ; int(v)
    imul    eax,Tmap1.src_offset      ; scan offset
    mov     edx,ebx                     ; copy u
    sar     edx,16                      ; int(u)
    add     eax,edx                     ; texture offset
    mov     al,[esi+eax]                ; get source pixel
mov al, 0
    mov     [edi],al                    ; store it
    inc     edi
    add     ebx,Tmap1.DeltaU                  ; increment u coordinate
    add     ecx,Tmap1.DeltaV                  ; increment v coordinate

    dec     Tmap1.WidthModLength            ; decrement loop count
    jl     FPUReturn                ; finish up
	 

LeftoverLoop:
    mov     eax,ecx                     ; copy v
    sar     eax,16                      ; int(v)
    imul    eax,Tmap1.src_offset      ; scan offset
    mov     edx,ebx                     ; copy u
    sar     edx,16                      ; int(u)
    add     eax,edx                     ; texture offset
    mov     al,[esi+eax]                ; get source pixel
    mov     [edi],al                    ; store it
    inc     edi
    add     ebx,Tmap1.DeltaU                  ; increment u coordinate
    add     ecx,Tmap1.DeltaV                  ; increment v coordinate

    dec     Tmap1.WidthModLength            ; decrement loop count
    jge     LeftoverLoop                ; finish up
*/

FPUReturn:

    ; busy FPU registers:               ; st0  st1  st2  st3  st4  st5  st6  st7
                                        ; xxx  xxx  xxx  xxx  xxx  xxx  xxx
    ffree   st(0)
    ffree   st(1)
    ffree   st(2)
    ffree   st(3)
    ffree   st(4)
    ffree   st(5)
    ffree   st(6)

//Return:

    fldcw   Tmap1.OldFPUCW                  // restore the FPU

	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	pop	edx
	pop	ecx
	pop	eax
	}


}




add edx,DeltaVFrac        ; Add in 0.32 DeltaVFrac to VFrac
sbb ebp,ebp               ; ebp will equal -1 if there was a carry
mov BYTE PTR [edi], al    ; blit destination pixel
mov al, BYTE PTR [esi]    ; get next texel
add ecx,ebx               ; add 0.32 DeltaUFrac to UFrac, plus light
adc esi, [UVStepCarry1+(ebp*4)]
mov ah, ch                ; move lighting value into place
mov al, ShadeTable[eax]   ; Get shaded pixel










#endif
