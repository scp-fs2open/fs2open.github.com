/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

/* 
 * $Log: not supported by cvs2svn $
 * Revision 2.6  2003/11/19 20:37:24  randomtiger
 * Almost fully working 32 bit pcx, use -pcx32 flag to activate.
 * Made some commandline variables fit the naming standard.
 * Changed timerbar system not to run pushes and pops if its not in use.
 * Put in a note about not uncommenting asserts.
 * Fixed up a lot of missing POP's on early returns?
 * Perhaps the motivation for Assert functionality getting commented out?
 * Fixed up some bad asserts.
 * Changed nebula poofs to render in 2D in htl, it makes it look how it used to in non htl. (neb.cpp,1248)
 * Before the poofs were creating a nasty stripe effect where they intersected with ships hulls.
 * Put in a special check for the signs of that D3D init bug I need to lock down.
 *
 */

#include "globalincs/pstypes.h"
#include "grd3dbatch.h"

#include <d3d8.h>
#include <d3d8types.h>

#include "2d.h"
#include "graphics/grd3dinternal.h"
#include "debugconsole/timerbar.h"

const int FONT_VTYPE = D3DVT_TLVERTEX;
typedef	D3DTLVERTEX FONT_VERTEX;

// BATCH STUFF
const int MAX_BATCH_BUFFERS = 5;

typedef struct
{
	IDirect3DVertexBuffer8 *vbuffer;
	int max_size;
	int current_size;
	int render_from;
	int vertex_type;
	int prim_type;

	int lock;

	BatchInfo current_info; 

} Batch;

extern VertexTypeInfo vertex_types[D3DVT_MAX];

Batch *batch_array;
int    batch_array_max;

// FONT BATCH STUFF

const int MIN_STRING_LEN = 1;

const int MAX_SM_STRINGS = 64;
const int MAX_SM_STR_LEN = 20;

const int MAX_LG_STRINGS =  10;
const int MAX_LG_STR_LEN = 256;

const int MAX_STRING_LEN = 512;

typedef struct
{
	int magic_number;
	bool used_this_frame;
	bool free_slot;
	int x,y, len;
	int char_count;
	uint color;
	int offsetx;
	int offsety;

	IDirect3DVertexBuffer8 *vbuffer;
} StringBatch; 

StringBatch	*long_str_array;
StringBatch	*small_str_array;

FONT_VERTEX d3d_verts[MAX_STRING_LEN*6];

bool d3d_batch_string_init();
void d3d_batch_string_deinit();
void d3d_batch_string_end_frame();

bool batch_info_are_equal(const BatchInfo &lhs, const BatchInfo &rhs)
{
	return 
		(lhs.state_set_func		== rhs.state_set_func)   &&
		(lhs.texture_id			== rhs.texture_id)		 &&
		(lhs.bitmap_type		== rhs.bitmap_type)		 &&
		(lhs.filter_type		== rhs.filter_type)		 &&
		(lhs.alpha_blend_type	== rhs.alpha_blend_type) &&
		(lhs.zbuffer_type		== rhs.zbuffer_type);
}


bool d3d_batch_init()
{
	Assert(batch_array == NULL);

	batch_array = (Batch *) malloc(MAX_BATCH_BUFFERS * sizeof(Batch));

	if(batch_array == NULL) return false;

	batch_array_max = MAX_BATCH_BUFFERS;
	ZeroMemory(batch_array, batch_array_max * sizeof(Batch));

	return d3d_batch_string_init();
}

void d3d_batch_deinit()
{
	if(batch_array == NULL) return;

	for(int i = 0; i < batch_array_max; i++) {
		if(batch_array[i].vbuffer != NULL) {
			d3d_destory_batch(i);
		}
	}

	free(batch_array);

	d3d_batch_string_deinit();
}

