/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/* 
** $Logfile: /Freespace2/code/Graphics/GrD3DCalls.cpp $ 
*/ 

#include <d3d8.h>
#include "graphics/grd3d.h"
#include "graphics/grd3dinternal.h"


// Uncomment this to disable checking if states are already set, slower but useful for searching for bugs
#define D3D_CALLS_CHECK 1

// Heres the internal vertex information that DX need to know to render them
VertexTypeInfo vertex_types[D3DVT_MAX] =
{
	(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1),	sizeof(D3DVERTEX2D),   
	(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2),	sizeof(D3DTLVERTEX),   
	(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1),		sizeof(D3DLVERTEX),	 
	(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2),							sizeof(D3DVERTEX),	 
	(D3DFVF_XYZ /*| D3DFVF_PSIZE*/),										sizeof(D3DPOINTVERTEX),	 
};

// This is all stuff needed for set render state calls
const float fone  = 1.0f;
const float fzero = 0.0f;
const float fsixtyfour = 64.0f;


DWORD *pfone  = (DWORD *) &fone;
DWORD *pfzero = (DWORD *) &fzero;
DWORD *pfsixtyfour = (DWORD *) &fsixtyfour;

const int NUM_STATES = 174;
unsigned long current_render_states[NUM_STATES];

// These are the default states that D3D8 starts with an returns to on a reset
const unsigned long default_render_states[NUM_STATES] =
{
	0,0,0,0,0,0,0, // not used
	D3DZB_TRUE,			// D3DRS_ZENABLE                   =   7,
	D3DFILL_SOLID,		// D3DRS_FILLMODE                  =   8,
	D3DSHADE_GOURAUD,	// D3DRS_SHADEMODE                 =   9,
	0,					// D3DRS_LINEPATTERN               =  10,
	0,0,0,
	TRUE,				// D3DRS_ZWRITEENABLE              =  14, 
	FALSE,				// D3DRS_ALPHATESTENABLE           =  15, 
	TRUE,				// D3DRS_LASTPIXEL                 =  16, 
	0,0,
	D3DBLEND_ONE,		// D3DRS_SRCBLEND                  =  19, 
	D3DBLEND_ZERO,		// D3DRS_DESTBLEND                 =  20, 
	0,
	D3DCULL_CCW,		// D3DRS_CULLMODE                  =  22, 
	D3DCMP_LESSEQUAL,	// D3DRS_ZFUNC                     =  23, 
	0x00000000,		    // D3DRS_ALPHAREF                  =  24, ?????????????????????
    D3DCMP_ALWAYS,		// D3DRS_ALPHAFUNC                 =  25,
    FALSE,				// D3DRS_DITHERENABLE              =  26,
    FALSE,				// D3DRS_ALPHABLENDENABLE          =  27,
	FALSE,				// D3DRS_FOGENABLE                 =  28,
    FALSE,				// D3DRS_SPECULARENABLE            =  29,
    0,					// D3DRS_ZVISIBLE (not supported)  =  30,
	0,0,0,
    0,					// D3DRS_FOGCOLOR                  =  34,
    D3DFOG_NONE,		// D3DRS_FOGTABLEMODE              =  35,
    *pfzero,			// D3DRS_FOGSTART                  =  36,
    *pfone,				// D3DRS_FOGEND                    =  37,
    *pfone,				// D3DRS_FOGDENSITY                =  38,
	0,
    FALSE,				// D3DRS_EDGEANTIALIAS             =  40,
	0,0,0,0,0,0,
    0,					// D3DRS_ZBIAS                     =  47,
    FALSE,				// D3DRS_RANGEFOGENABLE            =  48,
	0,0,0,
    FALSE,				// D3DRS_STENCILENABLE             =  52,
    D3DSTENCILOP_KEEP,	// D3DRS_STENCILFAIL               =  53,
    D3DSTENCILOP_KEEP,	// D3DRS_STENCILZFAIL              =  54,
    D3DSTENCILOP_KEEP,	// D3DRS_STENCILPASS               =  55,
    D3DCMP_ALWAYS,		// D3DRS_STENCILFUNC               =  56,
    0,					// D3DRS_STENCILREF                =  57,
    0xFFFFFFFF,			// D3DRS_STENCILMASK               =  58,
    0xFFFFFFFF,			// D3DRS_STENCILWRITEMASK          =  59,
    0xFFFFFFFF,			// D3DRS_TEXTUREFACTOR             =  60,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,  	
	0,0,0,0,0,0,0,
    0,					// D3DRS_WRAP0                     = 128,
    0,					// D3DRS_WRAP1                     = 129,
    0,					// D3DRS_WRAP2                     = 130,
    0,					// D3DRS_WRAP3                     = 131,
    0,					// D3DRS_WRAP4                     = 132,
    0,					// D3DRS_WRAP5                     = 133,
    0,					// D3DRS_WRAP6                     = 134,
    0,					// D3DRS_WRAP7                     = 135,
    TRUE,				// D3DRS_CLIPPING                  = 136,
    TRUE,				// D3DRS_LIGHTING                  = 137,
	0,
    0,					// D3DRS_AMBIENT                   = 139,
	0,
    D3DFOG_NONE,		// D3DRS_FOGVERTEXMODE             = 140,
    TRUE,				// D3DRS_COLORVERTEX               = 141,
    TRUE,				// D3DRS_LOCALVIEWER               = 142,
    FALSE,				// D3DRS_NORMALIZENORMALS          = 143,
	0,
    D3DMCS_COLOR1,		// D3DRS_DIFFUSEMATERIALSOURCE     = 145,
    D3DMCS_COLOR2,		// D3DRS_SPECULARMATERIALSOURCE    = 146,
    D3DMCS_COLOR2,		// D3DRS_AMBIENTMATERIALSOURCE     = 147,
    D3DMCS_MATERIAL,	// D3DRS_EMISSIVEMATERIALSOURCE    = 148,
	0,0,
    D3DVBF_DISABLE,		// D3DRS_VERTEXBLEND               = 151,
    0,					// D3DRS_CLIPPLANEENABLE           = 152,
    TRUE,				// D3DRS_SOFTWAREVERTEXPROCESSING  = 153, RT, never us this!
    *pfone,				// D3DRS_POINTSIZE                 = 154,
    *pfzero,			// D3DRS_POINTSIZE_MIN             = 155,
    FALSE,				// D3DRS_POINTSPRITEENABLE         = 156,
    FALSE,				// D3DRS_POINTSCALEENABLE          = 157,
    *pfone,				// D3DRS_POINTSCALE_A              = 158,
    *pfzero,			// D3DRS_POINTSCALE_B              = 159,
    *pfzero,			// D3DRS_POINTSCALE_C              = 160,
    TRUE,				// D3DRS_MULTISAMPLEANTIALIAS      = 161,
    0xFFFFFFFF,			// D3DRS_MULTISAMPLEMASK           = 162,
    D3DPATCHEDGE_DISCRETE, // D3DRS_PATCHEDGESTYLE         = 163,
    *pfone,				// D3DRS_PATCHSEGMENTS             = 164,
    D3DDMT_ENABLE,		// D3DRS_DEBUGMONITORTOKEN         = 165,
    *pfsixtyfour,		// D3DRS_POINTSIZE_MAX             = 166,
    FALSE,				// D3DRS_INDEXEDVERTEXBLENDENABLE  = 167,
    0x0000000F,			// D3DRS_COLORWRITEENABLE          = 168,
    *pfzero,			// D3DRS_TWEENFACTOR               = 170,
    D3DBLENDOP_ADD,		// D3DRS_BLENDOP                   = 171,    
    D3DORDER_CUBIC,		// D3DRS_POSITIONORDER             = 172,
    D3DORDER_LINEAR		// D3DRS_NORMALORDER               = 173,
};

