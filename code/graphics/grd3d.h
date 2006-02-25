/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

/*
 * $Logfile: /Freespace2/code/Graphics/GrD3D.h $
 * $Revision: 2.22 $
 * $Date: 2006-02-25 21:47:00 $
 * $Author: Goober5000 $
 *
 * Include file for our Direct3D renderer
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.21  2006/01/18 16:14:04  taylor
 * allow gr_render_buffer() to take TMAP flags
 * let gr_render_buffer() render untextured polys (OGL only until some D3D people fix it on their side)
 * add MR_SHOW_OUTLINE_HTL flag so we easily render using HTL mode for wireframe views
 * make Interp_verts/Interp_norms/etc. dynamic and get rid of the extra htl_* versions
 *
 * Revision 2.20  2005/10/16 11:20:43  taylor
 * use unsigned index buffers
 *
 * Revision 2.19  2005/07/13 02:50:47  Goober5000
 * remove PreProcDefine #includes in FS2
 * --Goober5000
 *
 * Revision 2.18  2005/04/05 05:53:17  taylor
 * s/vector/vec3d/g, better support for different compilers (Jens Granseuer)
 *
 * Revision 2.17  2005/03/07 13:10:21  bobboau
 * commit of render target code, d3d should be totaly functional,
 * OGL still needs implementation.
 *
 * Revision 2.16  2004/08/11 05:06:24  Kazan
 * added preprocdefines.h to prevent what happened with fred -- make sure to make all fred2 headers include this file as the _first_ include -- i have already modified fs2 files to do this
 *
 * Revision 2.15  2004/07/11 03:22:49  bobboau
 * added the working decal code
 *
 * Revision 2.14  2004/07/05 05:09:19  bobboau
 * FVF code, only the data that is needed is sent off to the card,,
 * OGL can take advantage of this if they want but it won't break
 * anything if they don't. also state block code has been implemented,
 * that's totaly internal to D3D no high level code is involved.
 *
 * Revision 2.13  2004/07/01 01:12:31  bobboau
 * implemented index buffered background bitmaps,
 * OGL people you realy should get this implemented
 *
 * Revision 2.12  2004/06/28 02:13:07  bobboau
 * high level index buffer suport and d3d implementation,
 * OGL people need to get this working on your end as it's broke now
 *
 * Revision 2.11  2004/03/17 04:07:29  bobboau
 * new fighter beam code
 * fixed old after burner trails
 * had to bump a few limits, working on some dynamic solutions
 * a few fixed to background POF rendering
 * fixing asorted bugs
 *
 * Revision 2.10  2004/02/20 21:45:41  randomtiger
 * Removed some uneeded code between NO_DIRECT3D and added gr_zbias call, ogl is set to a stub func.
 * Changed -htl param to -nohtl. Fixed some badly named functions to match convention.
 * Fixed setup of center_alpha in OGL which was causing crash.
 *
 * Revision 2.9  2004/02/15 06:02:31  bobboau
 * fixed sevral asorted matrix errors,
 * OGL people make sure I didn't break anything,
 * most of what I did was replaceing falses with (if graphicts_mode == D3D)
 *
 * Revision 2.8  2004/02/14 00:18:31  randomtiger
 * Please note that from now on OGL will only run with a registry set by Launcher v4. See forum for details.
 * OK, these changes effect a lot of file, I suggest everyone updates ASAP:
 * Removal of many files from project.
 * Removal of meanless Gr_bitmap_poly variable.
 * Removal of glide, directdraw, software modules all links to them, and all code specific to those paths.
 * Removal of redundant Fred paths that arent needed for Fred OGL.
 * Have seriously tidied the graphics initialisation code and added generic non standard mode functionality.
 * Fixed many D3D non standard mode bugs and brought OGL up to the same level.
 * Removed texture section support for D3D8, voodoo 2 and 3 cards will no longer run under fs2_open in D3D, same goes for any card with a maximum texture size less than 1024.
 *
 * Revision 2.7  2003/11/17 04:25:56  bobboau
 * made the poly list dynamicly alocated,
 * started work on fixing the node model not rendering,
 * but most of that got commented out so I wouldn't have to deal with it
 * while mucking about with the polylist
 *
 * Revision 2.6  2003/11/01 21:59:21  bobboau
 * new matrix handeling code, and fixed some problems with 3D lit verts,
 * several other small fixes
 *
 * Revision 2.5  2003/10/25 03:26:39  phreak
 * fixed some old bugs that reappeared after RT committed his texture code
 *
 * Revision 2.4  2003/10/23 18:03:24  randomtiger
 * Bobs changes (take 2)
 *
 * Revision 2.3  2003/10/18 02:45:39  phreak
 * edited gr_d3d_start_instance_matrix to make it take a vector* and a matrix*, but it doesn't do anything yet
 *
 * Revision 2.2  2003/10/17 17:18:42  randomtiger
 * Big restructure for D3D and new modules grd3dlight and grd3dsetup
 *
 * Revision 2.1  2003/03/18 10:07:02  unknownplayer
 * The big DX/main line merge. This has been uploaded to the main CVS since I can't manage to get it to upload to the DX branch. Apologies to all who may be affected adversely, but I'll work to debug it as fast as I can.
 *
 * Revision 2.0.2.5  2002/11/09 19:28:15  randomtiger
 *
 * Fixed small gfx initialisation bug that wasnt actually causing any problems.
 * Tided DX code, shifted stuff around, removed some stuff and documented some stuff.
 *
 * Revision 2.0.2.4  2002/10/16 00:41:38  randomtiger
 * Fixed small bug that was stopping unactive text from displaying greyed out
 * Also added ability to run FS2 DX8 in 640x480, however I needed to make a small change to 2d.cpp
 * which invloved calling the resolution processing code after initialising the device for D3D only.
 * This is because D3D8 for the moment has its own internal launcher.
 * Also I added a fair bit of documentation and tidied some stuff up. - RT
 *
 * Revision 2.0.2.3  2002/10/11 18:50:54  randomtiger
 * Checked in fix for 16 bit problem, thanks to Righteous1
 * Removed a fair bit of code that was used by the 16 bit code path which no longer exists.
 * 32 bit and 16 bit should now work in exactly the same way. - RT
 *
 * Revision 2.0.2.2  2002/10/02 11:40:19  randomtiger
 * Bmpmap has been reverted to an old non d3d8 version.
 * All d3d8 code is now in the proper place.
 * PCX code is now working to an extent. Problems with alpha though.
 * Ani's work slowly with alpha problems.
 * Also I have done a bit of tidying - RT
 *
 * Revision 2.0.2.1  2002/09/24 18:56:42  randomtiger
 * DX8 branch commit
 *
 * This is the scub of UP's previous code with the more up to date RT code.
 * For full details check previous dev e-mails
 *
 * Revision 2.0  2002/06/03 04:02:22  penguin
 * Warpcore CVS sync
 *
 * Revision 1.1  2002/05/02 18:03:07  mharris
 * Initial checkin - converted filenames and includes to lower case
 *
 * 
 * 6     9/13/99 11:25p Dave
 * Fixed problem with mode-switching and D3D movies.
 * 
 * 5     9/04/99 8:00p Dave
 * Fixed up 1024 and 32 bit movie support.
 * 
 * 4     6/29/99 10:35a Dave
 * Interface polygon bitmaps! Whee!
 * 
 * 3     1/15/99 11:29a Neilk
 * Fixed D3D screen/texture pixel formatting problem. 
 * 
 * 2     10/07/98 10:52a Dave
 * Initial checkin.
 * 
 * 1     10/07/98 10:49a Dave
 * 
 * 2     5/12/97 12:27p John
 * Restructured Graphics Library to add support for multiple renderers.
 * 
 * 1     5/01/97 2:17p John
 *
 * $NoKeywords: $
 */

