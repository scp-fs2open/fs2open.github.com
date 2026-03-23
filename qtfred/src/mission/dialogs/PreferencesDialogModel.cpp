#include "PreferencesDialogModel.h"

#include "mission/EditorViewport.h"
#include "mission/missiongrid.h"
#include "math/vecmat.h"

namespace fso::fred::dialogs {

PreferencesDialogModel::PreferencesDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
	, _moveShipsWhenUndocking(viewport->Move_ships_when_undocking)
	, _alwaysSaveDisplayNames(viewport->Always_save_display_names)
	, _errorCheckerChecksForPotentialIssues(viewport->Error_checker_checks_potential_issues)
	, _showSexpHelpMissionEvents(viewport->Show_sexp_help_mission_events)
	, _showSexpHelpMissionGoals(viewport->Show_sexp_help_mission_goals)
	, _showSexpHelpMissionCutscenes(viewport->Show_sexp_help_mission_cutscenes)
	, _showSexpHelpShipEditor(viewport->Show_sexp_help_ship_editor)
	, _showSexpHelpWingEditor(viewport->Show_sexp_help_wing_editor)
	, _gridCenterX(static_cast<int>(viewport->The_grid->center.xyz.x))
	, _gridCenterY(static_cast<int>(viewport->The_grid->center.xyz.y))
	, _gridCenterZ(static_cast<int>(viewport->The_grid->center.xyz.z))
{
	auto& bindings = ControlBindings::instance();
	for (const auto& def : bindings.definitions()) {
		_controlKeys.emplace(def.action, bindings.keyFor(def.action));
	}

	// Detect current grid plane from the normal vector (uvec)
	const auto& uvec = viewport->The_grid->gmatrix.vec.uvec;
	if (uvec.xyz.y != 0.0f) {
		_gridPlane = GridPlane::XZ;
	} else if (uvec.xyz.z != 0.0f) {
		_gridPlane = GridPlane::XY;
	} else {
		_gridPlane = GridPlane::YZ;
	}
}

bool PreferencesDialogModel::apply() {
	_viewport->Move_ships_when_undocking = _moveShipsWhenUndocking;
	_viewport->Always_save_display_names = _alwaysSaveDisplayNames;
	_viewport->Error_checker_checks_potential_issues = _errorCheckerChecksForPotentialIssues;
	_viewport->Show_sexp_help_mission_events    = _showSexpHelpMissionEvents;
	_viewport->Show_sexp_help_mission_goals     = _showSexpHelpMissionGoals;
	_viewport->Show_sexp_help_mission_cutscenes = _showSexpHelpMissionCutscenes;
	_viewport->Show_sexp_help_ship_editor       = _showSexpHelpShipEditor;
	_viewport->Show_sexp_help_wing_editor       = _showSexpHelpWingEditor;

	_viewport->saveSettings();

	auto& bindings = ControlBindings::instance();
	for (const auto& entry : _controlKeys) {
		bindings.setKey(entry.first, entry.second);
	}
	bindings.save();

	_viewport->The_grid->center.xyz.x = static_cast<float>(_gridCenterX);
	_viewport->The_grid->center.xyz.y = static_cast<float>(_gridCenterY);
	_viewport->The_grid->center.xyz.z = static_cast<float>(_gridCenterZ);

	switch (_gridPlane) {
	case GridPlane::XY:
		_viewport->The_grid->gmatrix.vec.fvec = vmd_x_vector;
		_viewport->The_grid->gmatrix.vec.rvec = vmd_y_vector;
		break;
	case GridPlane::XZ:
		_viewport->The_grid->gmatrix.vec.fvec = vmd_x_vector;
		_viewport->The_grid->gmatrix.vec.rvec = vmd_z_vector;
		break;
	case GridPlane::YZ:
		_viewport->The_grid->gmatrix.vec.fvec = vmd_y_vector;
		_viewport->The_grid->gmatrix.vec.rvec = vmd_z_vector;
		break;
	}

	modify_grid(_viewport->The_grid);

	return true;
}

void PreferencesDialogModel::reject() {
	// Nothing to do — data sources are not modified until apply()
}

bool PreferencesDialogModel::getMoveShipsWhenUndocking() const { return _moveShipsWhenUndocking; }
void PreferencesDialogModel::setMoveShipsWhenUndocking(bool value) { modify(_moveShipsWhenUndocking, value); }

bool PreferencesDialogModel::getAlwaysSaveDisplayNames() const { return _alwaysSaveDisplayNames; }
void PreferencesDialogModel::setAlwaysSaveDisplayNames(bool value) { modify(_alwaysSaveDisplayNames, value); }

bool PreferencesDialogModel::getErrorCheckerChecksForPotentialIssues() const { return _errorCheckerChecksForPotentialIssues; }
void PreferencesDialogModel::setErrorCheckerChecksForPotentialIssues(bool value) { modify(_errorCheckerChecksForPotentialIssues, value); }

bool PreferencesDialogModel::getShowSexpHelpMissionEvents() const { return _showSexpHelpMissionEvents; }
void PreferencesDialogModel::setShowSexpHelpMissionEvents(bool value) { modify(_showSexpHelpMissionEvents, value); }
bool PreferencesDialogModel::getShowSexpHelpMissionGoals() const { return _showSexpHelpMissionGoals; }
void PreferencesDialogModel::setShowSexpHelpMissionGoals(bool value) { modify(_showSexpHelpMissionGoals, value); }
bool PreferencesDialogModel::getShowSexpHelpMissionCutscenes() const { return _showSexpHelpMissionCutscenes; }
void PreferencesDialogModel::setShowSexpHelpMissionCutscenes(bool value) { modify(_showSexpHelpMissionCutscenes, value); }
bool PreferencesDialogModel::getShowSexpHelpShipEditor() const { return _showSexpHelpShipEditor; }
void PreferencesDialogModel::setShowSexpHelpShipEditor(bool value) { modify(_showSexpHelpShipEditor, value); }
bool PreferencesDialogModel::getShowSexpHelpWingEditor() const { return _showSexpHelpWingEditor; }
void PreferencesDialogModel::setShowSexpHelpWingEditor(bool value) { modify(_showSexpHelpWingEditor, value); }

QKeySequence PreferencesDialogModel::getControlKey(ControlAction action) const {
	auto it = _controlKeys.find(action);
	Assertion(it != _controlKeys.end(), "Unknown control action!");
	return it->second;
}

void PreferencesDialogModel::setControlKey(ControlAction action, const QKeySequence& sequence) {
	auto it = _controlKeys.find(action);
	Assertion(it != _controlKeys.end(), "Unknown control action!");
	modify(it->second, sequence);
}

void PreferencesDialogModel::resetControlDefaults() {
	auto& bindings = ControlBindings::instance();
	for (const auto& def : bindings.definitions()) {
		modify(_controlKeys[def.action], def.defaultKey);
	}
}

int PreferencesDialogModel::getGridCenterX() const { return _gridCenterX; }
int PreferencesDialogModel::getGridCenterY() const { return _gridCenterY; }
int PreferencesDialogModel::getGridCenterZ() const { return _gridCenterZ; }
void PreferencesDialogModel::setGridCenterX(int value) { modify(_gridCenterX, value); }
void PreferencesDialogModel::setGridCenterY(int value) { modify(_gridCenterY, value); }
void PreferencesDialogModel::setGridCenterZ(int value) { modify(_gridCenterZ, value); }

GridPlane PreferencesDialogModel::getGridPlane() const { return _gridPlane; }
void PreferencesDialogModel::setGridPlane(GridPlane plane) { modify(_gridPlane, plane); }

void PreferencesDialogModel::resetGrid() {
	modify(_gridCenterX, 0);
	modify(_gridCenterY, 0);
	modify(_gridCenterZ, 0);
	modify(_gridPlane, GridPlane::XZ);
}

} // namespace fso::fred::dialogs