/**
 * Resets all render states to defaults,
 *
 * @return void
 */
void d3d_reset_render_states()
{
	memcpy(current_render_states, default_render_states, sizeof(unsigned long) * NUM_STATES);
}

/**
 * This is a wrapper function for the D3D8.1 call SetRenderState,
 *
 * @param D3DRENDERSTATETYPE render_state_type - Check DX docs
 * @param DWORD render_state - Check DX docs
 *
 * @return HRESULT - directx error code
 */
HRESULT d3d_SetRenderState( D3DRENDERSTATETYPE render_state_type,  DWORD render_state, bool set, bool initializing )
{
	Assert(render_state_type < NUM_STATES);

#ifdef D3D_CALLS_CHECK
	// If the state is already set to that parameter
	if(!initializing && current_render_states[render_state_type] == render_state) {
		// Leave without setting
		return S_OK;
	}
#endif


	HRESULT hr = D3D_OK;
	if(set)
	hr = GlobalD3DVars::lpD3DDevice->SetRenderState(render_state_type, render_state );

	// A set render state should never fail because we should always know the caps.
	if(FAILED(hr)) {
		mprintf(("Failed set render state %d!", render_state_type));
	} else {
		current_render_states[render_state_type] = render_state;
	}

	return hr;
}


// This is all stuff needed for draw primitive call
uint D3D_vertex_type = 0;