#ifndef _GRD3D_H
#define _GRD3D_H

struct poly_list;
struct line_list;
struct colored_vector;

#include "globalincs/systemvars.h"
#include <d3dx8math.h>

void gr_d3d_exb_flush(int end_of_frame);

extern DWORD 
initial_state_block, 
defuse_state_block, 
glow_mapped_defuse_state_block, 
nonmapped_specular_state_block, 
glow_mapped_nonmapped_specular_state_block, 
mapped_specular_state_block,
cell_state_block, 
glow_mapped_cell_state_block, 
additive_glow_mapping_state_block, 
//single_pass_specmapping_state_block, 
//single_pass_glow_spec_mapping_state_block, 
background_fog_state_block, 
env_state_block, 
cloak_state_block;

void d3d_start_frame();
void d3d_stop_frame();
void d3d_set_initial_render_state(bool set = false)	;
void set_stage_for_defuse(bool set = false);
void set_stage_for_glow_mapped_defuse(bool set = false);
void set_stage_for_defuse_and_non_mapped_spec(bool set = false);
void set_stage_for_glow_mapped_defuse_and_non_mapped_spec(bool set = false);
bool set_stage_for_spec_mapped(bool set = false);
void set_stage_for_cell_shaded(bool set = false);
void set_stage_for_cell_glowmapped_shaded(bool set = false);
void set_stage_for_additive_glowmapped(bool set = false);
void set_stage_for_background_fog(bool set = false);
bool set_stage_for_env_mapped(bool set = false);
//void set_stage_for_single_pass_specmapping(int SAME, bool set = true);
//void set_stage_for_single_pass_glow_specmapping(int SAME, bool set = true);

