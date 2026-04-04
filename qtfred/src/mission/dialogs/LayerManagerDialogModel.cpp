#include "LayerManagerDialogModel.h"

#include "mission/EditorViewport.h"
#include <iff_defs/iff_defs.h>

namespace fso::fred::dialogs {

LayerManagerDialogModel::LayerManagerDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	// Ensure the Show_iff vector is sized for all current IFF entries
	while ((int)_viewport->view.Show_iff.size() < (int)Iff_info.size()) {
		_viewport->view.Show_iff.push_back(true);
	}
}

bool LayerManagerDialogModel::apply() {
	// All changes in this dialog are applied immediately; nothing to do here.
	return true;
}

void LayerManagerDialogModel::reject() {
	// No staged changes to discard.
}

// --- Layers ---

SCP_vector<SCP_string> LayerManagerDialogModel::getLayerNames() const {
	return _viewport->getLayerNames();
}

bool LayerManagerDialogModel::getLayerVisibility(const SCP_string& name) const {
	bool visible = true;
	_viewport->getLayerVisibility(name, &visible);
	return visible;
}

bool LayerManagerDialogModel::setLayerVisibility(const SCP_string& name, bool visible, SCP_string* error) {
	if (!_viewport->setLayerVisibility(name, visible, error)) {
		return false;
	}
	modelChanged();
	return true;
}

bool LayerManagerDialogModel::addLayer(const SCP_string& name, SCP_string* error) {
	if (!_viewport->addLayer(name, error)) {
		return false;
	}
	modelChanged();
	return true;
}

bool LayerManagerDialogModel::deleteLayer(const SCP_string& name, SCP_string* error) {
	if (!_viewport->deleteLayer(name, error)) {
		return false;
	}
	modelChanged();
	return true;
}

bool LayerManagerDialogModel::isDefaultLayer(const SCP_string& name) {
	return name == EditorViewport::DefaultLayerName;
}

// --- Object type filters ---

bool LayerManagerDialogModel::getShowShips() const    { return _viewport->view.Show_ships; }
bool LayerManagerDialogModel::getShowStarts() const   { return _viewport->view.Show_starts; }
bool LayerManagerDialogModel::getShowWaypoints() const { return _viewport->view.Show_waypoints; }

void LayerManagerDialogModel::setShowShips(bool value) {
	if (_viewport->view.Show_ships != value) {
		_viewport->view.Show_ships = value;
		_viewport->needsUpdate();
	}
}

void LayerManagerDialogModel::setShowStarts(bool value) {
	if (_viewport->view.Show_starts != value) {
		_viewport->view.Show_starts = value;
		_viewport->needsUpdate();
	}
}

void LayerManagerDialogModel::setShowWaypoints(bool value) {
	if (_viewport->view.Show_waypoints != value) {
		_viewport->view.Show_waypoints = value;
		_viewport->needsUpdate();
	}
}

// --- IFF team filters ---

int LayerManagerDialogModel::getIffCount() {
	return static_cast<int>(Iff_info.size());
}

SCP_string LayerManagerDialogModel::getIffName(int index) {
	return Iff_info[index].iff_name;
}

bool LayerManagerDialogModel::getShowIff(int index) const {
	return _viewport->view.Show_iff[index];
}

void LayerManagerDialogModel::setShowIff(int index, bool value) {
	if (_viewport->view.Show_iff[index] != value) {
		_viewport->view.Show_iff[index] = value;
		_viewport->needsUpdate();
	}
}

} // namespace fso::fred::dialogs
