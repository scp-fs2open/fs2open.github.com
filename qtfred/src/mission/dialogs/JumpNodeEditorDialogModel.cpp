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

bool JumpNodeEditorDialogModel::apply()
{
	if (_currentlySelectedNodeIndex < 0) {
		// Nothing to apply
		return true;
	}

	// Validate
	if (!validateData()) {
		return false;
	}

	// Commit
	auto* jnp = jumpnode_get_by_objnum(getSelectedJumpNodeObjnum(_currentlySelectedNodeIndex));
	Assertion(jnp != nullptr, "Jump node not found during apply!");

	char old_name_buf[NAME_LENGTH];
	std::strncpy(old_name_buf, jnp->GetName(), NAME_LENGTH - 1);
	old_name_buf[NAME_LENGTH - 1] = '\0';

	lcl_fred_replace_stuff(_display);

	jnp->SetName(_name.c_str());
	jnp->SetDisplayName(lcase_equal(_display, "<none>") ? _name.c_str() : _display.c_str());

	// Only set a non default model
	if (!lcase_equal(_modelFilename, JN_DEFAULT_MODEL)) {
		jnp->SetModel(_modelFilename.c_str());
	}

	jnp->SetAlphaColor(_red, _green, _blue, _alpha);
	jnp->SetVisibility(!_hidden);

	// Update sexp references when name changes
	if (strcmp(old_name_buf, _name.c_str()) != 0) {
		update_sexp_references(old_name_buf, _name.c_str());
	}

	_editor->missionChanged();
	return true;
}

void JumpNodeEditorDialogModel::reject()
{
	// do nothing
}

void JumpNodeEditorDialogModel::initializeData()
{
	buildNodeList();
	
	// Find the currently selected object if it's a jump node
	int objnum = -1;
	if (query_valid_object(_editor->currentObject) && Objects[_editor->currentObject].type == OBJ_JUMP_NODE) {
		objnum = _editor->currentObject;
	}
	
	if (objnum >= 0) {
		auto* jnp = jumpnode_get_by_objnum(objnum);
		Assertion(jnp != nullptr, "Jump node not found for current object!");

		_name = jnp->GetName();
		_display = jnp->HasDisplayName() ? jnp->GetDisplayName() : "<none>";

		const int model_num = jnp->GetModelNumber();
		if (auto* pm = model_get(model_num)) {
			_modelFilename = pm->filename;
		} else {
			_modelFilename.clear();
		}

		const auto& c = jnp->GetColor();
		_red = c.red;
		_green = c.green;
		_blue = c.blue;
		_alpha = c.alpha;

		_hidden = jnp->IsHidden();

		// Find the index of the jump node in the local list
		for (const auto& node : _nodes) {
			if (!stricmp(node.first.c_str(), _name.c_str())) {
				_currentlySelectedNodeIndex = node.second;
				break;
			}
		}
	} else {
		_name.clear();
		_display.clear();
		_modelFilename.clear();
		_red = _green = _blue = _alpha = 0;
		_hidden = false;

		_currentlySelectedNodeIndex = -1;
	}

	Q_EMIT jumpNodeMarkingChanged();
}

void JumpNodeEditorDialogModel::buildNodeList()
{
	_nodes.clear();
	int idx = 0;
	for (auto& node : Jump_nodes) {
		_nodes.emplace_back(node.GetName(), idx++);
	}
}

bool JumpNodeEditorDialogModel::validateData()
{
	_bypass_errors = false;

	SCP_trim(_name);

	const SCP_string name = _name;
	if (name.empty()) {
		showErrorDialogNoCancel("A jump node name cannot be empty.");
		return false;
	}

	// Disallow leading '<'
	if (!name.empty() && name[0] == '<') {
		showErrorDialogNoCancel("Jump node names are not allowed to begin with '<'.");
		return false;
	}

	// Wing name collision
	for (auto& wing : Wings) {
		if (!stricmp(wing.name, name.c_str())) {
			showErrorDialogNoCancel("This jump node name is already being used by a wing.");
			return false;
		}
	}

	// Ship/start name collision
	for (auto* ptr = GET_FIRST(&obj_used_list); ptr != END_OF_LIST(&obj_used_list); ptr = GET_NEXT(ptr)) {
		if (ptr->type == OBJ_SHIP || ptr->type == OBJ_START) {
			if (!stricmp(name.c_str(), Ships[ptr->instance].ship_name)) {
				showErrorDialogNoCancel("This jump node name is already being used by a ship.");
				return false;
			}
		}
	}

	// AI target priority group collision
	for (auto& ai : Ai_tp_list) {
		if (!stricmp(name.c_str(), ai.name)) {
			showErrorDialogNoCancel("This jump node name is already being used by a target priority group.");
			return false;
		}
	}

	// Waypoint path collision
	if (find_matching_waypoint_list(name.c_str()) != nullptr) {
		showErrorDialogNoCancel("This jump node name is already being used by a waypoint path.");
		return false;
	}

	// Another jump node with the same name (but not this one)
	auto* jnp = jumpnode_get_by_objnum(getSelectedJumpNodeObjnum(_currentlySelectedNodeIndex));
	auto* found = jumpnode_get_by_name(name.c_str());
	if (found != nullptr && found != jnp) {
		showErrorDialogNoCancel("This jump node name is already being used by another jump node.");
		return false;
	}

	if (!cf_exists_full(_modelFilename.c_str(), CF_TYPE_MODELS)) {
		showErrorDialogNoCancel("This jump node model file does not exist.");
		return false;
	}

	return true;
}