void gr_d3d_flip();
void gr_d3d_flip_cleanup();
void gr_d3d_fade_in(int instantaneous);
void gr_d3d_fade_out(int instantaneous);
int gr_d3d_save_screen();
void gr_d3d_restore_screen(int id);
void gr_d3d_free_screen(int id);
void gr_d3d_dump_frame_start(int first_frame, int frames_between_dumps);
void gr_d3d_flush_frame_dump();
void gr_d3d_dump_frame_stop();
void gr_d3d_dump_frame();
uint gr_d3d_lock();
void gr_d3d_unlock();
void gr_d3d_fog_set(int fog_mode, int r, int g, int b, float fog_near, float fog_far);
void gr_d3d_set_gamma(float gamma);
void gr_d3d_set_cull(int cull);
void gr_d3d_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct);
void gr_d3d_filter_set(int filter);
void gr_d3d_set_clear_color(int r, int g, int b);
void gr_d3d_get_region(int front, int w, int h, ubyte *data);

int gr_d3d_make_buffer(poly_list *list, uint flags);
void gr_d3d_destroy_buffer(int idx);
void gr_d3d_render_buffer(int start, int n_prim, ushort* index_list, int flags);
void gr_d3d_set_buffer(int idx);
int gr_d3d_make_flat_buffer(poly_list *list);
int gr_d3d_make_line_buffer(line_list *list);

void gr_d3d_set_proj_matrix(float fov, float ratio, float n, float f);
void gr_d3d_end_proj_matrix();
void gr_d3d_set_view_matrix(vec3d* offset, matrix *orient);
void gr_d3d_end_view_matrix();
void gr_d3d_set_scale_matrix(vec3d* scale);
void gr_d3d_end_scale_matrix();
void gr_d3d_start_instance_matrix(vec3d*, matrix*);
void gr_d3d_start_angles_instance_matrix(vec3d* offset, angles *orient);
void gr_d3d_end_instance_matrix();

void d3d_set_texture_panning(float u, float v, bool enable);

void gr_d3d_start_clip();
void gr_d3d_end_clip();

extern ID3DXMatrixStack *world_matrix_stack;
extern ID3DXMatrixStack *view_matrix_stack;
extern ID3DXMatrixStack *proj_matrix_stack;

void gr_d3d_set_texture_addressing(int);

void gr_d3d_setup_background_fog(bool);

void gr_d3d_draw_line_list(colored_vector*lines, int num);
//void d3d_render_to_env(int FACE);
#endif

