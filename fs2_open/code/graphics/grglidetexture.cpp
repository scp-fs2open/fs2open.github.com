/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrGlideTexture.cpp $
 * $Revision: 2.1 $
 * $Date: 2002-08-01 01:41:05 $
 * $Author: penguin $
 *
 * Code to manage Glide texture RAM
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 14    10/13/99 9:22a Daveb
 * Fixed Fred jumpnode placing bug. Fixed 1024 glide tiled texture problem
 * related to movies. Fixed launcher spawning from PXO screen.
 * 
 * 13    7/16/99 1:49p Dave
 * 8 bit aabitmaps. yay.
 * 
 * 12    7/13/99 1:15p Dave
 * 32 bit support. Whee!
 * 
 * 11    7/09/99 9:51a Dave
 * Added thick polyline code.
 * 
 * 10    7/08/99 8:10a Mikek
 * Suppress compiler warning.  Now I get an FS2 programming credit! :)
 * 
 * 9     6/29/99 4:16p Dave
 * Temporary speedup for tcache init.
 * 
 * 8     6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 7     6/16/99 4:06p Dave
 * New pilot info popup. Added new draw-bitmap-as-poly function.
 * 
 * 6     2/08/99 5:07p Dave
 * FS2 chat server support. FS2 specific validated missions.
 * 
 * 5     1/15/99 11:29a Neilk
 * Fixed D3D screen/texture pixel formatting problem. 
 * 
 * 4     12/01/98 8:06a Dave
 * Temporary checkin to fix some texture transparency problems in d3d.
 * 
 * 3     11/30/98 1:07p Dave
 * 16 bit conversion, first run.
 * 
 * 2     10/07/98 10:53a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 18    5/14/98 5:39p John
 * Added in code for multiple non-dimming lights.
 * 
 * 17    4/27/98 9:33p John
 * Removed mprintf
 * 
 * 16    4/27/98 9:25p John
 * Made Glide skip any 2MB boundry, not just first one.
 * 
 * 15    4/27/98 9:09p John
 * Fixed bug where texture was crossing a 2MB line on Voodoo2.  Added code
 * to tell how full VRAM is and how much got paged in each frame.
 * 
 * 14    4/27/98 10:44a John
 * Put in a new texture caching algorithm that doesn't flush everything
 * every so oftem.
 * 
 * 13    4/26/98 12:02p John
 * Made glide texturing do its own allocation rather than using the gu_
 * functions.
 * 
 * 12    4/26/98 11:14a John
 * Restructured/Cleaned up the caching code in preparation for the new
 * improved caching system.
 * 
 * 11    4/22/98 9:13p John
 * Added code to replace frames of animations in vram if so desired.
 * 
 * 10    4/21/98 9:16a John
 * Fixed bug with directives display in Glide.
 * 
 * 9     4/20/98 8:41p John
 * Made debris culling actually reduce Glide texture resolution.
 * 
 * 8     4/09/98 4:38p John
 * Made non-darkening and transparent textures work under Glide.  Fixed
 * bug with Jim's computer not drawing any bitmaps.
 * 
 * 7     4/09/98 2:21p John
 * Fixed transparency bug with Glide
 * 
 * 6     4/08/98 9:16p John
 * Made transparency work for textures in Glide by using chromakey.  Made
 * nondarkening colors work.
 * 
 * 5     4/08/98 8:47a John
 * Moved all texture caching into a new module
 * 
 * 4     4/06/98 4:45p John
 * Removed some debug code that made textures 2x too small.
 * 
 * 3     3/04/98 5:43p Hoffoss
 * Fixed warning.
 * 
 * 2     3/04/98 3:50p John
 * Made the Glide texture cache manager of Leighton's work with 256x256
 * maps.
 * 
 * 1     3/03/98 4:42p John
 * Added in Leighton's code to do texture caching on Glide.
 *
 * $NoKeywords: $
 */

//#define USE_8BPP_TEXTURES 

#include <windows.h>
#include <windowsx.h>
#include "glide/glide.h"
#include "glide/glideutl.h"

#include "osapi/osapi.h"
#include "graphics/2d.h"
#include "bmpman/bmpman.h"
#include "math/floating.h"
#include "palman/palman.h"
#include "graphics/grinternal.h"
#include "graphics/grglide.h"
#include "graphics/line.h"
#include "graphics/font.h"
#include "io/mouse.h"
#include "io/key.h"
#include "globalincs/systemvars.h"
#include "graphics/grglideinternal.h"

