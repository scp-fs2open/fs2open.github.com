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
 * $Revision: 1.44.2.12 $
 * $Date: 2007-02-12 00:19:48 $
 * $Author: taylor $
 *
 * source for doing the fun TNL stuff
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.44.2.11  2007/02/10 00:05:25  taylor
 * obsolete gluLookAt() in favor of doing it manually, should be slight faster and more precise
 *
 * Revision 1.44.2.10  2006/12/27 09:26:21  taylor
 * fix OpenGL envmap "issues" (ie, stupid taylor stuff)
 * get rid of that RCS_Name thing, CVS kept changing it automatically and it was getting /really/ annoying
 *
 * Revision 1.44.2.9  2006/12/26 05:25:18  taylor
 * lots of little cleanup, stale code removal, and small performance adjustments
 * get rid of the default combine texture state, we don't need it in general, and it can screw up fonts
 * get rid of the secondary color support, it doesn't do much in non-HTL mode, screws up various things, and has long since been obsolete but material setup
 * get rid of the old gamma setup, it actually conflicts with newer gamma support
 * default texture wrapping to edge clamp
 * do second gr_clear() on init to be sure and catch double-buffer
 * make sure that our active texture will always get reset to 0, rather than leaving it at whatever was used last
 * fixed that damn FBO bug from it hanging on textures and causing some rendering errors for various people
 * only lock verts once in HTL model rendering
 *
 * Revision 1.44.2.8  2006/12/07 18:16:11  taylor
 * minor cleanups and speedups
 * when using lighting falloff, be sure to drop emissive light when falloff gets low enough (otherwise we still se everything fine)
 *
 * Revision 1.44.2.7  2006/10/27 06:42:29  taylor
 * rename set_warp_globals() to model_set_warp_globals()
 * remove two old/unused MR flags (MR_ALWAYS_REDRAW, used for caching that doesn't work; MR_SHOW_DAMAGE, didn't do anything)
 * add MR_FULL_DETAIL to render an object regardless of render/detail box setting
 * change "model_current_LOD" to a global "Interp_detail_level" and the static "Interp_detail_level" to "Interp_detail_level_locked", a bit more descriptive
 * minor bits of cleanup
 * change a couple of vm_vec_scale_add2() calls to just vm_vec_add2() calls in ship.cpp, since that was the final result anyway
 *
 * Revision 1.44.2.6  2006/10/24 13:38:23  taylor
 * fix the envmap-getting-rotated-by-turrets bug (though it did make me chuckle a bit when I first saw it ;))
 * try to reuse old view setup info, if we are using the same settings again (for speed up purposes, will be a bigger deal when bumpmapping gets here)
 *
 * Revision 1.44.2.5  2006/10/01 19:24:46  taylor
 * bit of cleanup (technically the vertex buffer stuff is part of a much larger change slated for post-3.6.9, but this should be a little faster)
 *
 * Revision 1.44.2.4  2006/09/24 13:26:01  taylor
 * minor clean and code optimizations
 * clean up view/proj matrix fubar that made us need far more matrix levels that actually needed (partial fix for Mantis #563)
 * add debug safety check to make sure that we don't use more than 2 proj matrices (all that GL is required to support)
 * set up a texture matrix for the env map to that it doesn't move/look funky
 *
 * Revision 1.44.2.3  2006/07/28 02:45:10  taylor
 * don't render extra lighting passes when fog is enabled, prevents fog fragment from getting added for each pass
 * wobble wobble no more!!
 *
 * Revision 1.44.2.2  2006/07/24 07:38:00  taylor
 * minor cleanup/optimization to beam warmup glow rendering function
 * various lighting code cleanups
 *  - try to always make sure beam origin lights occur outside of model
 *  - make Static_lights[] dynamic
 *  - be sure to reset to first 8 lights when moving on to render spec related texture passes
 *  - add ambient color to point lights (helps warp effects)
 *  - sort lights to try and get more important and/or visible lights to always happen in initial render pass
 *
 * Revision 1.44.2.1  2006/06/12 03:37:24  taylor
 * sync current OGL changes:
 *  - go back to using minimize mode which non-active, but doin't minimize when Fred_running
 *  - remove temporary cmdline options (-spec_scale, -env_scale, -alpha_alpha_blend)
 *  - change FBO renderbuffer link around a little to maybe avoid freaky drivers (or freaky code)
 *
 * Revision 1.44  2006/05/30 03:53:52  taylor
 * z-range for 2D ortho is -1.0 to 1.0, may help avoid some strangeness if we actually get that right. :)
 * minor cleanup of old code and default settings
 * try not to bother with depth test unless we are actually going to need it (small performance boost in some cases)
 * don't clear depth bit in flip(), while technically correct it's also a bit redundant (and comes with a slight performance hit)
 *
 * Revision 1.43  2006/05/27 17:07:48  taylor
 * remove grd3dparticle.* and grd3dbatch.*, they are obsolete
 * allow us to build without D3D support under Windows (just define NO_DIRECT3D)
 * clean up TMAP flags
 * fix a couple of minor OpenGL state change issues with spec and env map rendering
 * make sure we build again for OS X (OGL extension functions work a little different there)
 * render targets always need to be power-of-2 to avoid incomplete buffer issues in the code
 * when we disable culling in opengl_3dunlit be sure that we re-enable it on exit of function
 * re-fix screenshots
 * add true alpha blending support (with cmdline for now since the artwork has the catch up)
 * draw lines with float positioning, to be more accurate with resizing on non-standard resolutions
 * don't load cubemaps from file for D3D, not sure how to do it anyway
 * update geometry batcher code, memory fixes, dynamic stuff, basic fixage, etc.
 *
 * Revision 1.42  2006/05/13 07:29:52  taylor
 * OpenGL envmap support
 * newer OpenGL extension support
 * add GL_ARB_texture_rectangle support for non-power-of-2 textures as interface graphics
 * add cubemap reading and writing support to DDS loader
 * fix bug in DDS loader that made compressed images with mipmaps use more memory than they really required
 * add support for a default envmap named "cubemap.dds"
 * new mission flag "$Environment Map:" to use a pre-existing envmap
 * minor cleanup of compiler warning messages
 * get rid of wasteful math from gr_set_proj_matrix()
 * remove extra gr_set_*_matrix() calls from starfield.cpp as there was no longer a reason for them to be there
 * clean up bmpman flags in reguards to cubemaps and render targets
 * disable D3D envmap code until it can be upgraded to current level of code
 * remove bumpmap code from OpenGL stuff (sorry but it was getting in the way, if it was more than copy-paste it would be worth keeping)
 * replace gluPerspective() call with glFrustum() call, it's a lot less math this way and saves the extra function call
 *
 * Revision 1.41  2006/04/12 01:10:35  taylor
 * some cleanup and slight reorg
 *  - remove special uv offsets for non-standard res, they were stupid anyway and don't actually fix the problem (which should actually be fixed now)
 *  - avoid some costly math where possible in the drawing functions
 *  - add opengl_error_string(), this is part of a later update but there wasn't a reason to not go ahead and commit this peice now
 *  - minor cleanup to Win32 extension defines
 *  - make opengl_lights[] allocate only when using OGL
 *  - cleanup some costly per-frame lighting stuff
 *  - clamp textures for interface and aabitmap (font) graphics since they shouldn't normally repeat anyway (the default)
 *    (doing this for D3D, if it doesn't already, may fix the blue-lines problem since a similar issue was seen with OGL)
 *
 * Revision 1.40  2006/02/25 21:47:00  Goober5000
 * spelling
 *
 * Revision 1.39  2006/01/31 06:43:21  wmcoolmon
 * Debug warning; compiler warning fix.
 *
 * Revision 1.38  2006/01/30 06:52:15  taylor
 * better lighting for OpenGL
 * remove some extra stuff that was from sectional bitmaps since we don't need it anymore
 *
 * Revision 1.37  2006/01/25 03:11:52  phreak
 * htl_lines should follow zbuffering rules
 *
 * Revision 1.36  2006/01/18 16:14:04  taylor
 * allow gr_render_buffer() to take TMAP flags
 * let gr_render_buffer() render untextured polys (OGL only until some D3D people fix it on their side)
 * add MR_SHOW_OUTLINE_HTL flag so we easily render using HTL mode for wireframe views
 * make Interp_verts/Interp_norms/etc. dynamic and get rid of the extra htl_* versions
 *
 * Revision 1.35  2005/12/29 08:08:33  wmcoolmon
 * Codebase commit, most notably including objecttypes.tbl
 *
 * Revision 1.34  2005/12/16 06:48:28  taylor
 * "House Keeping!!"
 *   - minor cleanup of things that have bothered me at one time or another
 *   - slight speedup from state switching
 *   - slightly better specmap handling, fixes a couple of (not frequent) strange and sorta random issues
 *   - make sure to only disable HTL arb stuff when in HTL mode
 *   - handle any extra lighting pass before spec pass so the light can be applied properly
 *
 * Revision 1.33  2005/12/08 15:07:57  taylor
 * remove GL_NO_HTL define since it's basically useless at this point and can produced non-functioning builds
 * minor cleanup and readability changes
 * get Apple GL version change in CVS finally, the capabilities of an Apple GL version don't neccessarily correspond to it's features
 *
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
#include "lighting/lighting.h"
#include "graphics/grinternal.h"
#include "graphics/gropengl.h"
#include "graphics/gropenglextension.h"
#include "graphics/gropengltexture.h"
#include "graphics/gropengllight.h"
#include "graphics/gropengltnl.h"

#include "math/vecmat.h"
#include "render/3d.h"

#include "debugconsole/timerbar.h"

#include <vector>


extern int VBO_ENABLED;
extern int GLOWMAP;
extern int CLOAKMAP;
extern int SPECMAP;
extern int BUMPMAP;
extern vec3d G3_user_clip_normal;
extern vec3d G3_user_clip_point;
extern int Interp_multitex_cloakmap;
extern int Interp_cloakmap_alpha;
extern float Interp_light;

static int GL_modelview_matrix_depth = 1;
static int GL_htl_projection_matrix_set = 0;
static int GL_htl_view_matrix_set = 0;
static int GL_htl_2d_matrix_depth = 0;
static int GL_htl_2d_matrix_set = 0;

static GLfloat GL_env_texture_matrix[16] = { 0.0f };
static bool GL_env_texture_matrix_set = false;

int GL_vertex_data_in = 0;

GLint GL_max_elements_vertices = 4096;
GLint GL_max_elements_indices = 4096;


struct opengl_vertex_buffer
{
	uint stride;		// the current stride
	GLenum format;		// the format passed to glInterleavedArrays()
	uint n_prim;
	uint n_verts;
	float *array_list;	// interleaved array
	GLuint vbo;			// buffer for VBO
	uint flags;			// FVF
	uint vbo_size;

	opengl_vertex_buffer() { memset( this, 0, sizeof(opengl_vertex_buffer) ); }
};


static std::vector<opengl_vertex_buffer> GL_vertex_buffers;
static opengl_vertex_buffer *g_vbp = NULL;
static int GL_vertex_buffers_in_use = 0;

GLuint opengl_create_vbo(uint size, GLfloat *data)
{
	if (!data)
		return 0;

	if (!*data)
		return 0;

	if (size == 0)
		return 0;


	// Kazan: A) This makes that if (buffer_name) work correctly (false = 0, true = anything not 0)
	//				if glGenBuffersARB() doesn't initialized it for some reason
	//        B) It shuts up MSVC about may be used without been initialized
	GLuint buffer_name = 0;

	vglGenBuffersARB(1, &buffer_name);
	
	// make sure we have one
	if (buffer_name) {
		vglBindBufferARB(GL_ARRAY_BUFFER_ARB, buffer_name);
		vglBufferDataARB(GL_ARRAY_BUFFER_ARB, size, data, GL_STATIC_DRAW_ARB );

		// just in case
		if ( opengl_check_for_errors() )
			return 0;
	}

	return buffer_name;
}

int gr_opengl_make_buffer(poly_list *list, uint flags)
{
	if (Cmdline_nohtl)
		return -1;

	int i;
	uint arsize = 0, list_size = 0;
	bool make_vbo = false;
	opengl_vertex_buffer vbuffer;

	// clear out any old errors before we continue with this
	opengl_check_for_errors();

	// don't create vbo for small stuff, performance gain
	// FIXME: This little speed increase appears to cause problems with some cards/drivers.
	//        Disabling it until I can find a solution that works better.
	if (VBO_ENABLED /*&& (list->n_verts >= 250)*/)
		make_vbo = true;

	// setup using flags
	if ( (flags & VERTEX_FLAG_UV1) && (flags & VERTEX_FLAG_NORMAL) && (flags & VERTEX_FLAG_POSITION) ) {
		vbuffer.stride = (8 * sizeof(float));
		vbuffer.format = GL_T2F_N3F_V3F;
	} else if ( (flags & VERTEX_FLAG_UV1) && (flags & VERTEX_FLAG_POSITION) ) {
		vbuffer.stride = (5 * sizeof(float));
		vbuffer.format = GL_T2F_V3F;
	} else if ( (flags & VERTEX_FLAG_POSITION) ) {
		vbuffer.stride = (3 * sizeof(float));
		vbuffer.format = GL_V3F;
	} else {
		Assert( 0 );
	}

	// total size of data
	list_size = vbuffer.stride * list->n_verts;

	// allocate the storage list
	vbuffer.array_list = (float*)vm_malloc(list_size);

	// return invalid if we don't have the memory
	if (vbuffer.array_list == NULL)
		return -1;
		
	memset( vbuffer.array_list, 0, list_size );

	// generate the array
	for (i = 0; i < list->n_verts; i++) {
		vertex *vl = &list->vert[i];
		vec3d *nl = &list->norm[i];

		// don't try to generate more data than what's available
		Assert( ((arsize * sizeof(float)) + vbuffer.stride) <= list_size );

		// NOTE: TEX->NORM->VERT, This array order *must* be preserved!!

		// tex coords
		if (flags & VERTEX_FLAG_UV1) {
			vbuffer.array_list[arsize++] = vl->u;
			vbuffer.array_list[arsize++] = vl->v;
		}

		// normals
		if (flags & VERTEX_FLAG_NORMAL) {
			vbuffer.array_list[arsize++] = nl->xyz.x;
			vbuffer.array_list[arsize++] = nl->xyz.y;
			vbuffer.array_list[arsize++] = nl->xyz.z;
		}

		// verts
		if (flags & VERTEX_FLAG_POSITION) {
			vbuffer.array_list[arsize++] = vl->x;
			vbuffer.array_list[arsize++] = vl->y;
			vbuffer.array_list[arsize++] = vl->z;
		}
	}

	vbuffer.flags = flags;

	vbuffer.n_prim = (list->n_verts / 3);
	vbuffer.n_verts = list->n_verts;

	// maybe load it into a vertex buffer object
	if (make_vbo) {
		vbuffer.vbo = opengl_create_vbo( list_size, vbuffer.array_list );

		if (vbuffer.vbo) {
			// figure up the size so we can know how much VBO data is in card memory
			vbuffer.vbo_size = list_size;
			GL_vertex_data_in += list_size;

			vm_free(vbuffer.array_list);
			vbuffer.array_list = NULL;
		}
	}

	GL_vertex_buffers.push_back( vbuffer );
	GL_vertex_buffers_in_use++;

	return (int)(GL_vertex_buffers.size() - 1);
}

void gr_opengl_set_buffer(int idx)
{
	if (Cmdline_nohtl)
		return;

	g_vbp = NULL;

	if ( (idx < 0) || (idx >= (int)GL_vertex_buffers.size()) )
		return;

	g_vbp = &GL_vertex_buffers[idx];
}

void gr_opengl_destroy_buffer(int idx)
{
	if (Cmdline_nohtl)
		return;

	if ( (idx < 0) || (idx >= (int)GL_vertex_buffers.size()) )
		return;

	opengl_vertex_buffer *vbp = &GL_vertex_buffers[idx];

	if (vbp->array_list)
		vm_free(vbp->array_list);

	if (vbp->vbo) {
		vglDeleteBuffersARB(1, &vbp->vbo);
		GL_vertex_data_in -= vbp->vbo_size;
	}

	memset( vbp, 0, sizeof(opengl_vertex_buffer) );

	// we try to take advantage of the fact that there shouldn't be a lot of buffer
	// deletions/additions going on all of the time, so a model_unload_all() and/or
	// game_level_close() should pretty much keep everything cleared out on a
	// pretty much regular basis
	if (--GL_vertex_buffers_in_use <= 0)
		GL_vertex_buffers.clear();

	g_vbp = NULL;
}

#define DO_RENDER() {	\
	if ( (ibuffer != NULL) || (sbuffer != NULL) ) {	\
		if (ibuffer) {	\
			vglDrawRangeElements(GL_TRIANGLES, start, end, count, GL_UNSIGNED_INT, ibuffer + start);	\
		} else {	\
			vglDrawRangeElements(GL_TRIANGLES, start, end, count, GL_UNSIGNED_SHORT, sbuffer + start);	\
		}	\
	} else {	\
		glDrawArrays(GL_TRIANGLES, 0, vbp->n_verts);	\
	}	\
}



