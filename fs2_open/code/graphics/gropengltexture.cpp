/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


/*
 * $Logfile: /Freespace2/code/Graphics/GrOpenGLTexture.cpp $
 * $Revision: 1.12 $
 * $Date: 2004-12-05 01:28:39 $
 * $Author: taylor $
 *
 * source for texturing in OpenGL
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.11  2004/12/02 11:14:29  taylor
 * not used at the moment but that code was stupid
 *
 * Revision 1.10  2004/11/29 18:02:01  taylor
 * make sure we are checking the right bitmap_id
 *
 * Revision 1.9  2004/10/31 21:46:10  taylor
 * Linux tree merge, better DDS support, texture panning
 *
 * Revision 1.8  2004/09/05 19:23:24  Goober5000
 * fixed a few warnings
 * --Goober5000
 *
 * Revision 1.7  2004/07/26 20:47:32  Kazan
 * remove MCD complete
 *
 * Revision 1.6  2004/07/21 00:03:46  taylor
 * fix problem with sectioned menu bitmaps in OGL
 *
 * Revision 1.5  2004/07/17 18:49:57  taylor
 * oops, I can't spell
 *
 * Revision 1.4  2004/07/17 18:43:46  taylor
 * don't use bitmap sections by default, openil_close()
 *
 * Revision 1.3  2004/07/12 16:32:48  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 1.2  2004/07/01 01:12:31  bobboau
 * implemented index buffered background bitmaps,
 * OGL people you realy should get this implemented
 *
 * Revision 1.1  2004/05/24 07:25:32  taylor
 * filename case change
 *
 * Revision 2.5  2004/05/06 22:26:52  taylor
 * fix DDS textures
 *
 * Revision 2.4  2004/04/26 12:43:58  taylor
 * minor fixes, HTL lines, 32-bit texture support
 *
 * Revision 2.3  2004/04/13 01:55:41  phreak
 * put in the correct fields for the CVS comments to register
 * fixed a glowmap problem that occured when rendering glowmapped and non-glowmapped ships
 *
 *
 * $NoKeywords: $
 */

#ifdef _WIN32
#include <windows.h>
#endif

#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"

#include "bmpman/bmpman.h"

#include "cmdline/cmdline.h"

#include "graphics/gropengl.h"
#include "graphics/gropengltexture.h"
#include "graphics/gropenglextension.h"
#include "graphics/grinternal.h"

#include "ddsutils/ddsutils.h"


static tcache_slot_opengl *Textures = NULL;

#ifdef GL_SECTIONS
static void *Texture_sections = NULL;
int GL_texture_sections = 0;
#endif // GL_SECTIONS
int GL_texture_ram = 0;
int GL_frame_count = 0;
int GL_min_texture_width = 0;
int GL_max_texture_width = 0;
int GL_min_texture_height = 0;
int GL_max_texture_height = 0;
int GL_square_textures = 0;
int GL_textures_in = 0;
int GL_textures_in_frame = 0;
int GL_last_bitmap_id = -1;
int GL_last_detail = -1;
int GL_last_bitmap_type = -1;
int GL_last_section_x = -1;
int GL_last_section_y = -1;
int GL_supported_texture_units = 2;
int GL_should_preload = 0;

extern int vram_full;
extern int GLOWMAP;
extern int SPECMAP;
extern int CLOAKMAP;
extern int Interp_multitex_cloakmap;

//opengl supports 32 multitexture units
//we will too incase people are playing fs2_open in 2020
static int GL_texture_units_enabled[32]={0};


void gr_opengl_set_additive_tex_env()
{
	if (opengl_extension_is_enabled(GL_ARB_ENV_COMBINE))
	{
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_ADD);
		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1.0f);
	}
	else if (GL_Extensions[GL_EXT_ENV_COMBINE].enabled)
	{
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_ADD);
		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, 1.0f);
	}
	else {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
	}
}

void gr_opengl_set_tex_env_scale(float scale)
{
	if (GL_Extensions[GL_ARB_ENV_COMBINE].enabled)
		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, scale);
	else if (GL_Extensions[GL_EXT_ENV_COMBINE].enabled)
		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_EXT, scale);
	else
	{}
}


void opengl_set_max_anistropy()
{
//	if (GL_Extensions[GL_TEX_FILTER_aniso].enabled)		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
}

