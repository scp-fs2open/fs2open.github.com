/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrD3D.cpp $
 * $Revision: 2.1 $
 * $Date: 2002-07-30 14:29:15 $
 * $Author: unknownplayer $
 *
 * Code for our Direct3D renderer
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 42    10/13/99 3:49p Jefff
 * fixed unnumbered XSTRs
 * 
 * 41    9/13/99 11:25p Dave
 * Fixed problem with mode-switching and D3D movies.
 * 
 * 40    9/13/99 11:30a Dave
 * Added checkboxes and functionality for disabling PXO banners as well as
 * disabling d3d zbuffer biasing.
 * 
 * 39    9/10/99 11:53a Dave
 * Shutdown graphics before sound to eliminate apparent lockups when
 * Directsound decides to be lame. Fix TOPMOST problem with D3D windows.
 * 
 * 38    9/04/99 8:00p Dave
 * Fixed up 1024 and 32 bit movie support.
 * 
 * 37    8/30/99 5:01p Dave
 * Made d3d do less state changing in the nebula. Use new chat server for
 * PXO.
 * 
 * 36    8/20/99 2:09p Dave
 * PXO banner cycling.
 * 
 * 35    8/18/99 9:35a Dave
 * Made d3d shutdown more stable.
 * 
 * 34    8/11/99 3:30p Dave
 * Fixed window focus problems.
 * 
 * 33    8/04/99 5:36p Dave
 * Make glide and D3D switch out properly.
 * 
 * 32    8/02/99 6:25p Dave
 * Fixed d3d screen save/popup problem.
 * 
 * 31    7/30/99 7:01p Dave
 * Dogfight escort gauge. Fixed up laser rendering in Glide.
 * 
 * 30    7/29/99 10:47p Dave
 * Standardized D3D fogging using vertex fog. Shook out Savage 4 bugs.
 * 
 * 29    7/27/99 3:09p Dave
 * Made g400 work. Whee.
 * 
 * 28    7/24/99 4:19p Dave
 * Fixed dumb code with briefing bitmaps. Made d3d zbuffer work much
 * better. Made model code use zbuffer more intelligently.
 * 
 * 27    7/16/99 1:49p Dave
 * 8 bit aabitmaps. yay.
 * 
 * 26    7/14/99 9:42a Dave
 * Put in clear_color debug function. Put in base for 3dnow stuff / P3
 * stuff
 * 
 * 25    7/13/99 1:15p Dave
 * 32 bit support. Whee!
 * 
 * 24    6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 23    6/03/99 6:37p Dave
 * More TNT fun. Made perspective bitmaps more flexible.
 * 
 * 22    5/05/99 9:02p Dave
 * Fixed D3D aabitmap rendering. Spiffed up nebula effect a bit (added
 * rotations, tweaked values, made bitmap selection more random). Fixed
 * D3D beam weapon clipping problem. Added D3d frame dumping.
 * 
 * 21    2/03/99 11:44a Dave
 * Fixed d3d transparent textures.
 * 
 * 20    1/24/99 11:36p Dave
 * First full rev of beam weapons. Very customizable. Removed some bogus
 * Int3()'s in low level net code.
 * 
 * 19    1/15/99 11:29a Neilk
 * Fixed D3D screen/texture pixel formatting problem. 
 * 
 * 18    1/11/99 6:21p Neilk
 * Fixed broken D3D card fog-capability check.
 * 
 * 17    1/06/99 2:24p Dave
 * Stubs and release build fixes.
 * 
 * 16    12/18/98 1:13a Dave
 * Rough 1024x768 support for Direct3D. Proper detection and usage through
 * the launcher.
 * 
 * 15    12/09/98 7:34p Dave
 * Cleanup up nebula effect. Tweaked many values.
 * 
 * 14    12/08/98 7:30p Dave
 * Fixed broken compile.
 * 
 * 13    12/08/98 7:03p Dave
 * Much improved D3D fogging. Also put in vertex fogging for the cheesiest
 * of 3d cards.
 * 
 * 12    12/08/98 2:47p Johnson
 * Made D3D fogging use eye-relative depth instead of z-depth. Works like
 * Glide w-buffer now.
 * 
 * 11    12/08/98 9:36a Dave
 * Almost done nebula effect for D3D. Looks 85% as good as Glide.
 * 
 * 10    12/07/98 5:51p Dave
 * Finally got d3d fog working! Now we just need to tweak values.
 * 
 * 9     12/06/98 6:53p Dave
 * 
 * 8     12/06/98 3:08p Dave
 * Fixed grx_tmapper to handle pixel fog flag. First run fog support for
 * D3D.
 * 
 * 7     12/06/98 2:36p Dave
 * Drastically improved nebula fogging.
 * 
 * 6     12/01/98 10:25a Johnson
 * Fixed direct3d texture coord/font problems.
 * 
 * 5     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 4     11/11/98 5:37p Dave
 * Checkin for multiplayer testing.
 * 
 * 3     10/09/98 2:57p Dave
 * Starting splitting up OS stuff.
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 110   6/13/98 6:01p Hoffoss
 * Externalized all new (or forgot to be added) strings to all the code.
 * 
 * 109   6/13/98 3:18p Hoffoss
 * NOX()ed out a bunch of strings that shouldn't be translated.
 * 
 * 108   5/24/98 9:41p John
 * changed allender's previous fix to actually not draw the lines on
 * NDEBUG.
 * 
 * 107   5/24/98 9:16p Allender
 * put in previously non-NDEBUG code to draw bogus cursor when Gr_cursor
 * wasn't defined.  Caused d3d to crash before playing movies
 * 
 * 106   5/22/98 10:29p John
 * fixed some mode switching and line offset detection bugs.
 * 
 * 105   5/22/98 1:11p John
 * Added code to actually detect which offset a line needs
 * 
 *
 * $NoKeywords: $
 */

#include <math.h>

#include "grd3dinternal.h"

#include "osapi.h"
#include "2d.h"
#include "bmpman.h"
#include "key.h"
#include "floating.h"
#include "palman.h"
#include "osregistry.h"
#include "grd3d.h"
#include "line.h"
#include "font.h"
#include "grinternal.h"
#include "mouse.h"
#include "alphacolors.h"
#include "systemvars.h"
#include "cfile.h"
#include "cmdline.h"


LPDIRECTDRAW			lpDD1 = NULL;
LPDIRECTDRAW2			lpDD = NULL;
LPDIRECT3D2				lpD3D = NULL;
LPDIRECT3DDEVICE2		lpD3DDevice = NULL; 
// LPDIRECT3DDEVICE		lpD3DDeviceEB = NULL; 
LPDIRECTDRAWSURFACE	lpBackBuffer = NULL;
LPDIRECTDRAWSURFACE	lpFrontBuffer = NULL;
LPDIRECTDRAWSURFACE	lpZBuffer = NULL;

LPDIRECT3DVIEWPORT2	lpViewport;

DDPIXELFORMAT			AlphaTextureFormat;
int Largest_alpha = 0;
DDPIXELFORMAT			NonAlphaTextureFormat;
DDPIXELFORMAT			NonAlphaTextureFormat_1555;
DDPIXELFORMAT			NonAlphaTextureFormat_565;
int Largest_rgb = 0;
DDPIXELFORMAT			ScreenFormat;

static RECT D3D_cursor_clip_rect;

D3DDEVICEDESC			D3DHWDevDesc, D3DHELDevDesc;
LPD3DDEVICEDESC		lpDevDesc = NULL;

DDCAPS DD_driver_caps;
DDCAPS DD_hel_caps;

int D3D_texture_divider = 1;

int D3D_window = 0;

char Device_init_error[512] = "";

// -1 == no fog, bad bad bad
// 0 == vertex fog
// 1 == table fog
int D3D_fog_mode = -1;

static int In_frame = 0;

int D3D_inited = 0;

int DrawPrim = 0;

int D3d_rendition_uvs = 0;	

int D3D_32bit = 0;

int D3D_zbias = 1;
DCF(zbias, "")
{
	D3D_zbias = !D3D_zbias;
}

#define MAX_D2D_DEVICES 8
#define MAX_D3D_DEVICES 16

typedef struct d3d_device {
	GUID		guid_2d;
	LPGUID	pguid_2d;

	GUID		guid_3d;
	LPGUID	pguid_3d;

	char		name[1024];
} d3d_device;

d3d_device D2D_devices[MAX_D2D_DEVICES];
d3d_device D3D_devices[MAX_D3D_DEVICES];

int Num_d2d_devices = 0;
int Num_d3d_devices = 0;

d3d_device *D3D_device;

void mprint_guid( LPGUID lpGuid )
{
	/*
	if ( !lpGuid )		{
		mprintf(( "None\n" ));
	} else {				
		int i;
		for (i=0; i<sizeof(GUID); i++ )	{
			mprintf(( "%x ", *ptr++ ));
		}		
		mprintf(( "\n", *ptr++ ));
	}
	*/
}

#define PUTD3DINSTRUCTION(op, sz, cnt, ptr) do {	\
    ((LPD3DINSTRUCTION) ptr)->bOpcode = op; \
    ((LPD3DINSTRUCTION) ptr)->bSize = sz; \
    ((LPD3DINSTRUCTION) ptr)->wCount = cnt; \
    ptr = (void *)(((LPD3DINSTRUCTION) ptr) + 1); } while (0)

#define VERTEX_DATA(loc, cnt, ptr) do {	\
    if ((ptr) != (loc)) memcpy((ptr), (loc), sizeof(D3DVERTEX) * (cnt)); \
    ptr = (void *)(((LPD3DVERTEX) (ptr)) + (cnt)); } while (0)

// OP_MATRIX_MULTIPLY size: 4 (sizeof D3DINSTRUCTION)
#define OP_MATRIX_MULTIPLY(cnt, ptr) \
    PUTD3DINSTRUCTION(D3DOP_MATRIXMULTIPLY, sizeof(D3DMATRIXMULTIPLY), cnt, ptr)

// MATRIX_MULTIPLY_DATA size: 12 (sizeof MATRIXMULTIPLY)
#define MATRIX_MULTIPLY_DATA(src1, src2, dest, ptr) do {	\
    ((LPD3DMATRIXMULTIPLY) ptr)->hSrcMatrix1 = src1; \
    ((LPD3DMATRIXMULTIPLY) ptr)->hSrcMatrix2 = src2; \
    ((LPD3DMATRIXMULTIPLY) ptr)->hDestMatrix = dest; \
    ptr = (void *)(((LPD3DMATRIXMULTIPLY) ptr) + 1); } while (0)

// OP_STATE_LIGHT size: 4 (sizeof D3DINSTRUCTION)
#define OP_STATE_LIGHT(cnt, ptr) \
    PUTD3DINSTRUCTION(D3DOP_STATELIGHT, sizeof(D3DSTATE), cnt, ptr)

// OP_STATE_TRANSFORM size: 4 (sizeof D3DINSTRUCTION)
#define OP_STATE_TRANSFORM(cnt, ptr) \
    PUTD3DINSTRUCTION(D3DOP_STATETRANSFORM, sizeof(D3DSTATE), cnt, ptr)

// OP_STATE_RENDER size: 4 (sizeof D3DINSTRUCTION)
#define OP_STATE_RENDER(cnt, ptr) \
    PUTD3DINSTRUCTION(D3DOP_STATERENDER, sizeof(D3DSTATE), cnt, ptr)

// STATE_DATA size: 8 (sizeof D3DSTATE)
#define STATE_DATA(type, arg, ptr) do { \
    ((LPD3DSTATE) ptr)->drstRenderStateType = (D3DRENDERSTATETYPE)type; \
    ((LPD3DSTATE) ptr)->dwArg[0] = arg; \
    ptr = (void *)(((LPD3DSTATE) ptr) + 1); } while (0)

// OP_PROCESS_VERTICES size: 4 (sizeof D3DINSTRUCTION)
#define OP_PROCESS_VERTICES(cnt, ptr) \
    PUTD3DINSTRUCTION(D3DOP_PROCESSVERTICES, sizeof(D3DPROCESSVERTICES), cnt, ptr)

// PROCESSVERTICES_DATA size: 16 (sizeof D3DPROCESSVERTICES)
#define PROCESSVERTICES_DATA(flgs, strt, cnt, ptr) do { \
    ((LPD3DPROCESSVERTICES) ptr)->dwFlags = flgs; \
    ((LPD3DPROCESSVERTICES) ptr)->wStart = strt; \
    ((LPD3DPROCESSVERTICES) ptr)->wDest = strt; \
    ((LPD3DPROCESSVERTICES) ptr)->dwCount = cnt; \
    ((LPD3DPROCESSVERTICES) ptr)->dwReserved = 0; \
    ptr = (void *)(((LPD3DPROCESSVERTICES) ptr) + 1); } while (0)

// OP_TRIANGLE_LIST size: 4 (sizeof D3DINSTRUCTION)
#define OP_TRIANGLE_LIST(cnt, ptr) \
    PUTD3DINSTRUCTION(D3DOP_TRIANGLE, sizeof(D3DTRIANGLE), cnt, ptr)

#define TRIANGLE_LIST_DATA(loc, count, ptr) do { \
    if ((ptr) != (loc)) memcpy((ptr), (loc), sizeof(D3DTRIANGLE) * (count)); \
    ptr = (void *)(((LPD3DTRIANGLE) (ptr)) + (count)); } while (0)

// OP_LINE_LIST size: 4 (sizeof D3DINSTRUCTION)
#define OP_LINE_LIST(cnt, ptr) \
    PUTD3DINSTRUCTION(D3DOP_LINE, sizeof(D3DLINE), cnt, ptr)

#define LINE_LIST_DATA(loc, count, ptr) do { \
    if ((ptr) != (loc)) memcpy((ptr), (loc), sizeof(D3DLINE) * (count)); \
    ptr = (void *)(((LPD3DLINE) (ptr)) + (count)); } while (0)

// OP_POINT_LIST size: 8 (sizeof D3DINSTRUCTION + sizeof D3DPOINT)
#define OP_POINT_LIST(first, cnt, ptr) do { \
    PUTD3DINSTRUCTION(D3DOP_POINT, sizeof(D3DPOINT), 1, ptr); \
    ((LPD3DPOINT)(ptr))->wCount = cnt; \
    ((LPD3DPOINT)(ptr))->wFirst = first; \
    ptr = (void*)(((LPD3DPOINT)(ptr)) + 1); } while(0)

// OP_SPAN_LIST size: 8 (sizeof D3DINSTRUCTION + sizeof D3DSPAN)
#define OP_SPAN_LIST(first, cnt, ptr) \
    PUTD3DINSTRUCTION(D3DOP_SPAN, sizeof(D3DSPAN), 1, ptr); \
    ((LPD3DSPAN)(ptr))->wCount = cnt; \
    ((LPD3DSPAN)(ptr))->wFirst = first; \
    ptr = (void*)(((LPD3DSPAN)(ptr)) + 1); } while(0)

