#include "coordinate_points/coordinate_point_render.h"

#include "coordinate_points/coordinate_point.h"
#include "graphics/material.h"
#include "math/floating.h"
#include "math/vecmat.h"
#include "network/multi.h"
#include "object/object.h"
#include "render/3d.h"

namespace {

// Unit-radius 2D vertex tables per shape, then a triangle-index table that triangulates each
// shape into a TRIS primitive list (each consecutive index triple is one triangle, CCW).

struct vec2f {
	float x, y;
};

// --- Vertex tables ---

const vec2f Tri_verts[] = {
	{  0.0f,    1.0f},
	{ -0.866f, -0.5f},
	{  0.866f, -0.5f},
};

const vec2f Sq_verts[] = {
	{ -1.0f,  1.0f},
	{  1.0f,  1.0f},
	{  1.0f, -1.0f},
	{ -1.0f, -1.0f},
};

const vec2f Dia_verts[] = {
	{  0.0f,  1.0f},
	{  1.0f,  0.0f},
	{  0.0f, -1.0f},
	{ -1.0f,  0.0f},
};

const vec2f Pent_verts[] = {
	{  0.0f,      1.0f   },
	{  0.951f,    0.309f },
	{  0.588f,   -0.809f },
	{ -0.588f,   -0.809f },
	{ -0.951f,    0.309f },
};

const vec2f Hex_verts[] = {
	{ -0.5f,    0.866f},
	{  0.5f,    0.866f},
	{  1.0f,    0.0f  },
	{  0.5f,   -0.866f},
	{ -0.5f,   -0.866f},
	{ -1.0f,    0.0f  },
};

const vec2f Cross_verts[] = {
	{ -0.33f,  1.0f },   // 0: top-left of vertical bar
	{  0.33f,  1.0f },   // 1: top-right of vertical bar
	{  0.33f,  0.33f},   // 2
	{  1.0f,   0.33f},   // 3: top-right of horizontal bar
	{  1.0f,  -0.33f},   // 4: bottom-right of horizontal bar
	{  0.33f, -0.33f},   // 5
	{  0.33f, -1.0f },   // 6: bottom-right of vertical bar
	{ -0.33f, -1.0f },   // 7: bottom-left of vertical bar
	{ -0.33f, -0.33f},   // 8
	{ -1.0f,  -0.33f},   // 9: bottom-left of horizontal bar
	{ -1.0f,   0.33f},   // 10: top-left of horizontal bar
	{ -0.33f,  0.33f},   // 11
};

// 5-point star: alternating outer (radius 1) and inner (radius ≈ 0.382) points, starting at
// the top and going counterclockwise. Inner radius gives the classic 36° star arm angle.
const vec2f Star_verts[] = {
	{  0.000f,   1.000f},   // 0: outer (top)
	{ -0.225f,   0.309f},   // 1: inner (top-left)
	{ -0.951f,   0.309f},   // 2: outer (top-left)
	{ -0.363f,  -0.118f},   // 3: inner (lower-left)
	{ -0.588f,  -0.809f},   // 4: outer (bottom-left)
	{  0.000f,  -0.382f},   // 5: inner (bottom)
	{  0.588f,  -0.809f},   // 6: outer (bottom-right)
	{  0.363f,  -0.118f},   // 7: inner (lower-right)
	{  0.951f,   0.309f},   // 8: outer (top-right)
	{  0.225f,   0.309f},   // 9: inner (top-right)
};

// --- Triangle index tables (each triple = one CCW triangle) ---

const int Tri_tris[] = {
	0, 1, 2,
};

const int Sq_tris[] = {
	0, 3, 2,
	0, 2, 1,
};

const int Dia_tris[] = {
	0, 3, 2,
	0, 2, 1,
};

const int Pent_tris[] = {
	0, 4, 3,
	0, 3, 2,
	0, 2, 1,
};

const int Hex_tris[] = {
	0, 5, 4,
	0, 4, 3,
	0, 3, 2,
	0, 2, 1,
};

// Cross: two rectangles, each split into two triangles. Indices reference Cross_verts above.
const int Cross_tris[] = {
	// Vertical bar (corners 0,1,6,7)
	0, 7, 6,
	0, 6, 1,
	// Horizontal bar (corners 10,3,4,9)
	10, 9, 4,
	10, 4, 3,
};

// Star: 5 arm-triangles (outer point + its two neighboring inner points) plus the central
// inner pentagon fanned from inner vertex 1. Indices reference Star_verts above.
const int Star_tris[] = {
	// 5 arms
	9, 0, 1,    // top arm
	1, 2, 3,    // top-left arm
	3, 4, 5,    // bottom-left arm
	5, 6, 7,    // bottom-right arm
	7, 8, 9,    // top-right arm
	// Inner pentagon, fanned from vertex 1
	1, 3, 5,
	1, 5, 7,
	1, 7, 9,
};

struct ShapeDef {
	const vec2f* verts;
	int          vert_count;
	const int*   tris;
	int          tri_index_count;  // number of indices, always a multiple of 3
};

const ShapeDef Shape_defs[] = {
	{ Tri_verts,    static_cast<int>(sizeof(Tri_verts)   / sizeof(vec2f)),
	  Tri_tris,     static_cast<int>(sizeof(Tri_tris)    / sizeof(int))   },
	{ Sq_verts,     static_cast<int>(sizeof(Sq_verts)    / sizeof(vec2f)),
	  Sq_tris,      static_cast<int>(sizeof(Sq_tris)     / sizeof(int))   },
	{ Dia_verts,    static_cast<int>(sizeof(Dia_verts)   / sizeof(vec2f)),
	  Dia_tris,     static_cast<int>(sizeof(Dia_tris)    / sizeof(int))   },
	{ Pent_verts,   static_cast<int>(sizeof(Pent_verts)  / sizeof(vec2f)),
	  Pent_tris,    static_cast<int>(sizeof(Pent_tris)   / sizeof(int))   },
	{ Hex_verts,    static_cast<int>(sizeof(Hex_verts)   / sizeof(vec2f)),
	  Hex_tris,     static_cast<int>(sizeof(Hex_tris)    / sizeof(int))   },
	{ Cross_verts,  static_cast<int>(sizeof(Cross_verts) / sizeof(vec2f)),
	  Cross_tris,   static_cast<int>(sizeof(Cross_tris)  / sizeof(int))   },
	{ Star_verts,   static_cast<int>(sizeof(Star_verts)  / sizeof(vec2f)),
	  Star_tris,    static_cast<int>(sizeof(Star_tris)   / sizeof(int))   },
};

static_assert(sizeof(Shape_defs) / sizeof(ShapeDef) == static_cast<size_t>(CoordinatePointShape::NUM_SHAPES),
			  "Shape_defs is out of sync with CoordinatePointShape");

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

