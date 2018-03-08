
#include "matrix.h"

transform_stack gr_model_matrix_stack;
matrix4 gr_view_matrix;
matrix4 gr_model_view_matrix;
matrix4 gr_projection_matrix;
matrix4 gr_last_projection_matrix;
matrix4 gr_last_view_matrix;

matrix4 gr_texture_matrix;

matrix4 gr_env_texture_matrix;
static bool gr_env_texture_matrix_set = false;

bool gr_htl_projection_matrix_set = false;

static int modelview_matrix_depth = 1;
static bool htl_view_matrix_set = false;
static int htl_2d_matrix_depth = 0;
static bool htl_2d_matrix_set = false;

static matrix4 create_view_matrix(const vec3d *pos, const matrix *orient)
{
	vec3d scaled_pos;
	vec3d inv_pos;
	matrix scaled_orient = *orient;
	matrix inv_orient;

	vm_vec_copy_scale(&scaled_pos, pos, -1.0f);
	vm_vec_scale(&scaled_orient.vec.fvec, -1.0f);

	vm_copy_transpose(&inv_orient, &scaled_orient);
	vm_vec_rotate(&inv_pos, &scaled_pos, &scaled_orient);

	matrix4 out;
	vm_matrix4_set_transform(&out, &inv_orient, &inv_pos);

	return out;
}
static void create_perspective_projection_matrix(matrix4 *out, float left, float right, float bottom, float top, float near_dist, float far_dist)
{
	memset(out, 0, sizeof(matrix4));

	out->a1d[0] = 2.0f * near_dist / (right - left);
	out->a1d[5] = 2.0f * near_dist / (top - bottom);
	out->a1d[8] = (right + left) / (right - left);
	out->a1d[9] = (top + bottom) / (top - bottom);
	out->a1d[10] = -(far_dist + near_dist) / (far_dist - near_dist);
	out->a1d[11] = -1.0f;
	out->a1d[14] = -2.0f * far_dist * near_dist / (far_dist - near_dist);
}

static void create_orthographic_projection_matrix(matrix4* out, float left, float right, float bottom, float top, float near_dist, float far_dist)
{
	memset(out, 0, sizeof(matrix4));

	out->a1d[0] = 2.0f / (right - left);
	out->a1d[5] = 2.0f / (top - bottom);
	out->a1d[10] = -2.0f / (far_dist - near_dist);
	out->a1d[12] = -(right + left) / (right - left);
	out->a1d[13] = -(top + bottom) / (top - bottom);
	out->a1d[14] = -(far_dist + near_dist) / (far_dist - near_dist);
	out->a1d[15] = 1.0f;
}

void gr_start_instance_matrix(const vec3d *offset, const matrix *rotation)
{
	Assert( htl_view_matrix_set );

	if (offset == NULL) {
		offset = &vmd_zero_vector;
	}

	if (rotation == NULL) {
		rotation = &vmd_identity_matrix;
	}

	gr_model_matrix_stack.push(offset, rotation);

	auto model_matrix = gr_model_matrix_stack.get_transform();
	vm_matrix4_x_matrix4(&gr_model_view_matrix, &gr_view_matrix, &model_matrix);

	modelview_matrix_depth++;
}

void gr_start_angles_instance_matrix(const vec3d *pos, const angles *rotation)
{
	Assert(htl_view_matrix_set);

	matrix m;
	vm_angles_2_matrix(&m, rotation);

	gr_start_instance_matrix(pos, &m);
}

void gr_end_instance_matrix()
{
	Assert(htl_view_matrix_set);

	gr_model_matrix_stack.pop();

	auto model_matrix = gr_model_matrix_stack.get_transform();
	vm_matrix4_x_matrix4(&gr_model_view_matrix, &gr_view_matrix, &model_matrix);

	modelview_matrix_depth--;
}