// OP_BRANCH_FORWARD size: 18 (sizeof D3DINSTRUCTION + sizeof D3DBRANCH)
#define OP_BRANCH_FORWARD(tmask, tvalue, tnegate, toffset, ptr) \
    PUTD3DINSTRUCTION(D3DOP_BRANCHFORWARD, sizeof(D3DBRANCH), 1, ptr); \
    ((LPD3DBRANCH) ptr)->dwMask = tmask; \
    ((LPD3DBRANCH) ptr)->dwValue = tvalue; \
    ((LPD3DBRANCH) ptr)->bNegate = tnegate; \
    ((LPD3DBRANCH) ptr)->dwOffset = toffset; \
    ptr = (void *)(((LPD3DBRANCH) (ptr)) + 1); } while (0)

// OP_SET_STATUS size: 20 (sizeof D3DINSTRUCTION + sizeof D3DSTATUS)
#define OP_SET_STATUS(flags, status, _x1, _y1, _x2, _y2, ptr) \
    PUTD3DINSTRUCTION(D3DOP_SETSTATUS, sizeof(D3DSTATUS), 1, ptr); \
    ((LPD3DSTATUS)(ptr))->dwFlags = flags; \
    ((LPD3DSTATUS)(ptr))->dwStatus = status; \
    ((LPD3DSTATUS)(ptr))->drExtent.x1 = _x1; \
    ((LPD3DSTATUS)(ptr))->drExtent.y1 = _y1; \
    ((LPD3DSTATUS)(ptr))->drExtent.x2 = _x2; \
    ((LPD3DSTATUS)(ptr))->drExtent.y2 = _y2; \
    ptr = (void *)(((LPD3DSTATUS) (ptr)) + 1); } while (0)

// OP_NOP size: 4
#define OP_NOP(ptr) \
    PUTD3DINSTRUCTION(D3DOP_TRIANGLE, sizeof(D3DTRIANGLE), 0, ptr)

#define OP_EXIT(ptr) \
    PUTD3DINSTRUCTION(D3DOP_EXIT, 0, 0, ptr)

#define QWORD_ALIGNED(ptr) \
    (!(0x00000007L & (ULONG)(ptr)))

#define D3D_MAX_VERTS 512
#define D3D_EXBUFFER_SIZE 65536
static int D3D_num_verts;
//static D3DTLVERTEX D3D_vertices[D3D_MAX_VERTS];
static D3DTLVERTEX *D3D_vertices;
//static ubyte D3D_exbuffer[D3D_EXBUFFER_SIZE];
static void *D3D_ex_ptr, *D3D_ex_end;
LPDIRECT3DEXECUTEBUFFER	lpExBuf = NULL;

LPVOID lpBufStart, lpPointer, lpInsStart;
int Exb_size;


void gr_d3d_exb_init()
{
	/*
	HRESULT ddrval;
	D3DEXECUTEBUFFERDESC debDesc;

	if ( DrawPrim ) {
		return;
	}
	
	Exb_size = D3D_EXBUFFER_SIZE + sizeof(D3DTLVERTEX)*D3D_MAX_VERTS;

   // create a D3DEXECUTEBUFFERDESC
	memset( &debDesc, 0, sizeof( debDesc ) );
	debDesc.dwSize       = sizeof( debDesc );
	debDesc.dwFlags      = D3DDEB_BUFSIZE;
	debDesc.dwBufferSize = Exb_size;

	// create the buffer
	ddrval = lpD3DDeviceEB->CreateExecuteBuffer( &debDesc, &lpExBuf, NULL );
	if ( ddrval != DD_OK )	{
		mprintf(( "GR_D3D_INIT: CreateExecuteBuffer failed. size=%d, '%s'\n", Exb_size, d3d_error_string(ddrval) ));
		goto D3DError;
	}

	memset( &debDesc, 0, sizeof( debDesc ) );
	debDesc.dwSize       = sizeof( debDesc );
	ddrval = lpExBuf->Lock( &debDesc );
	if ( ddrval != DD_OK )	{
		mprintf(( "Failed to lock the execute buffer!\n" ));
		goto D3DError;
	}

	lpPointer = lpBufStart = lpInsStart = debDesc.lpData;

	lpPointer = (void *)((uint)lpPointer+sizeof(D3DTLVERTEX)*D3D_MAX_VERTS);
	lpInsStart = lpPointer;

	OP_PROCESS_VERTICES( 1, lpPointer );
	PROCESSVERTICES_DATA( D3DPROCESSVERTICES_COPY, 0,  1, lpPointer );

	D3D_num_verts = 0;
	D3D_ex_ptr = lpPointer;
	D3D_ex_end = (void *)((uint)lpBufStart + Exb_size - 1024);
	D3D_vertices = (D3DTLVERTEX *)lpBufStart;
	return;


D3DError:
	// Reset everything
	if ( lpExBuf )	{
		lpExBuf->Release();
		lpExBuf = NULL;
	}
	gr_d3d_cleanup();
	exit(1);
	*/
}

HRESULT set_wbuffer_planes(LPDIRECT3DDEVICE2 lpDev, float dvWNear, float dvWFar)
{
  HRESULT res;
  D3DMATRIX matWorld;
  D3DMATRIX matView;
  D3DMATRIX matProj;

  memset(&matWorld, 0, sizeof(matWorld));
  memset(&matView, 0, sizeof(matWorld));
  memset(&matProj, 0, sizeof(matWorld));
  matWorld._11 = 1; matWorld._22 = 1; matWorld._33 = 1; matWorld._44 = 1;
  matView._11 = 1; matView._22 = 1; matView._33 = 1; matView._44 = 1;
  matProj._11 = 1; matProj._22 = 1; matProj._33 = 1; matProj._44 = 1;
  
  res = lpDev->SetTransform( D3DTRANSFORMSTATE_WORLD,      &matWorld );
  if (res) return res;
  res = lpDev->SetTransform( D3DTRANSFORMSTATE_VIEW,       &matView );
  if (res) return res;
  
  matProj._43 = 0;
  matProj._34 = 1;
  matProj._44 = dvWNear; // not used
  matProj._33 = dvWNear / (dvWFar - dvWNear) + 1;  

  res = lpDev->SetTransform( D3DTRANSFORMSTATE_PROJECTION, &matProj );
  return res;
}

DCF(wplanes, "")
{
	dc_get_arg(ARG_FLOAT);
	float n = Dc_arg_float;
	dc_get_arg(ARG_FLOAT);
	float f = Dc_arg_float;
	set_wbuffer_planes(lpD3DDevice, n, f);
}

void gr_d3d_exb_flush(int end_of_frame)
{
	/*
	HRESULT ddrval;
	D3DEXECUTEBUFFERDESC debDesc;
	D3DEXECUTEDATA d3dExData;

	if ( DrawPrim ) {
		return;
	}

	if (!lpExBuf) return;

	OP_EXIT( D3D_ex_ptr );

	lpPointer = lpInsStart;
	OP_PROCESS_VERTICES( 1, lpPointer );
	PROCESSVERTICES_DATA( D3DPROCESSVERTICES_COPY, 0,  D3D_num_verts, lpPointer );

	ddrval = lpExBuf->Unlock();
	if (ddrval != DD_OK )	{
		mprintf(( "Failed to unlock the execute buffer!\n" ));
		goto D3DError;
	}

	memset(&d3dExData, 0, sizeof(D3DEXECUTEDATA));
	d3dExData.dwSize = sizeof(D3DEXECUTEDATA);
	d3dExData.dwVertexCount = D3D_num_verts;
	d3dExData.dwInstructionOffset = (ULONG)((char*)lpInsStart - (char*)lpBufStart);
	d3dExData.dwInstructionLength = (ULONG)((char*)D3D_ex_ptr - (char*)lpInsStart);

//	if (end_of_frame==0)	{
//		mprintf(( "Flushing execute buffer in frame, %d verts, %d data size!\n", D3D_num_verts, d3dExData.dwInstructionLength ));
//	} else if (end_of_frame==2)	{ 
//		mprintf(( "Flushing execute buffer in frame, because of VRAM flush!\n" ));
//	}

	ddrval = lpExBuf->SetExecuteData(&d3dExData);
	if (ddrval != DD_OK )	{
		mprintf(( "Failed to SetExecuteData!\n" ));
		goto D3DError;
	}

	ddrval = lpD3DDeviceEB->Execute( lpExBuf, lpViewport, D3DEXECUTE_UNCLIPPED );
	if (ddrval != DD_OK )	{
		mprintf(( "Failed to Execute! nverts=%d\n", D3D_num_verts));
		mprintf(( "(%s)\n", d3d_error_string(ddrval) ));
		goto D3DError;
	}


	memset( &debDesc, 0, sizeof( debDesc ) );
	debDesc.dwSize       = sizeof( debDesc );
	ddrval = lpExBuf->Lock( &debDesc );
	if ( ddrval != DD_OK )	{
		mprintf(( "Failed to lock the execute buffer!\n" ));
		goto D3DError;
	}

	lpPointer = lpBufStart = lpInsStart = debDesc.lpData;

	lpPointer = (void *)((uint)lpPointer+sizeof(D3DTLVERTEX)*D3D_MAX_VERTS);
	lpInsStart = lpPointer;

	OP_PROCESS_VERTICES( 1, lpPointer );
	PROCESSVERTICES_DATA( D3DPROCESSVERTICES_COPY, 0,  1, lpPointer );

	D3D_num_verts = 0;
	D3D_ex_ptr = lpPointer;
	D3D_ex_end = (void *)((uint)lpBufStart + Exb_size - 1024);
	D3D_vertices = (D3DTLVERTEX *)lpBufStart;
	return;


D3DError:
	// Reset everything

	if ( lpExBuf )	{
		lpExBuf->Release();
		lpExBuf = NULL;
	}
//	gr_d3d_cleanup();
//	exit(1);
*/
}



HRESULT d3d_SetRenderState( D3DRENDERSTATETYPE dwRenderStateType,  DWORD dwRenderState )
{
	if ( DrawPrim )	{
		return lpD3DDevice->SetRenderState(dwRenderStateType, dwRenderState );
	} else {
		if ( (uint)D3D_ex_ptr > (uint)D3D_ex_end )  {
			gr_d3d_exb_flush(0);
		}
		OP_STATE_RENDER(1, D3D_ex_ptr);
		STATE_DATA(dwRenderStateType, dwRenderState, D3D_ex_ptr);
		return DD_OK;
	}

}

HRESULT d3d_DrawPrimitive( D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags )
{
	if ( DrawPrim )	{
		return lpD3DDevice->DrawPrimitive(dptPrimitiveType, dvtVertexType, lpvVertices, dwVertexCount, dwFlags );
	} else {	

		switch( dptPrimitiveType )	{

		case D3DPT_TRIANGLEFAN:	
			if ( dvtVertexType == D3DVT_TLVERTEX )	{

				D3DTLVERTEX *Verts = (D3DTLVERTEX *)lpvVertices;
				
				if ( D3D_num_verts + dwVertexCount > D3D_MAX_VERTS ) gr_d3d_exb_flush(0);
				if ( (uint)D3D_ex_ptr > (uint)D3D_ex_end )  gr_d3d_exb_flush(0);

				D3DTLVERTEX *src_v = &D3D_vertices[D3D_num_verts];

				int i;
				for (i=0; i<(int)dwVertexCount; i++ )	{
					*src_v++ = Verts[i];
				}

				// triangle data must be QWORD aligned, so we need to make sure
				// that the OP_TRIANGLE_LIST is unaligned!  Note that you MUST have
				// the braces {} around the OP_NOP since the macro in D3DMACS.H will
				// fail if you remove them.

				if ( QWORD_ALIGNED( D3D_ex_ptr ) ) {
					OP_NOP( D3D_ex_ptr );
				}

				OP_TRIANGLE_LIST( unsigned short(dwVertexCount-2), D3D_ex_ptr );

				LPD3DTRIANGLE tri = ( LPD3DTRIANGLE )D3D_ex_ptr;

				for (i=0; i<(int)dwVertexCount-2; i++ )	{
					tri->v1 = unsigned short(D3D_num_verts+0);
					tri->v2 = unsigned short(D3D_num_verts+i+1);
					tri->v3 = unsigned short(D3D_num_verts+i+2);
					tri->wFlags = D3DTRIFLAG_EDGEENABLETRIANGLE;
					tri++;
				
				}
				D3D_ex_ptr = ( LPVOID )tri;
				D3D_num_verts += (int)dwVertexCount;
			}
			break;

		case D3DPT_LINELIST:
			if ( dvtVertexType == D3DVT_TLVERTEX )	{

				D3DTLVERTEX *Verts = (D3DTLVERTEX *)lpvVertices;
				
				if ( D3D_num_verts + dwVertexCount > D3D_MAX_VERTS ) gr_d3d_exb_flush(0);
				if ( (uint)D3D_ex_ptr > (uint)D3D_ex_end )  gr_d3d_exb_flush(0);

				D3DTLVERTEX *src_v = &D3D_vertices[D3D_num_verts];

				int i;
				for (i=0; i<(int)dwVertexCount; i++ )	{
					*src_v++ = Verts[i];
				}

				// triangle data must be QWORD aligned, so we need to make sure
				// that the OP_TRIANGLE_LIST is unaligned!  Note that you MUST have
				// the braces {} around the OP_NOP since the macro in D3DMACS.H will
				// fail if you remove them.

				if ( QWORD_ALIGNED( D3D_ex_ptr ) ) {
					OP_NOP( D3D_ex_ptr );
				}

				ushort nlines = ushort(dwVertexCount/2);

				OP_LINE_LIST( nlines, D3D_ex_ptr );
	
				for (i=0; i<(int)nlines; i++ )	{
					LPD3DLINE line = (LPD3DLINE )D3D_ex_ptr;
					line->v1 = unsigned short(D3D_num_verts);
					line->v2 = unsigned short(D3D_num_verts+1);
					D3D_ex_ptr = (void *)(((LPD3DLINE)D3D_ex_ptr) + 1);
				}	

				D3D_num_verts += (int)dwVertexCount;
			}
			break;
		}
		return DD_OK;
	}
}

volatile int D3D_running = 0;
volatile int D3D_activate = 0;
volatile int D3D_deactivate = 0;

void gr_d3d_activate(int active)
{
	if ( !D3D_running )	{
		return;
	}
	mprintf(( "Direct3D activate: %d\n", active ));

	HWND hwnd = (HWND)os_get_window();
	
	if ( active  )	{
		D3D_activate++;

		if ( hwnd )	{
			ClipCursor(&D3D_cursor_clip_rect);
			ShowWindow(hwnd,SW_RESTORE);
		}
	} else {
		D3D_deactivate++;

		if ( hwnd )	{
			ClipCursor(NULL);
			ShowWindow(hwnd,SW_MINIMIZE);
		}
	}
}


