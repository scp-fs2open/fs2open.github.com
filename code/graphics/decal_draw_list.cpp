#include "graphics/decal_draw_list.h"

#include "graphics/matrix.h"

#include "render/3d.h"
#include "tracing/tracing.h"
#include "light.h"

namespace {

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
gr_buffer_handle decal_instance_buffer;

void init_buffers() {
	box_vertex_buffer = gr_create_buffer(BufferType::Vertex, BufferUsageHint::Static);
	gr_update_buffer_data(box_vertex_buffer, sizeof(BOX_VERTS), BOX_VERTS);

	box_index_buffer = gr_create_buffer(BufferType::Index, BufferUsageHint::Static);
	gr_update_buffer_data(box_index_buffer, sizeof(BOX_FACES), BOX_FACES);

	decal_instance_buffer = gr_create_buffer(BufferType::Vertex, BufferUsageHint::Streaming);
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

void decal_draw_list::globalInit() {
	init_buffers();

	gr_maybe_create_shader(SDR_TYPE_DECAL, 0);
	gr_maybe_create_shader(SDR_TYPE_DECAL, SDR_FLAG_DECAL_USE_NORMAL_MAP);
}
void decal_draw_list::globalShutdown() {
	gr_delete_buffer(box_vertex_buffer);
	gr_delete_buffer(box_index_buffer);
}

void decal_draw_list::prepare_global_data() {
	_buffer       = gr_get_uniform_buffer(uniform_block_type::DecalInfo, _draws.size());
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

	gr_get_ambient_light(&header->ambientLight);

	// Square the ambient part of the light to match the formula used in the main model shader
	header->ambientLight.xyz.x *= header->ambientLight.xyz.x;
	header->ambientLight.xyz.y *= header->ambientLight.xyz.y;
	header->ambientLight.xyz.z *= header->ambientLight.xyz.z;

	header->ambientLight.xyz.x += gr_light_emission[0];
	header->ambientLight.xyz.y += gr_light_emission[1];
	header->ambientLight.xyz.z += gr_light_emission[2];

	for (auto& [batch_info, draw_info] : _draws) {
		auto info = aligner.addTypedElement<graphics::decal_info>();
		info->diffuse_index = batch_info.diffuse < 0 ? -1 : bm_get_array_index(batch_info.diffuse);
		info->glow_index = batch_info.glow < 0 ? -1 : bm_get_array_index(batch_info.glow);
		info->normal_index = batch_info.normal < 0 ? -1 : bm_get_array_index(batch_info.normal);

		draw_info.first.uniform_offset = _buffer.getBufferOffset(aligner.getCurrentOffset());

		material_set_decal(&draw_info.first.material,
						   bm_get_base_frame(batch_info.diffuse),
						   bm_get_base_frame(batch_info.glow),
						   bm_get_base_frame(batch_info.normal));
		info->diffuse_blend_mode = draw_info.first.material.get_blend_mode(0) == ALPHA_BLEND_ADDITIVE ? 1 : 0;
		info->glow_blend_mode = draw_info.first.material.get_blend_mode(2) == ALPHA_BLEND_ADDITIVE ? 1 : 0;
	}
}

void decal_draw_list::render() {
	GR_DEBUG_SCOPE("Render decals");
	TRACE_SCOPE(tracing::RenderDecals);

	prepare_global_data();

	_buffer.submitData();

	vertex_layout layout;
	layout.add_vertex_component(vertex_format_data::POSITION3, sizeof(vec3d), 0);
	layout.add_vertex_component(vertex_format_data::MATRIX4, sizeof(matrix4), 0, 1, 1);

	indexed_vertex_source source;
	source.Vbuffer_handle = box_vertex_buffer;
	source.Ibuffer_handle = box_index_buffer;

	// Bind the global data only once
	gr_bind_uniform_buffer(uniform_block_type::DecalGlobals,
		_buffer.getBufferOffset(0),
		sizeof(graphics::decal_globals),
		_buffer.bufferHandle());
	gr_screen.gf_start_decal_pass();

	for (auto& [textures, decal_list] : _draws) {
		GR_DEBUG_SCOPE("Draw decal type");
		TRACE_SCOPE(tracing::RenderSingleDecal);

		gr_update_buffer_data(decal_instance_buffer, sizeof(matrix4) * decal_list.second.size(), decal_list.second.data());
		gr_bind_uniform_buffer(uniform_block_type::DecalInfo, decal_list.first.uniform_offset, sizeof(graphics::decal_info), _buffer.bufferHandle());
		gr_screen.gf_render_decals(&decal_list.first.material, PRIM_TYPE_TRIS, &layout, BOX_NUM_FACES, source, decal_instance_buffer, static_cast<int>(decal_list.second.size()));
	}

	gr_screen.gf_stop_decal_pass();
}
void decal_draw_list::add_decal(int diffuse_bitmap,
								int glow_bitmap,
								int normal_bitmap,
								float  /*decal_timer*/,
								const matrix4& instancedata) {
	if (!check_box_in_view(instancedata)) {
		// The decal box is not in view so we don't need to render it
		return;
	}

	_draws[decal_batch_info{diffuse_bitmap, glow_bitmap, normal_bitmap}].second.emplace_back(instancedata);
}

}
