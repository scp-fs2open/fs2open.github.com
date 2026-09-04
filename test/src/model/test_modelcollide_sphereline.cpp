// Correctness coverage for mc_triangle_edges_sphereline() (modelcollide.cpp) -- the triangle-
// specialized replacement for fvi_polyedge_sphereline(nv=3) used by the live hull sphereline path
// (mc_check_triangle_sphereline_face()). Compares against fvi_polyedge_sphereline() itself as ground
// truth, since that function is still live (shield collision, via mc_check_sphereline_face(), still
// calls it with nv>=3 for n-gons) and has 25+ years of production use behind it.
#include <gtest/gtest.h>

#include <math/fvi.h>
#include <math/vecmat.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <random>

// Not declared in any header (modelcollide.cpp keeps it non-static purely so this test can call it
// directly, bypassing the Mc thread_local state the rest of that file reads).
bool mc_triangle_edges_sphereline(const vec3d& v0, const vec3d& v1, const vec3d& v2, const vec3d& xs0,
	const vec3d& vs, float Rs, vec3d& out_hit_point, float& out_t);

namespace {

vec3d V(float x, float y, float z)
{
	vec3d v;
	v.xyz.x = x;
	v.xyz.y = y;
	v.xyz.z = z;
	return v;
}

TEST(TriangleEdgesSphereline, SingleEdgeHit)
{
	// Triangle in the z=0 plane; sphere sweeps straight down through one edge's midpoint.
	vec3d v0 = V(-1, -1, 0), v1 = V(1, -1, 0), v2 = V(0, 1, 0);
	vec3d xs0 = V(0, -1, 5);
	vec3d vs = V(0, 0, -10); // reaches z=-5 at t=1, crosses z=0 (the edge) around t=0.5
	float radius = 0.3f;

	vec3d hit_point;
	float t;
	ASSERT_TRUE(mc_triangle_edges_sphereline(v0, v1, v2, xs0, vs, radius, hit_point, t));
	EXPECT_NEAR(t, 0.47f, 0.05f);
}

TEST(TriangleEdgesSphereline, Miss)
{
	vec3d v0 = V(-1, -1, 0), v1 = V(1, -1, 0), v2 = V(0, 1, 0);
	vec3d xs0 = V(100, 100, 5);
	vec3d vs = V(0, 0, -10);
	float radius = 0.3f;

	vec3d hit_point;
	float t;
	EXPECT_FALSE(mc_triangle_edges_sphereline(v0, v1, v2, xs0, vs, radius, hit_point, t));
}

TEST(TriangleEdgesSphereline, ManyRandomTriangles_MatchesFviPolyedgeSphereline)
{
	std::mt19937 rng(1234); // fixed seed, deterministic
	std::uniform_real_distribution<float> pos_dist(-10.0f, 10.0f);
	std::uniform_real_distribution<float> offset_dist(0.5f, 4.0f);
	std::uniform_real_distribution<float> radius_dist(0.1f, 1.5f);

	int checked = 0;
	int agree = 0;
	int both_hit = 0;

	for (int i = 0; i < 500; ++i) {
		vec3d v0 = V(pos_dist(rng), pos_dist(rng), pos_dist(rng));
		vec3d v1 = v0 + V(offset_dist(rng), offset_dist(rng) * 0.2f, offset_dist(rng) * 0.2f);
		vec3d v2 = v0 + V(offset_dist(rng) * 0.2f, offset_dist(rng), offset_dist(rng) * 0.2f);

		vec3d xs0 = V(pos_dist(rng), pos_dist(rng), pos_dist(rng));
		vec3d vs = V(offset_dist(rng) * (pos_dist(rng) < 0 ? -1 : 1), offset_dist(rng) * (pos_dist(rng) < 0 ? -1 : 1),
			offset_dist(rng) * (pos_dist(rng) < 0 ? -1 : 1));
		float radius = radius_dist(rng);

		const vec3d* verts[3] = {&v0, &v1, &v2};
		vec3d expected_hit_point;
		float expected_t;
		bool expected = fvi_polyedge_sphereline(&expected_hit_point, &xs0, &vs, radius, 3, verts, &expected_t);

		vec3d actual_hit_point;
		float actual_t;
		bool actual = mc_triangle_edges_sphereline(v0, v1, v2, xs0, vs, radius, actual_hit_point, actual_t);

		++checked;
		if (actual == expected) {
			++agree;
			if (expected) {
				++both_hit;
				EXPECT_NEAR(actual_t, expected_t, 1e-2f) << "case " << i;
				EXPECT_NEAR(vm_vec_dist(&actual_hit_point, &expected_hit_point), 0.0f, 1e-2f) << "case " << i;
			}
		} else {
			ADD_FAILURE() << "case " << i << ": hit mismatch, new=" << actual << " old=" << expected;
		}
	}

	SUCCEED() << checked << " cases, " << agree << " agreed on hit/miss, " << both_hit << " both hit.";
}

TEST(TriangleEdgesSphereline, ManyRandomTriangles_GrazingRadius)
{
	// Radius comparable to triangle size, more likely to exercise the vertex fallback / edge-boundary
	// (root-out-of-range) branches than the "small sphere through a big triangle" configurations above.
	std::mt19937 rng(5678);
	std::uniform_real_distribution<float> pos_dist(-5.0f, 5.0f);
	std::uniform_real_distribution<float> tri_extent(0.5f, 2.0f);
	std::uniform_real_distribution<float> radius_dist(0.5f, 3.0f);

	for (int i = 0; i < 500; ++i) {
		vec3d v0 = V(pos_dist(rng), pos_dist(rng), pos_dist(rng));
		vec3d v1 = v0 + V(tri_extent(rng), 0, 0);
		vec3d v2 = v0 + V(0, tri_extent(rng), 0);

		vec3d xs0 = V(pos_dist(rng), pos_dist(rng), pos_dist(rng));
		vec3d vs = V(pos_dist(rng) * 0.3f, pos_dist(rng) * 0.3f, pos_dist(rng) * 0.3f);
		float radius = radius_dist(rng);

		const vec3d* verts[3] = {&v0, &v1, &v2};
		vec3d expected_hit_point;
		float expected_t;
		bool expected = fvi_polyedge_sphereline(&expected_hit_point, &xs0, &vs, radius, 3, verts, &expected_t);

		vec3d actual_hit_point;
		float actual_t;
		bool actual = mc_triangle_edges_sphereline(v0, v1, v2, xs0, vs, radius, actual_hit_point, actual_t);

		ASSERT_EQ(actual, expected) << "case " << i;
		if (expected) {
			EXPECT_NEAR(actual_t, expected_t, 1e-2f) << "case " << i;
		}
	}
}

// Not a correctness test -- isolated ns/call comparison of the two edge-test implementations
// directly (no BVH/model_collide() overhead in either arm, since both take the triangle's verts and
// sphere params directly), alternating trials to cancel warmup/thermal drift.
TEST(TriangleEdgesSphereline, BenchmarkVsFviPolyedgeSphereline)
{
	std::mt19937 rng(9999);
	std::uniform_real_distribution<float> pos_dist(-10.0f, 10.0f);
	std::uniform_real_distribution<float> tri_extent(0.5f, 2.0f);
	std::uniform_real_distribution<float> radius_dist(0.1f, 0.5f);
	std::uniform_real_distribution<float> near_edge(0.05f, 0.95f); // parametrize along an edge
	std::uniform_real_distribution<float> jitter(-0.3f, 0.3f);
	std::uniform_real_distribution<float> approach_dist(2.0f, 6.0f);

	struct Case {
		vec3d v0, v1, v2, xs0, vs;
		float radius;
	};
	// Biased toward guaranteed-ish near-edge crossings (BVH-pruned leaves in real gameplay are
	// exactly this: triangles the sphere's path is already known to be close to), not uniformly
	// scattered misses -- representative of the case this change actually targets (stage-1 succeeds,
	// s in range), not the cheap-reject case both old and new short-circuit identically.
	constexpr int kCasesPerTrial = 20000;
	SCP_vector<Case> cases;
	cases.reserve(kCasesPerTrial);
	for (int i = 0; i < kCasesPerTrial; ++i) {
		Case c;
		c.v0 = V(pos_dist(rng), pos_dist(rng), pos_dist(rng));
		c.v1 = c.v0 + V(tri_extent(rng), 0, 0);
		c.v2 = c.v0 + V(0, tri_extent(rng), 0);

		// Aim through a random point along the v0-v1 edge, from a random approach direction.
		float s = near_edge(rng);
		vec3d target = c.v0 + (c.v1 - c.v0) * s + V(jitter(rng), jitter(rng), jitter(rng));
		vec3d approach_dir = V(jitter(rng), jitter(rng), 1.0f);
		float dist = approach_dist(rng);
		c.xs0 = target - approach_dir * dist;
		c.vs = (target - c.xs0) * 1.2f; // overshoot slightly past the target point
		c.radius = radius_dist(rng);
		cases.push_back(c);
	}

	auto run_old = [&]() -> double {
		auto start = std::chrono::steady_clock::now();
		volatile bool sink = false;
		for (const Case& c : cases) {
			const vec3d* verts[3] = {&c.v0, &c.v1, &c.v2};
			vec3d hit_point;
			float t;
			sink = fvi_polyedge_sphereline(&hit_point, &c.xs0, &c.vs, c.radius, 3, verts, &t);
		}
		(void)sink;
		auto end = std::chrono::steady_clock::now();
		return std::chrono::duration<double, std::nano>(end - start).count() / kCasesPerTrial;
	};

	auto run_new = [&]() -> double {
		auto start = std::chrono::steady_clock::now();
		volatile bool sink = false;
		for (const Case& c : cases) {
			vec3d hit_point;
			float t;
			sink = mc_triangle_edges_sphereline(c.v0, c.v1, c.v2, c.xs0, c.vs, c.radius, hit_point, t);
		}
		(void)sink;
		auto end = std::chrono::steady_clock::now();
		return std::chrono::duration<double, std::nano>(end - start).count() / kCasesPerTrial;
	};

	constexpr int kTrials = 9;
	SCP_vector<double> old_ns, new_ns;

	run_old();
	run_new(); // warmup, discarded

	for (int t = 0; t < kTrials; ++t) {
		if (t % 2 == 0) {
			new_ns.push_back(run_new());
			old_ns.push_back(run_old());
		} else {
			old_ns.push_back(run_old());
			new_ns.push_back(run_new());
		}
	}

	auto median = [](SCP_vector<double> v) -> double {
		std::sort(v.begin(), v.end());
		return v[v.size() / 2];
	};

	double new_median = median(new_ns);
	double old_median = median(old_ns);

	std::cerr << "new (mc_triangle_edges_sphereline) ns/call: ";
	for (double v : new_ns)
		std::cerr << v << " ";
	std::cerr << "(median " << new_median << ")\n";
	std::cerr << "old (fvi_polyedge_sphereline) ns/call:      ";
	for (double v : old_ns)
		std::cerr << v << " ";
	std::cerr << "(median " << old_median << ")\n";
	std::cerr << "speedup (old/new): " << (old_median / new_median) << "x\n";

	SUCCEED() << "new median " << new_median << " ns/call, old median " << old_median << " ns/call";
}

} // namespace
