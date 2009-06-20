/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef NO_DIRECT3D

#include <math.h>
#include <d3d8.h>
#include <D3dx8tex.h>
#include <Dxerr8.h>

#include "graphics/2d.h"

#include "globalincs/systemvars.h"
#include "globalincs/alphacolors.h"

#include "graphics/grinternal.h"

#include "graphics/grd3d.h"
#include "graphics/grd3dinternal.h"
#include "graphics/grd3dlight.h"
#include "graphics/grd3dbmpman.h"

#include "osapi/osapi.h"
#include "bmpman/bmpman.h"
#include "io/key.h"
#include "math/floating.h"
#include "palman/palman.h"
#include "osapi/osregistry.h"

#include "graphics/line.h"
#include "graphics/font.h"
#include "io/mouse.h"
#include "cfile/cfile.h"
#include "cmdline/cmdline.h"
#include "debugconsole/timerbar.h"
#include "debugconsole/dbugfile.h"
#include "freespace2/freespaceresource.h"   
#include "cmdline/cmdline.h"

#include <vector>


enum vertex_buffer_type{TRILIST_,LINELIST_,FLAT_};

// Structures and enums
struct Vertex_buffer {
	Vertex_buffer() { memset(this, 0, sizeof(Vertex_buffer)); };
	int n_prim;
	int n_verts;
	vertex_buffer_type type;
	uint FVF;
	int size;
	IDirect3DVertexBuffer8 *buffer;
};

enum stage_state{
	NONE = -1, 
	INITIAL = 0, 
	DEFUSE = 1, 
	GLOW_MAPPED_DEFUSE = 2, 
	NONMAPPED_SPECULAR = 3, 
	GLOWMAPPED_NONMAPPED_SPECULAR = 4, 
	MAPPED_SPECULAR = 5, CELL = 6, 
	GLOWMAPPED_CELL = 7, 
	ADDITIVE_GLOWMAPPING = 8, 
	SINGLE_PASS_SPECMAPPING = 9, 
	SINGLE_PASS_GLOW_SPEC_MAPPING = 10,
	BACKGROUND_FOG = 11,
	ENV = 12
};

//LPDIRECT3DCUBETEXTURE8 cube_map;


// External variables - booo!
extern bool env_enabled;
extern matrix View_matrix;
extern vec3d View_position;
extern matrix Eye_matrix;
extern vec3d Eye_position;
extern vec3d Object_position;
extern matrix Object_matrix;
extern float	Canv_w2;				// Canvas_width / 2
extern float	Canv_h2;				// Canvas_height / 2
extern float	View_zoom;

extern int G3_user_clip;
extern vec3d G3_user_clip_normal;
extern vec3d G3_user_clip_point;

static int D3d_dump_frames = 0;
static ubyte *D3d_dump_buffer = NULL;
static int D3d_dump_frame_number = 0;
static int D3d_dump_frame_count = 0;
static int D3d_dump_frame_count_max = 0;
static int D3d_dump_frame_size = 0;

// Variables
stage_state current_render_state = NONE;

int In_frame = 0;

IDirect3DSurface8 *Gr_saved_surface = NULL;
std::vector<Vertex_buffer> D3D_vertex_buffers;
static int D3D_vertex_buffers_in_use = 0;
extern int n_active_lights;

D3DXPLANE d3d_user_clip_plane;

IDirect3DSurface8 *old_render_target = NULL;
//IDirect3DTexture8 *background_render_target = NULL;

// Function declarations
void shift_active_lights(int pos);
void pre_render_lights_init();
const char *d3d_error_string(HRESULT error);

void d3d_string_mem_use(int x, int y)
{
	char mem_buffer[50];
	sprintf(mem_buffer,"Texture mem free: %d Meg", GlobalD3DVars::lpD3DDevice->GetAvailableTextureMem()/1024/1024);
	gr_string( x, y, mem_buffer);
}

void d3d_fill_pixel_format(PIXELFORMAT *pixelf, D3DFORMAT tformat);

// Whats all this then? Can we get rid of it? - RT

// LPVOID lpBufStart, lpPointer, lpInsStart;
// int Exb_size;

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

// No objects should be rendered before this frame
void d3d_start_frame()
{
	if(!GlobalD3DVars::D3D_activate) return;

	HRESULT ddrval;



	if (!GlobalD3DVars::D3D_inited) return;

	if ( In_frame < 0 || In_frame > 1 )	{
		mprintf(( "Start frame error! (%d)\n", In_frame ));
		return;
	}

	if ( In_frame == 1 ) return;

	In_frame++;


	ddrval = GlobalD3DVars::lpD3DDevice->BeginScene();
	if (ddrval != D3D_OK )	{
		//mprintf(( "Failed to begin scene!\n%s\n", d3d_error_string(ddrval) ));
		return;
	}
}

// No objects should be rendered after this frame
void d3d_stop_frame()
{

	if (!GlobalD3DVars::D3D_inited) return;
	if(!GlobalD3DVars::D3D_activate) return;

	if ( In_frame < 0 || In_frame > 1 )	{
		mprintf(( "Stop frame error! (%d)\n", In_frame ));
		return;
	}

	if ( In_frame == 0 ) return;

	In_frame--;

	if(GlobalD3DVars::D3D_activate)TIMERBAR_END_FRAME();
	if(FAILED(GlobalD3DVars::lpD3DDevice->EndScene()))
	{
		return;
	}
	TIMERBAR_START_FRAME();
						 
	TIMERBAR_PUSH(1);
	// Must cope with device being lost
	if(GlobalD3DVars::lpD3DDevice->Present(NULL,NULL,NULL,NULL) == D3DERR_DEVICELOST)
	{
		d3d_lost_device();
	}
	TIMERBAR_POP();
}



// This function calls these render state one when the device is initialised and when the device is lost.
void d3d_set_initial_render_state(bool set)
{
	if(current_render_state == INITIAL)return;

	if(current_render_state == NONE){//this only needs to be done the first time-Bobboau
		d3d_SetRenderState(D3DRS_DITHERENABLE, TRUE );
		d3d_SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
		d3d_SetRenderState(D3DRS_SPECULARENABLE, FALSE ); 

		// Turn lighting off here, its on by default!
		d3d_SetRenderState(D3DRS_LIGHTING , FALSE);
	}


	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE), set, set;
	d3d_SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_LINEAR , set, set);
	d3d_SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR , set, set);
	d3d_SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0, set, set);
	d3d_SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_LINEAR , set, set);
	d3d_SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_LINEAR , set, set);
	d3d_SetTexture(1, NULL);

	d3d_SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, 0, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);
	d3d_SetTextureStageState( 4, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(initial_state_block);
	current_render_state = INITIAL;
}

//BACKGROUND_FOG
void set_stage_for_background_fog(bool set){
	if(current_render_state == BACKGROUND_FOG)return;
/*
	if(!set){
		//d3d_proj_fov
		//d3d_proj_ratio;
		float fov = (1.0/d3d_proj_fov) * 0.65; // I don't know whay this needs to be 0.65 ???
//		int timestamp();
//		float distort = sin(float(timestamp()) / 1000.0);
		float distort = 0.0;

		D3DXMATRIX world;
		D3DXMATRIX cloak(
			fov,		0,						0,		0,
			0,			-fov * d3d_proj_ratio,	0,		0,
			0.5,		0.5,					1,		0,
			0,			0,						0,		1);
		D3DXMATRIX corection(
			1,	0,	0,	0,
			0,	1,	0,	0,
			0,	0,	1,	0,
			0,	0,	-1,	1);
		D3DXMatrixMultiply(&world, &corection, &cloak);

		GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_TEXTURE0, &world);
		
	}

	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1, set, set);

	d3d_SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);
	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	d3d_SetTexture(0, background_render_target);
*/	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(background_fog_state_block);
	current_render_state = BACKGROUND_FOG;
}

extern bool env_enabled;

void set_stage_for_cell_shaded(bool set){
	if(current_render_state == CELL)return;
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);

	d3d_SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_POINT , set, set);
	d3d_SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_POINT , set, set);

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE, set, set);

	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(cell_state_block);
	current_render_state = CELL;
		
}

