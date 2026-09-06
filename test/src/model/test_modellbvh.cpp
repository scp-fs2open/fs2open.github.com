#include <gtest/gtest.h>

#include <math/vecmat.h>
#include <model/modellbvh.h>

#include <algorithm>
#include <cfloat>
#include <chrono>
#include <iostream>
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

lbvh_item make_item(const vec3d& box_min, const vec3d& box_max, int submodel_index)
{
	lbvh_item it;
	it.box_min = box_min;
	it.box_max = box_max;
	it.submodel_index = submodel_index;
	return it;
}

SCP_vector<int32_t> visit_all(const lbvh_tree& tree, const vec3d& p0, const vec3d& p1, float radius = 0.0f)
{
	SCP_vector<int32_t> hits;
	vec3d dir = p1 - p0;
	float t_max = 1.0f;
	lbvh_visit(tree, p0, dir, t_max, radius, [&](int32_t idx) { hits.push_back(idx); });
	return hits;
}

TEST(ModelLbvh, EmptyTree)
{
	lbvh_tree tree = lbvh_build({});
	EXPECT_EQ(tree.root, INT32_MIN);
	auto hits = visit_all(tree, V(-10, 0, 0), V(10, 0, 0));
	EXPECT_TRUE(hits.empty());
}

TEST(ModelLbvh, SingleItemHitAndMiss)
{
	lbvh_tree tree = lbvh_build({make_item(V(-1, -1, -1), V(1, 1, 1), 42)});

	auto hit = visit_all(tree, V(-10, 0, 0), V(10, 0, 0));
	ASSERT_EQ(hit.size(), 1u);
	EXPECT_EQ(hit[0], 42);

	auto miss = visit_all(tree, V(-10, 100, 0), V(10, 100, 0));
	EXPECT_TRUE(miss.empty());
}

TEST(ModelLbvh, VisitsOnlyBoxesNearTheRay)
{
	// Boxes spread out along X, ray only grazes the ones near the origin.
	SCP_vector<lbvh_item> items;
	for (int i = 0; i < 20; ++i) {
		float x = static_cast<float>(i) * 10.0f;
		items.push_back(make_item(V(x - 1, -1, -1), V(x + 1, 1, 1), i));
	}
	lbvh_tree tree = lbvh_build(std::move(items));

	// Ray along Y at x=0 (item 0's box) -- should hit only item 0.
	auto hits = visit_all(tree, V(0, -10, 0), V(0, 10, 0));
	ASSERT_EQ(hits.size(), 1u);
	EXPECT_EQ(hits[0], 0);
}

TEST(ModelLbvh, SphereRadiusReachesNearbyBox)
{
	lbvh_tree tree = lbvh_build({make_item(V(-1, -1, -1), V(1, 1, 1), 7)});

	// Segment passes at y=3, well outside the unit box -- misses as a bare ray...
	auto miss = visit_all(tree, V(-10, 3, 0), V(10, 3, 0));
	EXPECT_TRUE(miss.empty());

	// ...but a radius-5 sphere sweeping the same segment reaches it.
	auto hit = visit_all(tree, V(-10, 3, 0), V(10, 3, 0), 5.0f);
	ASSERT_EQ(hit.size(), 1u);
	EXPECT_EQ(hit[0], 7);
}