void d3d_start_frame()
{
	HRESULT ddrval;

	if (!D3D_inited) return;

	if ( In_frame < 0 || In_frame > 1 )	{
		mprintf(( "Start frame error! (%d)\n", In_frame ));
		return;
	}
	if ( In_frame == 1 ) return;

//	gr_d3d_clear_surface(lpBackBuffer, 0.0f, 0.0f, 0.0f);

	In_frame++;

	ddrval = lpD3DDevice->BeginScene();
	if (ddrval != D3D_OK )	{
		//mprintf(( "Failed to begin scene!\n%s\n", d3d_error_string(ddrval) ));
		return;
	}

	
}


void d3d_stop_frame()
{
	HRESULT ddrval;

	if (!D3D_inited) return;

	if ( In_frame < 0 || In_frame > 1 )	{
		mprintf(( "Stop frame error! (%d)\n", In_frame ));
		return;
	}

	if ( In_frame == 0 ) return;

	gr_d3d_exb_flush(1);

	In_frame--;
	
	ddrval = lpD3DDevice->EndScene();
	if (ddrval != D3D_OK )	{
		//mprintf(( "Failed to end scene!\n%s\n", d3d_error_string(ddrval) ));
		return;
	}

}

void d3d_flush()	
{
	d3d_stop_frame();
	d3d_start_frame();
}

static void d3d_dump_format(DDPIXELFORMAT *pf)
{
	unsigned long m;
	int r, g, b, a;
	for (r = 0, m = pf->dwRBitMask; !(m & 1); r++, m >>= 1);
	for (r = 0; m & 1; r++, m >>= 1);
	for (g = 0, m = pf->dwGBitMask; !(m & 1); g++, m >>= 1);
	for (g = 0; m & 1; g++, m >>= 1); 
	for (b = 0, m = pf->dwBBitMask; !(m & 1); b++, m >>= 1);
	for (b = 0; m & 1; b++, m >>= 1);
	if ( pf->dwFlags & DDPF_ALPHAPIXELS ) {
		for (a = 0, m = pf->dwRGBAlphaBitMask; !(m & 1); a++, m >>= 1);
		for (a = 0; m & 1; a++, m >>= 1);
		mprintf(( "ARGB, %d:%d:%d:%d\n", a, r, g, b ));
	} else {
		a = 0;
		mprintf(( "RGB, %d:%d:%d\n", r, g, b ));
	}
}

int D3D_found_1555_tex = 0;
int D3D_found_565_tex = 0;
static HRESULT WINAPI EnumTextureFormatsCallback(LPDDSURFACEDESC lpDDSD, LPVOID lpContext)
{
	if (lpDDSD->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXEDTO8) {
		mprintf(( "Palettized to an 8 bpp palette\n" ));
	}

	if (lpDDSD->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) {
		mprintf(( "Palettized 8 bpp\n" ));
//		TextureFormat = *lpDDSD;
//		mprintf(( " ^-- And I'm using this one!\n" ));
	} else if (lpDDSD->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED4) {
		mprintf(( "Palettized 4 bpp\n" ));
		return DDENUMRET_OK;
	} else if (lpDDSD->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED2) {
		mprintf(( "Palettized 2 bpp\n" ));
		return DDENUMRET_OK;
	} else if (lpDDSD->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED1) {
		mprintf(( "Palettized 1 bpp\n" ));
	} else if (lpDDSD->ddpfPixelFormat.dwFlags & DDPF_ALPHA ) {
		mprintf(( "Alpha something or other\n" ));
		return DDENUMRET_OK;
	} else if (lpDDSD->ddpfPixelFormat.dwFlags & DDPF_YUV ) {
		mprintf(( "YUV?\n" ));
		return DDENUMRET_OK;
	} else if (lpDDSD->ddpfPixelFormat.dwFlags & DDPF_ZBUFFER ) {
		mprintf(( "ZBUFFER?\n" ));
		return DDENUMRET_OK;
	} else if (lpDDSD->ddpfPixelFormat.dwFlags & DDPF_COMPRESSED ) {
		mprintf(( "Compressed?\n" ));
		return DDENUMRET_OK;
	} else if (lpDDSD->ddpfPixelFormat.dwFlags & DDPF_FOURCC) {
		mprintf(( "FourCC?\n" ));
		return DDENUMRET_OK;
	} else {
		unsigned long m;
		int r, g, b, a;
		for (r = 0, m = lpDDSD->ddpfPixelFormat.dwRBitMask; !(m & 1) && (r < 32); r++, m >>= 1);
		for (r = 0; m & 1; r++, m >>= 1);
		for (g = 0, m = lpDDSD->ddpfPixelFormat.dwGBitMask; !(m & 1) && (g < 32); g++, m >>= 1);
		for (g = 0; m & 1; g++, m >>= 1); 
		for (b = 0, m = lpDDSD->ddpfPixelFormat.dwBBitMask; !(m & 1) && (b < 32); b++, m >>= 1);
		for (b = 0; m & 1; b++, m >>= 1);
		if ( lpDDSD->ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS ) {
			for (a = 0, m = lpDDSD->ddpfPixelFormat.dwRGBAlphaBitMask; !(m & 1) && (a < 32); a++, m >>= 1);
			for (a = 0; m & 1; a++, m >>= 1);
			mprintf(( "ARGB, %d:%d:%d:%d\n", a, r, g, b ));
		} else {
			a = 0;
			mprintf(( "RGB, %d:%d:%d\n", r, g, b ));
		}

		// if we found a nice 1555 texture format
		if( (r == 5) && (g == 5) && (b == 5) && (a == 1)){
			Largest_rgb = 15;
			NonAlphaTextureFormat_1555 = lpDDSD->ddpfPixelFormat;
			NonAlphaTextureFormat = lpDDSD->ddpfPixelFormat;
			D3D_found_1555_tex = 1;
		}
		// 565 textures
		else if ( (r == 5) && (g == 6) && (b == 5) ){
			NonAlphaTextureFormat_565 = lpDDSD->ddpfPixelFormat;
			D3D_found_565_tex = 1;
		} 
		// otherwise keep looking
		else if(!D3D_found_1555_tex) {
			//if ( (r==4) && (g==4) && (b==4) && (a==4) && (lpDDSD->ddpfPixelFormat.dwRGBBitCount==16) )	{
			if ( (r>0) && (g>0) && (b>0) && (lpDDSD->ddpfPixelFormat.dwRGBBitCount==16) )	{
				if ( r+g+b > Largest_rgb )	{
					Largest_rgb = r+g+b;
					NonAlphaTextureFormat = lpDDSD->ddpfPixelFormat;
				}
			}		
		}

		// HACK!!! Some 3dfx cards (Allender, Whiteside, Johnson) choose
		// ARGB=8:3:3:2 as a texture format but when you go to create a surface
		// in system RAM with this format, it fails, saying invalid surface format...
		// So I'll just force 4:4:4:4 which seems to work on all cards I've seen.
		if ( (a>0) && (a<8) && (lpDDSD->ddpfPixelFormat.dwRGBBitCount<=16) )	{
			if ( a > Largest_alpha )	{
				Largest_alpha = a;
				AlphaTextureFormat = lpDDSD->ddpfPixelFormat;
			}
		}

	}
    return DDENUMRET_OK;
}

HRESULT WINAPI gr_d3d_enum( LPGUID lpGUID,
							LPSTR lpDeviceDescription,
							LPSTR lpDeviceName,
							LPD3DDEVICEDESC lpHWDesc,
							LPD3DDEVICEDESC lpHELDesc, 
							LPVOID lpContext )
{
	int use_it = 0;
	
//	mprintf(( "Found 3d device %s: %s\n",  lpDeviceName, lpDeviceDescription ));

	if ( lpHWDesc && lpHWDesc->dwFlags != 0 )	{
		use_it = 1;
	} //else if ( lpHELDesc )	{

	
	if ( use_it )	{
		d3d_device *d2d = (d3d_device *)lpContext;
		d3d_device *d3d = (d3d_device *)&D3D_devices[Num_d3d_devices++];

		if ( lpGUID )	{
			memmove( &d3d->guid_3d, lpGUID, sizeof(GUID) );
			d3d->pguid_3d = &d3d->guid_3d;
		} else {
			memset( &d3d->guid_3d, 0, sizeof(GUID) );
			d3d->pguid_3d = NULL;
		}

		memmove( &d3d->guid_2d, &d2d->guid_2d, sizeof(GUID) );
		if ( d2d->pguid_2d )	{
			d3d->pguid_2d = &d3d->guid_2d;
		} else {
			d3d->pguid_2d = NULL;
		}

		//strcpy( d3d->name, lpDeviceName );
		//strcat( d3d->name, " - " );
		strcpy( d3d->name, NOX("Direct 3D - ") );
		strcat( d3d->name, d2d->name );
		//strcat( d3d->name, lpDeviceDescription );
	}

	return D3DENUMRET_OK;
}
 
BOOL WINAPI gr_d2d_enum( LPGUID lpGUID,
							LPSTR lpDeviceDescription,
							LPSTR lpDeviceName,
							LPVOID lpContext )
{
	d3d_device *d2d = (d3d_device *)&D2D_devices[Num_d2d_devices++];

//	mprintf(( "Found 2d device %s: %s\n",  lpDeviceName, lpDeviceDescription ));
	
	if ( lpGUID )	{
		memmove( &d2d->guid_2d, lpGUID, sizeof(GUID) );
		d2d->pguid_2d = &d2d->guid_2d;
	} else {
		memset( &d2d->guid_2d, 0, sizeof(GUID) );
		d2d->pguid_2d = NULL;
	}

	strcpy( d2d->name, lpDeviceDescription );
//	strcat( d2d->name, lpDeviceName );

	return D3DENUMRET_OK;
}

d3d_device *d3d_poll_devices()
{
	int i;
	HRESULT ddrval;

	Num_d2d_devices = 0;
	Num_d3d_devices = 0;
		
	ddrval = DirectDrawEnumerate( gr_d2d_enum, NULL );
	if ( ddrval != DD_OK ) {
		mprintf(( "GR_D3D_INIT: DirectDrawEnumerate failed.\n" ));
		goto D3DError;
	}

	for ( i=0; i<Num_d2d_devices; i++)	{
		d3d_device *d2d = (d3d_device *)&D2D_devices[i];

		ddrval = DirectDrawCreate( d2d->pguid_2d, &lpDD1, NULL );
		if ( ddrval != DD_OK ) {
			mprintf(( "GR_D3D_INIT: DirectDrawCreate failed.\n" ));
			goto D3DError;
		}

		ddrval = lpDD1->QueryInterface( IID_IDirect3D2, ( LPVOID *) &lpD3D );
		if ( ddrval != DD_OK ) {
			mprintf(( "GR_D3D_INIT: QueryInterface failed.\n" ));
			goto D3DError;
		}

		ddrval = lpD3D->EnumDevices(gr_d3d_enum, d2d );
		if ( ddrval != DD_OK )	{
			mprintf(( "WIN_DD32: D3D enum devices failed. (0x%x)\n", ddrval ));
		}

		lpD3D->Release();
		lpD3D = NULL;

		lpDD1->Release();
		lpDD1 = NULL;
	}

	for ( i=0; i<Num_d3d_devices; i++)	{
		mprintf(( "D3D Device %d: %s\n", i, D3D_devices[i].name ));
		//mprint_guid(D3D_devices[i].pguid_2d);
	}

	if ( Num_d3d_devices <= 0 )	{
		mprintf(( "No D3D device found!\n" ));
		return NULL;
	} 
	

	if ( Num_d3d_devices > 0 )	{
		//mprintf(( "More than one D3D device found!\n" ));

		char *name = os_config_read_string( NULL, "VideoCard", NULL );

		if ( name && (strlen(name)>0) )	{
			for ( i=0; i<Num_d3d_devices; i++)	{
				if ( strstr( name, D3D_devices[i].name ) )	{
					mprintf(( "Using one that matched Direct3D registry value '%s'.\n", name ));
					return &D3D_devices[i];
				}
			}
			mprintf(( "WARNING!!!! Couldn't find one that matched Direct3D registry value '%s'.\n", name ));
		} 

		mprintf(( "But no Direct3D registry key or no match, so use the last one.\n" ));

		// Use the last device.
		return &D3D_devices[Num_d3d_devices-1];
	}

	return NULL;

D3DError:
	mprintf(( "Direct3D Polling failed.\n" ));
	return NULL;
}


LPDIRECTDRAW gr_d3d_get_dd()
{
	return lpDD1;
}


LPDIRECTDRAWSURFACE gr_d3d_get_dd_surface()
{
	return lpBackBuffer;
}

void gr_d3d_clip_cursor(int active)
{
	if ( active  )	{
		ClipCursor(&D3D_cursor_clip_rect);
	} else {
		ClipCursor(NULL);
	}
}

