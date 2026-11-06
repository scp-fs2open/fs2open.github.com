#pragma once

#include "graphics/2d.h"
#include "render/3d.h"

extern transform_stack gr_model_matrix_stack;
extern matrix4 gr_view_matrix;
extern matrix4 gr_model_view_matrix;
extern matrix4 gr_projection_matrix;
extern matrix4 gr_last_projection_matrix;
extern matrix4 gr_env_texture_matrix;
extern float gr_near_plane;

// True while the current scene writes motion vectors. Only ever set by a renderer that supports them.
extern bool Gr_motion_vectors_active;

// Records the current view and projection as the main scene's camera, once per rendered scene.
void gr_capture_scene_camera();
// The cockpit has its own near plane, FOV and frame, so draws between these use a separate camera. It's drawn around the
// origin at offsets that turn with the ship and move with its sway, so the camera needs both, and the orientation last frame.
void gr_begin_cockpit_camera(const matrix* ship_orient, const matrix* ship_last_orient, const vec3d* sway);
void gr_end_cockpit_camera();
// The camera that draws write motion vectors against, viewport_rect being xy offset and zw size as fractions of the target.
void gr_get_velocity_camera(matrix4* view_proj, matrix4* previous_view_proj, float viewport_rect[4]);
// The camera moved discontinuously, so temporal effects must not reuse earlier frames.
void gr_reset_temporal_history();
bool gr_consume_temporal_reset();

void gr_matrix_on_frame();

void gr_start_instance_matrix(const vec3d* offset, const matrix* rotation);
void gr_start_angles_instance_matrix(const vec3d* pos, const angles* rotation);
void gr_end_instance_matrix();

void gr_set_proj_matrix(fov_t fov, float aspect, float z_near, float z_far);
void gr_end_proj_matrix();

// Orthographic projection override... when active gr_set_proj_matrix substitutes an
// orthographic frustum sized to match the perspective view at the given camera distance.
void gr_activate_ortho_proj_override(float camera_distance);
void gr_deactivate_ortho_proj_override();

void gr_set_view_matrix(const vec3d* pos, const matrix* orient);
void gr_end_view_matrix();

void gr_set_2d_matrix(/*int x, int y, int w, int h*/);
void gr_end_2d_matrix();

void gr_push_scale_matrix(const vec3d *scale_factor);
void gr_pop_scale_matrix();

void gr_setup_viewport();

void gr_reset_matrices();

extern matrix4 gr_texture_matrix;

void gr_set_texture_panning(float u, float v, bool enable);

/**
 * @brief Set current matrix uniforms
 *
 * Use this before rendering with a shader requiring the matrix uniforms so that the matrix uniform block point has the
 * up-to-date data.
 */
void gr_matrix_set_uniforms();
