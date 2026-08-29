#include <gtest/gtest.h>

#include <math/vecmat.h>
#include <model/modelbvh_leafindex.h>

#include <algorithm>
#include <cfloat>
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

bvh_leaf_primitive make_box(const vec3d& bmin, const vec3d& bmax, int payload)
{
	bvh_leaf_primitive p;
	p.bmin = bmin;
	p.bmax = bmax;
	p.payload = payload;
	return p;
}

// Ground-truth ray-vs-AABB test (independent of the BVH's own internal slab test), used as an
// oracle for the random test below.
bool brute_force_ray_aabb(const vec3d& origin, const vec3d& dir, float t_max, const vec3d& bmin, const vec3d& bmax)
{
	float tmin = 0.0f;
	float tmax = t_max;
	float o[3] = {origin.xyz.x, origin.xyz.y, origin.xyz.z};
	float d[3] = {dir.xyz.x, dir.xyz.y, dir.xyz.z};
	float lo[3] = {bmin.xyz.x, bmin.xyz.y, bmin.xyz.z};
	float hi[3] = {bmax.xyz.x, bmax.xyz.y, bmax.xyz.z};

	for (int axis = 0; axis < 3; ++axis) {
		if (d[axis] == 0.0f) {
			if (o[axis] < lo[axis] || o[axis] > hi[axis])
				return false;
			continue;
		}
		float inv = 1.0f / d[axis];
		float t0 = (lo[axis] - o[axis]) * inv;
		float t1 = (hi[axis] - o[axis]) * inv;
		if (t0 > t1)
			std::swap(t0, t1);
		tmin = std::max(tmin, t0);
		tmax = std::min(tmax, t1);
		if (tmin > tmax)
			return false;
	}
	return true;
}

std::set<int32_t> visit_all(const bvh_leaf_tree& tree, const vec3d& origin, const vec3d& dir, float t_max)
{
	std::set<int32_t> visited;
	bvh_visit_leaves(tree, origin, dir, t_max, [&](int32_t payload) { visited.insert(payload); });
	return visited;
}

} // namespace

TEST(BvhLeafIndexTests, EmptyInput_NoVisits)
{
	bvh_leaf_tree tree = bvh_build_leaves({});
	EXPECT_TRUE(tree.nodes.empty());
	EXPECT_TRUE(tree.items.empty());

	std::set<int32_t> visited = visit_all(tree, V(0, 0, -10), V(0, 0, 1), FLT_MAX);
	EXPECT_TRUE(visited.empty());
}

TEST(BvhLeafIndexTests, SinglePrimitive_HitAndMiss)
{
	SCP_vector<bvh_leaf_primitive> input;
	input.push_back(make_box(V(-1, -1, -1), V(1, 1, 1), 42));

	bvh_leaf_tree tree = bvh_build_leaves(input);
	ASSERT_EQ(tree.items.size(), 1u);

	// Ray straight through the box.
	std::set<int32_t> hit = visit_all(tree, V(0, 0, -10), V(0, 0, 1), FLT_MAX);
	EXPECT_EQ(hit, (std::set<int32_t>{42}));

	// Ray that clearly misses.
	std::set<int32_t> miss = visit_all(tree, V(100, 100, -10), V(0, 0, 1), FLT_MAX);
	EXPECT_TRUE(miss.empty());
}

// NOTE on the contract these tests check: bvh_visit_leaves() guarantees it visits *at least*
// every item whose own AABB the ray truly intersects -- it does NOT guarantee it visits *only*
// those items. A BVH leaf slot can bundle multiple nearby primitives (see LEAF_THRESHOLD in
// modelbvh_leafindex.cpp) behind one shared bounding box; a ray that hits that shared box but not
// every individual item inside it still visits all of them. That's correct, expected behavior --
// precise filtering is the caller's job (in the real integration, the existing per-polygon test
// functions), not this spatial index. So these tests assert containment of the true hit(s), not
// exact-set equality, except where the tree holds only a single item (no grouping possible).

TEST(BvhLeafIndexTests, MultipleSeparatedBoxes_TrueHitAlwaysVisited)
{
	SCP_vector<bvh_leaf_primitive> input;
	for (int i = 0; i < 8; ++i) {
		float x = static_cast<float>(i) * 20.0f;
		input.push_back(make_box(V(x - 1, -1, -1), V(x + 1, 1, 1), i));
	}

	bvh_leaf_tree tree = bvh_build_leaves(input);
	ASSERT_EQ(tree.items.size(), 8u);

	// Ray along +Z through box index 3's x-slot only.
	float x3 = 3.0f * 20.0f;
	std::set<int32_t> visited = visit_all(tree, V(x3, 0, -10), V(0, 0, 1), FLT_MAX);
	EXPECT_TRUE(visited.count(3)) << "the truly-intersected box must always be visited";
	// These boxes are spaced 20 units apart with LEAF_THRESHOLD == 2, so at most one immediate
	// neighbor could plausibly be bundled into the same leaf slot -- visiting far-away boxes
	// (e.g. 0 or 7) would indicate the spatial pruning itself is broken.
	EXPECT_LE(visited.size(), 2u);
	for (int32_t p : visited)
		EXPECT_TRUE(p == 2 || p == 3 || p == 4) << "visited implausible payload " << p;
}

