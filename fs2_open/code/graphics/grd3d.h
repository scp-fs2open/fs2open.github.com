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
 * $Revision: 2.10 $
 * $Date: 2004-02-20 21:45:41 $
 * $Author: randomtiger $
 *
 * Include file for our Direct3D renderer
 *
 * $Log: not supported by cvs2svn $
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

#include "globalincs/systemvars.h"
#include <d3dx8math.h>

void gr_d3d_exb_flush(int end_of_frame);

void d3d_start_frame();
void d3d_stop_frame();
void d3d_set_initial_render_state()	;
void set_stage_for_cell_shaded();
void set_stage_for_cell_glowmapped_shaded();
void set_stage_for_additive_glowmapped();
void set_stage_for_defuse();
void set_stage_for_glow_mapped_defuse();
void set_stage_for_defuse_and_non_mapped_spec();
void set_stage_for_glow_mapped_defuse_and_non_mapped_spec();
bool set_stage_for_spec_mapped();
bool set_stage_for_spec_glow_mapped();
void set_stage_for_mapped_environment_mapping();

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

int gr_d3d_make_buffer(poly_list *list);
void gr_d3d_destroy_buffer(int idx);
void gr_d3d_render_buffer(int idx);
int gr_d3d_make_flat_buffer(poly_list *list);
int gr_d3d_make_line_buffer(line_list *list);

void gr_d3d_set_proj_matrix(float fov, float ratio, float n, float f);
void gr_d3d_end_proj_matrix();
void gr_d3d_set_view_matrix(vector* offset, matrix *orient);
void gr_d3d_end_view_matrix();
void gr_d3d_set_scale_matrix(vector* scale);
void gr_d3d_end_scale_matrix();
void gr_d3d_start_instance_matrix(vector*, matrix*);
void gr_d3d_start_angles_instance_matrix(vector* offset, angles *orient);
void gr_d3d_end_instance_matrix();

void gr_d3d_start_clip();
void gr_d3d_end_clip();

extern ID3DXMatrixStack *world_matrix_stack;
extern ID3DXMatrixStack *view_matrix_stack;
extern ID3DXMatrixStack *proj_matrix_stack;

void gr_d3d_set_texture_addressing(int);

#endif
