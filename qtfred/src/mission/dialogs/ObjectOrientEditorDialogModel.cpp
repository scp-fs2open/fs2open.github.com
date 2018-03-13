//
//


#include "ObjectOrientEditorDialogModel.h"

#include <mission/object.h>
#include <globalincs/linklist.h>
#include <ship/ship.h>
#include <math/bitarray.h>

const float PREC = 0.0001f;

namespace fso {
namespace fred {
namespace dialogs {

ObjectOrientEditorDialogModel::ObjectOrientEditorDialogModel(QObject* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport) {
	vm_vec_make(&_location, 0.f, 0.f, 0.f);
	Assert(query_valid_object(_editor->currentObject));
	_position = Objects[_editor->currentObject].pos;

	initializeData();
}
bool ObjectOrientEditorDialogModel::apply() {
	vec3d delta;
	object *ptr;
	
	vm_vec_sub(&delta, &_position, &Objects[_editor->currentObject].pos);

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->flags[Object::Object_Flags::Marked]) {
			vm_vec_add2(&ptr->pos, &delta);
			update_object(ptr);

			_editor->missionChanged();
		}

		ptr = GET_NEXT(ptr);
	}

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->flags[Object::Object_Flags::Marked])
			object_moved(ptr);

		ptr = GET_NEXT(ptr);
	}

	return true;
}

void ObjectOrientEditorDialogModel::update_object(object *ptr)
{
	if (ptr->type != OBJ_WAYPOINT && _point_to) {
		vec3d v;
		matrix m;

		memset(&v, 0, sizeof(vec3d));
		if (_pointMode == PointToMode::Object) {
			if (_selectedObjectNum >= 0) {
				v = Objects[_selectedObjectNum].pos;
				vm_vec_sub2(&v, &ptr->pos);
			}
		}
		else if (_pointMode == PointToMode::Location) {
			vm_vec_sub(&v, &_location, &ptr->pos);
		}
		else {
			Assert(0);  // neither radio button is checked.
		}

		if (!v.xyz.x && !v.xyz.y && !v.xyz.z) {
			return;  // can't point to itself.
		}

		vm_vector_2_matrix(&m, &v, NULL, NULL);
		ptr->orient = m;
	}
}
void ObjectOrientEditorDialogModel::reject() {

}
void ObjectOrientEditorDialogModel::initializeData() {
	char text[80];
	int type;
	object *ptr;


	total = 0;
	_entries.clear();

	ptr = GET_FIRST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (_editor->getNumMarked() != 1 || OBJ_INDEX(ptr) != _editor->currentObject) {
			if ((ptr->type == OBJ_START) || (ptr->type == OBJ_SHIP)) {
				_entries.push_back(ObjectEntry(Ships[ptr->instance].ship_name, OBJ_INDEX(ptr)));
			} else if (ptr->type == OBJ_WAYPOINT) {
				int waypoint_num;
				waypoint_list *wp_list = find_waypoint_list_with_instance(ptr->instance, &waypoint_num);
				Assert(wp_list != NULL);
				sprintf(text, "%s:%d", wp_list->get_name(), waypoint_num + 1);

				_entries.push_back(ObjectEntry(text, OBJ_INDEX(ptr)));
			} else if ((ptr->type == OBJ_POINT) || (ptr->type == OBJ_JUMP_NODE)) {
			} else {
				Assert(0);  // unknown object type
			}
		}

		ptr = GET_NEXT(ptr);
	}

	type = Objects[_editor->currentObject].type;
	if (_editor->getNumMarked() == 1 && type == OBJ_WAYPOINT) {
		_enabled = false;
		_selectedObjectNum = -1;
	} else {
		_selectedObjectNum = _entries.empty() ? -1 : _entries[0].objIndex;
	}

	modelChanged();
}


bool ObjectOrientEditorDialogModel::query_modified()
{
	float dif;

	dif = Objects[_editor->currentObject].pos.xyz.x - _position.xyz.x;
	if ((dif > PREC) || (dif < -PREC))
		return true;
	dif = Objects[_editor->currentObject].pos.xyz.y - _position.xyz.y;
	if ((dif > PREC) || (dif < -PREC))
		return true;
	dif = Objects[_editor->currentObject].pos.xyz.z - _position.xyz.z;
	if ((dif > PREC) || (dif < -PREC))
		return true;

	if (_point_to)
		return true;

	return false;
}
int ObjectOrientEditorDialogModel::getObjectIndex() const {
	return _selectedObjectNum;
}
bool ObjectOrientEditorDialogModel::isPointTo() const {
	return _point_to;
}
const vec3d& ObjectOrientEditorDialogModel::getPosition() const {
	return _position;
}
const vec3d& ObjectOrientEditorDialogModel::getLocation() const {
	return _location;
}
bool ObjectOrientEditorDialogModel::isEnabled() const {
	return _enabled;
}
const SCP_vector<fso::fred::dialogs::ObjectOrientEditorDialogModel::ObjectEntry>&
ObjectOrientEditorDialogModel::getEntries() const {
	return _entries;
}
ObjectOrientEditorDialogModel::PointToMode ObjectOrientEditorDialogModel::getPointMode() const {
	return _pointMode;
}
void ObjectOrientEditorDialogModel::setSelectedObjectNum(int selectedObjectNum) {
	if (_selectedObjectNum != selectedObjectNum) {
		_selectedObjectNum = selectedObjectNum;
		modelChanged();
	}
}
void ObjectOrientEditorDialogModel::setPointTo(bool point_to) {
	if (_point_to != point_to) {
		_point_to = point_to;
		modelChanged();
	}
}
void ObjectOrientEditorDialogModel::setPosition(const vec3d& position) {
	if (_position != position) {
		_position = position;
		modelChanged();
	}
}
void ObjectOrientEditorDialogModel::setLocation(const vec3d& location) {
	if (_location != location) {
		_location = location;
		modelChanged();
	}
}
void ObjectOrientEditorDialogModel::setPointMode(ObjectOrientEditorDialogModel::PointToMode pointMode) {
	if (_pointMode != pointMode) {
		_pointMode = pointMode;
		modelChanged();
	}
}

ObjectOrientEditorDialogModel::ObjectEntry::ObjectEntry(const SCP_string& in_name, int in_objIndex) :
	name(in_name), objIndex(in_objIndex) {
}
}
}
}
