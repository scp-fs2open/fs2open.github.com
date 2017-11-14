#pragma once

#include "graphics/2d.h"

extern transform_stack gr_model_matrix_stack;
extern matrix4 gr_view_matrix;
extern matrix4 gr_model_view_matrix;
extern matrix4 gr_projection_matrix;
extern matrix4 gr_last_projection_matrix;
extern matrix4 gr_env_texture_matrix;

void gr_start_instance_matrix(const vec3d *offset, const matrix *rotation);
void gr_start_angles_instance_matrix(const vec3d *pos, const angles *rotation);
void gr_end_instance_matrix();

void gr_set_proj_matrix(float fov, float aspect, float z_near, float z_far);
void gr_end_proj_matrix();

void gr_set_view_matrix(const vec3d *pos, const matrix *orient);
void gr_end_view_matrix();

void gr_set_2d_matrix(/*int x, int y, int w, int h*/);
void gr_end_2d_matrix();

void gr_push_scale_matrix(const vec3d *scale_factor);
void gr_pop_scale_matrix();

void gr_setup_viewport();

void gr_reset_matrices();

extern matrix4 gr_texture_matrix;

void gr_set_texture_panning(float u, float v, bool enable);
