#include <gtest/gtest.h>

#include <math/fvi.h>
#include <math/vecmat.h>
#include <model/modelbvh.h>

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <functional>
#include <random>
#include <set>

namespace {

vec3d V(float x, float y, float z)
{
	vec3d v;
	v.xyz.x = x;
	v.xyz.y = y;
	v.xyz.z = z;
	return v;
}

bvh_triangle make_tri(const vec3d& v0, const vec3d& v1, const vec3d& v2, int tmap_num, int original_index)
{
	bvh_triangle t;
	t.v0 = v0;
	t.v1 = v1;
	t.v2 = v2;
	t.tmap_num = tmap_num;
	t.original_index = original_index;
	return t;
}

// Builds a unit cube [-1,1]^3 as 12 triangles, 2 per face, one tmap_num per face (0-5).
// Face order: -X, +X, -Y, +Y, -Z, +Z.
SCP_vector<bvh_triangle> make_cube()
{
	vec3d p[8] = {
		V(-1, -1, -1), V(1, -1, -1), V(1, 1, -1), V(-1, 1, -1), // z = -1
		V(-1, -1, 1), V(1, -1, 1), V(1, 1, 1), V(-1, 1, 1), // z = 1
	};

	struct Face {
		int a, b, c, d;
		int tmap;
	};
	// Each face wound so two triangles (a,b,c) and (a,c,d) cover the quad.
	Face faces[6] = {
		{0, 4, 7, 3, 0}, // -X
		{1, 2, 6, 5, 1}, // +X
		{0, 1, 5, 4, 2}, // -Y
		{3, 7, 6, 2, 3}, // +Y
		{0, 3, 2, 1, 4}, // -Z
		{4, 5, 6, 7, 5}, // +Z
	};

	SCP_vector<bvh_triangle> tris;
	int idx = 0;
	for (const Face& f : faces) {
		tris.push_back(make_tri(p[f.a], p[f.b], p[f.c], f.tmap, idx++));
		tris.push_back(make_tri(p[f.a], p[f.c], p[f.d], f.tmap, idx++));
	}
	return tris;
}

// Recursively checks that every node's populated slots are well-formed (valid leaf/internal
// references) and that every populated slot's box contains the true bounds of what it covers.
void check_containment(const bvh_tree& tree, int node_idx, std::set<int>& visited_triangles)
{
	const bvh_node& node = tree.nodes[node_idx];
	for (int i = 0; i < BVH_N; ++i) {
		if (node.child[i] < 0) {
			ASSERT_EQ(node.count[i], 0);
			continue;
		}

		float bminx = node.minx[i], bminy = node.miny[i], bminz = node.minz[i];
		float bmaxx = node.maxx[i], bmaxy = node.maxy[i], bmaxz = node.maxz[i];
		ASSERT_LE(bminx, bmaxx);
		ASSERT_LE(bminy, bmaxy);
		ASSERT_LE(bminz, bmaxz);

		if (node.count[i] > 0) {
			int start = node.child[i];
			int count = node.count[i];
			ASSERT_GE(start, 0);
			ASSERT_LE(start + count, static_cast<int>(tree.triangle_count()));
			for (int t = start; t < start + count; ++t) {
				bvh_triangle tri = tree.triangle_at(t);
				visited_triangles.insert(tri.original_index);
				for (const vec3d* v : {&tri.v0, &tri.v1, &tri.v2}) {
					constexpr float EPS = 1e-4f;
					EXPECT_LE(bminx - EPS, v->xyz.x);
					EXPECT_GE(bmaxx + EPS, v->xyz.x);
					EXPECT_LE(bminy - EPS, v->xyz.y);
					EXPECT_GE(bmaxy + EPS, v->xyz.y);
					EXPECT_LE(bminz - EPS, v->xyz.z);
					EXPECT_GE(bmaxz + EPS, v->xyz.z);
				}
			}
		} else {
			ASSERT_GE(node.child[i], 0);
			ASSERT_LT(node.child[i], static_cast<int>(tree.nodes.size()));
			check_containment(tree, node.child[i], visited_triangles);
		}
	}
}

// Trivial, obviously-correct brute-force oracle used as a ground truth for the random-ray test.
bool brute_force_ray_intersect(const SCP_vector<bvh_triangle>& triangles, const vec3d& origin, const vec3d& dir,
	float& out_t, int& out_index)
{
	bool found = false;
	float best_t = FLT_MAX;
	int best_index = -1;
	for (int i = 0; i < static_cast<int>(triangles.size()); ++i) {
		const bvh_triangle& tri = triangles[i];
		vec3d e1 = tri.v1 - tri.v0;
		vec3d e2 = tri.v2 - tri.v0;
		vec3d pvec;
		vm_vec_cross(&pvec, &dir, &e2);
		float det = vm_vec_dot(&e1, &pvec);
		if (std::fabs(det) < 1e-8f)
			continue;
		float inv_det = 1.0f / det;
		vec3d tvec = origin - tri.v0;
		float u = vm_vec_dot(&tvec, &pvec) * inv_det;
		if (u < 0.0f || u > 1.0f)
			continue;
		vec3d qvec;
		vm_vec_cross(&qvec, &tvec, &e1);
		float v = vm_vec_dot(&dir, &qvec) * inv_det;
		if (v < 0.0f || u + v > 1.0f)
			continue;
		float t = vm_vec_dot(&e2, &qvec) * inv_det;
		if (t < 0.0f || t >= best_t)
			continue;
		best_t = t;
		best_index = i;
		found = true;
	}
	if (found) {
		out_t = best_t;
		out_index = best_index;
	}
	return found;
}

// Trivial, obviously-correct brute-force oracle for sphere-line-vs-triangle, built directly from
// the same public fvi.cpp primitives (fvi_sphere_plane, fvi_polyedge_sphereline) the production
// scalar path (mc_check_triangle_sphereline_face() in modelcollide.cpp) uses -- reproduces that
// function's face-priority-then-edge-fallback dispatch and backface cull as an independent,
// unbatched implementation, to serve as ground truth for sphere_triangle_leaf_simd().
bool brute_force_sphere_intersect(const SCP_vector<bvh_triangle>& triangles, const vec3d& sphere_p0,
	const vec3d& sphere_dir, float radius, float& out_t, int& out_index)
{
	bool found = false;
	float best_t = FLT_MAX;
	int best_index = -1;

	for (int i = 0; i < static_cast<int>(triangles.size()); ++i) {
		const bvh_triangle& tri = triangles[i];
		vec3d e1 = tri.v1 - tri.v0;
		vec3d e2 = tri.v2 - tri.v0;
		vec3d normal;
		vm_vec_cross(&normal, &e1, &e2);
		if (vm_vec_normalize_safe(&normal, true) <= 0.0f)
			continue;
		if (vm_vec_dot(&sphere_dir, &normal) > 0.0f)
			continue;

		vec3d hit_point;
		float face_t, delta_t;
		vec3d v0 = tri.v0;
		if (!fvi_sphere_plane(&hit_point, &sphere_p0, &sphere_dir, radius, &normal, &v0, &face_t, &delta_t))
			continue;

		bool hit = false;
		float t = FLT_MAX;

		if (face_t >= 0.0f) {
			vec3d vp0 = hit_point - tri.v0;
			float d00 = vm_vec_dot(&e1, &e1);
			float d01 = vm_vec_dot(&e1, &e2);
			float d11 = vm_vec_dot(&e2, &e2);
			float d20 = vm_vec_dot(&vp0, &e1);
			float d21 = vm_vec_dot(&vp0, &e2);
			float denom = d00 * d11 - d01 * d01;
			if (std::fabs(denom) >= 1e-12f) {
				constexpr float BARY_EPS = 1e-4f;
				float gamma_v1 = (d11 * d20 - d01 * d21) / denom;
				float gamma_v2 = (d00 * d21 - d01 * d20) / denom;
				float gamma_v0 = 1.0f - gamma_v1 - gamma_v2;
				if (gamma_v0 >= -BARY_EPS && gamma_v1 >= -BARY_EPS && gamma_v2 >= -BARY_EPS) {
					hit = true;
					t = face_t;
				}
			}
		}

		if (!hit) {
			const vec3d* verts[3] = {&tri.v0, &tri.v1, &tri.v2};
			vec3d edge_hit_point;
			float edge_hit_time;
			if (fvi_polyedge_sphereline(&edge_hit_point, &sphere_p0, &sphere_dir, radius, 3, verts, &edge_hit_time)) {
				hit = true;
				t = edge_hit_time;
			}
		}

		if (hit && t < best_t) {
			best_t = t;
			best_index = i;
			found = true;
		}
	}

	if (found) {
		out_t = best_t;
		out_index = best_index;
	}
	return found;
}

// Collects the original_index of every triangle in every leaf range bvh_visit_triangles() visits
// (including any degenerate padding triangles -- they just duplicate an original_index already in
// the set, so they're harmless here, and their presence/behavior is covered separately by the
// padding-specific tests above).
std::set<int> visit_all_original_indices(const bvh_tree& tree, const vec3d& origin, const vec3d& dir, float t_max)
{
	std::set<int> visited;
	bvh_visit_triangles(tree, origin, dir, t_max, 0.0f, [&](int32_t start, int32_t count, float & /*t_max*/) {
		for (int32_t t = start; t < start + count; ++t)
			visited.insert(tree.original_index[t]);
	});
	return visited;
}

} // namespace

