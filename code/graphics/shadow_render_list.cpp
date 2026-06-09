#include "graphics/shadow_render_list.h"

#include "graphics/grinternal.h"
#include "graphics/matrix.h"
#include "graphics/shadows.h"
#include "model/model.h"
#include "model/modelrender.h"
#include "render/3d.h"

extern float model_render_determine_depth(int obj_num, int model_num, const matrix* orient, const vec3d* pos, int detail_level_locked);
extern int model_render_determine_detail(float depth, int model_num, int detail_level_locked);
extern model_batch_buffer TransformBufferHandler;

namespace {

// Exact replica of scale_matrix from code/graphics/uniforms.cpp lines 15-27
void scale_matrix(matrix4& mat, const vec3d& scale)
{
	mat.a2d[0][0] *= scale.xyz.x;
	mat.a2d[0][1] *= scale.xyz.x;
	mat.a2d[0][2] *= scale.xyz.x;

	mat.a2d[1][0] *= scale.xyz.y;
	mat.a2d[1][1] *= scale.xyz.y;
	mat.a2d[1][2] *= scale.xyz.y;

	mat.a2d[2][0] *= scale.xyz.z;
	mat.a2d[2][1] *= scale.xyz.z;
	mat.a2d[2][2] *= scale.xyz.z;
}

// Exact mirror of graphics::uniforms::convert_model_material (lines 33-189),
// with fields not present in shadow_uniform_data removed.
// Line references in comments map to the original function in code/graphics/uniforms.cpp.
void convert_shadow_material(graphics::shadow_uniform_data* data_out,
                             const matrix4& model_transform,
                             const vec3d& scale,
                             size_t transform_buffer_offset)
{
	// Line 43: matrix4 scaled_matrix = model_transform;
	matrix4 scaled_matrix = model_transform;
	// Line 44: scale_matrix(scaled_matrix, scale);
	scale_matrix(scaled_matrix, scale);

	// Line 46: data_out->modelMatrix = scaled_matrix;
	data_out->modelMatrix = scaled_matrix;
	// Lines 47-48: (viewMatrix removed) vm_matrix4_x_matrix4(&data_out->modelViewMatrix, &gr_view_matrix, &scaled_matrix);
	vm_matrix4_x_matrix4(&data_out->modelViewMatrix, &gr_view_matrix, &scaled_matrix);

	// Lines 49-50: projMatrix, textureMatrix — not in shadow_uniform_data

	// Lines 52-66: color, vpwidth/height, effect, anim_timer, flags — not in shadow_uniform_data

	// Lines 68-73: texture indices — not in shadow_uniform_data

	// Lines 75-86: clip_equation, use_clip_plane — managed in build_uniform_buffer from batch_entry

	// Lines 88-110: lighting — not in shadow_uniform_data

	// Line 111: defaultGloss — not in shadow_uniform_data

	// Lines 113-166: texture map indices, blend, gammaSpec, alphaGloss — not in shadow_uniform_data

	// Lines 168-179: shadow_proj_matrix — filled in build_uniform_buffer (applied globally)

	// Line 181-182: (no is_batched guard) data_out->buffer_matrix_offset = (int) transform_buffer_offset;
	data_out->buffer_matrix_offset = (int) transform_buffer_offset;

	// Lines 185+: team colors — not in shadow_uniform_data
}

} // namespace

shadow_render_list::shadow_render_list()
{
}

shadow_render_list::~shadow_render_list()
{
}

void shadow_render_list::reset()
{
	_render_elements.clear();
	_render_keys.clear();
}

void shadow_render_list::add_draw(const indexed_vertex_source* vert_src,
                                  vertex_buffer* buffer,
                                  size_t texi,
                                  const matrix4& model_matrix,
                                  const vec3d& scale,
                                  size_t transform_buffer_offset,
                                  const clip_plane_info* clip)
{
	batch_entry entry;

	// Mirror of add_buffer_draw lines 610-614 (non-batched path)
	entry.model_matrix = model_matrix;
	entry.scale = scale;
	entry.transform_buffer_offset = transform_buffer_offset;

	// Mirror of add_buffer_draw lines 616-622
	entry.flags = 0;
	entry.vert_src = vert_src;
	entry.buffer = buffer;
	entry.texi = texi;

	entry.has_clip_plane = (clip != nullptr);
	if (clip != nullptr) {
		entry.clip_equation.xyzw.x = clip->normal.xyz.x;
		entry.clip_equation.xyzw.y = clip->normal.xyz.y;
		entry.clip_equation.xyzw.z = clip->normal.xyz.z;
		entry.clip_equation.xyzw.w = -vm_vec_dot(&clip->normal, &clip->position);
	} else {
		entry.clip_equation.xyzw.x = 0.0f;
		entry.clip_equation.xyzw.y = 0.0f;
		entry.clip_equation.xyzw.z = 0.0f;
		entry.clip_equation.xyzw.w = 0.0f;
	}

	// Mirror of add_buffer_draw lines 624-625
	_render_elements.push_back(entry);
	_render_keys.push_back((int)(_render_elements.size() - 1));
}