HRESULT d3d_CreateVertexBuffer(int vertex_type, int size, DWORD usage, void **buffer)
{
	return GlobalD3DVars::lpD3DDevice->CreateVertexBuffer(
		vertex_types[D3DVT_VERTEX].size * size, 
		usage, 
		vertex_types[D3DVT_VERTEX].fvf,
		D3DPOOL_MANAGED,
		(IDirect3DVertexBuffer8**) buffer);
}

int d3d_get_num_prims(int vertex_count, D3DPRIMITIVETYPE prim_type)
{
	int prim_count;
	switch( prim_type)
	{
		case D3DPT_POINTLIST:
			prim_count = vertex_count; 
			break;
		case D3DPT_LINELIST:       
			prim_count = vertex_count / 2; 
			break;
		case D3DPT_LINESTRIP:      
			prim_count = vertex_count - 1; 
			break;
		case D3DPT_TRIANGLELIST:   
			prim_count = vertex_count / 3; 
			break;
		case D3DPT_TRIANGLESTRIP: 
		case D3DPT_TRIANGLEFAN:    
			prim_count = vertex_count - 2; 
			break;
		default:
			return -1;
	}

	return prim_count;
}

int d3d_get_num_vertex(int prim_count, D3DPRIMITIVETYPE prim_type)
{
	int vertex_count;
	switch( prim_type)
	{
		case D3DPT_POINTLIST:
			vertex_count = prim_count; 
			break;
		case D3DPT_LINELIST:       
			vertex_count = prim_count * 2; 
			break;
		case D3DPT_LINESTRIP:      
			vertex_count = prim_count + 1; 
			break;
		case D3DPT_TRIANGLELIST:   
			vertex_count = prim_count * 3; 
			break;
		case D3DPT_TRIANGLESTRIP: 
		case D3DPT_TRIANGLEFAN:    
			vertex_count = prim_count + 2; 
			break;
		default:
			return -1;
	}

	return vertex_count;
}

/**
 * This is a wrapper function for the D3D8.1 call DrawPrimitveUP, it also handles the vertex shader
 *
 * @param int vertex_type - This should be one of D3DVT_TLVERTEX, D3DVT_LVERTEX or D3DVT_VERTEX,
 * @param D3DPRIMITIVETYPE prim_type - Look at DX docs
 * @param LPVOID pvertices - Pointer to the vertex data, must match the vertex_type chosen 
 * @param DWORD vertex_count - Number of vertices in the array to render
 *
 * @return HRESULT - directx error code
 */