TEST(BvhBuildTests, EmptyInput_ProducesEmptyTree)
{
	bvh_tree tree = bvh_build({});
	EXPECT_EQ(tree.triangle_count(), 0u);
	EXPECT_TRUE(tree.nodes.empty());
}

TEST(BvhBuildTests, SingleTriangle_ProducesOneLeaf)
{
	SCP_vector<bvh_triangle> input;
	input.push_back(make_tri(V(0, 0, 0), V(1, 0, 0), V(0, 1, 0), 0, 0));

	bvh_tree tree = bvh_build(input);
	// bvh_build() pads every leaf's triangle range up to a multiple of BVH_N (see
	// pad_leaves_to_simd_width() in modelbvh.cpp) so a leaf-batched SIMD intersection pass always
	// gets clean BVH_N-wide chunks; a single real triangle still occupies one full leaf, now with
	// BVH_N-1 degenerate padding copies appended.
	ASSERT_EQ(tree.triangle_count(), static_cast<size_t>(BVH_N));
	ASSERT_FALSE(tree.nodes.empty());

	const bvh_node& root = tree.nodes[tree.root];
	int populated = 0;
	for (int i = 0; i < BVH_N; ++i) {
		if (root.child[i] < 0)
			continue;
		populated++;
		EXPECT_EQ(root.count[i], BVH_N);
		EXPECT_EQ(root.child[i], 0);
	}
	EXPECT_EQ(populated, 1);

	EXPECT_EQ(tree.original_index[0], 0);
	for (int i = 1; i < BVH_N; ++i) {
		// Padding triangles are degenerate (zero-area) copies of the leaf's last real triangle.
		bvh_triangle tri = tree.triangle_at(i);
		EXPECT_TRUE(vm_vec_same(&tri.v0, &tri.v1));
		EXPECT_TRUE(vm_vec_same(&tri.v0, &tri.v2));
	}
}

