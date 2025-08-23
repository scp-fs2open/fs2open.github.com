#include "ObjectOrientEditorDialogModel.h"

#include <mission/object.h>
#include <globalincs/linklist.h>
#include <ship/ship.h>
#include <math/bitarray.h>

namespace fso::fred::dialogs {

ObjectOrientEditorDialogModel::ObjectOrientEditorDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	vm_vec_make(&_location, 0.f, 0.f, 0.f);
	Assert(query_valid_object(_editor->currentObject));
	_position = Objects[_editor->currentObject].pos;

	angles ang{};
	vm_extract_angles_matrix(&ang, &Objects[_editor->currentObject].orient);
	_orientationDeg.xyz.x = normalize_degrees(fl_degrees(ang.p));
	_orientationDeg.xyz.y = normalize_degrees(fl_degrees(ang.b));
	_orientationDeg.xyz.z = normalize_degrees(fl_degrees(ang.h));

	initializeData();
}

void ObjectOrientEditorDialogModel::initializeData()
{
	char text[80];
	int type;
	object* ptr;

	_pointToObjectList.clear();

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (_editor->getNumMarked() != 1 || OBJ_INDEX(ptr) != _editor->currentObject) {
			switch (ptr->type) {
				case OBJ_START:
				case OBJ_SHIP:
					_pointToObjectList.emplace_back(ObjectEntry(Ships[ptr->instance].ship_name, OBJ_INDEX(ptr)));
					break;
				case OBJ_WAYPOINT: {
					int waypoint_num;
					waypoint_list* wp_list = find_waypoint_list_with_instance(ptr->instance, &waypoint_num);
					Assertion(wp_list != nullptr, "Waypoint list was nullptr!");
					sprintf(text, "%s:%d", wp_list->get_name(), waypoint_num + 1);

					_pointToObjectList.emplace_back(ObjectEntry(text, OBJ_INDEX(ptr)));
					break;
				}
				case OBJ_POINT:
				case OBJ_JUMP_NODE:
					break;
				default:
					Assertion(false, "Unknown object type in Object Orient Dialog!"); // unknown object type
			}
		}

		ptr = GET_NEXT(ptr);
	}

	type = Objects[_editor->currentObject].type;
	if (_editor->getNumMarked() == 1 && (type == OBJ_WAYPOINT || type == OBJ_JUMP_NODE)) {
		_orientationEnabledForType = false;
		_selectedPointToObjectIndex = -1;
	} else {
		_selectedPointToObjectIndex = _pointToObjectList.empty() ? -1 : _pointToObjectList[0].objIndex;
	}

	modelChanged();
}

void ObjectOrientEditorDialogModel::updateObject(object* ptr)
{
	if (ptr->type != OBJ_WAYPOINT && _pointTo) {
		vec3d v;
		matrix m;

		memset(&v, 0, sizeof(vec3d));
		if (_pointMode == PointToMode::Object) {
			if (_selectedPointToObjectIndex >= 0) {
				v = Objects[_selectedPointToObjectIndex].pos;
				vm_vec_sub2(&v, &ptr->pos);
			}
		} else if (_pointMode == PointToMode::Location) {
			vm_vec_sub(&v, &_location, &ptr->pos);
		} else {
			Assertion(false, "Unknown Point To mode in Object Orient Dialog!"); // neither radio button is checked.
		}

		if (!v.xyz.x && !v.xyz.y && !v.xyz.z) {
			return; // can't point to itself.
		}

		vm_vector_2_matrix(&m, &v, nullptr, nullptr);
		ptr->orient = m;
	}
}

// Also in objectorient.cpp in FRED. TODO Would be nice if this were somewhere common
float ObjectOrientEditorDialogModel::normalize_degrees(float deg)
{
	while (deg < -180.0f)
		deg += 360.0f;
	while (deg > 180.0f)
		deg -= 360.0f;
	// collapse negative zero
	if (deg == -0.0f)
		deg = 0.0f;
	return deg;
}

