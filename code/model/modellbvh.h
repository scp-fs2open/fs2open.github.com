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

// Every node has exactly two children -- the recursive build only ever creates a node to join two
// already-built subtrees, so there's no partial/padding slot to represent (unlike modelbvh.h's
// bvh_node, which pads unused slots of a wider fixed fan-out). Each slot's box is stored inline,
// mirroring bvh_node's per-slot boxes, so the slab test can reject a child before following it.
//
// A slot is either another node or a leaf naming a submodel directly, distinguished by the sign of
// child[i]: child[i] >= 0 is a node index into lbvh_tree::nodes; child[i] < 0 is a leaf, and its
// submodel index is -child[i] - 1 (the -1 shift avoids colliding with node index 0, since -0 == 0).
struct lbvh_node {
	vec3d min[2], max[2];
	int32_t child[2];
};

struct lbvh_tree {
	SCP_vector<lbvh_node> nodes;
	vec3d root_min, root_max; // whole-tree box; only nodes reached via a parent slot get one otherwise
	int32_t root = INT32_MIN; // INT32_MIN == empty (no items); otherwise same encoding as lbvh_node::child
};

// Builds an LBVH over the given items. Takes ownership (same convention as bvh_build()) since the
// items are consumed during construction.
lbvh_tree lbvh_build(SCP_vector<lbvh_item> items);

// Visits every leaf item whose (radius-inflated) box the ray [origin, origin + dir*t_max] could
// reach. Mirrors bvh_visit_triangles()'s pruning contract: t_max is a mutable running bound the
// visitor may tighten to prune later subtrees (leave it untouched for an all-hits query). radius is
// 0.0f for a plain ray, the query sphere's radius for a sphereline query -- same convention as
// modelbvh.h's traversal.
void lbvh_visit(const lbvh_tree &tree, const vec3d &origin, const vec3d &dir, float &t_max, float radius,
	const std::function<void(int32_t submodel_index)> &visitor);
