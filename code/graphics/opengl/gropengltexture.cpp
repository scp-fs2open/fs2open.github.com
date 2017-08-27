/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifdef _WIN32
#include <windows.h>
#endif

#include "bmpman/bmpman.h"
#include "cmdline/cmdline.h"
#include "ddsutils/ddsutils.h"
#include "globalincs/systemvars.h"
#include "graphics/grinternal.h"
#include "gropenglstate.h"
#include "gropengltexture.h"
#include "math/vecmat.h"
#include "osapi/osregistry.h"


static tcache_slot_opengl *Textures = NULL;

matrix4 GL_texture_matrix;

int GL_texture_ram = 0;
int GL_min_texture_width = 0;
GLint GL_max_texture_width = 0;
int GL_min_texture_height = 0;
GLint GL_max_texture_height = 0;
int GL_textures_in_frame = 0;
int GL_last_detail = -1;
GLint GL_supported_texture_units = 2;
int GL_should_preload = 0;
GLfloat GL_anisotropy = 1.0f;
GLfloat GL_max_anisotropy = -1.0f;
int GL_mipmap_filter = 0;
GLenum GL_texture_target = GL_TEXTURE_2D;
GLenum GL_texture_face = GL_TEXTURE_2D;
GLenum GL_texture_addressing = GL_REPEAT;
bool GL_rendering_to_texture = false;
GLint GL_max_renderbuffer_size = 0;

extern int GLOWMAP;
extern int SPECMAP;
extern int CLOAKMAP;
extern int ENVMAP;
extern int Interp_multitex_cloakmap;


// forward declarations
int opengl_free_texture(tcache_slot_opengl *t);
int opengl_create_texture (int bitmap_handle, int bitmap_type, tcache_slot_opengl *tslot = NULL);

extern int get_num_mipmap_levels(int w, int h);

void opengl_set_additive_tex_env()
{
	GL_CHECK_FOR_ERRORS("start of set_additive_tex_env()");


	GL_CHECK_FOR_ERRORS("end of set_additive_tex_env()");
}

void opengl_set_modulate_tex_env()
{
	GL_CHECK_FOR_ERRORS("start of set_modulate_tex_env()");


	GL_CHECK_FOR_ERRORS("end of set_modulate_tex_env()");
}

GLfloat opengl_get_max_anisotropy()
{
	if ( !GLAD_GL_EXT_texture_filter_anisotropic ) {
		return 0.0f;
	}

	if ( GL_max_anisotropy < 0.0f ) {
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &GL_max_anisotropy);
	}

	// the spec says that it should be a minimum of 2.0
	Assert( GL_max_anisotropy >= 2.0f );

	return GL_max_anisotropy;
}

void opengl_set_texture_target( GLenum target )
{
    GL_texture_target = target;
}

void opengl_tcache_init()
{
	int i;

	GL_should_preload = 1;

	GL_min_texture_width = 16;
	GL_min_texture_height = 16;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &GL_max_texture_width);

	GL_max_texture_height = GL_max_texture_width;

	// if we are not using sections then make sure we have the min texture size available to us
	// 1024 is what we need with the standard resolutions - taylor
	if (GL_max_texture_width < 1024) {
		Error(LOCATION, "A minimum texture size of \"1024x1024\" is required for FS2_Open but only \"%ix%i\" was found.  Can not continue.", GL_max_texture_width, GL_max_texture_height);
	}

	if (Textures == NULL) {
		Textures = (tcache_slot_opengl *) vm_malloc(MAX_BITMAPS * sizeof(tcache_slot_opengl), memory::quiet_alloc);
	}

	if ( !Textures )
		Error(LOCATION, "Unable to allocate memory for OpenGL texture slots!");

	// Init the texture structures
	for (i = 0; i < MAX_BITMAPS; i++) {
		Textures[i].reset();
	}

	// check what mipmap filter we should be using
	//   0  ==  Bilinear
	//   1  ==  Trilinear
	GL_mipmap_filter = os_config_read_uint(NULL, "TextureFilter", 1);

	if (GL_mipmap_filter > 1) {
		GL_mipmap_filter = 1;
	}

	// max size (width and/or height) that we can use for framebuffer/renderbuffer
	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &GL_max_renderbuffer_size);

	// if we can't do at least 128x128 then just disable FBOs
	if (GL_max_renderbuffer_size < 128) {
		mprintf(("WARNING: Max dimensions of FBO, %ix%i, is less the required minimum!!  Extension will be disabled!\n", GL_max_renderbuffer_size, GL_max_renderbuffer_size));
		Cmdline_no_fbo = 1;
	}

	// anisotropy
	if ( GLAD_GL_EXT_texture_filter_anisotropic ) {
		// set max value first thing
		opengl_get_max_anisotropy();

		// now for the user setting
		const char *plevel = os_config_read_string( NULL, NOX("OGL_AnisotropicFilter"), NOX("1.0") );
		GL_anisotropy = (GLfloat) strtod(plevel, (char**)NULL);

		CLAMP(GL_anisotropy, 1.0f, GL_max_anisotropy);
	}

	Assert( GL_supported_texture_units >= 2 );

	GL_last_detail = Detail.hardware_textures;

	GL_textures_in_frame = 0;

	vm_matrix4_set_identity(&GL_texture_matrix);
}

void opengl_tcache_flush()
{
	int i;

	if ( Textures == NULL )
		return;

	for (i = 0; i < MAX_BITMAPS; i++)
		opengl_free_texture( &Textures[i] );
}

extern void opengl_kill_all_render_targets();

void opengl_tex_array_storage(GLenum target, GLint levels, GLenum format, GLint width, GLint height, GLint frames);
void opengl_tcache_shutdown()
{
	opengl_kill_all_render_targets();

	opengl_tcache_flush();

	GL_textures_in_frame = 0;

	if (Textures != NULL) {
		vm_free(Textures);
		Textures = NULL;
	}
}

void opengl_tcache_frame()
{
	GL_textures_in_frame = 0;
}

extern bool GL_initted;

void opengl_free_texture_slot( int n )
{
	if ( !GL_initted ) {
		return;
	}

	opengl_free_texture( &Textures[n] );
}

/**
 * Determine if a bitmap is in API memory, so that we can just reuse it rather
 * that having to load it from disk again
 */
bool opengl_texture_slot_valid(int n, int handle)
{
	tcache_slot_opengl *t = &Textures[n];

	if (t->bitmap_handle < 0) {
		return false;
	}

	if (t->bitmap_handle != handle) {
		return false;
	}

	if ( !glIsTexture(t->texture_id) ) {
		return false;
	}

	return true;
}

int opengl_free_texture(tcache_slot_opengl *t)
{
	if (!t->texture_id) {
		// No texture here anymore
		return 1;
	}
	Assertion(t->used, "Tried to free unused texture slot. This shouldn't happen!");

	// First mark this as unused and then check if all frames are unused. If that's the case we can free the texture array
	t->used = false;

	// Check if the bitmap handle is valid
	if (t->bitmap_handle >= 0) {
		int num_frames = 0;
		auto animation_begin = bm_get_base_frame(t->bitmap_handle, &num_frames);

		if (animation_begin < 0) {
			// Some kind of error
			return 0;
		}

		// Get the index of the first slot and then check every slot of the animation
		auto begin_slot_idx = bm_get_cache_slot(animation_begin, 0);
		auto end_slot_idx = begin_slot_idx + num_frames - 1; // Slots are contiguous for the same animation. Since num_frames is 1-base we need to substract 1

		bool something_in_use = false; // Will be used to check if there is still a frame left in use
		for (int slot_idx = begin_slot_idx; slot_idx <= end_slot_idx; ++slot_idx) {
			auto current_slot = &Textures[slot_idx];
			something_in_use = something_in_use || current_slot->used; // We use or here since only one slot needs to be in use

			if (something_in_use) {
				// If this is true then we can break early since we already determined that one frame is in use
				break;
			}
		}

		if (something_in_use) {
			// There are still used frames left in the animation so we can't free the texture object here

			// We still need to reset this slot though since it has been "freed"
			t->reset();

			// Everything is fine
			return 1;
		}

		// No frame is used anymore so now we can free the texture
	} else {
		// If we are here then the texture slot must contain an FBO texture
		Assertion(t->fbo_id >= 0, "Found texture slot with invalid bitmap number which wasn't an FBO!");
	}

	GR_DEBUG_SCOPE("Delete texture");

	// ok, now we know its legal to free everything safely
	GL_state.Texture.Delete(t->texture_id);
	glDeleteTextures (1, &t->texture_id);

	// Finally, reset the data of this slot
	t->reset();

	return 1;
}

