#include "JumpNodeEditorDialogModel.h"

#include "globalincs/linklist.h"
#include <jumpnode/jumpnode.h>
#include <localization/localize.h>
#include <mission/missionparse.h>
#include <mission/object.h>
#include <model/model.h>
#include <ship/ship.h>

namespace fso::fred::dialogs {

JumpNodeEditorDialogModel::JumpNodeEditorDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	connect(viewport->editor, &Editor::currentObjectChanged, this, &JumpNodeEditorDialogModel::onSelectedObjectChanged);
	connect(viewport->editor, &Editor::objectMarkingChanged, this, &JumpNodeEditorDialogModel::onSelectedObjectMarkingChanged);
	connect(viewport->editor, &Editor::missionChanged, this, &JumpNodeEditorDialogModel::onMissionChanged);

	initializeData();
}

bool JumpNodeEditorDialogModel::apply() {
	return true;
}

void JumpNodeEditorDialogModel::reject() {}

void JumpNodeEditorDialogModel::initializeData()
{
	_selectedJumpNodes.clear();
	_redMixed = _greenMixed = _blueMixed = _alphaMixed = false;
	_hiddenMixed = false;

	// Collect all marked OBJ_JUMP_NODE objects whose JN entry still exists.
	// Guard against mid-delete notifications where the object lingers in obj_used_list
	// but the jump node has already been removed (e.g. during undo of CreateObjectCommand).
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr->type == OBJ_JUMP_NODE && ptr->flags[Object::Object_Flags::Marked] &&
		    jumpnode_get_by_objnum(OBJ_INDEX(ptr)) != nullptr) {
			_selectedJumpNodes.push_back(OBJ_INDEX(ptr));
		}
	}

	// Fall back to currentObject if nothing is marked
	if (_selectedJumpNodes.empty() && query_valid_object(_editor->currentObject) &&
	    Objects[_editor->currentObject].type == OBJ_JUMP_NODE &&
	    jumpnode_get_by_objnum(_editor->currentObject) != nullptr) {
		_selectedJumpNodes.push_back(_editor->currentObject);
	}

	if (!_selectedJumpNodes.empty()) {
		auto* jnp = jumpnode_get_by_objnum(_selectedJumpNodes.front());

		_name = jnp->GetName();
		_display = jnp->HasDisplayName() ? jnp->GetDisplayName() : "<none>";

		const int model_num = jnp->GetModelNumber();
		if (auto* pm = model_get(model_num)) {
			_modelFilename = pm->filename;
		} else {
			_modelFilename = JN_DEFAULT_MODEL;
		}

		const auto& c = jnp->GetColor();
		_red = c.red;
		_green = c.green;
		_blue = c.blue;
		_alpha = c.alpha;

		_hidden = jnp->IsHidden();

		if (hasMultipleSelection()) {
			bool displayConsistent = true;
			bool modelConsistent   = true;
			for (size_t i = 1; i < _selectedJumpNodes.size(); ++i) {
				auto* other = jumpnode_get_by_objnum(_selectedJumpNodes[i]);
				if (!other) continue;

				SCP_string otherDisplay = other->HasDisplayName() ? other->GetDisplayName() : "<none>";
				if (_display != otherDisplay)
					displayConsistent = false;

				SCP_string otherModel;
				if (auto* pm = model_get(other->GetModelNumber()))
					otherModel = pm->filename;
				else
					otherModel = JN_DEFAULT_MODEL;
				if (_modelFilename != otherModel)
					modelConsistent = false;

				const auto& oc = other->GetColor();
				if (oc.red   != _red)   _redMixed   = true;
				if (oc.green != _green) _greenMixed = true;
				if (oc.blue  != _blue)  _blueMixed  = true;
				if (oc.alpha != _alpha) _alphaMixed = true;

				if (other->IsHidden() != _hidden) _hiddenMixed = true;
			}
			if (!displayConsistent) _display.clear();
			if (!modelConsistent)   _modelFilename.clear();
		}
	} else {
		_name.clear();
		_display.clear();
		_modelFilename.clear();
		_red = _green = _blue = _alpha = 0;
		_hidden = false;
	}

	Q_EMIT jumpNodeMarkingChanged();
	_modified = false;
}

