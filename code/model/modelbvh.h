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

// Compile-time branching factor: how many children one bvh_node holds, and (via emit_node()'s
// greedy collapse in modelbvh.cpp) how many binary-tree levels get merged into one node. This is a
// memory-locality knob, not a compute-vectorization one -- see bvh_node's own doc comment below for
// why: the per-node box test is an ordinary scalar loop over these BVH_N slots, not a vectorized
// compare across them. SIMD_WIDTH (below) is the separate constant for that.
constexpr int BVH_N = 4;

// Compile-time SIMD batch width: how many triangles ray_triangle_leaf_simd() (below) processes per
// vectorized pass, and what bvh_build() pads every leaf's triangle range up to a multiple of so that
// pass never has a ragged remainder. Kept as a plain constant, not a template parameter, so the
// arithmetic stays plain float[SIMD_WIDTH] arrays left to the autovectorizer. SSE2 (4-wide float) is
// the project's safe baseline (CI forces -DFORCED_SIMD_INSTRUCTIONS=SSE2 on Windows), hence 4.
//
// Independent of BVH_N -- currently the same value by coincidence, not by any coupling in the code
// (same relationship as LEAF_THRESHOLD's independence from BVH_N, see that constant's own comment).
// Do not bump this to 8 for AVX without separately retuning LEAF_THRESHOLD (modelbvh.cpp) if leaf
// *shape* also needs to change: measured on real content, moving leaf-intersection batching to 8
// wide while LEAF_THRESHOLD stayed at 4 was a net loss (leaf padding roughly doubles the total
// padded triangle count, which outweighs the wider SIMD registers), and raising LEAF_THRESHOLD to
// match reshapes the tree and shifts leaf-visitation order enough to matter for the
// traversal-order-sensitive triangle-edge tie-break (see BARY_EPS below).
constexpr int SIMD_WIDTH = 4;

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

// One N-wide BVH node, Structure-of-Arrays. The benefit this actually delivers is memory
// locality, not vectorized compute: one node fetch (128 bytes at BVH_N=4, 2 cache lines) brings
// every child's box and child[]/count[] metadata into cache at once, turning "which of these
// children are worth pursuing" into a single memory transaction instead of the several scattered
// node fetches an equivalent binary tree would need to make the same decision. The traversal's
// per-node box test (bvh_visit_triangles() below) is a plain scalar loop over the BVH_N slots, not
// a vectorized compare across them -- a genuine 4-wide rewrite of that test was tried and measured
// slower on real content (SIMD across sibling children turned out not to pay off here), so it stays
// scalar; don't re-attempt this blind. SoA still earns its keep elsewhere: ray_triangle_leaf_simd()
// (below) does genuinely vectorize, across SIMD_WIDTH triangles at a time -- a different array, and
// a different constant, than this struct's own BVH_N.
//
// Per slot i:
//   count[i] > 0  -> leaf: child[i] is the start index into bvh_tree::triangles, count[i] triangles.
//   count[i] == 0 -> internal: child[i] is the index of the child node in bvh_tree::nodes.
// Unused slots (fewer than BVH_N real children) are padded with an impossible box (min > max) so
// the slab test fails them for free, and child[i] == -1 so an accidental traversal into a padding
// slot is obvious rather than reading garbage.
//
// TODO(bvh-stage2): consider alignas(64)/an aligned allocator for bvh_node once hand SIMD
// intrinsics (which need aligned loads) are introduced. Not needed for autovectorization.
struct bvh_node {
	float minx[BVH_N], miny[BVH_N], minz[BVH_N];
	float maxx[BVH_N], maxy[BVH_N], maxz[BVH_N];
	int32_t child[BVH_N];
	int32_t count[BVH_N];
};

// One triangle's vertex-pool indices, interleaved rather than three parallel arrays -- i0/i1/i2 of
// a given triangle are always fetched together (never independently), so keeping them adjacent
// means one 12-byte read instead of three reads to three unrelated arrays.
struct bvh_tri_indices {
	uint32_t i0, i1, i2;
};