// data == start of bitmap data
// bmap_w == width of source bitmap
// bmap_h == height of source bitmap
// tex_w == width of final texture
// tex_h == height of final texture
int opengl_texture_set_level(int bitmap_handle,
							 int bitmap_type,
							 int bmap_w,
							 int bmap_h,
							 int tex_w,
							 int tex_h,
							 ubyte* data,
							 tcache_slot_opengl* tSlot,
							 int base_level,
							 bool resize,
							 GLenum intFormat)
{
	GR_DEBUG_SCOPE("Upload single frame");

	int ret_val = 1;
	ubyte *bmp_data = data;
	ubyte *texmem = NULL, *texmemp = NULL;
	int skip_size = 0, mipmap_levels = 0;

	GL_CHECK_FOR_ERRORS("start of create_texture_sub()");

	// bogus
	if ( (tSlot == NULL) || (bmp_data == NULL) ) {
		return 0;
	}

	// for everything that might use mipmaps
	auto mipmap_w = tex_w;
	auto mipmap_h = tex_h;

	if ( (bitmap_type == TCACHE_TYPE_AABITMAP) || (bitmap_type == TCACHE_TYPE_INTERFACE) ) {
		tSlot->u_scale = (float)bmap_w / (float)tex_w;
		tSlot->v_scale = (float)bmap_h / (float)tex_h;
	} else {
		tSlot->u_scale = 1.0f;
		tSlot->v_scale = 1.0f;
	}

	// set the byte per pixel multiplier
	auto byte_mult = (tSlot->bpp >> 3);

	GLenum texFormat = GL_UNSIGNED_SHORT_1_5_5_5_REV;
	GLenum glFormat = GL_BGRA;
	// GL_BGRA_EXT is *much* faster with some hardware/drivers
	if (byte_mult == 4) {
		texFormat = GL_UNSIGNED_INT_8_8_8_8_REV;
		intFormat = (gr_screen.bits_per_pixel == 32) ? GL_RGBA8 : GL_RGB5_A1;
		glFormat = GL_BGRA;
	} else if (byte_mult == 3) {
		texFormat = GL_UNSIGNED_BYTE;
		intFormat = (gr_screen.bits_per_pixel == 32) ? GL_RGB8 : GL_RGB5;
		glFormat = GL_BGR;
	} else if (byte_mult == 2) {
		texFormat = GL_UNSIGNED_SHORT_1_5_5_5_REV;
		intFormat = GL_RGB5_A1;
		glFormat = GL_BGRA;
	} else if (byte_mult == 1) {
		Assertion( bitmap_type == TCACHE_TYPE_AABITMAP, "Invalid type for bitmap: %s BMPMAN handle: %d. Type expected was 0, we got %d instead.\nThis can be caused by using texture compression on a non-power-of-2 texture.\n", bm_get_filename(bitmap_handle), bitmap_handle, bitmap_type );
		texFormat = GL_UNSIGNED_BYTE;
		intFormat = GL_RED;
		glFormat = GL_RED;
	} else {
		texFormat = GL_UNSIGNED_SHORT_1_5_5_5_REV;
		intFormat = GL_RGBA;
		glFormat = GL_BGRA;
	}

	// check for compressed image types
	auto block_size = 0;
	switch ( bm_is_compressed(bitmap_handle) ) {
		case DDS_DXT1:
		case DDS_CUBEMAP_DXT1:
			intFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
			block_size = 8;
			break;

		case DDS_DXT3:
		case DDS_CUBEMAP_DXT3:
			intFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			block_size = 16;
			break;

		case DDS_DXT5:
		case DDS_CUBEMAP_DXT5:
			intFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			block_size = 16;
			break;
	}


	if ( bitmap_type == TCACHE_TYPE_CUBEMAP ) {
		tSlot->texture_target = GL_TEXTURE_CUBE_MAP;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	mipmap_levels = bm_get_num_mipmaps(bitmap_handle);

	auto dsize = 0;
	auto doffset = 0;

	switch (bitmap_type) {
		case TCACHE_TYPE_COMPRESSED: {
			if (block_size > 0) {
				// size of data block (4x4)
				dsize = ((mipmap_h + 3) / 4) * ((mipmap_w + 3) / 4) * block_size;

				// if we are skipping mipmap levels in order to resize then we have to calc the new offset
				doffset = 0;
				for (auto i = 0; i < base_level; i++) {
					doffset += dsize;

					mipmap_w >>= 1;
					mipmap_h >>= 1;

					if (mipmap_w <= 0) {
						mipmap_w = 1;
					}

					if (mipmap_h <= 0) {
						mipmap_h = 1;
					}

					dsize = ((mipmap_h + 3) / 4) * ((mipmap_w + 3) / 4) * block_size;
				}

				skip_size = doffset;

				glCompressedTexSubImage3D(tSlot->texture_target, 0, 0, 0, tSlot->array_index, mipmap_w, mipmap_h, 1, intFormat, dsize, bmp_data + doffset);

				// now that the base image is done handle any mipmap levels
				for (auto i = 1; i < (mipmap_levels - base_level); i++) {
					// adjust the data offset for the next block
					doffset += dsize;

					// reduce size by half for the next pass
					mipmap_w >>= 1;
					mipmap_h >>= 1;

					if (mipmap_w <= 0) {
						mipmap_w = 1;
					}

					if (mipmap_h <= 0) {
						mipmap_h = 1;
					}

					// size of data block (4x4)
					dsize = ((mipmap_h + 3) / 4) * ((mipmap_w + 3) / 4) * block_size;

					glCompressedTexSubImage3D(tSlot->texture_target, i, 0, 0, tSlot->array_index, mipmap_w, mipmap_h, 1, intFormat, dsize, bmp_data + doffset);
				}
			} else {
				Int3();
				return 0;
			}

			break;
		}

		case TCACHE_TYPE_AABITMAP: {
			texmem = (ubyte *) vm_malloc (tex_w*tex_h*byte_mult);
			texmemp = texmem;

			Assert( texmem != NULL );

			int luminance = 0;
			for (auto i = 0; i < tex_h; i++) {
				for (auto j = 0; j < tex_w; j++) {
					if ( (i < bmap_h) && (j < bmap_w) ) {
						if ( byte_mult > 1 ) {
							luminance = 0;

							if ( byte_mult > 3 ) {
								for (auto k = 0; k < 3; k++) {
									luminance += bmp_data[(i*bmap_w+j)*byte_mult+k];
								}

								*texmemp++ = (ubyte)((luminance / 3) * (bmp_data[(i*bmap_w+j)*byte_mult+3]/255.0f));
							} else {
								for (auto k = 0; k < byte_mult; k++) {
									luminance += bmp_data[(i*bmap_w+j)*byte_mult+k];
								}

								*texmemp++ = (ubyte)(luminance / byte_mult);
							}
						} else {
							*texmemp++ = bmp_data[i*bmap_w+j];
						}
					} else {
						*texmemp++ = 0;
					}
				}
			}

			GLenum aa_format = GL_RED;

			glTexSubImage3D (tSlot->texture_target, 0, 0, 0, tSlot->array_index, tex_w, tex_h, 1, aa_format, GL_UNSIGNED_BYTE, texmem);

			if (texmem != NULL) {
				vm_free(texmem);
			}

			break;
		}

		case TCACHE_TYPE_INTERFACE: {
			// if we aren't resizing then we can just use bmp_data directly
			if (resize) {
				texmem = (ubyte *) vm_malloc (tex_w*tex_h*byte_mult);
				texmemp = texmem;

				Assert( texmem != NULL );

				for (auto i = 0; i < tex_h; i++) {
					for (auto j = 0;j < tex_w; j++) {
						if ( (i < bmap_h) && (j < bmap_w) ) {
							for (auto k = 0; k < byte_mult; k++) {
								*texmemp++ = bmp_data[(i*bmap_w+j)*byte_mult+k];
							}
						} else {
							for (auto k = 0; k < byte_mult; k++) {
								*texmemp++ = 0;
							}
						}
					}
				}
			}

			glTexSubImage3D (tSlot->texture_target, 0, 0, 0, tSlot->array_index, mipmap_w, mipmap_h, 1, glFormat, texFormat, (resize) ? texmem : bmp_data);

			if (texmem != NULL) {
				vm_free(texmem);
			}

			break;
		}

		case TCACHE_TYPE_CUBEMAP: {
			Assert( !resize );
			Assert( texmem == NULL );

			// we have to load in all 6 faces...
			doffset = 0;
			for (auto i = 0; i < 6; i++) {
				doffset += dsize;

				// check if it's a compressed cubemap first
				if (block_size > 0) {
					// size of data block (4x4)
					dsize = ((mipmap_h + 3) / 4) * ((mipmap_w + 3) / 4) * block_size;

					glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, 0, 0, mipmap_w, mipmap_h, intFormat, dsize, bmp_data + doffset);

					// now that the base image is done handle any mipmap levels
					for (auto j = 1; j < mipmap_levels; j++) {
						// adjust the data offset for the next block
						doffset += dsize;

						// reduce size by half for the next pass
						mipmap_w >>= 1;
						mipmap_h >>= 1;

						if (mipmap_w <= 0) {
							mipmap_w = 1;
						}

						if (mipmap_h <= 0) {
							mipmap_h = 1;
						}

						// size of data block (4x4)
						dsize = ((mipmap_h + 3) / 4) * ((mipmap_w + 3) / 4) * block_size;

						glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, j, 0, 0, mipmap_w, mipmap_h, intFormat, dsize, bmp_data + doffset);
					}
				}
				// nope, it's uncompressed...
				else {
					dsize = mipmap_h * mipmap_w * byte_mult;

					glTexSubImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, 0, 0, mipmap_w, mipmap_h, glFormat, texFormat, bmp_data + doffset);

					// base image is done so now take care of any mipmap levels
					for (auto j = 1; j < mipmap_levels; j++) {
						doffset += dsize;
						mipmap_w >>= 1;
						mipmap_h >>= 1;

						if (mipmap_w <= 0) {
							mipmap_w = 1;
						}

						if (mipmap_h <= 0) {
							mipmap_h = 1;
						}

						dsize = mipmap_h * mipmap_w * byte_mult;

						glTexSubImage2D (GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, j, 0, 0, mipmap_w, mipmap_h, glFormat, texFormat, bmp_data + doffset);
					}
				}

				// reset width and height for next face
				mipmap_w = tex_w;
				mipmap_h = tex_h;
			}

			break;
		}

		default: {
			// if we aren't resizing then we can just use bmp_data directly
			if (resize) {
				texmem = (ubyte *) vm_malloc (tex_w*tex_h*byte_mult);
				texmemp = texmem;

				Assert( texmem != NULL );

				fix u, utmp, v, du, dv;

				u = v = 0;

				du = ( (bmap_w-1)*F1_0 ) / tex_w;
				dv = ( (bmap_h-1)*F1_0 ) / tex_h;

				for (auto j=0;j<tex_h;j++)
				{
					utmp = u;
					for (auto i=0;i<tex_w;i++)
					{
						for (auto k = 0; k < byte_mult; k++) {
							*texmemp++ = bmp_data[(f2i(v)*bmap_w+f2i(utmp))*byte_mult+k];
						}
						utmp += du;
					}
					v += dv;
				}
			}

			// should never have mipmap levels if we also have to manually resize
			if ( (mipmap_levels > 1) && resize ) {
				Assert( texmem == NULL );
			}

			dsize = mipmap_h * mipmap_w * byte_mult;

			// if we are skipping mipmap levels in order to resize then we have to calc the new offset
			doffset = 0;
			for (auto i = 0; i < base_level; i++) {
				doffset += dsize;

				mipmap_w >>= 1;
				mipmap_h >>= 1;

				if (mipmap_w <= 0) {
					mipmap_w = 1;
				}

				if (mipmap_h <= 0) {
					mipmap_h = 1;
				}

				dsize = mipmap_h * mipmap_w * byte_mult;
			}

			skip_size = doffset;

			glTexSubImage3D (tSlot->texture_target, 0, 0, 0, tSlot->array_index, mipmap_w, mipmap_h, 1, glFormat, texFormat, (texmem != NULL) ? texmem : bmp_data + doffset);

			// base image is done so now take care of any mipmap levels
			for (auto i = 1; i < (mipmap_levels - base_level); i++) {
				doffset += dsize;
				mipmap_w >>= 1;
				mipmap_h >>= 1;

				if (mipmap_w <= 0) {
					mipmap_w = 1;
				}

				if (mipmap_h <= 0) {
					mipmap_h = 1;
				}

				dsize = mipmap_h * mipmap_w * byte_mult;

				glTexSubImage3D (tSlot->texture_target, i, 0, 0, tSlot->array_index, mipmap_w, mipmap_h, 1, glFormat, texFormat, bmp_data + doffset);
			}

			if (texmem != NULL) {
				vm_free(texmem);
			}

			break;
		}
	}//end switch


	tSlot->bitmap_handle = bitmap_handle;
	tSlot->size = (dsize) ? ((doffset + dsize) - skip_size) : (tex_w * tex_h * byte_mult);
	tSlot->w = (ushort)tex_w;
	tSlot->h = (ushort)tex_h;

	GL_textures_in_frame += tSlot->size;

	GL_CHECK_FOR_ERRORS("end of create_texture_sub()");

	return ret_val;
}

