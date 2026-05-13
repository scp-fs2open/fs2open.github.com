#pragma once

#include "coordinate_points/coordinate_point_flags.h"
#include "globalincs/pstypes.h"
#include "graphics/2d.h"

struct vec3d;

enum class CoordinatePointShape {
	Triangle = 0,
	Square,
	Diamond,
	Pentagon,
	Hexagon,
	Cross,
	Star,
	NUM_SHAPES
};

constexpr float COORDINATE_POINT_SIZE_MIN = 0.25f;
constexpr float COORDINATE_POINT_SIZE_MAX = 8.0f;

const char* coordinate_point_shape_to_string(CoordinatePointShape s);
CoordinatePointShape coordinate_point_shape_from_string(const char* s);  // returns Diamond on miss

struct mission_coordinate_point
{
	SCP_string name;
	SCP_string category;
	color      display_color;
	CoordinatePointShape shape = CoordinatePointShape::Diamond;
	float      size_scale = 1.0f;
	int        escort_priority = 0;      // 0 = not on escort list; >0 = on the list (and sort key)
	int        multi_team = -1;          // -1 = visible to all; otherwise TVT team index (0..MAX_TVT_TEAMS-1)
	flagset<CoordinatePoint::Flags> flags;
	SCP_string fred_layer = "Default";   // FRED view layer assignment
	int        objnum = -1;

	mission_coordinate_point();
};

extern SCP_list<mission_coordinate_point> Coordinate_points;

struct parsed_coordinate_point
{
	SCP_string name;
	SCP_string category;
	vec3d      position;
	color      display_color;
	CoordinatePointShape shape = CoordinatePointShape::Diamond;
	float      size_scale = 1.0f;
	int        escort_priority = 0;
	int        multi_team = -1;
	flagset<CoordinatePoint::Flags> flags;
	SCP_string fred_layer = "Default";

	parsed_coordinate_point();
};

extern SCP_vector<parsed_coordinate_point> Parse_coordinate_points;

void coordinate_points_level_close();

void post_process_mission_coordinate_points();

// Creates a coordinate point at the given position, allocating an Objects[] slot.
// If name is null or empty, an auto-name "Coordinate Point N" is generated (unique in mission).
// Returns the objnum on success, or -1 on failure.
int coordinate_point_create(const vec3d* pos, const char* name = nullptr);

// Removes the coordinate point whose Objects[] slot is objnum.
void coordinate_point_delete(int objnum);

mission_coordinate_point* find_coordinate_point_by_name(const char* name);
mission_coordinate_point* find_coordinate_point_by_objnum(int objnum);
