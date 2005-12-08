/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


/*
 * $Logfile: /Freespace2/code/Graphics/GrOpenGLTNL.cpp $
 * $Revision: 1.33 $
 * $Date: 2005-12-08 15:07:57 $
 * $Author: taylor $
 *
 * source for doing the fun TNL stuff
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.32  2005/12/07 05:42:50  taylor
 * partial spec fix, can't mass kill the pointers when they are still needed for the second pass (still something else wrong though)
 * forgot that the extra rangeelement optimization check isn't needed anymore, just look at indices since that's all we're using
 *
 * Revision 1.31  2005/12/06 02:50:41  taylor
 * clean up some init stuff and fix a minor SDL annoyance
 * make debug messages a bit more readable
 * clean up the debug console commands for minimize and anisotropic filter setting
 * make anisotropic filter actually work correctly and have it settable with a reg option
 * give opengl_set_arb() the ability to disable all features on all arbs at once so I don't have to everywhere
 *
 * Revision 1.30  2005/11/30 03:07:54  phreak
 * texturing shouldn't be on when drawing lines.
 *
 * Revision 1.29  2005/11/13 06:44:18  taylor
 * small bit of EFF cleanup
 * add -img2dds support
 * cleanup some D3D stuff (missing a lot since the old code is so unstable I couldn't get it working like I wanted)
 * some minor OGL cleanup and small performance changes
 * converge the various pcx_read_bitmap* functions into one
 * cleanup/rename/remove some cmdline options
 *
 * Revision 1.28  2005/10/16 11:20:43  taylor
 * use unsigned index buffers
 *
 * Revision 1.27  2005/09/05 09:36:41  taylor
 * merge of OSX tree
 * fix OGL fullscreen switch for SDL since the old way only worked under Linux and not OSX or Windows
 * fix OGL version check, it would allow a required major version to be higher if the required minor version was lower than current
 *
 * Revision 1.26  2005/08/29 02:23:04  phreak
 * Get rid of alpha blending when using the htl drawing fuctions for lines and spheres.
 * Right now, gr_opengl_draw_htl_line and gr_opengl_draw_htl_sphere are only used in fred, so
 * it shouldn't change anything critical
 *
 * Revision 1.25  2005/08/08 01:19:50  taylor
 * hopefully fix the problem that was causing geometry issues in just OGL when using VBOs
 *
 * Revision 1.24  2005/06/19 02:37:02  taylor
 * general cleanup, remove some old code
 * speed up gr_opengl_flip() just a tad
 * inverted gamma slider fix that Sticks made to D3D
 * possible fix for ATI green screens
 * move opengl_check_for_errors() out of gropentnl so we can use it everywhere
 * fix logged OGL info from debug builds to be a little more readable
 * if an extension is found but required function is not then fail
 * try to optimize glDrawRangeElements so we are not rendering more than the card is optimized for
 * some 2d matrix usage checks
 *
 * Revision 1.23  2005/05/12 17:43:20  taylor
 * use vm_malloc(), vm_free(), vm_realloc(), vm_strdup() rather than system named macros
 *   fixes various problems and is past time to make the switch
 * try and keep up with how much VBO data is in memory too since that takes away from texture memory
 *
 * Revision 1.22  2005/04/05 05:53:17  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 1.21  2005/03/27 02:59:27  wmcoolmon
 * Forgot to define 'i' outside of for scope...ironically, in the OGL code.
 *
 * Revision 1.20  2005/03/24 23:42:21  taylor
 * s/gr_ogl_/gr_opengl_/g
 * add empty gr_opengl_draw_line_list() so that it's not a NULL pointer
 * make gr_opengl_draw_htl_sphere() just use GLU so we don't need yet another friggin API
 *
 * Revision 1.19  2005/03/20 00:09:07  phreak
 * Added gr_draw_htl_line and gr_draw_htl sphere
 * There still needs to be D3D versions implemented, but OGL is done.
 * Follow that or ask phreak about how its implemented/
 *
 * Revision 1.18  2005/03/19 21:03:54  wmcoolmon
 * OpenGL display lists
 *
 * Revision 1.17  2005/03/19 18:02:34  bobboau
 * added new graphic functions for state blocks
 * also added a class formanageing a new effect
 *
 * Revision 1.16  2005/02/23 05:11:13  taylor
 * more consolidation of various graphics variables
 * some header cleaning
 * only one tmapper_internal for OGL, don't use more than two tex/pass now
 * seperate out 2d matrix mode to allow -2d_poof in OGL and maybe fix missing
 *    interface when set 3d matrix stuff doesn't have corresponding end
 * add dump_frame stuff for OGL, mostly useless but allows trailer recording
 *
 * Revision 1.15  2005/02/15 00:06:27  taylor
 * clean up some model related globals
 * code to disable individual thruster glows
 * fix issue where 1 extra OGL light pass didn't render
 *
 * Revision 1.14  2005/02/12 10:44:10  taylor
 * fix possible crash in bm_get_section_size()
 * get jpeg_read_header() working properly
 * VBO fixes and minor optimizations
 *
 * Revision 1.13  2005/01/13 04:55:57  taylor
 * plug leak from VBO crash fix
 *
 * Revision 1.12  2005/01/03 18:45:22  taylor
 * dynamic allocation of num supported OpenGL lights
 * add config option for more realistic light settings
 * don't render spec maps in nebula to address lighting issue
 *
 * Revision 1.11  2005/01/01 11:24:23  taylor
 * good OpenGL spec mapping
 * fix VBO crash with multitexture using same uv coord data
 * little speedup of opengl_tcache_frame()
 * error message to make sure hardware supports the minimum texture size
 * move OpenGL version check out of the extention printout code
 * disable 2d_poof with OpenGL
 *
 * Revision 1.10  2004/10/31 21:45:13  taylor
 * Linux tree merge, single array for VBOs/HTL
 *
 * Revision 1.9  2004/07/29 09:35:29  taylor
 * fix NULL pointer and try to prevent in future, remove excess commands in opengl_cleanup()
 *
 * Revision 1.8  2004/07/26 20:47:32  Kazan
 * remove MCD complete
 *
 * Revision 1.7  2004/07/17 18:49:57  taylor
 * oops, I can't spell
 *
 * Revision 1.6  2004/07/17 18:40:40  taylor
 * Bob say we fix OGL, so me fix OGL <grunt>
 *
 * Revision 1.5  2004/07/12 16:32:48  Kazan
 * MCD - define _MCD_CHECK to use memory tracking
 *
 * Revision 1.4  2004/07/11 03:22:49  bobboau
 * added the working decal code
 *
 * Revision 1.3  2004/07/05 05:09:19  bobboau
 * FVF code, only the data that is needed is sent off to the card,,
 * OGL can take advantage of this if they want but it won't break
 * anything if they don't. also state block code has been implemented,
 * that's totaly internal to D3D no high level code is involved.
 *
 * Revision 1.2  2004/06/28 02:13:07  bobboau
 * high level index buffer suport and d3d implementation,
 * OGL people need to get this working on your end as it's broke now
 *
 * Revision 1.1  2004/05/24 07:25:32  taylor
 * filename case change
 *
 * Revision 2.4  2004/04/26 13:05:19  taylor
 * respect -glow and -spec
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

#include "graphics/2d.h"
#include "graphics/grinternal.h"
#include "graphics/gropengl.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropengltexture.h"
#include "graphics/gropengllight.h"
#include "graphics/gropengltnl.h"

#include "math/vecmat.h"
#include "render/3d.h"

#include "debugconsole/timerbar.h"



extern int VBO_ENABLED;
extern int GLOWMAP;
extern int CLOAKMAP;
extern int SPECMAP;
extern vec3d G3_user_clip_normal;
extern vec3d G3_user_clip_point;
extern int Interp_multitex_cloakmap;
extern int Interp_cloakmap_alpha;

static int GL_modelview_matrix_depth = 1;
static int GL_htl_projection_matrix_set = 0;
static int GL_htl_view_matrix_set = 0;
static int GL_htl_2d_matrix_depth = 0;
static int GL_htl_2d_matrix_set = 0;

int GL_vertex_data_in = 0;

GLint GL_max_elements_vertices = 4096;
GLint GL_max_elements_indices = 4096;


struct opengl_vertex_buffer
{
	int used;
	int stride;			// the current stride
	GLenum format;		// the format passed to glInterleavedArrays()
	int n_prim;
	int n_verts;
	float *array_list;	// interleaved array
	GLuint vbo;			// buffer for VBO
	uint flags;			// FVF
	int vbo_size;
};


static opengl_vertex_buffer vertex_buffers[MAX_BUFFERS];
static opengl_vertex_buffer *g_vbp;

//zeros everything out
void opengl_init_vertex_buffers()
{
	memset(vertex_buffers,0,sizeof(opengl_vertex_buffer)*MAX_BUFFERS);
}

int opengl_find_first_free_buffer()
{
	for (int i=0; i < MAX_BUFFERS; i++)
	{
		if (!vertex_buffers[i].used)
			return i;
	}
	
	return -1;
}

int opengl_mod_depth()
{
	GLint mv;
	glGetIntegerv(GL_MODELVIEW_STACK_DEPTH, &mv);
	return (int)mv;
}

void gr_opengl_set_buffer(int idx)
{
	if (Cmdline_nohtl)
		return;

	g_vbp = &vertex_buffers[idx];
}

uint opengl_create_vbo(uint size, GLfloat *data)
{
	if (!data)
		return 0;

	if (!*data)
		return 0;

	if (size == 0)
		return 0;


	// Kazan: A) This makes that if (buffer_name) work correctly (false = 0, true = anything not 0)
	//				if glGenBuffersARB() doesn't initialized it for some reason
	//        B) It shuts up MSVC about may be used without been initalized
	GLuint buffer_name=0;

	glGenBuffersARB(1, &buffer_name);
	
	//make sure we have one
	if (buffer_name)
	{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffer_name);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, data, GL_STATIC_DRAW_ARB );

		// just in case
		if (opengl_check_for_errors()) {
			return 0;
		}
	}

	return (int)buffer_name;
}

int gr_opengl_make_buffer(poly_list *list, uint flags)
{
	if (Cmdline_nohtl)
		return -1;

	int buffer_num = -1;

	buffer_num = opengl_find_first_free_buffer();

	// clear out any old errors before we continue with this
	opengl_check_for_errors();

	//we have a valid buffer
	if (buffer_num > -1) {
		int arsize = 0, make_vbo = 0;
		int list_size, i;

		gr_opengl_set_buffer( buffer_num );

		opengl_vertex_buffer *vbp = g_vbp;

		// defaults
		vbp->format = 0;
		vbp->vbo = 0;

		// don't create vbo for small stuff, performance gain
		// FIXME: This little speed increase appears to cause problems with some cards/drivers.
		//        Disabling it until I can find a solution that works better.
		if (VBO_ENABLED /*&& (list->n_verts >= 250)*/)
			make_vbo = 1;

		// setup using flags
		if ( (flags & VERTEX_FLAG_UV1) && (flags & VERTEX_FLAG_NORMAL) && (flags & VERTEX_FLAG_POSITION) ) {
			vbp->stride = (8 * sizeof(float));
			vbp->format = GL_T2F_N3F_V3F;
		} else if ( (flags & VERTEX_FLAG_UV1) && (flags & VERTEX_FLAG_POSITION) ) {
			vbp->stride = (5 * sizeof(float));
			vbp->format = GL_T2F_V3F;
		} else if ( (flags & VERTEX_FLAG_POSITION) ) {
			vbp->stride = (3 * sizeof(float));
			vbp->format = GL_V3F;
		} else {
			Assert( 0 );
		}

		// total size of data
		list_size = vbp->stride * list->n_verts;

		// allocate the storage list
		vbp->array_list = (float*)vm_malloc(list_size);

		// return invalid if we don't have the memory
		if (vbp->array_list == NULL)
			return -1;
		
		memset(vbp->array_list, 0, list_size);

		// generate the array
		for (i=0; i<list->n_verts; i++) {
			vertex *vl = &list->vert[i];
			vec3d *nl = &list->norm[i];

			// don't try to generate more data than what's available
			Assert( int((arsize * sizeof(float)) + vbp->stride) <= list_size );

			// NOTE: TEX->NORM->VERT, This array order *must* be preserved!!

			// tex coords
			if (flags & VERTEX_FLAG_UV1) {
				vbp->array_list[arsize++] = vl->u;
				vbp->array_list[arsize++] = vl->v;
			}

			// normals
			if (flags & VERTEX_FLAG_NORMAL) {
				vbp->array_list[arsize++] = nl->xyz.x;
				vbp->array_list[arsize++] = nl->xyz.y;
				vbp->array_list[arsize++] = nl->xyz.z;
			}

			// verts
			if (flags & VERTEX_FLAG_POSITION) {
				vbp->array_list[arsize++] = vl->x;
				vbp->array_list[arsize++] = vl->y;
				vbp->array_list[arsize++] = vl->z;
			}
		}

		vbp->used = 1;
		vbp->flags = flags;

		vbp->n_prim = (list->n_verts / 3);
		vbp->n_verts = list->n_verts;

		// maybe load it into a vertex buffer object
		if (make_vbo) {
			vbp->vbo = opengl_create_vbo( list_size, vbp->array_list );

			if (vbp->vbo) {
				// figure up the size so we can know how much VBO data is in card memory
				vbp->vbo_size = list_size;
				GL_vertex_data_in += list_size;
				vm_free(vbp->array_list);
				vbp->array_list = NULL;
			}
		}
	}

	return buffer_num;
}
	