static GLenum opengl_get_internal_format(int handle, int bitmap_type, int bpp) {

	// set the byte per pixel multiplier
	auto byte_mult = (bpp >> 3);

	// check for compressed image types
	switch ( bm_is_compressed(handle) ) {
		case DDS_DXT1:
		case DDS_CUBEMAP_DXT1:
			return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;

		case DDS_DXT3:
		case DDS_CUBEMAP_DXT3:
			return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			break;

		case DDS_DXT5:
		case DDS_CUBEMAP_DXT5:
			return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		default:
			// Not compressed
			break;
	}

	if (byte_mult == 4) {
		return (gr_screen.bits_per_pixel == 32) ? GL_RGBA8 : GL_RGB5_A1;
	} else if (byte_mult == 3) {
		return (gr_screen.bits_per_pixel == 32) ? GL_RGB8 : GL_RGB5;
	} else if (byte_mult == 2) {
		return GL_RGB5_A1;
	} else if (byte_mult == 1) {
		Assertion( bitmap_type == TCACHE_TYPE_AABITMAP, "Invalid type for bitmap: %s BMPMAN handle: %d. Type expected was 0, we got %d instead.\nThis can be caused by using texture compression on a non-power-of-2 texture.\n", bm_get_filename(handle), handle, bitmap_type );
		return GL_R8;
	} else {
		return GL_RGBA8;
	}
}