int d3d_create_batch(int num_verts, int vert_type, int ptype)
{
	if(batch_array_max == 0) return -1;

	int i = 0;
	// Find a free space
	while(batch_array[i].vbuffer != NULL) {
		i++;

		// Failed to find
		if(i == batch_array_max) return -1;
	}

	HRESULT hr = GlobalD3DVars::lpD3DDevice->CreateVertexBuffer(
		num_verts * vertex_types[vert_type].size, 
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, // Usage
		vertex_types[vert_type].fvf,
		D3DPOOL_DEFAULT, 
		&batch_array[i].vbuffer);

	if(FAILED(hr)) return -1;
	if(batch_array[i].vbuffer == NULL) return -1;

	batch_array[i].vertex_type = vert_type;
	batch_array[i].prim_type = ptype;

	ZeroMemory(&batch_array[i].current_info, sizeof(BatchInfo)); 

	batch_array[i].max_size = num_verts;

	return i;

}

void d3d_destory_batch(int batch_id)
{
	if(batch_array_max == 0) return;

	Assert(batch_id < batch_array_max);
	Assert(batch_array[batch_id].vbuffer != NULL);

	if(batch_array[batch_id].vbuffer) {
		batch_array[batch_id].vbuffer->Release();
	}

	ZeroMemory(&batch_array[batch_id], sizeof(Batch));
}

void *d3d_batch_lock_vbuffer(int batch_id, int num_to_lock, BatchInfo &batch_info)
{
	if(batch_array_max == 0) return NULL;

	Assert(batch_id < batch_array_max);
	Assert(batch_array[batch_id].vbuffer != NULL);

	HRESULT hr;
	BYTE *p_data = NULL;

	// Default flag
	int flags = D3DLOCK_NOOVERWRITE;
	
	// If we run out of buffer space or render state or texture changes we must render and start
	// a new batch
	bool enough_space = (batch_array[batch_id].current_size + num_to_lock) < batch_array[batch_id].max_size;
	bool info_match   = true; // assume true

	// If the info is actually then correct this
	if(batch_array[batch_id].current_info.is_set)
	{
		info_match = batch_info_are_equal(batch_info, batch_array[batch_id].current_info);
	}
	else
	{
		memcpy(&batch_array[batch_id].current_info, &batch_info, sizeof(BatchInfo));
	}

	if(!enough_space || !info_match)
	{
		// Render now!
		d3d_batch_draw_vbuffer(batch_id);
		memcpy(&batch_array[batch_id].current_info, &batch_info, sizeof(BatchInfo));
		batch_array[batch_id].current_info.is_set = true;

	 	batch_array[batch_id].render_from   = batch_array[batch_id].current_size;
		flags = D3DLOCK_DISCARD;
	}

	int size = vertex_types[batch_array[batch_id].vertex_type].size;
	
	hr = batch_array[batch_id].vbuffer->Lock(
		batch_array[batch_id].current_size * size,	// OffsetToLock,
		num_to_lock * size,							// SizeToLock,
		&p_data,
		flags); // Flags

	batch_array[batch_id].lock++;

	batch_array[batch_id].current_size += num_to_lock;

	Assert(SUCCEEDED(hr));

	return p_data; 
}

void d3d_batch_unlock_vbuffer(int batch_id)
{
	if(batch_array_max == 0) return;

	Assert(batch_id < batch_array_max);
	Assert(batch_array[batch_id].vbuffer != NULL);

	// Goober5000 - commented to bypass warning
	/*HRESULT hr =*/ batch_array[batch_id].vbuffer->Unlock();


	batch_array[batch_id].lock--;

	// Kazan - Commented out to bypass compiler saying symbol undefined :D
	//Assert(hr);
}

