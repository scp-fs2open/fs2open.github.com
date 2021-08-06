#include <jumpnode/jumpnode.h>
#include <mission/object.h>
#include <globalincs/linklist.h>
#include <ship/ship.h>
#include <iff_defs/iff_defs.h>
#include "mission/dialogs/WaypointEditorDialogModel.h"

namespace fso {
namespace fred {
namespace dialogs {

WaypointEditorDialogModel::WaypointEditorDialogModel(QObject* parent, EditorViewport* viewport) :
	AbstractDialogModel(parent, viewport) {
	connect(viewport->editor, &Editor::currentObjectChanged, this, &WaypointEditorDialogModel::onSelectedObjectChanged);
	connect(viewport->editor,
			&Editor::objectMarkingChanged,
			this,
			&WaypointEditorDialogModel::onSelectedObjectMarkingChanged);
	connect(viewport->editor, &Editor::missionChanged, this, &WaypointEditorDialogModel::missionChanged);

	initializeData();
}
bool WaypointEditorDialogModel::showErrorDialog(const SCP_string& message, const SCP_string& title) {
	if (bypass_errors) {
		return true;
	}

	bypass_errors = 1;
	auto z = _viewport->dialogProvider->showButtonDialog(DialogType::Error,
														 title,
														 message,
														 { DialogButton::Ok, DialogButton::Cancel });

	if (z == DialogButton::Cancel) {
		return true;
	}

	return false;
}
bool WaypointEditorDialogModel::apply() {
	// Reset flag before applying
	bypass_errors = false;

	const char* str;
	char old_name[255];
	int i;
	object* ptr;
	SCP_list<CJumpNode>::iterator jnp;

	if (query_valid_object(_editor->currentObject) && Objects[_editor->currentObject].type == OBJ_WAYPOINT) {
		Assert(
			_editor->cur_waypoint_list == find_waypoint_list_with_instance(Objects[_editor->currentObject].instance));
	}

	if (_editor->cur_waypoint_list != NULL) {
		for (i = 0; i < MAX_WINGS; i++) {
			if (!stricmp(Wings[i].name, _currentName.c_str())) {
				if (showErrorDialog("This waypoint path name is already being used by a wing\n"
										"Press OK to restore old name", "Error")) {
					return false;
				}

				_currentName = _editor->cur_waypoint_list->get_name();
				modelChanged();
			}
		}

		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) {
				if (!stricmp(_currentName.c_str(), Ships[ptr->instance].ship_name)) {
					if (showErrorDialog("This waypoint path name is already being used by a ship\n"
											"Press OK to restore old name", "Error")) {
						return false;
					}

					_currentName = _editor->cur_waypoint_list->get_name();
					modelChanged();
				}
			}

			ptr = GET_NEXT(ptr);
		}

		// We don't need to check teams.  "Unknown" is a valid name and also an IFF.

		for (i = 0; i < (int) Ai_tp_list.size(); i++) {
			if (!stricmp(_currentName.c_str(), Ai_tp_list[i].name)) {
				if (showErrorDialog("This waypoint path name is already being used by a target priority group.\n"
										"Press OK to restore old name", "Error")) {
					return false;
				}

				_currentName = _editor->cur_waypoint_list->get_name();
				modelChanged();
			}
		}

		SCP_list<waypoint_list>::iterator ii;
		for (ii = Waypoint_lists.begin(); ii != Waypoint_lists.end(); ++ii) {
			if (!stricmp(ii->get_name(), _currentName.c_str()) && (&(*ii) != _editor->cur_waypoint_list)) {
				if (showErrorDialog("This waypoint path name is already being used by another waypoint path\n"
										"Press OK to restore old name", "Error")) {
					return false;
				}

				_currentName = _editor->cur_waypoint_list->get_name();
				modelChanged();
			}
		}

		if (jumpnode_get_by_name(_currentName.c_str()) != NULL) {
			if (showErrorDialog("This waypoint path name is already being used by a jump node\n"
									"Press OK to restore old name", "Error")) {
				return false;
			}

			_currentName = _editor->cur_waypoint_list->get_name();
			modelChanged();
		}