bool JumpNodeEditorDialogModel::hasValidSelection() const {
	return !_selectedJumpNodes.empty();
}

bool JumpNodeEditorDialogModel::hasMultipleSelection() const {
	return _selectedJumpNodes.size() > 1;
}

bool JumpNodeEditorDialogModel::hasAnyNodesInMission() {
	return !Jump_nodes.empty();
}

int JumpNodeEditorDialogModel::getSelectionCount() const {
	return static_cast<int>(_selectedJumpNodes.size());
}

void JumpNodeEditorDialogModel::showErrorDialogNoCancel(const SCP_string& message) {
	if (_bypass_errors) {
		return;
	}
	_bypass_errors = true;
	_viewport->dialogProvider->showButtonDialog(DialogType::Error, "Error", message, {DialogButton::Ok});
}

bool JumpNodeEditorDialogModel::validateName(const SCP_string& name) {
	if (name.empty()) {
		showErrorDialogNoCancel("A jump node name cannot be empty.");
		return false;
	}

	if (name[0] == '<') {
		showErrorDialogNoCancel("Jump node names are not allowed to begin with '<'.");
		return false;
	}

	for (auto& wing : Wings) {
		if (!stricmp(wing.name, name.c_str())) {
			showErrorDialogNoCancel("This jump node name is already being used by a wing.");
			return false;
		}
	}

	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr->type == OBJ_SHIP || ptr->type == OBJ_START) {
			if (!stricmp(name.c_str(), Ships[ptr->instance].ship_name)) {
				showErrorDialogNoCancel("This jump node name is already being used by a ship.");
				return false;
			}
		}
	}

	for (auto& ai : Ai_tp_list) {
		if (!stricmp(name.c_str(), ai.name)) {
			showErrorDialogNoCancel("This jump node name is already being used by a target priority group.");
			return false;
		}
	}

	if (find_matching_waypoint_list(name.c_str()) != nullptr) {
		showErrorDialogNoCancel("This jump node name is already being used by a waypoint path.");
		return false;
	}

	auto* current_jnp = _selectedJumpNodes.empty() ? nullptr : jumpnode_get_by_objnum(_selectedJumpNodes.front());
	auto* found = jumpnode_get_by_name(name.c_str());
	if (found != nullptr && found != current_jnp) {
		showErrorDialogNoCancel("This jump node name is already being used by another jump node.");
		return false;
	}

	return true;
}

bool JumpNodeEditorDialogModel::setName(const SCP_string& v) {
	if (hasMultipleSelection() || _selectedJumpNodes.empty()) {
		return true;
	}

	_bypass_errors = false;

	SCP_string trimmed = v;
	SCP_trim(trimmed);

	if (!validateName(trimmed)) {
		return false;
	}

	auto* jnp = jumpnode_get_by_objnum(_selectedJumpNodes.front());
	if (!jnp) {
		return false;
	}

	char old_name[NAME_LENGTH];
	std::strncpy(old_name, jnp->GetName(), NAME_LENGTH - 1);
	old_name[NAME_LENGTH - 1] = '\0';

	jnp->SetName(trimmed.c_str());

	if (strcmp(old_name, trimmed.c_str()) != 0) {
		update_sexp_references(old_name, trimmed.c_str());
	}

	_name = trimmed;
	set_modified();
	_editor->missionChanged();
	return true;
}

const SCP_string& JumpNodeEditorDialogModel::getName() const { return _name; }

bool JumpNodeEditorDialogModel::setDisplayName(const SCP_string& v) {
	if (_selectedJumpNodes.empty() || v.empty()) {
		return true;
	}

	SCP_string display = v;
	lcl_fred_replace_stuff(display);
	const bool useNodeName = lcase_equal(display, "<none>");

	for (auto objnum : _selectedJumpNodes) {
		auto* jnp = jumpnode_get_by_objnum(objnum);
		if (!jnp) continue;
		jnp->SetDisplayName(useNodeName ? jnp->GetName() : display.c_str());
	}

	_display = useNodeName ? "<none>" : display;
	set_modified();
	_editor->missionChanged();
	return true;
}