void d3d_set_render_states(BatchInfo &batch_info)
{
	gr_set_bitmap(batch_info.texture_id);

	// Get this now rather than inside the loop
	gr_d3d_set_state( 
		(gr_texture_source) batch_info.filter_type, 
		(gr_alpha_blend)    batch_info.alpha_blend_type, 
		(gr_zbuffer_type)   batch_info.zbuffer_type );

	float u, v;
	gr_tcache_set(batch_info.texture_id, 
		batch_info.bitmap_type, &u, &v );

	if(batch_info.state_set_func != NULL) 
		batch_info.state_set_func();
}

bool d3d_batch_draw_vbuffer(int batch_id)
{
	if(batch_array_max == 0) return false;

	Assert(batch_id < batch_array_max);
	Assert(batch_array[batch_id].vbuffer != NULL);

	int vtype		 = batch_array[batch_id].vertex_type;
	int ptype 		 = batch_array[batch_id].prim_type;
	int vertex_count = batch_array[batch_id].current_size - batch_array[batch_id].render_from;

	if(vertex_count == 0)
		return true;

	if(batch_array[batch_id].lock > 0) {
		d3d_batch_unlock_vbuffer(batch_id);
	}

	Assert(batch_array[batch_id].lock != 0);

	int prim_count = d3d_get_num_prims(vertex_count, (D3DPRIMITIVETYPE) ptype);

	d3d_set_render_states(batch_array[batch_id].current_info);

	HRESULT hr;
	hr = d3d_SetVertexShader(vtype);
	Assert(SUCCEEDED(hr));

	hr = GlobalD3DVars::lpD3DDevice->SetStreamSource(
		0, batch_array[batch_id].vbuffer, vertex_types[vtype].size);
	Assert(SUCCEEDED(hr));


  	hr = GlobalD3DVars::lpD3DDevice->DrawPrimitive(
  		(D3DPRIMITIVETYPE) ptype, batch_array[batch_id].render_from, prim_count);
  	Assert(SUCCEEDED(hr));

	batch_array[batch_id].current_size = 0;
	batch_array[batch_id].render_from  = 0;

	return SUCCEEDED(hr);
}

void d3d_batch_end_frame()
{
	d3d_batch_string_end_frame();

	if(batch_array_max == 0) return;

	for(int i = 0; i < batch_array_max; i++) {
		if(batch_array[i].vbuffer) {
			d3d_batch_draw_vbuffer(i);
		}
	}
}

/**
 * @param int x
 * @param int y
 * @param int w
 * @param int h
 * @param int sx
 * @param int sy
 *
 * @return void
 */
