// Direct parity check between the legacy BSP collision traversal and the new stage-3 BVH-based
// traversal (see model_collide_bvh()/mc_check_bvh_leaf() in modelcollide.cpp), both reached
// through the real model_collide() entry point -- not an independent oracle like
// test_modelbvh_parity.cpp (stage 2). Cmdline_use_bvh_collision is toggled live around each call,
// so the exact same ray is tested through both code paths in the same run.
//
// Skipped by default (no .pof in-repo, see test_modelbvh_parity.cpp for why); point
// FSO_BVH_PARITY_POF at a real .pof file or directory (or several, ';'-separated) to run it.

#include <gtest/gtest.h>

#include <cfile/cfilesystem.h>
#include <cmdline/cmdline.h>
#include <math/vecmat.h>
#include <model/model.h>

#include "util/FSTestFixture.h"

#include <cfloat>
#include <cmath>
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

// NOTE: deliberately does NOT compare num_hits for equality. mc_check_face()/
// mc_check_sphereline_face() increment it once per polygon test that both succeeds AND improves
// on the current best-so-far *at the time it's tested* -- it's an order-dependent bookkeeping
// counter of the traversal's internal history, not a stable "did we hit anything" flag. Since the
// BVH visits leaves in a structurally different order than the BSP tree, the two paths can (and
// routinely do) log a different number of intermediate record-setting hits while still converging
// on the exact same final nearest hit. Only whether a hit was found at all, and the winning hit's
// own fields, are the order-independent, actually-observable result worth comparing.
bool mc_info_matches(const mc_info& a, const mc_info& b, float tol, std::string& why)
{
	if ((a.num_hits > 0) != (b.num_hits > 0)) {
		why = "hit/miss disagreement: num_hits " + std::to_string(a.num_hits) + " vs " + std::to_string(b.num_hits);
		return false;
	}
	if (a.num_hits == 0)
		return true;

	if (std::fabs(a.hit_dist - b.hit_dist) > tol) {
		why = "hit_dist " + std::to_string(a.hit_dist) + " vs " + std::to_string(b.hit_dist);
		return false;
	}
	if (vm_vec_dist(&a.hit_point, &b.hit_point) > tol) {
		why = "hit_point differs";
		return false;
	}
	if (a.hit_submodel != b.hit_submodel) {
		why = "hit_submodel " + std::to_string(a.hit_submodel) + " vs " + std::to_string(b.hit_submodel);
		return false;
	}
	if (a.hit_bitmap != b.hit_bitmap) {
		why = "hit_bitmap " + std::to_string(a.hit_bitmap) + " vs " + std::to_string(b.hit_bitmap);
		return false;
	}
	if (vm_vec_dist(&a.hit_normal, &b.hit_normal) > 0.01f) {
		why = "hit_normal differs";
		return false;
	}
	return true;
}

} // namespace

class BvhTraversalParityTest : public test::FSTestFixture {
protected:
	BvhTraversalParityTest() : FSTestFixture(INIT_CFILE | INIT_GRAPHICS) {}
};