// front buffer, backbuffer, d3d device, viewport
int gr_d3d_create_rendering_objects(int clear)
{
	HRESULT ddrval;

	{
		DDSURFACEDESC ddsd;
		DDSCAPS ddscaps;

		memset( &ddsd, 0, sizeof( ddsd ));

		ddsd.dwSize = sizeof( ddsd );

		// windowed
		if(D3D_window){		
			ddsd.dwFlags = DDSD_CAPS;
			ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;			
		}
		// fullscreen
		else {
			ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
			ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_3DDEVICE | DDSCAPS_COMPLEX;
			ddsd.dwBackBufferCount = 1;
		}

		ddrval = lpDD->CreateSurface( &ddsd, &lpFrontBuffer, NULL );
		if ( ddrval != DD_OK )	{
			// mprintf(( "GR_D3D_INIT: CreateSurface (Front) failed.\n" ));
			strcpy(Device_init_error, "CreateSurface (Front) failed.");
			return 0;
		}
		
		// create a clipper and a backbuffer in windowed mode
		if(D3D_window){
			// create a clipper for windowed mode	
			LPDIRECTDRAWCLIPPER pcClipper;
			HRESULT hr = lpDD->CreateClipper(0, &pcClipper, NULL);
			if(hr != DD_OK){
				return 0;
			}

			// Associate the clipper with our window.
			pcClipper->SetHWnd(0, (HWND)os_get_window());
			lpFrontBuffer->SetClipper(pcClipper);
			pcClipper->Release();	

			// backbuffer
			ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;				
			ddsd.dwWidth = gr_screen.max_w;
			ddsd.dwHeight = gr_screen.max_h;		
			hr = lpDD->CreateSurface(&ddsd, &lpBackBuffer, NULL);
			if(hr != DD_OK){
				strcpy(Device_init_error, "Windowed backbuffer create failed.");
				return 0;
			}
		} 
		// backbuffer is already created for us in fullscreen mode
		else {
			ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
			ddrval = lpFrontBuffer->GetAttachedSurface( &ddscaps, &lpBackBuffer );
			if ( ddrval != DD_OK )	{
				// mprintf(( "GR_D3D_INIT: GetAttachedSurface (Back) failed.\n" ));
				strcpy(Device_init_error, "GetAttachedSurface (Back) failed.");
				return 0;
			}
		}
	}

	// Create a z-buffer and attach it to the backbuffer
	{
		DDSURFACEDESC ddsd;

		memset( &ddsd, 0, sizeof( ddsd ) );
		ddsd.dwSize = sizeof(ddsd);
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_ZBUFFERBITDEPTH;
		ddsd.dwWidth = gr_screen.max_w;
		ddsd.dwHeight = gr_screen.max_h;
		ddsd.dwZBufferBitDepth = 16;

		ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;
		if (lpDD->CreateSurface(&ddsd, &lpZBuffer, NULL) != DD_OK)	{
			// mprintf(( "GR_D3D_INIT: Create Zbuffer failed.\n" ));
			strcpy(Device_init_error, "Create Zbuffer failed.");
			return 0;
		}

		if (lpBackBuffer->AddAttachedSurface(lpZBuffer) != DD_OK)	{
			// mprintf(( "GR_D3D_INIT: Attach Zbuffer failed.\n" ));
			strcpy(Device_init_error, "Attach Zbuffer failed.");
			return 0;
		}
	}	

	// blit all the buffers clear
	if(clear){
		// Clear the surface
		DDBLTFX ddBltFx;		
		ddBltFx.dwSize = sizeof(DDBLTFX);
		ddBltFx.dwFillColor = 0;	
		lpFrontBuffer->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddBltFx);		
		lpBackBuffer->Blt(NULL, NULL, NULL, DDBLT_COLORFILL, &ddBltFx);		
	}

	// Create the D3D device	
	lpD3DDevice = NULL;
   ddrval = lpD3D->CreateDevice( D3D_device->guid_3d, lpBackBuffer, &lpD3DDevice );
	if ( ddrval != DD_OK )	{
		// mprintf(( "GR_D3D_INIT: Create D3D Device2 failed. %s\n", d3d_error_string(ddrval) ));
		sprintf(Device_init_error, "Create D3D Device2 failed. %s\n", d3d_error_string(ddrval));
		return 0;
	}	

	// Create and add viewport
	ddrval = lpD3D->CreateViewport( &lpViewport, NULL );
	if ( ddrval != DD_OK )	{
		// mprintf(( "GR_D3D_INIT: CreateViewport failed.\n" ));
		strcpy(Device_init_error, "CreateViewport failed.");
		return 0;
	}

 	ddrval = lpD3DDevice->AddViewport( lpViewport );
	if ( ddrval != DD_OK )	{
		// mprintf(( "GR_D3D_INIT: AddViewport failed.\n" ));
		strcpy(Device_init_error, "CreateViewport failed.");
		return 0;
	}


	// Setup the viewport for a reasonable viewing area
	D3DVIEWPORT viewdata;
	DWORD       largest_side;

	memset( &viewdata, 0, sizeof( viewdata ) );

	// Compensate for aspect ratio	
	if ( gr_screen.max_w > gr_screen.max_h ){
		largest_side = gr_screen.max_w;
	} else {
		largest_side = gr_screen.max_h;
	}

	// this sets up W data so the fogging can work
	extern float z_mult;
	set_wbuffer_planes(lpD3DDevice, 0.0f, z_mult);

	viewdata.dwSize = sizeof( viewdata );
	viewdata.dwX = viewdata.dwY = 0;
	viewdata.dwWidth = gr_screen.max_w;
	viewdata.dwHeight = gr_screen.max_h;
	viewdata.dvScaleX = largest_side / 2.0F;
	viewdata.dvScaleY = largest_side / 2.0F;
	viewdata.dvMaxX = ( float ) ( viewdata.dwWidth / ( 2.0F * viewdata.dvScaleX ) );
	viewdata.dvMaxY = ( float ) ( viewdata.dwHeight / ( 2.0F * viewdata.dvScaleY ) );
	viewdata.dvMinZ = 0.0f;
	viewdata.dvMaxZ = 1.0f; // choose something appropriate here!

	ddrval = lpViewport->SetViewport( &viewdata );
	if ( ddrval != DD_OK )	{
		// mprintf(( "GR_D3D_INIT: SetViewport failed.\n" ));
		strcpy(Device_init_error, "SetViewport failed.");
		return 0;
	}

	ddrval = lpD3DDevice->SetCurrentViewport(lpViewport );
	if ( ddrval != DD_OK )	{
		// mprintf(( "GR_D3D_INIT: SetCurrentViewport failed.\n" ));
		strcpy(Device_init_error, "SetCurrentViewport failed.");
		return 0;
	}	

	return 1;
}

void gr_d3d_release_rendering_objects()
{
	if ( lpViewport )	{
		lpViewport->Release();
		lpViewport = NULL;
	}	
	
	if ( lpD3DDevice )	{
		lpD3DDevice->Release();
		lpD3DDevice = NULL;
	}
	
	if (lpZBuffer)	{
		lpZBuffer->Release();
		lpZBuffer = NULL;
	}

	if (lpBackBuffer)	{
		lpBackBuffer->Release();
		lpBackBuffer = NULL;
	}

	if (lpFrontBuffer)	{
		lpFrontBuffer->Release();
		lpFrontBuffer = NULL;
	}
}

void gr_d3d_set_initial_render_state()
{
	d3d_SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE );
	d3d_SetRenderState(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR );
	d3d_SetRenderState(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR );
	d3d_SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD );
	d3d_SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, TRUE );
	d3d_SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE );			
}

void gr_d3d_init_device(int screen_width, int screen_height)
{
	HRESULT ddrval;
	HWND hwnd;	

	if ( os_config_read_uint( NULL, NOX("D3DUseExecuteBuffers"), 0 ))	{
		DrawPrim = 0;
	} else {
		DrawPrim = 1;
	}

	os_suspend();
	d3d_device *dd = d3d_poll_devices();
	os_resume();
	if (!dd )	{
		// Error( LOCATION, "No Direct3D devices found!\n" );
		strcpy(Device_init_error, "No Direct3D devices found!");
		goto D3DError;
	}

	// Let things catch up....
//	Sleep(1000);
	
	hwnd = (HWND)os_get_window();
	if ( !hwnd )	{
		// mprintf(( "gr_d3d_init_device: No window handle.\n" ));
		strcpy(Device_init_error, "Could not get application window handle");
		return;
	}

	// windowed
	if(D3D_window){
		SetWindowPos(hwnd, HWND_TOP, 0, 0, gr_screen.max_w, gr_screen.max_h, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_DRAWFRAME);
		SetForegroundWindow(hwnd);
		SetActiveWindow(hwnd);	
		
		D3D_cursor_clip_rect.left = 0;
		D3D_cursor_clip_rect.top = 0;
		D3D_cursor_clip_rect.right = gr_screen.max_w-1;
		D3D_cursor_clip_rect.bottom = gr_screen.max_h-1;
	} else {
		// Prepare the window to go full screen
	#ifndef NDEBUG
		mprintf(( "Window in debugging mode... mouse clicking may cause problems!\n" ));
		SetWindowLong( hwnd, GWL_EXSTYLE, 0 );
		SetWindowLong( hwnd, GWL_STYLE, WS_POPUP );
		ShowWindow(hwnd, SW_SHOWNORMAL );
		RECT work_rect;
		SystemParametersInfo( SPI_GETWORKAREA, 0, &work_rect, 0 );
		SetWindowPos( hwnd, HWND_TOPMOST, work_rect.left, work_rect.top, gr_screen.max_w, gr_screen.max_h, 0 );	
		SetActiveWindow(hwnd);
		SetForegroundWindow(hwnd);
		D3D_cursor_clip_rect.left = work_rect.left;
		D3D_cursor_clip_rect.top = work_rect.top;
		D3D_cursor_clip_rect.right = work_rect.left + gr_screen.max_w - 1;
		D3D_cursor_clip_rect.bottom = work_rect.top + gr_screen.max_h - 1;
	#else
		SetWindowLong( hwnd, GWL_EXSTYLE, 0 );
		SetWindowLong( hwnd, GWL_STYLE, WS_POPUP );
		ShowWindow(hwnd, SW_SHOWNORMAL );
		// SetWindowPos( hwnd, HWND_TOPMOST, 0, 0, GetSystemMetrics( SM_CXSCREEN ), GetSystemMetrics( SM_CYSCREEN ), 0 );	
		SetWindowPos( hwnd, HWND_TOPMOST, 0, 0, gr_screen.max_w, gr_screen.max_h, 0 );	
		SetActiveWindow(hwnd);
		SetForegroundWindow(hwnd);
		D3D_cursor_clip_rect.left = 0;
		D3D_cursor_clip_rect.top = 0;
		D3D_cursor_clip_rect.right = gr_screen.max_w-1;
		D3D_cursor_clip_rect.bottom = gr_screen.max_h-1;
	#endif
	}

	// active d3d device
	D3D_device = dd;	

	os_suspend();
	ddrval = DirectDrawCreate( dd->pguid_2d, &lpDD1, NULL );
	os_resume();
	if ( ddrval != DD_OK ) {
		// mprintf(( "GR_D3D_INIT: DirectDrawCreate failed.\n" ));
		strcpy(Device_init_error, "DirectDrawCreate failed");
		goto D3DError;
	}

	ddrval = lpDD1->QueryInterface(IID_IDirectDraw2,  (LPVOID *)&lpDD); 
	if(ddrval != DD_OK)	{
		// mprintf(( "GR_D3D_INIT: DirectDrawCreate2 failed.\n" ));
		strcpy(Device_init_error, "DirectDrawCreate2 failed");
		goto D3DError;
	}	
 
	if(D3D_window){
		ddrval = lpDD->SetCooperativeLevel( hwnd, DDSCL_NORMAL );
	} else {
		ddrval = lpDD->SetCooperativeLevel( hwnd, DDSCL_EXCLUSIVE | DDSCL_FULLSCREEN | DDSCL_NOWINDOWCHANGES );
	}
	if ( ddrval != DD_OK )	{
		// mprintf(( "GR_D3D_INIT: SetCooperativeLevel EXCLUSIVE failed.\n" ));
		strcpy(Device_init_error, "SetCooperativeLevel EXCLUSIVE failed.");
		goto D3DError;
	}
		
	// Go to full screen!
	if(!D3D_window){
		os_suspend();
		ddrval = lpDD->SetDisplayMode( screen_width, screen_height, gr_screen.bits_per_pixel, 0, 0 );
		os_resume();
		if ( ddrval != DD_OK )	{
			// mprintf(( "GR_D3D_INIT: SetDisplayMode failed.\n" ));
			strcpy(Device_init_error, "SetDisplayMode failed.");
			goto D3DError;
		}
	}

	gr_d3d_clip_cursor(1);

	ddrval = lpDD->QueryInterface( IID_IDirect3D2, ( LPVOID *) &lpD3D );
	if ( ddrval != DD_OK ) {
		// mprintf(( "GR_D3D_INIT: QueryInterface failed.\n" ));
		strcpy(Device_init_error, "QueryInterface failed.");
		goto D3DError;
	}

	// create all surfaces here
	if(!gr_d3d_create_rendering_objects(1)){
		goto D3DError;
	}
	
	{
		extern void dd_get_shift_masks( DDSURFACEDESC *ddsd );
		DDSURFACEDESC ddsd;
		memset( &ddsd, 0, sizeof( ddsd ) );
		ddsd.dwSize = sizeof(ddsd);
		lpBackBuffer->GetSurfaceDesc(&ddsd);   
		dd_get_shift_masks( &ddsd );
		ScreenFormat = ddsd.ddpfPixelFormat;
	}

	// if we're in windowed mode, fill in bits per pixel and bytes per pixel here
	if(D3D_window){
		gr_screen.bits_per_pixel = ScreenFormat.dwRGBBitCount;
		gr_screen.bytes_per_pixel = gr_screen.bits_per_pixel / 8;
	}

	// Init values so we can choose the largest of each format
	Largest_alpha = 0;
	Largest_rgb = 0;
	D3D_found_1555_tex = 0;
	D3D_found_565_tex = 0;
	ddrval = lpD3DDevice->EnumTextureFormats(EnumTextureFormatsCallback, NULL );
	if ( ddrval != DD_OK )	{
		// mprintf(( "GR_D3D_INIT: EnumTextureFormats failed.\n" ));
		strcpy(Device_init_error, "EnumTextureFormats failed.");
		goto D3DError;
	}

	// if we found our ideal texture, set the global pixel format
	if(D3D_found_1555_tex){
		Bm_pixel_format = BM_PIXEL_FORMAT_ARGB_D3D;
	}

	if ( Largest_alpha == 0 )	{
		gr_d3d_cleanup();
		MessageBox( NULL, XSTR("Alpha channel textures",620), XSTR("Missing Features",621), MB_OK|MB_TASKMODAL|MB_SETFOREGROUND );
		exit(1);
	}
	if ( Largest_rgb == 0 )	{
		gr_d3d_cleanup();
		MessageBox( NULL, XSTR("16-bpp RGB textures",622), XSTR("Missing Features",621), MB_OK|MB_TASKMODAL|MB_SETFOREGROUND );
		exit(1);
	}

	// setup texture pixel formats
	{
		DDPIXELFORMAT *surface_desc;
		int s;	
		// RGB decoder
		unsigned long m;		
					
		// Determine the red, green and blue masks' shift and scale.
		surface_desc = &NonAlphaTextureFormat;	
		for (s = 0, m = surface_desc->dwRBitMask; !(m & 1); s++, m >>= 1);
		Gr_t_red.mask = surface_desc->dwRBitMask;
		Gr_t_red.shift = s;
		Gr_t_red.scale = 255 / (surface_desc->dwRBitMask >> s);
		for (s = 0, m = surface_desc->dwGBitMask; !(m & 1); s++, m >>= 1);
		Gr_t_green.mask = surface_desc->dwRBitMask;
		Gr_t_green.shift = s;
		Gr_t_green.scale = 255 / (surface_desc->dwGBitMask >> s);
		for (s = 0, m = surface_desc->dwBBitMask; !(m & 1); s++, m >>= 1);
		Gr_t_blue.mask = surface_desc->dwRBitMask;
		Gr_t_blue.shift = s;
		Gr_t_blue.scale = 255 / (surface_desc->dwBBitMask >> s);
		Gr_t_alpha.mask = surface_desc->dwRGBAlphaBitMask;
		if ( surface_desc->dwFlags & DDPF_ALPHAPIXELS ) {			
			for (s = 0, m = surface_desc->dwRGBAlphaBitMask; !(m & 1); s++, m >>= 1);
			Gr_t_alpha.shift = s;
			Gr_t_alpha.scale = 255 / (surface_desc->dwRGBAlphaBitMask >> s);			
		} else {
			Gr_t_alpha.shift = 0;
			Gr_t_alpha.scale = 256;
		}

		// Determine the red, green and blue masks' shift and scale.
		surface_desc = &AlphaTextureFormat;	
		for (s = 0, m = surface_desc->dwRBitMask; !(m & 1); s++, m >>= 1);
		Gr_ta_red.mask = surface_desc->dwRBitMask;
		Gr_ta_red.shift = s;
		Gr_ta_red.scale = 255 / (surface_desc->dwRBitMask >> s);
		for (s = 0, m = surface_desc->dwGBitMask; !(m & 1); s++, m >>= 1);
		Gr_ta_green.mask = surface_desc->dwRBitMask;
		Gr_ta_green.shift = s;
		Gr_ta_green.scale = 255 / (surface_desc->dwGBitMask >> s);
		for (s = 0, m = surface_desc->dwBBitMask; !(m & 1); s++, m >>= 1);
		Gr_ta_blue.mask = surface_desc->dwRBitMask;
		Gr_ta_blue.shift = s;
		Gr_ta_blue.scale = 255 / (surface_desc->dwBBitMask >> s);
		Gr_ta_alpha.mask = surface_desc->dwRGBAlphaBitMask;
		if ( surface_desc->dwFlags & DDPF_ALPHAPIXELS ) {
			for (s = 0, m = surface_desc->dwRGBAlphaBitMask; !(m & 1); s++, m >>= 1);
			Gr_ta_alpha.shift = s;
			Gr_ta_alpha.scale = 255 / (surface_desc->dwRGBAlphaBitMask >> s);
		} else {
			Gr_ta_alpha.shift = 0;
			Gr_ta_alpha.scale = 256;
		}		
	}

	mprintf(( "Alpha texture format = " ));
	d3d_dump_format(&AlphaTextureFormat);

	mprintf(( "Non-alpha texture format = " ));
	d3d_dump_format(&NonAlphaTextureFormat);

	mprintf(( "Screen format = " ));
	d3d_dump_format(&ScreenFormat);

//	key_getch();

	// gr_d3d_exb_init();

	memset( &D3DHWDevDesc, 0, sizeof( D3DHWDevDesc ) );
	D3DHWDevDesc.dwSize = sizeof(D3DHELDevDesc);

	memset( &D3DHELDevDesc, 0, sizeof( D3DHELDevDesc ) );
	D3DHELDevDesc.dwSize = sizeof(D3DHELDevDesc);

	ddrval = lpD3DDevice->GetCaps( &D3DHWDevDesc, &D3DHELDevDesc );
	if ( ddrval != DD_OK )	{
		mprintf(( "GR_D3D_INIT: 3DDevice->GetCaps failed.\n" ));
		goto D3DError;
	}
	lpDevDesc = &D3DHWDevDesc;

	lpDD->GetCaps(&DD_driver_caps,&DD_hel_caps);

	{
		int not_good = 0;

		char missing_features[128*1024];

		strcpy( missing_features, XSTR("Your video card is missing the following features required by FreeSpace:\r\n\r\n",623) );

		// fog
		if ( !(lpDevDesc->dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGVERTEX) && !(lpDevDesc->dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGTABLE)){
			strcat( missing_features, XSTR("Vertex fog or Table fog\r\n", 1499) );
			not_good++;			
		}		
				
		// Texture blending values
		if ( !(lpDevDesc->dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATE ))	{
			strcat( missing_features, XSTR("Texture blending mode = Modulate\r\n", 624) );
			not_good++;
		}

		// Source blending values
//		if ( !(lpDevDesc->dpcTriCaps.dwSrcBlendCaps & D3DPBLENDCAPS_ONE) )	{
//			strcat( missing_features, "Source blending mode = ONE\r\n" );
//			not_good++;
//		}

		if ( !(lpDevDesc->dpcTriCaps.dwSrcBlendCaps & (D3DPBLENDCAPS_SRCALPHA|D3DPBLENDCAPS_BOTHSRCALPHA)) )	{
			strcat( missing_features, XSTR("Source blending mode = SRCALPHA or BOTHSRCALPHA\r\n", 625) );
			not_good++;
		}

//		if ( !(lpDevDesc->dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_ZERO ) )	{
//			strcat( missing_features, "Destination blending mode = ZERO\r\n" );
//			not_good++;
//		}

//		if ( !(lpDevDesc->dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_ONE ) )	{
//			strcat( missing_features, "Destination blending mode = ONE\r\n" );
//			not_good++;
//		}

		// Dest blending values
		if ( !(lpDevDesc->dpcTriCaps.dwDestBlendCaps & (D3DPBLENDCAPS_INVSRCALPHA|D3DPBLENDCAPS_BOTHINVSRCALPHA)) )	{
			strcat( missing_features, XSTR("Destination blending mode = INVSRCALPHA or BOTHINVSRCALPHA\r\n",626) );
			not_good++;
		}
	
		// If card is Mystique 220, turn off modulaalpha since it doesn't work...
		if ( Largest_alpha < 4 )	{
			lpDevDesc->dpcTriCaps.dwTextureBlendCaps &= (~D3DPTBLENDCAPS_MODULATEALPHA);
		}

		if ( not_good )	{
			gr_d3d_cleanup();
			MessageBox( NULL, missing_features, XSTR("Missing Features",621), MB_OK|MB_TASKMODAL|MB_SETFOREGROUND );
			exit(1);
		}	
	}

	// fog info - for now we'll prefer table fog over vertex fog
	D3D_fog_mode = 1;	
	// if the user wants to force w-fog, maybe do it	
	if(os_config_read_uint(NULL, "ForceWFOG", 0) && (lpDevDesc->dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGTABLE)){
		D3D_fog_mode = 2;
	}	
	// if the card does not have vertex fog, but has table fog, let it go
	if(!(lpDevDesc->dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGVERTEX) && (lpDevDesc->dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_FOGTABLE)){
		D3D_fog_mode = 2;
	}

	{
		DDSCAPS ddsCaps;
		DWORD dwFree, dwTotal;

		memset(&ddsCaps,0,sizeof(ddsCaps) );
		ddsCaps.dwCaps = DDSCAPS_TEXTURE;
		HRESULT ddrval = lpDD->GetAvailableVidMem(&ddsCaps, &dwTotal, &dwFree);
		if ( ddrval != DD_OK )	{
			mprintf(( "GR_D3D_INIT: GetAvailableVidMem failed.\n" ));
			dwFree = 0;
		}

		if ( dwFree < (1024*1024) )	{
			gr_d3d_cleanup();
			MessageBox( NULL, XSTR("At least 1 MB of available video memory required.",627), XSTR("Missing Features",621), MB_OK|MB_TASKMODAL|MB_SETFOREGROUND );
			exit(1);
		}
	}

	// setup proper render state
	gr_d3d_set_initial_render_state();	

	mprintf(( "Direct3D Initialized OK!\n" ));

	D3D_inited = 1;	
	return;

D3DError:
	mprintf(( "Direct3D Initialization failed.\n" ));

	gr_d3d_cleanup();
}


