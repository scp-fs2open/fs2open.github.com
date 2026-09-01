// Benchmarks the real, gameplay-shaped collision query: MC_CHECK_MODEL with no MC_SUBMODEL (full
// multi-submodel recursion from detail[0], same as weapon-vs-ship-hull, AI's
// ai_big_pick_attack_point(), turret hull-blocking checks, and beam-vs-ship), with a bounded
// segment whose length is ~1-2x the model's own radius -- matching
// check_inside_radius_for_big_ships()'s own geometry (code/object/collideshipweapon.cpp), which is
// exactly the case that dominates real per-frame collision cost against capital-ship-scale models
// with 100+ submodels (see collision_bvh_rewrite_plan project notes, 2026-08-31 investigation).
//
// This is deliberately a *different* query shape from test_modelbvh_triangle_benchmark.cpp, which
// uses MC_SUBMODEL to scope every call to exactly one already-known submodel's own tree (matching
// e.g. will_collide_pp()) -- that shape can never exercise mc_check_subobj()'s submodel-level
// early-out (modelcollide.cpp, the `vm_vec_dist(&Mc_p0, &hitpt) >= Mc->hit_dist` check added
// 2026-08-31), since there's only ever one submodel in play per call. This test's whole point is to
// give that specific optimization something real to prune against.
//
// Skipped by default (no .pof in-repo); point FSO_BVH_PARITY_POF at a real .pof file or directory
// (or several, ';'-separated) to run it.

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

class BvhSubmodelPruneBenchmarkTest : public test::FSTestFixture {
protected:
	BvhSubmodelPruneBenchmarkTest() : FSTestFixture(INIT_CFILE | INIT_GRAPHICS) {}
};

TEST_F(BvhSubmodelPruneBenchmarkTest, CompareOldAndTriangleBvhWholeModelSpeed)
{
	const char* env = std::getenv("FSO_BVH_PARITY_POF");
	if (env == nullptr || env[0] == '\0') {
		GTEST_SKIP() << "FSO_BVH_PARITY_POF not set; skipping whole-model submodel-recursion benchmark.";
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

	Cmdline_use_triangle_collision = true;

	std::mt19937 rng(20260831);
	std::vector<RayCase> cases;
	int files_loaded = 0;
	int files_used = 0;

	for (const auto& pof_path : pof_files) {
		SCP_string filename = pof_path.filename().string();
		int model_num = model_load(filename.c_str(), nullptr, ErrorType::WARNING);
		if (model_num < 0)
			continue;
		polymodel* pm = model_get(model_num);
		if (pm == nullptr)
			continue;
		files_loaded++;

		// Only models with real multi-submodel structure are relevant to this specific
		// optimization -- a single-submodel prop/debris POF can never exercise cross-submodel
		// pruning at all, so including them would just dilute the measurement with noise.
		if (pm->n_models < 20)
			continue;
		files_used++;

		vec3d center = (pm->mins + pm->maxs) * 0.5f;
		float radius = std::max(pm->rad, 1.0f);
		std::uniform_real_distribution<float> unit_dist(-1.0f, 1.0f);

		// Mirrors check_inside_radius_for_big_ships()'s own geometry
		// (code/object/collideshipweapon.cpp): a bounded segment from just outside the model,
		// through roughly the center, to just past the far side -- length on the order of the
		// model's own radius, exactly the regime the real-content investigation (2026-08-31)
		// found dominates weapon-vs-capital-ship collision cost.
		for (int r = 0; r < 15; ++r) {
			vec3d dir = vm_vec_new(unit_dist(rng), unit_dist(rng), unit_dist(rng));
			if (vm_vec_mag_squared(&dir) < 1e-6f)
				continue;
			vm_vec_normalize(&dir);

			RayCase c;
			c.model_num = model_num;
			c.origin = center - dir * (radius * 1.1f);
			c.target = center + dir * (radius * 1.1f);
			cases.push_back(c);
		}
	}

	ASSERT_GT(files_loaded, 0) << "No .pof file loaded successfully";
	ASSERT_GT(files_used, 0) << "No multi-submodel (n_models >= 20) POF found -- nothing to benchmark";
	ASSERT_FALSE(cases.empty()) << "No case produced -- nothing to benchmark";

	printf("Loaded %d/%d files, %d used (n_models >= 20), %d whole-model bounded-segment cases\n", files_loaded,
		static_cast<int>(pof_files.size()), files_used, static_cast<int>(cases.size()));

	auto run_all = [&](bool use_triangle_bvh) {
		Cmdline_use_triangle_collision = use_triangle_bvh;
		for (const RayCase& c : cases) {
			mc_info mc;
			mc.model_num = c.model_num;
			mc.orient = &vmd_identity_matrix;
			mc.pos = &vmd_zero_vector;
			mc.p0 = &c.origin;
			mc.p1 = &c.target;
			// No MC_SUBMODEL: full recursion from detail[0], matching real weapon-vs-hull/AI/beam
			// callers. No MC_CHECK_RAY: bounded to [p0, p1], matching
			// check_inside_radius_for_big_ships()'s own bounded segment.
			mc.flags = MC_CHECK_MODEL | MC_CHECK_INVISIBLE_FACES;
			model_collide(&mc);
		}
	};

	run_all(false);
	run_all(true);

	constexpr int TRIALS = 9;
	std::vector<double> old_ms, new_ms;
	old_ms.reserve(TRIALS);
	new_ms.reserve(TRIALS);

	for (int trial = 0; trial < TRIALS; ++trial) {
		auto t0 = std::chrono::steady_clock::now();
		run_all(false);
		auto t1 = std::chrono::steady_clock::now();
		run_all(true);
		auto t2 = std::chrono::steady_clock::now();

		old_ms.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
		new_ms.push_back(std::chrono::duration<double, std::milli>(t2 - t1).count());
	}

	Cmdline_use_triangle_collision = false;

	double old_med = median(old_ms), new_med = median(new_ms);
	double old_min = *std::min_element(old_ms.begin(), old_ms.end());
	double new_min = *std::min_element(new_ms.begin(), new_ms.end());
	long calls = static_cast<long>(cases.size());

	printf("\n=== Whole-model (no MC_SUBMODEL) bounded-segment benchmark, BVH(triangle) vs BSP (%d trials, %ld calls/trial) ===\n",
		TRIALS, calls);
	printf("%-16s %10s %10s %14s %14s\n", "", "median ms", "min ms", "median ns/call", "min ns/call");
	printf("%-16s %10.3f %10.3f %14.1f %14.1f\n", "BSP (old)", old_med, old_min, old_med * 1e6 / calls, old_min * 1e6 / calls);
	printf("%-16s %10.3f %10.3f %14.1f %14.1f\n", "BVH-triangle", new_med, new_min, new_med * 1e6 / calls, new_min * 1e6 / calls);
	printf("Speedup (median): %.2fx    Speedup (min): %.2fx\n", old_med / new_med, old_min / new_min);

	SUCCEED() << "See printed output above for timing results -- this test only measures, it does not assert on speed.";
}