TEST(BvhBuildTests, TwoTriangles_BothPresent)
{
	SCP_vector<bvh_triangle> input;
	input.push_back(make_tri(V(-10, 0, 0), V(-9, 0, 0), V(-10, 1, 0), 0, 0));
	input.push_back(make_tri(V(10, 0, 0), V(11, 0, 0), V(10, 1, 0), 0, 1));

	bvh_tree tree = bvh_build(input);
	// Both triangles fall within one leaf (count 2 <= LEAF_THRESHOLD == BVH_N, so no split is even
	// attempted); padded up to BVH_N.
	ASSERT_EQ(tree.triangle_count(), static_cast<size_t>(BVH_N));

	std::set<int> seen(tree.original_index.begin(), tree.original_index.end());
	EXPECT_EQ(seen, (std::set<int>{0, 1}));
}

TEST(BvhBuildTests, Cube_AllTrianglesPreservedAndContained)
{
	SCP_vector<bvh_triangle> input = make_cube();
	bvh_tree tree = bvh_build(input);

	// Padding only ever grows the array (never drops real triangles), and every leaf's range is a
	// multiple of BVH_N.
	ASSERT_GE(tree.triangle_count(), 12u);
	EXPECT_EQ(tree.triangle_count() % BVH_N, 0u);

	std::set<int> visited;
	check_containment(tree, tree.root, visited);

	std::set<int> expected;
	for (int i = 0; i < 12; ++i)
		expected.insert(i);
	EXPECT_EQ(visited, expected);
}

TEST(BvhBuildTests, AllLeafRangesArePaddedToMultipleOfBVH_N)
{
	// Covers modelbvh.cpp's pad_leaves_to_simd_width(): every leaf slot's count must come out a
	// clean multiple of BVH_N, and any padding triangles appended beyond the real content of that
	// leaf must be degenerate (zero-area) so they can never win a ray/SIMD intersection test.
	std::function<void(const bvh_tree&, int)> check = [&](const bvh_tree& tree, int node_idx) {
		const bvh_node& node = tree.nodes[node_idx];
		for (int i = 0; i < BVH_N; ++i) {
			if (node.child[i] < 0)
				continue;
			if (node.count[i] > 0) {
				EXPECT_EQ(node.count[i] % BVH_N, 0) << "leaf at node " << node_idx << " slot " << i;
			} else {
				check(tree, node.child[i]);
			}
		}
	};

	for (const auto& make_input : {&make_cube}) {
		SCP_vector<bvh_triangle> input = make_input();
		bvh_tree tree = bvh_build(input);
		check(tree, tree.root);
	}

	// A lone, oddly-sized input (not a multiple of BVH_N) still ends up padded.
	SCP_vector<bvh_triangle> odd;
	for (int i = 0; i < 5; ++i)
		odd.push_back(make_tri(V(float(i) * 10.0f, 0, 0), V(float(i) * 10.0f + 1, 0, 0), V(float(i) * 10.0f, 1, 0), 0, i));
	bvh_tree odd_tree = bvh_build(odd);
	check(odd_tree, odd_tree.root);
	EXPECT_EQ(odd_tree.triangle_count() % BVH_N, 0u);
}