void opengl_determine_bpp_and_flags(int bitmap_handle, int bitmap_type, ubyte& flags, int& bpp) {
	flags = 0;
	bpp = 16;
	switch (bitmap_type) {
		case TCACHE_TYPE_AABITMAP:
			flags |= BMP_AABITMAP;
			bpp = 8;
			break;

		case TCACHE_TYPE_CUBEMAP:
		case TCACHE_TYPE_NORMAL:
			flags |= BMP_TEX_OTHER;
			if (bm_has_alpha_channel(bitmap_handle)) {
				bpp = 32; // Since this bitmap has an alpha channel we need to respect that
			} else {
				bpp = 24; // RGB, 8-bits per channel
			}
			break;

		case TCACHE_TYPE_INTERFACE:
		case TCACHE_TYPE_XPARENT:
			flags |= BMP_TEX_XPARENT;
			bpp = 32; // RGBA, 8-bits per channel
			break;

		case TCACHE_TYPE_COMPRESSED:
			switch ( bm_is_compressed(bitmap_handle) ) {
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

				case DDS_CUBEMAP_DXT1:
					bpp = 24;
					flags |= BMP_TEX_CUBEMAP;
					break;

				case DDS_CUBEMAP_DXT3:
				case DDS_CUBEMAP_DXT5:
					bpp = 32;
					flags |= BMP_TEX_CUBEMAP;
					break;

				default:
					Assert( 0 );
					break;
			}

			break;
	}
}

void opengl_tex_array_storage(GLenum target, GLint levels, GLenum format, GLint width, GLint height, GLint frames) {
	if (target == GL_TEXTURE_CUBE_MAP) {
		Assertion(frames == 1, "Cube map texture arrays aren't supported yet!");

		if (GL_version >= 42) {
			// This version has a better way of specifying the texture storage
			glTexStorage2D(target, levels, format, width, height);
		} else {
			for (auto i = 0; i < 6; ++i) {
				auto mip_width = width;
				auto mip_height = height;
				for (auto j = 0; j < levels; j++) {
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, j, format, mip_width, mip_height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
								 nullptr);
					mip_width = std::max(1, (mip_width / 2));
					mip_height = std::max(1, (mip_height / 2));
				}
			}
		}
	} else {
		if (GL_version >= 42) {
			// This version has a better way of specifying the texture storage
			glTexStorage3D(target, levels, format, width, height, frames);
		} else {
			for (auto i = 0; i < levels; i++) {
				glTexImage3D(target, i, format, width, height, frames, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
				width = std::max(1, (width / 2));
				height = std::max(1, (height / 2));
			}
		}
	}
}