void set_stage_for_cell_glowmapped_shaded(bool set){
	if(current_render_state == GLOWMAPPED_CELL)return;
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);

	d3d_SetTextureStageState(1, D3DTSS_MINFILTER, D3DTEXF_POINT, set, set );
	d3d_SetTextureStageState(1, D3DTSS_MAGFILTER, D3DTEXF_POINT, set, set );

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 1, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE, set, set);

	d3d_SetTextureStageState( 2, D3DTSS_TEXCOORDINDEX, 0, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_ADD, set, set);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(glow_mapped_cell_state_block);
	current_render_state = GLOWMAPPED_CELL;
		
}

void set_stage_for_additive_glowmapped(bool set){
	if(current_render_state == ADDITIVE_GLOWMAPPING)return;

	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);
	
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(additive_glow_mapping_state_block);
	current_render_state = ADDITIVE_GLOWMAPPING;
}

void set_stage_for_defuse(bool set){
	if(current_render_state == DEFUSE)return;

	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0, set, set);

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE, set, set);

	d3d_SetTexture(1, NULL);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(defuse_state_block);
	current_render_state = DEFUSE;
		
}

void set_stage_for_glow_mapped_defuse(bool set){
	if(current_render_state == GLOW_MAPPED_DEFUSE)return;
	if(!set && GLOWMAP < 0){
		set_stage_for_defuse();
		return;
	}
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);

		d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0, set, set);

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0, set, set);
		
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADD, set, set);
//	d3d_SetTexture(1, GLOWMAP);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(glow_mapped_defuse_state_block);
	current_render_state = GLOW_MAPPED_DEFUSE;
}

void set_stage_for_defuse_and_non_mapped_spec(bool set){
	if(current_render_state == NONMAPPED_SPECULAR)return;
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_SPECULAR, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADD, set, set);

	d3d_SetTexture(1, NULL);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(nonmapped_specular_state_block);
	current_render_state = NONMAPPED_SPECULAR;
}

void set_stage_for_glow_mapped_defuse_and_non_mapped_spec(bool set){
	if(current_render_state == GLOWMAPPED_NONMAPPED_SPECULAR)return;
	if(!set && GLOWMAP < 0){
		set_stage_for_defuse_and_non_mapped_spec();
		return;
	}
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);

	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0, set, set);
		
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADD, set, set);
//	d3d_SetTexture(1, GLOWMAP);

	d3d_SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_SPECULAR, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_ADD, set, set);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(glow_mapped_nonmapped_specular_state_block);
	current_render_state = GLOWMAPPED_NONMAPPED_SPECULAR;
}

bool set_stage_for_spec_mapped(bool set){
	if(current_render_state == MAPPED_SPECULAR)return true;
	if(!set && SPECMAP < 0){
	//	Error(LOCATION, "trying to set stage when there is no specmap");
		return false;
	}
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);


	d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_SPECULAR, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE4X, set, set);
//	d3d_SetTexture(0, SPECMAP);

	d3d_SetTexture(1, NULL);
	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);
	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);

	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);


	current_render_state = MAPPED_SPECULAR;
	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(mapped_specular_state_block);
	return true;
}

extern bool texture_has_alpha(int t_idx);

bool set_stage_for_env_mapped(bool set){
	if(current_render_state == ENV)return true;
	if(!set && SPECMAP < 0){
	//	Error(LOCATION, "trying to set stage when there is no specmap");
		return false;
	}

//	D3DXMATRIX world;

	if(!set){
		D3DXMATRIX world(
			Eye_matrix.vec.rvec.xyz.x, Eye_matrix.vec.rvec.xyz.y, Eye_matrix.vec.rvec.xyz.z, 0,
			Eye_matrix.vec.uvec.xyz.x, Eye_matrix.vec.uvec.xyz.y, Eye_matrix.vec.uvec.xyz.z, 0,
			Eye_matrix.vec.fvec.xyz.x, Eye_matrix.vec.fvec.xyz.y, Eye_matrix.vec.fvec.xyz.z, 0,
			0, 0, 0, 1);

		GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_TEXTURE1, &world);
	}

	d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 0, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);


//	d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_SPECULAR);
//	if(texture_has_alpha(SPECMAP))
	if(Cmdline_alpha_env)
		d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_ALPHAREPLICATE, set, set);
	else
		d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE, set, set);

	d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1, set, set);
//	d3d_SetTexture(0, SPECMAP);

//	if(!set)d3d_SetTexture(1, cube_map);
	d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR, set, set);

	d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE, set, set);
	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE, set, set);
//	d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_PREMODULATE, set, set);

	d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);
	d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE, set, set);
//D3DTOP_BLENDTEXTUREALPHA

	current_render_state = ENV;

	if(!set)GlobalD3DVars::lpD3DDevice->ApplyStateBlock(env_state_block);
	return true;
}


//glow texture stage 3
void set_stage_for_single_pass_glow_specmapping(int SAME){
	//D3DPMISCCAPS_TSSARGTEMP
	static int same = -1;
//	if((current_render_state == SINGLE_PASS_GLOW_SPEC_MAPPING) && (same == SAME))return;
	if(SPECMAP < 0)return;
	if(GLOWMAP < 0){
	//	set_stage_for_single_pass_specmapping(SAME);
		return;
	}
	float u_scale = 1.0f, v_scale = 1.0f;
	if(!SAME){
		d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
		d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);

		d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT);
		d3d_SetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
		d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_ADD);

		d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT);

		d3d_SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_SPECULAR);
		d3d_SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_TEXTURE);
		d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_MODULATE4X);
	
		d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_TEMP );
	
		d3d_SetTextureStageState( 3, D3DTSS_COLORARG1, D3DTA_CURRENT);
		d3d_SetTextureStageState( 3, D3DTSS_COLORARG2, D3DTA_TEMP);
		d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_ADD);

		d3d_SetTextureStageState( 3, D3DTSS_RESULTARG, D3DTA_CURRENT);

		same = SAME;
	}else{
		gr_screen.gf_set_bitmap(SPECMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0);
 		d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, 0, 1);

		gr_screen.gf_set_bitmap(GLOWMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0);
		d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, 0, 5);

		d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_SPECULAR);
		d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

		d3d_SetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_TEMP);

		d3d_SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		d3d_SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

		d3d_SetTextureStageState( 2, D3DTSS_RESULTARG, D3DTA_CURRENT);

		d3d_SetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEMP);
		d3d_SetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT);
		d3d_SetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_MODULATE4X);

		d3d_SetTextureStageState( 3, D3DTSS_RESULTARG, D3DTA_TEMP);

		d3d_SetTextureStageState( 3, D3DTSS_COLORARG1, D3DTA_TEMP);
		d3d_SetTextureStageState( 3, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		d3d_SetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_MODULATE);

		d3d_SetTextureStageState( 4, D3DTSS_RESULTARG, D3DTA_CURRENT);

		d3d_SetTextureStageState( 4, D3DTSS_COLORARG1, D3DTA_TEMP);
		d3d_SetTextureStageState( 4, D3DTSS_COLORARG2, D3DTA_CURRENT);
		d3d_SetTextureStageState( 4, D3DTSS_COLOROP, D3DTOP_ADD);

		d3d_SetTextureStageState( 5, D3DTSS_RESULTARG, D3DTA_CURRENT);

		d3d_SetTextureStageState( 5, D3DTSS_COLORARG1, D3DTA_CURRENT);
		d3d_SetTextureStageState( 5, D3DTSS_COLORARG2, D3DTA_TEXTURE);
		d3d_SetTextureStageState( 5, D3DTSS_COLOROP, D3DTOP_ADD);

		same = SAME;
	}
	current_render_state = SINGLE_PASS_GLOW_SPEC_MAPPING;
}

void set_stage_for_mapped_environment_mapping(){
/*	if(ENVMAP > 0 && env_enabled){
		d3d_SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 1);
	
		d3d_SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_CURRENT);
		d3d_SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
		d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_ADD);
		d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	}else{
		d3d_SetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0);
		d3d_SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	}

	state = INITIAL;*/
}