float ObjectOrientEditorDialogModel::round1(float v)
{
	return std::round(v * 10.0f) / 10.0f;
}

ObjectOrientEditorDialogModel::ObjectEntry::ObjectEntry(SCP_string in_name, int in_objIndex)
	: name(std::move(in_name)), objIndex(in_objIndex)
{
}

bool ObjectOrientEditorDialogModel::apply()
{
	object* origin_objp = nullptr;

	// Build translation delta and orientation matrix from UI values
	vec3d delta = vmd_zero_vector;
	matrix desired_orient = vmd_identity_matrix;
	bool change_pos = false, change_orient = false;

	auto& obj = Objects[_editor->currentObject];

	// ----- Position -----
	// If Relative: _position is a local space delta; unrotate into world
	// If Absolute: delta = _position - obj.pos
	{
		const vec3d& refPos = (_setMode == SetMode::Relative) ? vmd_zero_vector : obj.pos;
		if (!is_close(refPos.xyz.x, _position.xyz.x) || !is_close(refPos.xyz.y, _position.xyz.y) ||
			!is_close(refPos.xyz.z, _position.xyz.z)) {

			if (_setMode == SetMode::Relative) {
				vm_vec_unrotate(&delta, &_position, &obj.orient);
			} else {
				vm_vec_sub(&delta, &_position, &refPos);
			}
			change_pos = true;
		}
	}

	// ----- Orientation -----
	{
		angles object_ang{};
		vm_extract_angles_matrix(&object_ang, &obj.orient);

		vec3d refDeg = (_setMode == SetMode::Relative) ? vmd_zero_vector
													   : vec3d{{{normalize_degrees(fl_degrees(object_ang.p)),
															 normalize_degrees(fl_degrees(object_ang.b)),
															 normalize_degrees(fl_degrees(object_ang.h))}}};

		if (!is_close(refDeg.xyz.x, normalize_degrees(_orientationDeg.xyz.x)) ||
			!is_close(refDeg.xyz.y, normalize_degrees(_orientationDeg.xyz.y)) ||
			!is_close(refDeg.xyz.z, normalize_degrees(_orientationDeg.xyz.z))) {

			angles ang{};
			ang.p = fl_radians(_orientationDeg.xyz.x);
			ang.b = fl_radians(_orientationDeg.xyz.y);
			ang.h = fl_radians(_orientationDeg.xyz.z);

			if (_setMode == SetMode::Relative) {
				ang.p = object_ang.p + ang.p;
				ang.b = object_ang.b + ang.b;
				ang.h = object_ang.h + ang.h;
			}
			vm_angles_2_matrix(&desired_orient, &ang);
			change_orient = true;
		}
	}

	// ----- Transform mode -----
	// If multiple marked and using Relative to Origin, move/rotate the origin first, then
	// bring everyone else along by the origin’s delta rotation and position.
	matrix origin_rotation = vmd_identity_matrix;
	vec3d origin_prev_pos = vmd_zero_vector;

	const bool manyMarked = (_editor->getNumMarked() > 1);
	const bool relativeToOrigin = (manyMarked && _transformMode == TransformMode::Relative);

	if (relativeToOrigin) {
		origin_objp = &obj;
		origin_prev_pos = origin_objp->pos;
		matrix saved_orient = origin_objp->orient;

		// Move the origin first
		if (change_pos) {
			vm_vec_add2(&origin_objp->pos, &delta);
			_editor->missionChanged();
		}
		if (_pointTo) {
			updateObject(origin_objp);
			_editor->missionChanged();
		} else if (change_orient) {
			origin_objp->orient = desired_orient;
			_editor->missionChanged();
		}

		if (origin_objp->type != OBJ_WAYPOINT) {
			vm_transpose(&saved_orient);
			origin_rotation = saved_orient * origin_objp->orient;
		}
	}

	// Apply to all marked objects
	for (auto ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (!ptr->flags[Object::Object_Flags::Marked])
			continue;

		// Skip the origin in the second pass
		if (relativeToOrigin && ptr == origin_objp)
			continue;

		if (relativeToOrigin) {
			// Transform relative to new origin pose
			vec3d relative_pos, transformed_pos;
			vm_vec_sub(&relative_pos, &ptr->pos, &origin_prev_pos);
			vm_vec_unrotate(&transformed_pos, &relative_pos, &origin_rotation);
			vm_vec_add(&ptr->pos, &transformed_pos, &origin_objp->pos);

			ptr->orient = ptr->orient * origin_rotation;
			_editor->missionChanged();
		} else {
			// Independent transform of each marked object
			if (change_pos) {
				vm_vec_add2(&ptr->pos, &delta);
				_editor->missionChanged();
			}
			if (_pointTo) {
				updateObject(ptr);
				_editor->missionChanged();
			} else if (change_orient) {
				ptr->orient = desired_orient;
				_editor->missionChanged();
			}
		}
	}

	// Notify the engine about moved objects
	if (change_pos || relativeToOrigin) {
		for (auto ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
			if (ptr->flags[Object::Object_Flags::Marked]) {
				object_moved(ptr);
			}
		}
	}

	return true;
}