void JumpNodeEditorDialogModel::showErrorDialogNoCancel(const SCP_string& message)
{
	if (_bypass_errors) {
		return;
	}

	_bypass_errors = true;
	_viewport->dialogProvider->showButtonDialog(DialogType::Error, "Error", message, {DialogButton::Ok});
}

int JumpNodeEditorDialogModel::getSelectedJumpNodeObjnum(int idx) const
{
	// Find the jump node and then mark it
	for (const auto& node : Jump_nodes) {
		if (!stricmp(node.GetName(), _nodes[idx].first.c_str())) {
			return node.GetSCPObjectNumber();
		}
	}

	return -1;
}

void JumpNodeEditorDialogModel::onSelectedObjectChanged(int)
{
	initializeData();
}

void JumpNodeEditorDialogModel::onSelectedObjectMarkingChanged(int, bool)
{
	initializeData();
}

void JumpNodeEditorDialogModel::onMissionChanged()
{
	initializeData();
}

const SCP_vector<std::pair<SCP_string, int>>& JumpNodeEditorDialogModel::getJumpNodeList() const
{
	return _nodes;
}

void JumpNodeEditorDialogModel::selectJumpNodeByListIndex(int idx)
{
	if (_currentlySelectedNodeIndex == idx) {
		// No change
		return;
	}

	if (!SCP_vector_inbounds(_nodes, idx))
		return;

	if (apply()) {
		_editor->unmark_all();

		int objnum = getSelectedJumpNodeObjnum(idx);

		if (objnum < 0) {
			_currentlySelectedNodeIndex = -1;
			return;
		}

		_editor->markObject(objnum);
		_currentlySelectedNodeIndex = idx;
	}
}

int JumpNodeEditorDialogModel::getCurrentJumpNodeIndex() const
{
	return _currentlySelectedNodeIndex;
}
bool JumpNodeEditorDialogModel::hasValidSelection() const
{
	return _currentlySelectedNodeIndex >= 0;
}

void JumpNodeEditorDialogModel::setName(const SCP_string& v)
{
	SCP_trim(_name);
	
	SCP_string current = _name;

	_name = v;
	if (apply()) {
		set_modified();
	} else {
		_name = current; // restore the old name
	}
}

const SCP_string& JumpNodeEditorDialogModel::getName() const
{
	return _name;
}

void JumpNodeEditorDialogModel::setDisplayName(const SCP_string& v)
{
	modify(_display, v);
	apply(); // Apply changes immediately to update the display name
}

const SCP_string& JumpNodeEditorDialogModel::getDisplayName() const
{
	return _display;
}

void JumpNodeEditorDialogModel::setModelFilename(const SCP_string& v)
{
	SCP_string current = _modelFilename;

	_modelFilename = v;
	if (apply()) {
		set_modified();
	} else {
		_modelFilename = current; // restore the old name
	}
}

const SCP_string& JumpNodeEditorDialogModel::getModelFilename() const
{
	return _modelFilename;
}

void JumpNodeEditorDialogModel::setColorR(int v)
{
	CLAMP(v, 0, 255);
	modify(_red, v);
	apply(); // Apply changes immediately to update the model color
}

int JumpNodeEditorDialogModel::getColorR() const
{
	return _red;
}

void JumpNodeEditorDialogModel::setColorG(int v)
{
	CLAMP(v, 0, 255);
	modify(_green, v);
	apply(); // Apply changes immediately to update the model color
}

int JumpNodeEditorDialogModel::getColorG() const
{
	return _green;
}

void JumpNodeEditorDialogModel::setColorB(int v)
{
	CLAMP(v, 0, 255);
	modify(_blue, v);
	apply(); // Apply changes immediately to update the model color
}

int JumpNodeEditorDialogModel::getColorB() const
{
	return _blue;
}

void JumpNodeEditorDialogModel::setColorA(int v)
{
	CLAMP(v, 0, 255);
	modify(_alpha, v);
	apply(); // Apply changes immediately to update the model color
}

int JumpNodeEditorDialogModel::getColorA() const
{
	return _alpha;
}

void JumpNodeEditorDialogModel::setHidden(bool v)
{
	modify(_hidden, v);
	apply(); // Apply changes immediately to update the visibility
}

bool JumpNodeEditorDialogModel::getHidden() const
{
	return _hidden;
}

} // namespace fso::fred::dialogs
