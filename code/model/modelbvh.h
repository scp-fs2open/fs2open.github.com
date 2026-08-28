#pragma once

#include "globalincs/pstypes.h"
#include "globalincs/vmallocator.h"

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
constexpr int BVH_N = 4;

// One triangle of input geometry. Deliberately minimal and engine-agnostic: no submodel/version/
// texture-filename references, just the geometry plus a caller-defined material tag and a stable
// index back into whatever the caller's original array was, for traceability.
struct bvh_triangle {
	vec3d v0, v1, v2;
	int tmap_num = -1;
	int original_index = -1;
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
// reordered into leaf-contiguous (SAH build) order. tree.triangles[node.child[i] .. +count[i])
// is the triangle range for leaf slot i.
struct bvh_tree {
	SCP_vector<bvh_node> nodes;
	SCP_vector<bvh_triangle> triangles;
	int root = 0;
};

// Builds a BVH over the given triangle soup using a binary SAH build, greedily collapsed into
// BVH_N-wide nodes and flattened depth-first. Takes triangles by value: the builder reorders them
// internally, and taking ownership keeps the API unambiguous.
bvh_tree bvh_build(SCP_vector<bvh_triangle> triangles);

// Nearest-hit ray query against a built tree. Not the final optimized traversal (no SIMD, no
// explicit stack tuning) -- it exists so the build can be validated by ray queries in tests.
// Returns true and fills out_t/out_triangle_index on a hit; out_triangle_index indexes tree.triangles.
bool bvh_ray_intersect(const bvh_tree& tree, const vec3d& origin, const vec3d& dir, float& out_t, int& out_triangle_index);