extern bool d3d_init_device();
void gr_d3d_flip()
{
	if(!GlobalD3DVars::D3D_activate) return;
	int mx, my;

	// Attempt to allow D3D8 to recover from task switching
	// this can't work if it's NULL, check reversed - taylor
	if ( GlobalD3DVars::lpD3DDevice == NULL ) {
		// we really lost the device, try a reinit
		if ( !d3d_init_device() )
			Assert( 0 );
	}

	// if the device is not going to be available then just loop around until we get it back
	// Returns:
	//   TRUE  = the device is lost and cannot be recovered yet
	//   FALSE = the device is fine or has been successfully reset
	if ( d3d_lost_device() )
		return;

	gr_reset_clip();	

	mouse_eval_deltas();

	if ( mouse_is_visible() )	{				
		gr_reset_clip();
		mouse_get_pos( &mx, &my );
		
		if ( Gr_cursor != -1 )	{
			gr_set_bitmap(Gr_cursor);				
			gr_bitmap( mx, my, false);
		}		
	} 	

	d3d_stop_frame();

	d3d_tcache_frame();

	d3d_start_frame();

}

void gr_d3d_flip_cleanup()
{
	d3d_stop_frame();
}

void gr_d3d_flip_window(uint _hdc, int x, int y, int w, int h )
{
}

void gr_d3d_fade_in(int instantaneous)
{
}

void gr_d3d_fade_out(int instantaneous)
{
}

void d3d_setup_format_components(PIXELFORMAT *surface, color_gun *r_gun, color_gun *g_gun, color_gun *b_gun, color_gun *a_gun);

int gr_d3d_save_screen()
{

	int rect_size_x;
	int rect_size_y;

	if(!GlobalD3DVars::D3D_activate) return -1;
	gr_reset_clip();

	if (GlobalD3DVars::lpD3D == NULL)
		return -1;

	if ( Gr_saved_surface )	{
		mprintf(( "Screen alread saved!\n" ));
		return -1;
	}

	IDirect3DSurface8 *front_buffer_a8r8g8b8 = NULL;
	D3DDISPLAYMODE mode;
	mode.Width = mode.Height = 0;

	// although this doesn't really matter in fullscreen mode it doesn't hurt either
	if (FAILED(GlobalD3DVars::lpD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode)))
	{
		DBUGFILE_OUTPUT_0("Could not get adapter display mode");
		return -1;
	}

	// we get the full mode size which for windowed mode is the entire screen and not just the window
	// Problem that we can only get front buffer in A8R8G8B8
	mprintf(("Creating surface for front buffer of size: %d %d", mode.Width, mode.Height));
	if(FAILED(GlobalD3DVars::lpD3DDevice->CreateImageSurface(
		mode.Width, mode.Height, D3DFMT_A8R8G8B8, &front_buffer_a8r8g8b8))) {

		DBUGFILE_OUTPUT_0("Failed to create image surface");
		return -1;
	}

	if(FAILED(GlobalD3DVars::lpD3DDevice->GetFrontBuffer(front_buffer_a8r8g8b8))) {

		DBUGFILE_OUTPUT_0("Failed to get front buffer");
		goto Failed;
	}

	// Get the back buffer format
	PIXELFORMAT dest_format;
	color_gun r_gun, g_gun, b_gun, a_gun;

	d3d_fill_pixel_format( &dest_format, GlobalD3DVars::d3dpp.BackBufferFormat);
	d3d_setup_format_components(&dest_format, &r_gun, &g_gun, &b_gun, &a_gun);


	// Create a surface of a compatable type
	if(FAILED(GlobalD3DVars::lpD3DDevice->CreateImageSurface(
		gr_screen.max_w, gr_screen.max_h, 
		GlobalD3DVars::d3dpp.BackBufferFormat, &Gr_saved_surface))) {

		DBUGFILE_OUTPUT_0("Failed to create image surface");
		goto Failed;
	}

	// Make a copy of the thing
	D3DLOCKED_RECT src_rect;
	D3DLOCKED_RECT dst_rect;
	
	if(FAILED(Gr_saved_surface->LockRect(&dst_rect, NULL, 0))) { 

		DBUGFILE_OUTPUT_0("Failed to lock save buffer");
		goto Failed;

	}

	RECT rct;

	if (GlobalD3DVars::D3D_window) {
		POINT pnt;
		pnt.x = pnt.y = 0;

		HWND wnd = (HWND)os_get_window();
		ClientToScreen(wnd, &pnt);

		rct.left = pnt.x;
		rct.top = pnt.y;

		if(pnt.x < 0) {
			rct.left = 0;
		}

		if(pnt.y < 0) {
			rct.top = 0;
		}

		//We can't write to anything larger than the desktop resolution, so check against it
		if((UINT)(pnt.x + gr_screen.max_w) > mode.Width) {
			rct.right = mode.Width;
		}
		else {
			rct.right = pnt.x + gr_screen.max_w;
		}

		//And again
		if((UINT)(pnt.y + gr_screen.max_h) > mode.Height) {
			rct.bottom = mode.Height;
		}
		else{
			rct.bottom = pnt.y + gr_screen.max_h;
		}

	} else {
		rct.left = rct.top = 0;
		rct.right = gr_screen.max_w;
		rct.bottom = gr_screen.max_h;
	}

	//Find the total size of our rectangle
	rect_size_x = rct.right - rct.left;
	rect_size_y = rct.bottom - rct.top;


	if(FAILED(front_buffer_a8r8g8b8->LockRect(&src_rect, &rct, D3DLOCK_READONLY))) {

		DBUGFILE_OUTPUT_0("Failed to lock front buffer");
		goto Failed;

	}

	typedef struct { unsigned char b,g,r,a; } TmpC;

	if(gr_screen.bits_per_pixel == 32) {
		for(int j = 0; j < (rect_size_y); j++) {
		
			TmpC *src = (TmpC *)  (((char *) src_rect.pBits) + (src_rect.Pitch * j)); 
			uint *dst = (uint *) (((char *) dst_rect.pBits) + (dst_rect.Pitch * j));
		
			for(int i = 0; i < (rect_size_x); i++) {
			 	dst[i] = 0;
				dst[i] |= (uint)(( (int) src[i].r / r_gun.scale ) << r_gun.shift);
				dst[i] |= (uint)(( (int) src[i].g / g_gun.scale ) << g_gun.shift);
				dst[i] |= (uint)(( (int) src[i].b / b_gun.scale ) << b_gun.shift);
			}
		}
	} else {
		for(int j = 0; j < (rect_size_y); j++) {
		
			TmpC   *src = (TmpC *)  (((char *) src_rect.pBits) + (src_rect.Pitch * j)); 
			ushort *dst = (ushort *) (((char *) dst_rect.pBits) + (dst_rect.Pitch * j));
		
			for(int i = 0; i < (rect_size_x); i++) {
			 	dst[i] = 0;
				dst[i] |= (ushort)(( (int) src[i].r / r_gun.scale ) << r_gun.shift);
				dst[i] |= (ushort)(( (int) src[i].g / g_gun.scale ) << g_gun.shift);
				dst[i] |= (ushort)(( (int) src[i].b / b_gun.scale ) << b_gun.shift);
			}
		}
	}

	front_buffer_a8r8g8b8->UnlockRect();
	Gr_saved_surface->UnlockRect();

	if(front_buffer_a8r8g8b8) {
		front_buffer_a8r8g8b8->Release();
	}

	return 0;

Failed:

	if(front_buffer_a8r8g8b8) {
		front_buffer_a8r8g8b8->Release();
	}

	extern void gr_d3d_free_screen(int id);
	gr_d3d_free_screen(0);
	return -1;
}

void gr_d3d_restore_screen(int id)
{
	gr_reset_clip();

	if ( !Gr_saved_surface )	{
		gr_clear();
		return;
	}

	// attempt to replace DX5 code with DX8 
	IDirect3DSurface8 *dest_buffer = NULL;
		
	if(FAILED(GlobalD3DVars::lpD3DDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &dest_buffer)))
	{
		DBUGFILE_OUTPUT_0("FAiled");
		return;
	}

	if(FAILED(GlobalD3DVars::lpD3DDevice->CopyRects(Gr_saved_surface, NULL, 0, dest_buffer, NULL)))
	{
		DBUGFILE_OUTPUT_0("FAiled");
	}

	dest_buffer->Release();
}

void gr_d3d_free_screen(int id)
{
	if ( Gr_saved_surface )	{
		Gr_saved_surface->Release();
		Gr_saved_surface = NULL;
	}
}

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
		D3d_dump_buffer = (ubyte *)vm_malloc(size);
		if ( !D3d_dump_buffer )	{
			Error(LOCATION, "Unable to malloc %d bytes for dump buffer", size );
		}
	}
}

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
			/*
			ubyte *src_ptr = D3d_dump_buffer+(i*D3d_dump_frame_size)+(j*w*2);

			int len = tga_compress( (char *)outrow, (char *)src_ptr, w*sizeof(short) );
			*/
			int len = 0;

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
		vm_free(D3d_dump_buffer);
		D3d_dump_buffer = NULL;
	}
}