//#define DRAW_DEBUG_LINES
extern void opengl_default_light_settings(int amb = 1, int emi = 1, int spec = 1);

//start is the first part of the buffer to render, n_prim is the number of primitives, index_list is an index buffer, if index_list == NULL render non-indexed
void gr_opengl_render_buffer(int start, int n_prim, ushort *sbuffer, uint *ibuffer, int flags)
{
	if (Cmdline_nohtl)
		return;

	Assert(GL_htl_projection_matrix_set);
	Assert(GL_htl_view_matrix_set);

	TIMERBAR_PUSH(2);

	float u_scale,v_scale;
	int render_pass = 0;
	int i, r, g, b, a, tmap_type;
	bool use_spec = false;

	int end = ((n_prim * 3) - 1);
	int count = (end - start + 1); //(n_prim * 3);

	int textured = (flags & TMAP_FLAG_TEXTURED);

	opengl_vertex_buffer *vbp = g_vbp;
	Assert( g_vbp );


	// disable all arbs before we start
	opengl_switch_arb(-1, 0);

	if ( glIsEnabled(GL_CULL_FACE) )
		glFrontFace(GL_CW);

	opengl_setup_render_states(r, g, b, a, tmap_type, (textured) ? TMAP_FLAG_TEXTURED : 0);
	glColor4ub( (ubyte)r, (ubyte)g, (ubyte)b, (ubyte)a );


// -------- Begin 1st PASS ------------------------------------------------------- //
	// basic setup of all data and first texture
	vglClientActiveTextureARB(GL_TEXTURE0_ARB+render_pass);
	if (vbp->vbo) {
		vglBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo);
		glInterleavedArrays(vbp->format, 0, (void*)NULL);
	} else {
		glInterleavedArrays(vbp->format, 0, vbp->array_list);
	}

	if (textured) {
		// base texture
		gr_opengl_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0, 0, render_pass);

		// increment texture count for this pass
		render_pass++; // bump!

		if ( (Interp_multitex_cloakmap > 0) && (vbp->flags & VERTEX_FLAG_UV1) ) {
			SPECMAP = -1;	// don't add a spec map if we are cloaked
			GLOWMAP = -1;	// don't use a glowmap either, shouldn't see them

			vglClientActiveTextureARB(GL_TEXTURE0_ARB+render_pass);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			if (vbp->vbo) {
				vglBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo);
				glTexCoordPointer( 2, GL_FLOAT, vbp->stride, (void*)NULL );
			} else {
				glTexCoordPointer( 2, GL_FLOAT, vbp->stride, vbp->array_list );
			}

			gr_opengl_tcache_set(Interp_multitex_cloakmap, tmap_type, &u_scale, &v_scale, 0, 0, render_pass);

			render_pass++; // bump!
		}

		// glowmaps!
		if ( (GLOWMAP > -1) && !Cmdline_noglow && (vbp->flags & VERTEX_FLAG_UV1) ) {
			vglClientActiveTextureARB(GL_TEXTURE0_ARB+render_pass);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			if (vbp->vbo) {
				vglBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo);
				glTexCoordPointer( 2, GL_FLOAT, vbp->stride, (void*)NULL );
			} else {
				glTexCoordPointer( 2, GL_FLOAT, vbp->stride, vbp->array_list );
			}

			// set glowmap on relevant ARB
			gr_opengl_tcache_set(GLOWMAP, tmap_type, &u_scale, &v_scale, 0, 0, render_pass);

			opengl_switch_arb(render_pass, 1);
			opengl_set_additive_tex_env();

			render_pass++; // bump!
		}

		// determine if we are going to use a specmap (for later checking)
		if ( (SPECMAP > -1) && !Cmdline_nospec && (lighting_is_enabled) && !(glIsEnabled(GL_FOG)) && (vbp->flags & VERTEX_FLAG_UV1) ) {
			use_spec = true;
		}
	}

	if (lighting_is_enabled)
		glEnable(GL_NORMALIZE);

	opengl_pre_render_init_lights();
	opengl_change_active_lights(0);

	// only change lights if we have a specmap, don't render the specmap when fog is enabled
	if ( use_spec ) {
		opengl_default_light_settings(!GL_center_alpha, (Interp_light > 0.25f), 0); // don't render with spec lighting here
	} else {
		// reset to defaults
		opengl_default_light_settings(!GL_center_alpha, (Interp_light > 0.25f));
	}

	gr_opengl_set_center_alpha(GL_center_alpha);

	// NOTE: the unlock call is at the end of all drawing ...
	vglLockArraysEXT( 0, vbp->n_verts);

	// DRAW IT!!
	DO_RENDER();

