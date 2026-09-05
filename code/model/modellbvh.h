#pragma once

#include "globalincs/pstypes.h"
#include "globalincs/vmallocator.h"
#include "model/modelbvh.h" // reuses bvh_detail::ray_aabb_visit for the slab test

#include <cstdint>
#include <functional>

// Top-level BVH over a ship's own submodel bounding boxes, distinct from modelbvh.h's per-submodel
// triangle BVH. Binary (2-wide) nodes and a Morton-code (LBVH) build, not the SAH build the triangle
// module uses -- the item count here is always small (a ship's own n_models, realistically tens to a
// couple hundred) and this tree is meant to be thrown away and rebuilt every frame for an animated
// ship instance (see model_collide_get_submodel_bvh() in modelcollide.cpp), so build cost matters
// more than tree quality; SAH's cost-evaluation search isn't worth paying every frame at this scale.

struct lbvh_item {
	vec3d box_min, box_max; // in the ship's top-level (detail[0]) local frame -- see modelcollide.cpp
	int submodel_index = -1;
};

struct lbvh_node {
	vec3d min, max;
	int32_t left = -1, right = -1; // child node indices; both -1 means this is a leaf
	int32_t item = -1;             // valid only when left/right == -1 -- index into lbvh_tree::items
};

struct lbvh_tree {
	SCP_vector<lbvh_node> nodes;
	SCP_vector<lbvh_item> items; // kept in caller-supplied order, not sorted-by-Morton-code order
	int32_t root = -1;
};

// Builds an LBVH over the given items. Takes ownership (same convention as bvh_build()) since the
// items end up stored in the returned tree.
lbvh_tree lbvh_build(SCP_vector<lbvh_item> items);

// Visits every leaf item whose (radius-inflated) box the ray [origin, origin + dir*t_max] could
// reach. Mirrors bvh_visit_triangles()'s pruning contract: t_max is a mutable running bound the
// visitor may tighten to prune later subtrees (leave it untouched for an all-hits query). radius is
// 0.0f for a plain ray, the query sphere's radius for a sphereline query -- same convention as
// modelbvh.h's traversal.
void lbvh_visit(const lbvh_tree &tree, const vec3d &origin, const vec3d &dir, float &t_max, float radius,
	const std::function<void(int32_t submodel_index)> &visitor);