TEST(BvhBuildTests, Cube_RootBoundsMatchCube)
{
	SCP_vector<bvh_triangle> input = make_cube();
	bvh_tree tree = bvh_build(input);

	const bvh_node& root = tree.nodes[tree.root];
	float minx = FLT_MAX, miny = FLT_MAX, minz = FLT_MAX;
	float maxx = -FLT_MAX, maxy = -FLT_MAX, maxz = -FLT_MAX;
	for (int i = 0; i < BVH_N; ++i) {
		if (root.child[i] < 0)
			continue;
		minx = std::min(minx, root.minx[i]);
		miny = std::min(miny, root.miny[i]);
		minz = std::min(minz, root.minz[i]);
		maxx = std::max(maxx, root.maxx[i]);
		maxy = std::max(maxy, root.maxy[i]);
		maxz = std::max(maxz, root.maxz[i]);
	}

	constexpr float EPS = 1e-4f;
	EXPECT_NEAR(minx, -1.0f, EPS);
	EXPECT_NEAR(miny, -1.0f, EPS);
	EXPECT_NEAR(minz, -1.0f, EPS);
	EXPECT_NEAR(maxx, 1.0f, EPS);
	EXPECT_NEAR(maxy, 1.0f, EPS);
	EXPECT_NEAR(maxz, 1.0f, EPS);
}

TEST(BvhBuildTests, UnderfullNode_PaddingSlotsAreSentinel)
{
	// 3 triangles: BVH_N == 4, so the root cannot have more than 3 real children -- at least one
	// slot must be padding (assuming they don't all further collapse to fewer top-level slots,
	// which can't happen here since count <= LEAF_THRESHOLD(2) forces at least a 2-way split
	// below the root already for 3 triangles > LEAF_THRESHOLD).
	SCP_vector<bvh_triangle> input;
	input.push_back(make_tri(V(-10, 0, 0), V(-9, 0, 0), V(-10, 1, 0), 0, 0));
	input.push_back(make_tri(V(0, 0, 0), V(1, 0, 0), V(0, 1, 0), 0, 1));
	input.push_back(make_tri(V(10, 0, 0), V(11, 0, 0), V(10, 1, 0), 0, 2));

	bvh_tree tree = bvh_build(input);

	bool found_padding = false;
	for (const bvh_node& node : tree.nodes) {
		for (int i = 0; i < BVH_N; ++i) {
			if (node.child[i] < 0) {
				found_padding = true;
				EXPECT_GT(node.minx[i], node.maxx[i]);
				EXPECT_GT(node.miny[i], node.maxy[i]);
				EXPECT_GT(node.minz[i], node.maxz[i]);
				EXPECT_EQ(node.count[i], 0);
			}
		}
	}
	EXPECT_TRUE(found_padding);
}

TEST(BvhRayTests, Cube_RayHitsFace)
{
	SCP_vector<bvh_triangle> input = make_cube();
	bvh_tree tree = bvh_build(input);

	// Ray toward +Z face (tmap_num == 5) from outside.
	vec3d origin = V(0, 0, 5);
	vec3d dir = V(0, 0, -1);

	float t;
	int tri_index;
	ASSERT_TRUE(bvh_ray_intersect(tree, origin, dir, t, tri_index));
	EXPECT_NEAR(t, 4.0f, 1e-3f);
	ASSERT_GE(tri_index, 0);
	EXPECT_EQ(tree.tmap_num[tri_index], 5);
}

TEST(BvhRayTests, Cube_RayMisses)
{
	SCP_vector<bvh_triangle> input = make_cube();
	bvh_tree tree = bvh_build(input);

	vec3d origin = V(100, 100, 100);
	vec3d dir = V(0, 0, -1);

	float t;
	int tri_index;
	EXPECT_FALSE(bvh_ray_intersect(tree, origin, dir, t, tri_index));
}

TEST(BvhRayTests, Cube_RayFromInsideHitsNearestFace)
{
	SCP_vector<bvh_triangle> input = make_cube();
	bvh_tree tree = bvh_build(input);

	vec3d origin = V(0, 0, 0);
	vec3d dir = V(0, 0, 1);

	float t;
	int tri_index;
	ASSERT_TRUE(bvh_ray_intersect(tree, origin, dir, t, tri_index));
	EXPECT_NEAR(t, 1.0f, 1e-3f);
	EXPECT_EQ(tree.tmap_num[tri_index], 5);
}

