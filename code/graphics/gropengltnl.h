/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


/*
 * $Logfile: /Freespace2/code/Graphics/GrOpenGLTNL.h $
 * $Revision: 1.5 $
 * $Date: 2004-08-11 05:06:24 $
 * $Author: Kazan $
 *
 * header file containing function definitions for HT&L rendering in OpenGL
 *
 * $Log: not supported by cvs2svn $
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
 * Revision 2.3  2004/04/13 01:55:41  phreak
 * put in the correct fields for the CVS comments to register
 * fixed a glowmap problem that occured when rendering glowmapped and non-glowmapped ships
 *
 *
 * $NoKeywords: $
 */

#include "PreProcDefines.h"
#ifndef _GROPENGLTNL_H
#define _GROPENGLTNL_H

#include "globalincs/pstypes.h"
struct poly_list;

void gr_opengl_start_instance_matrix(vector *offset, matrix* rotation);
void gr_opengl_start_instance_angles(vector *pos, angles* rotation);
void gr_opengl_end_instance_matrix();
void gr_opengl_set_projection_matrix(float fov, float aspect, float z_near, float z_far);
void gr_opengl_end_projection_matrix();
void gr_opengl_set_view_matrix(vector *pos, matrix* orient);
void gr_opengl_end_view_matrix();
void gr_opengl_push_scale_matrix(vector *scale_factor);
void gr_opengl_pop_scale_matrix();

void gr_opengl_start_clip_plane();
void gr_opengl_end_clip_plane();

void opengl_init_vertex_buffers();
int gr_opengl_make_buffer(poly_list *list, uint flags);
void gr_opengl_destroy_buffer(int idx);
void gr_opengl_set_buffer(int idx);
void gr_opengl_render_buffer(int start, int n_prim, short* index_list);
void gr_opengl_render_to_env(int FACE);

#endif //_GROPENGLTNL_H