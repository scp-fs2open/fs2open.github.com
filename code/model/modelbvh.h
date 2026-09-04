#pragma once

#include "globalincs/pstypes.h"
#include "globalincs/vmallocator.h"

#include <algorithm>
#include <cstdint>

// Standalone SAH-built BVH over a flat triangle soup.
//
// This module is deliberately independent of the POF/BSP loading pipeline (no dependency on
// model.h, modelsinc.h, or bsp_data) so it can be built and tested against synthetic geometry.
// A later stage is responsible for extracting bvh_triangle arrays from real submodel geometry
// and wiring bvh_build()/traversal into the collision pipeline.

// Compile-time branching factor. Not a template parameter (yet) -- kept as a single constant so
// traversal/build code can be written generically over plain float[BVH_N] arrays and left to the
// autovectorizer. SSE2 (4-wide float) is the project's safe baseline (CI forces
// -DFORCED_SIMD_INSTRUCTIONS=SSE2 on Windows), hence N=4.
//
// Do not bump this to 8 for AVX without also retuning LEAF_THRESHOLD (modelbvh.cpp) in the same
// change: measured on real content, N=8 with LEAF_THRESHOLD left at 4 is a net loss (leaf padding
// roughly doubles the total padded triangle count, which outweighs the wider SIMD registers), and
// with LEAF_THRESHOLD raised to match it reshapes the tree and shifts leaf-visitation order enough
// to matter for the traversal-order-sensitive triangle-edge tie-break (see BARY_EPS below).
constexpr int BVH_N = 4;

// Minimal UV pair, kept local to this module rather than reusing model.h's uv_pair -- this header
// must stay independent of model.h (see file comment above).
struct bvh_uv {
	float u = 0.0f, v = 0.0f;
};

// One triangle of input geometry. Deliberately minimal and engine-agnostic: no submodel/version/
// texture-filename references, just the geometry plus a caller-defined material tag and a stable
// index back into whatever the caller's original array was, for traceability.
struct bvh_triangle {
	vec3d v0, v1, v2;
	int tmap_num = -1;
	int original_index = -1;
	int leaf_index = -1; // caller-defined provenance tag (e.g. source leaf at extraction time); -1 if unused
	bvh_uv uv0, uv1, uv2;
};

// One N-wide BVH node, Structure-of-Arrays so a ray-vs-N-children test is a handful of SIMD
// compares over plain float arrays with no transpose.
//
// Per slot i:
//   count[i] > 0  -> leaf: child[i] is the start index into bvh_tree::triangles, count[i] triangles.
//   count[i] == 0 -> internal: child[i] is the index of the child node in bvh_tree::nodes.
// Unused slots (fewer than BVH_N real children) are padded with an impossible box (min > max) so
// the SIMD slab test fails them for free, and child[i] == -1 so an accidental traversal into a
// padding slot is obvious rather than reading garbage.
//
// TODO(bvh-stage2): consider alignas(64)/an aligned allocator for bvh_node once hand SIMD
// intrinsics (which need aligned loads) are introduced. Not needed for autovectorization.
struct bvh_node {
	float minx[BVH_N], miny[BVH_N], minz[BVH_N];
	float maxx[BVH_N], maxy[BVH_N], maxz[BVH_N];
	int32_t child[BVH_N];
	int32_t count[BVH_N];
};

// Build output: a depth-first (pre-order) array of N-wide nodes, plus the input triangles
// reordered into leaf-contiguous (SAH build) order. Vertex positions are stored once each in a
// shared pool (vx/vy/vz), referenced per-triangle by index (i0/i1/i2) -- not duplicated per
// triangle, avoiding the extra storage duplicated-per-triangle vertices would cost. The pool is
// ordered by first reference when walking triangles in leaf order (see bvh_build()), so a leaf
// visit's index lookups tend to land on recently-touched, still-hot pool entries. [node.child[i] ..
// +count[i]) indexes every one of these parallel arrays for leaf slot i. Metadata that's only ever
// read once per accepted hit (never per SIMD lane) -- tmap_num, original_index, leaf_index, UVs --
// stays out of the hot index arrays but is still parallel-indexed the same way, for the same
// reason: one source of truth, no syncing.
struct bvh_tree {
	SCP_vector<bvh_node> nodes;

	SCP_vector<float> vx, vy, vz;
	SCP_vector<uint32_t> i0, i1, i2;
	SCP_vector<int> tmap_num;
	SCP_vector<int> original_index;
	SCP_vector<int> leaf_index;
	SCP_vector<bvh_uv> uv0, uv1, uv2;

	int root = 0;

	size_t triangle_count() const { return i0.size(); }

	vec3d vertex(uint32_t vi) const
	{
		vec3d v;
		v.xyz.x = vx[vi];
		v.xyz.y = vy[vi];
		v.xyz.z = vz[vi];
		return v;
	}

