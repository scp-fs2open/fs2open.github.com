// Performance comparison between the legacy BSP submodel-collision traversal and the new
// BVH-based one (see model_collide_bvh()/mc_check_bvh_leaf() in modelcollide.cpp), both reached
// through the real model_collide() entry point via the live Cmdline_use_bvh_collision toggle --
// same infrastructure as test_modelbvh_traversal_parity.cpp, but measuring wall-clock cost
// instead of checking correctness.
//
// Skipped by default (no .pof in-repo); point FSO_BVH_PARITY_POF at a real .pof file or
// directory (or several, ';'-separated) to run it. This is a Release-only, real-world-scale
// timing comparison, not a micro-benchmark isolated from OS/allocator noise -- multiple
// alternating trials are used to reduce that, but treat the numbers as directional, not exact.

#include <gtest/gtest.h>

#include <cfile/cfilesystem.h>
#include <cmdline/cmdline.h>
#include <math/vecmat.h>
#include <model/model.h>

#include "util/FSTestFixture.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <numeric>
#include <random>
#include <set>
#include <vector>

namespace {

std::vector<std::filesystem::path> collect_pof_files(const std::filesystem::path& entry)
{
	std::vector<std::filesystem::path> result;
	if (!std::filesystem::exists(entry))
		return result;
	if (std::filesystem::is_directory(entry)) {
		for (const auto& de : std::filesystem::recursive_directory_iterator(entry)) {
			if (de.is_regular_file() && stricmp(de.path().extension().string().c_str(), ".pof") == 0)
				result.push_back(de.path());
		}
	} else {
		result.push_back(entry);
	}
	return result;
}

std::filesystem::path find_mod_root(const std::filesystem::path& pof_file)
{
	for (std::filesystem::path search = pof_file.parent_path(); search.has_parent_path(); search = search.parent_path()) {
		if (stricmp(search.filename().string().c_str(), "data") == 0)
			return search.parent_path();
	}
	return pof_file.parent_path().parent_path().parent_path();
}

struct RayCase {
	int model_num;
	int submodel_num;
	vec3d origin;
	vec3d target;
};

double median(std::vector<double> v)
{
	std::sort(v.begin(), v.end());
	size_t n = v.size();
	if (n == 0)
		return 0.0;
	return (n % 2) ? v[n / 2] : (v[n / 2 - 1] + v[n / 2]) * 0.5;
}

} // namespace

class BvhBenchmarkTest : public test::FSTestFixture {
protected:
	BvhBenchmarkTest() : FSTestFixture(INIT_CFILE | INIT_GRAPHICS) {}
};