TEST(BvhLeafIndexTests, TMaxBoundsTheRay)
{
	SCP_vector<bvh_leaf_primitive> input;
	input.push_back(make_box(V(-1, -1, 9), V(1, 1, 11), 7)); // centered at z=10

	bvh_leaf_tree tree = bvh_build_leaves(input);

	vec3d origin = V(0, 0, 0);
	vec3d dir = V(0, 0, 1);

	// t_max = 5 -> box at z~[9,11] is out of reach.
	EXPECT_TRUE(visit_all(tree, origin, dir, 5.0f).empty());

	// t_max = 20 -> box is reachable.
	EXPECT_EQ(visit_all(tree, origin, dir, 20.0f), (std::set<int32_t>{7}));

	// Unbounded ray also reaches it.
	EXPECT_EQ(visit_all(tree, origin, dir, FLT_MAX), (std::set<int32_t>{7}));
}

TEST(BvhLeafIndexTests, UnderfullNode_NoSpuriousVisits)
{
	// 3 primitives with BVH_N == 4: forces at least one padding slot somewhere in the tree (see
	// modelbvh.h's padding-sentinel contract, reused unchanged by this module's bvh_node).
	SCP_vector<bvh_leaf_primitive> input;
	input.push_back(make_box(V(-10, -1, -1), V(-8, 1, 1), 0));
	input.push_back(make_box(V(-1, -1, -1), V(1, 1, 1), 1));
	input.push_back(make_box(V(8, -1, -1), V(10, 1, 1), 2));

	bvh_leaf_tree tree = bvh_build_leaves(input);

	// Each box's own ray must always be visited (true hit); padding slots must never contribute a
	// visit, and with only 3 items total nothing outside {0,1,2} can ever legitimately appear.
	std::set<int32_t> visited;
	for (int i = 0; i < 3; ++i) {
		float cx = input[i].bmin.xyz.x + 1.0f;
		auto v = visit_all(tree, V(cx, 0, -10), V(0, 0, 1), FLT_MAX);
		visited.insert(v.begin(), v.end());
		EXPECT_TRUE(v.count(i)) << "box " << i << "'s own ray must visit it";
		for (int32_t p : v)
			EXPECT_TRUE(p >= 0 && p <= 2) << "visited out-of-range payload " << p;
	}
	EXPECT_EQ(visited, (std::set<int32_t>{0, 1, 2}));
}

TEST(BvhLeafIndexTests, ManyRandomBoxes_MatchesBruteForceOracle)
{
	std::mt19937 rng(2026); // fixed seed, deterministic
	std::uniform_real_distribution<float> pos_dist(-50.0f, 50.0f);
	std::uniform_real_distribution<float> size_dist(0.5f, 3.0f);

	SCP_vector<bvh_leaf_primitive> input;
	for (int i = 0; i < 250; ++i) {
		vec3d c = V(pos_dist(rng), pos_dist(rng), pos_dist(rng));
		float hx = size_dist(rng), hy = size_dist(rng), hz = size_dist(rng);
		input.push_back(make_box(V(c.xyz.x - hx, c.xyz.y - hy, c.xyz.z - hz), V(c.xyz.x + hx, c.xyz.y + hy, c.xyz.z + hz), i));
	}

	bvh_leaf_tree tree = bvh_build_leaves(input);
	ASSERT_EQ(tree.items.size(), input.size());

	std::uniform_real_distribution<float> ray_dist(-60.0f, 60.0f);
	int missed_true_hits = 0;
	long total_expected = 0;
	long total_actual = 0;
	for (int i = 0; i < 500; ++i) {
		vec3d origin = V(ray_dist(rng), ray_dist(rng), ray_dist(rng));
		vec3d target = V(ray_dist(rng), ray_dist(rng), ray_dist(rng));
		vec3d dir = target - origin;

		std::set<int32_t> expected;
		for (int j = 0; j < static_cast<int>(input.size()); ++j) {
			if (brute_force_ray_aabb(origin, dir, 1.0f, input[j].bmin, input[j].bmax))
				expected.insert(j);
		}

		std::set<int32_t> actual = visit_all(tree, origin, dir, 1.0f);

		// The essential correctness property: every item the brute-force oracle says is truly
		// intersected must be in the visited set (bvh_visit_leaves is a conservative superset,
		// see the note above these tests -- extra items sharing a leaf slot with a true hit are
		// fine, but a missing true hit is not).
		bool all_true_hits_visited = std::all_of(expected.begin(), expected.end(), [&](int32_t p) { return actual.count(p) != 0; });
		if (!all_true_hits_visited) {
			missed_true_hits++;
			if (missed_true_hits <= 3)
				ADD_FAILURE() << "ray " << i << " missed a true hit: expected " << expected.size() << " items";
		}

		total_expected += static_cast<long>(expected.size());
		total_actual += static_cast<long>(actual.size());
	}
	EXPECT_EQ(missed_true_hits, 0);

	// Sanity bound against a "visits everything regardless of the ray" regression (exactly the
	// bug this test caught during development -- a poisoned SAH cost computation made every
	// build collapse into a single leaf covering all 250 items). Some over-visiting from leaf
	// grouping is expected; visiting most of the tree on every ray is not.
	EXPECT_LT(total_actual, total_expected + 250 * 500 / 4) << "visiting far more than expected -- pruning may be broken";
}