int opengl_create_texture(int bitmap_handle, int bitmap_type, tcache_slot_opengl *tslot)
{
	GR_DEBUG_SCOPE("Create Texture");

	if (tslot == NULL) {
		return 0;
	}

	int num_frames = 0;
	// This takes care of checking if the animation can be put into a texture array
	auto animation_begin = bm_get_base_frame(bitmap_handle, &num_frames);

	if (animation_begin < 0) {
		// Some kind of error
		return 0;
	}

	int width, height;
	bm_get_info(animation_begin, &width, &height, nullptr, nullptr);

	auto array_begin_slot = bm_get_cache_slot(animation_begin, 1);
	if (Textures[array_begin_slot].bitmap_handle != -1) {
		Assertion(Textures[array_begin_slot].bitmap_handle != animation_begin, "opengl_create_texture was called for the same bitmap again!");

		auto start_slot = &Textures[array_begin_slot];

		if (start_slot->texture_id != 0) {
			// Delete the previous texture that was stored in this slot
			GL_state.Texture.Delete(start_slot->texture_id);
			glDeleteTextures(1, &start_slot->texture_id);

			start_slot->reset();
		}
	}

	auto max_levels = bm_get_num_mipmaps(bitmap_handle);

	auto base_level = 0;
	auto resize = false;
	if ( (Detail.hardware_textures < 4) && (bitmap_type != TCACHE_TYPE_AABITMAP) && (bitmap_type != TCACHE_TYPE_INTERFACE)
		&& (bitmap_type != TCACHE_TYPE_CUBEMAP)
		&& ((bitmap_type != TCACHE_TYPE_COMPRESSED) || ((bitmap_type == TCACHE_TYPE_COMPRESSED) && (max_levels > 1))) )
	{
		if (max_levels == 1) {
			// if we are going to cull the size then we need to force a resize
			// Detail.hardware_textures goes from 0 to 4.
			width /= (16 >> Detail.hardware_textures);
			height /= (16 >> Detail.hardware_textures);

			CLAMP(width, GL_min_texture_width, GL_max_texture_width);
			CLAMP(height, GL_min_texture_height, GL_max_texture_height);

			resize = true;
		} else {
			// we have mipmap levels so use those as a resize point (image should already be power-of-2)
			base_level = -(Detail.hardware_textures - 4);
			Assert(base_level >= 0);

			if (base_level >= max_levels) {
				base_level = max_levels - 1;
			}

			Assert( (max_levels - base_level) >= 1 );
		}
	}

	if ( (width < 1) || (height< 1) )       {
		mprintf(("Bitmap %s is too small at %dx%d.\n", bm_get_filename(bitmap_handle), width, height));
		return 0;
	}

	// there should only ever be one mipmap level for interface graphics!!!
	if ( (bitmap_type == TCACHE_TYPE_INTERFACE) && (max_levels > 1) ) {
		max_levels = 1;
	}

	// if we ended up locking a texture that wasn't originally compressed then this should catch it
	if ( bitmap_type != TCACHE_TYPE_CUBEMAP && bm_is_compressed(bitmap_handle) ) {
		bitmap_type = TCACHE_TYPE_COMPRESSED;
	}

	// Create the texture array and bind it
	glGenTextures(1, &tslot->texture_id);
	tslot->texture_target = GL_TEXTURE_2D_ARRAY;
	if ( bitmap_type == TCACHE_TYPE_CUBEMAP ) {
		tslot->texture_target = GL_TEXTURE_CUBE_MAP;
	}

	if (tslot->texture_id == 0) {
		mprintf(("!!OpenGL DEBUG!! t->texture_id == 0\n"));
		tslot->reset();
		return 0;
	}

	// This is very important! All bitmaps are bound as texture arrays even if they only contain a single image. This
	// simplifies shader handling since they only ever work with texture arrays instead of having to introduce another
	// compile time flag for every single sampler
	GL_state.Texture.SetTarget(tslot->texture_target);
	GL_state.Texture.Enable(tslot->texture_id);

	opengl_set_object_label(GL_TEXTURE, tslot->texture_id, bm_get_filename(bitmap_handle));

	GLenum min_filter = GL_LINEAR;

	auto mipmap_levels = (ubyte)(max_levels - base_level);
	if (mipmap_levels > 1) {
		min_filter = (GL_mipmap_filter) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_NEAREST;

		if ( GLAD_GL_EXT_texture_filter_anisotropic ) {
			glTexParameterf(tslot->texture_target, GL_TEXTURE_MAX_ANISOTROPY_EXT, GL_anisotropy);
		}
	}

	glTexParameteri(tslot->texture_target, GL_TEXTURE_MAX_LEVEL, mipmap_levels);
	glTexParameteri(tslot->texture_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(tslot->texture_target, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(tslot->texture_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(tslot->texture_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(tslot->texture_target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	tslot->wrap_mode = GL_CLAMP_TO_EDGE;

	ubyte bitmap_flags;
	int bits_per_pixel;
	opengl_determine_bpp_and_flags(animation_begin, bitmap_type, bitmap_flags, bits_per_pixel);

	auto intFormat = opengl_get_internal_format(bitmap_handle, bitmap_type, bits_per_pixel);
	opengl_tex_array_storage(tslot->texture_target, max_levels, intFormat, width, height, num_frames);

	bool frames_loaded = true;
	for (int frame = animation_begin; frame < animation_begin + num_frames; ++frame) {
#ifndef NDEBUG
        // I'm not sure if these values are consistent across the whole animation but they really should be.
		// This should catch any instances where this assumption isn't right
		ubyte debug_flags = 0;
		int debug_bpp;
		opengl_determine_bpp_and_flags(frame, bitmap_type, debug_flags, debug_bpp);

		Assertion(debug_flags == bitmap_flags, "Bitmap flags mismatch detected! Get a coder to take a look at this bitmap: %s.", bm_get_filename(frame));
		Assertion(debug_bpp == bits_per_pixel, "Bits per pixel mismatch detected! Get a coder to take a look at this bitmap: %s.", bm_get_filename(frame));
#endif

		// lock the bitmap into the proper format
		auto bmp = bm_lock(frame, bits_per_pixel, bitmap_flags);

		if ( bmp == NULL ) {
			mprintf(("Couldn't lock bitmap %d (%s).\n", frame, bm_get_filename(frame) ));
			continue;
		}

		auto index = frame - animation_begin;
		auto frame_slot_idx = bm_get_cache_slot(frame, 1);

		auto frame_slot = &Textures[frame_slot_idx];
		*frame_slot = *tslot; // Copy the existing properties to the slot of this texture
		frame_slot->array_index = (uint32_t) index;

		// set the bits per pixel
		frame_slot->bpp = bmp->bpp;

		// max number of mipmap levels (NOTE: this is the max number used by the API, not how many true mipmap levels there are in the image!!)
		frame_slot->mipmap_levels = mipmap_levels;
		frame_slot->used = true; // Mark all frames as used
		frame_slot->array_index = (uint32_t) (frame - animation_begin);

		// call the helper
		int ret_val = opengl_texture_set_level(frame,
											   bitmap_type,
											   bmp->w,
											   bmp->h,
											   width,
											   height,
											   (ubyte*) bmp->data,
											   frame_slot,
											   base_level,
											   resize,
											   intFormat);
		frames_loaded = frames_loaded && ret_val;

		// unlock the bitmap
		bm_unlock(frame);
	}

	if ( tslot->mipmap_levels == 1 && bitmap_type == TCACHE_TYPE_CUBEMAP ) {
		Assertion(num_frames == 1, "Cube map arrays are not supported yet!");
		// generate mip maps for cube maps so we can get glossy reflections; necessary for gloss maps and physically-based lighting
		// OGL_EXT_FRAMEBUFFER_OBJECT required to use glGenerateMipmap()
		tslot->mipmap_levels = get_num_mipmap_levels(tslot->w, tslot->h);

		glTexParameteri(tslot->texture_target, GL_TEXTURE_MAX_LEVEL, tslot->mipmap_levels - 1);
		glTexParameteri(tslot->texture_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glGenerateMipmap(tslot->texture_target);
	}

	// if we successfully sent the texture to API memory then go ahead and dump
	// the rest of the bitmap data in system memory.
	// NOTE: this doesn't do anything for user bitmaps (like screen grabs or streamed animations)
	if (frames_loaded) {
		bm_unload_fast(bitmap_handle);
	}

	return frames_loaded;
}

// WARNING:  Needs to match what is in bm_internal.h!!!!!
#define RENDER_TARGET_DYNAMIC	17

int gr_opengl_tcache_set_internal(int bitmap_handle, int bitmap_type, float *u_scale, float *v_scale, uint32_t *array_index, int tex_unit = 0)
{
	int ret_val = 1;

	GR_DEBUG_SCOPE("Activate texture");

	if (GL_last_detail != Detail.hardware_textures) {
		GL_last_detail = Detail.hardware_textures;
		opengl_tcache_flush();
	}

	int n = bm_get_cache_slot (bitmap_handle, 1);
	tcache_slot_opengl *t = &Textures[n];

	if (!bm_is_render_target(bitmap_handle) && t->bitmap_handle < 0)
	{
		GL_state.Texture.SetActiveUnit(tex_unit);

		ret_val = opengl_create_texture( bitmap_handle, bitmap_type, t );
	}

	// everything went ok
	if (ret_val && t->texture_id) {
		*u_scale = t->u_scale;
		*v_scale = t->v_scale;
		*array_index = t->array_index;

		GL_state.Texture.Enable(tex_unit, t->texture_target, t->texture_id);

		if ( (t->wrap_mode != GL_texture_addressing) && (bitmap_type != TCACHE_TYPE_AABITMAP)
			&& (bitmap_type != TCACHE_TYPE_INTERFACE) && (bitmap_type != TCACHE_TYPE_CUBEMAP) )
		{
			// In this case we need to make sure that the texture unit is actually active
			GL_state.Texture.SetActiveUnit(tex_unit);
			glTexParameteri(t->texture_target, GL_TEXTURE_WRAP_S, GL_texture_addressing);
			glTexParameteri(t->texture_target, GL_TEXTURE_WRAP_T, GL_texture_addressing);
			glTexParameteri(t->texture_target, GL_TEXTURE_WRAP_R, GL_texture_addressing);

			t->wrap_mode = GL_texture_addressing;
		}
	}
	// gah
	else {
		mprintf(("Texturing disabled for bitmap %d (%s) due to internal error.\n", bitmap_handle, bm_get_filename(bitmap_handle)));

		return 0;
	}


	return 1;
}

int gr_opengl_tcache_set(int bitmap_handle, int bitmap_type, float *u_scale, float *v_scale, uint32_t *array_index, int stage)
{
	Assertion(u_scale != nullptr, "U scale must be a valid pointer!");
	Assertion(v_scale != nullptr, "V scale must be a valid pointer!");
	Assertion(array_index != nullptr, "Array index must be a valid pointer!");

	int rc = 0;

	if (bitmap_handle <= 0) {
		return 0;
	}

	GL_CHECK_FOR_ERRORS("start of tcache_set()");

	// set special type if it's so, needed to be right later, but cubemaps are special
	if ( !(bitmap_type == TCACHE_TYPE_CUBEMAP) ) {
		int type = bm_get_tcache_type(bitmap_handle);

		if (type != TCACHE_TYPE_NORMAL) {
			bitmap_type = type;
		}
	}

	rc = gr_opengl_tcache_set_internal(bitmap_handle, bitmap_type, u_scale, v_scale, array_index, stage);

	GL_CHECK_FOR_ERRORS("end of tcache_set()");

	return rc;
}

void opengl_preload_init()
{
	if (gr_screen.mode != GR_OPENGL)
		return;

//	opengl_tcache_flush ();
}

int gr_opengl_preload(int bitmap_num, int is_aabitmap)
{
	float u_scale, v_scale;
	int retval;

	Assert( gr_screen.mode == GR_OPENGL );

	if ( !GL_should_preload ) {
		return 0;
	}

	uint32_t array_index;
	retval = gr_opengl_tcache_set(bitmap_num, (is_aabitmap) ? TCACHE_TYPE_AABITMAP : TCACHE_TYPE_NORMAL, &u_scale, &v_scale, &array_index);

	if ( !retval ) {
		mprintf(("Texture upload failed!\n"));
	}

	return retval;
}

static int GL_texture_panning_enabled = 0;
void gr_opengl_set_texture_panning(float u, float v, bool enable)
{
	if (enable) {
		vm_matrix4_set_identity(&GL_texture_matrix);
		GL_texture_matrix.vec.pos.xyzw.x = u;
		GL_texture_matrix.vec.pos.xyzw.y = v;


		GL_texture_panning_enabled = 1;
	} else if (GL_texture_panning_enabled) {
		vm_matrix4_set_identity(&GL_texture_matrix);

	
		GL_texture_panning_enabled = 0;
	}
}

void gr_opengl_set_texture_addressing(int mode)
{
	GL_CHECK_FOR_ERRORS("start of set_texture_addressing()");

	switch (mode) {
		case TMAP_ADDRESS_CLAMP:
			GL_texture_addressing = GL_CLAMP_TO_EDGE;
			break;

		case TMAP_ADDRESS_WRAP:
			GL_texture_addressing = GL_REPEAT;
			break;

		case TMAP_ADDRESS_MIRROR: {
			GL_texture_addressing = GL_MIRRORED_REPEAT;
			break;
		}

		default:
			Int3();
			break;
	}

	GL_CHECK_FOR_ERRORS("end of set_texture_addressing()");
}

int opengl_compress_image( ubyte **compressed_data, ubyte *in_data, int width, int height, int alpha, int num_mipmaps )
{
	Assert( in_data != NULL );

	if ( !Texture_compression_available ) {
		return 0;
	}

	GL_CHECK_FOR_ERRORS("start of compress_image()");

	GLuint tex;
	GLint compressed = GL_FALSE;
	GLint compressed_size = 0;
	ubyte *out_data = NULL;
	GLint testing = 0;
	GLint intFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
	GLenum texFormat = GL_UNSIGNED_BYTE;
	GLenum glFormat = GL_BGR;
	int i;

	if (alpha) {
		intFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		texFormat = GL_UNSIGNED_INT_8_8_8_8_REV;
		glFormat = GL_BGRA;
	}

	glGenTextures(1, &tex);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_TEXTURE_2D);
	GL_state.Texture.Enable(tex);

	// a quick proxy test.  this will tell us if it's possible without wasting a lot of time and resources in the attempt
	glTexImage2D(GL_PROXY_TEXTURE_2D, 0, intFormat, width, height, 0, glFormat, texFormat, in_data);

	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &compressed);
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &testing);

	if (compressed == GL_TRUE) {
		glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &compressed);
	}

	if ( (compressed == GL_FALSE) || (compressed != intFormat) || (testing == 0) ) {
		GL_state.Texture.Delete(tex);
		glDeleteTextures(1, &tex);
		return 0;
	}

	// use best compression quality
	glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);

	// alright, it should work if we are still here, now do it for real
	glTexImage2D(GL_TEXTURE_2D, 0, intFormat, width, height, 0, glFormat, texFormat, in_data);

	// if we got this far then it should have worked, but check anyway
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &compressed);
	Assert( compressed != GL_FALSE );

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &compressed);
	Assert( compressed == intFormat );

	// for each mipmap level we generate go ahead and figure up the total memory required
	for (i = 0; i < num_mipmaps; i++) {
		glGetTexLevelParameteriv(GL_TEXTURE_2D, i, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &testing);
		compressed_size += testing;
	}

	out_data = (ubyte*)vm_malloc(compressed_size * sizeof(ubyte));

	Assert( out_data != NULL );

	memset(out_data, 0, compressed_size * sizeof(ubyte));

	// reset compressed_size and go back through each mipmap level to get both size and the image data itself
	compressed_size = 0;

	for (i = 0; i < num_mipmaps; i++) {
		glGetTexLevelParameteriv(GL_TEXTURE_2D, i, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &testing);
		glGetCompressedTexImage(GL_TEXTURE_2D, i, out_data + compressed_size);
		compressed_size += testing;
	}

	glHint(GL_TEXTURE_COMPRESSION_HINT, GL_DONT_CARE);

	GL_state.Texture.Delete(tex);
	glDeleteTextures(1, &tex);

	// send the data back out
	*compressed_data = out_data;

	GL_CHECK_FOR_ERRORS("end of compress_image()");

	return compressed_size;
}

