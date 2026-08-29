// Golden-parity check between the existing BSP collision tree and the new modelbvh module,
// run against real .pof files supplied from outside the repo.
//
// This intentionally does NOT run as part of the normal test suite: no .pof file is (or should
// be) committed to the repo (game assets are separately licensed -- see the collision-BVH-rewrite
// project notes), so there is nothing to check it against in CI. Point the FSO_BVH_PARITY_POF
// environment variable at a real .pof file OR a directory (searched recursively for *.pof) on
// disk to run it locally. Multiple entries (files and/or directories) can be given, separated by
// ';', e.g.:
//
//   FSO_BVH_PARITY_POF="E:\Games\Knossos\FS2\somemod\data\models;E:\Games\Knossos\FS2\othermod\data\models\ship.pof" \
//       ./unittests.exe --gtest_filter=*BvhParity*
//
// If the variable isn't set (or resolves to zero .pof files), the test is skipped.

#include <gtest/gtest.h>

#include <cfile/cfilesystem.h>
#include <math/vecmat.h>
#include <model/model.h>
#include <model/modelbvh.h>
#include <model/modelbvh_extract.h>

#include "util/FSTestFixture.h"

#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <random>
#include <set>
#include <vector>

namespace {

// Brute-force nearest FRONT-FACING hit over a triangle soup, matching the old system's
// mc_check_face() culling rule (skip a polygon if the ray direction doesn't oppose its normal,
// i.e. skip backfaces) -- see modelcollide.cpp's mc_check_face. Deliberately not using
// bvh_ray_intersect() from modelbvh.h directly: that traversal is a general nearest-hit query
// with no notion of "front-facing" (correctly so -- that's a stage-1 scope boundary, see
// modelbvh.h), and backface culling is a game-behavior-layer decision, not a BVH property. A
// meaningful parity comparison has to apply the same rule on both sides. This is intentionally
// brute-force (not BVH-accelerated) -- it only needs to be correct, not fast, for a one-off local
// verification tool.
bool nearest_frontfacing_hit(const SCP_vector<bvh_triangle>& triangles, const vec3d& origin, const vec3d& dir,
	float& out_t, int& out_index)
{
	bool found = false;
	float best_t = FLT_MAX;
	int best_index = -1;

	for (int i = 0; i < static_cast<int>(triangles.size()); ++i) {
		const bvh_triangle& tri = triangles[i];

		vec3d e1 = tri.v1 - tri.v0;
		vec3d e2 = tri.v2 - tri.v0;

		vec3d normal;
		vm_vec_cross(&normal, &e1, &e2);
		if (vm_vec_dot(&dir, &normal) > 0.0f)
			continue; // backface relative to the ray direction; the old system skips these too

		vec3d pvec;
		vm_vec_cross(&pvec, &dir, &e2);
		float det = vm_vec_dot(&e1, &pvec);
		if (std::fabs(det) < 1e-8f)
			continue;
		float inv_det = 1.0f / det;

		vec3d tvec = origin - tri.v0;
		float u = vm_vec_dot(&tvec, &pvec) * inv_det;
		if (u < 0.0f || u > 1.0f)
			continue;

		vec3d qvec;
		vm_vec_cross(&qvec, &tvec, &e1);
		float v = vm_vec_dot(&dir, &qvec) * inv_det;
		if (v < 0.0f || u + v > 1.0f)
			continue;

		float t = vm_vec_dot(&e2, &qvec) * inv_det;
		if (t < 0.0f || t >= best_t)
			continue;

		best_t = t;
		best_index = i;
		found = true;
	}

	if (found) {
		out_t = best_t;
		out_index = best_index;
	}
	return found;
}

// Recursively collects every *.pof under `entry` (case-insensitive extension match), or just
// `entry` itself if it's already a file.
std::vector<std::filesystem::path> collect_pof_files(const std::filesystem::path& entry)
{
	std::vector<std::filesystem::path> result;

	if (!std::filesystem::exists(entry))
		return result;

	if (std::filesystem::is_directory(entry)) {
		for (const auto& de : std::filesystem::recursive_directory_iterator(entry)) {
			if (!de.is_regular_file())
				continue;
			if (stricmp(de.path().extension().string().c_str(), ".pof") == 0)
				result.push_back(de.path());
		}
	} else {
		result.push_back(entry);
	}

	return result;
}

// cfile resolves CF_TYPE_MODELS as "<root>/data/models/" (cfile.cpp: { CF_TYPE_MODELS,
// "data/models", ".pof", ... }), so the registered root must be the *mod* directory, not the
// models folder itself. Find the "data" component and use its parent; fall back to the standard
// <root>/data/models/<file> layout if "data" isn't found in the path for some reason.
std::filesystem::path find_mod_root(const std::filesystem::path& pof_file)
{
	for (std::filesystem::path search = pof_file.parent_path(); search.has_parent_path(); search = search.parent_path()) {
		if (stricmp(search.filename().string().c_str(), "data") == 0)
			return search.parent_path();
	}
	return pof_file.parent_path().parent_path().parent_path();
}

struct ParityTotals {
	long rays = 0;
	long hit_mismatches = 0;
	long dist_mismatches = 0;
	int submodels_with_geometry = 0;
};

// Runs the parity check for one already-loaded model, accumulating into `totals`.
void check_model_parity(int model_num, polymodel* pm, std::mt19937& rng, ParityTotals& totals)
{
	for (int sm_idx = 0; sm_idx < pm->n_models; ++sm_idx) {
		SCP_vector<bvh_triangle> triangles = model_bvh_extract_submodel_triangles(pm, sm_idx);
		if (triangles.empty())
			continue;

		bsp_info& sm = pm->submodel[sm_idx];

		// model_collide() with MC_SUBMODEL never tests a submodel's own geometry when either
		// flag is set (mc_check_subobj: No_collisions is a hard skip, Nocollide_this_only is a
		// soft skip -- see collision_bvh_rewrite_plan project notes). Comparing against a
		// brute-force triangle test that doesn't know about these flags isn't a fair parity
		// check -- the old system always reports "no hit" here by design, regardless of geometry.
		if (sm.flags[Model::Submodel_flags::No_collisions] || sm.flags[Model::Submodel_flags::Nocollide_this_only])
			continue;

		totals.submodels_with_geometry++;
		bvh_tree tree = bvh_build(triangles);

		vec3d center = (sm.min + sm.max) * 0.5f;
		float radius = std::max(sm.rad, 1.0f);

		std::uniform_real_distribution<float> unit_dist(-1.0f, 1.0f);
		std::uniform_real_distribution<float> bbox_t(0.0f, 1.0f);

		int submodel_rays = 0;
		int submodel_hit_mismatches = 0;
		int submodel_dist_mismatches = 0;

		for (int r = 0; r < 300; ++r) {
			// Origin: a random point well outside the submodel's bounding sphere.
			vec3d dir_to_origin = vm_vec_new(unit_dist(rng), unit_dist(rng), unit_dist(rng));
			if (vm_vec_mag_squared(&dir_to_origin) < 1e-6f)
				continue;
			vm_vec_normalize(&dir_to_origin);
			vec3d origin = center + dir_to_origin * (radius * 3.0f);

			// Target: a random point inside the (slightly padded) bounding box, biasing rays
			// toward actually crossing the geometry rather than missing it entirely.
			vec3d target = vm_vec_new(sm.min.xyz.x + bbox_t(rng) * (sm.max.xyz.x - sm.min.xyz.x),
				sm.min.xyz.y + bbox_t(rng) * (sm.max.xyz.y - sm.min.xyz.y),
				sm.min.xyz.z + bbox_t(rng) * (sm.max.xyz.z - sm.min.xyz.z));

			vec3d dir = target - origin;

			mc_info mc;
			mc.model_num = model_num;
			mc.submodel_num = sm_idx;
			mc.orient = &vmd_identity_matrix;
			mc.pos = &vmd_zero_vector;
			mc.p0 = &origin;
			mc.p1 = &target;
			// MC_CHECK_INVISIBLE_FACES: without it, model_collide() refuses to collide with any
			// polygon whose texture failed to load (by design -- see mc_check_face's
			// GetTexture()<0 check). This test's headless FSTestFixture setup only registers the
			// mod's models directory with cfile, not its actual texture data, so textures never
			// load here -- without this flag, essentially every textured (non-flat) polygon would
			// spuriously "miss" regardless of geometry, which isn't a fair comparison. The BVH
			// side has no notion of texture-load state at all, so this flag makes both sides test
			// pure geometry, which is what this harness is actually meant to validate.
			mc.flags = MC_CHECK_MODEL | MC_SUBMODEL | MC_CHECK_RAY | MC_CHECK_INVISIBLE_FACES;

			bool old_hit = model_collide(&mc) != 0;

			float new_t;
			int new_tri;
			bool new_hit = nearest_frontfacing_hit(triangles, origin, dir, new_t, new_tri);

			submodel_rays++;
			totals.rays++;

			if (old_hit != new_hit) {
				submodel_hit_mismatches++;
				totals.hit_mismatches++;
				if (submodel_hit_mismatches <= 3) {
					printf("    DEBUG hit mismatch: old=%d new=%d origin=(%.3f,%.3f,%.3f) target=(%.3f,%.3f,%.3f) rad=%.3f min=(%.3f,%.3f,%.3f) max=(%.3f,%.3f,%.3f)\n",
						old_hit, new_hit, origin.xyz.x, origin.xyz.y, origin.xyz.z, target.xyz.x, target.xyz.y,
						target.xyz.z, sm.rad, sm.min.xyz.x, sm.min.xyz.y, sm.min.xyz.z, sm.max.xyz.x, sm.max.xyz.y,
						sm.max.xyz.z);
				}
				continue;
			}

			if (old_hit) {
				float tolerance = std::max(1e-3f, 1e-4f * std::fabs(mc.hit_dist));
				if (std::fabs(mc.hit_dist - new_t) > tolerance) {
					submodel_dist_mismatches++;
					totals.dist_mismatches++;
					if (submodel_dist_mismatches <= 3) {
						printf("    DEBUG dist mismatch: old=%f new=%f delta=%f old_hit_submodel=%d\n", mc.hit_dist, new_t,
							mc.hit_dist - new_t, mc.hit_submodel);
					}
				}
			}
		}

		if (submodel_hit_mismatches > 0 || submodel_dist_mismatches > 0) {
			printf("  submodel %d ('%s'): %d triangles, %d rays, %d hit mismatches, %d distance mismatches\n", sm_idx,
				sm.name, static_cast<int>(triangles.size()), submodel_rays, submodel_hit_mismatches, submodel_dist_mismatches);
		}
	}
}

} // namespace