void opengl_switch_arb(int unit, int state)
{
	if (unit >= GL_supported_texture_units)
		return;

	if (state)
	{
		if (GL_texture_units_enabled[unit])	return;

		glActiveTextureARB(GL_TEXTURE0_ARB + unit);
		glEnable(GL_TEXTURE_2D);
		GL_texture_units_enabled[unit] = 1;
	}

	else
	{
		if (!GL_texture_units_enabled[unit])	return;

		glActiveTextureARB(GL_TEXTURE0_ARB + unit);
		glDisable(GL_TEXTURE_2D);
		GL_texture_units_enabled[unit] = 0;
	}
}

void opengl_tcache_init (int use_sections)
{
#ifdef GL_SECTIONS
	int idx, s_idx;
#endif
	int i;

	// DDOI - FIXME skipped a lot of stuff here
	GL_should_preload = 0;

	//uint tmp_pl = os_config_read_uint( NULL, NOX("D3DPreloadTextures"), 255 );
	uint tmp_pl = 1;

	if ( tmp_pl == 0 )      {
		GL_should_preload = 0;
	} else if ( tmp_pl == 1 )       {
		GL_should_preload = 1;
	} else {
		GL_should_preload = 1;
	}

	GL_min_texture_width = 16;
	GL_min_texture_height = 16;
	
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &GL_max_texture_width);

	GL_max_texture_height=GL_max_texture_width;

	mprintf(("max texture size is: %dx%d\n", GL_max_texture_width,GL_max_texture_height));

	GL_square_textures = 0;

	Textures = (tcache_slot_opengl *)malloc(MAX_BITMAPS*sizeof(tcache_slot_opengl));
	if ( !Textures )        {
		exit(1);
	}

#ifdef GL_SECTIONS
	if(use_sections){
		Texture_sections = (tcache_slot_opengl*)malloc(MAX_BITMAPS * MAX_BMAP_SECTIONS_X * MAX_BMAP_SECTIONS_Y * sizeof(tcache_slot_opengl));
		if(!Texture_sections){
			exit(1);
		}
		memset(Texture_sections, 0, MAX_BITMAPS * MAX_BMAP_SECTIONS_X * MAX_BMAP_SECTIONS_Y * sizeof(tcache_slot_opengl));
	}
#endif // GL_SECTIONS

	// Init the texture structures
#ifdef GL_SECTIONS
	int section_count = 0;
#endif // GL_SECTIONS
	for( i=0; i<MAX_BITMAPS; i++ )  {
		/*
		Textures[i].vram_texture = NULL;
		Textures[i].vram_texture_surface = NULL;
		*/
		Textures[i].texture_handle = 0;

		Textures[i].bitmap_id = -1;
		Textures[i].size = 0;
		Textures[i].used_this_frame = 0;
		Textures[i].bpp = 0;

		Textures[i].parent = NULL;

#ifdef GL_SECTIONS
		// allocate sections
		if(use_sections){
			for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
				for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
					Textures[i].data_sections[idx][s_idx] = &((tcache_slot_opengl*)Texture_sections)[section_count++];
					Textures[i].data_sections[idx][s_idx]->parent = &Textures[i];
					/*
					Textures[i].data_sections[idx][s_idx]->vram_texture = NULL;
					Textures[i].data_sections[idx][s_idx]->vram_texture_surface = NULL;
					*/
					Textures[i].data_sections[idx][s_idx]->texture_handle = 0;
					Textures[i].data_sections[idx][s_idx]->bitmap_id = -1;
					Textures[i].data_sections[idx][s_idx]->size = 0;
					Textures[i].data_sections[idx][s_idx]->used_this_frame = 0;
					Textures[i].data_sections[idx][s_idx]->bpp = 0;
				}
			}
		} else {
			for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
				for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
					Textures[i].data_sections[idx][s_idx] = NULL;
				}
			}
		}
#endif // GL_SECTIONS
	}

#ifdef GL_SECTIONS
	GL_texture_sections = use_sections;
#endif // GL_SECTIONS

	//GL_last_detail = Detail.hardware_textures;
	GL_last_bitmap_id = -1;
	GL_last_bitmap_type = -1;

	GL_last_section_x = -1;
	GL_last_section_y = -1;

	GL_textures_in = 0;
	GL_textures_in_frame = 0;
}

int opengl_free_texture (tcache_slot_opengl *t);

void opengl_free_texture_with_handle(int handle)
{
	int n = bm_get_cache_slot(handle, 1);

	Textures[n].used_this_frame = 0;
	opengl_free_texture( &Textures[n] );
}

void opengl_tcache_flush ()
{
	int i;

	for( i=0; i<MAX_BITMAPS; i++ )  {
		opengl_free_texture ( &Textures[i] );
	}
	if (GL_textures_in != 0) {
		mprintf(( "WARNING: VRAM is at %d instead of zero after flushing!\n", GL_textures_in ));
		GL_textures_in = 0;
	}

	GL_last_bitmap_id = -1;
	GL_last_section_x = -1;
	GL_last_section_y = -1;
}

