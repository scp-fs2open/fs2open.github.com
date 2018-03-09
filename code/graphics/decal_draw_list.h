#pragma once

#include "globalincs/pstypes.h"

#include "graphics/material.h"
#include "graphics/util/UniformBuffer.h"

namespace graphics {

class decal_draw_list {
	struct decal_draw_info {
		decal_material draw_mat;

		size_t uniform_offset;
	};
	SCP_vector<decal_draw_info> _draws;

	util::UniformBuffer* _buffer = nullptr;

	static bool sort_draws(const decal_draw_info& left, const decal_draw_info& right);

 public:
	decal_draw_list();
	~decal_draw_list();

	decal_draw_list(const decal_draw_list&) = delete;
	decal_draw_list& operator=(const decal_draw_list&) = delete;

	void add_decal(int diffuse_bitmap,
				   int glow_bitmap,
				   int normal_bitmap,
				   float decal_timer,
				   const matrix4& transform,
				   float base_alpha);

	void render();

	static void globalInit();

	static void globalShutdown();
};

}