#define TEXMEM_2MB_EDGE (2*1024*1024)

// Hardware specific values
typedef struct tcache_data {
	GrLOD_t           lod;
	GrAspectRatio_t   aspect;
	GrTextureFormat_t format;
	uint					vram_offset;
	int					vram_size;	
} tcache_data;

typedef struct tcache_slot {
	int				bitmap_id;	
	float				uscale;
	float				vscale;
	
	tcache_data		data;
	tcache_slot		*data_sections[MAX_BMAP_SECTIONS_X][MAX_BMAP_SECTIONS_Y];		// NULL if no subsections are present. 
	tcache_slot		*parent;
} tcache_slot;

#define MAX_AUX_TEXTURES						200
#define MAX_TEXTURES								MAX_BITMAPS

// search for AUX to find all instancees of aux textures which can be put back in.

// AUX
// static tcache_slot *Textures_aux = NULL;														// auxiliary textures for storing sections of a texture
static void *Texture_sections = NULL;
static tcache_slot *Textures = NULL;
ubyte *Glide_tmp_ram = NULL;

int Glide_last_bitmap_id = -1;
int Glide_last_bitmap_type = -1;
int Glide_last_section_x = -1;
int Glide_last_section_y = -1;
float Glide_last_u_ratio, Glide_last_v_ratio;

static int Glide_last_detail = -1;

uint Glide_total_memory;
uint Glide_start_address;
uint Glide_end_address;
uint Glide_current_address;

int Glide_textures_in_frame = 0;

int Glide_explosion_vram = 0;

//=======================================================
// Code to manage texture memory.   
// This basically allocates until out of VRAM, and then starts freeing 
// blocks at the start of VRAM to make room for new textures.   
// Based on code from Jason Scannell's webpage.
// http://members.home.net/jscannell.  

typedef struct	{ 
	int		next; 
	uint		start_address; 
	int		size;
	tcache_slot		*texture_ptr;
} tblock; 

tblock	*Tblocks = NULL;		// Memory tracking blocks 
int		Tblock_num_blocks = 0;		// How many blocks to use

int Tblock_freelist_start;				// First block in free list 
int Tblock_usedlist_start;				// First block in allocation list 
int Tblock_usedlist_head;				// Last block in allocation list 

uint Tblock_min_address;				// Lowest address in texture memory 
uint Tblock_max_address;				// Highest address in texture memory 

void FlushBlocks()
{
	int i;

	memset(Tblocks,0,sizeof(tblock)*Tblock_num_blocks); 

	for(i = 0; i < Tblock_num_blocks - 1; i++)	{
		Tblocks[i].next = i + 1; 
	}

	Tblocks[Tblock_num_blocks - 1].next = -1; 

	Tblock_usedlist_start = -1; 
	Tblock_usedlist_head  = -1; 
	Tblock_freelist_start  = 0; 
}

void InitBlocks(uint min, uint max, int num_blocks) 
{ 
	Tblock_num_blocks = num_blocks;
	Tblock_min_address = min;
	Tblock_max_address = max;

	if(Tblocks == NULL){
		Tblocks = (tblock *)malloc(Tblock_num_blocks*sizeof(tblock));
		Assert(Tblocks!=NULL);
	}

	FlushBlocks();
} 

void ReleaseBlocks()
{
	if ( Tblocks )	{
		free(Tblocks);
		Tblocks = NULL;
	}
	Tblock_num_blocks = 0;
}

void ReleaseSlotSub(tcache_slot *t)
{
	int idx, s_idx;

	if(t == NULL){
		return;
	}

	t->bitmap_id = -1;
	for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
		for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
			// recurse
			if(t->data_sections[idx][s_idx] != NULL){				
				ReleaseSlotSub(t->data_sections[idx][s_idx]);
				t->data_sections[idx][s_idx] = NULL;
			}
		}
	}
}

