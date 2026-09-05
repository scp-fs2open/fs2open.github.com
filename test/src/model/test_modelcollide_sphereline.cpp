// Correctness coverage for mc_triangle_edges_sphereline() (modelcollide.cpp) -- the triangle edge
// test used by the live hull/shield sphereline path (mc_check_triangle_sphereline_face()).
#include <gtest/gtest.h>

#include <math/vecmat.h>

#include <algorithm>
#include <cmath>
#include <random>

// Not declared in any header (modelcollide.cpp keeps it non-static purely so this test can call it
// directly, bypassing the Mc thread_local state the rest of that file reads).
bool mc_triangle_edges_sphereline(const vec3d& v0, const vec3d& v1, const vec3d& v2, const vec3d& xs0,
	const vec3d& vs, float Rs, vec3d& out_hit_point, float& out_t);

namespace {

vec3d V(float x, float y, float z)
{
	vec3d v;
	v.xyz.x = x;
	v.xyz.y = y;
	v.xyz.z = z;
	return v;
}

// Self-consistency check for a reported hit, used in place of an independent oracle: the hit point
// must lie on one of the triangle's 3 (bounded) edges, and the sphere's center at the reported time
// must be within `tol` of exactly `Rs` from that point -- the geometric definition of "the sphere's
// surface first touches this edge here".
void ExpectValidEdgeHit(const vec3d& v0, const vec3d& v1, const vec3d& v2, const vec3d& xs0, const vec3d& vs,
	float Rs, const vec3d& hit_point, float t, float tol = 5e-2f)
{
	const vec3d* tri[3] = {&v0, &v1, &v2};
	bool on_some_edge = false;
	for (int i = 0; i < 3 && !on_some_edge; ++i) {
		const vec3d& a = *tri[i];
		const vec3d& b = *tri[(i + 1) % 3];
		vec3d ab = b - a;
		float ab_sqr = vm_vec_mag_squared(&ab);
		vec3d ap = hit_point - a;
		float s = vm_vec_dot(&ap, &ab) / ab_sqr;
		if (s < -tol || s > 1.0f + tol)
			continue;
		s = std::clamp(s, 0.0f, 1.0f);
		vec3d closest = a + ab * s;
		if (vm_vec_dist(&closest, &hit_point) < tol)
			on_some_edge = true;
	}
	EXPECT_TRUE(on_some_edge) << "hit point is not on any of the triangle's 3 edges";

	vec3d sphere_center = xs0 + vs * t;
	EXPECT_NEAR(vm_vec_dist(&sphere_center, &hit_point), Rs, tol) << "hit point is not Rs from the sphere center at t";
}

TEST(TriangleEdgesSphereline, SingleEdgeHit)
{
	// Triangle in the z=0 plane; sphere sweeps straight down through one edge's midpoint.
	vec3d v0 = V(-1, -1, 0), v1 = V(1, -1, 0), v2 = V(0, 1, 0);
	vec3d xs0 = V(0, -1, 5);
	vec3d vs = V(0, 0, -10); // reaches z=-5 at t=1, crosses z=0 (the edge) around t=0.5
	float radius = 0.3f;

	vec3d hit_point;
	float t;
	ASSERT_TRUE(mc_triangle_edges_sphereline(v0, v1, v2, xs0, vs, radius, hit_point, t));
	EXPECT_NEAR(t, 0.47f, 0.05f);
	ExpectValidEdgeHit(v0, v1, v2, xs0, vs, radius, hit_point, t);
}

TEST(TriangleEdgesSphereline, Miss)
{
	vec3d v0 = V(-1, -1, 0), v1 = V(1, -1, 0), v2 = V(0, 1, 0);
	vec3d xs0 = V(100, 100, 5);
	vec3d vs = V(0, 0, -10);
	float radius = 0.3f;

	vec3d hit_point;
	float t;
	EXPECT_FALSE(mc_triangle_edges_sphereline(v0, v1, v2, xs0, vs, radius, hit_point, t));
}

TEST(TriangleEdgesSphereline, VertexHit)
{
	// Sphere sweeps straight at a single vertex, well clear of both adjacent edges' midpoints --
	// only reachable via the vertex fallback, not a direct edge-segment hit.
	vec3d v0 = V(-1, -1, 0), v1 = V(1, -1, 0), v2 = V(0, 1, 0);
	vec3d xs0 = V(0, 1, 5);
	vec3d vs = V(0, 0, -10);
	float radius = 0.3f;

	vec3d hit_point;
	float t;
	ASSERT_TRUE(mc_triangle_edges_sphereline(v0, v1, v2, xs0, vs, radius, hit_point, t));
	EXPECT_NEAR(vm_vec_dist(&hit_point, &v2), 0.0f, 1e-2f);
	ExpectValidEdgeHit(v0, v1, v2, xs0, vs, radius, hit_point, t);
}

TEST(TriangleEdgesSphereline, ManyRandomTriangles_SelfConsistent)
{
	std::mt19937 rng(1234); // fixed seed, deterministic
	std::uniform_real_distribution<float> pos_dist(-10.0f, 10.0f);
	std::uniform_real_distribution<float> offset_dist(0.5f, 4.0f);
	std::uniform_real_distribution<float> radius_dist(0.1f, 1.5f);

	int checked = 0;
	int hits = 0;

	for (int i = 0; i < 1000; ++i) {
		vec3d v0 = V(pos_dist(rng), pos_dist(rng), pos_dist(rng));
		vec3d v1 = v0 + V(offset_dist(rng), offset_dist(rng) * 0.2f, offset_dist(rng) * 0.2f);
		vec3d v2 = v0 + V(offset_dist(rng) * 0.2f, offset_dist(rng), offset_dist(rng) * 0.2f);

		vec3d xs0 = V(pos_dist(rng), pos_dist(rng), pos_dist(rng));
		vec3d vs = V(offset_dist(rng) * (pos_dist(rng) < 0 ? -1 : 1), offset_dist(rng) * (pos_dist(rng) < 0 ? -1 : 1),
			offset_dist(rng) * (pos_dist(rng) < 0 ? -1 : 1));
		float radius = radius_dist(rng);

		vec3d hit_point;
		float t;
		++checked;
		if (mc_triangle_edges_sphereline(v0, v1, v2, xs0, vs, radius, hit_point, t)) {
			++hits;
			EXPECT_GE(t, 0.0f) << "case " << i;
			EXPECT_LE(t, 1.0f) << "case " << i;
			ExpectValidEdgeHit(v0, v1, v2, xs0, vs, radius, hit_point, t);
		}
	}

	SUCCEED() << checked << " cases, " << hits << " hits, all self-consistent.";
}

TEST(TriangleEdgesSphereline, ManyRandomTriangles_GrazingRadius)
{
	// Radius comparable to triangle size, more likely to exercise the vertex fallback / edge-boundary
	// (root-out-of-range) branches than the "small sphere through a big triangle" configurations above.
	std::mt19937 rng(5678);
	std::uniform_real_distribution<float> pos_dist(-5.0f, 5.0f);
	std::uniform_real_distribution<float> tri_extent(0.5f, 2.0f);
	std::uniform_real_distribution<float> radius_dist(0.5f, 3.0f);

	int hits = 0;

	for (int i = 0; i < 1000; ++i) {
		vec3d v0 = V(pos_dist(rng), pos_dist(rng), pos_dist(rng));
		vec3d v1 = v0 + V(tri_extent(rng), 0, 0);
		vec3d v2 = v0 + V(0, tri_extent(rng), 0);

		vec3d xs0 = V(pos_dist(rng), pos_dist(rng), pos_dist(rng));
		vec3d vs = V(pos_dist(rng) * 0.3f, pos_dist(rng) * 0.3f, pos_dist(rng) * 0.3f);
		float radius = radius_dist(rng);

		vec3d hit_point;
		float t;
		if (mc_triangle_edges_sphereline(v0, v1, v2, xs0, vs, radius, hit_point, t)) {
			++hits;
			ExpectValidEdgeHit(v0, v1, v2, xs0, vs, radius, hit_point, t);
		}
	}

	SUCCEED() << hits << " of 1000 grazing-radius cases hit, all self-consistent.";
}

} // namespace