const SCP_string& JumpNodeEditorDialogModel::getDisplayName() const { return _display; }

bool JumpNodeEditorDialogModel::setModelFilename(const SCP_string& v) {
	if (_selectedJumpNodes.empty() || v.empty()) {
		return true;
	}

	_bypass_errors = false;

	if (!lcase_equal(v, JN_DEFAULT_MODEL) && !cf_exists_full(v.c_str(), CF_TYPE_MODELS)) {
		showErrorDialogNoCancel("This jump node model file does not exist.");
		return false;
	}

	_modelFilename = v;
	const bool useDefault = lcase_equal(_modelFilename, JN_DEFAULT_MODEL);

	for (auto objnum : _selectedJumpNodes) {
		auto* jnp = jumpnode_get_by_objnum(objnum);
		if (!jnp) continue;
		if (!useDefault) {
			jnp->SetModel(_modelFilename.c_str());
		}
	}

	set_modified();
	_editor->missionChanged();
	return true;
}

const SCP_string& JumpNodeEditorDialogModel::getModelFilename() const { return _modelFilename; }

// Writes one color channel to every selected node. Channels still flagged as mixed
// keep their per-node values; the channel being set is cleared of its mixed flag.
static void applyChannelToAll(const SCP_vector<int>& selected,
    int red, int green, int blue, int alpha,
    bool redMixed, bool greenMixed, bool blueMixed, bool alphaMixed)
{
	for (auto objnum : selected) {
		auto* jnp = jumpnode_get_by_objnum(objnum);
		if (!jnp) continue;
		const auto& c = jnp->GetColor();
		int r = redMixed   ? c.red   : red;
		int g = greenMixed ? c.green : green;
		int b = blueMixed  ? c.blue  : blue;
		int a = alphaMixed ? c.alpha : alpha;
		jnp->SetAlphaColor(r, g, b, a);
	}
}

void JumpNodeEditorDialogModel::setColorR(int v) {
	if (v < 0) return; // mixed-state sentinel from the dialog — no-op
	CLAMP(v, 0, 255);
	_red = v;
	_redMixed = false;
	applyChannelToAll(_selectedJumpNodes, _red, _green, _blue, _alpha,
	    _redMixed, _greenMixed, _blueMixed, _alphaMixed);
	set_modified();
	_editor->missionChanged();
}

int JumpNodeEditorDialogModel::getColorR() const { return _red; }

void JumpNodeEditorDialogModel::setColorG(int v) {
	if (v < 0) return;
	CLAMP(v, 0, 255);
	_green = v;
	_greenMixed = false;
	applyChannelToAll(_selectedJumpNodes, _red, _green, _blue, _alpha,
	    _redMixed, _greenMixed, _blueMixed, _alphaMixed);
	set_modified();
	_editor->missionChanged();
}

int JumpNodeEditorDialogModel::getColorG() const { return _green; }

void JumpNodeEditorDialogModel::setColorB(int v) {
	if (v < 0) return;
	CLAMP(v, 0, 255);
	_blue = v;
	_blueMixed = false;
	applyChannelToAll(_selectedJumpNodes, _red, _green, _blue, _alpha,
	    _redMixed, _greenMixed, _blueMixed, _alphaMixed);
	set_modified();
	_editor->missionChanged();
}

int JumpNodeEditorDialogModel::getColorB() const { return _blue; }

void JumpNodeEditorDialogModel::setColorA(int v) {
	if (v < 0) return;
	CLAMP(v, 0, 255);
	_alpha = v;
	_alphaMixed = false;
	applyChannelToAll(_selectedJumpNodes, _red, _green, _blue, _alpha,
	    _redMixed, _greenMixed, _blueMixed, _alphaMixed);
	set_modified();
	_editor->missionChanged();
}

