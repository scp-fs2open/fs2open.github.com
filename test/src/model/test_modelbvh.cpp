#include <gtest/gtest.h>

#include <math/vecmat.h>
#include <model/modelbvh.h>

#include <algorithm>
#include <cfloat>
#include <cmath>
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
			ASSERT_LE(start + count, static_cast<int>(tree.triangles.size()));
			for (int t = start; t < start + count; ++t) {
				visited_triangles.insert(tree.triangles[t].original_index);
				const bvh_triangle& tri = tree.triangles[t];
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

} // namespace

TEST(BvhBuildTests, EmptyInput_ProducesEmptyTree)
{
	bvh_tree tree = bvh_build({});
	EXPECT_TRUE(tree.triangles.empty());
	EXPECT_TRUE(tree.nodes.empty());
}

TEST(BvhBuildTests, SingleTriangle_ProducesOneLeaf)
{
	SCP_vector<bvh_triangle> input;
	input.push_back(make_tri(V(0, 0, 0), V(1, 0, 0), V(0, 1, 0), 0, 0));

	bvh_tree tree = bvh_build(input);
	ASSERT_EQ(tree.triangles.size(), 1u);
	ASSERT_FALSE(tree.nodes.empty());

	const bvh_node& root = tree.nodes[tree.root];
	int populated = 0;
	for (int i = 0; i < BVH_N; ++i) {
		if (root.child[i] < 0)
			continue;
		populated++;
		EXPECT_EQ(root.count[i], 1);
		EXPECT_EQ(root.child[i], 0);
	}
	EXPECT_EQ(populated, 1);
}

TEST(BvhBuildTests, TwoTriangles_BothPresent)
{
	SCP_vector<bvh_triangle> input;
	input.push_back(make_tri(V(-10, 0, 0), V(-9, 0, 0), V(-10, 1, 0), 0, 0));
	input.push_back(make_tri(V(10, 0, 0), V(11, 0, 0), V(10, 1, 0), 0, 1));

	bvh_tree tree = bvh_build(input);
	ASSERT_EQ(tree.triangles.size(), 2u);

	std::set<int> seen;
	for (const bvh_triangle& t : tree.triangles)
		seen.insert(t.original_index);
	EXPECT_EQ(seen, (std::set<int>{0, 1}));
}

TEST(BvhBuildTests, Cube_AllTrianglesPreservedAndContained)
{
	SCP_vector<bvh_triangle> input = make_cube();
	bvh_tree tree = bvh_build(input);

	ASSERT_EQ(tree.triangles.size(), 12u);

	std::set<int> visited;
	check_containment(tree, tree.root, visited);

	std::set<int> expected;
	for (int i = 0; i < 12; ++i)
		expected.insert(i);
	EXPECT_EQ(visited, expected);
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
	EXPECT_EQ(tree.triangles[tri_index].tmap_num, 5);
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
	EXPECT_EQ(tree.triangles[tri_index].tmap_num, 5);
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
	ASSERT_EQ(tree.triangles.size(), input.size());

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
			EXPECT_EQ(tree.triangles[actual_index].original_index, input[expected_index].original_index) << "ray " << i;
		}
	}
}