void ReleaseSlot(int nBlock)
{
	if ( Tblocks[nBlock].texture_ptr != NULL )	{				
		// if this guy has a parent (in the case of a sectioned bitmap), unset his bitmap
		if(Tblocks[nBlock].texture_ptr->parent != NULL){
			Tblocks[nBlock].texture_ptr->parent->bitmap_id = -1;
		}
		Tblocks[nBlock].texture_ptr->bitmap_id = -1;

		// AUX
		// ReleaseSlotSub(Tblocks[nBlock].texture_ptr);
		Tblocks[nBlock].texture_ptr = NULL;
		

		Glide_textures_in -= Tblocks[nBlock].size;
	}
}

void FreeBlock(int nBlock) 
{ 
	int nFreeNext; 

	ReleaseSlot(nBlock);

	//---- Save next indices ---- 
	nFreeNext = Tblock_freelist_start; 

	//---- Move alloc list block to start of free list ---- 
	Tblock_freelist_start = nBlock; 
	Tblocks[nBlock].next = nFreeNext; 

} 

int AllocBlock(void) 
{ 
	int     nNewBlock; 

	//---- Get block from free list ---- 
	nNewBlock = Tblock_freelist_start; 

	//**** DOS NOT HANDLE EMPTY FREE LIST **** 
	Tblock_freelist_start = Tblocks[Tblock_freelist_start].next; 

	if(Tblock_usedlist_head < 0)	{ 
		//---- Alloc list is empty, add to start ---- 
		Tblock_usedlist_start = nNewBlock; 
		Tblocks[nNewBlock].next = -1; 
	} else { 
		//---- Insert at head of alloc list ---- 
		Tblocks[nNewBlock].next = Tblocks[Tblock_usedlist_head].next; 
		Tblocks[Tblock_usedlist_head].next = nNewBlock; 
	} 

	//---- Set new head index ---- 
	Tblock_usedlist_head = nNewBlock; 

	return nNewBlock; 
} 

// Macro to compute ending address of block 
#define BLOCK_END(b) (Tblocks[b].start_address + Tblocks[b].size) 

uint AllocateTexture(uint size, tcache_slot *texture_ptr)
{ 
	int     nNewBlock; 
	int     next; 

	if (Tblock_usedlist_start < 0)	{ 

		//---- Alloc list is empty ---- 
		nNewBlock = AllocBlock(); 
		Tblocks[nNewBlock].start_address = Tblock_min_address; 
	} else { 

		uint dwAddress = BLOCK_END(Tblock_usedlist_head);
		if ( dwAddress + size < Tblock_max_address )	{
			int a1 = dwAddress / TEXMEM_2MB_EDGE;
			int a2 = (dwAddress+size) / TEXMEM_2MB_EDGE;
	
			if ( a2 > a1 )	{
				//mprintf(( "GrGlideTexture: Skipping a 2MB edge!\n" ));
				dwAddress = a2*TEXMEM_2MB_EDGE;
			}
		}

		if( (dwAddress + size > Tblock_max_address) || (Tblocks[Tblock_freelist_start].next<0) )	{ 
			#ifndef NDEBUG
			if ( Tblocks[Tblock_freelist_start].next < 0 )	{
				mprintf(( "GrGlideTexture: AllocateTexture out of blocks!  Get John.\n" ));
			}
			#endif

			//---- No room left, go back to start ---- 
			ReleaseSlot(Tblock_usedlist_start);

			nNewBlock = Tblock_usedlist_head = Tblock_usedlist_start; 
			dwAddress = Tblock_min_address; 
		} else { 

			//---- Make new block ---- 
			nNewBlock = AllocBlock(); 
		} 

		next = Tblocks[Tblock_usedlist_head].next; 

		//---- Unlink blocks being overwritten ---- 
		while((next >= 0) && (dwAddress + size > Tblocks[next].start_address)) { 

			int    nTemp = Tblocks[next].next; 

			FreeBlock(next); 
			next = nTemp; 
		} 

		//---- Init new block ---- 
		Tblocks[nNewBlock].next          = next; 
		Tblocks[nNewBlock].start_address = dwAddress; 
	} 

	Tblocks[nNewBlock].size = size; 
	Tblocks[nNewBlock].texture_ptr = texture_ptr;

	return Tblocks[nNewBlock].start_address; 
} 

void glide_tcache_set_initial_texture_mode()
{
	grTexMipMapMode( GR_TMU0, GR_MIPMAP_DISABLE, FXFALSE);
	grTexLodBiasValue( GR_TMU0, .5f);
	grTexClampMode( GR_TMU0, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);
	grTexFilterMode( GR_TMU0, GR_TEXTUREFILTER_BILINEAR, GR_TEXTUREFILTER_BILINEAR);
}