	// Reconstructs triangle i as a single bvh_triangle, for call sites where that's more convenient
	// than reading the parallel arrays directly (tests, the scalar bvh_ray_intersect() reference,
	// and one-off per-accepted-hit lookups) -- none of which are hot paths.
	bvh_triangle triangle_at(size_t i) const
	{
		bvh_triangle t;
		t.v0 = vertex(i0[i]);
		t.v1 = vertex(i1[i]);
		t.v2 = vertex(i2[i]);
		t.tmap_num = tmap_num[i];
		t.original_index = original_index[i];
		t.leaf_index = leaf_index[i];
		t.uv0 = uv0[i];
		t.uv1 = uv1[i];
		t.uv2 = uv2[i];
		return t;
	}
};

// Builds a BVH over the given triangle soup using a binary SAH build, greedily collapsed into
// BVH_N-wide nodes and flattened depth-first. Takes triangles by value: the builder reorders them
// internally, and taking ownership keeps the API unambiguous.
bvh_tree bvh_build(SCP_vector<bvh_triangle> triangles);

// Nearest-hit ray query against a built tree. Not the final optimized traversal (no SIMD, no
// explicit stack tuning) -- it exists so the build can be validated by ray queries in tests.
// Returns true and fills out_t/out_triangle_index on a hit; out_triangle_index indexes the tree's
// parallel triangle arrays (see triangle_at()).
bool bvh_ray_intersect(const bvh_tree& tree, const vec3d& origin, const vec3d& dir, float& out_t, int& out_triangle_index);

// Batched, SIMD-friendly nearest-hit ray-vs-triangle test over one leaf range [start, start+count)
// (as handed to a bvh_visit_triangles() visitor -- count is always a multiple of BVH_N). Processes
// BVH_N triangles per iteration reading directly from the tree's SoA vertex arrays (no transpose:
// that's the point of storing them this way), computing every lane's Moller-Trumbore result
// unconditionally (no early return per lane, unlike the scalar ray_triangle() this mirrors) so the
// loop stays a fixed-trip-count, branch-free-per-lane shape for the autovectorizer -- same "plain
// float[BVH_N] arrays, SSE2 baseline, no hand intrinsics" strategy bvh_node's own child-AABB test
// already uses. Degenerate padding triangles (see bvh_build()) always fail the |det| check and are
// never returned as a hit. The final "pick nearest valid lane" reduction is ordinary scalar control
// flow -- only the per-lane geometry math is written to vectorize.
// best_t bounds the search (e.g. an already-found candidate's t, or FLT_MAX); returns true and
// fills out_t/out_triangle_index (an index into the tree's parallel arrays) only for a strictly
// closer hit.
bool ray_triangle_leaf_simd(const bvh_tree& tree, int32_t start, int32_t count, const vec3d& origin, const vec3d& dir,
	float best_t, float& out_t, int32_t& out_triangle_index);

// Batched, SIMD-friendly nearest-hit sphere-line-vs-triangle test over one leaf range, mirroring
// ray_triangle_leaf_simd() above: same "plain float[BVH_N] arrays, every lane computed
// unconditionally" strategy, same scalar-gather-then-vector-math shape.
//
// NOT WIRED INTO THE LIVE COLLISION PATH -- correctness-tested (SphereTriangleLeafSimdTests in
// test_modelbvh.cpp) but slower on real ship geometry than the scalar path it was meant to replace
// (see collision_bvh_rewrite_plan memory note for benchmark numbers). Root cause: unlike
// ray-triangle's Moller-Trumbore, which is already near-flat-cost per triangle regardless of
// outcome, the scalar reference this batches (fvi_sphere_plane() + fvi_polyedge_sphereline() via
// mc_check_triangle_sphereline_face()) has a cheap early-exit that matters a lot in practice --
// fvi_sphere_plane() failing skips the entire expensive edge test, which is the common case for most
// triangles in an AABB-pruned leaf. "Every lane computed unconditionally" throws that away: this
// function pays the full 3-edge quadratic-plus-vertex-fallback cost for every triangle in every
// leaf, where the scalar path usually bails out after one cheap plane test. Kept in the tree as a
// validated (correct, just not fast) building block for a future redesign with a cheap per-chunk
// pre-gate (e.g. a fvi_sphere_plane-only pass to skip the edge lambda for chunks where no lane needs
// it), not because it's currently useful as-is.
//
// A moving sphere against a triangle is the classic "sphere at each vertex, cylinder along each
// edge, slab for the face" case (see e.g. Real-Time Rendering's collision-detection chapter), and
// this function reproduces FS2's own existing scalar algorithm for that -- fvi_sphere_plane() for
// the face/slab test, then fvi_polyedge_sphereline()'s edge-cylinder quadratic (with its
// vertex-sphere fallback) for the three edges -- batched across BVH_N triangles per iteration
// instead of one triangle at a time. It is a faithful (not approximate) reproduction of that scalar
// math: every branch in the scalar version (the two Hit/TryVertex fallbacks in
// fvi_polyedge_sphereline, the on-face/edge dispatch in mc_check_triangle_sphereline_face())
// becomes an unconditional per-lane bool combined with && / || instead of goto, so the result
// should agree with the scalar reference bit-for-bit modulo floating-point reassociation -- which
// is exactly why a caller would still need to re-confirm with the real scalar function rather than
// trusting this function's fields directly, same as ray_triangle_leaf_simd()'s caller does.
//
// radius is the sphere radius (Mc->radius); this function is not meant to be used for MC_COLLIDE_ALL
// queries, which need every hit, not just the nearest, same as ray_triangle_leaf_simd() is skipped
// for MC_COLLIDE_ALL. best_t bounds the search; returns true and fills out_t/out_triangle_index only
// for a strictly closer candidate.
bool sphere_triangle_leaf_simd(const bvh_tree& tree, int32_t start, int32_t count, const vec3d& sphere_p0,
	const vec3d& sphere_dir, float radius, float best_t, float& out_t, int32_t& out_triangle_index);