// -------- End 1st PASS --------------------------------------------------------- //

// -------- Begin lighting pass (conditional but should happen before spec pass) - //
	if ( (textured) && (lighting_is_enabled) && !(glIsEnabled(GL_FOG)) && (Num_active_gl_lights > GL_max_lights) ) {
		// the lighting code needs to do this better, may need some adjustment later since I'm only trying
		// to avoid rendering 7+ extra passes for lights which probably won't affect current object, but as
		// a performance hack I guess this will have to do for now...
		// restrict the number of extra lighting passes based on LOD:
		//  - LOD0:  only 2 extra passes (3 main passes total, rendering 24 light sources)
		//  - LOD1:  only 1 extra pass   (2 main passes total, rendering 16 light sources)
		//  - LOD2+: no extra passes     (1 main pass   total, rendering  8 light sources)
		extern int Interp_detail_level;
		int max_passes = (2 - Interp_detail_level);

		if (max_passes > 0) {
			opengl_set_state( TEXTURE_SOURCE_DECAL, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_READ );

			for (i = 1; i < render_pass; i++) {
				opengl_switch_arb(i, 0);
			}

			int max_lights = (Num_active_gl_lights - 1) / GL_max_lights;
			for (i = 1; (i < max_lights) && (i < max_passes); i++) {
				opengl_change_active_lights(i);

				// DRAW IT!!
				DO_RENDER();
			}

			// reset the active lights to the first set to render the spec related passes with
			// for performance and quality reasons they don't get special lighting passes
			opengl_change_active_lights(0);
		}
	}
