#pragma once

#include "globalincs/pstypes.h"

// Tabled custom shapes for coordinate points. Loaded from coordinate_points.tbl plus any
// *-cps.tbm modular tables. TBMs are additive only -- duplicate names and the reserved names
// "NGon" / "Star" are rejected with a warning. First definition wins, default table loads first.

struct coordinate_shape_vec2f {
	float x, y;
};

struct coordinate_shape_def {
	SCP_string name;
	SCP_vector<coordinate_shape_vec2f> verts;
	SCP_vector<int> tri_indices;  // size always a multiple of 3; indices in [0, verts.size())
};

extern SCP_vector<coordinate_shape_def> Coordinate_shapes;

// Loads the default table and any modular tables. Safe to call once at game init.
void coordinate_shapes_init();

// Returns -1 if no shape matches the given name (case-insensitive). nullptr is treated as miss.
int find_coordinate_shape_index_by_name(const char* name);