void glide_tcache_init()
{
	// if we're already inited, allocate nothing new)
	if(Textures == NULL){
		Textures = (tcache_slot *)malloc((MAX_TEXTURES)*sizeof(tcache_slot));
		if ( !Textures )	{
			exit(1);	
		}
	}

	// AUX	
	/*
	Textures_aux = (tcache_slot*)malloc((MAX_TEXTURES) * MAX_BMAP_SECTIONS_X * MAX_BMAP_SECTIONS_Y sizeof(tcache_slot));
	if( !Textures_aux){
		exit(1);
	}
	*/
	// if we're already inited, allocate nothing new)
	if(Texture_sections == NULL){
		Texture_sections = malloc(MAX_TEXTURES * MAX_BMAP_SECTIONS_X * MAX_BMAP_SECTIONS_Y * sizeof(tcache_slot));
		if(!Texture_sections){
			exit(1);
		}
	}

	// if we're already inited, allocate nothing new)
	if(Glide_tmp_ram == NULL){
		Glide_tmp_ram = (ubyte *)malloc(256*256*2);
		if ( !Glide_tmp_ram )	{
			exit(1);
		}
	}

	Glide_total_memory = guTexMemQueryAvail(GR_TMU0);
	Glide_start_address = grTexMinAddress(GR_TMU0);
	Glide_end_address = grTexMaxAddress(GR_TMU0);
	Glide_current_address = Glide_start_address;

	mprintf(( "Total texture memory on 3dfx card=%d bytes\n", Glide_total_memory ));

	glide_tcache_set_initial_texture_mode();
	
	InitBlocks(Glide_start_address,Glide_end_address, MAX_TEXTURES);

	// Init the texture structures	
	int i, idx, s_idx, count;
	count = 0;
	for( i=0; i<MAX_TEXTURES; i++ )	{
		Textures[i].bitmap_id = -1;		
		Textures[i].parent = NULL;
		
		for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
			for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
				// AUX
				// Textures[i].data_sections[idx][s_idx] = NULL;
				// Textures[i].data_sections[idx][s_idx] = (tcache_slot*)malloc(sizeof(tcache_slot));
				Textures[i].data_sections[idx][s_idx] = &((tcache_slot*)Texture_sections)[count++];
				Textures[i].data_sections[idx][s_idx]->parent = &Textures[i];
			}
		}		
	}
	
	// AUX
	/*
	for( i=0; i<MAX_AUX_TEXTURES; i++){
		Textures_aux[i].bitmap_id = -1;
		
		for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
			for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
				Textures_aux[i].data_sections[idx][s_idx] = NULL;
			}
		}
	}
	*/

	Glide_textures_in = 0;
	Glide_last_bitmap_id = -1;
	Glide_last_section_x = -1;
	Glide_last_section_y = -1;

	Glide_last_detail = Detail.hardware_textures;
}

void glide_tcache_cleanup()
{
	if ( Textures )	{
		free(Textures);
		Textures = NULL;
	}

	// AUX
	/*
	if ( Textures_aux )	{
		free(Textures_aux);
		Textures_aux = NULL;
	}
	*/
	if(Texture_sections){
		free(Texture_sections);
		Texture_sections = NULL;
	}

	if ( Glide_tmp_ram )	{
		free(Glide_tmp_ram);
		Glide_tmp_ram = NULL;
	}

	ReleaseBlocks();
}

void glide_tcache_flush()
{
	if ( grSstIsBusy() )	{
		//mprintf(( "Pausing until 3DFX is idle before clearing texture RAM...\n" ));
		grSstIdle();	// Wait for idle
	}

	Glide_current_address = Glide_start_address;

	// Init the texture structures
	int i; //, idx, s_idx;
	for( i=0; i<MAX_TEXTURES; i++ )	{
		Textures[i].bitmap_id = -1;		

		// AUX
		/*
		for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
			for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
				Textures[i].data_sections[idx][s_idx] = NULL;
			}
		}
		*/
	}
	
	// AUX
	/*
	for( i=0; i<MAX_AUX_TEXTURES; i++){
		Textures_aux[i].bitmap_id = -1;
		
		for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
			for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
				Textures_aux[i].data_sections[idx][s_idx] = NULL;
			}
		}
	}
	*/

	Glide_textures_in = 0;
	Glide_last_bitmap_id = -1;	
	Glide_last_section_x = -1;
	Glide_last_section_y = -1;


	Glide_explosion_vram = 0;

	FlushBlocks();
}