void gr_d3d_dump_frame()
{
	D3d_dump_frame_count++;

	if ( D3d_dump_frame_count == D3d_dump_frame_count_max ) {
		gr_d3d_flush_frame_dump();
	}
}	

/**
 * Empty function
 *
 * @return uint, always 1
 */
uint gr_d3d_lock()
{
	return 1;
}

/**
 * Empty function
 *
 * @return void
 */
void gr_d3d_unlock()
{
}

/**
 * Set fog
 *
 * @param int fog_mode 
 * @param int r 
 * @param int g 
 * @param int b 
 * @param float fog_near 
 * @param float fog_far
 * @return void
 */
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
			d3d_SetRenderState(D3DRS_FOGENABLE, FALSE );	
		}
		gr_screen.current_fog_mode = fog_mode;

		// to prevent further state changes
		return;
	}

	// maybe switch fogging on
	if(gr_screen.current_fog_mode != fog_mode){		
		d3d_SetRenderState(D3DRS_FOGENABLE, TRUE);	

		// if we're using table fog, enable table fogging
		if(!Cmdline_nohtl){

			if(GlobalD3DVars::d3d_caps.RasterCaps & D3DPRASTERCAPS_FOGTABLE)
			{
				d3d_SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_LINEAR );
			}
			else 
			{
				d3d_SetRenderState( D3DRS_FOGTABLEMODE,   D3DFOG_NONE );
				d3d_SetRenderState( D3DRS_FOGVERTEXMODE,  D3DFOG_LINEAR);

		  		if(GlobalD3DVars::d3d_caps.RasterCaps & D3DPRASTERCAPS_FOGRANGE)
		  	  		d3d_SetRenderState(D3DRS_RANGEFOGENABLE, TRUE);
			}
		}

		gr_screen.current_fog_mode = fog_mode;	
	}	

	// is color changing?
	if( (gr_screen.current_fog_color.red != r) || (gr_screen.current_fog_color.green != g) || (gr_screen.current_fog_color.blue != b) ){
		// store the values
		gr_init_color( &gr_screen.current_fog_color, r, g, b );

		color = D3DCOLOR_XRGB(r, g, b);
		d3d_SetRenderState(D3DRS_FOGCOLOR, color);	
	}		

	// planes changing?
	if( (fog_near >= 0.0f) && (fog_far >= 0.0f) && ((fog_near != gr_screen.fog_near) || (fog_far != gr_screen.fog_far)) ){		
		gr_screen.fog_near = fog_near;		
		gr_screen.fog_far = fog_far;   
				
		// only generate a new fog table if we have to (wfog/table fog mode)
		if(!Cmdline_nohtl){
			d3d_SetRenderState( D3DRS_FOGSTART, *((DWORD *)(&fog_near)));		
			d3d_SetRenderState( D3DRS_FOGEND, *((DWORD *)(&fog_far)));
		}				
	}  
}

/**
 * Support function for gr_d3d_set_gamma 
 *
 */
inline ushort d3d_ramp_val(UINT i, float recip_gamma, int scale = 65535)
{
    return static_cast<ushort>(scale*pow(i/255.f, 1.0f/recip_gamma));
}

/**
 * Set the gamma, or brightness
 *
 * @param float gamma
 * @return void
 */
void gr_d3d_set_gamma(float gamma)
{
	if(Cmdline_no_set_gamma) return;

	Gr_gamma = gamma;
	D3DGAMMARAMP gramp;

	// Create the Gamma lookup table
	for (int i = 0; i < 256; i++ ) {
	  	gramp.red[i] = gramp.green[i] = gramp.blue[i] = d3d_ramp_val(i, gamma);
	}

   	GlobalD3DVars::lpD3DDevice->SetGammaRamp(D3DSGR_CALIBRATE, &gramp);
}

/**
 * Toggle polygon culling mode Counter-clockwise or none
 *
 * @param int cull 
 * @return void
 */
void gr_d3d_set_cull(int cull)
{
	d3d_SetRenderState( D3DRS_CULLMODE, cull ? D3DCULL_CCW : D3DCULL_NONE );				
}

void gr_d3d_set_fill_mode(int mode)
{
	if(mode == GR_FILL_MODE_SOLID){
		d3d_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		return;
	}
	if(mode == GR_FILL_MODE_WIRE){
		d3d_SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		return;
	}
	//defalt value
	d3d_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

}


/**
 * Cross fade, actually works now
 *
 * @param int bmap1 
 * @param int bmap2 
 * @param int x1 
 * @param int y1 
 * @param int x2 
 * @param int y2 
 * @param float pct	- Fade value, between 0.0 - 1.0
 * @return void
 */
void gr_d3d_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct)
{
   	gr_set_bitmap(bmap1, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0f - pct );
	gr_bitmap(x1, y1);

  	gr_set_bitmap(bmap2, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, pct );
	gr_bitmap(x2, y2);
}

/**
 * Empty function
 *
 * @param int filter
 * @return void
 */
void gr_d3d_filter_set(int filter)
{
}

/**
 * Set clear color
 *
 * @param int r
 * @param int g
 * @param int b
 * @return void
 */
void gr_d3d_set_clear_color(int r, int g, int b)
{
	gr_init_color(&gr_screen.current_clear_color, r, g, b);
}

// Not sure if we need this any more
void gr_d3d_get_region(int front, int w, int h, ubyte *data)
{	
	HRESULT hr;

	// No support for getting the front buffer
	if(front) {
		mprintf(("No support for front buffer"));
		return;
	}

	IDirect3DSurface8 *back_buffer = NULL;

	hr = GlobalD3DVars::lpD3DDevice->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &back_buffer);
	if( FAILED(hr))
	{
		mprintf(("Unsuccessful GetBackBuffer",d3d_error_string(hr)));
		return;
	}

	D3DLOCKED_RECT buffer_desc;
	hr = back_buffer->LockRect(&buffer_desc, NULL, D3DLOCK_READONLY );
	if( FAILED(hr))
	{
		mprintf(("Unsuccessful buffer lock",d3d_error_string(hr)));
		return;
	}

	ubyte *dptr = data;	
	ubyte *rptr = (ubyte*) buffer_desc.pBits;  
	int pitch   = buffer_desc.Pitch;// / gr_screen.bytes_per_pixel;   

	for (int i=0; i<h; i++ )	{
		ubyte *sptr = (ubyte*)&rptr[ i * pitch ];

		// don't think we need to swizzle here ...
		for(int j=0; j<w; j++ )	{
		  	memcpy(dptr, sptr, gr_screen.bytes_per_pixel);
			dptr += gr_screen.bytes_per_pixel;
			sptr += gr_screen.bytes_per_pixel;

		}
	}	

	back_buffer->UnlockRect();
	back_buffer->Release();
}

//*******Vertex buffer stuff*******//
int vertex_size(uint flags){
	int size = 0;
	Assert(! ((flags & VERTEX_FLAG_RHW) && (flags & VERTEX_FLAG_NORMAL)));
	if(flags & VERTEX_FLAG_UV1)Assert(! ((flags & VERTEX_FLAG_UV2) || (flags & VERTEX_FLAG_UV3) || (flags & VERTEX_FLAG_UV4)));
	if(flags & VERTEX_FLAG_UV2)Assert(! ((flags & VERTEX_FLAG_UV1) || (flags & VERTEX_FLAG_UV3) || (flags & VERTEX_FLAG_UV4)));
	if(flags & VERTEX_FLAG_UV3)Assert(! ((flags & VERTEX_FLAG_UV2) || (flags & VERTEX_FLAG_UV1) || (flags & VERTEX_FLAG_UV4)));
	if(flags & VERTEX_FLAG_UV4)Assert(! ((flags & VERTEX_FLAG_UV2) || (flags & VERTEX_FLAG_UV3) || (flags & VERTEX_FLAG_UV1)));

	if(flags & VERTEX_FLAG_POSITION)size	+= sizeof(vec3d);
	if(flags & VERTEX_FLAG_RHW)size			+= sizeof(float);
	if(flags & VERTEX_FLAG_NORMAL)size		+= sizeof(vec3d);
	if(flags & VERTEX_FLAG_DIFUSE)size		+= sizeof(DWORD);
	if(flags & VERTEX_FLAG_SPECULAR)size	+= sizeof(DWORD);
	if(flags & VERTEX_FLAG_UV1)size			+= sizeof(float)*2;
	else if(flags & VERTEX_FLAG_UV2)size	+= sizeof(float)*4;
	else if(flags & VERTEX_FLAG_UV3)size	+= sizeof(float)*6;
	else if(flags & VERTEX_FLAG_UV4)size	+= sizeof(float)*8;

	return size;
}