TEST_F(BvhBenchmarkTest, CompareOldAndNewTraversalSpeed)
{
	const char* env = std::getenv("FSO_BVH_PARITY_POF");
	if (env == nullptr || env[0] == '\0') {
		GTEST_SKIP() << "FSO_BVH_PARITY_POF not set; skipping BVH-vs-BSP benchmark.";
	}

	std::vector<std::filesystem::path> pof_files;
	{
		SCP_string entries = env;
		size_t pos = 0;
		while (pos <= entries.size()) {
			size_t next = entries.find(';', pos);
			SCP_string entry = entries.substr(pos, next == SCP_string::npos ? SCP_string::npos : next - pos);
			if (!entry.empty()) {
				auto found = collect_pof_files(entry);
				pof_files.insert(pof_files.end(), found.begin(), found.end());
			}
			if (next == SCP_string::npos)
				break;
			pos = next + 1;
		}
	}
	ASSERT_FALSE(pof_files.empty()) << "FSO_BVH_PARITY_POF resolved to zero .pof files: " << env;

	std::set<std::string> roots_added;
	for (const auto& f : pof_files) {
		std::string root = find_mod_root(f).string();
		if (roots_added.insert(root).second)
			cf_add_external_path_root(root.c_str());
	}

	// Build the BVH for every submodel loaded from here on, alongside the legacy BSP tree, so
	// both traversals are available to benchmark against the same loaded content.
	Cmdline_use_bvh_collision = true;

	std::mt19937 rng(20260829);
	std::vector<RayCase> cases;
	int files_loaded = 0;
	long total_triangles = 0;

	for (const auto& pof_path : pof_files) {
		SCP_string filename = pof_path.filename().string();
		int model_num = model_load(filename.c_str(), nullptr, ErrorType::WARNING);
		if (model_num < 0)
			continue;
		polymodel* pm = model_get(model_num);
		if (pm == nullptr)
			continue;
		files_loaded++;

		for (int sm_idx = 0; sm_idx < pm->n_models; ++sm_idx) {
			bsp_info& sm = pm->submodel[sm_idx];
			if (sm.flags[Model::Submodel_flags::No_collisions] || sm.flags[Model::Submodel_flags::Nocollide_this_only])
				continue;
			if (!sm.bvh)
				continue;

			total_triangles += static_cast<long>(sm.bvh->items.size());

			vec3d center = (sm.min + sm.max) * 0.5f;
			float radius = std::max(sm.collision_rad, 1.0f);
			std::uniform_real_distribution<float> unit_dist(-1.0f, 1.0f);
			std::uniform_real_distribution<float> bbox_t(0.0f, 1.0f);

			// A smaller per-submodel ray count than the correctness tests -- this benchmark
			// already spans every loaded submodel, and each case gets replayed many times below.
			for (int r = 0; r < 20; ++r) {
				vec3d dir_to_origin = vm_vec_new(unit_dist(rng), unit_dist(rng), unit_dist(rng));
				if (vm_vec_mag_squared(&dir_to_origin) < 1e-6f)
					continue;
				vm_vec_normalize(&dir_to_origin);

				RayCase c;
				c.model_num = model_num;
				c.submodel_num = sm_idx;
				c.origin = center + dir_to_origin * (radius * 3.0f);
				c.target = vm_vec_new(sm.min.xyz.x + bbox_t(rng) * (sm.max.xyz.x - sm.min.xyz.x),
					sm.min.xyz.y + bbox_t(rng) * (sm.max.xyz.y - sm.min.xyz.y),
					sm.min.xyz.z + bbox_t(rng) * (sm.max.xyz.z - sm.min.xyz.z));
				cases.push_back(c);
			}
		}
	}

	ASSERT_GT(files_loaded, 0) << "No .pof file loaded successfully";
	ASSERT_FALSE(cases.empty()) << "No submodel produced any ray cases -- nothing to benchmark";

	printf("Loaded %d/%d files, %d ray cases, %ld total leaf/triangle primitives across all BVHs\n", files_loaded,
		static_cast<int>(pof_files.size()), static_cast<int>(cases.size()), total_triangles);

	auto run_all = [&](bool use_bvh) {
		Cmdline_use_bvh_collision = use_bvh;
		for (const RayCase& c : cases) {
			mc_info mc;
			mc.model_num = c.model_num;
			mc.submodel_num = c.submodel_num;
			mc.orient = &vmd_identity_matrix;
			mc.pos = &vmd_zero_vector;
			mc.p0 = &c.origin;
			mc.p1 = &c.target;
			mc.flags = MC_CHECK_MODEL | MC_SUBMODEL | MC_CHECK_RAY | MC_CHECK_INVISIBLE_FACES;
			model_collide(&mc);
		}
	};

	// Warm up both paths once (page faults, allocator warm-up, branch predictor) before timing.
	run_all(false);
	run_all(true);

	constexpr int TRIALS = 9;
	std::vector<double> old_ms, new_ms;
	old_ms.reserve(TRIALS);
	new_ms.reserve(TRIALS);

	// Alternate old/new trials rather than measuring all-old-then-all-new, so a transient system
	// hiccup (background process, thermal ramp) doesn't bias one side more than the other.
	for (int trial = 0; trial < TRIALS; ++trial) {
		auto t0 = std::chrono::steady_clock::now();
		run_all(false);
		auto t1 = std::chrono::steady_clock::now();
		run_all(true);
		auto t2 = std::chrono::steady_clock::now();

		old_ms.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
		new_ms.push_back(std::chrono::duration<double, std::milli>(t2 - t1).count());
	}

	Cmdline_use_bvh_collision = true; // restore default for anything else sharing the process

	double old_med = median(old_ms), new_med = median(new_ms);
	double old_min = *std::min_element(old_ms.begin(), old_ms.end());
	double new_min = *std::min_element(new_ms.begin(), new_ms.end());
	long calls = static_cast<long>(cases.size());

	printf("\n=== BVH vs BSP submodel-collision benchmark (%d trials, %ld calls/trial) ===\n", TRIALS, calls);
	printf("%-12s %10s %10s %14s %14s\n", "", "median ms", "min ms", "median ns/call", "min ns/call");
	printf("%-12s %10.3f %10.3f %14.1f %14.1f\n", "BSP (old)", old_med, old_min, old_med * 1e6 / calls, old_min * 1e6 / calls);
	printf("%-12s %10.3f %10.3f %14.1f %14.1f\n", "BVH (new)", new_med, new_min, new_med * 1e6 / calls, new_min * 1e6 / calls);
	printf("Speedup (median): %.2fx    Speedup (min): %.2fx\n", old_med / new_med, old_min / new_min);

	SUCCEED() << "See printed output above for timing results -- this test only measures, it does not assert on speed.";
}
