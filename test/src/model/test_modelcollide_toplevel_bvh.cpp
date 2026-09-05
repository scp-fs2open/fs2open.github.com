// Correctness parity + benchmark for the top-level submodel LBVH (model_collide_get_submodel_bvh()/
// mc_check_children_via_toplevel_bvh() in modelcollide.cpp), which replaces the linear
// first_child/next_sibling recursive walk for whole-model queries (MC_CHECK_MODEL, neither
// MC_SUBMODEL nor MC_SUBMODEL_INSTANCE). Compares against that same legacy walk (still present,
// forced via the temporary Mc_force_legacy_submodel_walk toggle) as ground truth, since both are
// meant to produce identical results -- the LBVH path is a spatial-indexing change, not a behavior
// change.
//
// Only runs when FSO_BVH_PROFILE_POF_DIR names a directory of real .pof files (a Knossos mod install
// works); skipped otherwise, same convention as test_modelbvh_profile.cpp.
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

// TEMPORARY: see modelcollide.cpp's own doc comment. Non-static purely for this test's A/B toggle.
extern thread_local bool Mc_force_legacy_submodel_walk;

namespace {

vec3d V(float x, float y, float z)
{
	vec3d v;
	v.xyz.x = x;
	v.xyz.y = y;
	v.xyz.z = z;
	return v;
}

matrix random_orient(std::mt19937& rng)
{
	std::uniform_real_distribution<float> angle_dist(0.0f, 6.28318f);
	angles a;
	a.p = angle_dist(rng);
	a.b = angle_dist(rng);
	a.h = angle_dist(rng);
	matrix m;
	vm_angles_2_matrix(&m, &a);
	return m;
}

struct QueryResult {
	int hits;
	int hit_submodel;
	float hit_dist;
	vec3d hit_point_world;
};

// num_hits is a cumulative "how many times a new best was found during traversal" counter (see
// mc_check_triangle_face()/mc_check_triangle_sphereline_face()), not a boolean -- it can legitimately
// differ between the legacy and LBVH paths purely from visiting candidates in a different order, even
// when the final nearest hit is identical. Only hit-vs-miss (hits > 0) and the final nearest hit's
// own fields are meaningful to compare here.
bool results_match(const QueryResult& legacy, const QueryResult& lbvh)
{
	if ((legacy.hits > 0) != (lbvh.hits > 0)) {
		return false;
	}
	if (legacy.hits == 0) {
		return true;
	}
	return legacy.hit_submodel == lbvh.hit_submodel && std::fabs(legacy.hit_dist - lbvh.hit_dist) < 1e-3f &&
		vm_vec_dist(&legacy.hit_point_world, &lbvh.hit_point_world) < 1e-2f;
}

QueryResult run_query(int model_num, int model_instance_num, const vec3d& p0, const vec3d& p1, bool sphereline,
	float radius, const SCP_vector<char>* collision_checked)
{
	matrix orient = vmd_identity_matrix;
	vec3d pos = vmd_zero_vector;

	mc_info mc;
	mc.model_num = model_num;
	mc.model_instance_num = model_instance_num;
	mc.orient = &orient;
	mc.pos = &pos;
	mc.p0 = &p0;
	mc.p1 = &p1;
	mc.flags = MC_CHECK_MODEL | (sphereline ? MC_CHECK_SPHERELINE : 0);
	mc.radius = radius;
	if (collision_checked) {
		mc.collision_checked = *collision_checked;
	}

	int hits = model_collide(&mc);
	return {hits, mc.hit_submodel, mc.hit_dist, mc.hit_point_world};
}

class TopLevelBvhTest : public test::FSTestFixture {
 protected:
	TopLevelBvhTest() : test::FSTestFixture(test::INIT_CFILE | test::INIT_GRAPHICS) {}
};

TEST_F(TopLevelBvhTest, MatchesLegacyWalk_NoInstance_RealShips)
{
	const char* dir_env = std::getenv("FSO_BVH_PROFILE_POF_DIR");
	if (dir_env == nullptr || *dir_env == '\0') {
		GTEST_SKIP() << "Set FSO_BVH_PROFILE_POF_DIR to a mod root directory (containing data/models) "
						"to run this correctness check.";
	}

	cf_add_external_path_root(dir_env);
	std::filesystem::path models_dir = std::filesystem::path(dir_env) / "data" / "models";
	ASSERT_TRUE(std::filesystem::is_directory(models_dir)) << models_dir << " does not exist";

	SCP_vector<int> model_nums;
	for (const auto& entry : std::filesystem::directory_iterator(models_dir)) {
		if (!entry.is_regular_file() || entry.path().extension() != ".pof") {
			continue;
		}
		int model_num = model_load(entry.path().filename().string().c_str());
		if (model_num >= 0) {
			model_nums.push_back(model_num);
		}
		if (model_nums.size() >= 40) {
			break;
		}
	}
	ASSERT_FALSE(model_nums.empty()) << "No .pof files loaded from " << dir_env;

	std::mt19937 rng(13579);
	int checked = 0, mismatches = 0;

	for (int model_num : model_nums) {
		polymodel* pm = model_get(model_num);
		float r = pm->rad * 1.5f;
		std::uniform_real_distribution<float> dist(-r, r);

		for (int q = 0; q < 100; ++q) {
			vec3d p0 = V(dist(rng), dist(rng), dist(rng));
			vec3d p1 = V(dist(rng), dist(rng), dist(rng));
			bool sphereline = (q % 2) == 0;

			Mc_force_legacy_submodel_walk = true;
			QueryResult legacy = run_query(model_num, -1, p0, p1, sphereline, 1.0f, nullptr);
			Mc_force_legacy_submodel_walk = false;
			QueryResult lbvh = run_query(model_num, -1, p0, p1, sphereline, 1.0f, nullptr);

			++checked;
			if (!results_match(legacy, lbvh)) {
				++mismatches;
				ADD_FAILURE() << "model " << pm->filename << " query " << q << " (sphereline=" << sphereline
							   << "): legacy hits=" << legacy.hits << " submodel=" << legacy.hit_submodel
							   << " dist=" << legacy.hit_dist << " vs lbvh hits=" << lbvh.hits
							   << " submodel=" << lbvh.hit_submodel << " dist=" << lbvh.hit_dist;
			}
		}
	}

	Mc_force_legacy_submodel_walk = false;
	SUCCEED() << checked << " queries, " << mismatches << " mismatches (no instance / rest pose).";
}

TEST_F(TopLevelBvhTest, MatchesLegacyWalk_RotatedAndBlownOffSubmodels)
{
	const char* dir_env = std::getenv("FSO_BVH_PROFILE_POF_DIR");
	if (dir_env == nullptr || *dir_env == '\0') {
		GTEST_SKIP() << "Set FSO_BVH_PROFILE_POF_DIR to a mod root directory (containing data/models) "
						"to run this correctness check.";
	}

	cf_add_external_path_root(dir_env);
	std::filesystem::path models_dir = std::filesystem::path(dir_env) / "data" / "models";
	ASSERT_TRUE(std::filesystem::is_directory(models_dir)) << models_dir << " does not exist";

	SCP_vector<int> model_nums;
	for (const auto& entry : std::filesystem::directory_iterator(models_dir)) {
		if (!entry.is_regular_file() || entry.path().extension() != ".pof") {
			continue;
		}
		int model_num = model_load(entry.path().filename().string().c_str());
		polymodel* pm = model_num >= 0 ? model_get(model_num) : nullptr;
		// Focus on ships with enough submodels for rotation/blown-off/collision_checked state to be
		// meaningfully exercised.
		if (pm != nullptr && pm->n_models >= 5) {
			model_nums.push_back(model_num);
		}
		if (model_nums.size() >= 20) {
			break;
		}
	}
	ASSERT_FALSE(model_nums.empty()) << "No suitable .pof files loaded from " << dir_env;

	std::mt19937 rng(24680);
	int checked = 0, mismatches = 0;

	for (int model_num : model_nums) {
		polymodel* pm = model_get(model_num);
		int model_instance_num = model_create_instance(-1, model_num);
		polymodel_instance* pmi = model_get_instance(model_instance_num);

		// Rotate/offset a handful of non-root submodels, blow off one, and (for a subset of queries)
		// populate collision_checked to skip another -- directly exercising the three pieces of
		// per-query/per-instance dynamic state that must NOT be baked into the once-per-frame cache
		// the same way the cache's own transforms are (see mc_check_children_via_toplevel_bvh()'s own
		// doc comment on why collision_checked specifically can't be).
		std::uniform_int_distribution<int> submodel_dist(0, pm->n_models - 1);
		for (int i = 0; i < std::min(pm->n_models, 5); ++i) {
			int sm = submodel_dist(rng);
			if (sm == pm->detail[0]) {
				continue;
			}
			pmi->submodel[sm].canonical_orient = random_orient(rng);
			std::uniform_real_distribution<float> offset_dist(-2.0f, 2.0f);
			pmi->submodel[sm].canonical_offset = V(offset_dist(rng), offset_dist(rng), offset_dist(rng));
		}
		int blown_off_submodel = -1;
		for (int attempt = 0; attempt < pm->n_models; ++attempt) {
			int sm = submodel_dist(rng);
			if (sm != pm->detail[0]) {
				pmi->submodel[sm].blown_off = true;
				blown_off_submodel = sm;
				break;
			}
		}

		SCP_vector<char> collision_checked(pm->n_models, 0);
		int skip_submodel = -1;
		for (int attempt = 0; attempt < pm->n_models; ++attempt) {
			int sm = submodel_dist(rng);
			if (sm != pm->detail[0] && sm != blown_off_submodel) {
				collision_checked[sm] = 1;
				skip_submodel = sm;
				break;
			}
		}

		float r = pm->rad * 1.5f;
		std::uniform_real_distribution<float> dist(-r, r);

		for (int q = 0; q < 60; ++q) {
			vec3d p0 = V(dist(rng), dist(rng), dist(rng));
			vec3d p1 = V(dist(rng), dist(rng), dist(rng));
			bool sphereline = (q % 2) == 0;
			const SCP_vector<char>* cc = (q % 3 == 0 && skip_submodel >= 0) ? &collision_checked : nullptr;

			Mc_force_legacy_submodel_walk = true;
			QueryResult legacy = run_query(model_num, model_instance_num, p0, p1, sphereline, 1.0f, cc);
			Mc_force_legacy_submodel_walk = false;
			QueryResult lbvh = run_query(model_num, model_instance_num, p0, p1, sphereline, 1.0f, cc);

			++checked;
			if (!results_match(legacy, lbvh)) {
				++mismatches;
				ADD_FAILURE() << "model " << pm->filename << " query " << q << " (sphereline=" << sphereline
							   << ", collision_checked=" << (cc != nullptr)
							   << "): legacy hits=" << legacy.hits << " submodel=" << legacy.hit_submodel
							   << " dist=" << legacy.hit_dist << " vs lbvh hits=" << lbvh.hits
							   << " submodel=" << lbvh.hit_submodel << " dist=" << lbvh.hit_dist;
			}
		}

		model_delete_instance(model_instance_num);
	}

	Mc_force_legacy_submodel_walk = false;
	SUCCEED() << checked << " queries, " << mismatches
			  << " mismatches (rotated/blown-off/collision_checked submodels).";
}

// Not a correctness test -- honest A/B against the legacy walk it replaces, two shapes: (a) one
// query per ship (worst case for the LBVH path -- pays the full per-frame build cost with no
// amortization) and (b) many queries per ship per frame (the realistic case this design targets --
// weapon impacts, AI checks, turret los checks routinely hit the same ship many times per frame).
TEST_F(TopLevelBvhTest, BenchmarkVsLegacyWalk)
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
		if (!entry.is_regular_file() || entry.path().extension() != ".pof") {
			continue;
		}
		int model_num = model_load(entry.path().filename().string().c_str());
		polymodel* pm = model_num >= 0 ? model_get(model_num) : nullptr;
		if (pm != nullptr && pm->n_models >= 20) { // the ships this design is meant to help
			model_nums.push_back(model_num);
		}
		if (model_nums.size() >= 15) {
			break;
		}
	}
	if (model_nums.empty()) {
		GTEST_SKIP() << "No ships with >= 20 submodels found under " << dir_env;
	}

	std::mt19937 rng(112233);

	auto make_instance = [&](int model_num) -> int {
		polymodel* pm = model_get(model_num);
		int model_instance_num = model_create_instance(-1, model_num);
		polymodel_instance* pmi = model_get_instance(model_instance_num);
		std::uniform_int_distribution<int> submodel_dist(0, pm->n_models - 1);
		for (int i = 0; i < std::min(pm->n_models, 5); ++i) {
			int sm = submodel_dist(rng);
			if (sm == pm->detail[0]) {
				continue;
			}
			pmi->submodel[sm].canonical_orient = random_orient(rng);
		}
		return model_instance_num;
	};

	auto run_n_queries = [&](int model_num, int model_instance_num, int n) {
		polymodel* pm = model_get(model_num);
		float r = pm->rad * 1.5f;
		std::uniform_real_distribution<float> dist(-r, r);
		volatile int sink = 0;
		for (int q = 0; q < n; ++q) {
			vec3d p0 = V(dist(rng), dist(rng), dist(rng));
			vec3d p1 = V(dist(rng), dist(rng), dist(rng));
			QueryResult res = run_query(model_num, model_instance_num, p0, p1, (q % 2) == 0, 1.0f, nullptr);
			sink += res.hits;
		}
		(void)sink;
	};

	for (int queries_per_ship_per_frame : {1, 20}) {
		SCP_vector<double> legacy_ns, lbvh_ns;

		for (int model_num : model_nums) {
			int model_instance_num = make_instance(model_num);
			polymodel_instance* pmi = model_get_instance(model_instance_num);

			// Force a rebuild each "frame" by bumping the cache's stored frame stamp backward --
			// simplest way to simulate "a new frame" without touching the real Framecount global.
			auto force_next_rebuild = [&]() { pmi->submodel_bvh_cache_frame = -1; };

			constexpr int kTrials = 7;
			for (int t = 0; t < kTrials; ++t) {
				force_next_rebuild();
				Mc_force_legacy_submodel_walk = true;
				auto start = std::chrono::steady_clock::now();
				run_n_queries(model_num, model_instance_num, queries_per_ship_per_frame);
				auto end = std::chrono::steady_clock::now();
				legacy_ns.push_back(
					std::chrono::duration<double, std::nano>(end - start).count() / queries_per_ship_per_frame);

				force_next_rebuild();
				Mc_force_legacy_submodel_walk = false;
				start = std::chrono::steady_clock::now();
				run_n_queries(model_num, model_instance_num, queries_per_ship_per_frame);
				end = std::chrono::steady_clock::now();
				lbvh_ns.push_back(
					std::chrono::duration<double, std::nano>(end - start).count() / queries_per_ship_per_frame);
			}

			model_delete_instance(model_instance_num);
		}

		std::sort(legacy_ns.begin(), legacy_ns.end());
		std::sort(lbvh_ns.begin(), lbvh_ns.end());
		double legacy_median = legacy_ns[legacy_ns.size() / 2];
		double lbvh_median = lbvh_ns[lbvh_ns.size() / 2];

		std::cerr << "[" << queries_per_ship_per_frame << " queries/ship/frame] legacy median "
				  << legacy_median << " ns/query, LBVH median " << lbvh_median
				  << " ns/query, speedup " << (legacy_median / lbvh_median) << "x\n";
	}

	Mc_force_legacy_submodel_walk = false;
	SUCCEED();
}

} // namespace