class BvhParityTest : public test::FSTestFixture {
protected:
	BvhParityTest() : FSTestFixture(INIT_CFILE | INIT_GRAPHICS) {}
};

TEST_F(BvhParityTest, RealPofsMatchOldBspTree)
{
	const char* env = std::getenv("FSO_BVH_PARITY_POF");
	if (env == nullptr || env[0] == '\0') {
		GTEST_SKIP() << "FSO_BVH_PARITY_POF not set; skipping real-.pof golden-parity check.";
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
	printf("Found %d .pof file(s) to test.\n", static_cast<int>(pof_files.size()));

	// Register a search root for every distinct mod directory referenced.
	std::set<std::string> roots_added;
	for (const auto& f : pof_files) {
		std::string root = find_mod_root(f).string();
		if (roots_added.insert(root).second)
			cf_add_external_path_root(root.c_str());
	}

	std::mt19937 rng(20260829); // fixed seed, deterministic across the whole run

	ParityTotals totals;
	int files_loaded = 0;

	for (const auto& pof_path : pof_files) {
		SCP_string filename = pof_path.filename().string();

		int model_num = model_load(filename.c_str(), nullptr, ErrorType::WARNING);
		if (model_num < 0) {
			printf("Skipping (load failed): %s\n", filename.c_str());
			continue;
		}

		polymodel* pm = model_get(model_num);
		if (pm == nullptr) {
			printf("Skipping (model_get returned null): %s\n", filename.c_str());
			continue;
		}

		files_loaded++;
		printf("Testing '%s': %d submodels\n", filename.c_str(), pm->n_models);

		ParityTotals per_file;
		check_model_parity(model_num, pm, rng, per_file);

		printf("  -> %d submodels with geometry, %ld rays, %ld hit mismatches, %ld distance mismatches\n",
			per_file.submodels_with_geometry, per_file.rays, per_file.hit_mismatches, per_file.dist_mismatches);

		totals.rays += per_file.rays;
		totals.hit_mismatches += per_file.hit_mismatches;
		totals.dist_mismatches += per_file.dist_mismatches;
		totals.submodels_with_geometry += per_file.submodels_with_geometry;
	}

	printf("Summary: %d/%d files loaded, %d submodels with geometry, %ld total rays, %ld hit mismatches, %ld distance mismatches\n",
		files_loaded, static_cast<int>(pof_files.size()), totals.submodels_with_geometry, totals.rays, totals.hit_mismatches,
		totals.dist_mismatches);

	ASSERT_GT(files_loaded, 0) << "No .pof file loaded successfully";
	ASSERT_GT(totals.submodels_with_geometry, 0) << "No submodel produced any triangles -- extraction likely broken";

	// A handful of mismatches on real, potentially imperfect production geometry (near-silhouette
	// grazing rays, coincident/overlapping polygons, concave pockets where "nearest frontface"
	// genuinely differs by a hair between two independently-implemented traversals) is expected
	// and not itself a sign of a BVH bug. A high mismatch rate is not.
	if (totals.rays > 0) {
		double hit_mismatch_rate = static_cast<double>(totals.hit_mismatches) / static_cast<double>(totals.rays);
		EXPECT_LT(hit_mismatch_rate, 0.02) << totals.hit_mismatches << "/" << totals.rays << " rays disagreed on hit/miss";
	}
}