void d3d_stuff_char(D3DTLVERTEX *src_v, int x,int y,int w,int h,int sx,int sy, int bw, int bh, float u_scale, float v_scale, uint color)
{
	float u0, u1, v0, v1;
	float x1, x2, y1, y2;

	float fbw = i2fl(bw);
	float fbh = i2fl(bh);

	// Rendition 
	if(GlobalD3DVars::D3D_Antialiasing) {
		u0 = u_scale*(i2fl(sx)-0.5f) / fbw;
		v0 = v_scale*(i2fl(sy)+0.05f) / fbh;
		u1 = u_scale*(i2fl(sx+w)-0.5f) / fbw;
		v1 = v_scale*(i2fl(sy+h)-0.5f) / fbh;
	} else if (GlobalD3DVars::D3D_rendition_uvs )	{
		u0 = u_scale*(i2fl(sx)+0.5f) / fbw;
		v0 = v_scale*(i2fl(sy)+0.5f) / fbh;

		u1 = u_scale*(i2fl(sx+w)+0.5f) / fbw;
		v1 = v_scale*(i2fl(sy+h)+0.5f) / fbh;
	} else {
		u0 = u_scale*i2fl(sx)/ fbw;
		v0 = v_scale*i2fl(sy)/ fbh;
		u1 = u_scale*i2fl(sx+w)/ fbw;
		v1 = v_scale*i2fl(sy+h)/ fbh;
	} 

	if(GlobalD3DVars::D3D_custom_size == -1)
	{
		x1 = i2fl(x+gr_screen.offset_x);
		y1 = i2fl(y+gr_screen.offset_y);
		x2 = i2fl(x+w+gr_screen.offset_x);
		y2 = i2fl(y+h+gr_screen.offset_y);

	} else {
		extern bool gr_d3d_resize_screen_pos(int *x, int *y);

		int nx = x+gr_screen.offset_x;
		int ny = y+gr_screen.offset_y;
		int nw = x+w+gr_screen.offset_x;
		int nh = y+h+gr_screen.offset_y;

		gr_d3d_resize_screen_pos(&nx, &ny);
		gr_d3d_resize_screen_pos(&nw, &nh);

		x1 = i2fl(nx);
		y1 = i2fl(ny);
		x2 = i2fl(nw);
		y2 = i2fl(nh);
	}

#if 1
	if(GlobalD3DVars::D3D_custom_size != -1)
	{
		u1 -= GlobalD3DVars::texture_adjust_u;
		v1 -= GlobalD3DVars::texture_adjust_v;
	}
#endif

	// 0
	src_v->sz = 0.99f;
	src_v->rhw = 1.0f;
	src_v->color = color;	 
	src_v->specular = 0;
	src_v->sx = x1;
	src_v->sy = y1;
	src_v->tu = u0;
	src_v->tv = v0;
	src_v++;

	// 1
	src_v->sz = 0.99f;
	src_v->rhw = 1.0f;
	src_v->color = color;	 
	src_v->specular = 0;
	src_v->sx = x2;
	src_v->sy = y1;
	src_v->tu = u1;
	src_v->tv = v0;
	src_v++;

	// 2
	src_v->sz = 0.99f;
	src_v->rhw = 1.0f;
	src_v->color = color;	 
	src_v->specular = 0;
	src_v->sx = x2;
	src_v->sy = y2;
	src_v->tu = u1;
	src_v->tv = v1;
	src_v++;

	// 0
	src_v->sz = 0.99f;
	src_v->rhw = 1.0f;
	src_v->color = color;	 
	src_v->specular = 0;
	src_v->sx = x1;
	src_v->sy = y1;
	src_v->tu = u0;
	src_v->tv = v0;
	src_v++;

	// 2
	src_v->sz = 0.99f;
	src_v->rhw = 1.0f;
	src_v->color = color;	 
	src_v->specular = 0;
	src_v->sx = x2;
	src_v->sy = y2;
	src_v->tu = u1;
	src_v->tv = v1;
	src_v++;

	// 3
	src_v->sz = 0.99f;
	src_v->rhw = 1.0f;
	src_v->color = color;	 
	src_v->specular = 0;
	src_v->sx = x1;
	src_v->sy = y2;
	src_v->tu = u0;
	src_v->tv = v1;
}	 

bool d3d_batch_string_init()
{
	int i;
	HRESULT hr;
	int size;

	size = MAX_LG_STRINGS * sizeof(StringBatch);
	long_str_array = (StringBatch *) malloc(size);
	if(long_str_array == NULL) return false;

	ZeroMemory(long_str_array, size); 
	for(i = 0; i < MAX_LG_STRINGS; i++) {

		hr = d3d_CreateVertexBuffer(
			FONT_VTYPE,
			MAX_LG_STR_LEN * 6, 
			D3DUSAGE_WRITEONLY, 
			(void **) &long_str_array[i].vbuffer);

		if(FAILED(hr)) {
			d3d_batch_string_deinit();
			return false;
		}
	}

	size = MAX_SM_STRINGS * sizeof(StringBatch);
	small_str_array = (StringBatch *) malloc(size);
	if(small_str_array == NULL) return false;

	ZeroMemory(small_str_array, size); 
	for(i = 0; i < MAX_SM_STRINGS; i++) {

		hr = d3d_CreateVertexBuffer(
			FONT_VTYPE,
			MAX_SM_STR_LEN * 6, 
			D3DUSAGE_WRITEONLY, 
			(void **) &small_str_array[i].vbuffer);

		if(FAILED(hr)) {
			d3d_batch_string_deinit();
			return false;
		}

	}

	return true;
}