	const auto shape_idx = static_cast<size_t>(cp.shape);
	if (shape_idx >= sizeof(Shape_defs) / sizeof(ShapeDef)) {
		return;
	}

	const ShapeDef& def = Shape_defs[shape_idx];
	const vec3d& pos = Objects[cp.objnum].pos;
	const float radius = get_coordinate_point_world_radius(cp, *camera_eye);

	const vec3d& rvec = camera_orient->vec.rvec;
	const vec3d& uvec = camera_orient->vec.uvec;

	// Build the camera-facing world-space vertices once, then expand the triangle index table
	// into a flat vertex array for PRIM_TYPE_TRIS.
	vec3d world_verts[32];
	Assertion(def.vert_count <= static_cast<int>(sizeof(world_verts) / sizeof(world_verts[0])),
			  "Coordinate-point shape vertex table is larger than the local buffer");
	for (int i = 0; i < def.vert_count; ++i) {
		world_verts[i] = pos;
		vm_vec_scale_add2(&world_verts[i], &rvec, def.verts[i].x * radius);
		vm_vec_scale_add2(&world_verts[i], &uvec, def.verts[i].y * radius);
	}

	vertex tri_verts[64];
	Assertion(def.tri_index_count <= static_cast<int>(sizeof(tri_verts) / sizeof(tri_verts[0])),
			  "Coordinate-point shape triangle list is larger than the local buffer");
	for (int i = 0; i < def.tri_index_count; ++i) {
		g3_transfer_vertex(&tri_verts[i], &world_verts[def.tris[i]]);
	}

	material mat;
	mat.set_color(cp.display_color);
	mat.set_depth_mode(ZBUFFER_TYPE_FULL);
	mat.set_blend_mode(ALPHA_BLEND_ALPHA_BLEND_ALPHA);

	g3_render_primitives(&mat, tri_verts, def.tri_index_count, PRIM_TYPE_TRIS, false);
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
