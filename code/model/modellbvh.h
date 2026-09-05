#pragma once

#include "globalincs/pstypes.h"
#include "globalincs/vmallocator.h"
#include "model/modelbvh.h" // reuses BVH_N and bvh_detail::ray_aabb_visit for the slab test

#include <cstdint>
#include <functional>

// Top-level BVH over a ship's own submodel bounding boxes, distinct from modelbvh.h's per-submodel
// triangle BVH. Morton-code (LBVH) built, not the SAH build the triangle module uses -- the item
// count here is always small (a ship's own n_models, realistically tens to a couple hundred) and
// this tree is meant to be thrown away and rebuilt every frame for an animated ship instance (see
// model_collide_get_submodel_bvh() in modelcollide.cpp), so build cost matters more than tree
// quality; SAH's cost-evaluation search isn't worth paying every frame at this scale.

struct lbvh_item {
	vec3d box_min, box_max; // in the ship's top-level (detail[0]) local frame -- see modelcollide.cpp
	int submodel_index = -1;
};

// Same BVH_N=4, Structure-of-Arrays shape as modelbvh.h's bvh_node (see that struct's own doc
// comment for the SIMD rationale), built by collapsing 2 binary LBVH split levels into each node
// instead of bvh_build()'s SAH cost search -- cheap enough to redo every frame.
//
// A slot is either another node or a leaf naming a submodel directly, distinguished by child[i]:
//   child[i] >= 0            -> internal: index of the child node in lbvh_tree::nodes.
//   child[i] < 0, != INT32_MIN -> leaf: submodel index is -child[i] - 1 (the -1 shift avoids
//                                  colliding with node index 0, since -0 == 0).
//   child[i] == INT32_MIN    -> unused padding slot (fewer than BVH_N real children).
// Unused slots are also given an impossible box (min > max, matching bvh_node's own padding
// convention) so the slab test fails them for free even if a caller ever skipped the child check.
struct lbvh_node {
	float minx[BVH_N], miny[BVH_N], minz[BVH_N];
	float maxx[BVH_N], maxy[BVH_N], maxz[BVH_N];
	int32_t child[BVH_N];
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