// the projection matrix; fov, aspect ratio, near, far
void gr_set_proj_matrix(float fov, float aspect, float z_near, float z_far) {
	if (gr_screen.rendering_to_texture != -1) {
		gr_set_viewport(gr_screen.offset_x, gr_screen.offset_y, gr_screen.clip_width, gr_screen.clip_height);
	} else {
		gr_set_viewport(gr_screen.offset_x, (gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_height), gr_screen.clip_width, gr_screen.clip_height);
	}

	float clip_width, clip_height;

	clip_height = tan( fov * 0.5f ) * z_near;
	clip_width = clip_height * aspect;

	gr_last_projection_matrix = gr_projection_matrix;
	if (gr_screen.rendering_to_texture != -1) {
		create_perspective_projection_matrix(&gr_projection_matrix, -clip_width, clip_width, clip_height, -clip_height, z_near, z_far);
	} else {
		create_perspective_projection_matrix(&gr_projection_matrix, -clip_width, clip_width, -clip_height, clip_height, z_near, z_far);
	}

	gr_htl_projection_matrix_set = true;
}

void gr_end_proj_matrix() {
	gr_set_viewport(0, 0, gr_screen.max_w, gr_screen.max_h);

	gr_last_projection_matrix = gr_projection_matrix;

	// the top and bottom positions are reversed on purpose, but RTT needs them the other way
	if (gr_screen.rendering_to_texture != -1) {
		create_orthographic_projection_matrix(&gr_projection_matrix, 0.0f, i2fl(gr_screen.max_w), 0.0f, i2fl(gr_screen.max_h), -1.0f, 1.0f);
	} else {
		create_orthographic_projection_matrix(&gr_projection_matrix, 0.0f, i2fl(gr_screen.max_w), i2fl(gr_screen.max_h), 0.0f, -1.0f, 1.0f);
	}

	gr_htl_projection_matrix_set = false;
}

void gr_set_view_matrix(const vec3d *pos, const matrix *orient)
{
	Assert(modelview_matrix_depth == 1);

	gr_view_matrix = create_view_matrix(pos, orient);

	gr_model_matrix_stack.clear();
	gr_model_view_matrix = gr_view_matrix;

	if (Cmdline_env) {
		gr_env_texture_matrix_set = true;

		// setup the texture matrix which will make the the envmap keep lined
		// up properly with the environment

		// r.xyz  <--  r.x, u.x, f.x
		gr_env_texture_matrix.a1d[0] = gr_model_view_matrix.a1d[0];
		gr_env_texture_matrix.a1d[1] = gr_model_view_matrix.a1d[4];
		gr_env_texture_matrix.a1d[2] = gr_model_view_matrix.a1d[8];
		// u.xyz  <--  r.y, u.y, f.y
		gr_env_texture_matrix.a1d[4] = gr_model_view_matrix.a1d[1];
		gr_env_texture_matrix.a1d[5] = gr_model_view_matrix.a1d[5];
		gr_env_texture_matrix.a1d[6] = gr_model_view_matrix.a1d[9];
		// f.xyz  <--  r.z, u.z, f.z
		gr_env_texture_matrix.a1d[8] = gr_model_view_matrix.a1d[2];
		gr_env_texture_matrix.a1d[9] = gr_model_view_matrix.a1d[6];
		gr_env_texture_matrix.a1d[10] = gr_model_view_matrix.a1d[10];

		gr_env_texture_matrix.a1d[15] = 1.0f;
	}

	modelview_matrix_depth = 2;
	htl_view_matrix_set = true;
}

void gr_end_view_matrix()
{
	Assert(modelview_matrix_depth == 2);

	gr_model_matrix_stack.clear();
	vm_matrix4_set_identity(&gr_view_matrix);
	vm_matrix4_set_identity(&gr_model_view_matrix);

	modelview_matrix_depth = 1;
	htl_view_matrix_set = false;
	gr_env_texture_matrix_set = false;
}