TEST(BvhRayTests, ManyRandomTriangles_MatchesBruteForceOracle)
{
	std::mt19937 rng(12345); // fixed seed, deterministic
	std::uniform_real_distribution<float> pos_dist(-50.0f, 50.0f);
	std::uniform_real_distribution<float> offset_dist(0.1f, 2.0f);

	SCP_vector<bvh_triangle> input;
	for (int i = 0; i < 200; ++i) {
		vec3d v0 = V(pos_dist(rng), pos_dist(rng), pos_dist(rng));
		vec3d v1 = v0 + V(offset_dist(rng), offset_dist(rng) * 0.1f, offset_dist(rng) * 0.1f);
		vec3d v2 = v0 + V(offset_dist(rng) * 0.1f, offset_dist(rng), offset_dist(rng) * 0.1f);
		input.push_back(make_tri(v0, v1, v2, 0, i));
	}

	bvh_tree tree = bvh_build(input);
	// Padding may add degenerate triangles on top of the real ones (see pad_leaves_to_simd_width()
	// in modelbvh.cpp), so the array can only grow, never shrink or drop input.
	ASSERT_GE(tree.triangle_count(), input.size());

	std::uniform_real_distribution<float> ray_dist(-60.0f, 60.0f);
	for (int i = 0; i < 500; ++i) {
		vec3d origin = V(ray_dist(rng), ray_dist(rng), ray_dist(rng));
		vec3d target = V(ray_dist(rng), ray_dist(rng), ray_dist(rng));
		vec3d dir = target - origin;

		float expected_t;
		int expected_index;
		bool expected_hit = brute_force_ray_intersect(input, origin, dir, expected_t, expected_index);

		float actual_t;
		int actual_index;
		bool actual_hit = bvh_ray_intersect(tree, origin, dir, actual_t, actual_index);

		ASSERT_EQ(actual_hit, expected_hit) << "ray " << i;
		if (expected_hit) {
			EXPECT_NEAR(actual_t, expected_t, 1e-2f) << "ray " << i;
			EXPECT_EQ(tree.original_index[actual_index], input[expected_index].original_index) << "ray " << i;
		}
	}
}

// bvh_visit_triangles() coverage: it's a conservative superset visitor (visits every leaf whose
// AABB the ray intersects, not a precise per-triangle test), so these tests assert containment of
// the true hit(s), not exact-set equality, except where grouping can't happen (a single triangle).

TEST(BvhVisitTrianglesTests, EmptyInput_NoVisits)
{
	bvh_tree tree = bvh_build({});
	EXPECT_TRUE(visit_all_original_indices(tree, V(0, 0, -10), V(0, 0, 1), FLT_MAX).empty());
}

TEST(BvhVisitTrianglesTests, SingleTriangle_HitAndMiss)
{
	SCP_vector<bvh_triangle> input;
	input.push_back(make_tri(V(-1, -1, 0), V(1, -1, 0), V(0, 1, 0), 0, 0));

	bvh_tree tree = bvh_build(input);

	std::set<int> hit = visit_all_original_indices(tree, V(0, 0, -10), V(0, 0, 1), FLT_MAX);
	EXPECT_EQ(hit, (std::set<int>{0}));

	std::set<int> miss = visit_all_original_indices(tree, V(100, 100, -10), V(0, 0, 1), FLT_MAX);
	EXPECT_TRUE(miss.empty());
}

TEST(BvhVisitTrianglesTests, MultipleSeparatedTriangles_TrueHitAlwaysVisited)
{
	SCP_vector<bvh_triangle> input;
	for (int i = 0; i < 8; ++i) {
		float x = static_cast<float>(i) * 20.0f;
		input.push_back(make_tri(V(x - 1, -1, 0), V(x + 1, -1, 0), V(x, 1, 0), 0, i));
	}

	bvh_tree tree = bvh_build(input);

	float x3 = 3.0f * 20.0f;
	std::set<int> visited = visit_all_original_indices(tree, V(x3, 0, -10), V(0, 0, 1), FLT_MAX);
	EXPECT_TRUE(visited.count(3)) << "the truly-intersected triangle must always be visited";
}

TEST(BvhVisitTrianglesTests, TMaxBoundsTheRay)
{
	SCP_vector<bvh_triangle> input;
	input.push_back(make_tri(V(-1, -1, 9), V(1, -1, 11), V(0, 1, 10), 0, 7));

	bvh_tree tree = bvh_build(input);

	vec3d origin = V(0, 0, 0);
	vec3d dir = V(0, 0, 1);

	EXPECT_TRUE(visit_all_original_indices(tree, origin, dir, 5.0f).empty());
	EXPECT_EQ(visit_all_original_indices(tree, origin, dir, 20.0f), (std::set<int>{7}));
	EXPECT_EQ(visit_all_original_indices(tree, origin, dir, FLT_MAX), (std::set<int>{7}));
}

TEST(BvhVisitTrianglesTests, LeafRangesVisitedAreAlwaysMultipleOfBVH_N)
{
	SCP_vector<bvh_triangle> input = make_cube();
	bvh_tree tree = bvh_build(input);

	int visits = 0;
	bvh_visit_triangles(tree, V(0, 0, -10), V(0, 0, 1), FLT_MAX, 0.0f, [&](int32_t /*start*/, int32_t count, float & /*t_max*/) {
		EXPECT_EQ(count % BVH_N, 0);
		visits++;
	});
	EXPECT_GT(visits, 0);
}