		if (_currentName[0] == '<') {
			if (showErrorDialog("Waypoint names not allowed to begin with <\n"
									"Press OK to restore old name", "Error")) {
				return false;
			}

			_currentName = _editor->cur_waypoint_list->get_name();
			modelChanged();
		}


		strcpy_s(old_name, _editor->cur_waypoint_list->get_name());
		strcpy_s(_editor->cur_waypoint_list->get_name(), NAME_LENGTH, _currentName.c_str());

		str = _currentName.c_str();
		if (strcmp(old_name, str) != 0) {
			update_sexp_references(old_name, str);
			_editor->ai_update_goal_references(REF_TYPE_WAYPOINT, old_name, str);
			_editor->update_texture_replacements(old_name, str);
		}

		_editor->missionChanged();
	} else if (Objects[_editor->currentObject].type == OBJ_JUMP_NODE) {
		for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {
			if (jnp->GetSCPObject() == &Objects[_editor->currentObject]) {
				break;
			}
		}

		for (i = 0; i < MAX_WINGS; i++) {
			if (!stricmp(Wings[i].name, _currentName.c_str())) {
				if (showErrorDialog("This jump node name is already being used by a wing\n"
										"Press OK to restore old name", "Error")) {
					return false;
				}

				_currentName = jnp->GetName();
				modelChanged();
			}
		}

		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if ((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) {
				if (!stricmp(_currentName.c_str(), Ships[ptr->instance].ship_name)) {
					if (showErrorDialog("This jump node name is already being used by a ship\n"
											"Press OK to restore old name", "Error")) {
						return false;
					}

					_currentName = jnp->GetName();
					modelChanged();
				}
			}

			ptr = GET_NEXT(ptr);
		}

		// We don't need to check teams.  "Unknown" is a valid name and also an IFF.

		for (i = 0; i < (int) Ai_tp_list.size(); i++) {
			if (!stricmp(_currentName.c_str(), Ai_tp_list[i].name)) {
				if (showErrorDialog("This jump node name is already being used by a target priority group.\n"
										"Press OK to restore old name", "Error")) {
					return false;
				}

				_currentName = jnp->GetName();
				modelChanged();
			}
		}

		if (find_matching_waypoint_list(_currentName.c_str()) != NULL) {
			if (showErrorDialog("This jump node name is already being used by a waypoint path\n"
									"Press OK to restore old name", "Error")) {
				return false;
			}

			_currentName = jnp->GetName();
			modelChanged();
		}

		if (_currentName[0] == '<') {
			if (showErrorDialog("Jump node names not allowed to begin with <\n"
									"Press OK to restore old name", "Error")) {
				return false;
			}

			_currentName = jnp->GetName();
			modelChanged();
		}

		CJumpNode* found = jumpnode_get_by_name(_currentName.c_str());
		if (found != NULL && &(*jnp) != found) {
			if (showErrorDialog("This jump node name is already being used by another jump node\n"
									"Press OK to restore old name", "Error")) {
				return false;
			}

			_currentName = jnp->GetName();
			modelChanged();
		}

		strcpy_s(old_name, jnp->GetName());
		jnp->SetName(_currentName.c_str());

		str = _currentName.c_str();
		if (strcmp(old_name, str) != 0) {
			update_sexp_references(old_name, str);
		}

		_editor->missionChanged();
	}

