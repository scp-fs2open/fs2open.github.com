// Real-content coverage for MC_COLLIDE_ALL against the stage-4 triangle-BVH path -- closes a gap
// explicitly flagged as deferred-but-untested in the stage-4 plan (nifty-twirling-teacup.md):
// MC_COLLIDE_ALL was never exercised even once against model_collide_bvh_triangle() before this.
//
// MC_COLLIDE_ALL's one production consumer is volumetric_nebula::renderVolumeBitmap()
// (code/nebula/volumetrics.cpp), which fires whole-model (not per-submodel) rays and voxelizes the
// hull by ray-parity: it only cares whether mc.hit_points_all.size() is even or odd along each
// sampled vertical ray (see the "Annoying hack" retry loop there for odd counts). So the property
// that actually matters here is hit-count *parity* agreement between the legacy and triangle-BVH
// paths, not point-for-point identity -- this test measures exactly that, documenting the
// divergence rate rather than asserting near-zero (per the plan's own accepted design: "accept and
// cover with a documented-divergence unit test, not a parity assertion").
//
// Skipped by default (no .pof in-repo); point FSO_BVH_PARITY_POF at a real .pof file or directory
// (or several, ';'-separated) to run it.

#include <gtest/gtest.h>

#include <cfile/cfilesystem.h>
#include <cmdline/cmdline.h>
#include <math/vecmat.h>
#include <model/model.h>

#include "util/FSTestFixture.h"

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

} // namespace

class BvhTriangleCollideAllTest : public test::FSTestFixture {
protected:
	BvhTriangleCollideAllTest() : FSTestFixture(INIT_CFILE | INIT_GRAPHICS) {}
};

TEST_F(BvhTriangleCollideAllTest, HitCountParityAgreesWithLegacy)
{
	const char* env = std::getenv("FSO_BVH_PARITY_POF");
	if (env == nullptr || env[0] == '\0') {
		GTEST_SKIP() << "FSO_BVH_PARITY_POF not set; skipping MC_COLLIDE_ALL coverage check.";
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

	// Whole-model rays (no MC_SUBMODEL), matching renderVolumeBitmap()'s own usage exactly --
	// this recurses through the full submodel hierarchy from detail[0], a code path none of the
	// other stage-4 real-content tests exercise (they're all MC_SUBMODEL-scoped to one submodel).
	Cmdline_use_triangle_collision = true;

	std::mt19937 rng(20260829);

	long total_rays = 0, parity_mismatches = 0, exact_count_mismatches = 0;
	int files_loaded = 0;

	for (const auto& pof_path : pof_files) {
		SCP_string filename = pof_path.filename().string();
		int model_num = model_load(filename.c_str(), nullptr, ErrorType::WARNING);
		if (model_num < 0)
			continue;
		polymodel* pm = model_get(model_num);
		if (pm == nullptr)
			continue;
		files_loaded++;

		vec3d size = pm->maxs - pm->mins;
		vec3d center = (pm->maxs + pm->mins) * 0.5f;
		float radius = std::max(pm->rad, 1.0f);

		long file_parity_mismatches = 0;

		// Vertical rays through the model's footprint, mirroring renderVolumeBitmap()'s own
		// sampling shape (fixed X/Y, sweeping the full Z extent) -- the actual query pattern the
		// one production consumer uses, not an arbitrary random-ray shape.
		std::uniform_real_distribution<float> frac(0.0f, 1.0f);
		for (int r = 0; r < 100; ++r) {
			vec3d start = pm->mins;
			start.xyz.x += frac(rng) * size.xyz.x;
			start.xyz.y += frac(rng) * size.xyz.y;
			start.xyz.z = pm->mins.xyz.z - radius * 0.1f;
			vec3d end = start;
			end.xyz.z = pm->maxs.xyz.z + radius * 0.1f;

			auto run = [&](bool use_triangle_bvh) {
				Cmdline_use_triangle_collision = use_triangle_bvh;
				mc_info mc;
				mc.model_num = model_num;
				mc.orient = &vmd_identity_matrix;
				mc.pos = &vmd_zero_vector;
				mc.p0 = &start;
				mc.p1 = &end;
				mc.flags = MC_CHECK_MODEL | MC_COLLIDE_ALL | MC_CHECK_INVISIBLE_FACES;
				model_collide(&mc);
				return mc.hit_points_all.size();
			};

			size_t old_count = run(false);
			size_t new_count = run(true);
			Cmdline_use_triangle_collision = true; // restore for the next model's triangle_bvh lookups

			total_rays++;
			if ((old_count % 2) != (new_count % 2)) {
				parity_mismatches++;
				file_parity_mismatches++;
			}
			if (old_count != new_count) {
				exact_count_mismatches++;
			}
		}

		(void)center;
		if (file_parity_mismatches > 0) {
			printf("%s: %ld/100 rays had a hit-count PARITY mismatch (the property renderVolumeBitmap() actually depends on)\n",
				filename.c_str(), file_parity_mismatches);
		}
	}

	Cmdline_use_triangle_collision = false;

	printf("Summary: %d/%d files loaded, %ld whole-model MC_COLLIDE_ALL rays\n", files_loaded, static_cast<int>(pof_files.size()),
		total_rays);
	printf("  Exact hit-count mismatches: %ld/%ld (%.3f%%)\n", exact_count_mismatches, total_rays,
		total_rays ? 100.0 * exact_count_mismatches / total_rays : 0.0);
	printf("  PARITY mismatches (even/odd -- what renderVolumeBitmap() actually needs): %ld/%ld (%.3f%%)\n", parity_mismatches,
		total_rays, total_rays ? 100.0 * parity_mismatches / total_rays : 0.0);

	ASSERT_GT(files_loaded, 0) << "No .pof file loaded successfully";

	// Documented-divergence test, not a strict parity assertion (per the stage-4 plan's own design
	// for this deferred item) -- MC_COLLIDE_ALL is hit-count-sensitive on non-planar/degenerate
	// geometry by nature, on both the legacy and triangle-BVH paths independently. This is a loose
	// sanity bound against a real regression (e.g. a systematic off-by-one in the triangle
	// traversal), not a claim that near-zero divergence is required for correctness.
	if (total_rays > 0) {
		double parity_rate = static_cast<double>(parity_mismatches) / static_cast<double>(total_rays);
		EXPECT_LT(parity_rate, 0.05) << parity_mismatches << "/" << total_rays << " rays had a hit-count parity mismatch";
	}
}