//D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1
int convert_to_fvf(uint flags){
	int fvf = 0;
	Assert(! ((flags & VERTEX_FLAG_RHW) && (flags & VERTEX_FLAG_NORMAL)));
	if(flags & VERTEX_FLAG_UV1)Assert(! ((flags & VERTEX_FLAG_UV2) || (flags & VERTEX_FLAG_UV3) || (flags & VERTEX_FLAG_UV4)));
	if(flags & VERTEX_FLAG_UV2)Assert(! ((flags & VERTEX_FLAG_UV1) || (flags & VERTEX_FLAG_UV3) || (flags & VERTEX_FLAG_UV4)));
	if(flags & VERTEX_FLAG_UV3)Assert(! ((flags & VERTEX_FLAG_UV2) || (flags & VERTEX_FLAG_UV1) || (flags & VERTEX_FLAG_UV4)));
	if(flags & VERTEX_FLAG_UV4)Assert(! ((flags & VERTEX_FLAG_UV2) || (flags & VERTEX_FLAG_UV3) || (flags & VERTEX_FLAG_UV1)));

	if(flags & VERTEX_FLAG_POSITION)	fvf |= D3DFVF_XYZ;
	if(flags & VERTEX_FLAG_RHW)			fvf |= D3DFVF_XYZRHW;
	if(flags & VERTEX_FLAG_NORMAL)		fvf |= D3DFVF_NORMAL;
	if(flags & VERTEX_FLAG_DIFUSE)		fvf |= D3DFVF_DIFFUSE;
	if(flags & VERTEX_FLAG_SPECULAR)	fvf |= D3DFVF_SPECULAR;
	if(flags & VERTEX_FLAG_UV1)			fvf |= D3DFVF_TEX1;
	else if(flags & VERTEX_FLAG_UV2)	fvf |= D3DFVF_TEX2;
	else if(flags & VERTEX_FLAG_UV3)	fvf |= D3DFVF_TEX3;
	else if(flags & VERTEX_FLAG_UV4)	fvf |= D3DFVF_TEX4;
	return fvf;
}

#define fill_v(V,v) {(*((float *) (V))) = v; V = ((byte*)(V)) + sizeof(float);}

vec3d *check_vec1, *check_vec2;
void fill_vert(void *V, vertex *L, vec3d* N, uint flags){
				if(flags & VERTEX_FLAG_RHW){
					fill_v(V, L->sx);
					fill_v(V, L->sy);
					fill_v(V, L->sw);
					fill_v(V, L->sw);
				}else
				if(flags & VERTEX_FLAG_POSITION){
					check_vec1 = (vec3d*)V;
					fill_v(V, L->x);
					fill_v(V, L->y);
					fill_v(V, L->z);
				}
				if(flags & VERTEX_FLAG_NORMAL){
					check_vec2 = (vec3d*)V;
					fill_v(V, N->xyz.x);
					fill_v(V, N->xyz.y);
					fill_v(V, N->xyz.z);
				}
				if(flags & VERTEX_FLAG_DIFUSE){
					*(byte*)(V = ((byte*)V)+1 ) = L->a;
					*(byte*)(V = ((byte*)V)+1 ) = L->r;
					*(byte*)(V = ((byte*)V)+1 ) = L->g;
					*(byte*)(V = ((byte*)V)+1 ) = L->b;
				}
				if(flags & VERTEX_FLAG_SPECULAR){
					*(byte*)(V = ((byte*)V)+1 ) = L->spec_a;
					*(byte*)(V = ((byte*)V)+1 ) = L->spec_r;
					*(byte*)(V = ((byte*)V)+1 ) = L->spec_g;
					*(byte*)(V = ((byte*)V)+1 ) = L->spec_b;
				}
				if(flags & VERTEX_FLAG_UV1){
					fill_v(V, L->u);
					fill_v(V, L->v);
				}else
				if(flags & VERTEX_FLAG_UV2){
					fill_v(V, L->u);
					fill_v(V, L->v);
					fill_v(V, L->u2);
					fill_v(V, L->v2);
				}else
				if(flags & VERTEX_FLAG_UV3){
					fill_v(V, L->u);
					fill_v(V, L->v);
					fill_v(V, L->u2);
					fill_v(V, L->v2);
					fill_v(V, L->u3);
					fill_v(V, L->v3);
				}else
				if(flags & VERTEX_FLAG_UV4){
					fill_v(V, L->u);
					fill_v(V, L->v);
					fill_v(V, L->u2);
					fill_v(V, L->v2);
					fill_v(V, L->u3);
					fill_v(V, L->v3);
					fill_v(V, L->u4);
					fill_v(V, L->v4);
				}
}

//makes the vertex buffer, returns an index to it
int gr_d3d_make_buffer(poly_list *list, uint flags)
{
	int k;
	byte *v;
	Vertex_buffer new_buffer;

	new_buffer.size = vertex_size(flags);
	new_buffer.FVF = convert_to_fvf(flags);

//	d3d_CreateVertexBuffer(D3DVT_VERTEX, (list->n_verts), NULL, (void**)buffer);
		
	k = GlobalD3DVars::lpD3DDevice->CreateVertexBuffer(	
			new_buffer.size * (list->n_verts ? list->n_verts : 1), 
			D3DUSAGE_WRITEONLY, 
			new_buffer.FVF,
			D3DPOOL_MANAGED,
			&new_buffer.buffer);

	switch (k)
	{
		case D3DERR_INVALIDCALL:
			Error(LOCATION, "CreateVertexBuffer returned D3DERR_INVALIDCALL.");
			break;

		case D3DERR_OUTOFVIDEOMEMORY:
			Error(LOCATION, "CreateVertexBuffer returned D3DERR_OUTOFVIDEOMEMORY");
			break;

		case E_OUTOFMEMORY:
			Error(LOCATION, "CreateVertexBuffer returned E_OUTOFMEMORY");
			break;

		default:
			break;
	}

	if (new_buffer.buffer == NULL)
		return -1;

	new_buffer.buffer->Lock(0, 0, &v, 0);

	for(k = 0; k<list->n_verts; k++){
			fill_vert(&v[k * new_buffer.size],  &list->vert[k], &list->norm[k], flags);
			new_buffer.n_verts++;
	}

	new_buffer.buffer->Unlock();

	new_buffer.n_verts  = list->n_verts;
	new_buffer.n_prim  = list->n_verts;
	new_buffer.type = TRILIST_;

	D3D_vertex_buffers.push_back( new_buffer );
	D3D_vertex_buffers_in_use++;

	return (int)(D3D_vertex_buffers.size() - 1);
}

//makes the vertex buffer, returns an index to it
int gr_d3d_make_flat_buffer(poly_list *list)
{
	int k;
	D3DLVERTEX *v, *V;
	vertex *L;
//	vec3d *N;
	Vertex_buffer new_buffer;

	IDirect3DVertexBuffer8 **buffer = &new_buffer.buffer;

	d3d_CreateVertexBuffer(D3DVT_LVERTEX, (list->n_verts), NULL, (void**)buffer);

	new_buffer.buffer->Lock(0, 0, (BYTE **)&v, NULL);

	for (k = 0; k < list->n_verts; k++) {
		V = &v[k];
		L = &list->vert[k];
	//	N = &list->norm[k]; //these don't have normals :\

		V->sx = L->x;
		V->sy = L->y;
		V->sz = L->z;

		V->tu = L->u;
		V->tv = L->v;

		V->color = D3DCOLOR_ARGB(255,L->r,L->g,L->b);
	}

	new_buffer.buffer->Unlock();

	new_buffer.n_prim  = list->n_verts/3;
	new_buffer.type = FLAT_;

	D3D_vertex_buffers.push_back( new_buffer );
	D3D_vertex_buffers_in_use++;

	return (int)(D3D_vertex_buffers.size() - 1);
}