// Mirror of model_draw_list::start_model_batch line 532-535
void shadow_render_list::start_model_batch(int n_models)
{
	TransformBufferHandler.set_num_models(n_models);
}

// Mirror of model_draw_list::add_submodel_to_batch lines 537-551
void shadow_render_list::add_submodel_to_batch(int model_num)
{
	matrix4 transform;

	transform = _transform_stack.get_transform();

	// set scale — mirror of lines 544-546
	const vec3d scale_identity = SCALE_IDENTITY_VECTOR;
	vm_vec_scale(&transform.vec.rvec, scale_identity.xyz.x);
	vm_vec_scale(&transform.vec.uvec, scale_identity.xyz.y);
	vm_vec_scale(&transform.vec.fvec, scale_identity.xyz.z);

	// set visibility — mirror of line 549
	transform.a1d[15] = 0.0f;

	TransformBufferHandler.set_model_transform(transform, model_num);
}

// Mirror of model_draw_list::sort_draw_pair — simplified since all draws use the same shader
bool shadow_render_list::sort_draw_pair(const shadow_render_list* target, const int a, const int b)
{
	auto* draw_call_a = &target->_render_elements[a];
	auto* draw_call_b = &target->_render_elements[b];

	if (draw_call_a->flags != draw_call_b->flags) {
		return draw_call_a->flags < draw_call_b->flags;
	}

	return a < b;
}

// Mirror of model_draw_list::sort_draws lines 526-530 — no-op allowed deviation
void shadow_render_list::sort_draws()
{
	// All shadow draws use the same shader — sorting is a no-op
}

// Mirror of model_draw_list::build_uniform_buffer lines 881-911
void shadow_render_list::build_uniform_buffer()
{
	GR_DEBUG_SCOPE("Build shadow uniform buffer");

	_dataBuffer = gr_get_uniform_buffer(uniform_block_type::ShadowMapData, _render_keys.size());

	for (auto render_index : _render_keys) {
		auto& queued_draw = _render_elements[render_index];

		// Mirror of lines 892-897: lighting setup (SIMPLIFIED - shadow materials are always unlit)
		_scene_light_handler.resetLightState();

		auto element = _dataBuffer.aligner().addTypedElement<graphics::shadow_uniform_data>();
		// Mirror of lines 900-904
		convert_shadow_material(element,
		                        queued_draw.model_matrix,
		                        queued_draw.scale,
		                        queued_draw.transform_buffer_offset);

		// Mirror of lines 168-179: shadow_proj_matrix
		for (size_t i = 0; i < MAX_SHADOW_CASCADES; i++) {
			element->shadow_proj_matrix[i] = Shadow_proj_matrix[i];
		}

		// Mirror of lines 75-86: clip_equation, use_clip_plane
		element->clip_equation = queued_draw.clip_equation;
		element->use_clip_plane = queued_draw.has_clip_plane ? 1 : 0;

		// Mirror of line 905
		queued_draw.uniform_buffer_offset = _dataBuffer.getCurrentAlignerOffset();
	}

	// Mirror of line 910
	_dataBuffer.submitData();
}

// Mirror of model_draw_list::init_render lines 696-707
void shadow_render_list::init_render(bool sort)
{
	if (sort) {
		sort_draws();
	}

	// Mirror of line 702: TransformBufferHandler.submit_buffer_data();
	TransformBufferHandler.submit_buffer_data();

	build_uniform_buffer();

	_render_initialized = true;
}

// Mirror of model_draw_list::render_buffer lines 628-637
void shadow_render_list::render_buffer(size_t render_index)
{
	GR_DEBUG_SCOPE("Render shadow buffer");

	auto& render_elements = _render_elements[render_index];

	auto* datap = &render_elements.buffer->tex_buf[render_elements.texi];
	if (datap->n_verts == 0) {
		return;
	}

	gr_bind_uniform_buffer(uniform_block_type::ShadowMapData, render_elements.uniform_buffer_offset,
	                       sizeof(graphics::shadow_uniform_data), _dataBuffer.bufferHandle());

	gr_render_shadow_draw(_dataBuffer.bufferHandle(),
	                      render_elements.uniform_buffer_offset,
	                      sizeof(graphics::shadow_uniform_data),
	                      render_elements.buffer,
	                      const_cast<indexed_vertex_source*>(render_elements.vert_src),
	                      render_elements.texi);
}

