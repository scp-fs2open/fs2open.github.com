#include "RelativeCoordinatesDialogModel.h"

#include "math/vecmat.h"
#include <mission/object.h>


namespace fso::fred::dialogs {

RelativeCoordinatesDialogModel::RelativeCoordinatesDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	_objects.clear();

	for (auto ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		bool added = false;

		int objnum = OBJ_INDEX(ptr);

		if (ptr->type == OBJ_START || ptr->type == OBJ_SHIP) {
			_objects.emplace_back(Ships[ptr->instance].ship_name, objnum);

			added = true;
		} else if (ptr->type == OBJ_WAYPOINT) {
			SCP_string text;
			int waypoint_num;

			auto wp_list = find_waypoint_list_with_instance(ptr->instance, &waypoint_num);
			Assert(wp_list != nullptr);
			text = wp_list->get_name();
			text += ":";
			text += std::to_string(waypoint_num + 1);
			_objects.emplace_back(text, objnum);

			added = true;
		}

		bool marked = (ptr->flags[Object::Object_Flags::Marked]);

		if (added && marked && _originIndex == -1) {
			_originIndex = objnum; // TODO: select first marked object as origin.. not sure QtFRED has cur_object_index available yet
		} else if (added && marked && _satelliteIndex == -1 && objnum != _originIndex) {
			_satelliteIndex = objnum;
		}
	}

	computeCoordinates();
}

bool RelativeCoordinatesDialogModel::apply()
{
	// Read only dialog
	return true;
}

void RelativeCoordinatesDialogModel::reject()
{
	// Read only dialog
}

float RelativeCoordinatesDialogModel::getDistance() const
{
	return _distance;
}
float RelativeCoordinatesDialogModel::getPitch() const
{
	return _orientation_p;
}
float RelativeCoordinatesDialogModel::getBank() const
{
	return _orientation_b;
}
float RelativeCoordinatesDialogModel::getHeading() const
{
	return _orientation_h;
}

int RelativeCoordinatesDialogModel::getOrigin() const
{
	return _originIndex;
}

void RelativeCoordinatesDialogModel::setOrigin(int index)
{
	if (_originIndex == index)
		return;
	_originIndex = index;
	computeCoordinates();
}

int RelativeCoordinatesDialogModel::getSatellite() const
{
	return _satelliteIndex;
}

void RelativeCoordinatesDialogModel::setSatellite(int index)
{
	if (_satelliteIndex == index)
		return;
	_satelliteIndex = index;
	computeCoordinates();
}

SCP_vector<std::pair<SCP_string, int>> RelativeCoordinatesDialogModel::getObjectsList() const
{
	return _objects;
}

void RelativeCoordinatesDialogModel::computeCoordinates()
{
	if (_originIndex < 0 || _satelliteIndex < 0 || (_originIndex == _satelliteIndex)) {
		_distance = 0.0f;
		_orientation_p = 0.0f;
		_orientation_b = 0.0f;
		_orientation_h = 0.0f;

		return;
	}

	auto origin_pos = Objects[_originIndex].pos;
	auto satellite_pos = Objects[_satelliteIndex].pos;

	// distance
	_distance = vm_vec_dist(&origin_pos, &satellite_pos);

	// transform the coordinate frame
	vec3d delta_vec, local_vec;
	vm_vec_sub(&delta_vec, &satellite_pos, &origin_pos);
	if (Objects[_originIndex].type != OBJ_WAYPOINT)
		vm_vec_rotate(&local_vec, &delta_vec, &Objects[_originIndex].orient);

	// find the orientation
	matrix m;
	vm_vector_2_matrix(&m, &local_vec);

	// find the angles
	angles ang;
	vm_extract_angles_matrix(&ang, &m);
	_orientation_p = to_degrees(ang.p);
	_orientation_b = to_degrees(ang.b);
	_orientation_h = to_degrees(ang.h);
}

float RelativeCoordinatesDialogModel::to_degrees(float rad)
{
	float deg = fl_degrees(rad);
	return normalize_degrees(deg);
}

float RelativeCoordinatesDialogModel::normalize_degrees(float deg)
{
	while (deg < -180.0f)
		deg += 180.0f;
	while (deg > 180.0f)
		deg -= 180.0f;
	// check for negative zero...
	if (deg == -0.0f)
		return 0.0f;
	return deg;
}

} // namespace fso::fred::dialogs