TEST_F(BvhTraversalParityTest, OldAndNewTraversalAgree)
{
	const char* env = std::getenv("FSO_BVH_PARITY_POF");
	if (env == nullptr || env[0] == '\0') {
		GTEST_SKIP() << "FSO_BVH_PARITY_POF not set; skipping traversal-parity check.";
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

	// Build the BVH for every submodel loaded from here on, alongside the legacy BSP tree.
	Cmdline_use_bvh_collision = true;

	std::mt19937 rng(20260829);

	long total_rays = 0, total_mismatches = 0;
	int files_loaded = 0, submodels_with_bvh = 0;

	for (const auto& pof_path : pof_files) {
		SCP_string filename = pof_path.filename().string();
		int model_num = model_load(filename.c_str(), nullptr, ErrorType::WARNING);
		if (model_num < 0) {
			printf("Skipping (load failed): %s\n", filename.c_str());
			continue;
		}
		polymodel* pm = model_get(model_num);
		if (pm == nullptr)
			continue;

		files_loaded++;
		long file_rays = 0, file_mismatches = 0;

		for (int sm_idx = 0; sm_idx < pm->n_models; ++sm_idx) {
			bsp_info& sm = pm->submodel[sm_idx];
			if (sm.flags[Model::Submodel_flags::No_collisions] || sm.flags[Model::Submodel_flags::Nocollide_this_only])
				continue;
			if (!sm.bvh)
				continue; // no geometry, or extraction produced nothing -- nothing to compare

			submodels_with_bvh++;

			vec3d center = (sm.min + sm.max) * 0.5f;
			float radius = std::max(sm.collision_rad, 1.0f);
			std::uniform_real_distribution<float> unit_dist(-1.0f, 1.0f);
			std::uniform_real_distribution<float> bbox_t(0.0f, 1.0f);

			for (int r = 0; r < 300; ++r) {
				vec3d dir_to_origin = vm_vec_new(unit_dist(rng), unit_dist(rng), unit_dist(rng));
				if (vm_vec_mag_squared(&dir_to_origin) < 1e-6f)
					continue;
				vm_vec_normalize(&dir_to_origin);
				vec3d origin = center + dir_to_origin * (radius * 3.0f);

				vec3d target = vm_vec_new(sm.min.xyz.x + bbox_t(rng) * (sm.max.xyz.x - sm.min.xyz.x),
					sm.min.xyz.y + bbox_t(rng) * (sm.max.xyz.y - sm.min.xyz.y),
					sm.min.xyz.z + bbox_t(rng) * (sm.max.xyz.z - sm.min.xyz.z));

				auto run = [&](bool use_bvh) {
					Cmdline_use_bvh_collision = use_bvh;
					mc_info mc;
					mc.model_num = model_num;
					mc.submodel_num = sm_idx;
					mc.orient = &vmd_identity_matrix;
					mc.pos = &vmd_zero_vector;
					mc.p0 = &origin;
					mc.p1 = &target;
					mc.flags = MC_CHECK_MODEL | MC_SUBMODEL | MC_CHECK_RAY | MC_CHECK_INVISIBLE_FACES;
					model_collide(&mc);
					return mc;
				};

				mc_info mc_old = run(false);
				mc_info mc_new = run(true);
				Cmdline_use_bvh_collision = true; // restore for the next submodel's BVH lookups

				file_rays++;
				total_rays++;

				std::string why;
				if (!mc_info_matches(mc_old, mc_new, 1e-2f, why)) {
					file_mismatches++;
					total_mismatches++;
					if (file_mismatches <= 5) {
						printf("  [submodel %d ('%s')] mismatch: %s\n", sm_idx, sm.name, why.c_str());
					}
				}
			}
		}

		if (file_mismatches > 0) {
			printf("%s: %ld rays, %ld mismatches\n", filename.c_str(), file_rays, file_mismatches);
		}
	}

	Cmdline_use_bvh_collision = false; // don't leak this into other tests sharing the process

	printf("Summary: %d/%d files loaded, %d submodels with a BVH, %ld total rays, %ld mismatches\n", files_loaded,
		static_cast<int>(pof_files.size()), submodels_with_bvh, total_rays, total_mismatches);

	ASSERT_GT(files_loaded, 0) << "No .pof file loaded successfully";
	ASSERT_GT(submodels_with_bvh, 0) << "No submodel had a BVH built -- extraction or the load hook is likely broken";

	// Both traversals run the exact same mc_check_face()/mc_check_sphereline_face() math on the
	// exact same leaves -- only the spatial index and visit order differ. A near-zero mismatch
	// rate is expected regardless: two coincident/near-duplicate leaves at (within tolerance) the
	// same hit distance can legitimately be "won" by either one depending on traversal order,
	// same class of tie as the benign near-tied-geometry cases already characterized in
	// collision_bvh_rewrite_plan's stage-2/3 golden-parity notes. A real bug in the new path would
	// show as a much higher rate (see BvhTraversalParityTest's development history: an initial
	// num_hits-based comparison mistake showed ~13% before being corrected to compare only the
	// order-independent winning-hit fields, at which point the true rate was already this low).
	if (total_rays > 0) {
		double mismatch_rate = static_cast<double>(total_mismatches) / static_cast<double>(total_rays);
		EXPECT_LT(mismatch_rate, 0.001) << total_mismatches << "/" << total_rays
			<< " rays disagreed between the legacy BSP traversal and the new BVH traversal";
	}
}
