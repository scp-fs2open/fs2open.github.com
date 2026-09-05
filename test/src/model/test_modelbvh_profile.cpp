// Standalone profiling workload, NOT a correctness test -- it's a long-running loop of real-POF
// model_collide() sphereline queries meant to be captured under a CPU-sampling profiler (wpr/ETW).
// Only runs when FSO_BVH_PROFILE_POF_DIR names a directory of real .pof files (a Knossos mod install
// works); skipped otherwise so it never affects normal `unittests` runs or CI.
#include <gtest/gtest.h>

#include <cfile/cfilesystem.h>
#include <math/vecmat.h>
#include <model/model.h>

#include <util/FSTestFixture.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>
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

class BvhProfileTest : public test::FSTestFixture {
 protected:
	// model_load() builds a GPU vertex buffer as part of loading, which needs a graphics backend
	// initialized (INIT_GRAPHICS) -- without it, that call chain hits an unbound backend function
	// pointer and throws std::bad_function_call.
	BvhProfileTest() : test::FSTestFixture(test::INIT_CFILE | test::INIT_GRAPHICS) {}
};

TEST_F(BvhProfileTest, FullModelSpherelineWorkload)
{
	// Names a mod root (a directory containing a data/models subfolder of loose .pof files, e.g. a
	// Knossos mod install) -- cfile resolves model_load()'s filename against data/models under
	// whatever root cf_add_external_path_root() registers, so the root itself must be the mod dir,
	// not data/models directly.
	const char* dir_env = std::getenv("FSO_BVH_PROFILE_POF_DIR");
	if (dir_env == nullptr || *dir_env == '\0') {
		GTEST_SKIP() << "Set FSO_BVH_PROFILE_POF_DIR to a mod root directory (containing data/models) "
						"to run this profiling workload.";
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
		if (model_nums.size() >= 40) {
			break;
		}
	}

	ASSERT_FALSE(model_nums.empty()) << "No .pof files loaded from " << dir_env;

	std::mt19937 rng(12345);

	auto random_query = [&](int model_num, vec3d& p0, vec3d& p1) {
		polymodel* pm = model_get(model_num);
		float r = pm->rad * 1.5f;
		std::uniform_real_distribution<float> dist(-r, r);
		p0 = V(dist(rng), dist(rng), dist(rng));
		p1 = V(dist(rng), dist(rng), dist(rng));
	};

	const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);
	long long queries = 0;

	while (std::chrono::steady_clock::now() < deadline) {
		for (int model_num : model_nums) {
			vec3d p0, p1;
			random_query(model_num, p0, p1);

			matrix orient = vmd_identity_matrix;
			vec3d pos = vmd_zero_vector;

			mc_info mc;
			mc.model_num = model_num;
			mc.orient = &orient;
			mc.pos = &pos;
			mc.p0 = &p0;
			mc.p1 = &p1;
			mc.flags = MC_CHECK_MODEL | MC_CHECK_SPHERELINE;
			mc.radius = 1.0f;

			model_collide(&mc);
			++queries;

			if (std::chrono::steady_clock::now() >= deadline) {
				break;
			}
		}
	}

	SUCCEED() << "Ran " << queries << " sphereline queries across " << model_nums.size() << " models.";
}

} // namespace