void opengl_tcache_cleanup ()
{
	opengl_tcache_flush ();

	GL_textures_in = 0;
	GL_textures_in_frame = 0;

	if ( Textures ) {
		free(Textures);
		Textures = NULL;
	}

#ifdef GL_SECTIONS
	if( Texture_sections != NULL ){
		free(Texture_sections);
		Texture_sections = NULL;
	}
#endif // GL_SECTIONS
}

void opengl_tcache_frame ()
{
#ifdef GL_SECTIONS
	int idx, s_idx;
#endif // GL_SECTIONS

	GL_last_bitmap_id = -1;
	GL_textures_in_frame = 0;

	GL_frame_count++;

	int i;
	for( i=0; i<MAX_BITMAPS; i++ )  {
		Textures[i].used_this_frame = 0;

#ifdef GL_SECTIONS
		// data sections
		if(Textures[i].data_sections[0][0] != NULL){
			Assert(GL_texture_sections);
			if(GL_texture_sections){
				for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
					for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
						if(Textures[i].data_sections[idx][s_idx] != NULL){
							Textures[i].data_sections[idx][s_idx]->used_this_frame = 0;
						}
					}
				}
			}
		}
#endif // GL_SECTIONS
	}

	if ( vram_full )        {
		opengl_tcache_flush();
		vram_full = 0;
	}
}

int opengl_free_texture ( tcache_slot_opengl *t )
{
#ifdef GL_SECTIONS
	int idx, s_idx;
#endif // GL_SECTIONS
	

	// Bitmap changed!!     
	if ( t->bitmap_id > -1 )        {
		// if I, or any of my children have been used this frame, bail  
		if(t->used_this_frame){
			return 0;
		}

#ifdef GL_SECTIONS
		for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
			for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
				if((t->data_sections[idx][s_idx] != NULL) && (t->data_sections[idx][s_idx]->used_this_frame)){
					return 0;
				}
			}
		}
#endif // GL_SECTIONS

		// ok, now we know its legal to free everything safely
		glDeleteTextures (1, &t->texture_handle);
		t->texture_handle = 0;

		if ( GL_last_bitmap_id == t->bitmap_id )       {
			GL_last_bitmap_id = -1;
		}

#ifdef GL_SECTIONS
		// if this guy has children, free them too, since the children
		// actually make up his size
		for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
			for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
				if(t->data_sections[idx][s_idx] != NULL){
					opengl_free_texture(t->data_sections[idx][s_idx]);
				}
			}
		}
#endif // GL_SECTIONS

		t->bitmap_id = -1;
		t->used_this_frame = 0;
		t->bpp = 0;
		GL_textures_in -= t->size;
	}

	return 1;
}

void opengl_tcache_get_adjusted_texture_size(int w_in, int h_in, int *w_out, int *h_out)
{
	int tex_w, tex_h;

	// bogus
	if((w_out == NULL) ||  (h_out == NULL)){
		return;
	}

	// starting size
	tex_w = w_in;
	tex_h = h_in;

	if (1)        {
		int i;
		for (i=0; i<16; i++ )   {
			if ( (tex_w > (1<<i)) && (tex_w <= (1<<(i+1))) )        {
				tex_w = 1 << (i+1);
				break;
			}
		}

		for (i=0; i<16; i++ )   {
			if ( (tex_h > (1<<i)) && (tex_h <= (1<<(i+1))) )        {
				tex_h = 1 << (i+1);
				break;
			}
		}
	}

	if ( tex_w < GL_min_texture_width ) {
		tex_w = GL_min_texture_width;
	} else if ( tex_w > GL_max_texture_width )     {
		tex_w = GL_max_texture_width;
	}

	if ( tex_h < GL_min_texture_height ) {
		tex_h = GL_min_texture_height;
	} else if ( tex_h > GL_max_texture_height )    {
		tex_h = GL_max_texture_height;
	}

	if ( GL_square_textures )      {
		int new_size;
		// Make the both be equal to larger of the two
		new_size = max(tex_w, tex_h);
		tex_w = new_size;
		tex_h = new_size;
	}

	// store the outgoing size
	*w_out = tex_w;
	*h_out = tex_h;
}