// get a free aux texture
// AUX
/*
int ga_reentrant = 0;
tcache_slot *glide_get_aux_slot()
{
	int idx;
	int index = -1;
	tcache_slot *ret;

	// never go in here more than twice
	if(ga_reentrant > 1){
		return NULL;
	}

	ga_reentrant++;

	for(idx=0; idx<MAX_AUX_TEXTURES; idx++){
		if(Textures_aux[idx].bitmap_id == -1){
			index = idx;
			break;
		}
	}

	// OH NO. This means we've run out of slots for bitmap sections. Let's just blast all of them free. whee!
	if(index == -1){
		mprintf(("AIEEEE! FLUSHIN SECTIONED BITMAPS!\n"));
		for(idx=0; idx<MAX_TEXTURES; idx++){
			if((Textures[idx].bitmap_id > -1) && (Textures[idx].data_sections[0][0] != NULL)){
				ReleaseSlot(&Textures[idx] - Textures);
			}
		}
			
		ret = glide_get_aux_slot();

		ga_reentrant--;
		return ret;
	}

	// hmm
	int s_idx;
	Textures_aux[index].bitmap_id = -1;	
	Textures_aux[index].data.vram_offset = 0;
	Textures_aux[index].data.vram_size = 0;
	for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
		for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
			Textures_aux[index].data_sections[idx][s_idx] = NULL;			
		}
	}

	ret = &Textures_aux[index];
	ga_reentrant--;
	return ret;	
}
*/

void glide_tcache_frame()
{
	Glide_textures_in_frame = 0;
}

extern int palman_is_nondarkening(int r,int g, int b);