TEST(BvhVisitTrianglesTests, ManyRandomTriangles_TrueHitsAlwaysVisited)
{
	std::mt19937 rng(2026); // fixed seed, deterministic
	std::uniform_real_distribution<float> pos_dist(-50.0f, 50.0f);
	std::uniform_real_distribution<float> offset_dist(0.1f, 2.0f);

	SCP_vector<bvh_triangle> input;
	for (int i = 0; i < 200; ++i) {
		vec3d v0 = V(pos_dist(rng), pos_dist(rng), pos_dist(rng));
		vec3d v1 = v0 + V(offset_dist(rng), offset_dist(rng) * 0.1f, offset_dist(rng) * 0.1f);
		vec3d v2 = v0 + V(offset_dist(rng) * 0.1f, offset_dist(rng), offset_dist(rng) * 0.1f);
		input.push_back(make_tri(v0, v1, v2, 0, i));
	}

	bvh_tree tree = bvh_build(input);

	std::uniform_real_distribution<float> ray_dist(-60.0f, 60.0f);
	int missed_true_hits = 0;
	for (int i = 0; i < 500; ++i) {
		vec3d origin = V(ray_dist(rng), ray_dist(rng), ray_dist(rng));
		vec3d target = V(ray_dist(rng), ray_dist(rng), ray_dist(rng));
		vec3d dir = target - origin;

		float expected_t;
		int expected_index;
		bool expected_hit = brute_force_ray_intersect(input, origin, dir, expected_t, expected_index);

		std::set<int> visited = visit_all_original_indices(tree, origin, dir, 1.0f);

		if (expected_hit && expected_t <= 1.0f && !visited.count(input[expected_index].original_index)) {
			missed_true_hits++;
			if (missed_true_hits <= 3)
				ADD_FAILURE() << "ray " << i << " missed a true hit";
		}
	}
	EXPECT_EQ(missed_true_hits, 0);
}

// ray_triangle_leaf_simd() correctness: cross-checked per-leaf against the same scalar
// Moller-Trumbore reference (brute_force_ray_intersect) already used as the oracle above, per the
// stage-4 plan's step-5 gate ("do not proceed to live wiring until it's solid"). Covers a leaf
// smaller than BVH_N (all-padding-but-one lanes), an exact-BVH_N leaf, and a clean miss.

TEST(BvhLeafSimdTests, SingleTriangleLeaf_MatchesScalarReference)
{
	SCP_vector<bvh_triangle> input;
	input.push_back(make_tri(V(-1, -1, 0), V(1, -1, 0), V(0, 1, 0), 0, 0));
	bvh_tree tree = bvh_build(input);
	ASSERT_EQ(tree.triangle_count(), static_cast<size_t>(BVH_N)); // 1 real + BVH_N-1 padding

	vec3d origin = V(0, -0.5f, -10);
	vec3d dir = V(0, 0, 1);

	float expected_t;
	int expected_index;
	bool expected_hit = brute_force_ray_intersect(input, origin, dir, expected_t, expected_index);
	ASSERT_TRUE(expected_hit);

	float simd_t;
	int32_t simd_idx;
	bool simd_hit =
		ray_triangle_leaf_simd(tree, 0, static_cast<int32_t>(tree.triangle_count()), origin, dir, FLT_MAX, simd_t, simd_idx);

	ASSERT_TRUE(simd_hit);
	EXPECT_NEAR(simd_t, expected_t, 1e-4f);
	EXPECT_EQ(tree.original_index[simd_idx], input[expected_index].original_index);
}

TEST(BvhLeafSimdTests, Miss_NoSpuriousHitFromPadding)
{
	SCP_vector<bvh_triangle> input;
	input.push_back(make_tri(V(-1, -1, 0), V(1, -1, 0), V(0, 1, 0), 0, 0));
	bvh_tree tree = bvh_build(input);

	// Ray well clear of the real triangle -- if padding triangles ever accidentally registered a
	// hit (e.g. a degenerate-triangle math bug), this would catch it.
	vec3d origin = V(100, 100, -10);
	vec3d dir = V(0, 0, 1);

	float simd_t;
	int32_t simd_idx;
	bool simd_hit =
		ray_triangle_leaf_simd(tree, 0, static_cast<int32_t>(tree.triangle_count()), origin, dir, FLT_MAX, simd_t, simd_idx);
	EXPECT_FALSE(simd_hit);
}

