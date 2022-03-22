#include "graphics/decal_draw_list.h"

#include "graphics/util/uniform_structs.h"
#include "graphics/matrix.h"

#include "render/3d.h"
#include "tracing/tracing.h"
#include "light.h"

namespace {

// Discard any fragments where the angle to the direction to greater than 45°
const float DECAL_ANGLE_CUTOFF = fl_radians(45.f);
const float DECAL_ANGLE_FADE_START = fl_radians(30.f);

vec3d BOX_VERTS[] = {{{{ -0.5f, -0.5f, -0.5f }}},
					 {{{ -0.5f, 0.5f,  -0.5f }}},
					 {{{ 0.5f,  0.5f,  -0.5f }}},
					 {{{ 0.5f,  -0.5f, -0.5f }}},
					 {{{ -0.5f, -0.5f, 0.5f }}},
					 {{{ -0.5f, 0.5f,  0.5f }}},
					 {{{ 0.5f,  0.5f,  0.5f }}},
					 {{{ 0.5f,  -0.5f, 0.5f }}}};

uint32_t BOX_FACES[] =
	{ 7, 3, 4, 3, 0, 4, 2, 6, 1, 6, 5, 1, 7, 6, 3, 6, 2, 3, 0, 1, 4, 1, 5, 4, 6, 7, 4, 5, 6, 4, 3, 2, 0, 2, 1, 0, };

const size_t BOX_NUM_FACES = sizeof(BOX_FACES) / sizeof(BOX_FACES[0]);

gr_buffer_handle box_vertex_buffer;
gr_buffer_handle box_index_buffer;

void init_buffers() {
	box_vertex_buffer = gr_create_buffer(BufferType::Vertex, BufferUsageHint::Static);
	gr_update_buffer_data(box_vertex_buffer, sizeof(BOX_VERTS), BOX_VERTS);

	box_index_buffer = gr_create_buffer(BufferType::Index, BufferUsageHint::Static);
	gr_update_buffer_data(box_index_buffer, sizeof(BOX_FACES), BOX_FACES);
}

bool check_box_in_view(const matrix4& transform) {
	for (auto& point : BOX_VERTS) {
		vec3d pt;
		vm_vec_transform(&pt, &point, &transform, true);
		vec3d tmp;
		if (!g3_rotate_vector(&tmp, &pt)) {
			// This point lies in the view cone so we need to render it
			return true;
		}
	}

	return false;
}

}

