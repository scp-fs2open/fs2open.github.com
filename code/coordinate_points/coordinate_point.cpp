#include "coordinate_points/coordinate_point.h"

#include "object/object.h"
#include "math/vecmat.h"
#include "parse/parselo.h"

SCP_list<mission_coordinate_point> Coordinate_points;
SCP_vector<parsed_coordinate_point> Parse_coordinate_points;

static constexpr float COORDINATE_POINT_DEFAULT_RADIUS = 1.0f;

static const char* Coordinate_point_shape_names[] = {
	"Triangle",
	"Square",
	"Diamond",
	"Pentagon",
	"Hexagon",
	"Cross",
};

static_assert(sizeof(Coordinate_point_shape_names) / sizeof(Coordinate_point_shape_names[0])
			  == static_cast<size_t>(CoordinatePointShape::NUM_SHAPES),
			  "Coordinate_point_shape_names is out of sync with CoordinatePointShape");

const char* coordinate_point_shape_to_string(CoordinatePointShape s)
{
	auto idx = static_cast<size_t>(s);
	if (idx >= static_cast<size_t>(CoordinatePointShape::NUM_SHAPES)) {
		return Coordinate_point_shape_names[0];
	}
	return Coordinate_point_shape_names[idx];
}

CoordinatePointShape coordinate_point_shape_from_string(const char* s)
{
	if (s != nullptr) {
		for (size_t i = 0; i < static_cast<size_t>(CoordinatePointShape::NUM_SHAPES); ++i) {
			if (!stricmp(Coordinate_point_shape_names[i], s)) {
				return static_cast<CoordinatePointShape>(i);
			}
		}
	}
	return CoordinatePointShape::Triangle;
}

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
	for (auto& cp : Coordinate_points) {
		if (cp.objnum >= 0 && Objects[cp.objnum].type != OBJ_NONE) {
			obj_delete(cp.objnum);
		}
	}
	Coordinate_points.clear();
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
			it->objnum = -1;  // mark detached so destructor / cleanup doesn't double-free
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

		cp->category        = parsed.category;
		cp->display_color   = parsed.display_color;
		cp->shape           = parsed.shape;
		cp->size_scale      = parsed.size_scale;
		cp->escort_priority = parsed.escort_priority;
	}

	Parse_coordinate_points.clear();
}
