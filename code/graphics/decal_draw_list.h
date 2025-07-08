#pragma once

#include "globalincs/pstypes.h"

#include "graphics/material.h"
#include "graphics/util/uniform_structs.h"
#include "graphics/util/UniformBuffer.h"

namespace graphics {

class decal_draw_list {
	struct decal_draw_info {
		decal_material material;
		size_t uniform_offset;
	};
	struct decal_batch_info {
		int diffuse, glow, normal;
		bool operator< (const decal_batch_info& r) const {
			return std::tie(diffuse, glow, normal) < std::tie(r.diffuse, r.glow, r.normal);
		}
	};

	SCP_map<decal_batch_info, std::pair<decal_draw_info, SCP_vector<matrix4>>> _draws;

	util::UniformBuffer _buffer;
  public:
	decal_draw_list(const decal_draw_list&) = delete;
	decal_draw_list& operator=(const decal_draw_list&) = delete;

	void add_decal(int diffuse_bitmap, int glow_bitmap, int normal_bitmap, float decal_timer, const matrix4& instancedata);

	void render();

	static void globalInit();

	static void globalShutdown();

	decal_draw_list() = default;

private:
	void prepare_global_data();
};
}