// The ray-vs-AABB slab test and its radius-inflated wrapper live here (inline), not in modelbvh.cpp,
// specifically so they can inline into bvh_visit_triangles()'s hot per-node-slot loop below -- this
// is the single most-executed operation in the traversal, so the header-inlining tradeoff is worth
// it here specifically.
namespace bvh_detail {
inline bool ray_aabb(const vec3d& origin, const vec3d& inv_dir, const float bmin[3], const float bmax[3], float t_max,
	float& out_tmin)
{
	float o[3] = {origin.xyz.x, origin.xyz.y, origin.xyz.z};
	float id[3] = {inv_dir.xyz.x, inv_dir.xyz.y, inv_dir.xyz.z};

	float tmin = 0.0f;
	float tmax = t_max;
	for (int axis = 0; axis < 3; ++axis) {
		float t0 = (bmin[axis] - o[axis]) * id[axis];
		float t1 = (bmax[axis] - o[axis]) * id[axis];
		if (t0 > t1)
			std::swap(t0, t1);
		tmin = std::max(tmin, t0);
		tmax = std::min(tmax, t1);
		if (tmin > tmax)
			return false;
	}
	out_tmin = tmin;
	return true;
}

inline bool ray_aabb_visit(const vec3d& origin, const vec3d& inv_dir, const float bmin[3], const float bmax[3],
	float t_max, float radius)
{
	// Inflate by `radius` before delegating to the shared ray_aabb() slab test (also used,
	// radius-free, by the scalar bvh_ray_intersect() reference) -- see the radius parameter's
	// doc comment on bvh_visit_triangles() below for why this exists.
	float inflated_min[3] = {bmin[0] - radius, bmin[1] - radius, bmin[2] - radius};
	float inflated_max[3] = {bmax[0] + radius, bmax[1] + radius, bmax[2] + radius};
	float ignored_tmin;
	return ray_aabb(origin, inv_dir, inflated_min, inflated_max, t_max, ignored_tmin);
}
} // namespace bvh_detail

// Walks the tree, invoking visit(start, count, t_max) for every leaf whose AABB the ray
// [origin, origin + dir*t_max] intersects, where [start, start+count) indexes the tree's parallel
// triangle arrays (see bvh_tree::triangle_at()). A whole leaf range is handed to the visitor at
// once (not one triangle at a time) so a caller can batch-test the leaf's triangles with SIMD (see
// ray_triangle_leaf_simd() below). Every leaf's count is a multiple of BVH_N (see bvh_build()'s
// padding step), with any padding triangles guaranteed to be degenerate (zero-area) and never a
// valid hit. Pass t_max = FLT_MAX for an unbounded ray (MC_CHECK_RAY-equivalent).
// `radius` inflates every AABB test by that amount on every axis before testing -- pass
// Mc->radius for an MC_CHECK_SPHERELINE query, 0.0f for a plain ray. Without this, a sphere query
// would silently miss any leaf its swept volume would reach but the bare centerline ray-segment
// doesn't pass through.
// The visitor receives t_max by mutable reference and may shrink it (e.g. to the closest hit found
// so far, for a nearest-hit-only query) -- every AABB test after that point uses the tightened
// value, pruning subtrees that can no longer contain a closer hit. A visitor that needs every hit
// (MC_COLLIDE_ALL) simply never writes to it, leaving traversal unpruned.
template <typename Visitor>
void bvh_visit_triangles(const bvh_tree& tree, const vec3d& origin, const vec3d& dir, float t_max, float radius,
	Visitor&& visit)
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
			if (!bvh_detail::ray_aabb_visit(origin, inv_dir, bmin, bmax, t_max, radius))
				continue;

			if (node.count[i] > 0) {
				visit(node.child[i], node.count[i], t_max);
			} else {
				Assertion(sp < 64, "modelbvh triangle traversal stack overflow -- tree unexpectedly deep");
				stack[sp++] = node.child[i];
			}
		}
	}
}
