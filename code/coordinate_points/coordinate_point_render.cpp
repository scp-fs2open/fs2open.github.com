#include "coordinate_points/coordinate_point_render.h"

#include "coordinate_points/coordinate_point.h"
#include "coordinate_points/coordinate_shapes.h"
#include "graphics/material.h"
#include "math/floating.h"
#include "math/vecmat.h"
#include "network/multi.h"
#include "object/object.h"
#include "render/3d.h"

#include <cmath>

namespace {

struct vec2f {
	float x, y;
};

// Cap on locally-built 2D vertex/triangle buffers. NGon caps out at NGON_SIDES_MAX (32), Star
// caps at 2 * STAR_POINTS_MAX (64), and tabled shapes are bounded by author hand. 128 / 384
// leaves plenty of headroom.
constexpr int MAX_LOCAL_VERTS = 128;
constexpr int MAX_LOCAL_TRIS  = MAX_LOCAL_VERTS * 3;

constexpr float PI_F = 3.14159265358979323846f;

// Generate a regular N-gon centered on (0,0), unit radius, first vertex at the top
// (i.e. starting angle = π/2). CCW winding. Triangulated as a fan from vertex 0.
void build_ngon(int sides, vec2f* out_verts, int& out_nv, int* out_tris, int& out_ntri_idx)
{
	if (sides < NGON_SIDES_MIN) sides = NGON_SIDES_MIN;
	if (sides > NGON_SIDES_MAX) sides = NGON_SIDES_MAX;

	const float start = PI_F * 0.5f;                    // top
	const float step  = 2.0f * PI_F / static_cast<float>(sides);
	for (int i = 0; i < sides; ++i) {
		// CCW means angles increase as i increases (standard math convention with +Y up).
		const float a = start + step * static_cast<float>(i);
		out_verts[i].x = std::cos(a);
		out_verts[i].y = std::sin(a);
	}
	out_nv = sides;

	// Fan from vertex 0: triangles (0, i, i+1) for i in [1, sides-2].
	int n = 0;
	for (int i = 1; i < sides - 1; ++i) {
		out_tris[n++] = 0;
		out_tris[n++] = i;
		out_tris[n++] = i + 1;
	}
	out_ntri_idx = n;
}

// Generate an N-pointed star, unit outer radius, first outer point at the top. Alternates outer
// (1.0) and inner (inner_r) radius. Triangulated as N arm triangles (outer, inner_prev, inner_next)
// plus an inner-polygon fan from inner vertex 1.
void build_star(int points, float inner_r, vec2f* out_verts, int& out_nv, int* out_tris, int& out_ntri_idx)
{
	if (points < STAR_POINTS_MIN) points = STAR_POINTS_MIN;
	if (points > STAR_POINTS_MAX) points = STAR_POINTS_MAX;
	if (inner_r < STAR_INNER_MIN) inner_r = STAR_INNER_MIN;
	if (inner_r > STAR_INNER_MAX) inner_r = STAR_INNER_MAX;

	const int   nv    = points * 2;
	const float start = PI_F * 0.5f;                            // first outer vert at top
	const float step  = PI_F / static_cast<float>(points);      // half-step between outer & inner
	for (int i = 0; i < nv; ++i) {
		const float a = start + step * static_cast<float>(i);
		const float r = (i & 1) ? inner_r : 1.0f;               // even=outer, odd=inner
		out_verts[i].x = std::cos(a) * r;
		out_verts[i].y = std::sin(a) * r;
	}
	out_nv = nv;

	int n = 0;
	// Arms: each outer vertex paired with its two neighbouring inner vertices.
	// Outer at index 2k (k = 0..points-1); neighbours are inner at indices 2k-1 (mod nv) and 2k+1.
	for (int k = 0; k < points; ++k) {
		const int outer = 2 * k;
		const int prev_inner = (outer - 1 + nv) % nv;
		const int next_inner = (outer + 1) % nv;
		out_tris[n++] = prev_inner;
		out_tris[n++] = outer;
		out_tris[n++] = next_inner;
	}
	// Inner polygon: vertices at odd indices 1, 3, 5, ..., nv-1. Fan from vertex 1.
	for (int k = 1; k + 1 < points; ++k) {
		out_tris[n++] = 1;
		out_tris[n++] = 1 + 2 * k;
		out_tris[n++] = 1 + 2 * (k + 1);
	}
	out_ntri_idx = n;
}

// Copy a tabled shape into the local buffers. Caller must have validated def has room.
bool copy_tabled(const coordinate_shape_def& def, vec2f* out_verts, int& out_nv, int* out_tris, int& out_ntri_idx)
{
	if (static_cast<int>(def.verts.size()) > MAX_LOCAL_VERTS ||
		static_cast<int>(def.tri_indices.size()) > MAX_LOCAL_TRIS) {
		return false;
	}
	out_nv = static_cast<int>(def.verts.size());
	for (int i = 0; i < out_nv; ++i) {
		out_verts[i].x = def.verts[i].x;
		out_verts[i].y = def.verts[i].y;
	}
	out_ntri_idx = static_cast<int>(def.tri_indices.size());
	for (int i = 0; i < out_ntri_idx; ++i) {
		out_tris[i] = def.tri_indices[i];
	}
	return true;
}

// Build the shape's 2D verts + triangle indices for the given coord point. Returns false if
// the shape can't be resolved (out-of-range tabled index, oversized table entry).
bool build_shape_local(const mission_coordinate_point& cp, vec2f* verts, int& nv, int* tris, int& nti)
{
	switch (cp.shape_kind) {
		case CoordinatePointShapeKind::NGon:
			build_ngon(cp.shape_sides, verts, nv, tris, nti);
			return true;
		case CoordinatePointShapeKind::Star:
			build_star(cp.shape_points, cp.shape_inner_radius, verts, nv, tris, nti);
			return true;
		case CoordinatePointShapeKind::Tabled: {
			const int idx = cp.shape_table_index;
			if (idx < 0 || idx >= static_cast<int>(Coordinate_shapes.size())) {
				return false;
			}
			return copy_tabled(Coordinate_shapes[idx], verts, nv, tris, nti);
		}
	}
	return false;
}

} // anonymous namespace