void gr_opengl_destroy_buffer(int idx)
{
	if (Cmdline_nohtl)
		return;

	if (idx > -1)
		gr_opengl_set_buffer(idx);

	opengl_vertex_buffer *vbp = g_vbp;

	if (vbp->array_list)
		vm_free(vbp->array_list);

	if (vbp->vbo) {
		glDeleteBuffersARB(1, &vbp->vbo);
		GL_vertex_data_in -= vbp->vbo_size;
	}

	memset(vbp, 0, sizeof(opengl_vertex_buffer));
}

//#define DRAW_DEBUG_LINES
extern float Model_Interp_scale_x,Model_Interp_scale_y,Model_Interp_scale_z;
extern void opengl_default_light_settings(int amb = 1, int emi = 1, int spec = 1);

//start is the first part of the buffer to render, n_prim is the number of primitives, index_list is an index buffer, if index_list == NULL render non-indexed
void gr_opengl_render_buffer(int start, int n_prim, ushort* index_buffer)
{
	if (Cmdline_nohtl)
		return;

	Assert(GL_htl_projection_matrix_set);
	Assert(GL_htl_view_matrix_set);

	TIMERBAR_PUSH(2);

	float u_scale,v_scale;
	int pass_one = 0;
	int i, r, g, b, a, tmap_type;
	bool use_spec = false;

	int end = ((n_prim * 3) - 1);
	int count = (end - start + 1); //(n_prim * 3);
	int start_tmp, end_tmp, count_tmp, multiple_elements = 0;

	opengl_vertex_buffer *vbp = g_vbp;


	// disable all arbs before we start
	opengl_switch_arb(-1, 0);

	// see if we need to optimize glDrawRangeElements
	if ( count > GL_max_elements_indices )
		multiple_elements = 1;

	if ( glIsEnabled(GL_CULL_FACE) )
		glFrontFace(GL_CW);

// -------- Begin 1st PASS ------------------------------------------------------- //
	if (gr_screen.current_bitmap == CLOAKMAP) {
		glBlendFunc(GL_ONE,GL_ONE);
		r = g = b = Interp_cloakmap_alpha;
		a = 255;
	}

	opengl_setup_render_states(r, g, b, a, tmap_type, TMAP_FLAG_TEXTURED, 0);
	glColor4ub( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)a );

	// basic setup of all data and first texture
	glClientActiveTextureARB(GL_TEXTURE0_ARB+pass_one);
	if (vbp->vbo) {
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo);
		glInterleavedArrays(vbp->format, 0, (void*)NULL);
	} else {
		glInterleavedArrays(vbp->format, 0, vbp->array_list);
	}

	// base texture
	gr_opengl_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, pass_one);

	// increment texture count for this pass
	pass_one++; // bump!

	if ( (Interp_multitex_cloakmap > 0) && (vbp->flags & VERTEX_FLAG_UV1) ) {
		SPECMAP = -1;	// don't add a spec map if we are cloaked
		GLOWMAP = -1;	// don't use a glowmap either, shouldn't see them

		glClientActiveTextureARB(GL_TEXTURE0_ARB+pass_one);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if (vbp->vbo) {
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo);
			glTexCoordPointer( 2, GL_FLOAT, vbp->stride, (void*)NULL );
		} else {
			glTexCoordPointer( 2, GL_FLOAT, vbp->stride, vbp->array_list );
		}

		gr_opengl_tcache_set(Interp_multitex_cloakmap, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, pass_one);

		pass_one++; // bump!
	}

	// glowmaps!
	if ( (GLOWMAP > -1) && !Cmdline_noglow && (vbp->flags & VERTEX_FLAG_UV1) ) {
		glClientActiveTextureARB(GL_TEXTURE0_ARB+pass_one);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if (vbp->vbo) {
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo);
			glTexCoordPointer( 2, GL_FLOAT, vbp->stride, (void*)NULL );
		} else {
			glTexCoordPointer( 2, GL_FLOAT, vbp->stride, vbp->array_list );
		}

		// set glowmap on relevant ARB
		gr_opengl_tcache_set(GLOWMAP, tmap_type, &u_scale, &v_scale, 0, gr_screen.current_bitmap_sx, gr_screen.current_bitmap_sy, 0, pass_one);

		opengl_switch_arb(pass_one, 1);
		opengl_set_additive_tex_env();

		pass_one++; // bump!
	}

	opengl_pre_render_init_lights();
	opengl_change_active_lights(0);

	// only change lights if we have a specmap, don't render the specmap when fog is enabled
	if ( (lighting_is_enabled) && !(glIsEnabled(GL_FOG)) && (SPECMAP > -1) && !Cmdline_nospec && (vbp->flags & VERTEX_FLAG_UV1) ) {
		use_spec = true;
		opengl_default_light_settings(1, 1, 0); // don't render with spec lighting here
	} else {
		// reset to defaults
		opengl_default_light_settings();
	}

	glLockArraysEXT( 0, vbp->n_verts);

	if (index_buffer != NULL) {
		if ( multiple_elements ) {
			start_tmp = 0;
			end_tmp = (GL_max_elements_indices - 1);
			count_tmp = (end_tmp - start_tmp + 1);

			glDrawRangeElements(GL_TRIANGLES, start_tmp, end_tmp, count_tmp, GL_UNSIGNED_SHORT, index_buffer + start_tmp);

			while (end_tmp < end) {
				start_tmp += (GL_max_elements_indices - 1);
				end_tmp = MIN( (start_tmp + GL_max_elements_indices - 1), end );
				count_tmp = (end_tmp - start_tmp + 1);

				glDrawRangeElements(GL_TRIANGLES, start_tmp, end_tmp, count_tmp, GL_UNSIGNED_SHORT, index_buffer + start_tmp);
			}
		} else {
			glDrawRangeElements(GL_TRIANGLES, start, end, count, GL_UNSIGNED_SHORT, index_buffer + start);
		}
	} else {
		glDrawArrays(GL_TRIANGLES, 0, vbp->n_verts);
	}

	glUnlockArraysEXT();