// set a view and projection matrix for a 2D element
// TODO: this probably needs to accept values
void gr_set_2d_matrix(/*int x, int y, int w, int h*/)
{
	// don't bother with this if we aren't even going to need it
	if (!gr_htl_projection_matrix_set) {
		return;
	}

	Assert( htl_2d_matrix_set == 0 );
	Assert( htl_2d_matrix_depth == 0 );

	// the viewport needs to be the full screen size since glOrtho() is relative to it
	gr_set_viewport(0, 0, gr_screen.max_w, gr_screen.max_h);

	gr_last_projection_matrix = gr_projection_matrix;

	// the top and bottom positions are reversed on purpose, but RTT needs them the other way
	if (gr_screen.rendering_to_texture != -1) {
		create_orthographic_projection_matrix(&gr_projection_matrix, 0, i2fl(gr_screen.max_w), 0, i2fl(gr_screen.max_h), -1, 1);
	} else {
		create_orthographic_projection_matrix(&gr_projection_matrix, 0, i2fl(gr_screen.max_w), i2fl(gr_screen.max_h), 0, -1, 1);
	}

	matrix4 identity_mat;
	vm_matrix4_set_identity(&identity_mat);

	gr_model_matrix_stack.push_and_replace(identity_mat);

	gr_last_view_matrix = gr_view_matrix;
	gr_view_matrix = identity_mat;

	vm_matrix4_x_matrix4(&gr_model_view_matrix, &gr_view_matrix, &identity_mat);

	htl_2d_matrix_set = true;
	htl_2d_matrix_depth++;
}

// ends a previously set 2d view and projection matrix
void gr_end_2d_matrix()
{
	if (!htl_2d_matrix_set)
		return;

	Assert( htl_2d_matrix_depth == 1 );

	// reset viewport to what it was originally set to by the proj matrix
	gr_set_viewport(gr_screen.offset_x, (gr_screen.max_h - gr_screen.offset_y - gr_screen.clip_height), gr_screen.clip_width, gr_screen.clip_height);

	gr_projection_matrix = gr_last_projection_matrix;

	gr_model_matrix_stack.pop();

	gr_view_matrix = gr_last_view_matrix;

	auto model_matrix = gr_model_matrix_stack.get_transform();
	vm_matrix4_x_matrix4(&gr_model_view_matrix, &gr_view_matrix, &model_matrix);

	htl_2d_matrix_set = false;
	htl_2d_matrix_depth = 0;
}

static bool scale_matrix_set = false;

void gr_push_scale_matrix(const vec3d *scale_factor)
{
	if ( (scale_factor->xyz.x == 1) && (scale_factor->xyz.y == 1) && (scale_factor->xyz.z == 1) )
		return;

	scale_matrix_set = true;

	modelview_matrix_depth++;

	gr_model_matrix_stack.push(NULL, NULL, scale_factor);

	auto model_matrix = gr_model_matrix_stack.get_transform();
	vm_matrix4_x_matrix4(&gr_model_view_matrix, &gr_view_matrix, &model_matrix);
}

void gr_pop_scale_matrix()
{
	if (!scale_matrix_set)
		return;

	gr_model_matrix_stack.pop();

	auto model_matrix = gr_model_matrix_stack.get_transform();
	vm_matrix4_x_matrix4(&gr_model_view_matrix, &gr_view_matrix, &model_matrix);

	modelview_matrix_depth--;
	scale_matrix_set = false;
}
void gr_setup_viewport() {
	if (Gr_inited) {
		// This may be called by FRED before the gr system is actually initialized so we need to make sure that
		// this function call is actually valid at this point.
		gr_set_viewport(0, 0, gr_screen.max_w, gr_screen.max_h);
	}


	gr_last_projection_matrix = gr_projection_matrix;

	// the top and bottom positions are reversed on purpose, but RTT needs them the other way
	if (gr_screen.rendering_to_texture != -1) {
		create_orthographic_projection_matrix(&gr_projection_matrix, 0, i2fl(gr_screen.max_w), 0, i2fl(gr_screen.max_h), -1, 1);
	} else {
		create_orthographic_projection_matrix(&gr_projection_matrix, 0, i2fl(gr_screen.max_w), i2fl(gr_screen.max_h), 0, -1, 1);
	}
}
void gr_reset_matrices() {
	vm_matrix4_set_identity(&gr_projection_matrix);
	vm_matrix4_set_identity(&gr_last_projection_matrix);

	vm_matrix4_set_identity(&gr_view_matrix);
	vm_matrix4_set_identity(&gr_last_view_matrix);

	vm_matrix4_set_identity(&gr_model_view_matrix);
	gr_model_matrix_stack.clear();

	vm_matrix4_set_identity(&gr_texture_matrix);
}

void gr_set_texture_panning(float u, float v, bool enable) {
	vm_matrix4_set_identity(&gr_texture_matrix);
	if (enable) {
		gr_texture_matrix.a2d[3][0] = u;
		gr_texture_matrix.a2d[3][1] = v;
	}
}