namespace graphics {

/**
 * @brief Sorts Decals so that as many decals can be batched together as possible
 *
 * This uses the bitmaps in the definitions to determine if two decals can be rendered at the same time. Since we use
 * texture arrays we can use the base frame for batching which increases the number of draw calls that can be batched together.
 *
 * @param left
 * @param right
 * @return
 */
bool decal_draw_list::sort_draws(const decal_draw_info& left, const decal_draw_info& right) {
	auto left_diffuse_base = bm_get_base_frame(left.draw_mat.get_texture_map(TM_BASE_TYPE));
	auto right_diffuse_base = bm_get_base_frame(right.draw_mat.get_texture_map(TM_BASE_TYPE));

	if (left_diffuse_base != right_diffuse_base) {
		return left_diffuse_base < right_diffuse_base;
	}
	auto left_glow_base = bm_get_base_frame(left.draw_mat.get_texture_map(TM_GLOW_TYPE));
	auto right_glow_base = bm_get_base_frame(right.draw_mat.get_texture_map(TM_GLOW_TYPE));

	if (left_glow_base != right_glow_base) {
		return left_glow_base < right_glow_base;
	}

	auto left_normal_base = bm_get_base_frame(left.draw_mat.get_texture_map(TM_NORMAL_TYPE));
	auto right_normal_base = bm_get_base_frame(left.draw_mat.get_texture_map(TM_NORMAL_TYPE));

	return left_normal_base < right_normal_base;
}
void decal_draw_list::globalInit() {
	init_buffers();

	gr_maybe_create_shader(SDR_TYPE_DECAL, 0);
	gr_maybe_create_shader(SDR_TYPE_DECAL, SDR_FLAG_DECAL_USE_NORMAL_MAP);
}
void decal_draw_list::globalShutdown() {
	gr_delete_buffer(box_vertex_buffer);
	gr_delete_buffer(box_index_buffer);
}

decal_draw_list::decal_draw_list(size_t num_decals)
{
	_buffer       = gr_get_uniform_buffer(uniform_block_type::DecalInfo, num_decals);
	auto& aligner = _buffer.aligner();

	// Initialize header data
	auto header = aligner.getHeader<graphics::decal_globals>();
	matrix4 invView;
	vm_inverse_matrix4(&invView, &gr_view_matrix);

	header->viewMatrix = gr_view_matrix;
	header->projMatrix = gr_projection_matrix;
	header->invViewMatrix = invView;
	vm_inverse_matrix4(&header->invProjMatrix, &gr_projection_matrix);

	header->viewportSize.x = (float) gr_screen.max_w;
	header->viewportSize.y = (float) gr_screen.max_h;

	header->ambientLight.xyz.x = gr_light_ambient[0] + gr_user_ambient;
	header->ambientLight.xyz.y = gr_light_ambient[1] + gr_user_ambient;
	header->ambientLight.xyz.z = gr_light_ambient[2] + gr_user_ambient;

	CLAMP(header->ambientLight.xyz.x, 0.02f, 1.0f);
	CLAMP(header->ambientLight.xyz.y, 0.02f, 1.0f);
	CLAMP(header->ambientLight.xyz.z, 0.02f, 1.0f);

	// Square the ambient part of the light to match the formula used in the main model shader
	header->ambientLight.xyz.x *= header->ambientLight.xyz.x;
	header->ambientLight.xyz.y *= header->ambientLight.xyz.y;
	header->ambientLight.xyz.z *= header->ambientLight.xyz.z;

	header->ambientLight.xyz.x += gr_light_emission[0];
	header->ambientLight.xyz.y += gr_light_emission[1];
	header->ambientLight.xyz.z += gr_light_emission[2];
}
decal_draw_list::~decal_draw_list() {
}
void decal_draw_list::render() {
	GR_DEBUG_SCOPE("Render decals");
	TRACE_SCOPE(tracing::RenderDecals);

	_buffer.submitData();

	std::sort(_draws.begin(), _draws.end(), decal_draw_list::sort_draws);

	vertex_layout layout;
	layout.add_vertex_component(vertex_format_data::POSITION3, sizeof(vec3d), 0);

	indexed_vertex_source source;
	source.Vbuffer_handle = box_vertex_buffer;
	source.Ibuffer_handle = box_index_buffer;

	// Bind the global data only once
	gr_bind_uniform_buffer(uniform_block_type::DecalGlobals,
		_buffer.getBufferOffset(0),
		sizeof(graphics::decal_globals),
		_buffer.bufferHandle());
	gr_screen.gf_start_decal_pass();

	for (auto& draw : _draws) {
		GR_DEBUG_SCOPE("Draw single decal");
		TRACE_SCOPE(tracing::RenderSingleDecal);

		gr_bind_uniform_buffer(uniform_block_type::DecalInfo, draw.uniform_offset, sizeof(graphics::decal_info),
		                       _buffer.bufferHandle());

		gr_screen.gf_render_decals(&draw.draw_mat, PRIM_TYPE_TRIS, &layout, BOX_NUM_FACES, source);
	}

	gr_screen.gf_stop_decal_pass();
}
void decal_draw_list::add_decal(int diffuse_bitmap,
								int glow_bitmap,
								int normal_bitmap,
								float  /*decal_timer*/,
								const matrix4& transform,
								float base_alpha) {
	if (!check_box_in_view(transform)) {
		// The decal box is not in view so we don't need to render it
		return;
	}

	auto& aligner = _buffer.aligner();

	auto info = aligner.addTypedElement<graphics::decal_info>();
	info->model_matrix = transform;
	// This is currently a constant but in the future this may be configurable by the decals table
	info->normal_angle_cutoff = DECAL_ANGLE_CUTOFF;
	info->angle_fade_start = DECAL_ANGLE_FADE_START;
	info->alpha_scale = base_alpha;

	matrix transform_rot;
	vm_matrix4_get_orientation(&transform_rot, &transform);

	// The decal shader works in view-space so the direction also has to be transformed into that space
	vm_vec_transform(&info->decal_direction, &transform_rot.vec.fvec, &gr_view_matrix, false);

	vm_inverse_matrix4(&info->inv_model_matrix, &info->model_matrix);

	info->diffuse_index = diffuse_bitmap < 0 ? -1 : bm_get_array_index(diffuse_bitmap);
	info->glow_index = glow_bitmap < 0 ? -1 : bm_get_array_index(glow_bitmap);
	info->normal_index = normal_bitmap < 0 ? -1 : bm_get_array_index(normal_bitmap);

	decal_draw_info current_draw;
	current_draw.uniform_offset = _buffer.getBufferOffset(aligner.getCurrentOffset());

	material_set_decal(&current_draw.draw_mat,
					   bm_get_base_frame(diffuse_bitmap),
					   bm_get_base_frame(glow_bitmap),
					   bm_get_base_frame(normal_bitmap));
	info->diffuse_blend_mode = current_draw.draw_mat.get_blend_mode(0) == ALPHA_BLEND_ADDITIVE ? 1 : 0;
	info->glow_blend_mode = current_draw.draw_mat.get_blend_mode(2) == ALPHA_BLEND_ADDITIVE ? 1 : 0;

	_draws.push_back(current_draw);
}

}