// Build output: a depth-first (pre-order) array of N-wide nodes, plus the input triangles
// reordered into leaf-contiguous (SAH build) order. Vertex positions are stored once each in a
// shared pool (verts), referenced per-triangle by index (tris) -- not duplicated per triangle,
// avoiding the extra storage duplicated-per-triangle vertices would cost. Both are interleaved
// (vec3d / bvh_tri_indices), not split into per-component arrays: a vertex's x/y/z (or a
// triangle's three indices) are never read independently of each other, so splitting them into
// separate arrays would only turn one small contiguous read into several unrelated ones. The
// SIMD leaf test still assembles its own per-component float[SIMD_WIDTH] arrays from these -- that's a
// destination-side shape for vectorized math, not a reason to store the source that way, since
// gathering by index requires a per-lane scalar read either way (no HW gather on this project's
// SIMD baseline). The pool is ordered by first reference when walking triangles in leaf order (see
// bvh_build()), so a leaf visit's index lookups tend to land on recently-touched, still-hot pool
// entries. [node.child[i] .. +count[i]) indexes every one of these parallel arrays for leaf slot i.
// Metadata that's only ever read once per accepted hit (never per SIMD lane) -- tmap_num,
// original_index, leaf_index, UVs -- stays out of the hot arrays but is still parallel-indexed the
// same way, for the same reason: one source of truth, no syncing.
//
// `normal` is the one exception to "only read once per accepted hit": the scalar sphereline/ray
// face tests (modelcollide.cpp) need a triangle's own unit face normal on *every* triangle they
// test, not just accepted hits (it's needed for the backface cull before a hit is even known). A
// triangle's local-space geometry never changes between queries (only the submodel's transform,
// applied to the query ray/sphere, not the triangle), so recomputing cross(e1,e2)+normalize --
// including a real sqrt -- from scratch on every single query was pure repeated work; precomputed
// once here at build time instead. Zero vector marks a degenerate (zero-area) triangle, matching
// vm_vec_normalize_safe(..., true)'s own fallback convention -- callers check magnitude, not a
// separate flag.
// Per-triangle bounding sphere (centroid + max vertex distance from it -- a valid, if not minimal,
// bound), for the leaf-level reject in mc_check_bvh_triangle_candidate() (modelcollide.cpp). The BVH
// already prunes at *leaf* granularity (LEAF_THRESHOLD triangles at a time); this rejects individual
// triangles inside an already-visited leaf, cheaper (no sqrt/divide) than running the full plane test
// on ones nowhere near the query, and skips the vertex-pool gather entirely for rejected triangles.
// Measured on the real 219-POF corpus as part of the front-to-back-traversal batch (see
// COLLISION_BVH_NOTES.md): a real, if smaller, contributor on its own (~8% faster in isolation).
struct bvh_bsphere {
	vec3d center;
	float radius = 0.0f;
};

struct bvh_tree {
	SCP_vector<bvh_node> nodes;

	SCP_vector<vec3d> verts;
	SCP_vector<bvh_tri_indices> tris;
	SCP_vector<int> tmap_num;
	SCP_vector<int> original_index;
	SCP_vector<int> leaf_index;
	SCP_vector<bvh_uv> uv0, uv1, uv2;
	SCP_vector<vec3d> normal; // precomputed unit face normal per triangle -- see doc comment above
	SCP_vector<bvh_bsphere> bsphere; // precomputed per-triangle bounding sphere -- see doc comment above

	int root = 0;

	size_t triangle_count() const { return tris.size(); }

	vec3d vertex(uint32_t vi) const { return verts[vi]; }

	// Reconstructs triangle i as a single bvh_triangle, for call sites where that's more convenient
	// than reading the parallel arrays directly (tests, one-off per-accepted-hit lookups) -- none of
	// which are hot paths.
	bvh_triangle triangle_at(size_t i) const
	{
		bvh_triangle t;
		t.v0 = vertex(tris[i].i0);
		t.v1 = vertex(tris[i].i1);
		t.v2 = vertex(tris[i].i2);
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

// Batched, SIMD-friendly nearest-hit ray-vs-triangle test over one leaf range [start, start+count)
// (as handed to a bvh_visit_triangles() visitor -- count is always a multiple of SIMD_WIDTH).
// Processes SIMD_WIDTH triangles per iteration reading directly from the tree's SoA vertex arrays
// (no transpose: that's the point of storing them this way), computing every lane's Moller-Trumbore
// result unconditionally (no early return per lane, matching a standard Moller-Trumbore test's math
// but avoiding per-lane branches) so the loop stays a fixed-trip-count, branch-free-per-lane shape
// for the autovectorizer -- plain float[SIMD_WIDTH] arrays, SSE2 baseline, no hand intrinsics. This
// is the function that actually vectorizes in this module; bvh_node's own child-AABB test does not
// (see that struct's doc comment). Degenerate padding triangles (see bvh_build()) always fail the
// |det| check and are never returned as a hit. The final "pick nearest valid lane" reduction is
// ordinary scalar control flow -- only the per-lane geometry math is written to vectorize.
// best_t bounds the search (e.g. an already-found candidate's t, or FLT_MAX); returns true and
// fills out_t/out_triangle_index (an index into the tree's parallel arrays) only for a strictly
// closer hit.
bool ray_triangle_leaf_simd(const bvh_tree& tree, int32_t start, int32_t count, const vec3d& origin, const vec3d& dir,
	float best_t, float& out_t, int32_t& out_triangle_index);

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
	// Inflate by `radius` before delegating to the shared ray_aabb() slab test -- see the radius
	// parameter's doc comment on bvh_visit_triangles() below for why this exists.
	float inflated_min[3] = {bmin[0] - radius, bmin[1] - radius, bmin[2] - radius};
	float inflated_max[3] = {bmax[0] + radius, bmax[1] + radius, bmax[2] + radius};
	float ignored_tmin;
	return ray_aabb(origin, inv_dir, inflated_min, inflated_max, t_max, ignored_tmin);
}

// Same as ray_aabb_visit() above but also reports tmin -- for bvh_visit_triangles()'s front-to-back
// ordering below, which needs each candidate child's own tmin to sort by.
inline bool ray_aabb_visit_tmin(const vec3d& origin, const vec3d& inv_dir, const float bmin[3], const float bmax[3],
	float t_max, float radius, float& out_tmin)
{
	float inflated_min[3] = {bmin[0] - radius, bmin[1] - radius, bmin[2] - radius};
	float inflated_max[3] = {bmax[0] + radius, bmax[1] + radius, bmax[2] + radius};
	return ray_aabb(origin, inv_dir, inflated_min, inflated_max, t_max, out_tmin);
}

} // namespace bvh_detail