int Gr_d3d_mouse_saved = 0;
int Gr_d3d_mouse_saved_x1 = 0;
int Gr_d3d_mouse_saved_y1 = 0;
int Gr_d3d_mouse_saved_x2 = 0;
int Gr_d3d_mouse_saved_y2 = 0;
int Gr_d3d_mouse_saved_w = 0;
int Gr_d3d_mouse_saved_h = 0;
#define MAX_SAVE_SIZE (32*32)
ushort Gr_d3d_mouse_saved_data[MAX_SAVE_SIZE];

// Clamps X between R1 and R2
#define CLAMP(x,r1,r2) do { if ( (x) < (r1) ) (x) = (r1); else if ((x) > (r2)) (x) = (r2); } while(0)

void gr_d3d_save_mouse_area(int x, int y, int w, int h )
{
	// bail in 32 bit
	if(D3D_32bit){
		return;
	}

	Gr_d3d_mouse_saved_x1 = x; 
	Gr_d3d_mouse_saved_y1 = y;
	Gr_d3d_mouse_saved_x2 = x+w-1;
	Gr_d3d_mouse_saved_y2 = y+h-1;
	 
	CLAMP(Gr_d3d_mouse_saved_x1, gr_screen.clip_left, gr_screen.clip_right );
	CLAMP(Gr_d3d_mouse_saved_x2, gr_screen.clip_left, gr_screen.clip_right );
	CLAMP(Gr_d3d_mouse_saved_y1, gr_screen.clip_top, gr_screen.clip_bottom );
	CLAMP(Gr_d3d_mouse_saved_y2, gr_screen.clip_top, gr_screen.clip_bottom );

	Gr_d3d_mouse_saved_w = Gr_d3d_mouse_saved_x2 - Gr_d3d_mouse_saved_x1 + 1;
	Gr_d3d_mouse_saved_h = Gr_d3d_mouse_saved_y2 - Gr_d3d_mouse_saved_y1 + 1;

	if ( Gr_d3d_mouse_saved_w < 1 ) return;
	if ( Gr_d3d_mouse_saved_h < 1 ) return;

	// Make sure we're not saving too much!
	Assert( (Gr_d3d_mouse_saved_w*Gr_d3d_mouse_saved_h) <= MAX_SAVE_SIZE );

	HRESULT ddrval;
	DDSURFACEDESC ddsd;

	memset( &ddsd, 0, sizeof( ddsd ) );
	ddsd.dwSize = sizeof( ddsd );

	ddrval = lpBackBuffer->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL );
	if ( ddrval == DD_OK )	{

		ushort *rptr;
		int short_per_row=ddsd.lPitch/2;

		rptr = (ushort *)ddsd.lpSurface;

		ushort *sptr, *dptr;

		dptr = Gr_d3d_mouse_saved_data;

		for (int i=0; i<Gr_d3d_mouse_saved_h; i++ )	{
			sptr = &rptr[(Gr_d3d_mouse_saved_y1+i)*short_per_row+Gr_d3d_mouse_saved_x1];

			for(int j=0; j<Gr_d3d_mouse_saved_w; j++ )	{
				*dptr++ = *sptr++;
			}
		}

		// Unlock it
		lpBackBuffer->Unlock( NULL );

		Gr_d3d_mouse_saved = 1;

	} else {
		mprintf(( "Couldn't get read-only lock to backbuffer for d3d mouse save\n" ));
	}

}


void gr_d3d_flip()
{
	int mx, my;
	HRESULT ddrval;	
	
	
	gr_reset_clip();	

	mouse_eval_deltas();

	Gr_d3d_mouse_saved = 0;		// assume not saved		

	if(!Gr_bitmap_poly){
		d3d_stop_frame();
	}

	if ( mouse_is_visible() )	{				
		gr_reset_clip();
		mouse_get_pos( &mx, &my );
		
		gr_d3d_save_mouse_area(mx,my,32,32);	
		if ( Gr_cursor == -1 )	{
			#ifndef NDEBUG
				gr_set_color(255,255,255);
				gr_line( mx, my, mx+7, my + 7 );
				gr_line( mx, my, mx+5, my );
				gr_line( mx, my, mx, my+5 );
			#endif
		} else {	
			gr_set_bitmap(Gr_cursor);				
			gr_bitmap( mx, my );
		}		
	} 	

	// Move stop frame before drawing cursor so cursor always appears on PowerVR.
	if(Gr_bitmap_poly){
		d3d_stop_frame();
	}

//	ddrval = lpFrontBuffer->Flip( NULL, 1 );
//	if (ddrval != DD_OK )	{
//		mprintf(( "Fullscreen flip failed!\n" ));
//		return;
//	}

	if(D3D_window){
		// blt region
		RECT rect, view_rect;
		GetClientRect( (HWND)os_get_window(), &rect);
		ClientToScreen( (HWND)os_get_window(), (POINT*)&rect.left );
		ClientToScreen( (HWND)os_get_window(), (POINT*)&rect.right );		
		view_rect.left = 0;
		view_rect.top = 0;
		view_rect.right = gr_screen.max_w - 1;
		view_rect.bottom = gr_screen.max_h - 1;

		// do the blit
		lpFrontBuffer->Blt(&rect, lpBackBuffer, &view_rect, 0, NULL );
		while(lpFrontBuffer->GetBltStatus(DDGBS_ISBLTDONE) != DD_OK){}  
	} else {
	TryFlipAgain:
		if ( lpFrontBuffer->IsLost() == DDERR_SURFACELOST )	{
			lpFrontBuffer->Restore();
		}
		if ( lpBackBuffer->IsLost() == DDERR_SURFACELOST )	{
			lpBackBuffer->Restore();
		}
		if ( lpZBuffer->IsLost() == DDERR_SURFACELOST )	{
			lpZBuffer->Restore();
		}
		ddrval = lpFrontBuffer->Flip( NULL, DDFLIP_WAIT  );
		if ( ddrval == DDERR_SURFACELOST )	{
			mprintf(( "Front surface lost... attempting to restore...\n" ));
			os_sleep(1000);	// Wait a second

			// os poll?
	#ifndef THREADED
			os_poll();
	#endif

			goto TryFlipAgain;
		} else if (ddrval != DD_OK )	{
			mprintf(( "Fullscreen flip failed!\n" ));
		}
	}

	d3d_tcache_frame();

	int cnt = D3D_activate;
	if ( cnt )	{
		D3D_activate-=cnt;
		d3d_tcache_flush();
		gr_d3d_clip_cursor(1);
	}

	cnt = D3D_deactivate; 
	if ( cnt )	{
		D3D_deactivate-=cnt;
		gr_d3d_clip_cursor(0);
	}

	d3d_start_frame();
}

void gr_d3d_flip_cleanup()
{
	d3d_stop_frame();

	if (lpFrontBuffer->Flip( NULL, 1 ) != DD_OK )	{
		mprintf(( "Flip failed!\n" ));
		return;
	}
}


void gr_d3d_flip_window(uint _hdc, int x, int y, int w, int h )
{
}

void gr_d3d_cleanup()
{
	if (!D3D_inited) return;

	d3d_tcache_cleanup();	

	// release surfaces
	gr_d3d_release_rendering_objects();

	if ( lpD3D ) {
		lpD3D->Release();
		lpD3D = NULL; 
	}

	if ( lpDD1 ) {

		HRESULT ddrval;
		HWND hwnd = (HWND)os_get_window();

		ddrval = lpDD->RestoreDisplayMode();
		if( ddrval != DD_OK )	{
			mprintf(( "WIN_DD32: RestoreDisplayMode failed (0x%x)\n", ddrval ));
		}

		ddrval = lpDD->SetCooperativeLevel( hwnd, DDSCL_NORMAL );
		if( ddrval != DD_OK )	{
			mprintf(( "WIN_DD32: SetCooperativeLevel W Failed (0x%x)\n", ddrval ));
		}

		// restore windows clipping rectangle
		ClipCursor(NULL);

//		SetWindowLong( hwnd, GWL_STYLE, WS_CAPTION | WS_SYSMENU );
//		SetWindowLong( hwnd, GWL_EXSTYLE, 0 );
//		SetWindowPos( hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW|SWP_NOMOVE|SWP_NOSIZE );

		// gr_d3d_clip_cursor(0);
		
		os_suspend();
		lpDD1->Release();
		os_resume();
		lpDD1 = NULL; 
	}

	D3D_inited = 0;
}