// -------- End 1st PASS --------------------------------------------------------- //

// -------- Begin 2nd (specular) PASS -------------------------------------------- //
	if ( use_spec ) {
		// turn all previously used arbs off before the specular pass
		// this fixes the glowmap multitexture rendering problem - taylor
		for (i = 0; i < pass_one; i++) {
			opengl_switch_arb(i, 0);
		}

		glClientActiveTextureARB(GL_TEXTURE0_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if (vbp->vbo) {
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo);
			glTexCoordPointer( 2, GL_FLOAT, vbp->stride, (void*)NULL );
		} else {
			glTexCoordPointer( 2, GL_FLOAT, vbp->stride, vbp->array_list );
		}

		opengl_set_spec_mapping(tmap_type, &u_scale, &v_scale);

		glLockArraysEXT( 0, vbp->n_verts );

		if (index_buffer != NULL) {
			if ( multiple_elements ) {
				start_tmp = 0;
				end_tmp = (GL_max_elements_indices - 1);
				count_tmp = (end_tmp - start_tmp + 1);

				glDrawRangeElements(GL_TRIANGLES, start_tmp, end_tmp, count_tmp, GL_UNSIGNED_SHORT, index_buffer + start_tmp);

				while (end_tmp < end) {
					start_tmp += (GL_max_elements_indices - 1);
					end_tmp = MIN( (start_tmp + GL_max_elements_indices - 1), end );
					count_tmp = (end_tmp - start_tmp + 1);

					glDrawRangeElements(GL_TRIANGLES, start_tmp, end_tmp, count_tmp, GL_UNSIGNED_SHORT, index_buffer + start_tmp);
				}
			} else {
				glDrawRangeElements(GL_TRIANGLES, start, end, count, GL_UNSIGNED_SHORT, index_buffer + start);
			}
		} else {
			glDrawArrays(GL_TRIANGLES, 0, vbp->n_verts);
		}

		glUnlockArraysEXT();

		opengl_reset_spec_mapping();
	}