int JumpNodeEditorDialogModel::getColorA() const { return _alpha; }

bool JumpNodeEditorDialogModel::isColorRMixed() const { return _redMixed; }
bool JumpNodeEditorDialogModel::isColorGMixed() const { return _greenMixed; }
bool JumpNodeEditorDialogModel::isColorBMixed() const { return _blueMixed; }
bool JumpNodeEditorDialogModel::isColorAMixed() const { return _alphaMixed; }
bool JumpNodeEditorDialogModel::hasAnyColorMixed() const {
	return _redMixed || _greenMixed || _blueMixed || _alphaMixed;
}

void JumpNodeEditorDialogModel::setHidden(bool v) {
	_hidden = v;
	_hiddenMixed = false;
	for (auto objnum : _selectedJumpNodes) {
		auto* jnp = jumpnode_get_by_objnum(objnum);
		if (jnp) {
			jnp->SetVisibility(!v);
		}
	}
	set_modified();
	_editor->missionChanged();
}

bool JumpNodeEditorDialogModel::getHidden() const { return _hidden; }

int JumpNodeEditorDialogModel::getHiddenState() const {
	if (_hiddenMixed) return Qt::PartiallyChecked;
	return _hidden ? Qt::Checked : Qt::Unchecked;
}

SCP_string JumpNodeEditorDialogModel::getLayer() const {
	SCP_string result;
	bool first = true;
	for (auto objnum : _selectedJumpNodes) {
		SCP_string layer = _viewport->getObjectLayerName(objnum);
		if (first) {
			result = layer;
			first = false;
		} else if (result != layer) {
			return "";
		}
	}
	return result;
}

void JumpNodeEditorDialogModel::setLayer(const SCP_string& v) {
	for (auto objnum : _selectedJumpNodes) {
		_viewport->moveObjectToLayer(objnum, v);
	}
	set_modified();
	_editor->missionChanged();
}

void JumpNodeEditorDialogModel::selectNodeFromObjectList(object* start, bool forward) {
	auto* ptr = start;
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->type == OBJ_JUMP_NODE) {
			_editor->unmark_all();
			_editor->markObject(OBJ_INDEX(ptr));
			return;
		}
		ptr = forward ? GET_NEXT(ptr) : GET_PREV(ptr);
	}
	// Wrap around
	ptr = forward ? GET_FIRST(&obj_used_list) : GET_LAST(&obj_used_list);
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (ptr->type == OBJ_JUMP_NODE) {
			_editor->unmark_all();
			_editor->markObject(OBJ_INDEX(ptr));
			return;
		}
		ptr = forward ? GET_NEXT(ptr) : GET_PREV(ptr);
	}
}

void JumpNodeEditorDialogModel::selectFirstNodeInMission() {
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr->type == OBJ_JUMP_NODE) {
			_editor->unmark_all();
			_editor->markObject(OBJ_INDEX(ptr));
			return;
		}
	}
}

void JumpNodeEditorDialogModel::selectNextNode() {
	if (!hasValidSelection()) {
		if (hasAnyNodesInMission()) {
			selectFirstNodeInMission();
		}
		return;
	}
	selectNodeFromObjectList(GET_NEXT(&Objects[_selectedJumpNodes.front()]), true);
}

void JumpNodeEditorDialogModel::selectPreviousNode() {
	if (!hasValidSelection()) {
		if (hasAnyNodesInMission()) {
			selectFirstNodeInMission();
		}
		return;
	}
	selectNodeFromObjectList(GET_PREV(&Objects[_selectedJumpNodes.front()]), false);
}

void JumpNodeEditorDialogModel::onSelectedObjectChanged(int) {
	initializeData();
}

void JumpNodeEditorDialogModel::onSelectedObjectMarkingChanged(int, bool) {
	initializeData();
}

void JumpNodeEditorDialogModel::onMissionChanged() {
	initializeData();
}

} // namespace fso::fred::dialogs