void ObjectOrientEditorDialogModel::reject() {
	// Do nothing
}

void ObjectOrientEditorDialogModel::setPositionX(float x)
{
	if (!is_close(_position.xyz.x, x)) {
		modify(_position.xyz.x, round1(x));
	}
}

void ObjectOrientEditorDialogModel::setPositionY(float y)
{
	if (!is_close(_position.xyz.y, y)) {
		modify(_position.xyz.y, round1(y));
	}
}
void ObjectOrientEditorDialogModel::setPositionZ(float z)
{
	if (!is_close(_position.xyz.z, z)) {
		modify(_position.xyz.z, round1(z));
	}
}

ObjectPosition ObjectOrientEditorDialogModel::getPosition() const
{
	return {_position.xyz.x, _position.xyz.y, _position.xyz.z};
}

void ObjectOrientEditorDialogModel::setOrientationP(float deg)
{
	float val = normalize_degrees(round1(deg));
	if (!is_close(_orientationDeg.xyz.x, deg)) {
		modify(_orientationDeg.xyz.x, val);
	}
}

void ObjectOrientEditorDialogModel::setOrientationB(float deg)
{
	float val = normalize_degrees(round1(deg));
	if (!is_close(_orientationDeg.xyz.y, deg)) {
		modify(_orientationDeg.xyz.y, val);
	}
}

void ObjectOrientEditorDialogModel::setOrientationH(float deg)
{
	float val = normalize_degrees(round1(deg));
	if (!is_close(_orientationDeg.xyz.z, deg)) {
		modify(_orientationDeg.xyz.z, val);
	}
}

ObjectOrientation ObjectOrientEditorDialogModel::getOrientation() const
{
	return {normalize_degrees(_orientationDeg.xyz.x),
		normalize_degrees(_orientationDeg.xyz.y),
		normalize_degrees(_orientationDeg.xyz.z)};
}