// data == start of bitmap data
// sx == x offset into bitmap
// sy == y offset into bitmap
// src_w == absolute width of section on source bitmap
// src_h == absolute height of section on source bitmap
// bmap_w == width of source bitmap
// bmap_h == height of source bitmap
// tex_w == width of final texture
// tex_h == height of final texture
int glide_create_texture_sub(int bitmap_type, int bitmap_handle, ushort *data, int sx, int sy, int src_w, int src_h, int bmap_w, int bmap_h, int tex_w, int tex_h, tcache_slot *tslot)
{	
	int i, j;	

	// sanity - make sure we're not going to run off the end of the bitmap
#ifndef NDEBUG
	Assert(src_w <= (bmap_w - sx));
	Assert(src_h <= (bmap_h - sy));
#endif

	if ( tex_w <= 16 ) tex_w = 16;
	else if ( tex_w <= 32 ) tex_w = 32;
	else if ( tex_w <= 64 ) tex_w = 64;
	else if ( tex_w <= 128 ) tex_w = 128;
	else {
		tex_w = 256;
	}

	if ( tex_h <= 16 ) tex_h = 16;
	else if ( tex_h <= 32 ) tex_h = 32;
	else if ( tex_h <= 64 ) tex_h = 64;
	else if ( tex_h <= 128 ) tex_h = 128;
	else {
		tex_h = 256;
	}	

	// these can never be shrunk. meaning, the texture _must_ be big enough to hold every pixel
	if ( (bitmap_type == TCACHE_TYPE_AABITMAP) || (bitmap_type == TCACHE_TYPE_BITMAP_SECTION) )	{
		// W & H must be within 8x of each other.	
		if ( (tex_w==16) && (tex_h==256) )	{
			tex_w = 32;
		} else if ( (tex_w==256) && (tex_h==16) )	{
			tex_h = 32;
		}
	} else {
		// W & H must be within 8x of each other.	
		if ( (tex_w==16) && (tex_h==256) )	{
			tex_h = 128;
		} else if ( (tex_w==256) && (tex_h==16) )	{
			tex_w = 128;
		}
	}

	ushort *bmp_data = data;
	ubyte *bmp_data_byte = (ubyte*)data;

	GrTextureFormat_t tex_format;

	if ( bitmap_type == TCACHE_TYPE_AABITMAP )	{
		tex_format = GR_TEXFMT_ALPHA_8;
		
		ubyte *lpSP;
		ubyte pix;
		ubyte xlat[256];
		
		for (i=0; i<16; i++ )	{
			xlat[i] = ubyte(Gr_gamma_lookup[(i*255)/15]);
		}
		xlat[15] = xlat[1];
		pix = 255;
		for ( ; i<256; i++ )	{
			xlat[i] = pix;
		}

		// upload to temp ram
		for (j = 0; j < tex_h; j++) {
			// the proper line in the temp ram
			lpSP = (ubyte*)(Glide_tmp_ram + tex_w * j);

			// upload the line of texture from the source bitmap
			for (i = 0; i < tex_w; i++) {
				// if we're within the bounds of the section we're copying
				if ( (j < src_h) && (i < src_w) )	{
					*lpSP++ = xlat[(ubyte)bmp_data_byte[(j * bmap_w) + i + sx]];
				}
				// otherwise just copy black
				else {
					*lpSP++ = 0;
				}
			}
		}
	} else if(bitmap_type == TCACHE_TYPE_BITMAP_SECTION){
		tex_format = GR_TEXFMT_ARGB_1555;				
		ushort *lpSP;						

		for (j = 0; j < src_h; j++) {
			// the proper line in the temp ram
			lpSP = (unsigned short*)(Glide_tmp_ram + tex_w * 2 * j);

			// nice and clean
			for (i = 0; i < src_w; i++) {												
				// stuff the texture into vram
				*lpSP++ = bmp_data[((j+sy) * bmap_w) + sx + i];
			}			
		}
	} else {		
		tex_format = GR_TEXFMT_ARGB_1555;		
		fix u, utmp, v, du, dv;				
		ushort *lpSP;		
		
		u = 0;

		// source line on the bitmap;		
		v = i2f(sy);

		// scale through the texture		
		du = (src_w * F1_0) / tex_w;
		dv = (src_h * F1_0) / tex_h;

		for (j = 0; j < tex_h; j++) {
			// the proper line in the temp ram
			lpSP = (unsigned short*)(Glide_tmp_ram + tex_w * 2 * j);

			// pixel offset on this individual line of the source bitmap
			utmp = u + i2f(sx);

			// nice and clean
			for (i = 0; i < tex_w; i++) {												
				// stuff the texture into vram
				*lpSP++ = bmp_data[f2i(v) * bmap_w + f2i(utmp)];

				// next pixel
				utmp += du;				
			}

			// next line in the source bitmap
			v += dv;
		}
	}	

	GrLOD_t lod=GR_LOD_16;

	int longest = max(tex_w, tex_h);

	float uscale, vscale;		
	if ( bitmap_type == TCACHE_TYPE_AABITMAP )	{
		uscale = i2fl(bmap_w)*256.0f / i2fl(longest);
		vscale = i2fl(bmap_h)*256.0f / i2fl(longest);
	} else if(bitmap_type == TCACHE_TYPE_BITMAP_SECTION){
		uscale = i2fl(src_w)*256.0f / i2fl(longest);
		vscale = i2fl(src_h)*256.0f / i2fl(longest);
	} else {
		uscale = i2fl(tex_w)*256.0f / i2fl(longest);
		vscale = i2fl(tex_h)*256.0f / i2fl(longest);
	}

	switch( longest )	{
	case 16:		lod = GR_LOD_16; break;
	case 32:		lod = GR_LOD_32; break;
	case 64:		lod = GR_LOD_64; break;
	case 128:	lod = GR_LOD_128; break;
	case 256:	lod = GR_LOD_256; break;
	default:		Int3();
	}

	// GR_ASPECT_2x1	
	GrAspectRatio_t aspect = GR_ASPECT_1x1;		// aspect is widthxheight

	if ( tex_w < tex_h )	{
		int ratio = tex_h / tex_w;
		switch(ratio)	{
		case 1:	aspect = GR_ASPECT_1x1; break;
		case 2:	aspect = GR_ASPECT_1x2; break;
		case 4:	aspect = GR_ASPECT_1x4; break;
		case 8:	aspect = GR_ASPECT_1x8; break;
		default:	Int3();
		}
	} else {
		int ratio = tex_w / tex_h;
		switch(ratio)	{
		case 1:	aspect = GR_ASPECT_1x1; break;
		case 2:	aspect = GR_ASPECT_2x1; break;
		case 4:	aspect = GR_ASPECT_4x1; break;
		case 8:	aspect = GR_ASPECT_8x1; break;
		default:	Int3();
		}
	}


	GrTexInfo info;

	info.smallLod = lod;					//GR_LOD_256;
	info.largeLod = lod;					//GR_LOD_256;
	info.aspectRatio = aspect;			//GR_ASPECT_1x1;
	info.format = tex_format;				//GR_TEXFMT_ARGB_1555;
	info.data = Glide_tmp_ram;						//source data

	int bytes_needed = grTexTextureMemRequired( GR_MIPMAPLEVELMASK_BOTH, &info );

	// if	( (tslot->data.vram_offset == 0) || ((tslot->data.vram_offset>0)&&(bytes_needed>tslot->data.vram_size)))	{
		tslot->data.vram_offset = AllocateTexture( bytes_needed, tslot );
		tslot->data.vram_size = bytes_needed;
		Glide_textures_in += bytes_needed;
	// }

	tslot->data.format = tex_format;
	tslot->data.lod = lod;
	tslot->data.aspect = aspect;
	tslot->bitmap_id = bitmap_handle;
	tslot->uscale = uscale;
	tslot->vscale = vscale;	
	
	grTexDownloadMipMap(GR_TMU0, tslot->data.vram_offset, GR_MIPMAPLEVELMASK_BOTH, &info); 
	Glide_textures_in_frame += bytes_needed;	

	return 1;
}

