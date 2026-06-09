#pragma once

#include "globalincs/pstypes.h"
#include "graphics/render_queue.h"
#include "graphics/2d.h"
#include "matrix.h"

class polymodel;
class polymodel_instance;

struct shadow_batch_entry {
	size_t uniform_buffer_offset = 0;
	size_t transform_buffer_offset = 0;
	bool has_clip_plane;
	vec4 clip_equation;
	matrix4 model_matrix;
	vec3d scale;
	int flags;
	const indexed_vertex_source* vert_src;
	vertex_buffer* buffer;
	size_t texi;
};

class shadow_render_list : public render_queue<shadow_render_list, shadow_batch_entry> {
	friend class render_queue<shadow_render_list, shadow_batch_entry>;
public:
	struct clip_plane_info {
		vec3d normal;
		vec3d position;
	};

	shadow_render_list() = default;
	~shadow_render_list() = default;

	void add_draw(const indexed_vertex_source* vert_src,
	              vertex_buffer* buffer,
	              size_t texi,
	              const matrix4& model_matrix,
	              const vec3d& scale,
	              const clip_plane_info* clip);

	static void add_model_draws(shadow_render_list* list,
	                            polymodel* pm,
	                            polymodel_instance* pmi,
	                            int obj_num,
	                            const vec3d* pos, const matrix* orient,
	                            const clip_plane_info* clip);

private:
	void build_uniform_buffer();
	void render_buffer(const shadow_batch_entry& entry);
	bool sort_draw_pair(int a, int b) const;

	void sort_draws() {}

	static void render_submodel_children(shadow_render_list* list,
	                                     polymodel* pm,
	                                     polymodel_instance* pmi,
	                                     int mn,
	                                     const clip_plane_info* clip);
};
