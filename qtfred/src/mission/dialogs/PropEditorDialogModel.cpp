#include "mission/dialogs/PropEditorDialogModel.h"

#include <globalincs/linklist.h>
#include <mission/object.h>
#include <prop/prop.h>

#include <unordered_set>

namespace fso::fred::dialogs {

PropEditorDialogModel::PropEditorDialogModel(QObject* parent, EditorViewport* viewport) : AbstractDialogModel(parent, viewport) {
	connect(viewport->editor, &Editor::currentObjectChanged, this, &PropEditorDialogModel::onSelectedObjectChanged);
	connect(viewport->editor, &Editor::objectMarkingChanged, this, &PropEditorDialogModel::onSelectedObjectMarkingChanged);
	connect(viewport->editor, &Editor::missionChanged, this, &PropEditorDialogModel::onMissionChanged);

	initializeData();
}

bool PropEditorDialogModel::apply() {
	return true;
}

void PropEditorDialogModel::reject() {}


void PropEditorDialogModel::initializeData() {
	_flagLabels.clear();
	_flagState.clear();
	_selectedPropObjects = computeSelectedPropObjects();

	for (size_t i = 0; i < Num_parse_prop_flags; ++i) {
		_flagLabels.emplace_back(Parse_prop_flags[i].name, i);
		_flagState.push_back(Qt::Unchecked);
	}

	if (hasValidSelection()) {
		if (!hasMultipleSelection()) {
			auto prp = prop_id_lookup(Objects[_selectedPropObjects.front()].instance);
			Assertion(prp != nullptr, "Selected prop could not be found.");
			_propName = prp->prop_name;
		} else {
			_propName.clear();
		}

		for (size_t i = 0; i < _flagLabels.size(); ++i) {
			bool first = true;
			for (auto obj_idx : _selectedPropObjects) {
				if (!query_valid_object(obj_idx) || Objects[obj_idx].type != OBJ_PROP) {
					continue;
				}

				auto value = getFlagValueForObject(Objects[obj_idx], _flagLabels[i].second);
				if (first) {
					_flagState[i] = value ? Qt::Checked : Qt::Unchecked;
					first = false;
				} else {
					_flagState[i] = tristate_set(value, _flagState[i]);
				}
			}
		}
	} else {
		_propName.clear();
	}

	Q_EMIT modelDataChanged();
	_modified = false;
}


void PropEditorDialogModel::showErrorDialogNoCancel(const SCP_string& message) {
	if (_bypass_errors) {
		return;
	}

	_bypass_errors = true;
	_viewport->dialogProvider->showButtonDialog(DialogType::Error, "Error", message, {DialogButton::Ok});
}

void PropEditorDialogModel::selectPropFromObjectList(object* start, bool forward) {
	auto ptr = start;

	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->type == OBJ_PROP) {
			_editor->unmark_all();
			_editor->markObject(OBJ_INDEX(ptr));
			return;
		}
		ptr = forward ? GET_NEXT(ptr) : GET_PREV(ptr);
	}

	ptr = forward ? GET_FIRST(&obj_used_list) : GET_LAST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->type == OBJ_PROP) {
			_editor->unmark_all();
			_editor->markObject(OBJ_INDEX(ptr));
			return;
		}
		ptr = forward ? GET_NEXT(ptr) : GET_PREV(ptr);
	}
}

void PropEditorDialogModel::selectFirstPropInMission() {
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr->type == OBJ_PROP) {
			_editor->unmark_all();
			_editor->markObject(OBJ_INDEX(ptr));
			return;
		}
	}
}

const SCP_vector<int>& PropEditorDialogModel::getSelectedPropObjects() const {
	return _selectedPropObjects;
}

SCP_vector<int> PropEditorDialogModel::computeSelectedPropObjects() const {
	SCP_vector<int> selected;
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr->type == OBJ_PROP && ptr->flags[Object::Object_Flags::Marked]) {
			selected.push_back(OBJ_INDEX(ptr));
		}
	}

	if (selected.empty() && query_valid_object(_editor->currentObject) && Objects[_editor->currentObject].type == OBJ_PROP) {
		selected.push_back(_editor->currentObject);
	}

	return selected;
}

bool PropEditorDialogModel::getFlagValueForObject(const object& obj, size_t flag_index) {
	if (flag_index >= Num_parse_prop_flags) {
		return false;
	}

	auto& def = Parse_prop_flags[flag_index];
	if (!stricmp(def.name, "no_collide")) {
		return !obj.flags[Object::Object_Flags::Collides];
	}

	return false;
}

int PropEditorDialogModel::tristate_set(bool value, int current_state) {
	if (value) {
		if (current_state == Qt::Unchecked) {
			return Qt::PartiallyChecked;
		}
	} else {
		if (current_state == Qt::Checked) {
			return Qt::PartiallyChecked;
		}
	}

	if (current_state == Qt::PartiallyChecked) {
		return Qt::PartiallyChecked;
	}

	return value ? Qt::Checked : Qt::Unchecked;
}

bool PropEditorDialogModel::hasValidSelection() const {
	return !_selectedPropObjects.empty();
}

bool PropEditorDialogModel::hasMultipleSelection() const {
	return _selectedPropObjects.size() > 1;
}