int glide_create_texture( int bitmap_handle, int bitmap_type, tcache_slot *tslot )
{
	int ret;
	ubyte flags;
	bitmap *bmp;
	ubyte bpp = 16;

	// setup texture/bitmap flags
	flags = 0;
	switch(bitmap_type){
	case TCACHE_TYPE_AABITMAP:
		flags |= BMP_AABITMAP;
		bpp = 8;
		break;
	case TCACHE_TYPE_XPARENT:
	case TCACHE_TYPE_BITMAP_SECTION:
		flags |= BMP_TEX_XPARENT;
		break;
	case TCACHE_TYPE_NORMAL:
		flags |= BMP_TEX_OTHER;
		break;
	case TCACHE_TYPE_NONDARKENING:
		flags |= BMP_TEX_NONDARK;
		break;
	} 
		
	// lock the bitmap in the proper format
	bmp = bm_lock(bitmap_handle, bpp, flags);	
	if ( bmp == NULL ) {
		mprintf(("Couldn't lock bitmap %d.\n", bitmap_handle ));
		return 0;
	}

	int max_w = bmp->w;
	int max_h = bmp->h; 

	if ( bitmap_type != TCACHE_TYPE_AABITMAP )	{
		// Detail.debris_culling goes from 0 to 4.
		max_w /= 16>>Detail.hardware_textures;
		max_h /= 16>>Detail.hardware_textures;
	}	

	// call the helper
	ret = glide_create_texture_sub(bitmap_type, bitmap_handle, (ushort*)bmp->data, 0, 0, bmp->w, bmp->h, bmp->w, bmp->h, max_w, max_h, tslot);	

	// unlock the bitmap
	bm_unlock(bitmap_handle);

	// return
	return ret;
}

// create a sectioned texture
int glide_create_texture_sectioned(int bitmap_handle, int bitmap_type, tcache_slot *tslot, int sx, int sy)
{
	int ret;
	ubyte flags;
	bitmap *bmp;
	int section_x, section_y;

	// setup texture/bitmap flags
	Assert(bitmap_type == TCACHE_TYPE_BITMAP_SECTION);
	if(bitmap_type != TCACHE_TYPE_BITMAP_SECTION){
		bitmap_type = TCACHE_TYPE_BITMAP_SECTION;
	}
	flags = BMP_TEX_XPARENT;	
		
	// lock the bitmap in the proper format
	bmp = bm_lock(bitmap_handle, 16, flags);	
	if ( bmp == NULL ) {
		mprintf(("Couldn't lock bitmap %d.\n", bitmap_handle ));
		return 0;
	}	

	// determine the width and height of this section
	bm_get_section_size(bitmap_handle, sx, sy, &section_x, &section_y);	

	// call the helper
	ret = glide_create_texture_sub(bitmap_type, bitmap_handle, (ushort*)bmp->data, bmp->sections.sx[sx], bmp->sections.sy[sy], section_x, section_y, bmp->w, bmp->h, section_x, section_y, tslot);	

	// unlock the bitmap
	bm_unlock(bitmap_handle);

	// return
	return ret;
}