//makes line buffer, returns index
int gr_d3d_make_line_buffer(line_list *list){
/*
	int idx = find_first_empty_buffer();

	if(idx > -1){
		IDirect3DVertexBuffer8 **buffer = &vertex_buffer[idx].buffer;

		d3d_CreateVertexBuffer(D3DVT_LVERTEX, (list->n_line*2), NULL, (void**)buffer);

		D3DLVERTEX *v, *V;
		vertex *L;
//		vec3d *N;
		int c = 0;

		vertex_buffer[idx].buffer->Lock(0, 0, (BYTE **)&v, NULL);
		for(int k = 0; k<list->n_line; k++){
			for(int j = 0; j < 2; j++){
				V = &v[(k*2)+j];
				L = &list->vert[k][j];

				V->sx = L->x;
				V->sy = L->y;
				V->sz = L->z;

				V->tu = L->u;
				V->tv = L->v;

				V->color = D3DCOLOR_ARGB(255,L->r,L->g,L->b);
				c++;
			}
		}

		vertex_buffer[idx].buffer->Unlock();

		vertex_buffer[idx].ocupied = true;
		vertex_buffer[idx].n_prim  = list->n_line;
		vertex_buffer[idx].type = LINELIST_;
	}
//	return-1;
	return idx;
	*/
	return-1;

}
	
//kills buffers dead!
void gr_d3d_destroy_buffer(int idx)
{
	if ( (idx < 0) || (idx >= (int)D3D_vertex_buffers.size()) )
		return;

	Vertex_buffer *vbp = &D3D_vertex_buffers[idx];

	if (vbp->buffer != NULL)
		vbp->buffer->Release();

	memset( vbp, 0, sizeof(Vertex_buffer) );

	// we try to take advantage of the fact that there shouldn't be a lot of buffer
	// deletions/additions going on all of the time, so a model_unload_all() and/or
	// game_level_close() should pretty much keep everything cleared out on a
	// regular basis
	if (--D3D_vertex_buffers_in_use <= 0)
		D3D_vertex_buffers.clear();
}

//enum vertex_buffer_type{TRILIST_,LINELIST_,FLAT_};

void gr_d3d_render_line_buffer(int idx)
{
	if ( (idx < 0) || (idx >= (int)D3D_vertex_buffers.size()) )
		return;

	if (D3D_vertex_buffers[idx].buffer == NULL)
		return;

	d3d_SetVertexShader(vertex_types[D3DVT_LVERTEX].fvf);

	GlobalD3DVars::lpD3DDevice->SetStreamSource(0, D3D_vertex_buffers[idx].buffer, sizeof(D3DLVERTEX));
	
	gr_d3d_fog_set(GR_FOGMODE_NONE, 0,0,0, gr_screen.fog_near, gr_screen.fog_far);		//it's a HUD item, should never be fogged

	gr_d3d_set_cull(0);
	d3d_SetRenderState(D3DRS_LIGHTING , FALSE);
	GlobalD3DVars::lpD3DDevice->DrawPrimitive(D3DPT_LINELIST , 0, D3D_vertex_buffers[idx].n_prim);
	d3d_SetRenderState(D3DRS_LIGHTING , TRUE);
	gr_d3d_set_cull(1);
}

extern int GR_center_alpha;
bool the_lights_are_on;
extern bool lighting_enabled;
void gr_d3d_center_alpha_int(int type);
Vertex_buffer *set_buffer;

void gr_d3d_set_buffer(int idx)
{
	set_buffer = NULL;

	if ( (idx < 0) || (idx >= (int)D3D_vertex_buffers.size()) )
		return;

	set_buffer = &D3D_vertex_buffers[idx];

	d3d_SetVertexShader(set_buffer->FVF);
	GlobalD3DVars::lpD3DDevice->SetStreamSource(0, set_buffer->buffer, set_buffer->size);
}

IDirect3DIndexBuffer8 *global_index_buffer = NULL;
int index_buffer_size = 0;
IDirect3DIndexBuffer8 *global_index_buffer32 = NULL;
int index_buffer_size32 = 0;


void gr_d3d_render_buffer(int start, int n_prim, ushort* index_buffer, uint *ibuf32, int flags)
{
	if(set_buffer == NULL)return;
	if(index_buffer != NULL && ibuf32 !=NULL){
		Error(LOCATION, "gr_d3d_render_buffer was given TWO indext buffers, that's not cool man!\n only useing the 16 bit one");
		ibuf32=NULL;
	}
	if(index_buffer != NULL || ibuf32 !=NULL){
		if(index_buffer){
			if(index_buffer_size < n_prim * 3 || !global_index_buffer){
				if(global_index_buffer)global_index_buffer->Release();
				GlobalD3DVars::lpD3DDevice->CreateIndexBuffer(n_prim * 3 * sizeof(ushort), D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, (IDirect3DIndexBuffer8**) &global_index_buffer);
				index_buffer_size = n_prim * 3;
			}
			ushort* i_buffer;
			global_index_buffer->Lock(0, 0, (BYTE **)&i_buffer, D3DLOCK_DISCARD);
			memcpy(i_buffer, index_buffer, n_prim*3*sizeof(ushort));
			global_index_buffer->Unlock();
			GlobalD3DVars::lpD3DDevice->SetIndices(global_index_buffer, 0);
		}
		if(ibuf32) {
			if(index_buffer_size32 < n_prim * 3 || !global_index_buffer32){
				if(global_index_buffer32)global_index_buffer32->Release();
				GlobalD3DVars::lpD3DDevice->CreateIndexBuffer(n_prim * 3 * sizeof(uint), D3DUSAGE_DYNAMIC|D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_DEFAULT, (IDirect3DIndexBuffer8**) &global_index_buffer32);
				index_buffer_size32 = n_prim * 3;
			}
			uint* i_buffer32;
			global_index_buffer32->Lock(0, 0, (BYTE **)&i_buffer32, D3DLOCK_DISCARD);
			memcpy(i_buffer32, ibuf32, n_prim*3*sizeof(uint));
			global_index_buffer32->Unlock();
			GlobalD3DVars::lpD3DDevice->SetIndices(global_index_buffer32, 0);
		}
	//	global_index_buffer->Lock(start, n_prim * 3 * sizeof(short), (BYTE **)&index_buffer, D3DLOCK_DISCARD);
	}
//	GlobalD3DVars::d3d_caps.MaxActiveLights = 1;

//	if(!the_lights_are_on){
//		d3d_SetVertexShader(D3DVT_VERTEX);
//		d3d_SetRenderState(D3DRS_LIGHTING , TRUE);
//		the_lights_are_on = true;
//	}												  

	extern D3DMATERIAL8 material;
	// Sets the current alpha of the object
	if(gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER){
		material.Ambient.a = gr_screen.current_alpha;
		material.Diffuse.a = gr_screen.current_alpha;
		material.Specular.a = gr_screen.current_alpha;
		material.Emissive.a = gr_screen.current_alpha;
	}else{
		material.Ambient.a = 1.0;
		material.Diffuse.a = 1.0;
		material.Specular.a = 1.0;
		material.Emissive.a = 1.0f;
	}
	if(!lighting_enabled){
		int l = int(255.0f*gr_screen.current_alpha);
		d3d_SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(l,l,l,l));
	}else{
		d3d_SetRenderState(D3DRS_AMBIENT, ambient_light);
	}
	GlobalD3DVars::lpD3DDevice->SetMaterial(&material);

	color old_fog_color = gr_screen.current_fog_color;

	if(gr_screen.current_fog_mode != GR_FOGMODE_NONE)//when fogging don't fog unlit things, but rather fade them in a fog like manner -Bobboau
		if(!lighting_enabled){
			gr_d3d_fog_set(gr_screen.current_fog_mode, 0,0,0, gr_screen.fog_near, gr_screen.fog_far);
		}


		if (set_buffer->buffer == NULL) {
			return;
		}
/*	if(set_buffer->type == LINELIST_) {
		gr_d3d_render_line_buffer(idx); 
		return;
	}
*/
	float u_scale = 1.0f, v_scale = 1.0f;

	gr_alpha_blend ab = ALPHA_BLEND_NONE;
	if(gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER)	
		ab = ALPHA_BLEND_ALPHA_ADDITIVE;

//	int same = (gr_screen.current_bitmap != SPECMAP)?0:1;
//	if(!same)d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, 0, 0);
	d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, 0, 0);