	return true;
}
void WaypointEditorDialogModel::reject() {
}
void WaypointEditorDialogModel::onSelectedObjectChanged(int) {
	initializeData();
}
void WaypointEditorDialogModel::onSelectedObjectMarkingChanged(int, bool) {
	initializeData();
}
void WaypointEditorDialogModel::initializeData() {
	_enabled = true;
	SCP_list<CJumpNode>::iterator jnp;

	updateElementList();

	if (query_valid_object(_editor->currentObject) && Objects[_editor->currentObject].type == OBJ_WAYPOINT) {
		Assert(
			_editor->cur_waypoint_list == find_waypoint_list_with_instance(Objects[_editor->currentObject].instance));
	}

	if (_editor->cur_waypoint_list != NULL) {
		_currentName = _editor->cur_waypoint_list->get_name();
	} else if (Objects[_editor->currentObject].type == OBJ_JUMP_NODE) {
		for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {
			if (jnp->GetSCPObject() == &Objects[_editor->currentObject]) {
				break;
			}
		}

		_currentName = jnp->GetName();
	} else {
		_currentName = "";
		_enabled = false;
	}

	modelChanged();
}
const SCP_string& WaypointEditorDialogModel::getCurrentName() const {
	return _currentName;
}
int WaypointEditorDialogModel::getCurrentElementId() const {
	return _currentElementId;
}
bool WaypointEditorDialogModel::isEnabled() const {
	return _enabled;
}
const SCP_vector<WaypointEditorDialogModel::PointListElement>& WaypointEditorDialogModel::getElements() const {
	return _elements;
}
void WaypointEditorDialogModel::updateElementList() {
	int i;
	SCP_list<waypoint_list>::iterator ii;
	SCP_list<CJumpNode>::iterator jnp;

	_elements.clear();
	_currentElementId = -1;

	for (i = 0, ii = Waypoint_lists.begin(); ii != Waypoint_lists.end(); ++i, ++ii) {
		_elements.push_back(PointListElement(ii->get_name(), ID_WAYPOINT_MENU + i));
	}

	i = 0;
	for (jnp = Jump_nodes.begin(); jnp != Jump_nodes.end(); ++jnp) {
		_elements.push_back(PointListElement(jnp->GetName(), ID_JUMP_NODE_MENU + i));
		if (jnp->GetSCPObjectNumber() == _editor->currentObject) {
			_currentElementId = ID_JUMP_NODE_MENU + i;
		}
		i++;

	}

	if (_editor->cur_waypoint_list != NULL) {
		int index = find_index_of_waypoint_list(_editor->cur_waypoint_list);
		Assert(index >= 0);
		_currentElementId = ID_WAYPOINT_MENU + index;
	}
}
void WaypointEditorDialogModel::idSelected(int id) {
	if (_currentElementId == id) {
		// Nothing to do here
		return;
	}

	int point;
	object* ptr;

	if ((id >= ID_WAYPOINT_MENU) && (id < ID_WAYPOINT_MENU + (int) Waypoint_lists.size())) {
		if (apply()) {
			point = id - ID_WAYPOINT_MENU;
			_editor->unmark_all();
			ptr = GET_FIRST(&obj_used_list);
			while (ptr != END_OF_LIST(&obj_used_list)) {
				if (ptr->type == OBJ_WAYPOINT) {
					if (calc_waypoint_list_index(ptr->instance) == point) {
						_editor->markObject(OBJ_INDEX(ptr));
					}
				}

				ptr = GET_NEXT(ptr);
			}

			return;
		}
	}

	if ((id >= ID_JUMP_NODE_MENU) && (id < ID_JUMP_NODE_MENU + (int) Jump_nodes.size())) {
		if (apply()) {
			point = id - ID_JUMP_NODE_MENU;
			_editor->unmark_all();
			ptr = GET_FIRST(&obj_used_list);
			while ((ptr != END_OF_LIST(&obj_used_list)) && (point > -1)) {
				if (ptr->type == OBJ_JUMP_NODE) {
					if (point == 0) {
						_editor->markObject(OBJ_INDEX(ptr));
					}
					point--;
				}

				ptr = GET_NEXT(ptr);
			}

			return;
		}
	}
}
void WaypointEditorDialogModel::setNameEditText(const SCP_string& name) {
	_currentName = name;

	modelChanged();
}
void WaypointEditorDialogModel::missionChanged() {
	// When the mission is changed we also need to update our data in case one of our elements changed
	initializeData();
}

WaypointEditorDialogModel::PointListElement::PointListElement(const SCP_string& in_name, int in_id) :
	name(in_name), id(in_id) {
}
}
}
}