// -------- End lighting PASS ---------------------------------------------------- //

// -------- Begin 2nd PASS (env) ------------------------------------------------- //
	if ( use_spec && Cmdline_env && (ENVMAP >= 0) && Is_Extension_Enabled(OGL_ARB_TEXTURE_ENV_COMBINE) ) {
		// turn all previously used arbs off before the specular pass
		// this fixes the glowmap multitexture rendering problem - taylor
		for (i = 0; i < render_pass; i++) {
			opengl_switch_arb(i, 0);
		}

		render_pass = 0;

		// set specmap, for us to modulate against
		vglClientActiveTextureARB(GL_TEXTURE0_ARB+render_pass);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if (vbp->vbo) {
			vglBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo);
			glTexCoordPointer( 2, GL_FLOAT, vbp->stride, (void*)NULL );
		} else {
			glTexCoordPointer( 2, GL_FLOAT, vbp->stride, vbp->array_list );
		}

		// set specmap on relevant ARB
		gr_opengl_tcache_set(SPECMAP, tmap_type, &u_scale, &v_scale, 0, 0, render_pass);

		opengl_switch_arb(render_pass, 1);

		// as a crazy and sometimes useless hack, avoid using alpha when specmap has none
		if ( Cmdline_alpha_env && bm_has_alpha_channel(SPECMAP) ) {
			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB );
			glTexEnvf( GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_TEXTURE );
			glTexEnvf( GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_ALPHA );
			glTexEnvf(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1.0f);
		} else {
			glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1.0f);
		}

		render_pass++; // bump!

		// now move the to the envmap
		vglClientActiveTextureARB(GL_TEXTURE0_ARB+render_pass);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if (vbp->vbo) {
			vglBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo);
			glTexCoordPointer( 2, GL_FLOAT, vbp->stride, (void*)NULL );
		} else {
			glTexCoordPointer( 2, GL_FLOAT, vbp->stride, vbp->array_list );
		}

		gr_opengl_tcache_set(ENVMAP, TCACHE_TYPE_CUBEMAP, &u_scale, &v_scale, 0, 0, render_pass);

		opengl_set_texture_target(GL_TEXTURE_CUBE_MAP);

		opengl_switch_arb(render_pass, 1);

		opengl_set_modulate_tex_env();

		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 2.0f);

		opengl_set_state( GL_current_tex_src, ALPHA_BLEND_ADDITIVE, GL_current_ztype);

		glDepthMask(GL_FALSE);
		glDepthFunc(GL_EQUAL);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
		glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);

		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);

		// set the matrix for the texture mode
		if (GL_env_texture_matrix_set) {
			glMatrixMode(GL_TEXTURE);
			glPushMatrix();
			glLoadMatrixf(GL_env_texture_matrix);
			// switch back to the default modelview mode
			glMatrixMode(GL_MODELVIEW);
		}

		render_pass++; // bump!

		// DRAW IT!!
		DO_RENDER();

		// disable and reset everything we changed
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);
		glDisable(GL_TEXTURE_CUBE_MAP);

		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1.0f);

		// pop off the texture matrix we used for the envmap
		if (GL_env_texture_matrix_set) {
			glMatrixMode(GL_TEXTURE);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
		}

		opengl_set_texture_target();

		if (Cmdline_alpha_env) {
			opengl_switch_arb(0, 1);  // assumes that the spec map was TEX0
			glTexEnvf(GL_TEXTURE_ENV, GL_ALPHA_SCALE, 1.0f);
			glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
		} else {
			opengl_switch_arb(0, 1);
			glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1.0f);
		}
	}