void ObjectOrientEditorDialogModel::setSetMode(SetMode mode)
{
	if (_setMode == mode) {
		return;
	}

	// Current object pose for capturing baseline when entering Relative
	const object& obj = Objects[_editor->currentObject];

	angles objAng{};
	vm_extract_angles_matrix(&objAng, &obj.orient);

	vec3d objAngDeg;
	objAngDeg.xyz.x = normalize_degrees(fl_degrees(objAng.p));
	objAngDeg.xyz.y = normalize_degrees(fl_degrees(objAng.b));
	objAngDeg.xyz.z = normalize_degrees(fl_degrees(objAng.h));

	if (mode == SetMode::Relative) {
		// Capture baseline once when switching to Relative
		_rebaseRefPos = obj.pos;
		_rebaseRefAnglesDeg = objAngDeg;

		// Absolute to Relative: subtract the captured baseline
		_position.xyz.x -= _rebaseRefPos.xyz.x;
		_position.xyz.y -= _rebaseRefPos.xyz.y;
		_position.xyz.z -= _rebaseRefPos.xyz.z;

		_orientationDeg.xyz.x = normalize_degrees(_orientationDeg.xyz.x - _rebaseRefAnglesDeg.xyz.x);
		_orientationDeg.xyz.y = normalize_degrees(_orientationDeg.xyz.y - _rebaseRefAnglesDeg.xyz.y);
		_orientationDeg.xyz.z = normalize_degrees(_orientationDeg.xyz.z - _rebaseRefAnglesDeg.xyz.z);
	} else {
		// Relative to Absolute: add the same captured baseline
		_position.xyz.x += _rebaseRefPos.xyz.x;
		_position.xyz.y += _rebaseRefPos.xyz.y;
		_position.xyz.z += _rebaseRefPos.xyz.z;

		_orientationDeg.xyz.x = normalize_degrees(_orientationDeg.xyz.x + _rebaseRefAnglesDeg.xyz.x);
		_orientationDeg.xyz.y = normalize_degrees(_orientationDeg.xyz.y + _rebaseRefAnglesDeg.xyz.y);
		_orientationDeg.xyz.z = normalize_degrees(_orientationDeg.xyz.z + _rebaseRefAnglesDeg.xyz.z);
	}

	// Round to one decimal and normalize angles
	_position.xyz.x = round1(_position.xyz.x);
	_position.xyz.y = round1(_position.xyz.y);
	_position.xyz.z = round1(_position.xyz.z);

	_orientationDeg.xyz.x = normalize_degrees(round1(_orientationDeg.xyz.x));
	_orientationDeg.xyz.y = normalize_degrees(round1(_orientationDeg.xyz.y));
	_orientationDeg.xyz.z = normalize_degrees(round1(_orientationDeg.xyz.z));

	modify(_setMode, mode);
}

ObjectOrientEditorDialogModel::SetMode ObjectOrientEditorDialogModel::getSetMode() const
{
	return _setMode;
}

void ObjectOrientEditorDialogModel::setTransformMode(TransformMode mode)
{
	modify(_transformMode, mode);
}

ObjectOrientEditorDialogModel::TransformMode ObjectOrientEditorDialogModel::getTransformMode() const
{
	return _transformMode;
}

void ObjectOrientEditorDialogModel::setPointTo(bool point_to)
{
	modify(_pointTo, point_to);
}

bool ObjectOrientEditorDialogModel::getPointTo() const
{
	return _pointTo;
}

void ObjectOrientEditorDialogModel::setPointMode(ObjectOrientEditorDialogModel::PointToMode pointMode)
{
	modify(_pointMode, pointMode);
}

ObjectOrientEditorDialogModel::PointToMode ObjectOrientEditorDialogModel::getPointMode() const
{
	return _pointMode;
}

void ObjectOrientEditorDialogModel::setPointToObjectIndex(int selectedObjectNum)
{
	modify(_selectedPointToObjectIndex, selectedObjectNum);
}

int ObjectOrientEditorDialogModel::getPointToObjectIndex() const
{
	return _selectedPointToObjectIndex;
}

void ObjectOrientEditorDialogModel::setLocationX(float x)
{
	if (!is_close(_location.xyz.x, x)) {
		modify(_location.xyz.x, round1(x));
	}
}

void ObjectOrientEditorDialogModel::setLocationY(float y)
{
	if (!is_close(_location.xyz.y, y)) {
		modify(_location.xyz.y, round1(y));
	}
}

void ObjectOrientEditorDialogModel::setLocationZ(float z)
{
	if (!is_close(_location.xyz.z, z)) {
		modify(_location.xyz.z, round1(z));
	}
}

ObjectPosition ObjectOrientEditorDialogModel::getLocation() const
{
	return {_location.xyz.x, _location.xyz.y, _location.xyz.z};
}

} // namespace fso::fred::dialogs