// data == start of bitmap data
// sx == x offset into bitmap
// sy == y offset into bitmap
// src_w == absolute width of section on source bitmap
// src_h == absolute height of section on source bitmap
// bmap_w == width of source bitmap
// bmap_h == height of source bitmap
// tex_w == width of final texture
// tex_h == height of final texture
int opengl_create_texture_sub(int bitmap_type, int texture_handle, ushort *data, int sx, int sy, int src_w, int src_h, int bmap_w, int bmap_h, int tex_w, int tex_h, tcache_slot_opengl *t, int reload, int fail_on_full)
{
	int ret_val = 1;
	int byte_mult = 0;
	GLenum texFormat = GL_UNSIGNED_BYTE;
	GLenum glFormat = GL_BGRA_EXT;
	GLuint intFormat = GL_RGBA;
	ubyte saved_bpp = 0;
	int mipmap_w = 0, mipmap_h = 0;
	int dsize = 0, doffset = 0;
	int i,j,k;

	// bogus
	if(t == NULL){
		return 0;
	}

	if ( t->used_this_frame )       {
		mprintf(( "ARGHH!!! Texture already used this frame!  Cannot free it!\n" ));
		return 0;
	}
	if ( !reload )  {
		// save the bpp since it will get reset - fixes anis being 0 bpp
		saved_bpp = t->bpp;

		// gah
		if(!opengl_free_texture(t)){
			return 0;
		}
	}

	// get final texture size
	opengl_tcache_get_adjusted_texture_size(tex_w, tex_h, &tex_w, &tex_h);

	if ( (tex_w < 1) || (tex_h < 1) )       {
		mprintf(("Bitmap is to small at %dx%d.\n", tex_w, tex_h ));
		return 0;
	}

	// for everything that might use mipmaps
	mipmap_w = tex_w;
	mipmap_h = tex_h;

#ifdef GL_SECTIONS
	if ( bitmap_type == TCACHE_TYPE_AABITMAP )      {
		t->u_scale = (float)bmap_w / (float)tex_w;
		t->v_scale = (float)bmap_h / (float)tex_h;
	} else if(bitmap_type == TCACHE_TYPE_BITMAP_SECTION){
		t->u_scale = (float)src_w / (float)tex_w;
		t->v_scale = (float)src_h / (float)tex_h;
#else
	if ( (bitmap_type == TCACHE_TYPE_AABITMAP) || (bitmap_type == TCACHE_TYPE_BITMAP_SECTION) ) {
		t->u_scale = (float)bmap_w / (float)tex_w;
		t->v_scale = (float)bmap_h / (float)tex_h;
#endif
	} else {
		t->u_scale = 1.0f;
		t->v_scale = 1.0f;
	}

	if (!reload) {
		glGenTextures (1, &t->texture_handle);
	}
	
	if (t->texture_handle == 0) {
		nprintf(("Error", "!!DEBUG!! t->texture_handle == 0"));
		return 0;
	}
	
	if (t->bpp == 0) {
		// it got reset, revert to saved setting
		t->bpp = saved_bpp;
	}

	// set the byte per pixel multiplier
	byte_mult = (t->bpp >> 3);

	// GL_BGRA_EXT is *much* faster with some hardware/drivers but never slower
	if (byte_mult == 4) {
		texFormat = GL_UNSIGNED_INT_8_8_8_8_REV;
		intFormat = GL_RGBA;
		glFormat = GL_BGRA_EXT;
	} else if (byte_mult == 3) {
		texFormat = GL_UNSIGNED_BYTE;
		intFormat = GL_RGB;
		glFormat = GL_BGR_EXT;
	} else if (byte_mult == 2) {
		texFormat = GL_UNSIGNED_SHORT_1_5_5_5_REV;
		intFormat = GL_RGBA;
		glFormat = GL_BGRA_EXT;
	} else if (byte_mult == 1) {
		texFormat = GL_UNSIGNED_BYTE;
	} else {
		texFormat = GL_UNSIGNED_SHORT_1_5_5_5_REV;
		intFormat = GL_RGBA;
		glFormat = GL_BGRA_EXT;
	}

	glBindTexture (GL_TEXTURE_2D, t->texture_handle);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	switch (bitmap_type) {

		case TCACHE_TYPE_COMPRESSED:
		{
			GLenum ctype = UNINITIALIZED;
			int block_size = 0;

			switch (bm_is_compressed(texture_handle))
			{
				case DDS_DXT1:
					ctype = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
					block_size = 8;
					break;

				case DDS_DXT3:
					ctype = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
					block_size = 16;
					break;

				case DDS_DXT5:
					ctype = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
					block_size = 16;
					break;

				default:
					Assert(0);
			}

			if (block_size > 0) {
				// render for each mipmap level
				for (i = 0; i<bm_get_num_mipmaps(texture_handle); i++) {
					// size of data block (4x4)
					dsize = ((mipmap_h + 3) / 4) * ((mipmap_w + 3) / 4) * block_size;
					
					glCompressedTexImage2D(GL_TEXTURE_2D, i, ctype, mipmap_w, mipmap_h, 0, dsize, (ubyte*)data + doffset);

					// adjust the data offset for the next block
					doffset += dsize;

					// reduce size by half for the next pass
					mipmap_w /= 2;
					mipmap_h /= 2;
				}
			}
		}
		break;
	
		case TCACHE_TYPE_AABITMAP:
		{
			ubyte *bmp_data = ((ubyte*)data);
			ubyte *texmem = (ubyte *) malloc (tex_w*tex_h*2);
			ubyte *texmemp = texmem;
			ubyte xlat[256];
			
			for (i=0; i<16; i++) {
				xlat[i] = (ubyte)Gr_gamma_lookup[(i*255)/15];
			}	
			xlat[15] = xlat[1];
			for ( ; i<256; i++ )    {
				xlat[i] = xlat[0];
			}

			for (i=0;i<tex_h;i++)
			{
				for (j=0;j<tex_w;j++)
				{
				if (i < bmap_h && j < bmap_w) {
						*texmemp++ = 0xff;
						*texmemp++ = xlat[bmp_data[i*bmap_w+j]];
					} else {
						*texmemp++ = 0;
						*texmemp++ = 0;
					}
				}
			}

			if (!reload)
				glTexImage2D (GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, tex_w, tex_h, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, texmem);
			else // faster anis
				glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, tex_w, tex_h, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, texmem);

			free (texmem);
		}
		break;


		case TCACHE_TYPE_BITMAP_SECTION:
		{
			ubyte *bmp_data = ((ubyte*)data);
			ubyte *texmem = (ubyte *) malloc (tex_w*tex_h*byte_mult);
			ubyte *texmemp = texmem;

			for (i=0;i<tex_h;i++)
			{
				for (j=0;j<tex_w;j++)
				{
					if (i < src_h && j < src_w) {
						for (k = 0; k < byte_mult; k++) {
							*texmemp++ = bmp_data[((i+sy)*bmap_w+(j+sx))*byte_mult+k];
						}
					} else {
						for (k = 0; k < byte_mult; k++) {
							*texmemp++ = 0;
						}
					}
				}
			}

			for (i = 0; i<bm_get_num_mipmaps(texture_handle); i++) {
				dsize = mipmap_h * mipmap_w * byte_mult;

				if (!reload)
					glTexImage2D (GL_TEXTURE_2D, i, intFormat, mipmap_w, mipmap_h, 0, glFormat, texFormat, texmem + doffset);
				else // faster anis
					glTexSubImage2D (GL_TEXTURE_2D, i, 0, 0, mipmap_w, mipmap_h, glFormat, texFormat, texmem + doffset);

				doffset += dsize;
				mipmap_h /= 2;
				mipmap_w /= 2;
			}

			free(texmem);
			break;
		}

		default:
		{
			ubyte *bmp_data = ((ubyte*)data);
			ubyte *texmem = (ubyte *) malloc (tex_w*tex_h*byte_mult);
			ubyte *texmemp = texmem;

			fix u, utmp, v, du, dv;

			u = v = 0;

			du = ( (bmap_w-1)*F1_0 ) / tex_w;
			dv = ( (bmap_h-1)*F1_0 ) / tex_h;

			for (j=0;j<tex_h;j++)
			{
				utmp = u;
				for (i=0;i<tex_w;i++)
				{
					for (k = 0; k < byte_mult; k++) {
						*texmemp++ = bmp_data[(f2i(v)*bmap_w+f2i(utmp))*byte_mult+k];
					}
					utmp += du;
				}
				v += dv;
			}

			for (i = 0; i<bm_get_num_mipmaps(texture_handle); i++) {
				dsize = mipmap_h * mipmap_w * byte_mult;

				if (!reload)
					glTexImage2D (GL_TEXTURE_2D, i, intFormat, mipmap_w, mipmap_h, 0, glFormat, texFormat, texmem + doffset);
				else // faster anis
					glTexSubImage2D (GL_TEXTURE_2D, i, 0, 0, mipmap_w, mipmap_h, glFormat, texFormat, texmem + doffset);

				doffset += dsize;
				mipmap_h /= 2;
				mipmap_w /= 2;
			}

			free(texmem);
			break;
		}
	}//end switch
	
	t->bitmap_id = texture_handle;
	t->time_created = GL_frame_count;
	t->used_this_frame = 0;
	if (bitmap_type & TCACHE_TYPE_COMPRESSED) t->size=bm_get_size(texture_handle);
	else	t->size = tex_w * tex_h * byte_mult;
	t->w = (ushort)tex_w;
	t->h = (ushort)tex_h;
	GL_textures_in_frame += t->size;
	if (!reload) {
		GL_textures_in += t->size;
	}

	return ret_val;
}