HRESULT d3d_DrawPrimitive(int vertex_type, D3DPRIMITIVETYPE prim_type, LPVOID pvertices, DWORD vertex_count)
{
	HRESULT hr;
	Assert( vertex_type < D3DVT_MAX);
	
	int prim_count = d3d_get_num_prims(vertex_count, prim_type);

	if(prim_count == -1) {
		return !S_OK;
	}

	hr = d3d_SetVertexShader(vertex_types[vertex_type].fvf );
	
	if(FAILED(hr))
	{
		return hr;
	}


	hr = GlobalD3DVars::lpD3DDevice->DrawPrimitiveUP(prim_type, prim_count, pvertices, vertex_types[vertex_type].size);

	if(FAILED(hr)) {
		mprintf(("Failed to draw primitive %d\n",prim_count));
	}
		
	return hr;
}

HRESULT d3d_SetVertexShader(uint fvf)
{
#ifdef D3D_CALLS_CHECK
	if(D3D_vertex_type != fvf) 
#endif
	{
		HRESULT hr = GlobalD3DVars::lpD3DDevice->SetVertexShader(fvf);
		
		if(FAILED(hr)) {
			mprintf(("Failed to set vertex shader"));
			Error( LOCATION, "Failed to set vertex shader: %d\n", fvf );
			return hr;
		}

		D3D_vertex_type = fvf;
	}

	return S_OK;
}

// This is all stuff needed for set texture stage state
const int NUM_STAGE_TYPES = 29;
unsigned long current_stages_zero[NUM_STAGE_TYPES];
const unsigned long default_stages_zero[NUM_STAGE_TYPES] =
{
	0,
	D3DTOP_MODULATE,	//    D3DTSS_COLOROP               =  1,
	D3DTA_TEXTURE,		//    D3DTSS_COLORARG1             =  2,
	D3DTA_CURRENT,		//    D3DTSS_COLORARG2             =  3,
	D3DTOP_SELECTARG1,	//    D3DTSS_ALPHAOP               =  4,
	D3DTA_DIFFUSE,		//    D3DTSS_ALPHAARG1             =  5,
	D3DTA_CURRENT,		//    D3DTSS_ALPHAARG2             =  6,
	*pfzero,			//    D3DTSS_BUMPENVMAT00          =  7,
	*pfzero,			//    D3DTSS_BUMPENVMAT01          =  8,
	*pfzero,			//    D3DTSS_BUMPENVMAT10          =  9,
	*pfzero,			//    D3DTSS_BUMPENVMAT11          = 10,
	0,					//    D3DTSS_TEXCOORDINDEX         = 11,
	0,
	D3DTADDRESS_WRAP,	//    D3DTSS_ADDRESSU              = 13,
	D3DTADDRESS_WRAP,	//    D3DTSS_ADDRESSV              = 14,
	0x00000000,			//    D3DTSS_BORDERCOLOR           = 15,
	D3DTEXF_POINT,		//    D3DTSS_MAGFILTER             = 16,
	D3DTEXF_POINT,		//    D3DTSS_MINFILTER             = 17,
	D3DTEXF_NONE,		//    D3DTSS_MIPFILTER             = 18,
	*pfzero,			//    D3DTSS_MIPMAPLODBIAS         = 19,
	0,					//    D3DTSS_MAXMIPLEVEL           = 20,
	1,					//    D3DTSS_MAXANISOTROPY         = 21,
	*pfzero,			//    D3DTSS_BUMPENVLSCALE         = 22,
	*pfzero,			//    D3DTSS_BUMPENVLOFFSET        = 23,
	D3DTTFF_DISABLE,	//    D3DTSS_TEXTURETRANSFORMFLAGS = 24,
	D3DTADDRESS_WRAP,	//    D3DTSS_ADDRESSW              = 25,
	D3DTA_CURRENT,		//    D3DTSS_COLORARG0             = 26,
	D3DTA_CURRENT,		//    D3DTSS_ALPHAARG0             = 27,
	D3DTA_CURRENT		//    D3DTSS_RESULTARG             = 28,
};