//	if(!gr_zbuffering_mode)
//		gr_d3d_set_state(TEXTURE_SOURCE_DECAL, ab, ZBUFFER_TYPE_NONE);
	if(gr_zbuffering_mode == GR_ZBUFF_NONE){
			gr_d3d_set_state(TEXTURE_SOURCE_DECAL, ab, ZBUFFER_TYPE_NONE);
			d3d_SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	}else if(gr_zbuffering_mode == GR_ZBUFF_READ){
			gr_d3d_set_state(TEXTURE_SOURCE_DECAL, ab, ZBUFFER_TYPE_READ);
			d3d_SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
	}else{
			gr_d3d_set_state(TEXTURE_SOURCE_DECAL, ab, ZBUFFER_TYPE_DEFAULT);
	}

	pre_render_lights_init();
	if(lighting_enabled){
		shift_active_lights(0);
	}

//	bool single_pass_spec = false;

	if(GLOWMAP > -1 && !Cmdline_noglow){
		//glowmapped
			gr_screen.gf_set_bitmap(GLOWMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0);
		 	d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, 0, 1);
			
	 		
			set_stage_for_glow_mapped_defuse();
	}else{
		//non glowmapped
			set_stage_for_defuse();
	}

	int passes = (n_active_lights / GlobalD3DVars::d3d_caps.MaxActiveLights);
//	d3d_SetVertexShader(D3DVT_VERTEX);

	gr_d3d_center_alpha_int(GR_center_alpha);
//	if(!lighting_enabled)		d3d_SetRenderState(D3DRS_LIGHTING , FALSE);
	if(index_buffer != NULL || ibuf32 != NULL)GlobalD3DVars::lpD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,set_buffer->n_verts, start, n_prim);
	else GlobalD3DVars::lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST , start, n_prim);
//	if(!lighting_enabled)		d3d_SetRenderState(D3DRS_LIGHTING , TRUE);


	if(!lighting_enabled)
	{
		return;
	}
	//single pass specmap rendering ends here
/*	if(single_pass_spec){
		return;
	}*/
	if (gr_zbuffering_mode)
	{
		gr_d3d_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_READ );
	}
	else
		gr_d3d_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_NONE );
		

	if(gr_screen.current_fog_mode != GR_FOGMODE_NONE){
		old_fog_color = gr_screen.current_fog_color;
		gr_d3d_fog_set(gr_screen.current_fog_mode, 0,0,0, gr_screen.fog_near, gr_screen.fog_far);
	}

//	if(single_pass_spec)set_stage_for_single_pass_specmapping(same);
//	else set_stage_for_defuse();
	set_stage_for_defuse();


	d3d_SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(0,0,0,0));
	for(int i = 1; i<passes; i++){
		shift_active_lights(i);
		TIMERBAR_PUSH(7);
		if(index_buffer != NULL || ibuf32 != NULL)GlobalD3DVars::lpD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,set_buffer->n_verts, start, n_prim);
		else GlobalD3DVars::lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST , start, n_prim);
		TIMERBAR_POP();
	}
	if(!lighting_enabled){
		int l = int(255.0f*gr_screen.current_alpha);
		d3d_SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(l,l,l,l));
	}else{
		d3d_SetRenderState(D3DRS_AMBIENT, ambient_light);
	}
								    
	pre_render_lights_init();
	shift_active_lights(0);

	//spec mapping
	if(SPECMAP > -1){
		gr_zbias(1);
		gr_screen.gf_set_bitmap(SPECMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0);
		if ( !d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, 0, 0))	{
				mprintf(( "Not rendering specmap texture because it didn't fit in VRAM!\n" ));
			//	Error(LOCATION, "Not rendering specmap texture because it didn't fit in VRAM!");
				return;
			}

		if(set_stage_for_spec_mapped()){
			gr_d3d_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_READ );
			if(index_buffer != NULL || ibuf32 != NULL)GlobalD3DVars::lpD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,set_buffer->n_verts, start, n_prim);
			else GlobalD3DVars::lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST , start, n_prim);
			d3d_SetRenderState(D3DRS_AMBIENT, D3DCOLOR_ARGB(0,0,0,0));
			for(int i = 1; i<passes; i++){
				shift_active_lights(i);
				if(index_buffer != NULL || ibuf32 != NULL)GlobalD3DVars::lpD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,set_buffer->n_verts, start, n_prim);
				else GlobalD3DVars::lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST , start, n_prim);
			}
			gr_d3d_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_NONE, ZBUFFER_TYPE_FULL );
			if(Cmdline_env){
				gr_zbias(2);

				extern int Game_subspace_effect;
				gr_screen.gf_set_bitmap(ENVMAP, gr_screen.current_alphablend_mode, gr_screen.current_bitblt_mode, 0.0);
				d3d_tcache_set_internal(gr_screen.current_bitmap, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 0, 0, 1);

				gr_d3d_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_READ );
				set_stage_for_env_mapped();
				d3d_SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
				if(index_buffer != NULL || ibuf32 != NULL)GlobalD3DVars::lpD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,0,set_buffer->n_verts, start, n_prim);
				else GlobalD3DVars::lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST , start, n_prim);
				d3d_SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
			}
		}
		gr_zbias(0);
	}

	// Revert back to old fog state
	if(gr_screen.current_fog_mode != GR_FOGMODE_NONE)
		gr_d3d_fog_set(gr_screen.current_fog_mode, old_fog_color.red,old_fog_color.green,old_fog_color.blue, gr_screen.fog_near, gr_screen.fog_far);

}

//*******matrix stuff*******//

/*
	//the projection matrix; fov, aspect ratio, near, far
 	void (*gf_set_proj_matrix)(float, float, float, float);
  	void (*gf_end_proj_matrix)();
	//the view matrix
 	void (*gf_set_view_matrix)(vec3d *, matrix*);
  	void (*gf_end_view_matrix)();
	//object scaleing
	void (*gf_push_scale_matrix)(vec3d *);
 	void (*gf_pop_scale_matrix)();
	//object position and orientation
	void (*gf_start_instance_matrix)(vec3d *, matrix*);
	void (*gf_start_angles_instance_matrix)(vec3d *, angles*);
	void (*gf_end_instance_matrix)();
*/

//fov = (4.0/9.0)*(PI)*View_zoom
//ratio = screen.aspect
//near = 1.0
//far = 30000
float fov_corection = 1.0f;
	//the projection matrix; fov, aspect ratio, near, far
void gr_d3d_set_proj_matrix(float fov, float ratio, float n, float f){
//	proj_matrix_stack->Push();
	fov *= fov_corection;
	D3DXMATRIX mat;
	D3DXMatrixPerspectiveFovLH(&mat, fov, ratio, n, f);
//	D3DXMatrixPerspectiveFovLH(&mat, (4.0f/9.0f)*(D3DX_PI)*View_zoom, 1.0f/gr_screen.aspect, 0.2f, 30000);
//	proj_matrix_stack->LoadMatrix(&mat);
	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_PROJECTION, &mat);
}

void gr_d3d_end_proj_matrix(){
//	proj_matrix_stack->Pop();
}

//extern float global_scaleing_factor;
	//the view matrix
void gr_d3d_set_view_matrix(vec3d* offset, matrix *orient){

//	view_matrix_stack->Push();

	D3DXMATRIX mat, scale_m;

	D3DXMATRIX MAT(
		orient->vec.rvec.xyz.x, orient->vec.rvec.xyz.y, orient->vec.rvec.xyz.z, 0,
		orient->vec.uvec.xyz.x, orient->vec.uvec.xyz.y, orient->vec.uvec.xyz.z, 0,
		orient->vec.fvec.xyz.x, orient->vec.fvec.xyz.y, orient->vec.fvec.xyz.z, 0,
		offset->xyz.x, offset->xyz.y, offset->xyz.z, 1);

	D3DXMatrixIdentity(&mat);

//	D3DXMatrixScaling(&scale_m, 1/global_scaleing_factor, 1/global_scaleing_factor, 1/global_scaleing_factor);//global sacaleing
//	D3DXMatrixMultiply(&MAT, &MAT, &scale_m);

	D3DXMatrixInverse(&mat, NULL, &MAT);
	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_VIEW, &mat);
}

void gr_d3d_end_view_matrix(){
//	view_matrix_stack->Pop();
//	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_VIEW, view_matrix_stack->GetTop());
}
int matr_depth = 0;
	//object position and orientation