int opengl_get_texture( GLenum target, GLenum pixel_format, GLenum data_format, int num_mipmaps, int width, int height, int bytes_per_pixel, void* image_data, int offset )
{
	Assertion(target != GL_TEXTURE_2D_ARRAY, "This code does not support texture arrays bitmaps!");

	int m_offset = offset;
	int m_width = width;
	int m_height = height;

	for ( int i = 0; i < num_mipmaps; i++ ) {
		glGetTexImage(target, i, pixel_format, data_format, (ubyte*)image_data + m_offset);

		m_offset += (m_width * m_height * bytes_per_pixel);

		// reduce by half for next mipmap level
		m_width >>= 1;
		m_height >>= 1;

		if (m_width < 1)
			m_width = 1;

		if (m_height < 1)
			m_height = 1;
	}

	return m_offset;
}

void gr_opengl_get_bitmap_from_texture(void* data_out, int bitmap_num)
{
	float u,v;

	uint32_t array_index = 0;
	gr_opengl_tcache_set(bitmap_num, TCACHE_TYPE_NORMAL, &u, &v, &array_index);

	int n = bm_get_cache_slot(bitmap_num, 1);
	tcache_slot_opengl *ts = &Textures[n];
	
	GLenum pixel_format = GL_RGB;
	GLenum data_format = GL_UNSIGNED_BYTE;
	int bytes_per_pixel = 3 * sizeof(ubyte);

	if ( bm_has_alpha_channel(bitmap_num) ) {
		pixel_format = GL_RGBA;
		bytes_per_pixel = 4 * sizeof(ubyte);
	}

	// We can't read a specific layer of the texture so we need to read the entire texture and then memcpy the right part from that...
	int num_frames = 0;
	bm_get_info(bitmap_num, nullptr, nullptr, nullptr, &num_frames);
	if (!bm_is_texture_array(bitmap_num)) {
		num_frames = 1;
	}

	// The size of a single frame in the array
	auto slice_size = ts->w * ts->h * bytes_per_pixel;
	std::unique_ptr<std::uint8_t[]> buffer(new std::uint8_t[num_frames * slice_size]);

	Assertion(ts->texture_target == GL_TEXTURE_2D_ARRAY, "Unexpected texture target encountered!");

	// Copy the entire texture level into the bitmap
	glGetTexImage(ts->texture_target, 0, pixel_format, data_format, buffer.get());

	auto buffer_offset = array_index * slice_size;
	memcpy(data_out, buffer.get() + buffer_offset, slice_size);
}

void gr_opengl_get_texture_scale(int bitmap_handle, float *u_scale, float *v_scale)
{
	int n = bm_get_cache_slot (bitmap_handle, 1);
	tcache_slot_opengl *t = &Textures[n];

	*u_scale = t->u_scale;
	*v_scale = t->v_scale;
}

/**
 * Sends a texture object out to "image_data"
 *
 * Image_data should be memory which is already allocated. Function should only
 * be used for uncompressed 24-bit or 32-bit (distiguished by "alpha" var) images
 *
 * @return 0 on failure
 * @return size of data on success
 */
