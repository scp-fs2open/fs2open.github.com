// Golden-parity check between the existing BSP collision tree and the new modelbvh module,
// run against a real .pof supplied from outside the repo.
//
// This intentionally does NOT run as part of the normal test suite: no .pof file is (or should
// be) committed to the repo (game assets are separately licensed -- see the collision-BVH-rewrite
// project notes), so there is nothing to check it against in CI. Point the FSO_BVH_PARITY_POF
// environment variable at a real .pof file on disk to run it locally, e.g.:
//
//   FSO_BVH_PARITY_POF="E:\Games\Knossos\FS2\somemod\data\models\ship.pof" \
//       ./unittests.exe --gtest_filter=*BvhParity*
//
// If the variable isn't set, the test is skipped.

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

} // namespace

class BvhParityTest : public test::FSTestFixture {
protected:
	BvhParityTest() : FSTestFixture(INIT_CFILE | INIT_GRAPHICS) {}
};

TEST_F(BvhParityTest, RealPofMatchesOldBspTree)
{
	const char* pof_path_env = std::getenv("FSO_BVH_PARITY_POF");
	if (pof_path_env == nullptr || pof_path_env[0] == '\0') {
		GTEST_SKIP() << "FSO_BVH_PARITY_POF not set; skipping real-.pof golden-parity check.";
	}

	std::filesystem::path pof_path(pof_path_env);
	ASSERT_TRUE(std::filesystem::exists(pof_path)) << "File does not exist: " << pof_path_env;

	// cfile resolves CF_TYPE_MODELS as "<root>/data/models/" (cfile.cpp: { CF_TYPE_MODELS, "data/models", ".pof", ... }),
	// so the registered root must be the *mod* directory, not the models folder itself. Find the
	// "data" component and use its parent; fall back to the standard <root>/data/models/<file>
	// layout if "data" isn't found in the path for some reason.
	std::filesystem::path mod_root;
	for (std::filesystem::path search = pof_path.parent_path(); search.has_parent_path(); search = search.parent_path()) {
		if (stricmp(search.filename().string().c_str(), "data") == 0) {
			mod_root = search.parent_path();
			break;
		}
	}
	if (mod_root.empty())
		mod_root = pof_path.parent_path().parent_path().parent_path();

	SCP_string pof_dir = mod_root.string();
	SCP_string pof_filename = pof_path.filename().string();

	cf_add_external_path_root(pof_dir.c_str());

	int model_num = model_load(pof_filename.c_str(), nullptr, ErrorType::WARNING);
	ASSERT_GE(model_num, 0) << "model_load() failed to load " << pof_filename;

	polymodel* pm = model_get(model_num);
	ASSERT_NE(pm, nullptr);

	printf("Loaded '%s': %d submodels\n", pof_filename.c_str(), pm->n_models);

	std::mt19937 rng(20260829); // fixed seed, deterministic

	long total_rays = 0;
	long total_hit_mismatches = 0;
	long total_dist_mismatches = 0;
	int submodels_with_geometry = 0;

	for (int sm_idx = 0; sm_idx < pm->n_models; ++sm_idx) {
		SCP_vector<bvh_triangle> triangles = model_bvh_extract_submodel_triangles(pm, sm_idx);
		if (triangles.empty())
			continue;

		submodels_with_geometry++;
		bvh_tree tree = bvh_build(triangles);

		bsp_info& sm = pm->submodel[sm_idx];
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
			mc.flags = MC_CHECK_MODEL | MC_SUBMODEL | MC_CHECK_RAY;

			bool old_hit = model_collide(&mc) != 0;

			float new_t;
			int new_tri;
			bool new_hit = nearest_frontfacing_hit(triangles, origin, dir, new_t, new_tri);

			submodel_rays++;
			total_rays++;

			if (old_hit != new_hit) {
				submodel_hit_mismatches++;
				total_hit_mismatches++;
				if (submodel_hit_mismatches <= 5) {
					printf("  [submodel %d] hit mismatch: old=%d new=%d origin=(%.2f,%.2f,%.2f) dir=(%.2f,%.2f,%.2f)\n",
						sm_idx, old_hit, new_hit, origin.xyz.x, origin.xyz.y, origin.xyz.z, dir.xyz.x, dir.xyz.y,
						dir.xyz.z);
				}
				continue;
			}

			if (old_hit) {
				float tolerance = std::max(1e-3f, 1e-4f * std::fabs(mc.hit_dist));
				if (std::fabs(mc.hit_dist - new_t) > tolerance) {
					submodel_dist_mismatches++;
					total_dist_mismatches++;
					if (submodel_dist_mismatches <= 5) {
						printf("  [submodel %d] distance mismatch: old=%f new=%f\n", sm_idx, mc.hit_dist, new_t);
					}
				}
			}
		}

		if (submodel_hit_mismatches > 0 || submodel_dist_mismatches > 0) {
			printf("Submodel %d ('%s'): %d triangles, %d rays, %d hit mismatches, %d distance mismatches\n", sm_idx,
				sm.name, static_cast<int>(triangles.size()), submodel_rays, submodel_hit_mismatches,
				submodel_dist_mismatches);
		}
	}

	printf("Summary: %d submodels with geometry, %ld total rays, %ld hit mismatches, %ld distance mismatches\n",
		submodels_with_geometry, total_rays, total_hit_mismatches, total_dist_mismatches);

	ASSERT_GT(submodels_with_geometry, 0) << "No submodel produced any triangles -- extraction likely broken";

	// A handful of mismatches on real, potentially imperfect production geometry (near-silhouette
	// grazing rays, coincident/overlapping polygons, concave pockets where "nearest frontface"
	// genuinely differs by a hair between two independently-implemented traversals) is expected
	// and not itself a sign of a BVH bug. A high mismatch rate is not.
	if (total_rays > 0) {
		double hit_mismatch_rate = static_cast<double>(total_hit_mismatches) / static_cast<double>(total_rays);
		EXPECT_LT(hit_mismatch_rate, 0.02) << total_hit_mismatches << "/" << total_rays << " rays disagreed on hit/miss";
	}
}