int opengl_create_texture (int bitmap_handle, int bitmap_type, tcache_slot_opengl *tslot, int fail_on_full)
{
	ubyte flags;
	bitmap *bmp;
	int final_w, final_h;
	ubyte bpp = 16;
	int reload = 0;

	// setup texture/bitmap flags
	flags = 0;
	switch(bitmap_type){
		case TCACHE_TYPE_AABITMAP:
			flags |= BMP_AABITMAP;
			bpp = 8;
			break;
		case TCACHE_TYPE_NORMAL:
			flags |= BMP_TEX_OTHER;
			break;
		case TCACHE_TYPE_XPARENT:
			flags |= BMP_TEX_XPARENT;
			break;
#ifndef GL_SECTIONS
		case TCACHE_TYPE_BITMAP_SECTION:
			flags |= BMP_TEX_XPARENT;
			break;
#endif // !GL_SECTIONS
		case TCACHE_TYPE_NONDARKENING:
			Int3();
			flags |= BMP_TEX_NONDARK;
			break;
	}
	
	switch (bm_is_compressed(bitmap_handle))
	{
		case DDS_DXT1:				//dxt1
			bpp = 24;
			flags |= BMP_TEX_DXT1;
			break;

		case DDS_DXT3:				//dxt3
			bpp = 32;
			flags |= BMP_TEX_DXT3;
			break;

		case DDS_DXT5:				//dxt5
			bpp = 32;
			flags |= BMP_TEX_DXT5;
			break;
		
		default:
			break;
	}

	// lock the bitmap into the proper format
	bmp = bm_lock(bitmap_handle, bpp, flags);
	if ( bmp == NULL ) {
		mprintf(("Couldn't lock bitmap %d.\n", bitmap_handle ));
		return 0;
	}

	int max_w = bmp->w;
	int max_h = bmp->h;

	// set the bits per pixel
	tslot->bpp = bmp->bpp;
	
	   // DDOI - TODO
	if ( (bitmap_type != TCACHE_TYPE_AABITMAP) && (bitmap_type != TCACHE_TYPE_BITMAP_SECTION) )      {
		// max_w /= D3D_texture_divider;
		// max_h /= D3D_texture_divider;

		// Detail.debris_culling goes from 0 to 4.
		max_w /= (16 >> Detail.hardware_textures);
		max_h /= (16 >> Detail.hardware_textures);
	}
	

	// get final texture size as it will be allocated as a DD surface
	opengl_tcache_get_adjusted_texture_size(max_w, max_h, &final_w, &final_h); 

	// if this tcache slot has no bitmap
	if ( tslot->bitmap_id < 0) {
		reload = 0;
	}
	// different bitmap altogether - determine if the new one can use the old one's slot
	else if (tslot->bitmap_id != bitmap_handle)     {
		if((final_w == tslot->w) && (final_h == tslot->h)){
			reload = 1;
			//ml_printf("Reloading texture %d\n", bitmap_handle);
		} else {
			reload = 0;
		}
	}

	// call the helper
	int ret_val = opengl_create_texture_sub(bitmap_type, bitmap_handle, (ushort*)bmp->data, 0, 0, bmp->w, bmp->h, bmp->w, bmp->h, max_w, max_h, tslot, reload, fail_on_full);

	// unlock the bitmap
	bm_unlock(bitmap_handle);

	return ret_val;
}