// -------- End 2nd PASS --------------------------------------------------------- //

	// reset lights to default values
//	opengl_default_light_settings();

	if ( (lighting_is_enabled) && ((n_active_gl_lights-1)/GL_max_lights > 0) ) {
		opengl_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_READ );
		for (i = 1; i < pass_one; i++) {
			opengl_switch_arb(i, 0);
		}

		glLockArraysEXT( 0, vbp->n_verts);

		for(i=0; i< (n_active_gl_lights-1)/GL_max_lights; i++)
		{
			opengl_change_active_lights(i+1);

			if (index_buffer != NULL) {
				if ( multiple_elements ) {
					start_tmp = 0;
					end_tmp = (GL_max_elements_indices - 1);
					count_tmp = (end_tmp - start_tmp + 1);

					glDrawRangeElements(GL_TRIANGLES, start_tmp, end_tmp, count_tmp, GL_UNSIGNED_SHORT, index_buffer + start_tmp);

					while (end_tmp < end) {
						start_tmp += (GL_max_elements_indices - 1);
						end_tmp = MIN( (start_tmp + GL_max_elements_indices - 1), end );
						count_tmp = (end_tmp - start_tmp + 1);

						glDrawRangeElements(GL_TRIANGLES, start_tmp, end_tmp, count_tmp, GL_UNSIGNED_SHORT, index_buffer + start_tmp);
					}
				} else {
					glDrawRangeElements(GL_TRIANGLES, start, end, count, GL_UNSIGNED_SHORT, index_buffer + start);
				}
			} else {
				glDrawArrays(GL_TRIANGLES, 0, vbp->n_verts);
			}
		}

		glUnlockArraysEXT();
	}

	TIMERBAR_POP();

	// make sure everthing gets turned back off, fixes hud issue with spec lighting and VBO crash in starfield
	opengl_switch_arb(-1, 0);