void gr_d3d_start_instance_matrix(vec3d* offset, matrix *orient){

	D3DXMATRIX old_world = *world_matrix_stack->GetTop(), scale_m;
	world_matrix_stack->Push();

	D3DXMATRIX world(
		orient->vec.rvec.xyz.x, orient->vec.rvec.xyz.y, orient->vec.rvec.xyz.z, 0,
		orient->vec.uvec.xyz.x, orient->vec.uvec.xyz.y, orient->vec.uvec.xyz.z, 0,
		orient->vec.fvec.xyz.x, orient->vec.fvec.xyz.y, orient->vec.fvec.xyz.z, 0,
		offset->xyz.x, offset->xyz.y, offset->xyz.z, 1);

//	D3DXMatrixScaling(&scale_m, global_scaleing_factor, global_scaleing_factor, global_scaleing_factor);//global sacaleing
//	D3DXMatrixMultiply(&world, &scale_m, &world);

	D3DXMatrixMultiply(&world, &world, &old_world);

	world_matrix_stack->LoadMatrix(&world);
	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_WORLD, world_matrix_stack->GetTop());
	matr_depth++;
}

void gr_d3d_start_angles_instance_matrix(vec3d* offset, angles *orient){

	D3DXMATRIX current = *world_matrix_stack->GetTop(), scale_m;
	world_matrix_stack->Push();

	D3DXMATRIX mat;
	D3DXMatrixRotationYawPitchRoll(&mat,orient->h,orient->p,orient->b);
	D3DXMATRIX world(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		offset->xyz.x, offset->xyz.y, offset->xyz.z, 1);

///	D3DXMatrixScaling(&scale_m, global_scaleing_factor, global_scaleing_factor, global_scaleing_factor);;//global sacaleing
//	D3DXMatrixMultiply(&world, &scale_m, &world);

	D3DXMatrixMultiply(&mat, &mat, &world);
	D3DXMatrixMultiply(&mat, &mat, &current);

	world_matrix_stack->LoadMatrix(&mat);
	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_WORLD, world_matrix_stack->GetTop());
	matr_depth++;

}

void gr_d3d_end_instance_matrix()
{
	world_matrix_stack->Pop();
	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_WORLD, world_matrix_stack->GetTop());
	matr_depth--;

}


	//object scaleing
void gr_d3d_set_scale_matrix(vec3d* scale){

	D3DXMATRIX mat = *world_matrix_stack->GetTop(), scale_m;
	world_matrix_stack->Push();

	D3DXMatrixScaling(&scale_m, scale->xyz.x, scale->xyz.y, scale->xyz.z);
	D3DXMatrixMultiply(&mat, &scale_m, &mat);

	world_matrix_stack->LoadMatrix(&mat);
	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_WORLD, &mat);

}

void gr_d3d_end_scale_matrix()
{
	world_matrix_stack->Pop();
	GlobalD3DVars::lpD3DDevice->SetTransform(D3DTS_WORLD, world_matrix_stack->GetTop());

}




/**
 * Turns on clip plane clip plane
 * Doenst seem to work at the moment
 *
 * @return void
 */
void gr_d3d_start_clip(){

	D3DXVECTOR3 point(G3_user_clip_point.xyz.x,G3_user_clip_point.xyz.y,G3_user_clip_point.xyz.z); 
	D3DXVECTOR3	normal(G3_user_clip_normal.xyz.x,G3_user_clip_normal.xyz.y,G3_user_clip_normal.xyz.z);

	D3DXPlaneFromPointNormal(&d3d_user_clip_plane, &point, &normal);

	HRESULT hr = GlobalD3DVars::lpD3DDevice->SetClipPlane(0, d3d_user_clip_plane);
  //	Assert(SUCCEEDED(hr));
	if(FAILED(hr))
	{
		mprintf(("Failed to set clip plane\n"));
	}

	hr = d3d_SetRenderState(D3DRS_CLIPPLANEENABLE , D3DCLIPPLANE0);
	Assert(SUCCEEDED(hr));
}

/**
 * Turns off clip plane
 *
 * @return void
 */
void gr_d3d_end_clip(){
	d3d_SetRenderState(D3DRS_CLIPPLANEENABLE , FALSE);
}

/**
 * Takes a D3D error and turns it into text, however for DX8 they have really reduced the 
 * number of error codes so the result may be quite general  
 *
 * @param HRESULT error
 * @return const char *
 */
const char *d3d_error_string(HRESULT error)
{
	return DXGetErrorString8(error);
}

//this is an attempt to get something more like the old style fogging that used the lockable back buffer, 
//it will render the model with the background texture then fog with black as the color
void gr_d3d_setup_background_fog(bool set){
	return;
/*	IDirect3DSurface8 *bg_surf;
	if(!background_render_target){
		GlobalD3DVars::lpD3DDevice->CreateTexture(
			1024, 1024,
			0, 
			D3DUSAGE_RENDERTARGET,
			D3DFMT_A8R8G8B8, 
			D3DPOOL_DEFAULT, &background_render_target);
	}
	background_render_target->GetSurfaceLevel(0,&bg_surf);
	if(set){
		GlobalD3DVars::lpD3DDevice->GetRenderTarget(&old_render_target);
		GlobalD3DVars::lpD3DDevice->SetRenderTarget(bg_surf , 0);
		gr_d3d_clear();
	}else{
	//	GlobalD3DVars::lpD3DDevice->GetRenderTarget(&background_render_target);
		GlobalD3DVars::lpD3DDevice->SetRenderTarget(old_render_target , 0);
	}*/
}


IDirect3DSurface8 *old_render_sten = NULL;
/*
extern D3DFORMAT d3d8_format;
LPDIRECT3DSURFACE8 face;

void d3d_init_environment(){
	if(!cube_map){
		D3DFORMAT use_format;
		use_format =  D3DFMT_X8R8G8B8;
		GlobalD3DVars::lpD3DDevice->CreateCubeTexture(512, 1, D3DUSAGE_RENDERTARGET , use_format, D3DPOOL_DEFAULT, &cube_map);

		// Check if it succeeds in allocating the cube map - it fails on a card that doesn't support
		// cubemaps, silly!
		if (NULL == cube_map) {
			mprintf(("Failed in creating a cube-map in d3d_init_environment!"));
			return;
		}
	}

	if(!old_render_target){
		GlobalD3DVars::lpD3DDevice->GetRenderTarget(&old_render_target);
		GlobalD3DVars::lpD3DDevice->GetDepthStencilSurface(&old_render_sten);
	}
	
	for(int i = 0; i<6; i++){
		cube_map->GetCubeMapSurface(_D3DCUBEMAP_FACES(i), 0, &face);
		GlobalD3DVars::lpD3DDevice->SetRenderTarget(face , NULL);
		gr_d3d_clear();
		int r = face->Release();
	}
	GlobalD3DVars::lpD3DDevice->SetRenderTarget(old_render_target, old_render_sten);

//	old_render_target = NULL;

}

LPDIRECT3DSURFACE8 env_face = NULL;
void d3d_render_to_env(int FACE){
	if(env_face){
		env_face->Release();
		env_face = NULL;
	}
	if(!cube_map){
		D3DFORMAT use_format;
		use_format =  D3DFMT_X8R8G8B8;
		GlobalD3DVars::lpD3DDevice->CreateCubeTexture(512, 1, D3DUSAGE_RENDERTARGET , use_format, D3DPOOL_DEFAULT, &cube_map);

		// Check if it succeeds in allocating the cube map - it fails on a card that doesn't support
		// cubemaps, silly!
		if (NULL == cube_map) {
			mprintf(("Failed in creating a cube-map in d3d_render_to_env!"));
			return;
		}
	}

	if(!old_render_target){
		GlobalD3DVars::lpD3DDevice->GetRenderTarget(&old_render_target);
		GlobalD3DVars::lpD3DDevice->GetDepthStencilSurface(&old_render_sten);
	}
	
	if(FACE > -1){
		cube_map->GetCubeMapSurface(_D3DCUBEMAP_FACES(FACE), 0, &env_face);
		GlobalD3DVars::lpD3DDevice->SetRenderTarget(env_face , NULL);
		gr_d3d_clear();
	}else{
		GlobalD3DVars::lpD3DDevice->SetRenderTarget(old_render_target, old_render_sten);
	}

//	old_render_target = NULL;

}
*/

#endif // !NO_DIRECT3D