#ifdef GL_SECTIONS
int opengl_create_texture_sectioned(int bitmap_handle, int bitmap_type, tcache_slot_opengl *tslot, int sx, int sy, int fail_on_full)
{
	ubyte flags;
	bitmap *bmp;
	int final_w, final_h;
	int section_x, section_y;
	int reload = 0;

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

	// set the bits per pixel
	tslot->bpp = bmp->bpp;

	// determine the width and height of this section
	bm_get_section_size(bitmap_handle, sx, sy, &section_x, &section_y);

	// get final texture size as it will be allocated as an opengl texture
	opengl_tcache_get_adjusted_texture_size(section_x, section_y, &final_w, &final_h);

	// if this tcache slot has no bitmap
	if ( tslot->bitmap_id < 0) {
		reload = 0;
	}
	// different bitmap altogether - determine if the new one can use the old one's slot
	else if (tslot->bitmap_id != bitmap_handle)     {
		if((final_w == tslot->w) && (final_h == tslot->h)){
			reload = 1;
		} else {
			reload = 0;
		}
	}

	// call the helper
	int ret_val = opengl_create_texture_sub(bitmap_type, bitmap_handle, (ushort*)bmp->data, bmp->sections.sx[sx], bmp->sections.sy[sy], section_x, section_y, bmp->w, bmp->h, section_x, section_y, tslot, reload, fail_on_full);

	// unlock the bitmap
	bm_unlock(bitmap_handle);

	return ret_val;
}
#endif // GL_SECTIONS