// Walks the tree, invoking visit(start, count, t_max) for every leaf whose AABB the ray
// [origin, origin + dir*t_max] intersects, where [start, start+count) indexes the tree's parallel
// triangle arrays (see bvh_tree::triangle_at()). A whole leaf range is handed to the visitor at
// once (not one triangle at a time) so a caller can batch-test the leaf's triangles with SIMD (see
// ray_triangle_leaf_simd() below). Every leaf's count is a multiple of SIMD_WIDTH (see bvh_build()'s
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
//
// Front-to-back: every node's passing children (leaves and internal alike) are sorted by their own
// tmin and visited/pushed nearest-first, and each stack entry carries the tmin it was computed with
// so a since-invalidated entry (t_max shrank below it while it sat on the stack) is discarded on pop
// instead of re-testing its AABB. Measured on the real 219-POF corpus: ~23% faster than the
// build-order traversal it replaced, by far the single biggest lever of the "big think" batch (see
// COLLISION_BVH_NOTES.md) -- letting t_max tighten as early as possible is what lets every other
// per-triangle prune in that batch actually fire. Reshuffles leaf visitation order, which matters for
// the BARY_EPS triangle-edge tie-break; verified against the full real-ship parity suite before this
// replaced the build-order traversal.
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

	struct StackEntry {
		int32_t index;
		int32_t count; // >0: leaf (index is a triangle-array start, matching bvh_node::count[i]'s own
		               // convention); ==0: internal node
		float tmin;
	};
	StackEntry stack[64];
	int sp = 0;
	stack[sp++] = {tree.root, 0, 0.0f};

	while (sp > 0) {
		StackEntry entry = stack[--sp];
		if (entry.tmin > t_max)
			continue; // stale: a nearer hit tightened t_max after this entry was queued

		if (entry.count > 0) {
			visit(entry.index, entry.count, t_max);
			continue;
		}

		const bvh_node& node = tree.nodes[entry.index];

		StackEntry candidates[BVH_N];
		int num_candidates = 0;
		for (int i = 0; i < BVH_N; ++i) {
			if (node.child[i] < 0)
				continue;

			float bmin[3] = {node.minx[i], node.miny[i], node.minz[i]};
			float bmax[3] = {node.maxx[i], node.maxy[i], node.maxz[i]};
			float tmin;
			if (!bvh_detail::ray_aabb_visit_tmin(origin, inv_dir, bmin, bmax, t_max, radius, tmin))
				continue;

			candidates[num_candidates++] = {node.child[i], node.count[i], tmin};
		}

		// Insertion sort ascending by tmin -- at most BVH_N (4) elements, cheaper than std::sort's
		// generality for a fixed tiny array.
		for (int i = 1; i < num_candidates; ++i) {
			StackEntry key = candidates[i];
			int j = i - 1;
			while (j >= 0 && candidates[j].tmin > key.tmin) {
				candidates[j + 1] = candidates[j];
				--j;
			}
			candidates[j + 1] = key;
		}

		// Push farthest-first so the nearest candidate is on top of the (LIFO) stack, popped next.
		for (int i = num_candidates - 1; i >= 0; --i) {
			Assertion(sp < 64, "modelbvh triangle traversal stack overflow -- tree unexpectedly deep");
			stack[sp++] = candidates[i];
		}
	}
}