// -------- End 2nd PASS --------------------------------------------------------- //

// -------- Begin 3rd (specular) PASS -------------------------------------------- //
	if ( use_spec ) {
		// turn all previously used arbs off before the specular pass
		// this fixes the glowmap multitexture rendering problem - taylor
		for (i = 0; i < render_pass; i++) {
			opengl_switch_arb(i, 0);
		}

		vglClientActiveTextureARB(GL_TEXTURE0_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if (vbp->vbo) {
			vglBindBufferARB(GL_ARRAY_BUFFER_ARB, vbp->vbo);
			glTexCoordPointer( 2, GL_FLOAT, vbp->stride, (void*)NULL );
		} else {
			glTexCoordPointer( 2, GL_FLOAT, vbp->stride, vbp->array_list );
		}

		opengl_set_spec_mapping(tmap_type, &u_scale, &v_scale);

		// DRAW IT!!
		DO_RENDER();

		glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE_ARB, 1.0f);

		opengl_reset_spec_mapping();
	}
// -------- End 3rd PASS --------------------------------------------------------- //


	// unlock the arrays
	vglUnlockArraysEXT();

	// make sure everthing gets turned back off, fixes hud issue with spec lighting and VBO crash in starfield
	opengl_switch_arb(-1, 0);
	glDisable(GL_NORMALIZE);


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

	TIMERBAR_POP();
}