size_t opengl_export_render_target( int slot, int width, int height, int alpha, int num_mipmaps, ubyte *image_data )
{
	tcache_slot_opengl *ts = &Textures[slot];

	GL_CHECK_FOR_ERRORS("start of export_image()");

	if (!image_data) {
		mprintf(("OpenGL ERROR: Tried to export a texture without a valid export location!\n"));
		return 0;
	}

	if (!ts->texture_target) {
		mprintf(("OpenGL ERROR: Tried to export a texture for which I don't know the texture target!\n"));
		return 0;
	}

	if (ts->mipmap_levels != num_mipmaps) {
		mprintf(("OpenGL ERROR: Number of mipmap levels requested is different from number available!\n"));
		return 0;
	}

	if ( (ts->texture_target != GL_TEXTURE_2D) && (ts->texture_target != GL_TEXTURE_CUBE_MAP) ) {
		mprintf(("OpenGL ERROR: Only 2D textures and cube maps can be exported!\n"));
		return 0;
	}

	if ( (ts->w != width) && (ts->h != height) ) {
		mprintf(("OpenGL ERROR: Passed width and height do not match values for texture!\n"));
		return 0;
	}

	int faces = 1;
	int m_width = width;
	int m_height = height;
	int m_bpp = (alpha) ? 4 : 3;
	int m_offset = 0;
	GLenum target = (ts->texture_target == GL_TEXTURE_CUBE_MAP) ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : GL_TEXTURE_2D;

	if (ts->texture_target == GL_TEXTURE_CUBE_MAP)
		faces = 6;

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(ts->texture_target);
	GL_state.Texture.Enable(ts->texture_id);

	for (int i = 0; i < faces; i++) {
		m_offset = opengl_get_texture(
			target + i, 
			(alpha) ? GL_BGRA : GL_BGR, 
			(alpha) ? GL_UNSIGNED_INT_8_8_8_8_REV : GL_UNSIGNED_BYTE, 
			ts->mipmap_levels, 
			m_width, 
			m_height, 
			m_bpp, 
			image_data, 
			m_offset
		);
	}

	GL_CHECK_FOR_ERRORS("end of export_image()");

	return (size_t)m_offset;
}

void gr_opengl_update_texture(int bitmap_handle, int bpp, const ubyte* data, int width, int height)
{
	GLenum texFormat, glFormat;
	int n = bm_get_cache_slot (bitmap_handle, 1);
	tcache_slot_opengl *t = &Textures[n];
	if(!t->texture_id)
		return;
	int byte_mult = (bpp >> 3);
	int true_byte_mult = (t->bpp >> 3);
	ubyte* texmem = NULL;
	// GL_BGRA_EXT is *much* faster with some hardware/drivers
	if (true_byte_mult == 4) {
		texFormat = GL_UNSIGNED_INT_8_8_8_8_REV;
		glFormat = GL_BGRA;
	} else if (true_byte_mult == 3) {
		texFormat = GL_UNSIGNED_BYTE;
		glFormat = GL_BGR;
	} else {
		texFormat = GL_UNSIGNED_SHORT_1_5_5_5_REV;
		glFormat = GL_BGRA;
	}
	if (byte_mult == 1) {
		texFormat = GL_UNSIGNED_BYTE;
		glFormat = GL_RED;
		texmem = (ubyte *) vm_malloc (width*height*byte_mult);
		ubyte* texmemp = texmem;

		Assert( texmem != NULL );

		int luminance = 0;
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				if ( (i < height) && (j < width) ) {
					if ( true_byte_mult > 1 ) {
						luminance = 0;

						if ( true_byte_mult > 3 ) {
							for (int k = 0; k < 3; k++) {
								luminance += data[(i*width+j)*true_byte_mult+k];
							}

							*texmemp++ = (ubyte)((luminance / 3) * (data[(i*width+j)*true_byte_mult+3]/255.0f));
						} else {
							for (int k = 0; k < true_byte_mult; k++) {
								luminance += data[(i*width+j)*true_byte_mult+k]; 
							}

							*texmemp++ = (ubyte)(luminance / true_byte_mult);
						}
					} else {
						*texmemp++ = data[i*width+j];
					}
				} else {
					*texmemp++ = 0;
				}
			}
		}
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(t->texture_target, t->texture_id);
	glTexSubImage3D(t->texture_target, 0, 0, 0, t->array_index, width, height, 1, glFormat, texFormat, (texmem)?texmem:data);

	if (texmem != NULL)
		vm_free(texmem);
}

// -----------------------------------------------------------------------------
// GL_EXT_framebuffer_object stuff (ie, render-to-texture)
//

struct fbo_t {
	// these first vars should only be modified in opengl_make_render_target()
	GLuint renderbuffer_id;
	GLuint framebuffer_id;
	int width;
	int height;
	// these next 2 should only be modifed in opengl_set_render_target()
	int working_slot;
	int is_static;
	int fbo_id;

	fbo_t() :
		renderbuffer_id(0), framebuffer_id(0), width(0), height(0),
		working_slot(-1), is_static(0), fbo_id(-1)
	{
	}
};

static SCP_vector<fbo_t> RenderTarget;
static fbo_t *render_target = NULL;
static int next_fbo_id = 0;

static fbo_t* opengl_get_fbo(int id) {
	if (id < 0) {
		return nullptr;
	}

	for (auto& target : RenderTarget) {
		if (target.fbo_id == id) {
			return &target;
		}
	}

	return nullptr;
}

static fbo_t* opengl_get_free_fbo() {
	for (auto& target : RenderTarget) {
		if (target.fbo_id < 0) {
			// This slot is free
			return &target;
		}
	}

	RenderTarget.push_back(fbo_t());
	auto& last = RenderTarget.back();
	last.fbo_id = next_fbo_id;
	++next_fbo_id;

	return &RenderTarget.back();
}

static void opengl_free_fbo_slot(int id) {
	auto fbo = opengl_get_fbo(id);

	Assertion(fbo != nullptr, "Invalid id passed to opengl_free_fbo_slot!");

	// Reset this slot using the default constructor
	*fbo = fbo_t();
}

int opengl_check_framebuffer()
{
	GLenum status;

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (status != GL_FRAMEBUFFER_COMPLETE) {
		char err_txt[100] = { 0 };

		switch (status) {
			case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
				strcpy_s(err_txt, "Incomplete attachments!");
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
				strcpy_s(err_txt, "Missing one or more image attachments!");
				break;
				
			case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
				strcpy_s(err_txt, "Draw buffer attachment point is NONE!");
				break;

			case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
				strcpy_s(err_txt, "Read buffer attachment point is NONE!");
				break;

			case GL_FRAMEBUFFER_UNSUPPORTED:
				strcpy_s(err_txt, "Attached images violate current FBO restrictions!");
				break;

			default:
				strcpy_s(err_txt, "Unknown error!\n");
				break;
		}

		mprintf(("Framebuffer ERROR: %s\n", err_txt));

		return (int)status;
	}

	return 0;
}

void opengl_kill_render_target(int slot)
{
	if ( (slot < 0) || (slot >= MAX_BITMAPS) ) {
		Int3();
		return;
	}

	// this will happen when opengl_kill_all_render_targets() gets called first on exit
	if ( RenderTarget.empty() ) {
		return;
	}

	tcache_slot_opengl *ts = &Textures[slot];

	auto fbo = opengl_get_fbo(ts->fbo_id);

	// this may happen when textures are flushed, the w and h will get reset to 0
	if ( fbo == nullptr ) {
		return;
	}

	if (fbo->framebuffer_id) {
		glDeleteFramebuffers(1, &fbo->framebuffer_id);
		fbo->framebuffer_id = 0;
	}

	if (fbo->renderbuffer_id) {
		glDeleteRenderbuffers(1, &fbo->renderbuffer_id);
		fbo->renderbuffer_id = 0;
	}

	opengl_free_fbo_slot(fbo->fbo_id);
}

void opengl_kill_all_render_targets()
{
	for (size_t i = 0; i < RenderTarget.size(); i++) {
		fbo_t *fbo = &RenderTarget[i];

		if (fbo->framebuffer_id) {
			glDeleteFramebuffers(1, &fbo->framebuffer_id);
			fbo->framebuffer_id = 0;
		}

		if (fbo->renderbuffer_id) {
			glDeleteRenderbuffers(1, &fbo->renderbuffer_id);
			fbo->renderbuffer_id = 0;
		}
	}

	RenderTarget.clear();
}

