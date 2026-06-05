#include "coordinate_points/coordinate_point.h"

#include "coordinate_points/coordinate_shapes.h"
#include "object/object.h"
#include "math/vecmat.h"
#include "parse/parselo.h"

SCP_list<mission_coordinate_point> Coordinate_points;
SCP_vector<parsed_coordinate_point> Parse_coordinate_points;

static constexpr float COORDINATE_POINT_DEFAULT_RADIUS = 1.0f;

mission_coordinate_point::mission_coordinate_point()
{
	gr_init_alphacolor(&display_color, 255, 255, 255, 255);
}

parsed_coordinate_point::parsed_coordinate_point()
{
	position = vmd_zero_vector;
	gr_init_alphacolor(&display_color, 255, 255, 255, 255);
}

void coordinate_points_level_close()
{
	// Snapshot the objnums and clear the list first so the per-object obj_delete callback
	// (which calls coordinate_point_delete) doesn't try to erase from the list we're walking.
	SCP_vector<int> objnums;
	objnums.reserve(Coordinate_points.size());
	for (const auto& cp : Coordinate_points) {
		if (cp.objnum >= 0) {
			objnums.push_back(cp.objnum);
		}
	}
	Coordinate_points.clear();

	for (int objnum : objnums) {
		if (Objects[objnum].type != OBJ_NONE) {
			obj_delete(objnum);
		}
	}
}

static bool coordinate_point_name_in_use(const char* name)
{
	for (const auto& cp : Coordinate_points) {
		if (!stricmp(cp.name.c_str(), name)) {
			return true;
		}
	}
	return false;
}

static SCP_string make_unique_default_name()
{
	for (int i = 1;; ++i) {
		SCP_string candidate;
		sprintf(candidate, "Coordinate Point %d", i);
		if (!coordinate_point_name_in_use(candidate.c_str())) {
			return candidate;
		}
	}
}

int coordinate_point_create(const vec3d* pos, const char* name)
{
	Assertion(pos != nullptr, "coordinate_point_create requires a non-null position");

	flagset<Object::Object_Flags> flags;
	flags.set(Object::Object_Flags::Renders);
	// Coordinate points have no meaningful orientation; mark that explicitly so any future
	// physics/rotation code shortcuts cleanly. Position is left mutable so SEXPs and Lua
	// can relocate the point at runtime.
	flags.set(Object::Object_Flags::Dont_change_orientation);
	int objnum = obj_create(OBJ_COORDINATE_POINT, -1, -1, nullptr, pos, COORDINATE_POINT_DEFAULT_RADIUS, flags);
	if (objnum < 0) {
		return -1;
	}

	Coordinate_points.emplace_back();
	auto& cp = Coordinate_points.back();
	if (name != nullptr && *name != '\0' && !coordinate_point_name_in_use(name)) {
		cp.name = name;
	} else {
		cp.name = make_unique_default_name();
	}
	cp.objnum = objnum;

	return objnum;
}

void coordinate_point_delete(int objnum)
{
	for (auto it = Coordinate_points.begin(); it != Coordinate_points.end(); ++it) {
		if (it->objnum == objnum) {
			Coordinate_points.erase(it);
			return;
		}
	}
}

mission_coordinate_point* find_coordinate_point_by_name(const char* name)
{
	if (name == nullptr) {
		return nullptr;
	}
	for (auto& cp : Coordinate_points) {
		if (!stricmp(cp.name.c_str(), name)) {
			return &cp;
		}
	}
	return nullptr;
}

mission_coordinate_point* find_coordinate_point_by_objnum(int objnum)
{
	if (objnum < 0) {
		return nullptr;
	}
	for (auto& cp : Coordinate_points) {
		if (cp.objnum == objnum) {
			return &cp;
		}
	}
	return nullptr;
}

void post_process_mission_coordinate_points()
{
	for (const auto& parsed : Parse_coordinate_points) {
		int objnum = coordinate_point_create(&parsed.position,
											  parsed.name.empty() ? nullptr : parsed.name.c_str());
		if (objnum < 0) {
			continue;
		}

		auto* cp = find_coordinate_point_by_objnum(objnum);
		if (cp == nullptr) {
			continue;
		}

		cp->group              = parsed.group;
		cp->display_color      = parsed.display_color;
		cp->shape_kind         = parsed.shape_kind;
		cp->shape_sides        = parsed.shape_sides;
		cp->shape_points       = parsed.shape_points;
		cp->shape_inner_radius = parsed.shape_inner_radius;
		cp->shape_angle_deg    = parsed.shape_angle_deg;
		cp->size_scale         = parsed.size_scale;
		cp->escort_priority    = parsed.escort_priority;
		cp->multi_team         = parsed.multi_team;
		cp->flags              = parsed.flags;
		cp->fred_layer         = parsed.fred_layer;

		// Resolve a tabled-shape name to an index. If the name doesn't match anything in the
		// registry (default table + TBMs), fall back to NGon(3) with a warning so the mission
		// still loads cleanly.
		if (cp->shape_kind == CoordinatePointShapeKind::Tabled) {
			cp->shape_table_index = find_coordinate_shape_index_by_name(parsed.shape_table_name.c_str());
			if (cp->shape_table_index < 0) {
				Warning(LOCATION,
					"Coordinate point '%s' references unknown shape '%s'; falling back to NGon(3).",
					cp->name.c_str(),
					parsed.shape_table_name.c_str());
				cp->shape_kind  = CoordinatePointShapeKind::NGon;
				cp->shape_sides = 3;
			}
		}
	}

	Parse_coordinate_points.clear();
}