int gr_opengl_tcache_set_internal(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full = 0, int sx = -1, int sy = -1, int force = 0, int tex_unit = 0)
{
#ifdef GL_SECTIONS
	bitmap *bmp = NULL;

	int idx, s_idx;
#endif // GL_SECTIONS
	int ret_val = 1;

	if ( GL_last_detail != Detail.hardware_textures )      {
		GL_last_detail = Detail.hardware_textures;
		opengl_tcache_flush();
	}

	if (vram_full) {
		return 0;
	}

	int n = bm_get_cache_slot (bitmap_id, 1);
	tcache_slot_opengl *t = &Textures[n];

	if ( (GL_last_bitmap_id == bitmap_id) && (GL_last_bitmap_type==bitmap_type) && (t->bitmap_id == bitmap_id) && (GL_last_section_x == sx) && (GL_last_section_y == sy))       {
		t->used_this_frame++;

#ifdef GL_SECTIONS
		// mark all children as used
		if(GL_texture_sections){
			for(idx=0; idx<MAX_BMAP_SECTIONS_X; idx++){
				for(s_idx=0; s_idx<MAX_BMAP_SECTIONS_Y; s_idx++){
					if(t->data_sections[idx][s_idx] != NULL){
						t->data_sections[idx][s_idx]->used_this_frame++;
					}
				}
			}
		}
#endif // GL_SECTIONS

		*u_scale = t->u_scale;
		*v_scale = t->v_scale;
		return 1;
	}


	glActiveTextureARB(GL_TEXTURE0_ARB+tex_unit);

	opengl_set_max_anistropy();

#ifdef GL_SECTIONS
	if (bitmap_type == TCACHE_TYPE_BITMAP_SECTION){
		Assert((sx >= 0) && (sy >= 0) && (sx < MAX_BMAP_SECTIONS_X) && (sy < MAX_BMAP_SECTIONS_Y));
		if(!((sx >= 0) && (sy >= 0) && (sx < MAX_BMAP_SECTIONS_X) && (sy < MAX_BMAP_SECTIONS_Y))){
			return 0;
		}

		ret_val = 1;

		// if the texture sections haven't been created yet
		if((t->bitmap_id < 0) || (t->bitmap_id != bitmap_id)){

			// lock the bitmap in the proper format
			bmp = bm_lock(bitmap_id, 16, BMP_TEX_XPARENT);

			// set the bits per pixel
			t->bpp = bmp->bpp;

			// now lets do something for each texture

			for(idx=0; idx<bmp->sections.num_x; idx++){
				for(s_idx=0; s_idx<bmp->sections.num_y; s_idx++){
					// hmm. i'd rather we didn't have to do it this way...
					if(!opengl_create_texture_sectioned(bitmap_id, bitmap_type, t->data_sections[idx][s_idx], idx, s_idx, fail_on_full)){
						ret_val = 0;
					}

					// not used this frame
					t->data_sections[idx][s_idx]->used_this_frame = 0;
				}
			}

			// unlock the bitmap now that we're good and done with it
			bm_unlock(bitmap_id);

			// zero out pretty much everything in the parent struct since he's just the root
			t->bitmap_id = bitmap_id;
			t->texture_handle = 0;
			t->time_created = t->data_sections[sx][sy]->time_created;
			t->used_this_frame = 0;
			/*
			t->vram_texture = NULL;
			t->vram_texture_surface = NULL
			*/
		}

		// argh. we failed to upload. free anything we can
		if(!ret_val){
			opengl_free_texture(t);
		}
		// swap in the texture we want
		else {
			t = t->data_sections[sx][sy];
		}
	}

	// all other "normal" textures
	else
#endif // GL_SECTIONS
		 if((t->bitmap_id < 0) || (bitmap_id != t->bitmap_id)){
		ret_val = opengl_create_texture( bitmap_id, bitmap_type, t, fail_on_full );
	}

	// everything went ok
	if(ret_val && (t->texture_handle) && !vram_full){
		*u_scale = t->u_scale;
		*v_scale = t->v_scale;

		
		glBindTexture (GL_TEXTURE_2D, t->texture_handle );

		GL_last_bitmap_id = t->bitmap_id;
		GL_last_bitmap_type = bitmap_type;
		GL_last_section_x = sx;
		GL_last_section_y = sy;
		t->used_this_frame++;
	}
	// gah
	else {
		glBindTexture (GL_TEXTURE_2D, 0);	// test - DDOI
		return 0;
	}

	return 1;
}
//extern int bm_get_cache_slot( int bitmap_id, int separate_ani_frames );
int gr_opengl_tcache_set(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full, int sx, int sy, int force)
{
	int r1=0,r2=1,r3=1;

	if (bitmap_id < 0)
	{
		GL_last_bitmap_id = -1;
		return 0;
	}

	// set compressed type if it's so, needed to be right later
	if (bm_is_compressed(bitmap_id) > 0) {
		bitmap_type = TCACHE_TYPE_COMPRESSED;
	}

	//make sure textuing is on
	opengl_switch_arb(0,1);

	r1=gr_opengl_tcache_set_internal(bitmap_id, bitmap_type, u_scale, v_scale, fail_on_full, sx, sy, force, 0);

	if (Interp_multitex_cloakmap>0)
	{
		// no point in using glow or spec maps when this is on so make sure they are off
		GLOWMAP = -1;
		SPECMAP = -1;

		opengl_switch_arb(1,1);

		glActiveTextureARB(GL_TEXTURE1_ARB);
		gr_opengl_set_additive_tex_env();

		r2=gr_opengl_tcache_set_internal(Interp_multitex_cloakmap, bm_get_tcache_type(Interp_multitex_cloakmap), u_scale, v_scale, fail_on_full, sx, sy, force, 1);

		r3=1;
		opengl_switch_arb(2,0);
	}

	if ((GLOWMAP>-1) && !Cmdline_noglow)
	{
		opengl_switch_arb(1,1);
	
		//set the glowmap stuff
		glActiveTextureARB(GL_TEXTURE1_ARB);
		gr_opengl_set_additive_tex_env();

		r2=gr_opengl_tcache_set_internal(GLOWMAP, bm_get_tcache_type(GLOWMAP), u_scale, v_scale, fail_on_full, sx, sy, force, 1);
	}
	else
	{
		opengl_switch_arb(1,0);
		r2=1;		
	}

//	if ((SPECMAP > -1) && !Cmdline_nospec && (GL_supported_texture_units > 2))
	if (0)
	{
		opengl_switch_arb(2,1);

		glActiveTextureARB(GL_TEXTURE2_ARB);
		// tex env thingy - batteries not included

		r3=gr_opengl_tcache_set_internal(SPECMAP, bm_get_tcache_type(SPECMAP), u_scale, v_scale, fail_on_full, sx, sy, force, 2);
	}
	else
	{
		opengl_switch_arb(2,0);
		r3=1;
	}

	return ((r1) && (r2) && (r3));

}