int opengl_set_render_target( int slot, int face, int is_static )
{
	tcache_slot_opengl *ts = NULL;
	fbo_t *fbo = NULL;

	GL_CHECK_FOR_ERRORS("start of set_render_target()");

	if (slot < 0) {
		if ( (render_target != NULL) && (render_target->working_slot >= 0) ) {
			if (Textures[render_target->working_slot].mipmap_levels > 1) {
				gr_opengl_bm_generate_mip_maps(render_target->working_slot);
			}

			if (render_target->is_static && Cmdline_save_render_targets) {
				extern void gr_opengl_bm_save_render_target(int slot);
				gr_opengl_bm_save_render_target(render_target->working_slot);
			}
		}

		GL_state.BindFrameBuffer(0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// done with this render target so lets move on
		render_target = NULL;

		GL_rendering_to_texture = false;

		GL_CHECK_FOR_ERRORS("end of set_render_target(0)");

		return 1;
	}

	ts = &Textures[slot];
	Assert( ts != NULL );

	if (!ts->texture_id) {
		Int3();
		return 0;
	}

	fbo = opengl_get_fbo(ts->fbo_id);

	if (fbo == NULL) {
		mprintf(("Tried to get an OpenGL FBO that didn't exist!\n"));
		return 0;
	}

	if ( !glIsFramebuffer(fbo->framebuffer_id) /*|| !glIsRenderbufferEXT(fbo->renderbuffer_id)*/ ) {
		Int3();
		return 0;
	}

//	glBindRenderbuffer(GL_RENDERBUFFER, fbo->renderbuffer_id);
	GL_state.BindFrameBuffer(fbo->framebuffer_id);

	if (ts->texture_target == GL_TEXTURE_CUBE_MAP) {
		// For cubemaps we can enable one of the six faces for rendering
		Assert( (face >= 0) && (face < 6) );
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, ts->texture_id, 0);
	} else {
		// Check if the face is valid for this case
		Assert( face <= 0 );
	}

//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo->renderbuffer_id);

	fbo->working_slot = slot;
	fbo->is_static = is_static;

	// save current fbo for later use
	render_target = fbo;

	GL_rendering_to_texture = true;

	GL_CHECK_FOR_ERRORS("end of set_render_target()");

	return 1;
}

int opengl_make_render_target( int handle, int slot, int *w, int *h, int *bpp, int *mm_lvl, int flags )
{
	GR_DEBUG_SCOPE("Make OpenGL render target");

	Assert( !GL_rendering_to_texture );

	if (slot < 0) {
		Int3();
		return 0;
	}

	// got to have at least width and height!
	if (!w || !h) {
		Int3();
		return 0;
	}

	// size check
	if (*w > GL_max_renderbuffer_size) {
		*w = GL_max_renderbuffer_size;
	}

	if (*h > GL_max_renderbuffer_size) {
		*h = GL_max_renderbuffer_size;
	}

	tcache_slot_opengl *ts = &Textures[slot];
	// now on to the good parts...

	fbo_t* new_fbo = opengl_get_free_fbo();
	ts->fbo_id = new_fbo->fbo_id;

	if (flags & BMP_FLAG_CUBEMAP) {
		opengl_set_texture_target( GL_TEXTURE_CUBE_MAP );
	} else {
		opengl_set_texture_target( GL_TEXTURE_2D_ARRAY );
	}

	// initialize color texture
	glGenTextures(1, &ts->texture_id);

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(GL_texture_target);
	GL_state.Texture.Enable(ts->texture_id);

	GLint min_filter = GL_LINEAR;

	if (flags & BMP_FLAG_RENDER_TARGET_MIPMAP) {
		min_filter = GL_LINEAR_MIPMAP_LINEAR;
	}

	glTexParameteri(GL_texture_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_texture_target, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(GL_texture_target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_texture_target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (flags & BMP_FLAG_RENDER_TARGET_MIPMAP) {
		extern int get_num_mipmap_levels(int w, int h);
		ts->mipmap_levels = get_num_mipmap_levels(*w, *h);
	} else {
		ts->mipmap_levels = 1;
	}

	// Initialize the texture storage of the framebuffer. Framebuffers always have only a single layer in the texture array
	// This automatically handles cubemaps correctly
	opengl_tex_array_storage(GL_state.Texture.GetTarget(), ts->mipmap_levels, GL_RGBA8, *w, *h, 1);

	if (flags & BMP_FLAG_RENDER_TARGET_MIPMAP) {
		glGenerateMipmap(GL_state.Texture.GetTarget());
	}

	glTexParameteri(GL_texture_target, GL_TEXTURE_MAX_LEVEL, ts->mipmap_levels - 1);

	// render buffer
//	glGenRenderbuffers(1, &new_fbo.renderbuffer_id);
//	glBindRenderbuffer(GL_RENDERBUFFER, new_fbo.renderbuffer_id);
//	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, *w, *h);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// frame buffer
	glGenFramebuffers(1, &new_fbo->framebuffer_id);
	GL_state.BindFrameBuffer(new_fbo->framebuffer_id);

	if (flags & BMP_FLAG_CUBEMAP) {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, ts->texture_id, 0);
	} else {
		// Since we use texture arrays for all bitmaps we use a single image array here
		glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_state.Texture.GetTarget(), ts->texture_id, 0, 0);
	}

//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, new_fbo.renderbuffer_id);

	if ( opengl_check_framebuffer() ) {
		// Oops!!  reset everything and then bail
		mprintf(("OpenGL: Unable to create FBO!\n"));
		auto fbo_id = ts->fbo_id;

		GL_state.BindFrameBuffer(0);

		GL_state.Texture.Delete(ts->texture_id);
		glDeleteTextures(1, &ts->texture_id);
		ts->reset();

		glDeleteFramebuffers(1, &new_fbo->framebuffer_id);

	//	glDeleteRenderbuffersEXT(1, &new_fbo.renderbuffer_id);

		opengl_set_texture_target();
		opengl_free_fbo_slot(fbo_id);

		return 0;
	}

	// save anything that needs saving, cleanup, and then exit
	ts->w = (ushort)*w;
	ts->h = (ushort)*h;
	ts->bpp = 24;
	ts->texture_target = GL_state.Texture.GetTarget();
	ts->bitmap_handle = -1;//handle;
	ts->u_scale = 1.0f;
	ts->v_scale = 1.0f;
	ts->used = true;
	ts->array_index = 0;

	new_fbo->width = ts->w;
	new_fbo->height = ts->h;

	if (bpp) {
		*bpp = ts->bpp;
	}

	if (mm_lvl) {
		*mm_lvl = ts->mipmap_levels;
	}

	// Clear the new Texture to black with alpha 1.0
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	GL_state.BindFrameBuffer(0);

	opengl_set_texture_target();

	mprintf(("OpenGL: Created %ix%i FBO!\n", ts->w, ts->h));

	return 1;
}

/**
 * @fn	GLuint opengl_get_rtt_framebuffer()
 *
 * @brief	Gets the current RTT framebuffer.
 * 
 * Gets the OpenGL framebuffer ID of the currently in use RTT framebuffer.
 * If there is currently none such framebuffer in use then this function returns
 * 0 so it can be used in any place where the framebuffer should be reset to the
 * default drawing surface.
 *
 * @author	m!m
 * @date	14.12.2011
 *
 * @return	The current RTT FBO ID or 0 when not doing RTT.
 */

GLuint opengl_get_rtt_framebuffer()
{
	if (render_target == NULL || render_target->working_slot < 0)
		return 0;
	else
		return render_target->framebuffer_id;
}

void gr_opengl_bm_generate_mip_maps(int slot)
{
	if ( slot < 0 ) {
		Int3();
		return;
	}

	tcache_slot_opengl *ts = NULL;

	ts = &Textures[slot];

	GL_state.Texture.SetActiveUnit(0);
	GL_state.Texture.SetTarget(ts->texture_target);
	GL_state.Texture.Enable(ts->texture_id);

	glGenerateMipmap(ts->texture_target);
}

//
// End of GL_EXT_framebuffer_object stuff
// -----------------------------------------------------------------------------
