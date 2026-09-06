// Standalone speed benchmark, NOT a correctness test -- reconstructs the dedicated per-call timing
// methodology used for the LEAF_THRESHOLD sweep (see COLLISION_BVH_NOTES.md): a FIXED query set
// (generated once with a fixed seed, reused identically across every trial) against the real
// 219-POF corpus, isolated wall-clock timing around model_collide() calls only, median over many
// trials. Deliberately distinct from test_modelbvh_profile.cpp's FullModelSpherelineWorkload, which
// is a throughput test (random queries every call, fixed wall-clock budget, only the first 40 models
// loaded) -- that one's ns/call derivation bakes in RNG/harness overhead and isn't comparable to this
// benchmark's numbers or to the LEAF_THRESHOLD sweep's own historical table.
// Only runs when FSO_BVH_PROFILE_POF_DIR names a directory of real .pof files (a Knossos mod install
// works); skipped otherwise so it never affects normal `unittests` runs or CI.
#include <gtest/gtest.h>

#include <cfile/cfilesystem.h>
#include <math/vecmat.h>
#include <model/model.h>

#include <util/FSTestFixture.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <random>

namespace {

vec3d V(float x, float y, float z)
{
	vec3d v;
	v.xyz.x = x;
	v.xyz.y = y;
	v.xyz.z = z;
	return v;
}

class BvhSpeedTest : public test::FSTestFixture {
 protected:
	BvhSpeedTest() : test::FSTestFixture(test::INIT_CFILE | test::INIT_GRAPHICS) {}
};

TEST_F(BvhSpeedTest, FullModelSpherelineSpeed)
{
	const char* dir_env = std::getenv("FSO_BVH_PROFILE_POF_DIR");
	if (dir_env == nullptr || *dir_env == '\0') {
		GTEST_SKIP() << "Set FSO_BVH_PROFILE_POF_DIR to a mod root directory (containing data/models) "
						"to run this benchmark.";
	}

	cf_add_external_path_root(dir_env);

	std::filesystem::path models_dir = std::filesystem::path(dir_env) / "data" / "models";
	ASSERT_TRUE(std::filesystem::is_directory(models_dir)) << models_dir << " does not exist";

	SCP_vector<int> model_nums;
	for (const auto& entry : std::filesystem::directory_iterator(models_dir)) {
		if (!entry.is_regular_file()) {
			continue;
		}
		if (entry.path().extension() != ".pof") {
			continue;
		}
		int model_num = model_load(entry.path().filename().string().c_str());
		if (model_num >= 0) {
			model_nums.push_back(model_num);
		}
	}
	ASSERT_FALSE(model_nums.empty()) << "No .pof files loaded from " << dir_env;

	// Fixed query set, generated once with a fixed seed and reused identically across every trial --
	// isolates model_collide()'s own cost from query-generation noise (unlike
	// FullModelSpherelineWorkload, which regenerates random queries on every call).
	struct Query {
		int model_num;
		vec3d p0, p1;
	};
	SCP_vector<Query> queries;
	std::mt19937 rng(12345);
	constexpr int QUERIES_PER_MODEL = 20;
	for (int model_num : model_nums) {
		polymodel* pm = model_get(model_num);
		float r = pm->rad * 1.5f;
		std::uniform_real_distribution<float> dir_dist(-1.0f, 1.0f);
		for (int q = 0; q < QUERIES_PER_MODEL; ++q) {
			// A random direction through the model's own center, not two independent random points
			// in a bounding cube -- the latter (as FullModelSpherelineWorkload uses for a throughput
			// test) mostly misses the ship entirely, resolving on an instant root-AABB reject, which
			// isn't representative of what this benchmark is meant to measure (real per-triangle
			// sphereline cost against real hull geometry -- the actual pattern weapon impacts and AI
			// checks produce).
			vec3d dir = V(dir_dist(rng), dir_dist(rng), dir_dist(rng));
			if (vm_vec_normalize_safe(&dir, true) <= 0.0f) {
				dir = V(1.0f, 0.0f, 0.0f);
			}
			Query query;
			query.model_num = model_num;
			query.p0 = dir * r;
			query.p1 = dir * -r;
			queries.push_back(query);
		}
	}

	constexpr int TRIALS = 15;
	SCP_vector<double> ns_per_call(TRIALS);
	matrix orient = vmd_identity_matrix;
	vec3d pos = vmd_zero_vector;

	for (int trial = 0; trial < TRIALS; ++trial) {
		auto start = std::chrono::high_resolution_clock::now();
		for (const Query& q : queries) {
			vec3d p0 = q.p0;
			vec3d p1 = q.p1;

			mc_info mc;
			mc.model_num = q.model_num;
			mc.orient = &orient;
			mc.pos = &pos;
			mc.p0 = &p0;
			mc.p1 = &p1;
			mc.flags = MC_CHECK_MODEL | MC_CHECK_SPHERELINE;
			mc.radius = 1.0f;

			model_collide(&mc);
		}
		auto end = std::chrono::high_resolution_clock::now();
		double total_ns = std::chrono::duration<double, std::nano>(end - start).count();
		ns_per_call[trial] = total_ns / static_cast<double>(queries.size());
	}

	std::sort(ns_per_call.begin(), ns_per_call.end());
	double median = ns_per_call[TRIALS / 2];

	std::cout << "BENCH_MEDIAN_NS_PER_CALL=" << median << std::endl;
	std::cout << "BENCH_MIN_NS_PER_CALL=" << ns_per_call.front() << std::endl;

	SUCCEED() << "Median " << median << " ns/call across " << TRIALS << " trials, " << queries.size()
			  << " queries/trial, " << model_nums.size() << " models.";
}

} // namespace
