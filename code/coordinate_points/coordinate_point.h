#pragma once

#include "coordinate_points/coordinate_point_flags.h"
#include "globalincs/pstypes.h"
#include "graphics/2d.h"

struct vec3d;

// What kind of 2D shape this coordinate point draws as. NGon and Star are procedural primitives
// driven by parameter fields below. Tabled refers to a custom vertex+triangle definition loaded
// from coordinate_points.tbl / *-cps.tbm.
enum class CoordinatePointShapeKind : int {
	NGon = 0,
	Star,
	Tabled,
};

constexpr float COORDINATE_POINT_SIZE_MIN = 0.25f;
constexpr float COORDINATE_POINT_SIZE_MAX = 8.0f;

constexpr int   NGON_SIDES_MIN  = 3;
constexpr int   NGON_SIDES_MAX  = 32;
constexpr int   STAR_POINTS_MIN = 3;
constexpr int   STAR_POINTS_MAX = 32;
constexpr float STAR_INNER_MIN  = 0.05f;
constexpr float STAR_INNER_MAX  = 0.95f;
constexpr float STAR_INNER_DEFAULT = 0.382f;  // classic 5-point arm angle

struct mission_coordinate_point
{
	SCP_string name;
	SCP_string group;
	color      display_color;

	CoordinatePointShapeKind shape_kind = CoordinatePointShapeKind::NGon;
	int   shape_sides        = 4;                  // NGon: clamp [NGON_SIDES_MIN, NGON_SIDES_MAX]
	int   shape_points       = 5;                  // Star: clamp [STAR_POINTS_MIN, STAR_POINTS_MAX]
	float shape_inner_radius = STAR_INNER_DEFAULT; // Star: clamp [STAR_INNER_MIN, STAR_INNER_MAX]
	int   shape_table_index  = -1;                 // Tabled: index into Coordinate_shapes; -1 = unset
	float shape_angle_deg    = 0.0f;               // all: extra rotation around local Z

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
	SCP_string group;
	vec3d      position;
	color      display_color;

	CoordinatePointShapeKind shape_kind = CoordinatePointShapeKind::NGon;
	int   shape_sides        = 4;
	int   shape_points       = 5;
	float shape_inner_radius = STAR_INNER_DEFAULT;
	SCP_string shape_table_name;          // resolved to shape_table_index at post-process time
	float shape_angle_deg    = 0.0f;

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