float get_coordinate_point_world_radius(const mission_coordinate_point& cp, const vec3d& camera_eye)
{
	if (cp.objnum < 0) {
		return COORDINATE_POINT_LOLLIPOP_SIZE * cp.size_scale;
	}
	const vec3d& pos = Objects[cp.objnum].pos;
	float dist = vm_vec_dist(&camera_eye, &pos);
	float size = fl_sqrt(dist / COORDINATE_POINT_DIST_DIVISOR) * cp.size_scale;
	const float floor_size = COORDINATE_POINT_LOLLIPOP_SIZE * cp.size_scale;
	if (size < floor_size) {
		size = floor_size;
	}
	return size;
}

void draw_coordinate_point_shape(const mission_coordinate_point& cp,
								  const vec3d* camera_eye,
								  const matrix* camera_orient)
{
	if (cp.objnum < 0) {
		return;
	}

	vec2f local_verts[MAX_LOCAL_VERTS];
	int   local_tris[MAX_LOCAL_TRIS];
	int   nv = 0, nti = 0;
	if (!build_shape_local(cp, local_verts, nv, local_tris, nti)) {
		return;
	}

	// Apply the per-instance rotation (degrees → radians) in 2D before billboarding.
	if (cp.shape_angle_deg != 0.0f) {
		const float a = cp.shape_angle_deg * (PI_F / 180.0f);
		const float ca = std::cos(a);
		const float sa = std::sin(a);
		for (int i = 0; i < nv; ++i) {
			const float x = local_verts[i].x;
			const float y = local_verts[i].y;
			local_verts[i].x = ca * x - sa * y;
			local_verts[i].y = sa * x + ca * y;
		}
	}

	const vec3d& pos = Objects[cp.objnum].pos;
	const float radius = get_coordinate_point_world_radius(cp, *camera_eye);

	const vec3d& rvec = camera_orient->vec.rvec;
	const vec3d& uvec = camera_orient->vec.uvec;

	// Build camera-facing world-space vertices, then expand the triangle index list into a flat
	// vertex array for PRIM_TYPE_TRIS.
	vec3d world_verts[MAX_LOCAL_VERTS];
	for (int i = 0; i < nv; ++i) {
		world_verts[i] = pos;
		vm_vec_scale_add2(&world_verts[i], &rvec, local_verts[i].x * radius);
		vm_vec_scale_add2(&world_verts[i], &uvec, local_verts[i].y * radius);
	}

	vertex tri_verts[MAX_LOCAL_TRIS];
	for (int i = 0; i < nti; ++i) {
		g3_transfer_vertex(&tri_verts[i], &world_verts[local_tris[i]]);
	}

	material mat;
	mat.set_color(cp.display_color);
	// Depth-test but DON'T write to depth: the lightshaft post-pass samples the depth texture
	// to find occluders, and coordinate points are conceptually HUD overlays, not physical
	// objects, so they shouldn't carve shadow shafts into the lightshaft pattern.
	mat.set_depth_mode(ZBUFFER_TYPE_READ);
	mat.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);

	g3_render_primitives(&mat, tri_verts, nti, PRIM_TYPE_TRIS, false);
}

void coordinate_points_render_all_in_mission()
{
	bool any_drawn = false;
	const bool in_multi = (Net_player != nullptr);
	const int  local_multi_team = in_multi ? Net_player->p_info.team : -1;

	for (const auto& cp : Coordinate_points) {
		if (cp.objnum < 0) {
			continue;
		}
		if (!cp.flags[CoordinatePoint::Flags::Visible_in_mission]) {
			continue;
		}
		// Multiplayer team filter: if this point is assigned to a specific team and the local
		// player is on a different team, skip rendering. Singleplayer and "no team" (team < 0)
		// points always render.
		if (in_multi && cp.multi_team >= 0 && cp.multi_team != local_multi_team) {
			continue;
		}
		draw_coordinate_point_shape(cp, &Eye_position, &Eye_matrix);
		any_drawn = true;
	}
	// g3_render_primitives leaks the material's color into the immediate-mode draw state.
	// Reset it so downstream HUD draws see a known starting color.
	if (any_drawn) {
		gr_set_color(255, 255, 255);
	}
}

void coordinate_points_render_all_in_editor(const vec3d* camera_eye, const matrix* camera_orient)
{
	for (const auto& cp : Coordinate_points) {
		if (cp.objnum < 0) {
			continue;
		}
		draw_coordinate_point_shape(cp, camera_eye, camera_orient);
	}
}
