/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrOpenGLTexture.h $
 * $Revision: 1.1 $
 * $Date: 2004-05-24 07:25:32 $
 * $Author: taylor $
 *
 * This file contains function and structure definitions
 * that are needed for managing texture mapping
 *
 * $Log: not supported by cvs2svn $
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



#ifndef _GROPENGLTEXTURE_H
#define _GROPENGLTEXTURE_H

#include "globalincs/pstypes.h"


//turns on/off GL_TEXTUREx_ARB
void opengl_switch_arb(int unit, int state);

typedef struct tcache_slot_opengl {
	uint	texture_handle;
	float	u_scale, v_scale;
	int	bitmap_id;
	int	size;
	char	used_this_frame;
	int	time_created;
	ushort	w,h;
	ubyte bpp;

	// sections
	tcache_slot_opengl	*data_sections[MAX_BMAP_SECTIONS_X][MAX_BMAP_SECTIONS_Y];
	tcache_slot_opengl	*parent;
} tcache_slot_opengl;

extern int GL_texture_sections;
extern int GL_texture_ram;
extern int GL_frame_count;
extern int GL_min_texture_width;
extern int GL_max_texture_width;
extern int GL_min_texture_height;
extern int GL_max_texture_height;
extern int GL_square_textures;
extern int GL_textures_in;
extern int GL_textures_in_frame;
extern int GL_last_bitmap_id;
extern int GL_last_detail;
extern int GL_last_bitmap_type;
extern int GL_last_section_x;
extern int GL_last_section_y;
extern int GL_supported_texture_units;
extern int GL_should_preload;


extern int vram_full;

void opengl_tcache_init(int use_sections);
int opengl_free_texture(tcache_slot_opengl *t);
void opengl_free_texture_with_handle(int handle);
void opengl_tcache_flush();
void opengl_tcache_cleanup();
void opengl_tcache_frame();
void opengl_tcache_get_adjusted_texture_size(int w_in, int h_in, int *w_out, int *h_out);
int opengl_create_texture_sub(int bitmap_type, int texture_handle, ushort *data, int sx, int sy, int src_w, int src_h, int bmap_w, int bmap_h, int tex_w, int tex_h, tcache_slot_opengl *t, int reload, int fail_on_full);
int opengl_create_texture (int bitmap_handle, int bitmap_type, tcache_slot_opengl *tslot, int fail_on_full);
int opengl_create_texture_sectioned(int bitmap_handle, int bitmap_type, tcache_slot_opengl *tslot, int sx, int sy, int fail_on_full);
int gr_opengl_tcache_set(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full = 0, int sx = -1, int sy = -1, int force = 0);
void gr_opengl_set_tex_env_scale(float scale);
int gr_opengl_preload(int bitmap_num, int is_aabitmap);
void gr_opengl_preload_init();
void opengl_set_max_anistropy();


#endif	//_GROPENGLTEXTURE_H
