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
int gr_opengl_make_buffer(poly_list *list);
void gr_opengl_destroy_buffer(int idx);
void gr_opengl_render_buffer(int idx);


#endif //_GROPENGLTNL_H