void d3d_batch_string_deinit()
{
	int i;
	if(long_str_array) {	

		for(i = 0; i < MAX_LG_STRINGS; i++) {
			if(long_str_array[i].vbuffer)
				long_str_array[i].vbuffer->Release();
				long_str_array[i].vbuffer = NULL;
		}

		free(long_str_array);
	}

	if(small_str_array) {	

		for(i = 0; i < MAX_SM_STRINGS; i++) {
			if(small_str_array[i].vbuffer)
				small_str_array[i].vbuffer->Release();
				small_str_array[i].vbuffer = NULL;
		}

		free(small_str_array);
	}
}

void d3d_batch_string_end_frame()
{
	int i;
	for(i = 0; i < MAX_LG_STRINGS; i++) {
		long_str_array[i].free_slot = !(long_str_array[i].used_this_frame);
		long_str_array[i].used_this_frame = false;
	}

	for(i = 0; i < MAX_SM_STRINGS; i++) {
		small_str_array[i].free_slot = !(small_str_array[i].used_this_frame);
		small_str_array[i].used_this_frame = false;
	}
}

int find_string(int len, int magic_num, int sx, int sy, uint color, bool long_str)
{
	StringBatch *array  = (long_str) ? long_str_array : small_str_array;
	int loop_len		= (long_str) ? MAX_LG_STRINGS : MAX_SM_STRINGS;

	for(int i = 0; i < loop_len; i++) {
		if(array[i].magic_number == magic_num &&
			array[i].len == len &&
			array[i].offsetx == gr_screen.offset_x && 
			array[i].offsety == gr_screen.offset_y && 
			array[i].x == sx &&
			array[i].y == sy &&
			array[i].color == color) {

			return i;
		}
	}

	return -1;
}

int find_free_slot(bool long_str)
{
	StringBatch *array  = (long_str) ? long_str_array : small_str_array;
	int loop_len		= (long_str) ? MAX_LG_STRINGS : MAX_SM_STRINGS;

	for(int i = 0; i < loop_len; i++) {

		if(array[i].used_this_frame == false &&
		   array[i].free_slot == true)
		{
			return i;
		}
	}

	return -1;
}

/**
 * Generate magic number to identify string
 *
 * @param char *s - String to find length of
 * @param int &magic_number
 * @return - Length of string
 */
inline int strlen_magic(char *s, int &magic_number)
{
	int count = 0;
	magic_number = 0;

	while(*s != '\0')
	{
		magic_number += *s;
		count++;

		s++;
	}

	return count;
}