/**
 * Set current texture states to default, stage 0 only 
 * @return void
 */
void d3d_reset_texture_stage_states()
{
	memcpy( current_stages_zero, default_stages_zero, sizeof(unsigned long) * NUM_STAGE_TYPES);
}

/**
 * This is a wrapper function for the D3D8.1 call SetTextureStageState, consult DX documentation
 * for parameter options
 *
 * @param DWORD stage - Texture state, currently only 0 is supported 
 * @param D3DTEXTURESTAGESTATETYPE type	- Type to attemp to change
 * @param DWORD value - New value to attempt to set
 *
 * @return HRESULT - DirectX error code
 */
HRESULT d3d_SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value, bool set, bool initializing)
{
	// Only for first stage because it has different default properties to the others but
	// since they are not even in use, who cares!
#ifdef D3D_CALLS_CHECK
	if(!initializing && stage == 0 && current_stages_zero[type] == value) {
		return S_OK;
	}
#endif


	HRESULT hr = D3D_OK;
	if(set)
	hr = GlobalD3DVars::lpD3DDevice->SetTextureStageState(stage, type, value);

	if(FAILED(hr)) {
		mprintf(("Failed to set TextureStageState %d for stage %d", type, stage));
	} else if (stage == 0) {
		current_stages_zero[type] = value;
	}

	return hr;
}

// Texture set call
const int MAX_TSTAGES = 8;
IDirect3DBaseTexture8 *tinterfaces[MAX_TSTAGES] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

/**
 * This should be called if the device is lost, this will ensure all state default are properly reset 
 *
 * @param int stage - Can only be in range 0 - 7, only 0 is currently supported by vertex coords
 * @param IDirect3DBaseTexture8* texture_ptr - Pointer to texture to be used
 * @return HRESULT
 *
 */
HRESULT d3d_SetTexture(int stage, IDirect3DBaseTexture8* texture_ptr)
{
#ifdef D3D_CALLS_CHECK
 	if(	texture_ptr == tinterfaces[stage]) {
   		return S_OK;
	}
#endif

	HRESULT hr = GlobalD3DVars::lpD3DDevice->SetTexture(stage, texture_ptr);

	if(FAILED(hr)) {
		mprintf(("Failed to set texture"));
	} else {
		tinterfaces[stage] = texture_ptr;
	}

	return hr;
}


void dynamic_buffer::allocate(int n_verts, int vert_type){
	int n_size = n_verts*vertex_types[vert_type].size;
	fvf = vertex_types[vert_type].fvf;
	vsize = vertex_types[vert_type].size;
	if(n_size > size){
		if(buffer)buffer->Release();

		GlobalD3DVars::lpD3DDevice->CreateVertexBuffer(	
			n_size, 
			D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 
			fvf,
			D3DPOOL_DEFAULT,
			&buffer);
		size=n_size;
	}
}

void dynamic_buffer::allocate(int n_verts, uint FVF, int SIZE){
	int n_size = n_verts*SIZE;
	fvf = FVF;
	vsize = SIZE;
	if(n_size > size){
		if(buffer)buffer->Release();

		GlobalD3DVars::lpD3DDevice->CreateVertexBuffer(	
			n_size, 
			D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 
			fvf,
			D3DPOOL_DEFAULT,
			&buffer);
		size=n_size;
	}
}

struct Vertex_buffer;
extern Vertex_buffer* set_buffer;

void dynamic_buffer::unlock(){
	buffer->Unlock();

	set_buffer = NULL;
	d3d_SetVertexShader(fvf);
	GlobalD3DVars::lpD3DDevice->SetStreamSource(0,buffer, vsize);
}