TEST(BvhLeafSimdTests, ManyRandomTriangles_MatchesScalarReferencePerLeaf)
{
	std::mt19937 rng(4242); // fixed seed, deterministic
	std::uniform_real_distribution<float> pos_dist(-50.0f, 50.0f);
	std::uniform_real_distribution<float> offset_dist(0.1f, 2.0f);

	SCP_vector<bvh_triangle> input;
	for (int i = 0; i < 200; ++i) {
		vec3d v0 = V(pos_dist(rng), pos_dist(rng), pos_dist(rng));
		vec3d v1 = v0 + V(offset_dist(rng), offset_dist(rng) * 0.1f, offset_dist(rng) * 0.1f);
		vec3d v2 = v0 + V(offset_dist(rng) * 0.1f, offset_dist(rng), offset_dist(rng) * 0.1f);
		input.push_back(make_tri(v0, v1, v2, 0, i));
	}

	bvh_tree tree = bvh_build(input);

	std::uniform_real_distribution<float> ray_dist(-60.0f, 60.0f);
	int checked_leaves = 0;
	for (int i = 0; i < 300; ++i) {
		vec3d origin = V(ray_dist(rng), ray_dist(rng), ray_dist(rng));
		vec3d target = V(ray_dist(rng), ray_dist(rng), ray_dist(rng));
		vec3d dir = target - origin;

		bvh_visit_triangles(tree, origin, dir, FLT_MAX, 0.0f, [&](int32_t start, int32_t count, float & /*t_max*/) {
			checked_leaves++;

			// Scalar reference over exactly this leaf's triangles (via triangle_at(), the
			// non-hot-path accessor -- includes any degenerate padding, which must never win).
			SCP_vector<bvh_triangle> leaf_tris;
			for (int32_t t = start; t < start + count; ++t)
				leaf_tris.push_back(tree.triangle_at(t));
			float expected_t;
			int expected_local_idx;
			bool expected_hit = brute_force_ray_intersect(leaf_tris, origin, dir, expected_t, expected_local_idx);

			float simd_t;
			int32_t simd_idx;
			bool simd_hit = ray_triangle_leaf_simd(tree, start, count, origin, dir, FLT_MAX, simd_t, simd_idx);

			ASSERT_EQ(simd_hit, expected_hit) << "ray " << i << " leaf [" << start << "," << count << ")";
			if (expected_hit) {
				EXPECT_NEAR(simd_t, expected_t, 1e-3f) << "ray " << i;
				EXPECT_EQ(simd_idx, start + expected_local_idx) << "ray " << i;
			}
		});
	}
	EXPECT_GT(checked_leaves, 0);
}

TEST(BvhLeafSimdTests, BestTBound_RejectsFartherHits)
{
	// best_t must act as a strict upper bound: a leaf hit farther than best_t must not be reported.
	SCP_vector<bvh_triangle> input;
	input.push_back(make_tri(V(-1, -1, 10), V(1, -1, 10), V(0, 1, 10), 0, 0)); // centered at z=10

	bvh_tree tree = bvh_build(input);

	vec3d origin = V(0, -0.5f, 0);
	vec3d dir = V(0, 0, 1);

	float simd_t;
	int32_t simd_idx;
	// The real hit is at t=10; bound the search to t<5, which must reject it.
	EXPECT_FALSE(
		ray_triangle_leaf_simd(tree, 0, static_cast<int32_t>(tree.triangle_count()), origin, dir, 5.0f, simd_t, simd_idx));
	// A generous bound must still find it.
	EXPECT_TRUE(
		ray_triangle_leaf_simd(tree, 0, static_cast<int32_t>(tree.triangle_count()), origin, dir, 20.0f, simd_t, simd_idx));
}

// sphere_triangle_leaf_simd() correctness: cross-checked per-leaf against brute_force_sphere_intersect()
// (built from the same public fvi.cpp primitives the production scalar path uses). Covers a leaf
// smaller than BVH_N, a clean miss, many random triangle/sphere-line configurations exercising both
// the face and edge-fallback branches, and the best_t bound.

TEST(SphereTriangleLeafSimdTests, SingleTriangleLeaf_MatchesScalarReference)
{
	SCP_vector<bvh_triangle> input;
	// Winding gives this triangle a -z normal, facing the incoming sphere below -- backface
	// culling (matching the scalar reference) would otherwise skip it entirely.
	input.push_back(make_tri(V(-1, -1, 0), V(0, 1, 0), V(1, -1, 0), 0, 0));
	bvh_tree tree = bvh_build(input);
	ASSERT_EQ(tree.triangle_count(), static_cast<size_t>(BVH_N));

	// sphereline queries parametrize the sphere's full per-frame displacement over t in [0,1] --
	// unlike the unbounded ray tests above, dir here must actually span origin to (and past) the
	// triangle within that unit interval, not just point at it.
	vec3d origin = V(0, -0.5f, -10);
	vec3d dir = V(0, 0, 10);
	float radius = 0.3f;

	float expected_t;
	int expected_index;
	bool expected_hit = brute_force_sphere_intersect(input, origin, dir, radius, expected_t, expected_index);
	ASSERT_TRUE(expected_hit);

	float simd_t;
	int32_t simd_idx;
	bool simd_hit = sphere_triangle_leaf_simd(
		tree, 0, static_cast<int32_t>(tree.triangle_count()), origin, dir, radius, FLT_MAX, simd_t, simd_idx);

	ASSERT_TRUE(simd_hit);
	EXPECT_NEAR(simd_t, expected_t, 1e-3f);
	EXPECT_EQ(tree.original_index[simd_idx], input[expected_index].original_index);
}