void gr_d3d_fade_in(int instantaneous)
{
}

void gr_d3d_fade_out(int instantaneous)
{
}



void gr_d3d_force_windowed()
{
	HWND hwnd = (HWND)os_get_window();

	// Simulate Alt+Tab
	PostMessage(hwnd,WM_SYSKEYUP, 0x9, 0xa00f0001 );
	PostMessage(hwnd,WM_SYSKEYUP, 0x12, 0xc0380001 );

	gr_d3d_clip_cursor(0);

	// Wait a second to give things a change to settle down.				
	Sleep(1000);
}

static char *Gr_saved_screen = NULL;

int gr_d3d_save_screen()
{
	gr_reset_clip();

	if ( Gr_saved_screen )	{
		mprintf(( "Screen alread saved!\n" ));
		return -1;
	}

	Gr_saved_screen = (char*)malloc( gr_screen.max_w * gr_screen.max_h * gr_screen.bytes_per_pixel );
	if (!Gr_saved_screen) {
		mprintf(( "Couldn't get memory for saved screen!\n" ));
		return -1;
	}

	HRESULT ddrval;
	DDSURFACEDESC ddsd;

	memset( &ddsd, 0, sizeof( ddsd ) );
	ddsd.dwSize = sizeof( ddsd );

	ddrval = lpFrontBuffer->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL );
	if ( ddrval != DD_OK )	{
		free(Gr_saved_screen);
		Gr_saved_screen = NULL;
		mprintf(( "Error locking surface for save_screen, %s\n", d3d_error_string(ddrval) ));
		return -1;
	} 

	// 32 bit
	if(D3D_32bit){
		uint *dptr = (uint*)ddsd.lpSurface;	
		int i;
		for (i=0; i<gr_screen.max_h; i++ )	{
			memcpy( &Gr_saved_screen[gr_screen.max_w * i * gr_screen.bytes_per_pixel], dptr, gr_screen.max_w * gr_screen.bytes_per_pixel );
		
			dptr = (uint *)((uint)dptr + ddsd.lPitch);
		}
	} else {
		ushort *dptr = (ushort *)ddsd.lpSurface;	
		int i;
		for (i=0; i<gr_screen.max_h; i++ )	{
			memcpy( &Gr_saved_screen[gr_screen.max_w * i * gr_screen.bytes_per_pixel], dptr, gr_screen.max_w * gr_screen.bytes_per_pixel );
		
			dptr = (ushort *)((uint)dptr + ddsd.lPitch);
		}
	}

	// Unlock the front buffer
	lpFrontBuffer->Unlock( NULL );
	
	if ( Gr_d3d_mouse_saved && !D3D_32bit)	{
		ushort *sptr, *dptr;

		sptr = Gr_d3d_mouse_saved_data;

		for (int i=0; i<Gr_d3d_mouse_saved_h; i++ )	{
			dptr = &((ushort*)Gr_saved_screen)[(Gr_d3d_mouse_saved_y1 + i) * gr_screen.max_w + Gr_d3d_mouse_saved_x1];

			for(int j=0; j<Gr_d3d_mouse_saved_w; j++ )	{
				*dptr++ = *sptr++;
			}
		}
	}	

	return 0;
}

void gr_d3d_restore_screen(int id)
{
	gr_reset_clip();

	if ( !Gr_saved_screen )	{
		gr_clear();
		return;
	}

	HRESULT ddrval;
	DDSURFACEDESC ddsd;

	memset( &ddsd, 0, sizeof( ddsd ) );
	ddsd.dwSize = sizeof( ddsd );

	ddrval = lpBackBuffer->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL );
	if ( ddrval != DD_OK )	{
		free(Gr_saved_screen);
		Gr_saved_screen = NULL;
		mprintf(( "Error locking surface for restore_screen, %s\n", d3d_error_string(ddrval) ));
		return;
	} 

	// restore
	if(D3D_32bit){
		uint *dptr = (uint *)ddsd.lpSurface;	
		int i;
		for (i=0; i<gr_screen.max_h; i++ )	{
			memcpy( dptr, &Gr_saved_screen[gr_screen.max_w * i * gr_screen.bytes_per_pixel], gr_screen.max_w * gr_screen.bytes_per_pixel );
		
			dptr = (uint *)((uint)dptr + ddsd.lPitch);
		}
	} else {
		ushort *dptr = (ushort *)ddsd.lpSurface;	
		int i;
		for (i=0; i<gr_screen.max_h; i++ )	{
			memcpy( dptr, &Gr_saved_screen[gr_screen.max_w * i * gr_screen.bytes_per_pixel], gr_screen.max_w * gr_screen.bytes_per_pixel );
		
			dptr = (ushort *)((uint)dptr + ddsd.lPitch);
		}
	}

	// Unlock the back buffer
	lpBackBuffer->Unlock( NULL );
}

void gr_d3d_free_screen(int id)
{
	if ( Gr_saved_screen )	{
		free( Gr_saved_screen );
		Gr_saved_screen = NULL;
	}
}

static int D3d_dump_frames = 0;
static ubyte *D3d_dump_buffer = NULL;
static int D3d_dump_frame_number = 0;
static int D3d_dump_frame_count = 0;
static int D3d_dump_frame_count_max = 0;
static int D3d_dump_frame_size = 0;
void gr_d3d_dump_frame_start(int first_frame, int frames_between_dumps)
{
	if ( D3d_dump_frames )	{
		Int3();		//  We're already dumping frames.  See John.
		return;
	}	
	D3d_dump_frames = 1;
	D3d_dump_frame_number = first_frame;
	D3d_dump_frame_count = 0;
	D3d_dump_frame_count_max = frames_between_dumps;
	D3d_dump_frame_size = gr_screen.max_w * gr_screen.max_h * 2;
	
	if ( !D3d_dump_buffer ) {
		int size = D3d_dump_frame_count_max * D3d_dump_frame_size;
		D3d_dump_buffer = (ubyte *)malloc(size);
		if ( !D3d_dump_buffer )	{
			Error(LOCATION, "Unable to malloc %d bytes for dump buffer", size );
		}
	}
}

extern int tga_compress(char *out, char *in, int bytecount);
void gr_d3d_flush_frame_dump()
{
	int i,j;
	char filename[MAX_PATH_LEN], *movie_path = ".\\";
	ubyte outrow[1024*3*4];

	if ( gr_screen.max_w > 1024)	{
		mprintf(( "Screen too wide for frame_dump\n" ));
		return;
	}

	for (i = 0; i < D3d_dump_frame_count; i++) {

		int w = gr_screen.max_w;
		int h = gr_screen.max_h;

		sprintf(filename, NOX("%sfrm%04d.tga"), movie_path, D3d_dump_frame_number );
		D3d_dump_frame_number++;

		CFILE *f = cfopen(filename, "wb");

		// Write the TGA header
		cfwrite_ubyte( 0, f );	//	IDLength;
		cfwrite_ubyte( 0, f );	//	ColorMapType;
		cfwrite_ubyte( 10, f );	//	ImageType;		// 2 = 24bpp, uncompressed, 10=24bpp rle compressed
		cfwrite_ushort( 0, f );	// CMapStart;
		cfwrite_ushort( 0, f );	//	CMapLength;
		cfwrite_ubyte( 0, f );	// CMapDepth;
		cfwrite_ushort( 0, f );	//	XOffset;
		cfwrite_ushort( 0, f );	//	YOffset;
		cfwrite_ushort( (ushort)w, f );	//	Width;
		cfwrite_ushort( (ushort)h, f );	//	Height;
		cfwrite_ubyte( 24, f );	//PixelDepth;
		cfwrite_ubyte( 0, f );	//ImageDesc;

		// Go through and write our pixels
		for (j=0;j<h;j++)	{
			ubyte *src_ptr = D3d_dump_buffer+(i*D3d_dump_frame_size)+(j*w*2);

			int len = tga_compress( (char *)outrow, (char *)src_ptr, w*sizeof(short) );

			cfwrite(outrow,len,1,f);
		}

		cfclose(f);

	}

	D3d_dump_frame_count = 0;
}

void gr_d3d_dump_frame_stop()
{

	if ( !D3d_dump_frames )	{
		Int3();		//  We're not dumping frames.  See John.
		return;
	}	

	// dump any remaining frames
	gr_d3d_flush_frame_dump();
	
	D3d_dump_frames = 0;
	if ( D3d_dump_buffer )	{
		free(D3d_dump_buffer);
		D3d_dump_buffer = NULL;
	}
}

void gr_d3d_dump_screen_hack( ushort * dst )
{
	HRESULT ddrval;
	DDSURFACEDESC ddsd;	
	LPDIRECTDRAWSURFACE surf = lpBackBuffer;
	ushort pixel;
	ubyte r, g, b;

	// don't dump in 32 bit
	if(D3D_32bit){
		return;
	}
	
	// screen format
	BM_SELECT_SCREEN_FORMAT();
	memset( &ddsd, 0, sizeof( ddsd ) );
	ddsd.dwSize = sizeof( ddsd );

	// try and lock the buffer
	ddrval = surf->Lock( NULL, &ddsd, DDLOCK_WAIT | DDLOCK_READONLY, NULL );
	if ( ddrval != DD_OK )	{
		mprintf(( "Error locking surface for get_region, %s\n", d3d_error_string(ddrval) ));
		return;
	}

	ushort *sptr;		
	ubyte *rptr = (ubyte*)ddsd.lpSurface;	

	for (int i=0; i<gr_screen.max_h; i++ )	{
		sptr = (ushort*)&rptr[(gr_screen.max_h-i-1)*ddsd.lPitch];
		
		for(int j=0; j<gr_screen.max_w; j++ )	{
			pixel = *sptr++;			

			bm_get_components((ubyte*)pixel, &r, &g, &b, NULL);

			// bash to 565, hackity hack
			pixel = 0;
			pixel |= ((r >> 3) << 11);
			pixel |= ((g >> 2) << 5);
			pixel |= (b >> 3);

			*dst++ = pixel;
		}
	}	
	
	// Unlock the buffer
	surf->Unlock( NULL );	
}

void gr_d3d_dump_frame()
{
	// A hacked function to dump the frame buffer contents
	gr_d3d_dump_screen_hack( (ushort *)(D3d_dump_buffer+(D3d_dump_frame_count*D3d_dump_frame_size)) );

	D3d_dump_frame_count++;

	if ( D3d_dump_frame_count == D3d_dump_frame_count_max ) {
		gr_d3d_flush_frame_dump();
	}
}	

uint gr_d3d_lock()
{
	return 1;
}

void gr_d3d_unlock()
{
}

void gr_d3d_fog_set(int fog_mode, int r, int g, int b, float fog_near, float fog_far)
{
	D3DCOLOR color = 0;	
	  
	Assert((r >= 0) && (r < 256));
	Assert((g >= 0) && (g < 256));
	Assert((b >= 0) && (b < 256));	

	// turning fog off
	if(fog_mode == GR_FOGMODE_NONE){
		// only change state if we need to
		if(gr_screen.current_fog_mode != fog_mode){
			d3d_SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE );		
		}
		gr_screen.current_fog_mode = fog_mode;

		// to prevent further state changes
		return;
	}

	// maybe switch fogging on
	if(gr_screen.current_fog_mode != fog_mode){		
		d3d_SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE);	

		// if we're using table fog, enable table fogging
		if(D3D_fog_mode == 2){
			d3d_SetRenderState( D3DRENDERSTATE_FOGTABLEMODE, D3DFOG_LINEAR );			
		}

		gr_screen.current_fog_mode = fog_mode;	
	}	

	// is color changing?
	if( (gr_screen.current_fog_color.red != r) || (gr_screen.current_fog_color.green != g) || (gr_screen.current_fog_color.blue != b) ){
		// store the values
		gr_d3d_init_color( &gr_screen.current_fog_color, r, g, b );

		color = RGB_MAKE(r, g, b);
		d3d_SetRenderState(D3DRENDERSTATE_FOGCOLOR, color);	
	}		

	// planes changing?
	if( (fog_near >= 0.0f) && (fog_far >= 0.0f) && ((fog_near != gr_screen.fog_near) || (fog_far != gr_screen.fog_far)) ){		
		gr_screen.fog_near = fog_near;		
		gr_screen.fog_far = fog_far;					

		// only generate a new fog table if we have to (wfog/table fog mode)
		if(D3D_fog_mode == 2){	
			d3d_SetRenderState( D3DRENDERSTATE_FOGTABLESTART, *((DWORD *)(&fog_near)));		
			d3d_SetRenderState( D3DRENDERSTATE_FOGTABLEEND, *((DWORD *)(&fog_far)));
		}				
	}	
}

void gr_d3d_set_gamma(float gamma)
{
	Gr_gamma = gamma;
	Gr_gamma_int = int(Gr_gamma*100);

	// Create the Gamma lookup table
	int i;
	for (i=0; i<256; i++ )	{
		int v = fl2i(pow(i2fl(i)/255.0f, 1.0f/Gr_gamma)*255.0f);
		if ( v > 255 ) {
			v = 255;
		} else if ( v < 0 )	{
			v = 0;
		}
		Gr_gamma_lookup[i] = v;
	}

	// Flush any existing textures
	d3d_tcache_flush();
}

void d3d_get_pixel(int x, int y, ubyte *pixel)
{
	HRESULT ddrval;
	DDSURFACEDESC ddsd;

	memset( &ddsd, 0, sizeof( ddsd ) );
	ddsd.dwSize = sizeof( ddsd );

	ddrval = lpFrontBuffer->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL );
	if ( ddrval != DD_OK )	{
		mprintf(( "Error locking surface for uv test, %s\n", d3d_error_string(ddrval) ));
		return;
	} 

	ubyte *dptr = (ubyte *)((uint)ddsd.lpSurface + y*ddsd.lPitch + (x * gr_screen.bytes_per_pixel));
	memcpy(pixel, dptr, gr_screen.bytes_per_pixel);	

	// Unlock the buffer
	lpFrontBuffer->Unlock( NULL );	
}

void gr_d3d_get_pixel(int x, int y, int *r, int *g, int *b)
{
}

// resolution checking
int gr_d3d_supports_res_interface(int res)
{
	return 1;
}

int gr_d3d_supports_res_ingame(int res)
{
	return 1;
}

void gr_d3d_set_cull(int cull)
{
	// switch culling on or off
	if(cull){
		d3d_SetRenderState( D3DRENDERSTATE_CULLMODE, D3DCULL_CCW );		
	} else {
		d3d_SetRenderState( D3DRENDERSTATE_CULLMODE, D3DCULL_NONE );				
	}
}