void d3d_batch_string(int sx, int sy, char *s, int bw, int bh, float u_scale, float v_scale, uint color)
{
	int spacing = 0;
	int width;

	int x = sx;
	int y = sy;

	// centered
	x =(sx==0x8000) ? get_centered_x(s) : sx;

  	do
	{
	HRESULT hr;
	int magic_num;
	int len = strlen_magic(s, magic_num);
	int new_id = -1;

	if(len == 0) return;

	// Set to 0 to turn off batching to gfx card memory
#if 1
	// Check the string is of a valid size
	if(len > MIN_STRING_LEN && len < MAX_LG_STR_LEN)
	{
		bool long_str = (len >= MAX_SM_STR_LEN);

		int index = find_string(len, magic_num, sx, sy, color, long_str);
		
		// We have found the string, render and mark as rendered
		if(index != -1)
		{
			StringBatch *array = (long_str) ? long_str_array : small_str_array; 

			extern VertexTypeInfo vertex_types[D3DVT_MAX];

			d3d_SetVertexShader(FONT_VTYPE);
			hr = GlobalD3DVars::lpD3DDevice->SetStreamSource(
				0, array[index].vbuffer, vertex_types[FONT_VTYPE].size);
			
			Assert(SUCCEEDED(hr));
			hr = GlobalD3DVars::lpD3DDevice->DrawPrimitive(
				D3DPT_TRIANGLELIST, 0, array[index].char_count * 2);

			Assert(SUCCEEDED(hr));

			array[index].used_this_frame = true;
			return;

		// Prepare to enter the string
		} else {
			new_id = find_free_slot(long_str);
		}
	}
#endif

	IDirect3DVertexBuffer8 *vbuffer = NULL;	
	FONT_VERTEX *locked_buffer = NULL;
	StringBatch *array = NULL;

	if(new_id != -1) {

		array = (len > MAX_SM_STR_LEN) ? long_str_array : small_str_array; 
		vbuffer = array[new_id].vbuffer;
		hr = vbuffer->Lock(0, len * 6 * vertex_types[FONT_VTYPE].size, (BYTE **) &locked_buffer, 0);
		
		array[new_id].used_this_frame = true;
		// We can always bail out at this point
		if(FAILED(hr)) {
			Assert(0);
			new_id = -1;
		} 
	}

	int char_count = 0;

	while (*s)	{
		x += spacing;

		while (*s== '\n' )	{
			s++;
			y += Current_font->h;
			// centered
			x =(sx==0x8000) ? get_centered_x(s) : sx;
		}
		if (*s == 0 ) break;

		int letter = get_char_width(s[0],s[1],&width,&spacing);
		s++;

		//not in font, draw as space
		if (letter<0)	{
			continue;
		}

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

		font_char *ch;
	
		ch = &Current_font->char_data[letter];

		int u = Current_font->bm_u[letter];
		int v = Current_font->bm_v[letter];

		if ( wc < 1 ) continue;
		if ( hc < 1 ) continue;

		FONT_VERTEX *src_v = NULL;

		if(new_id != -1) {
			src_v = &locked_buffer[char_count * 6];
		} else {
			src_v = &d3d_verts[char_count * 6];
		}

	// Change the color this way to see which strings are being cached
	  //	uint color2 = (new_id == -1) ? color : 0xff00ff00;

		// Marks the end of a batch blue
	  //	if(char_count > (MAX_STRING_LEN - 10)) 
	  //		color2 = 0xff0000ff;

	 	d3d_stuff_char(src_v, xc, yc, wc, hc, u+xd, v+yd, bw, bh, u_scale, v_scale, color);

		char_count++;
		if(char_count >= MAX_STRING_LEN) {
			// We've run out of space, we are going to have to go round again
			break;
		}
	}

	if(new_id != -1) {
		vbuffer->Unlock();

		if(char_count == 0) {
			array[new_id].free_slot		  = true;
			array[new_id].used_this_frame = false;
			continue;
		}

		d3d_SetVertexShader(FONT_VTYPE);
		hr = GlobalD3DVars::lpD3DDevice->SetStreamSource(0, vbuffer, vertex_types[FONT_VTYPE].size); 

		Assert(SUCCEEDED(hr));
		hr = GlobalD3DVars::lpD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, char_count * 2);
		Assert(SUCCEEDED(hr));

		// Fill in details
		array[new_id].char_count   = char_count;
		array[new_id].color        = color;
		array[new_id].free_slot    = false;
		array[new_id].len		   = len;
		array[new_id].magic_number = magic_num;
		array[new_id].x			   = sx;
		array[new_id].y			   = sy;
		array[new_id].offsetx      = gr_screen.offset_x; 
		array[new_id].offsety      = gr_screen.offset_y; 

		array[new_id].used_this_frame = true;

	} else if(char_count > 0) {
  	   	d3d_DrawPrimitive(FONT_VTYPE, D3DPT_TRIANGLELIST,(LPVOID)d3d_verts, char_count * 6);
	}

	} while (*s);
}