#if defined(DRAW_DEBUG_LINES) && defined(_DEBUG)
	glBegin(GL_LINES);
		glColor3ub(255,0,0);
		glVertex3d(0,0,0);
		glVertex3d(20,0,0);

		glColor3ub(0,255,0);
		glVertex3d(0,0,0);
		glVertex3d(0,20,0);

		glColor3ub(0,0,255);
		glVertex3d(0,0,0);
		glVertex3d(0,0,20);
	glEnd();
#endif
}

void gr_opengl_start_instance_matrix(vec3d *offset, matrix* rotation)
{
	if (Cmdline_nohtl)
		return;

	Assert(GL_htl_projection_matrix_set);
	Assert(GL_htl_view_matrix_set);

	if (!offset)
		offset = &vmd_zero_vector;
	if (!rotation)
		rotation = &vmd_identity_matrix;	

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	vec3d axis;
	float ang;
	vm_matrix_to_rot_axis_and_angle(rotation,&ang,&axis);
	glTranslatef(offset->xyz.x,offset->xyz.y,offset->xyz.z);
	glRotatef(fl_degrees(ang),axis.xyz.x,axis.xyz.y,axis.xyz.z);
	GL_modelview_matrix_depth++;
}

void gr_opengl_start_instance_angles(vec3d *pos, angles* rotation)
{
	if (Cmdline_nohtl)
		return;

	Assert(GL_htl_projection_matrix_set);
	Assert(GL_htl_view_matrix_set);

	matrix m;
	vm_angles_2_matrix(&m,rotation);
	gr_opengl_start_instance_matrix(pos,&m);
}