void gr_opengl_start_instance_matrix(vec3d *offset, matrix *rotation)
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
	vm_matrix_to_rot_axis_and_angle(rotation, &ang, &axis);

	glTranslatef( offset->xyz.x, offset->xyz.y, offset->xyz.z );
	glRotatef( fl_degrees(ang), axis.xyz.x, axis.xyz.y, axis.xyz.z );

	GL_modelview_matrix_depth++;
}

void gr_opengl_start_instance_angles(vec3d *pos, angles *rotation)
{
	if (Cmdline_nohtl)
		return;

	Assert(GL_htl_projection_matrix_set);
	Assert(GL_htl_view_matrix_set);

	matrix m;
	vm_angles_2_matrix(&m, rotation);

	gr_opengl_start_instance_matrix(pos, &m);
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

// the projection matrix; fov, aspect ratio, near, far
void gr_opengl_set_projection_matrix(float fov, float aspect, float z_near, float z_far)
{
	if (Cmdline_nohtl)
		return;

	glViewport(gr_screen.offset_x, (gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_height), gr_screen.clip_width, gr_screen.clip_height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	GLdouble clip_width, clip_height;

	clip_height = tan( (double)fov / 2.0 ) * z_near;
	clip_width = clip_height * (GLdouble)aspect;

	glFrustum( -clip_width, clip_width, -clip_height, clip_height, z_near, z_far );

	glMatrixMode(GL_MODELVIEW);

	GL_htl_projection_matrix_set = 1;
}

void gr_opengl_end_projection_matrix()
{
	if (Cmdline_nohtl)
		return;

	glViewport(0, 0, gr_screen.max_w, gr_screen.max_h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, gr_screen.max_w, gr_screen.max_h, 0, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);

	GL_htl_projection_matrix_set = 0;
}


static GLdouble eyex, eyey, eyez;
static GLdouble vmatrix[16];

static vec3d last_view_pos;
static matrix last_view_orient;

static bool use_last_view = false;

void gr_opengl_set_view_matrix(vec3d *pos, matrix *orient)
{
	if (Cmdline_nohtl)
		return;

	Assert(GL_htl_projection_matrix_set);
	Assert(GL_modelview_matrix_depth == 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	// right now it depends on your settings as to whether this has any effect in-mission
	// not much good now, but should be a bit more useful later on
	if ( !memcmp(pos, &last_view_pos, sizeof(vec3d)) && !memcmp(orient, &last_view_orient, sizeof(matrix)) ) {
		use_last_view = true;
	} else {
		memcpy(&last_view_pos, pos, sizeof(vec3d));
		memcpy(&last_view_orient, orient, sizeof(matrix));

		use_last_view = false;
	}

	if ( !use_last_view ) {
		// should already be normalized
		eyex =  (GLdouble)pos->xyz.x;
		eyey =  (GLdouble)pos->xyz.y;
		eyez = -(GLdouble)pos->xyz.z;

		// should already be normalized
		GLdouble fwdx =  (GLdouble)orient->vec.fvec.xyz.x;
		GLdouble fwdy =  (GLdouble)orient->vec.fvec.xyz.y;
		GLdouble fwdz = -(GLdouble)orient->vec.fvec.xyz.z;

		// should already be normalized
		GLdouble upx =  (GLdouble)orient->vec.uvec.xyz.x;
		GLdouble upy =  (GLdouble)orient->vec.uvec.xyz.y;
		GLdouble upz = -(GLdouble)orient->vec.uvec.xyz.z;

		GLdouble mag;

		// setup Side vector (crossprod of forward and up vectors)
		GLdouble Sx = (fwdy * upz) - (fwdz * upy);
		GLdouble Sy = (fwdz * upx) - (fwdx * upz);
		GLdouble Sz = (fwdx * upy) - (fwdy * upx);

		// normalize Side
		mag = 1.0 / sqrt( (Sx*Sx) + (Sy*Sy) + (Sz*Sz) );

		Sx *= mag;
		Sy *= mag;
		Sz *= mag;

		// setup Up vector (crossprod of Side and forward vectors)
		GLdouble Ux = (Sy * fwdz) - (Sz * fwdy);
		GLdouble Uy = (Sz * fwdx) - (Sx * fwdz);
		GLdouble Uz = (Sx * fwdy) - (Sy * fwdx);

		// normalize Up
		mag = 1.0 / sqrt( (Ux*Ux) + (Uy*Uy) + (Uz*Uz) );

		Ux *= mag;
		Uy *= mag;
		Uz *= mag;

		// store the result in our matrix
		memset( vmatrix, 0, sizeof(vmatrix) );
		vmatrix[0]  = Sx;   vmatrix[1]  = Ux;   vmatrix[2]  = -fwdx;
		vmatrix[4]  = Sy;   vmatrix[5]  = Uy;   vmatrix[6]  = -fwdy;
		vmatrix[8]  = Sz;   vmatrix[9]  = Uz;   vmatrix[10] = -fwdz;
		vmatrix[15] = 1.0;
	}

	glLoadMatrixd(vmatrix);
	
	glTranslated(-eyex, -eyey, -eyez);
	glScalef(1.0f, 1.0f, -1.0f);


	if ( Cmdline_env ) {
		GL_env_texture_matrix_set = true;

		// if our view setup is the same as previous call then we can skip this
		if ( !use_last_view ) {
			// setup the texture matrix which will make the the envmap keep lined
			// up properly with the environment
			GLfloat mview[16];

			glGetFloatv(GL_MODELVIEW_MATRIX, mview);

			// r.xyz  <--  r.x, u.x, f.x
			GL_env_texture_matrix[0]  =  mview[0];
			GL_env_texture_matrix[1]  = -mview[4];
			GL_env_texture_matrix[2]  =  mview[8];
			// u.xyz  <--  r.y, u.y, f.y
			GL_env_texture_matrix[4]  =  mview[1];
			GL_env_texture_matrix[5]  = -mview[5];
			GL_env_texture_matrix[6]  =  mview[9];
			// f.xyz  <--  r.z, u.z, f.z
			GL_env_texture_matrix[8]  =  mview[2];
			GL_env_texture_matrix[9]  = -mview[6];
			GL_env_texture_matrix[10] =  mview[10];

			GL_env_texture_matrix[15] = 1.0f;
		}
	}

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

	GL_modelview_matrix_depth = 1;
	GL_htl_view_matrix_set = 0;
	GL_env_texture_matrix_set = false;
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

	// the viewport needs to be the full screen size since glOrtho() is relative to it
	glViewport(0, 0, gr_screen.max_w, gr_screen.max_h);

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();

	// the top and bottom positions are reversed on purpose
	glOrtho( 0, gr_screen.max_w, gr_screen.max_h, 0, -1, 1 );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

#ifndef NDEBUG
	// safety check to make sure we don't use more than 2 projection matrices
	GLint num_proj_stacks = 0;
	glGetIntegerv( GL_PROJECTION_STACK_DEPTH, &num_proj_stacks );
	Assert( num_proj_stacks <= 2 );
#endif

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

	// reset viewport to what it was originally set to by the proj matrix
	glViewport(gr_screen.offset_x, (gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_height), gr_screen.clip_width, gr_screen.clip_height);

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

	glScalef(scale_factor->xyz.x, scale_factor->xyz.y, scale_factor->xyz.z);
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

	GLdouble clip_equation[4];

	clip_equation[0] = (GLdouble)G3_user_clip_normal.xyz.x;
	clip_equation[1] = (GLdouble)G3_user_clip_normal.xyz.y;
	clip_equation[2] = (GLdouble)G3_user_clip_normal.xyz.z;

	clip_equation[3] = (GLdouble)(G3_user_clip_normal.xyz.x * G3_user_clip_point.xyz.x)
						+ (GLdouble)(G3_user_clip_normal.xyz.y * G3_user_clip_point.xyz.y)
						+ (GLdouble)(G3_user_clip_normal.xyz.z * G3_user_clip_point.xyz.z);
	clip_equation[3] *= -1.0;

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

void gr_opengl_set_line_width(float width)
{
    glLineWidth(width);
}

void gr_opengl_draw_htl_line(vec3d *start, vec3d* end)
{
    if (Cmdline_nohtl)
        return;

    gr_zbuffer_type zbuffer_state = (gr_zbuffering) ? ZBUFFER_TYPE_FULL : ZBUFFER_TYPE_NONE;
    opengl_set_state(TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, zbuffer_state);

    glBegin(GL_LINES);
        if (gr_screen.current_color.is_alphacolor)
        {
            glColor4ub(gr_screen.current_color.red, gr_screen.current_color.green,
                       gr_screen.current_color.blue, gr_screen.current_color.alpha);
        }
        else
        {
            glColor3ub(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue);
        }

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
