#pragma once

#include "globalincs/pstypes.h"
#include "globalincs/vmallocator.h"
#include "model/modelbvh.h"

#include <cfloat>
#include <cstdint>

// A second, sibling BVH flavor to modelbvh.h's triangle-soup module (stage 1), for stage 3's
// engine integration: a SAH BVH over caller-supplied AABBs carrying an opaque int payload,
// instead of raw triangle geometry.
//
// Why a separate module rather than reusing bvh_build()/bvh_triangle: the live collision code's
// per-polygon test functions (mc_check_face/mc_check_sphereline_face in modelcollide.cpp) operate
// on original n-gon polygons via a point-in-polygon test, not triangles, and carry a lot of
// fragile behavior (MC_COLLIDE_ALL accumulation, sphere-line edge fallback, UV/bitmap/bsp_leaf
// output fields) that stage 3 deliberately does not touch. This module only replaces the spatial
// index (which leaf a ray's bounding box reaches); the polygon-level test stays wherever the
// caller already has it. Re-triangulating to reuse modelbvh.h's triangle-based traversal directly
// is deferred to a later stage (see collision_bvh_rewrite_plan project notes).
//
// This module reuses modelbvh.h's bvh_node/BVH_N (a generic SoA N-wide AABB node -- its
// child[]/count[] fields already just mean "start index + count into an external leaf-order
// array", not specifically bvh_triangle) but is otherwise independent: still zero dependency on
// model.h/bsp_data/POF types, same as modelbvh.h.

// One item to place in the tree: its AABB and an opaque payload the caller assigns meaning to
// (e.g. an index into a bsp_collision_tree's leaf_list).
struct bvh_leaf_primitive {
	vec3d bmin, bmax;
	int32_t payload = -1;
};

// Build output: same depth-first bvh_node layout as modelbvh.h's bvh_tree, but leaf slots'
// child[i]/count[i] index into `items` (reordered into leaf order by the build), not triangles.
struct bvh_leaf_tree {
	SCP_vector<bvh_node> nodes;
	SCP_vector<bvh_leaf_primitive> items;
	int root = 0;
};

// Builds a BVH over the given AABB+payload primitives using the same binary-SAH-build-then-
// collapse-then-flatten strategy as modelbvh.h's bvh_build(). Takes primitives by value for the
// same reason bvh_build() does: the builder reorders them internally.
bvh_leaf_tree bvh_build_leaves(SCP_vector<bvh_leaf_primitive> primitives);

namespace bvh_leafindex_detail {
bool ray_aabb_leafindex(const vec3d& origin, const vec3d& inv_dir, const float bmin[3], const float bmax[3], float t_max);
}

// Walks the tree, invoking visit(payload) for every item whose AABB the ray [origin, origin +
// dir*t_max] intersects. Does not track "nearest hit" or do any polygon-level testing itself --
// that stays with the caller, matching how the existing model_collide_bsp() traversal already
// visits every AABB-intersecting candidate rather than stopping at the first one (per-leaf
// "is this better than the current best" comparisons happen in the caller's own polygon test).
// Pass t_max = FLT_MAX for an unbounded ray (MC_CHECK_RAY-equivalent).
template <typename Visitor>
void bvh_visit_leaves(const bvh_leaf_tree& tree, const vec3d& origin, const vec3d& dir, float t_max, Visitor&& visit)
{
	if (tree.nodes.empty())
		return;

	vec3d inv_dir;
	inv_dir.xyz.x = dir.xyz.x != 0.0f ? 1.0f / dir.xyz.x : FLT_MAX;
	inv_dir.xyz.y = dir.xyz.y != 0.0f ? 1.0f / dir.xyz.y : FLT_MAX;
	inv_dir.xyz.z = dir.xyz.z != 0.0f ? 1.0f / dir.xyz.z : FLT_MAX;

	int32_t stack[64];
	int sp = 0;
	stack[sp++] = tree.root;

	while (sp > 0) {
		int32_t node_idx = stack[--sp];
		const bvh_node& node = tree.nodes[node_idx];

		for (int i = 0; i < BVH_N; ++i) {
			if (node.child[i] < 0)
				continue;

			float bmin[3] = {node.minx[i], node.miny[i], node.minz[i]};
			float bmax[3] = {node.maxx[i], node.maxy[i], node.maxz[i]};
			if (!bvh_leafindex_detail::ray_aabb_leafindex(origin, inv_dir, bmin, bmax, t_max))
				continue;

			if (node.count[i] > 0) {
				int start = node.child[i];
				int count = node.count[i];
				for (int j = start; j < start + count; ++j)
					visit(tree.items[j].payload);
			} else {
				// Explicit-stack traversal; a real submodel BVH is shallow (depth ~log_4(N)),
				// 64 slots is generous headroom.
				Assertion(sp < 64, "modelbvh_leafindex traversal stack overflow -- tree unexpectedly deep");
				stack[sp++] = node.child[i];
			}
		}
	}
}