bool PropEditorDialogModel::hasAnyPropsInMission() {
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr->type == OBJ_PROP) {
			return true;
		}
	}

	return false;
}

const SCP_string& PropEditorDialogModel::getPropName() const {
	return _propName;
}

bool PropEditorDialogModel::setPropName(const SCP_string& name) {
	if (hasMultipleSelection()) {
		return true;
	}

	_bypass_errors = false;

	SCP_string trimmed = name;
	SCP_trim(trimmed);

	if (trimmed.empty()) {
		showErrorDialogNoCancel("A prop name cannot be empty.");
		return false;
	}

	std::unordered_set<int> selected_instances;
	for (auto obj_idx : _selectedPropObjects) {
		if (query_valid_object(obj_idx) && Objects[obj_idx].type == OBJ_PROP) {
			selected_instances.insert(Objects[obj_idx].instance);
		}
	}

	for (size_t i = 0; i < Props.size(); ++i) {
		if (selected_instances.find(static_cast<int>(i)) != selected_instances.end() || !Props[i].has_value()) {
			continue;
		}
		if (!stricmp(trimmed.c_str(), Props[i].value().prop_name)) {
			showErrorDialogNoCancel("This prop name is already being used by another prop.");
			return false;
		}
	}

	auto obj_idx = _selectedPropObjects.front();
	auto prp = prop_id_lookup(Objects[obj_idx].instance);
	if (prp == nullptr) {
		return false;
	}

	strcpy_s(prp->prop_name, trimmed.c_str());
	_propName = trimmed;
	set_modified();
	_editor->missionChanged();
	return true;
}

const SCP_vector<std::pair<SCP_string, size_t>>& PropEditorDialogModel::getFlagLabels() const {
	return _flagLabels;
}

const SCP_vector<int>& PropEditorDialogModel::getFlagState() const {
	return _flagState;
}

void PropEditorDialogModel::setFlagState(size_t index, int state) {
	if (!SCP_vector_inbounds(_flagState, index)) {
		return;
	}

	if (_flagState[index] == state) {
		return;
	}

	_flagState[index] = state;

	if (state == Qt::PartiallyChecked) {
		return;
	}

	auto flag_index = _flagLabels[index].second;
	if (flag_index >= Num_parse_prop_flags) {
		return;
	}

	auto& def = Parse_prop_flags[flag_index];
	for (auto obj_idx : _selectedPropObjects) {
		if (!query_valid_object(obj_idx) || Objects[obj_idx].type != OBJ_PROP) {
			continue;
		}
		if (!stricmp(def.name, "no_collide")) {
			Objects[obj_idx].flags.set(Object::Object_Flags::Collides, state != Qt::Checked);
		}
	}
	set_modified();
	// Caller is responsible for triggering missionChanged() (deferred to avoid FlagListWidget re-entrancy)
}

SCP_string PropEditorDialogModel::getLayer() const
{
	SCP_string result;
	bool first = true;
	for (auto obj_idx : _selectedPropObjects) {
		if (!query_valid_object(obj_idx) || Objects[obj_idx].type != OBJ_PROP)
			continue;
		SCP_string layer = _viewport->getObjectLayerName(obj_idx);
		if (first) {
			result = layer;
			first = false;
		} else if (result != layer) {
			return "";
		}
	}
	return result;
}

void PropEditorDialogModel::setLayer(const SCP_string& layer)
{
	for (auto obj_idx : _selectedPropObjects) {
		if (!query_valid_object(obj_idx) || Objects[obj_idx].type != OBJ_PROP)
			continue;
		_viewport->moveObjectToLayer(obj_idx, layer);
	}
	set_modified();
	_editor->missionChanged();
}

void PropEditorDialogModel::selectNextProp() {
	if (!hasValidSelection()) {
		if (hasAnyPropsInMission()) {
			selectFirstPropInMission();
		}
		return;
	}
	selectPropFromObjectList(GET_NEXT(&Objects[_selectedPropObjects.front()]), true);
}

void PropEditorDialogModel::selectPreviousProp() {
	if (!hasValidSelection()) {
		if (hasAnyPropsInMission()) {
			selectFirstPropInMission();
		}
		return;
	}
	selectPropFromObjectList(GET_PREV(&Objects[_selectedPropObjects.front()]), false);
}

void PropEditorDialogModel::onSelectedObjectChanged(int) {
	initializeData();
}

void PropEditorDialogModel::onSelectedObjectMarkingChanged(int, bool) {
	initializeData();
}

void PropEditorDialogModel::onMissionChanged() {
	initializeData();
}

SCP_vector<std::pair<SCP_string, SCP_string>> PropEditorDialogModel::getPropFlagDescriptions()
{
	const size_t num_descs = Num_parse_prop_flag_descriptions;
	SCP_vector<std::pair<SCP_string, SCP_string>> descriptions;
	descriptions.reserve(Num_parse_prop_flags);
	for (size_t i = 0; i < Num_parse_prop_flags; ++i) {
		const auto& flagDef = Parse_prop_flags[i];
		for (size_t j = 0; j < num_descs; ++j) {
			if (Parse_prop_flag_descriptions[j].def == flagDef.def) {
				descriptions.emplace_back(flagDef.name, Parse_prop_flag_descriptions[j].flag_desc);
				break;
			}
		}
	}
	return descriptions;
}

}