// cross fade
void gr_d3d_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct)
{
	if ( pct <= 50 )	{
		gr_set_bitmap(bmap1);
		gr_bitmap(x1, y1);
	} else {
		gr_set_bitmap(bmap2);
		gr_bitmap(x2, y2);
	}	
}

// filtering
void gr_d3d_filter_set(int filter)
{
}

// set clear color
void gr_d3d_set_clear_color(int r, int g, int b)
{
	gr_init_color(&gr_screen.current_clear_color, r, g, b);
}

// JAS: Need to turn optimizations off or Alan's machine, with 3dfx direct3d hangs...
#pragma optimize("",off)		

void d3d_detect_texture_origin_32()
{
	int test_bmp = -1;
	ubyte data[32*32];
	color ac;
	uint pix1a, pix2a;
	uint pix1b, pix2b;

	mprintf(( "Detecting uv type...\n" ));

	gr_set_gamma(1.0f);
	gr_init_alphacolor(&ac,255,255,255,255);
		
	memset( data, 0, 32*32 );
	data[15*32+15] = 14;
	
	test_bmp = bm_create( 8, 32, 32, data, BMP_AABITMAP );
	
	mprintf(( "Trial #1\n" ));
	D3d_rendition_uvs = 0;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_set_bitmap( test_bmp );
	gr_aabitmap_ex(0, 0, 32, 32, 15, 15);
	Mouse_hidden++;
	gr_flip();
	d3d_get_pixel(0, 0, (ubyte*)&pix1a);
	d3d_get_pixel(1, 1, (ubyte*)&pix1b);
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	mprintf(( "Trial #2\n" ));
	D3d_rendition_uvs = 1;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_set_bitmap( test_bmp );
	gr_aabitmap_ex(0, 0, 32, 32, 15, 15);
	Mouse_hidden++;
	gr_flip();
	d3d_get_pixel(0, 0, (ubyte*)&pix2a);
	d3d_get_pixel(1, 1, (ubyte*)&pix2b);
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	bm_release(test_bmp);

	mprintf(( "Pixel 1 = %x , %x\n", pix1a, pix1b ));
	mprintf(( "Pixel 2 = %x , %x\n", pix2a, pix2b ));

	if ( (pix1b!=0) || (pix2b!=0)  )	{
		D3d_rendition_uvs = 1;
	} else {
		D3d_rendition_uvs = 0;
	}

	mprintf(( "Rendition uvs: %d\n", D3d_rendition_uvs ));
}
	
void d3d_detect_texture_origin_16()
{
	int test_bmp = -1;
	ubyte data[32*32];
	color ac;
	ushort pix1a, pix2a;
	ushort pix1b, pix2b;

	mprintf(( "Detecting uv type...\n" ));

	gr_set_gamma(1.0f);
	gr_init_alphacolor(&ac,255,255,255,255);
		
	memset( data, 0, 32*32 );
	data[15*32+15] = 14;
	
	test_bmp = bm_create( 8, 32, 32, data, BMP_AABITMAP );
	
	mprintf(( "Trial #1\n" ));
	D3d_rendition_uvs = 0;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_set_bitmap( test_bmp );
	gr_aabitmap_ex(0, 0, 32, 32, 15, 15);
	Mouse_hidden++;
	gr_flip();
	d3d_get_pixel(0, 0, (ubyte*)&pix1a);
	d3d_get_pixel(1, 1, (ubyte*)&pix1b);
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	mprintf(( "Trial #2\n" ));
	D3d_rendition_uvs = 1;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_set_bitmap( test_bmp );
	gr_aabitmap_ex(0, 0, 32, 32, 15, 15);
	Mouse_hidden++;
	gr_flip();
	d3d_get_pixel(0, 0, (ubyte*)&pix2a);
	d3d_get_pixel(1, 1, (ubyte*)&pix2b);
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	bm_release(test_bmp);

	mprintf(( "Pixel 1 = %x , %x\n", pix1a, pix1b ));
	mprintf(( "Pixel 2 = %x , %x\n", pix2a, pix2b ));

	if ( (pix1b!=0) || (pix2b!=0)  )	{
		D3d_rendition_uvs = 1;
	} else {
		D3d_rendition_uvs = 0;
	}

	mprintf(( "Rendition uvs: %d\n", D3d_rendition_uvs ));
}

void gr_d3d_get_region(int front, int w, int h, ubyte *data)
{	
	HRESULT ddrval;
	DDSURFACEDESC ddsd;	
	LPDIRECTDRAWSURFACE surf = front ? lpFrontBuffer : lpBackBuffer;
	
	memset( &ddsd, 0, sizeof( ddsd ) );
	ddsd.dwSize = sizeof( ddsd );

	// try and lock the buffer
	ddrval = surf->Lock( NULL, &ddsd, DDLOCK_WAIT | DDLOCK_READONLY, NULL );
	if ( ddrval != DD_OK )	{
		mprintf(( "Error locking surface for get_region, %s\n", d3d_error_string(ddrval) ));
		return;
	}

	ubyte *sptr;	
	ubyte *dptr = data;	
	ubyte *rptr = (ubyte*)ddsd.lpSurface;	

	for (int i=0; i<h; i++ )	{
		sptr = (ubyte*)&rptr[ i * ddsd.lPitch ];

		// don't think we need to swizzle here ...
		for(int j=0; j<w; j++ )	{
			memcpy(dptr, sptr, gr_screen.bytes_per_pixel);
			dptr += gr_screen.bytes_per_pixel;
			sptr += gr_screen.bytes_per_pixel;
		}
	}	
	
	// Unlock the buffer
	surf->Unlock( NULL );	
}

extern float D3D_line_offset;

void d3d_detect_line_offset_32()
{
	color ac;
	uint pix1a, pix2a;
	uint pix1b, pix2b;

	mprintf(( "Detecting line offset...\n" ));

	gr_set_gamma(1.0f);
	gr_init_alphacolor(&ac, 255,255, 255, 255);
	
	mprintf(( "Trial #1\n" ));
	D3D_line_offset = 0.0f;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_line( 0,0,0,0 );
	Mouse_hidden++;
	gr_flip();
	d3d_get_pixel(0, 0, (ubyte*)&pix1a);
	d3d_get_pixel(1, 1, (ubyte*)&pix1b);
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	mprintf(( "Trial #2\n" ));
	D3D_line_offset = 0.5f;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_line( 0,0,0,0 );
	Mouse_hidden++;
	gr_flip();
	d3d_get_pixel(0, 0, (ubyte*)&pix2a);
	d3d_get_pixel(1, 1, (ubyte*)&pix2b);
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	mprintf(( "Pixel 1 = %x , %x\n", pix1a, pix1b ));
	mprintf(( "Pixel 2 = %x , %x\n", pix2a, pix2b ));

	if ( (pix1a!=0) && (pix2a==0)  )	{
		D3D_line_offset = 0.0f;
	} else if ( (pix1a==0) && (pix2a!=0)  )	{
		D3D_line_offset = 0.5f;
	} else {
		D3D_line_offset = 0.0f;
	}

	mprintf(( "Line offset: %.1f\n", D3D_line_offset ));
}

void d3d_detect_line_offset_16()
{
	color ac;
	ushort pix1a, pix2a;
	ushort pix1b, pix2b;

	mprintf(( "Detecting line offset...\n" ));

	gr_set_gamma(1.0f);
	gr_init_alphacolor(&ac, 255,255, 255, 255);
	
	mprintf(( "Trial #1\n" ));
	D3D_line_offset = 0.0f;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_line( 0,0,0,0 );
	Mouse_hidden++;
	gr_flip();
	d3d_get_pixel(0, 0, (ubyte*)&pix1a);
	d3d_get_pixel(1, 1, (ubyte*)&pix1b);
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	mprintf(( "Trial #2\n" ));
	D3D_line_offset = 0.5f;
	gr_reset_clip();
	gr_clear();
	gr_set_color_fast(&ac);
	gr_line( 0,0,0,0 );
	Mouse_hidden++;
	gr_flip();
	d3d_get_pixel(0, 0, (ubyte*)&pix2a);
	d3d_get_pixel(1, 1, (ubyte*)&pix2b);
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;

	mprintf(( "Pixel 1 = %x , %x\n", pix1a, pix1b ));
	mprintf(( "Pixel 2 = %x , %x\n", pix2a, pix2b ));

	if ( (pix1a!=0) && (pix2a==0)  )	{
		D3D_line_offset = 0.0f;
	} else if ( (pix1a==0) && (pix2a!=0)  )	{
		D3D_line_offset = 0.5f;
	} else {
		D3D_line_offset = 0.0f;
	}

	mprintf(( "Line offset: %.1f\n", D3D_line_offset ));
}

#pragma optimize("",on)		

void gr_d3d_init()
{
	D3D_enabled = 1;		// Tell Freespace code that we're using Direct3D.
	D3D_running = 0;	

	Assert( !D3D_inited );

	// pixel format
	Bm_pixel_format = BM_PIXEL_FORMAT_D3D;	
	
	D3D_32bit = 0;
	D3D_window = Cmdline_window;

	// windowed?
	if(D3D_window){
		gr_d3d_init_device(gr_screen.max_w, gr_screen.max_h);
	} else {
		// 32 bit mode		
		if(Cmdline_force_32bit){
			gr_screen.bits_per_pixel = 32;
			gr_screen.bytes_per_pixel = 4;
			gr_d3d_init_device(gr_screen.max_w, gr_screen.max_h);						
		} else {
			// 16 bit mode			
			gr_screen.bits_per_pixel = 16;
			gr_screen.bytes_per_pixel = 2;
			gr_d3d_init_device(gr_screen.max_w, gr_screen.max_h);						
		}
	}

	// determine 32 bit status
	D3D_32bit = gr_screen.bits_per_pixel == 32 ? 1 : 0;
	d3d_tcache_init(D3D_32bit ? 1 : 0);
	Gr_bitmap_poly = D3D_32bit ? 1 : 0;	

	// did we initialize properly?
	if(!D3D_inited){
		return;
	}

	// zbiasing?
	if(os_config_read_uint(NULL, "DisableZbias", 0)){
		D3D_zbias = 0;
	}

	// perform a check for bump mapping capabilities - unknownplayer
	
	d3d_start_frame();	

	Gr_current_red = &Gr_red;
	Gr_current_blue = &Gr_blue;
	Gr_current_green = &Gr_green;
	Gr_current_alpha = &Gr_alpha;

	gr_screen.gf_flip = gr_d3d_flip;
	gr_screen.gf_flip_window = gr_d3d_flip_window;
	gr_screen.gf_set_clip = gr_d3d_set_clip;
	gr_screen.gf_reset_clip = gr_d3d_reset_clip;
	gr_screen.gf_set_font = grx_set_font;

	gr_screen.gf_get_color = gr_d3d_get_color;
	gr_screen.gf_init_color = gr_d3d_init_color;
	gr_screen.gf_set_color_fast = gr_d3d_set_color_fast;
	gr_screen.gf_set_color = gr_d3d_set_color;
	gr_screen.gf_init_color = gr_d3d_init_color;
	gr_screen.gf_init_alphacolor = gr_d3d_init_alphacolor;

	gr_screen.gf_set_bitmap = gr_d3d_set_bitmap;
	gr_screen.gf_create_shader = gr_d3d_create_shader;
	gr_screen.gf_set_shader = gr_d3d_set_shader;
	gr_screen.gf_clear = gr_d3d_clear;
	// gr_screen.gf_bitmap = gr_d3d_bitmap;
	// gr_screen.gf_bitmap_ex = gr_d3d_bitmap_ex;
	gr_screen.gf_aabitmap = gr_d3d_aabitmap;
	gr_screen.gf_aabitmap_ex = gr_d3d_aabitmap_ex;

	gr_screen.gf_rect = gr_d3d_rect;
	gr_screen.gf_shade = gr_d3d_shade;
	gr_screen.gf_string = gr_d3d_string;
	gr_screen.gf_circle = gr_d3d_circle;

	gr_screen.gf_line = gr_d3d_line;
	gr_screen.gf_aaline = gr_d3d_aaline;
	gr_screen.gf_pixel = gr_d3d_pixel;
	gr_screen.gf_scaler = gr_d3d_scaler;
	gr_screen.gf_aascaler = gr_d3d_aascaler;
	gr_screen.gf_tmapper = gr_d3d_tmapper;

	gr_screen.gf_gradient = gr_d3d_gradient;

	gr_screen.gf_set_palette = gr_d3d_set_palette;
	gr_screen.gf_print_screen = gr_d3d_print_screen;

	gr_screen.gf_fade_in = gr_d3d_fade_in;
	gr_screen.gf_fade_out = gr_d3d_fade_out;
	gr_screen.gf_flash = gr_d3d_flash;

	gr_screen.gf_zbuffer_get = gr_d3d_zbuffer_get;
	gr_screen.gf_zbuffer_set = gr_d3d_zbuffer_set;
	gr_screen.gf_zbuffer_clear = gr_d3d_zbuffer_clear;

	gr_screen.gf_save_screen = gr_d3d_save_screen;
	gr_screen.gf_restore_screen = gr_d3d_restore_screen;
	gr_screen.gf_free_screen = gr_d3d_free_screen;

	// Screen dumping stuff
	gr_screen.gf_dump_frame_start = gr_d3d_dump_frame_start;
	gr_screen.gf_dump_frame_stop = gr_d3d_dump_frame_stop;
	gr_screen.gf_dump_frame = gr_d3d_dump_frame;

	gr_screen.gf_set_gamma = gr_d3d_set_gamma;

	// Lock/unlock stuff
	gr_screen.gf_lock = gr_d3d_lock;
	gr_screen.gf_unlock = gr_d3d_unlock;

	// screen region
	gr_screen.gf_get_region = gr_d3d_get_region;

	// fog stuff
	gr_screen.gf_fog_set = gr_d3d_fog_set;

	// pixel get
	gr_screen.gf_get_pixel = gr_d3d_get_pixel;

	// poly culling
	gr_screen.gf_set_cull = gr_d3d_set_cull;

	// cross fade
	gr_screen.gf_cross_fade = gr_d3d_cross_fade;

	// filtering
	gr_screen.gf_filter_set = gr_d3d_filter_set;

	// texture cache
	gr_screen.gf_tcache_set = d3d_tcache_set;

	// set clear color
	gr_screen.gf_set_clear_color = gr_d3d_set_clear_color;
	
	uint tmp = os_config_read_uint( NULL, "D3DTextureOrigin", 0xFFFF );

	if ( tmp != 0xFFFF )	{
		if ( tmp )	{
			D3d_rendition_uvs = 1;
		} else {
			D3d_rendition_uvs = 0;
		}
	} else {
		if(D3D_32bit){
			d3d_detect_texture_origin_32();
		} else {
			d3d_detect_texture_origin_16();
		}
	}

	tmp = os_config_read_uint( NULL, "D3DLineOffset", 0xFFFF );

	if ( tmp != 0xFFFF )	{
		if ( tmp )	{
			D3D_line_offset = 0.5f;
		} else {
			D3D_line_offset = 0.0f;
		}
	} else {
		if(D3D_32bit){
			d3d_detect_line_offset_32();
		} else {
			d3d_detect_line_offset_16();
		}
	}

	D3D_running = 1;	

	Mouse_hidden++;
	gr_reset_clip();
	gr_clear();
	gr_flip();
	Mouse_hidden--;
}

