#include "graphics/shadow_render_list.h"

#include "graphics/util/uniform_structs.h"
#include "graphics/matrix.h"
#include "graphics/shadows.h"
#include "model/model.h"
#include "model/modelrender.h"

extern float model_render_determine_depth(int obj_num, int model_num, const matrix* orient, const vec3d* pos, int detail_level_locked);
extern int model_render_determine_detail(float depth, int model_num, int detail_level_locked);

namespace {

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

void convert_shadow_material(graphics::shadow_uniform_data* data_out,
                             const matrix4& model_transform,
                             const vec3d& scale,
                             size_t transform_buffer_offset)
{
	matrix4 scaled_matrix = model_transform;

	scale_matrix(scaled_matrix, scale);

	data_out->modelMatrix = scaled_matrix;

	vm_matrix4_x_matrix4(&data_out->modelViewMatrix, &gr_view_matrix, &scaled_matrix);

	data_out->buffer_matrix_offset = (int) transform_buffer_offset;
}

} // namespace

void shadow_render_list::add_draw(const indexed_vertex_source* vert_src,
                                  vertex_buffer* buffer,
                                  size_t texi,
                                  const matrix4& model_matrix,
                                  const vec3d& scale,
                                  const clip_plane_info* clip)
{
	shadow_batch_entry entry;

	entry.model_matrix = model_matrix;
	entry.scale = scale;
	entry.transform_buffer_offset = _batchBuffer.get_buffer_offset();

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

	push_element(std::move(entry));
}

bool shadow_render_list::sort_draw_pair(int a, int b) const
{
	auto* draw_call_a = &_elements[a];
	auto* draw_call_b = &_elements[b];

	if (draw_call_a->flags != draw_call_b->flags) {
		return draw_call_a->flags < draw_call_b->flags;
	}

	return a < b;
}

void shadow_render_list::build_uniform_buffer()
{
	GR_DEBUG_SCOPE("Build shadow uniform buffer");

	_dataBuffer = gr_get_uniform_buffer(uniform_block_type::ShadowMapData, _keys.size());

	for (auto render_index : _keys) {
		auto& queued_draw = _elements[render_index];

		_lights.resetLightState();

		auto element = _dataBuffer.aligner().addTypedElement<graphics::shadow_uniform_data>();
		convert_shadow_material(element,
		                        queued_draw.model_matrix,
		                        queued_draw.scale,
		                        queued_draw.transform_buffer_offset);

		for (size_t i = 0; i < MAX_SHADOW_CASCADES; i++) {
			element->shadow_proj_matrix[i] = Shadow_proj_matrix[i];
		}

		element->clip_equation = queued_draw.clip_equation;
		element->use_clip_plane = queued_draw.has_clip_plane ? 1 : 0;

		queued_draw.uniform_buffer_offset = _dataBuffer.getCurrentAlignerOffset();
	}

	_dataBuffer.submitData();
}

void shadow_render_list::render_buffer(const shadow_batch_entry& entry)
{
	GR_DEBUG_SCOPE("Render shadow buffer");

	auto* datap = &entry.buffer->tex_buf[entry.texi];
	if (datap->n_verts == 0) {
		return;
	}

	gr_bind_uniform_buffer(uniform_block_type::ShadowMapData, entry.uniform_buffer_offset,
	                       sizeof(graphics::shadow_uniform_data), _dataBuffer.bufferHandle());

	gr_render_shadow_draw(_dataBuffer.bufferHandle(),
	                      entry.uniform_buffer_offset,
	                      sizeof(graphics::shadow_uniform_data),
	                      entry.buffer,
	                      const_cast<indexed_vertex_source*>(entry.vert_src),
	                      entry.texi);
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

	list->clear_transforms();
	list->push_transform(pos, orient);

	//TODO This might be incorrect for some models
	const vec3d scale_identity = SCALE_IDENTITY_VECTOR;

	matrix4 identity_4;
	vm_matrix4_set_identity(&identity_4);

	{
		list->start_model_batch(pm->n_models);

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

			list->add_draw(&pm->vert_source, &detail_buffer, j, identity_4, scale_identity, clip);
		}
	}

	int i = pm->submodel[detail_root].first_child;

	while (i >= 0) {
		if (!pm->submodel[i].flags[Model::Submodel_flags::Is_thruster]) {
			render_submodel_children(list, pm, pmi, i, clip);
		}

		i = pm->submodel[i].next_sibling;
	}

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

	if (smi != nullptr) {
		submodel_orient = smi->canonical_orient;
		vm_vec_add2(&submodel_offset, &smi->canonical_offset);
	}

	list->push_transform(&submodel_offset, &submodel_orient);

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