void gr_opengl_end_instance_matrix()
{
	if (Cmdline_nohtl)
		return;

	Assert(GL_htl_projection_matrix_set);
	Assert(GL_htl_view_matrix_set);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	GL_modelview_matrix_depth--;
}

//the projection matrix; fov, aspect ratio, near, far
void gr_opengl_set_projection_matrix(float fov, float aspect, float z_near, float z_far)
{
	if (Cmdline_nohtl)
		return;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(fl_degrees(fov),aspect,z_near,z_far);
	glMatrixMode(GL_MODELVIEW);
	GL_htl_projection_matrix_set = 1;
}

void gr_opengl_end_projection_matrix()
{
	if (Cmdline_nohtl)
		return;

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	GL_htl_projection_matrix_set = 0;
}

void gr_opengl_set_view_matrix(vec3d *pos, matrix* orient)
{
	if (Cmdline_nohtl)
		return;

	Assert(GL_htl_projection_matrix_set);
	Assert(GL_modelview_matrix_depth == 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushMatrix();
	
	vec3d fwd;
	vec3d *uvec=&orient->vec.uvec;

	vm_vec_add(&fwd, pos, &orient->vec.fvec);

	gluLookAt(pos->xyz.x,pos->xyz.y,-pos->xyz.z,
	fwd.xyz.x,fwd.xyz.y,-fwd.xyz.z,
	uvec->xyz.x, uvec->xyz.y,-uvec->xyz.z);

	glScalef(1,1,-1);
	glViewport(gr_screen.offset_x,gr_screen.max_h-gr_screen.offset_y-gr_screen.clip_height,gr_screen.clip_width,gr_screen.clip_height);

	GL_modelview_matrix_depth = 2;
	GL_htl_view_matrix_set = 1;	
}

void gr_opengl_end_view_matrix()
{
	if (Cmdline_nohtl)
		return;

	Assert(GL_modelview_matrix_depth == 2);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glLoadIdentity();
	glViewport(0,0,gr_screen.max_w, gr_screen.max_h);

	GL_modelview_matrix_depth = 1;
	GL_htl_view_matrix_set = 0;
}

// set a view and projection matrix for a 2D element
// TODO: this probably needs to accept values
void gr_opengl_set_2d_matrix(/*int x, int y, int w, int h*/)
{
	if (Cmdline_nohtl)
		return;

	// don't bother with this if we aren't even going to need it
	if (!GL_htl_projection_matrix_set)
		return;

	Assert( GL_htl_2d_matrix_set == 0 );
	Assert( GL_htl_2d_matrix_depth == 0 );

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();

	// the top and bottom positions are reversed on purpose
	glOrtho( 0, gr_screen.max_w, gr_screen.max_h, 0, 0, 1 );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	// the viewport needs to be the full screen size since glOrtho() is relative to it
	glViewport(0, 0, gr_screen.max_w, gr_screen.max_h);

	GL_htl_2d_matrix_set++;
	GL_htl_2d_matrix_depth++;
}

// ends a previously set 2d view and projection matrix
void gr_opengl_end_2d_matrix()
{
	if (Cmdline_nohtl)
		return;

	if (!GL_htl_2d_matrix_set)
		return;

	Assert( GL_htl_2d_matrix_depth == 1 );

	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	glMatrixMode( GL_MODELVIEW );
	glPopMatrix();

	GL_htl_2d_matrix_set = 0;
	GL_htl_2d_matrix_depth = 0;
}

void gr_opengl_push_scale_matrix(vec3d *scale_factor)
{
	if (Cmdline_nohtl)
		return;

	Assert(GL_htl_projection_matrix_set);
	Assert(GL_htl_view_matrix_set);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	GL_modelview_matrix_depth++;
	glScalef(scale_factor->xyz.x,scale_factor->xyz.y,scale_factor->xyz.z);
}

void gr_opengl_pop_scale_matrix()
{
	if (Cmdline_nohtl)
		return;

	Assert(GL_htl_projection_matrix_set);
	Assert(GL_htl_view_matrix_set);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	GL_modelview_matrix_depth--;
}

void gr_opengl_end_clip_plane()
{
	if (Cmdline_nohtl)
		return;

	glDisable(GL_CLIP_PLANE0);
}

void gr_opengl_start_clip_plane()
{
	if (Cmdline_nohtl)
		return;

	double clip_equation[4];
	vec3d n;
	vec3d p;

	n=G3_user_clip_normal;	
	p=G3_user_clip_point;

	clip_equation[0]=n.xyz.x;
	clip_equation[1]=n.xyz.y;
	clip_equation[2]=n.xyz.z;
	clip_equation[3]=-vm_vec_dot(&p, &n);
	glClipPlane(GL_CLIP_PLANE0, clip_equation);
	glEnable(GL_CLIP_PLANE0);
}

//************************************State blocks************************************

//this is an array of reference counts for state block IDs
static GLuint *state_blocks = NULL;
static uint n_state_blocks = 0;
static GLuint current_state_block;

//this is used for places in the array that a state block ID no longer exists
#define EMPTY_STATE_BOX_REF_COUNT	0xffffffff

int opengl_get_new_state_block_internal()
{
	uint i;

	if (state_blocks == NULL) {
		state_blocks = (GLuint*)vm_malloc(sizeof(GLuint));
		memset(&state_blocks[n_state_blocks], 'f', sizeof(GLuint));
		n_state_blocks++;
	}

	for (i = 0; i < n_state_blocks; i++) {
		if (state_blocks[i] == EMPTY_STATE_BOX_REF_COUNT) {
			return i;
		}
	}

	// "i" should be n_state_blocks since we got here.
	state_blocks = (GLuint*)vm_realloc(state_blocks, sizeof(GLuint) * i);
	memset(&state_blocks[i], 'f', sizeof(GLuint));

	n_state_blocks++;

	return n_state_blocks-1;
}

void gr_opengl_start_state_block()
{
	gr_screen.recording_state_block = true;
	current_state_block = opengl_get_new_state_block_internal();
	glNewList(current_state_block, GL_COMPILE);
}

int gr_opengl_end_state_block()
{
	//sanity check
	if(!gr_screen.recording_state_block)
		return -1;

	//End the display list
	gr_screen.recording_state_block = false;
	glEndList();

	//now return
	return current_state_block;
}

void gr_opengl_set_state_block(int handle)
{
	if(handle < 0) return;
	glCallList(handle);
}


void gr_opengl_draw_htl_line(vec3d *start, vec3d* end)
{
	if (Cmdline_nohtl)
		return;

	opengl_set_state(TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_FULL);
	glBegin(GL_LINES);
		glColor3ub(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue);
		glSecondaryColor3ubvEXT(GL_zero_3ub);
		glVertex3fv(start->a1d);
		glVertex3fv(end->a1d);
	glEnd();
}

void gr_opengl_draw_htl_sphere(float rad)
{
	if (Cmdline_nohtl)
		return;

	GLUquadricObj *quad = NULL;

	// FIXME: before this is used in anything other than FRED2 we need to make this creation/deletion 
	// stuff global so that it's not so slow (it can be reused for multiple quadratic objects)
	quad = gluNewQuadric();

	Assert(quad != NULL);

	if (quad == NULL)
		return;

	opengl_set_state(TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_FULL);
	glColor3ub(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue);
	glSecondaryColor3ubvEXT(GL_zero_3ub);

	// FIXME: opengl_check_for_errors() needs to be modified to work with this at
	// some point but for now I just don't care so it does nothing
	gluQuadricCallback( quad, GLU_ERROR, NULL );

	// FIXME: maybe support fill/wireframe with a future flag?
	gluQuadricDrawStyle( quad, GLU_FILL );

	// assuming unlit spheres, otherwise use GLU_SMOOTH so that it looks better
	gluQuadricNormals( quad, GLU_NONE );

	// we could set the slices/stacks at some point in the future but just use 16 now since it looks ok
	gluSphere( quad, (GLdouble)rad, 16, 16 );

	// FIXME: I just heard this scream "Globalize Me!!".  It was really scary.  I even cried.
	gluDeleteQuadric( quad );
}
