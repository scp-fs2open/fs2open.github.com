// Measures model_load() time with the new per-triangle BVH construction on vs. off, per real POF,
// to isolate exactly how much extra load-time cost building the new collision data structure adds.
// The diff (new - old) is that construction cost -- everything else about model_load() (BSP parse,
// vertex buffers, etc.) is identical between the two runs since Cmdline_use_triangle_collision only
// gates the one extra build step in modelread.cpp (verified: the triangle_bvh build is inside an
// `if (Cmdline_use_triangle_collision) { ... }` block, so it is not constructed at all when the
// flag is off -- this test also asserts that directly, not just assumes it).
//
// Skipped by default (no .pof in-repo); point FSO_BVH_PARITY_POF at a real .pof file or directory
// (or several, ';'-separated) to run it.

#include <gtest/gtest.h>

#include <cfile/cfilesystem.h>
#include <cmdline/cmdline.h>
#include <model/model.h>

#include "util/FSTestFixture.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <set>
#include <string>
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

double median(std::vector<double> v)
{
	std::sort(v.begin(), v.end());
	size_t n = v.size();
	if (n == 0)
		return 0.0;
	return (n % 2) ? v[n / 2] : (v[n / 2 - 1] + v[n / 2]) * 0.5;
}

struct LoadTimeResult {
	std::string filename;
	double old_ms = 0.0;
	double new_ms = 0.0;
	double diff_ms = 0.0;
	double relative_pct = 0.0;
};

} // namespace

class ModelLoadTimeTest : public test::FSTestFixture {
protected:
	ModelLoadTimeTest() : FSTestFixture(INIT_CFILE | INIT_GRAPHICS) {}
};