TEST(SphereTriangleLeafSimdTests, Miss_NoSpuriousHitFromPadding)
{
	SCP_vector<bvh_triangle> input;
	input.push_back(make_tri(V(-1, -1, 0), V(1, -1, 0), V(0, 1, 0), 0, 0));
	bvh_tree tree = bvh_build(input);

	vec3d origin = V(100, 100, -10);
	vec3d dir = V(0, 0, 1);

	float simd_t;
	int32_t simd_idx;
	bool simd_hit = sphere_triangle_leaf_simd(
		tree, 0, static_cast<int32_t>(tree.triangle_count()), origin, dir, 0.3f, FLT_MAX, simd_t, simd_idx);
	EXPECT_FALSE(simd_hit);
}

TEST(SphereTriangleLeafSimdTests, ManyRandomTriangles_MatchesScalarReferencePerLeaf)
{
	std::mt19937 rng(9001); // fixed seed, deterministic
	std::uniform_real_distribution<float> pos_dist(-50.0f, 50.0f);
	std::uniform_real_distribution<float> offset_dist(0.1f, 2.0f);

	SCP_vector<bvh_triangle> input;
	for (int i = 0; i < 200; ++i) {
		vec3d v0 = V(pos_dist(rng), pos_dist(rng), pos_dist(rng));
		vec3d v1 = v0 + V(offset_dist(rng), offset_dist(rng) * 0.1f, offset_dist(rng) * 0.1f);
		vec3d v2 = v0 + V(offset_dist(rng) * 0.1f, offset_dist(rng), offset_dist(rng) * 0.1f);
		input.push_back(make_tri(v0, v1, v2, 0, i));
	}

	bvh_tree tree = bvh_build(input);

	std::uniform_real_distribution<float> ray_dist(-60.0f, 60.0f);
	std::uniform_real_distribution<float> radius_dist(0.2f, 3.0f);
	int checked_leaves = 0;
	for (int i = 0; i < 300; ++i) {
		vec3d origin = V(ray_dist(rng), ray_dist(rng), ray_dist(rng));
		vec3d target = V(ray_dist(rng), ray_dist(rng), ray_dist(rng));
		vec3d dir = target - origin;
		float radius = radius_dist(rng);

		bvh_visit_triangles(tree, origin, dir, FLT_MAX, radius, [&](int32_t start, int32_t count, float & /*t_max*/) {
			checked_leaves++;

			SCP_vector<bvh_triangle> leaf_tris;
			for (int32_t t = start; t < start + count; ++t)
				leaf_tris.push_back(tree.triangle_at(t));
			float expected_t;
			int expected_local_idx;
			bool expected_hit =
				brute_force_sphere_intersect(leaf_tris, origin, dir, radius, expected_t, expected_local_idx);

			float simd_t;
			int32_t simd_idx;
			bool simd_hit = sphere_triangle_leaf_simd(tree, start, count, origin, dir, radius, FLT_MAX, simd_t, simd_idx);

			ASSERT_EQ(simd_hit, expected_hit) << "query " << i << " leaf [" << start << "," << count << ")";
			if (expected_hit) {
				EXPECT_NEAR(simd_t, expected_t, 1e-2f) << "query " << i;
				EXPECT_EQ(simd_idx, start + expected_local_idx) << "query " << i;
			}
		});
	}
	EXPECT_GT(checked_leaves, 0);
}

TEST(SphereTriangleLeafSimdTests, BestTBound_RejectsFartherHits)
{
	SCP_vector<bvh_triangle> input;
	// -z-facing winding, for the same backface-cull reason as the SingleTriangleLeaf test above.
	input.push_back(make_tri(V(-1, -1, 10), V(0, 1, 10), V(1, -1, 10), 0, 0)); // centered at z=10

	bvh_tree tree = bvh_build(input);

	// dir must span origin (z=0) past the triangle (z=10) within t in [0,1] -- the real hit lands
	// near t~1.0 (fractionally earlier, from the radius offset).
	vec3d origin = V(0, -0.5f, 0);
	vec3d dir = V(0, 0, 10);
	float radius = 0.3f;

	float simd_t;
	int32_t simd_idx;
	// Bound the search to t<0.5, which must reject the ~t=1.0 hit.
	EXPECT_FALSE(sphere_triangle_leaf_simd(
		tree, 0, static_cast<int32_t>(tree.triangle_count()), origin, dir, radius, 0.5f, simd_t, simd_idx));
	// A generous bound must still find it.
	EXPECT_TRUE(sphere_triangle_leaf_simd(
		tree, 0, static_cast<int32_t>(tree.triangle_count()), origin, dir, radius, 2.0f, simd_t, simd_idx));
}
