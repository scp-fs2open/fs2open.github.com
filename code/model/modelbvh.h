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
// Tried N=8 to match AVX's 8-wide float32 lanes (2026-08-30) -- measured *worse*, not better, in
// two separate attempts:
// 1. First attempt: LEAF_THRESHOLD was defined as `= BVH_N` (modelbvh.cpp) at the time, so N=8 also
//    raised the SAH forced-leaf-stop floor -- leaves ended up with roughly twice the average
//    triangle count, reshaping the tree and changing leaf-visitation order, which flips the outcome
//    of an already-known traversal-order-sensitive tie-break (fvi_point_face vs. barycentric
//    containment disagreeing at float-precision boundaries -- see mc_check_bvh_triangle()'s doc
//    comment in modelcollide.cpp). Result: 1.97x/1.96x median/min BVH-vs-BSP speedup vs. N=4's
//    2.30x/2.22x, and RAY mismatch rate rose from 0.39% to 0.62% (tripped BvhTriangleParityTest's
//    calibrated 0.5% threshold).
// 2. LEAF_THRESHOLD was then decoupled into its own fixed constant (still 4) specifically to retry
//    N=8 with tree shape held constant. Re-measured: RAY mismatches came back bit-identical to N=4
//    (3749/950000, confirming the SIMD batch width genuinely cannot change RAY results -- each lane
//    is an independent per-triangle computation and the "nearest" reduction is a width-independent
//    linear min-scan, per BvhLeafSimdTests). SPHERELINE mismatches still shifted slightly (5864 vs.
//    5676) because BVH_N also controls the *internal node* branching factor (emit_node() collapses
//    the binary SAH tree into BVH_N-wide flat nodes), which reorders traversal independent of leaf
//    contents -- a second, narrower instance of the same order-sensitivity, not a new mechanism.
//    Speedup was still worse (1.72x/1.72x): with LEAF_THRESHOLD fixed at 4, real leaves stay sized
//    around ~4 triangles, so padding every leaf up to a multiple of 8 roughly doubled the total
//    padded triangle count (14.7M vs. 7.4M primitives) -- pure padding waste that outweighs
//    whatever wider-register throughput AVX might otherwise buy.
// Conclusion: N=8 is a straightforward net loss for this codebase's real content at N=4's leaf
// sizing, not a correctness or measurement artifact. A future width experiment would need to also
// retune LEAF_THRESHOLD to be a good fit for BVH_N=8-sized leaves (not just decoupled-and-fixed at
// 4).
//
// Codegen ruled out as the cause (2026-08-31): confirmed both widths directly by temporarily
// setting BVH_N=8, recompiling modelbvh.cpp standalone (/O2 /arch:AVX2, no /GL), and inspecting the
// generated assembly the same way as N=4 (see ray_triangle_leaf_simd()'s doc comment below). At
// N=4 the compiler emits packed 128-bit (xmm) SIMD; at N=8 it genuinely emits packed 256-bit (ymm)
// SIMD for the same det/u/v/t computation (e.g. `vmulps ymm0, ymm3, YMMWORD PTR ...` -- real
// single-pass 8-wide arithmetic, not two 4-wide passes disguised as one). So the autovectorizer did
// exactly what the width experiment hoped for -- N=8's measured loss is *entirely* explained by the
// leaf-padding-waste mechanism above (14.7M vs. 7.4M padded primitives), not a codegen failure. Any
// future retry of N=8 needs to fix the padding-waste problem (i.e. retune LEAF_THRESHOLD for it)
// specifically, not re-check whether the compiler can widen -- that part already works.
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
	int leaf_index = -1; // index into bsp_collision_tree::leaf_list; -1 for synthetic/test triangles
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
// reordered into leaf-contiguous (SAH build) order and stored Structure-of-Arrays -- this is the
// tree's actual triangle storage, not a cache derived from some other array, so leaf-batched SIMD
// intersection (ray_triangle_leaf_simd()) reads vertex components directly with no per-query
// transpose. [node.child[i] .. +count[i]) indexes every one of these parallel arrays for leaf slot
// i. Metadata that's only ever read once per accepted hit (never per SIMD lane) -- tmap_num,
// original_index, leaf_index, UVs -- stays out of the hot vertex arrays but is still
// parallel-indexed the same way, for the same reason: one source of truth, no syncing.
struct bvh_tree {
	SCP_vector<bvh_node> nodes;