// Mirror of model_draw_list::render_all lines 709-727
void shadow_render_list::render_all()
{
	GR_DEBUG_SCOPE("Render shadow draw list");

	Assertion(_render_initialized, "init_render must be called before any render_all call!");

	_scene_light_handler.resetLightState();

	for (size_t i = 0; i < _render_keys.size(); ++i) {
		int render_index = _render_keys[i];

		render_buffer(render_index);
	}

	gr_alpha_mask_set(0, 1.0f);
}

void shadow_render_list::push_transform(const vec3d* pos, const matrix* orient)
{
	_transform_stack.push(pos, orient);
}

void shadow_render_list::pop_transform()
{
	_transform_stack.pop();
}

const matrix4& shadow_render_list::get_current_transform() const
{
	return _transform_stack.get_transform();
}

void shadow_render_list::add_model_draws(shadow_render_list* list,
                                         polymodel* pm,
                                         polymodel_instance* pmi,
                                         int obj_num,
                                         const vec3d* pos, const matrix* orient,
                                         const clip_plane_info* clip)
{
	float depth = model_render_determine_depth(obj_num, pm->id, orient, pos, -1);
	int detail_level = model_render_determine_detail(depth, pm->id, -1);
	int detail_root = pm->detail[detail_level];

	list->_transform_stack.clear();
	list->push_transform(pos, orient);

	const vec3d scale_identity = SCALE_IDENTITY_VECTOR;

	// Mirror of add_buffer_draw line 600: vm_matrix4_set_identity(&draw_data.transform);
	matrix4 identity_4;
	vm_matrix4_set_identity(&identity_4);

	// Mirror of model_render_queue lines 3001-3004: batched detail buffer
	{
		list->start_model_batch(pm->n_models);
		size_t batch_offset = TransformBufferHandler.get_buffer_offset();

		auto& detail_buffer = pm->detail_buffers[detail_level];
		for (size_t j = 0; j < detail_buffer.tex_buf.size(); j++) {
			if (detail_buffer.tex_buf[j].n_verts == 0) {
				continue;
			}

			int tmap_num = detail_buffer.tex_buf[j].texture;
			int base_tex = pm->maps[tmap_num].textures[TM_BASE_TYPE].GetTexture();

			if (base_tex < 0) {
				continue;
			}

			list->add_draw(&pm->vert_source, &detail_buffer, j, identity_4, scale_identity, batch_offset, clip);
		}
	}

	// Mirror of model_render_queue lines 3006-3019: walk children
	int i = pm->submodel[detail_root].first_child;

	while (i >= 0) {
		if (!pm->submodel[i].flags[Model::Submodel_flags::Is_thruster]) {
			render_submodel_children(list, pm, pmi, i, clip);
		}

		i = pm->submodel[i].next_sibling;
	}

	// Mirror of model_render_queue lines 3026-3038 hull render:
	// model_render_buffers with mn=detail_model_num calls add_submodel_to_batch
	// and returns (no draw calls since batching is active).
	list->add_submodel_to_batch(detail_root);

	list->pop_transform();
}

void shadow_render_list::render_submodel_children(shadow_render_list* list,
                                                  polymodel* pm,
                                                  polymodel_instance* pmi,
                                                  int mn,
                                                  const clip_plane_info* clip)
{
	bsp_info* sm = &pm->submodel[mn];
	submodel_instance* smi = nullptr;

	if (pmi != nullptr) {
		smi = &pmi->submodel[mn];
		if (smi->blown_off) {
			return;
		}
	}

	matrix submodel_orient = vmd_identity_matrix;
	vec3d submodel_offset = sm->offset;

	const vec3d scale_identity = SCALE_IDENTITY_VECTOR;

	if (smi != nullptr) {
		submodel_orient = smi->canonical_orient;
		vm_vec_add2(&submodel_offset, &smi->canonical_offset);
	}

	list->push_transform(&submodel_offset, &submodel_orient);

	// Mirror of model_render_children_buffers: add this submodel to the transform batch
	list->add_submodel_to_batch(mn);

	// Recurse into children
	int i = sm->first_child;
	while (i >= 0) {
		if (!pm->submodel[i].flags[Model::Submodel_flags::Is_thruster]) {
			render_submodel_children(list, pm, pmi, i, clip);
		}

		i = pm->submodel[i].next_sibling;
	}

	list->pop_transform();
}