TEST(ModelLbvh, ManyRandomBoxes_VisitsExactlyTheOnesTheRayCrosses)
{
	std::mt19937 rng(2024);
	std::uniform_real_distribution<float> pos_dist(-50.0f, 50.0f);
	std::uniform_real_distribution<float> extent_dist(0.5f, 3.0f);

	for (int trial = 0; trial < 50; ++trial) {
		SCP_vector<lbvh_item> items;
		int n = 100;
		for (int i = 0; i < n; ++i) {
			vec3d center = V(pos_dist(rng), pos_dist(rng), pos_dist(rng));
			vec3d half = V(extent_dist(rng), extent_dist(rng), extent_dist(rng));
			items.push_back(make_item(center - half, center + half, i));
		}
		lbvh_tree tree = lbvh_build(items); // copy -- items reused below as ground truth

		vec3d p0 = V(pos_dist(rng), pos_dist(rng), pos_dist(rng));
		vec3d p1 = V(pos_dist(rng), pos_dist(rng), pos_dist(rng));
		vec3d dir = p1 - p0;

		// Brute-force ground truth: same slab test, applied to every box directly.
		std::set<int32_t> expected;
		for (const lbvh_item& it : items) {
			float tmin = 0.0f, tmax = 1.0f;
			bool hit = true;
			float o[3] = {p0.xyz.x, p0.xyz.y, p0.xyz.z};
			float d[3] = {dir.xyz.x, dir.xyz.y, dir.xyz.z};
			float bmin[3] = {it.box_min.xyz.x, it.box_min.xyz.y, it.box_min.xyz.z};
			float bmax[3] = {it.box_max.xyz.x, it.box_max.xyz.y, it.box_max.xyz.z};
			for (int axis = 0; axis < 3 && hit; ++axis) {
				if (d[axis] == 0.0f) {
					if (o[axis] < bmin[axis] || o[axis] > bmax[axis])
						hit = false;
					continue;
				}
				float t0 = (bmin[axis] - o[axis]) / d[axis];
				float t1 = (bmax[axis] - o[axis]) / d[axis];
				if (t0 > t1)
					std::swap(t0, t1);
				tmin = std::max(tmin, t0);
				tmax = std::min(tmax, t1);
				if (tmin > tmax)
					hit = false;
			}
			if (hit)
				expected.insert(it.submodel_index);
		}

		auto actual_vec = visit_all(tree, p0, p1);
		std::set<int32_t> actual(actual_vec.begin(), actual_vec.end());
		EXPECT_EQ(actual, expected) << "trial " << trial;
	}
}

// Not a correctness test -- confirms LBVH build time is negligible at realistic ship submodel
// counts (FS2/FSO ships run up to roughly 100-200 n_models for the largest capital ships), since
// the whole design in modelcollide.cpp depends on rebuilding this tree once per frame per ship
// instance being cheap enough not to matter.
TEST(ModelLbvh, BuildTimeNegligibleAtRealisticSubmodelCounts)
{
	std::mt19937 rng(99);
	std::uniform_real_distribution<float> pos_dist(-100.0f, 100.0f);
	std::uniform_real_distribution<float> extent_dist(0.2f, 5.0f);

	for (int n : {10, 50, 100, 200}) {
		SCP_vector<lbvh_item> base_items;
		for (int i = 0; i < n; ++i) {
			vec3d center = V(pos_dist(rng), pos_dist(rng), pos_dist(rng));
			vec3d half = V(extent_dist(rng), extent_dist(rng), extent_dist(rng));
			base_items.push_back(make_item(center - half, center + half, i));
		}

		constexpr int kTrials = 200;
		SCP_vector<double> ns_per_build;
		for (int t = 0; t < kTrials; ++t) {
			SCP_vector<lbvh_item> items = base_items; // build() consumes its argument
			auto start = std::chrono::steady_clock::now();
			lbvh_tree tree = lbvh_build(std::move(items));
			auto end = std::chrono::steady_clock::now();
			ns_per_build.push_back(std::chrono::duration<double, std::nano>(end - start).count());
			ASSERT_GE(tree.root, 0);
		}
		std::sort(ns_per_build.begin(), ns_per_build.end());
		double median_ns = ns_per_build[ns_per_build.size() / 2];

		std::cerr << "n=" << n << ": median build time " << median_ns << " ns (" << (median_ns / 1000.0)
				  << " us)\n";
		// A generous ceiling, not a tight assertion -- this is a real ballpark check, not a strict
		// perf contract. 50us for 200 submodels would still be a rounding error against a 16.6ms
		// frame budget even if 50+ large ships needed a rebuild in the same frame.
		EXPECT_LT(median_ns, 50000.0) << "n=" << n;
	}
}

} // namespace