	SCP_vector<float> v0x, v0y, v0z;
	SCP_vector<float> v1x, v1y, v1z;
	SCP_vector<float> v2x, v2y, v2z;
	SCP_vector<int> tmap_num;
	SCP_vector<int> original_index;
	SCP_vector<int> leaf_index;
	SCP_vector<bvh_uv> uv0, uv1, uv2;

	int root = 0;

	size_t triangle_count() const { return v0x.size(); }

	// Reconstructs triangle i as a single bvh_triangle, for call sites where that's more convenient
	// than reading the parallel arrays directly (tests, the scalar bvh_ray_intersect() reference,
	// and one-off per-accepted-hit lookups) -- none of which are hot paths.
	bvh_triangle triangle_at(size_t i) const
	{
		bvh_triangle t;
		t.v0.xyz.x = v0x[i];
		t.v0.xyz.y = v0y[i];
		t.v0.xyz.z = v0z[i];
		t.v1.xyz.x = v1x[i];
		t.v1.xyz.y = v1y[i];
		t.v1.xyz.z = v1z[i];
		t.v2.xyz.x = v2x[i];
		t.v2.xyz.y = v2y[i];
		t.v2.xyz.z = v2z[i];
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
// never returned as a hit.
//
// Confirmed, not assumed (2026-08-31): compiled this file standalone with the project's real
// release flags (/O2 /arch:AVX2, no /GL so codegen happens at compile time rather than deferred to
// LTCG) and inspected the generated assembly (/FAs). The det/u/v/t computation loop genuinely
// vectorizes to packed 128-bit SIMD (xmm registers, VEX-encoded vmulps/vsubps/vaddps/vandps/vxorps
// throughout the cross-product/dot-product chain) -- not scalar code that merely looks
// vectorization-friendly. Zero 256-bit (ymm) instructions, which is *correct* for BVH_N=4 (4
// floats = 128 bits), not a sign of failure -- see BVH_N's own doc comment for what that implies
// about a hypothetical BVH_N=8 retry. The final "pick nearest valid lane" reduction (the
// `valid[i]`/`if (valid[i] && t[i] < best_t)` loops) is genuinely scalar control flow (~50
// conditional jumps in the compiled function) -- expected and fine, since only the per-lane
// geometry math was ever written to vectorize; the BVH_N=4-iteration reduction was never meant to.
// best_t bounds the search (e.g. an already-found candidate's t, or FLT_MAX); returns true and
// fills out_t/out_triangle_index (an index into the tree's parallel arrays) only for a strictly
// closer hit.
bool ray_triangle_leaf_simd(const bvh_tree& tree, int32_t start, int32_t count, const vec3d& origin, const vec3d& dir,
	float best_t, float& out_t, int32_t& out_triangle_index);

// The ray-vs-AABB slab test and its radius-inflated wrapper live here (inline), not in modelbvh.cpp,
// specifically so they can inline into bvh_visit_triangles()'s hot per-node-slot loop below --
// out-of-line in a separate translation unit, the compiler couldn't fold the per-call
// inflated_min/inflated_max construction or vectorize across the BVH_N slots the SoA bvh_node
// layout exists for in the first place (measured 2026-08-31: this was the single most-executed
// operation in the traversal, so it's worth the header-inlining tradeoff here specifically).
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
// once (not one triangle at a time) so a caller can
// batch-test the leaf's triangles (see the SIMD-friendly leaf intersection design in the stage-4
// project plan). Every leaf's count is a multiple of BVH_N (see bvh_build()'s padding step), with
// any padding triangles guaranteed to be degenerate (zero-area) and never a valid hit. Pass
// t_max = FLT_MAX for an unbounded ray (MC_CHECK_RAY-equivalent).
// `radius` inflates every AABB test by that amount on every axis before testing -- pass
// Mc->radius for an MC_CHECK_SPHERELINE query, 0.0f for a plain ray. Without this, a sphere query
// would silently miss any leaf its swept volume would reach but the bare centerline ray-segment
// doesn't pass through -- a real bug found and fixed via this module's own real-content parity
// test (see collision_bvh_rewrite_plan project notes).
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