DCF(exp_flush, "")
{
	Glide_explosion_vram = 0;
}

extern int bm_get_cache_slot( int bitmap_id, int separate_ani_frames );

// Returns FALSE if error
int glide_tcache_set( int bitmap_id, int bitmap_type, float *u_ratio, float *v_ratio, int fail_on_full, int sx, int sy, int force )
{		
	bitmap *bmp;
	int idx, s_idx;

	if ( Glide_last_detail != Detail.hardware_textures )	{
		Glide_last_detail = Detail.hardware_textures;
		glide_tcache_flush();
	}

	// Check if this is the same as the last one... if so, we don't need to
	// do anything.
	if ( (Glide_last_bitmap_id == bitmap_id) && (Glide_last_bitmap_type==bitmap_type) && (Glide_last_section_x == sx) && (Glide_last_section_y == sy) && !force)	{
		*u_ratio = Glide_last_u_ratio;
		*v_ratio = Glide_last_v_ratio;
		return 1;
	}

	int n;
	n = bm_get_cache_slot( bitmap_id, 1 );

	tcache_slot * t = &Textures[n];

	// if this is a sectioned bitmap
	if(bitmap_type == TCACHE_TYPE_BITMAP_SECTION){		
		// if the texture sections haven't been created yet
		if((t->bitmap_id < 0) || (t->bitmap_id != bitmap_id) || force){

			if(t->bitmap_id < 0){
				t->data.vram_offset = 0;
			}			

			// lock the bitmap in the proper format
			bmp = bm_lock(bitmap_id, 16, BMP_TEX_XPARENT);	
			bm_unlock(bitmap_id);

			// first we need to get enough free aux textures			
			// AUX
			/*
			for(idx=0; idx<bmp->sections.num_x; idx++){				
				for(s_idx=0; s_idx<bmp->sections.num_y; s_idx++){
					// try and get a feww slot
					t->data_sections[idx][s_idx] = glide_get_aux_slot();
					if(t->data_sections[idx][s_idx] == NULL){
						Int3();
						return 0;
					}				
					t->data_sections[idx][s_idx]->bitmap_id = bitmap_id;
				}
			}
			*/

			// now lets do something for each texture			
			for(idx=0; idx<bmp->sections.num_x; idx++){
				for(s_idx=0; s_idx<bmp->sections.num_y; s_idx++){										
					if(t->bitmap_id < 0){
						t->data_sections[idx][s_idx]->data.vram_offset = 0;
					}

					t->data_sections[idx][s_idx]->bitmap_id = bitmap_id;
					glide_create_texture_sectioned( bitmap_id, bitmap_type, t->data_sections[idx][s_idx], idx, s_idx);
				}
			}

			t->bitmap_id = bitmap_id;
		}

		// swap in the texture we want				
		t = t->data_sections[sx][sy];
	}
	// all other "normal" textures
	else {
		// no texture yet
		if ( t->bitmap_id < 0) {
			t->data.vram_offset = 0;		
			glide_create_texture( bitmap_id, bitmap_type, t );
		}
		// different bitmap altogether
		else if ( (t->bitmap_id != bitmap_id) || force)	{
			glide_create_texture( bitmap_id, bitmap_type, t );
		}
	}		

	*u_ratio = t->uscale;
	*v_ratio = t->vscale;

	GrTexInfo info;

	info.smallLod = t->data.lod;				//GR_LOD_256;
	info.largeLod = t->data.lod;				//GR_LOD_256;
	info.aspectRatio = t->data.aspect;		//GR_ASPECT_1x1;
	info.format = t->data.format;				//GR_TEXFMT_ARGB_1555;
	info.data = 0;									//source data

	grTexSource( GR_TMU0, t->data.vram_offset, GR_MIPMAPLEVELMASK_BOTH, &info );

	// Save current state so we don't have to do anything time consuming next time
	// we set this exact same texture
	Glide_last_bitmap_id = bitmap_id;
	Glide_last_bitmap_type = bitmap_type;
	Glide_last_section_x = sx;
	Glide_last_section_y = sy;
	Glide_last_u_ratio = t->uscale;
	Glide_last_v_ratio = t->vscale;

	return 1;
}