void gr_opengl_preload_init()
{
	if (gr_screen.mode != GR_OPENGL) {
		return;
	}

	opengl_tcache_flush ();
}

int gr_opengl_preload(int bitmap_num, int is_aabitmap)
{
	if ( gr_screen.mode != GR_OPENGL) {
		return 0;
	}

	if ( !GL_should_preload )      {
		return 0;
	}

	float u_scale, v_scale;

	int retval;
	if ( is_aabitmap )      {
		retval = gr_tcache_set(bitmap_num, TCACHE_TYPE_AABITMAP, &u_scale, &v_scale, 1 );
	} else {
		retval = gr_tcache_set(bitmap_num, TCACHE_TYPE_NORMAL, &u_scale, &v_scale, 1 );
	}

	if ( !retval )  {
		mprintf(("Texture upload failed!\n" ));
	}

	return retval;
}

static int GL_texture_panning_enabled = 0;
void gr_opengl_set_texture_panning(float u, float v, bool enable)
{
	if (Cmdline_nohtl)
		return;

	GLint current_matrix;

	if (enable) {
		glGetIntegerv(GL_MATRIX_MODE, &current_matrix);
		glMatrixMode( GL_TEXTURE );
		glPushMatrix();
		glTranslatef( u, v, 0.0f );
		glMatrixMode(current_matrix);

		GL_texture_panning_enabled = 1;
	} else if (GL_texture_panning_enabled) {
		glGetIntegerv(GL_MATRIX_MODE, &current_matrix);
		glMatrixMode( GL_TEXTURE );
		glPopMatrix();
		glMatrixMode(current_matrix);
	
		GL_texture_panning_enabled = 0;
	}
}