void dynamic_buffer::draw(_D3DPRIMITIVETYPE TYPE, int num){
	GlobalD3DVars::lpD3DDevice->DrawPrimitive(TYPE, 0, d3d_get_num_prims(num, TYPE));
}
/**
 * This should be called if the device is lost, this will ensure all state default are properly reset 
 *
 * @return void
 *
 */
extern IDirect3DIndexBuffer8 *global_index_buffer;

// Returns:
//   TRUE  = the device is lost and cannot be recovered yet
//   FALSE = the device is fine or has been successfully reset
BOOL d3d_lost_device(bool force)
{


	// if we can't reset the device yet then back out but only after Sleep()ing a little
	if ( GlobalD3DVars::lpD3DDevice->TestCooperativeLevel() == D3DERR_DEVICELOST ) {
		Sleep(5);
		return TRUE;
	}

	// Calling Reset causes all texture memory surfaces to be lost, managed textures to be flushed 
	// from video memory, and all state information to be lost. Before calling the Reset method for a 
	// device, an application should release any explicit render targets, depth stencil surfaces, 
	// additional swap chains and D3DPOOL_DEFAULT resources associated with the device.
	if (force || GlobalD3DVars::lpD3DDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET ) {
		// Set states to what they will be after reset

		extern int In_frame;
		if(In_frame){
			GlobalD3DVars::lpD3DDevice->EndScene();
			In_frame--;
		}

		if (global_index_buffer != NULL) {
			int m = 1;

			while ( m > 0 ) {
				m = global_index_buffer->Release();
			}

			global_index_buffer = NULL;
		}

		extern IDirect3DIndexBuffer8 *global_index_buffer;
//		extern IDirect3DTexture8 *background_render_target;
//		extern LPDIRECT3DCUBETEXTURE8 cube_map;
		extern IDirect3DSurface8 *old_render_sten;
		extern IDirect3DSurface8 *old_render_target;

//		if(background_render_target){background_render_target->Release();background_render_target=NULL;}
//		int r = 0;
//		if(cube_map){
/*			LPDIRECT3DSURFACE8 face;
			for(int i = 0; i<6; i++){
				cube_map->GetCubeMapSurface(_D3DCUBEMAP_FACES(i), 0, &face);
				r = face->Release();
			}
*/
//			r = cube_map->Release();
//			cube_map=NULL;
//		}
		if(old_render_sten){old_render_sten->Release();old_render_sten=NULL;}
		if(old_render_target){old_render_target->Release();old_render_target=NULL;}
		if(global_index_buffer){global_index_buffer->Release();global_index_buffer=NULL;}

		extern bool cube_map_drawen;
		cube_map_drawen = false;
		

void bm_pre_lost();
		bm_pre_lost();

		gr_d3d_set_state(TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_DEFAULT);

		void d3d_kill_state_blocks();
		d3d_kill_state_blocks();

		render_buffer.lost();

		HRESULT hr = GlobalD3DVars::lpD3DDevice->Reset(&GlobalD3DVars::d3dpp);
		if(hr != D3D_OK){
			return true;
		}

void bm_post_lost();
		bm_post_lost();
//		In_frame = 0;

		if ( GlobalD3DVars::lpD3DDevice->TestCooperativeLevel() == D3DERR_DEVICELOST ) {
			Sleep(5);
			return TRUE;
		}



//		extern void d3d_init_environment();
//		d3d_init_environment();

		void d3d_generate_state_blocks();
		d3d_generate_state_blocks();

		d3d_reset_render_states();
		d3d_reset_texture_stage_states();	 

		memset(tinterfaces, 0, sizeof(IDirect3DBaseTexture8 *) * MAX_TSTAGES);

		d3d_set_initial_render_state();

		D3D_vertex_type = 0;

		GlobalD3DVars::lpD3DDevice->BeginScene();
		if(!In_frame)In_frame++;
	}

	return FALSE;
}