TEST_F(ModelLoadTimeTest, TriangleBvhConstructionCost)
{
	const char* env = std::getenv("FSO_BVH_PARITY_POF");
	if (env == nullptr || env[0] == '\0') {
		GTEST_SKIP() << "FSO_BVH_PARITY_POF not set; skipping model load-time measurement.";
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

	constexpr int TRIALS = 3;
	std::vector<LoadTimeResult> results;
	int files_measured = 0;
	int triangle_bvh_built_when_off = 0; // sanity check: must stay 0

	for (const auto& pof_path : pof_files) {
		SCP_string filename = pof_path.filename().string();

		// One warm-up load+unload per file (both flag states) so first-touch disk I/O doesn't skew
		// the timed trials -- by this point in the session these files are almost certainly already
		// in the OS file cache anyway, but this keeps the measurement honest either way.
		Cmdline_use_triangle_collision = false;
		{
			int mn = model_load(filename.c_str(), nullptr, ErrorType::WARNING, true);
			if (mn >= 0)
				model_unload(mn, 1);
		}
		Cmdline_use_triangle_collision = true;
		{
			int mn = model_load(filename.c_str(), nullptr, ErrorType::WARNING, true);
			if (mn >= 0)
				model_unload(mn, 1);
		}

		std::vector<double> old_ms, new_ms;
		bool load_failed = false;

		for (int trial = 0; trial < TRIALS; ++trial) {
			// OLD: triangle-BVH construction off.
			Cmdline_use_triangle_collision = false;
			auto t0 = std::chrono::steady_clock::now();
			int model_num_old = model_load(filename.c_str(), nullptr, ErrorType::WARNING, true);
			auto t1 = std::chrono::steady_clock::now();
			if (model_num_old < 0) {
				load_failed = true;
				break;
			}
			polymodel* pm_old = model_get(model_num_old);
			if (pm_old != nullptr) {
				for (int sm_idx = 0; sm_idx < pm_old->n_models; ++sm_idx) {
					// Sanity check, not just assumed: the flag must actually gate construction.
					EXPECT_EQ(pm_old->submodel[sm_idx].triangle_bvh, nullptr)
						<< filename << " submodel " << sm_idx << " has a triangle_bvh even though Cmdline_use_triangle_collision was false";
					if (pm_old->submodel[sm_idx].triangle_bvh)
						triangle_bvh_built_when_off++;
				}
			}
			model_unload(model_num_old, 1);
			old_ms.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());

			// NEW: triangle-BVH construction on.
			Cmdline_use_triangle_collision = true;
			auto t2 = std::chrono::steady_clock::now();
			int model_num_new = model_load(filename.c_str(), nullptr, ErrorType::WARNING, true);
			auto t3 = std::chrono::steady_clock::now();
			if (model_num_new < 0) {
				load_failed = true;
				break;
			}
			model_unload(model_num_new, 1);
			new_ms.push_back(std::chrono::duration<double, std::milli>(t3 - t2).count());
		}

		Cmdline_use_triangle_collision = false; // restore default between files

		if (load_failed || old_ms.empty() || new_ms.empty())
			continue;

		files_measured++;

		LoadTimeResult r;
		r.filename = filename;
		r.old_ms = median(old_ms);
		r.new_ms = median(new_ms);
		r.diff_ms = r.new_ms - r.old_ms;
		r.relative_pct = (r.old_ms > 1e-6) ? (r.diff_ms / r.old_ms) * 100.0 : 0.0;
		results.push_back(r);
	}

	ASSERT_GT(files_measured, 0) << "No .pof file could be measured";
	EXPECT_EQ(triangle_bvh_built_when_off, 0)
		<< "triangle_bvh was constructed at least once while Cmdline_use_triangle_collision was false -- the flag is not "
		   "correctly gating construction";

	// Full table, sorted by filename for readability.
	std::vector<LoadTimeResult> by_name = results;
	std::sort(by_name.begin(), by_name.end(), [](const LoadTimeResult& a, const LoadTimeResult& b) { return a.filename < b.filename; });

	printf("\n=== Model load time: old (legacy-only) vs new (+ triangle-BVH construction), median of %d trials ===\n", TRIALS);
	printf("%-32s %10s %10s %10s %10s\n", "POF", "old ms", "new ms", "diff ms", "diff %");
	double total_old = 0.0, total_new = 0.0;
	for (const auto& r : by_name) {
		printf("%-32s %10.3f %10.3f %10.3f %9.1f%%\n", r.filename.c_str(), r.old_ms, r.new_ms, r.diff_ms, r.relative_pct);
		total_old += r.old_ms;
		total_new += r.new_ms;
	}
	printf("%-32s %10.3f %10.3f %10.3f %9.1f%%\n", "TOTAL", total_old, total_new, total_new - total_old,
		total_old > 1e-6 ? (total_new - total_old) / total_old * 100.0 : 0.0);

	// Top 5 by absolute increase.
	std::vector<LoadTimeResult> by_abs = results;
	std::sort(by_abs.begin(), by_abs.end(), [](const LoadTimeResult& a, const LoadTimeResult& b) { return a.diff_ms > b.diff_ms; });
	printf("\n=== Top 5 by ABSOLUTE load-time increase ===\n");
	printf("%-32s %10s %10s %10s %10s\n", "POF", "old ms", "new ms", "diff ms", "diff %");
	for (size_t i = 0; i < std::min<size_t>(5, by_abs.size()); ++i) {
		const auto& r = by_abs[i];
		printf("%-32s %10.3f %10.3f %10.3f %9.1f%%\n", r.filename.c_str(), r.old_ms, r.new_ms, r.diff_ms, r.relative_pct);
	}

	// Top 5 by relative increase.
	std::vector<LoadTimeResult> by_rel = results;
	std::sort(by_rel.begin(), by_rel.end(), [](const LoadTimeResult& a, const LoadTimeResult& b) { return a.relative_pct > b.relative_pct; });
	printf("\n=== Top 5 by RELATIVE load-time increase ===\n");
	printf("%-32s %10s %10s %10s %10s\n", "POF", "old ms", "new ms", "diff ms", "diff %");
	for (size_t i = 0; i < std::min<size_t>(5, by_rel.size()); ++i) {
		const auto& r = by_rel[i];
		printf("%-32s %10.3f %10.3f %10.3f %9.1f%%\n", r.filename.c_str(), r.old_ms, r.new_ms, r.diff_ms, r.relative_pct);
	}

	printf("\nSummary: %d files measured, total load time %.2fms -> %.2fms (%.2fms, %.1f%% overall)\n", files_measured, total_old,
		total_new, total_new - total_old, total_old > 1e-6 ? (total_new - total_old) / total_old * 100.0 : 0.0);

	SUCCEED();
}