char* d3d_error_string(HRESULT error)
{
//XSTR:OFF
    switch(error) {
        case DD_OK:
            return "No error.\0";
        case DDERR_ALREADYINITIALIZED:
            return "This object is already initialized.\0";
        case DDERR_BLTFASTCANTCLIP:
            return "Return if a clipper object is attached to the source surface passed into a BltFast call.\0";
        case DDERR_CANNOTATTACHSURFACE:
            return "This surface can not be attached to the requested surface.\0";
        case DDERR_CANNOTDETACHSURFACE:
            return "This surface can not be detached from the requested surface.\0";
        case DDERR_CANTCREATEDC:
            return "Windows can not create any more DCs.\0";
        case DDERR_CANTDUPLICATE:
            return "Can't duplicate primary & 3D surfaces, or surfaces that are implicitly created.\0";
        case DDERR_CLIPPERISUSINGHWND:
            return "An attempt was made to set a cliplist for a clipper object that is already monitoring an hwnd.\0";
        case DDERR_COLORKEYNOTSET:
            return "No src color key specified for this operation.\0";
        case DDERR_CURRENTLYNOTAVAIL:
            return "Support is currently not available.\0";
        case DDERR_DIRECTDRAWALREADYCREATED:
            return "A DirectDraw object representing this driver has already been created for this process.\0";
        case DDERR_EXCEPTION:
            return "An exception was encountered while performing the requested operation.\0";
        case DDERR_EXCLUSIVEMODEALREADYSET:
            return "An attempt was made to set the cooperative level when it was already set to exclusive.\0";
        case DDERR_GENERIC:
            return "Generic failure.\0";
        case DDERR_HEIGHTALIGN:
            return "Height of rectangle provided is not a multiple of reqd alignment.\0";
        case DDERR_HWNDALREADYSET:
            return "The CooperativeLevel HWND has already been set. It can not be reset while the process has surfaces or palettes created.\0";
        case DDERR_HWNDSUBCLASSED:
            return "HWND used by DirectDraw CooperativeLevel has been subclassed, this prevents DirectDraw from restoring state.\0";
        case DDERR_IMPLICITLYCREATED:
            return "This surface can not be restored because it is an implicitly created surface.\0";
        case DDERR_INCOMPATIBLEPRIMARY:
            return "Unable to match primary surface creation request with existing primary surface.\0";
        case DDERR_INVALIDCAPS:
            return "One or more of the caps bits passed to the callback are incorrect.\0";
        case DDERR_INVALIDCLIPLIST:
            return "DirectDraw does not support the provided cliplist.\0";
        case DDERR_INVALIDDIRECTDRAWGUID:
            return "The GUID passed to DirectDrawCreate is not a valid DirectDraw driver identifier.\0";
        case DDERR_INVALIDMODE:
            return "DirectDraw does not support the requested mode.\0";
        case DDERR_INVALIDOBJECT:
            return "DirectDraw received a pointer that was an invalid DIRECTDRAW object.\0";
        case DDERR_INVALIDPARAMS:
            return "One or more of the parameters passed to the function are incorrect.\0";
        case DDERR_INVALIDPIXELFORMAT:
            return "The pixel format was invalid as specified.\0";
        case DDERR_INVALIDPOSITION:
            return "Returned when the position of the overlay on the destination is no longer legal for that destination.\0";
        case DDERR_INVALIDRECT:
            return "Rectangle provided was invalid.\0";
        case DDERR_LOCKEDSURFACES:
            return "Operation could not be carried out because one or more surfaces are locked.\0";
        case DDERR_NO3D:
            return "There is no 3D present.\0";
        case DDERR_NOALPHAHW:
            return "Operation could not be carried out because there is no alpha accleration hardware present or available.\0";
        case DDERR_NOBLTHW:
            return "No blitter hardware present.\0";
        case DDERR_NOCLIPLIST:
            return "No cliplist available.\0";
        case DDERR_NOCLIPPERATTACHED:
            return "No clipper object attached to surface object.\0";
        case DDERR_NOCOLORCONVHW:
            return "Operation could not be carried out because there is no color conversion hardware present or available.\0";
        case DDERR_NOCOLORKEY:
            return "Surface doesn't currently have a color key\0";
        case DDERR_NOCOLORKEYHW:
            return "Operation could not be carried out because there is no hardware support of the destination color key.\0";
        case DDERR_NOCOOPERATIVELEVELSET:
            return "Create function called without DirectDraw object method SetCooperativeLevel being called.\0";
        case DDERR_NODC:
            return "No DC was ever created for this surface.\0";
        case DDERR_NODDROPSHW:
            return "No DirectDraw ROP hardware.\0";
        case DDERR_NODIRECTDRAWHW:
            return "A hardware-only DirectDraw object creation was attempted but the driver did not support any hardware.\0";
        case DDERR_NOEMULATION:
            return "Software emulation not available.\0";
        case DDERR_NOEXCLUSIVEMODE:
            return "Operation requires the application to have exclusive mode but the application does not have exclusive mode.\0";
        case DDERR_NOFLIPHW:
            return "Flipping visible surfaces is not supported.\0";
        case DDERR_NOGDI:
            return "There is no GDI present.\0";
        case DDERR_NOHWND:
            return "Clipper notification requires an HWND or no HWND has previously been set as the CooperativeLevel HWND.\0";
        case DDERR_NOMIRRORHW:
            return "Operation could not be carried out because there is no hardware present or available.\0";
        case DDERR_NOOVERLAYDEST:
            return "Returned when GetOverlayPosition is called on an overlay that UpdateOverlay has never been called on to establish a destination.\0";
        case DDERR_NOOVERLAYHW:
            return "Operation could not be carried out because there is no overlay hardware present or available.\0";
        case DDERR_NOPALETTEATTACHED:
            return "No palette object attached to this surface.\0";
        case DDERR_NOPALETTEHW:
            return "No hardware support for 16 or 256 color palettes.\0";
        case DDERR_NORASTEROPHW:
            return "Operation could not be carried out because there is no appropriate raster op hardware present or available.\0";
        case DDERR_NOROTATIONHW:
            return "Operation could not be carried out because there is no rotation hardware present or available.\0";
        case DDERR_NOSTRETCHHW:
            return "Operation could not be carried out because there is no hardware support for stretching.\0";
        case DDERR_NOT4BITCOLOR:
            return "DirectDrawSurface is not in 4 bit color palette and the requested operation requires 4 bit color palette.\0";
        case DDERR_NOT4BITCOLORINDEX:
            return "DirectDrawSurface is not in 4 bit color index palette and the requested operation requires 4 bit color index palette.\0";
        case DDERR_NOT8BITCOLOR:
            return "DirectDrawSurface is not in 8 bit color mode and the requested operation requires 8 bit color.\0";
        case DDERR_NOTAOVERLAYSURFACE:
            return "Returned when an overlay member is called for a non-overlay surface.\0";
        case DDERR_NOTEXTUREHW:
            return "Operation could not be carried out because there is no texture mapping hardware present or available.\0";
        case DDERR_NOTFLIPPABLE:
            return "An attempt has been made to flip a surface that is not flippable.\0";
        case DDERR_NOTFOUND:
            return "Requested item was not found.\0";
        case DDERR_NOTLOCKED:
            return "Surface was not locked.  An attempt to unlock a surface that was not locked at all, or by this process, has been attempted.\0";
        case DDERR_NOTPALETTIZED:
            return "The surface being used is not a palette-based surface.\0";
        case DDERR_NOVSYNCHW:
            return "Operation could not be carried out because there is no hardware support for vertical blank synchronized operations.\0";
        case DDERR_NOZBUFFERHW:
            return "Operation could not be carried out because there is no hardware support for zbuffer blitting.\0";
        case DDERR_NOZOVERLAYHW:
            return "Overlay surfaces could not be z layered based on their BltOrder because the hardware does not support z layering of overlays.\0";
        case DDERR_OUTOFCAPS:
            return "The hardware needed for the requested operation has already been allocated.\0";
        case DDERR_OUTOFMEMORY:
            return "DirectDraw does not have enough memory to perform the operation.\0";
        case DDERR_OUTOFVIDEOMEMORY:
            return "DirectDraw does not have enough memory to perform the operation.\0";
        case DDERR_OVERLAYCANTCLIP:
            return "The hardware does not support clipped overlays.\0";
        case DDERR_OVERLAYCOLORKEYONLYONEACTIVE:
            return "Can only have ony color key active at one time for overlays.\0";
        case DDERR_OVERLAYNOTVISIBLE:
            return "Returned when GetOverlayPosition is called on a hidden overlay.\0";
        case DDERR_PALETTEBUSY:
            return "Access to this palette is being refused because the palette is already locked by another thread.\0";
        case DDERR_PRIMARYSURFACEALREADYEXISTS:
            return "This process already has created a primary surface.\0";
        case DDERR_REGIONTOOSMALL:
            return "Region passed to Clipper::GetClipList is too small.\0";
        case DDERR_SURFACEALREADYATTACHED:
            return "This surface is already attached to the surface it is being attached to.\0";
        case DDERR_SURFACEALREADYDEPENDENT:
            return "This surface is already a dependency of the surface it is being made a dependency of.\0";
        case DDERR_SURFACEBUSY:
            return "Access to this surface is being refused because the surface is already locked by another thread.\0";
        case DDERR_SURFACEISOBSCURED:
            return "Access to surface refused because the surface is obscured.\0";
        case DDERR_SURFACELOST:
            return "Access to this surface is being refused because the surface memory is gone. The DirectDrawSurface object representing this surface should have Restore called on it.\0";
        case DDERR_SURFACENOTATTACHED:
            return "The requested surface is not attached.\0";
        case DDERR_TOOBIGHEIGHT:
            return "Height requested by DirectDraw is too large.\0";
        case DDERR_TOOBIGSIZE:
            return "Size requested by DirectDraw is too large, but the individual height and width are OK.\0";
        case DDERR_TOOBIGWIDTH:
            return "Width requested by DirectDraw is too large.\0";
        case DDERR_UNSUPPORTED:
            return "Action not supported.\0";
        case DDERR_UNSUPPORTEDFORMAT:
            return "FOURCC format requested is unsupported by DirectDraw.\0";
        case DDERR_UNSUPPORTEDMASK:
            return "Bitmask in the pixel format requested is unsupported by DirectDraw.\0";
        case DDERR_VERTICALBLANKINPROGRESS:
            return "Vertical blank is in progress.\0";
        case DDERR_WASSTILLDRAWING:
            return "Informs DirectDraw that the previous Blt which is transfering information to or from this Surface is incomplete.\0";
        case DDERR_WRONGMODE:
            return "This surface can not be restored because it was created in a different mode.\0";
        case DDERR_XALIGN:
            return "Rectangle provided was not horizontally aligned on required boundary.\0";
        case D3DERR_BADMAJORVERSION:
            return "D3DERR_BADMAJORVERSION\0";
        case D3DERR_BADMINORVERSION:
            return "D3DERR_BADMINORVERSION\0";
        case D3DERR_EXECUTE_LOCKED:
            return "D3DERR_EXECUTE_LOCKED\0";
        case D3DERR_EXECUTE_NOT_LOCKED:
            return "D3DERR_EXECUTE_NOT_LOCKED\0";
        case D3DERR_EXECUTE_CREATE_FAILED:
            return "D3DERR_EXECUTE_CREATE_FAILED\0";
        case D3DERR_EXECUTE_DESTROY_FAILED:
            return "D3DERR_EXECUTE_DESTROY_FAILED\0";
        case D3DERR_EXECUTE_LOCK_FAILED:
            return "D3DERR_EXECUTE_LOCK_FAILED\0";
        case D3DERR_EXECUTE_UNLOCK_FAILED:
            return "D3DERR_EXECUTE_UNLOCK_FAILED\0";
        case D3DERR_EXECUTE_FAILED:
            return "D3DERR_EXECUTE_FAILED\0";
        case D3DERR_EXECUTE_CLIPPED_FAILED:
            return "D3DERR_EXECUTE_CLIPPED_FAILED\0";
        case D3DERR_TEXTURE_NO_SUPPORT:
            return "D3DERR_TEXTURE_NO_SUPPORT\0";
        case D3DERR_TEXTURE_NOT_LOCKED:
            return "D3DERR_TEXTURE_NOT_LOCKED\0";
        case D3DERR_TEXTURE_LOCKED:
            return "D3DERR_TEXTURELOCKED\0";
        case D3DERR_TEXTURE_CREATE_FAILED:
            return "D3DERR_TEXTURE_CREATE_FAILED\0";
        case D3DERR_TEXTURE_DESTROY_FAILED:
            return "D3DERR_TEXTURE_DESTROY_FAILED\0";
        case D3DERR_TEXTURE_LOCK_FAILED:
            return "D3DERR_TEXTURE_LOCK_FAILED\0";
        case D3DERR_TEXTURE_UNLOCK_FAILED:
            return "D3DERR_TEXTURE_UNLOCK_FAILED\0";
        case D3DERR_TEXTURE_LOAD_FAILED:
            return "D3DERR_TEXTURE_LOAD_FAILED\0";
        case D3DERR_MATRIX_CREATE_FAILED:
            return "D3DERR_MATRIX_CREATE_FAILED\0";
        case D3DERR_MATRIX_DESTROY_FAILED:
            return "D3DERR_MATRIX_DESTROY_FAILED\0";
        case D3DERR_MATRIX_SETDATA_FAILED:
            return "D3DERR_MATRIX_SETDATA_FAILED\0";
        case D3DERR_SETVIEWPORTDATA_FAILED:
            return "D3DERR_SETVIEWPORTDATA_FAILED\0";
        case D3DERR_MATERIAL_CREATE_FAILED:
            return "D3DERR_MATERIAL_CREATE_FAILED\0";
        case D3DERR_MATERIAL_DESTROY_FAILED:
            return "D3DERR_MATERIAL_DESTROY_FAILED\0";
        case D3DERR_MATERIAL_SETDATA_FAILED:
            return "D3DERR_MATERIAL_SETDATA_FAILED\0";
        case D3DERR_LIGHT_SET_FAILED:
            return "D3DERR_LIGHT_SET_FAILED\0";
        default:
            return "Unrecognized error value.\0";
    }
//XSTR:ON
}
