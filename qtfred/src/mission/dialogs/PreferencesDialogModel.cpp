#include "PreferencesDialogModel.h"

#include "mission/EditorViewport.h"
#include "ui/Theme.h"
#include "mission/missiongrid.h"
#include "math/vecmat.h"

namespace fso::fred::dialogs {

PreferencesDialogModel::PreferencesDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
	, _offerAutosaveRecovery(viewport->Offer_autosave_recovery)
	, _autosaveIntervalSeconds(viewport->autosave_interval_seconds)
	, _sexpNumberEveryN(viewport->sexp_number_every_n)
	, _createBakOnSave(viewport->Create_bak_on_save)
	, _moveShipsWhenUndocking(viewport->Move_ships_when_undocking)
	, _alwaysSaveDisplayNames(viewport->Always_save_display_names)
	, _checkPotentialIssues(viewport->Error_checker_checks_potential_issues)
	, _applyAutoCorrections(viewport->Error_checker_apply_auto_corrections)
	, _showSexpHelpMissionEvents(viewport->Show_sexp_help_mission_events)
	, _showSexpHelpMissionGoals(viewport->Show_sexp_help_mission_goals)
	, _showSexpHelpMissionCutscenes(viewport->Show_sexp_help_mission_cutscenes)
	, _showSexpHelpShipEditor(viewport->Show_sexp_help_ship_editor)
	, _showSexpHelpWingEditor(viewport->Show_sexp_help_wing_editor)
	, _darkMode(viewport->Dark_mode)
	, _toolbarIconSize(viewport->toolbar_icon_size)
	, _outlineLod(viewport->view.Outline_lod)
	, _invertOrbitX(viewport->camera.getInvertOrbitX())
	, _invertOrbitY(viewport->camera.getInvertOrbitY())
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
	const bool darkModeChanged = (_viewport->Dark_mode != _darkMode);

	_viewport->Offer_autosave_recovery   = _offerAutosaveRecovery;
	_viewport->autosave_interval_seconds = _autosaveIntervalSeconds;
	_viewport->sexp_number_every_n       = _sexpNumberEveryN;
	_viewport->Create_bak_on_save        = _createBakOnSave;
	_viewport->Move_ships_when_undocking = _moveShipsWhenUndocking;
	_viewport->Always_save_display_names                = _alwaysSaveDisplayNames;
	_viewport->Error_checker_checks_potential_issues    = _checkPotentialIssues;
	_viewport->Error_checker_apply_auto_corrections     = _applyAutoCorrections;
	_viewport->Show_sexp_help_mission_events    = _showSexpHelpMissionEvents;
	_viewport->Show_sexp_help_mission_goals     = _showSexpHelpMissionGoals;
	_viewport->Show_sexp_help_mission_cutscenes = _showSexpHelpMissionCutscenes;
	_viewport->Show_sexp_help_ship_editor       = _showSexpHelpShipEditor;
	_viewport->Show_sexp_help_wing_editor       = _showSexpHelpWingEditor;
	_viewport->Dark_mode                        = _darkMode;
	_viewport->toolbar_icon_size                = _toolbarIconSize;
	_viewport->view.Outline_lod                 = _outlineLod;
	_viewport->camera.setInvertOrbitX(_invertOrbitX);
	_viewport->camera.setInvertOrbitY(_invertOrbitY);

	_viewport->saveSettings();
	if (darkModeChanged) {
		applyEditorTheme(_darkMode);
	}

	auto& bindings = ControlBindings::instance();
	for (const auto& entry : _controlKeys) {
		bindings.setKey(entry.first, entry.second);
	}
	bindings.save();

	// Only rebuild the grid when its settings actually changed. apply() runs on every
	// preference change, and an unnecessary rebuild has visible side effects (e.g. it would
	// re-derive the grid plane orientation each time).
	const auto& curUvec = _viewport->The_grid->gmatrix.vec.uvec;
	GridPlane curPlane;
	if (curUvec.xyz.y != 0.0f) {
		curPlane = GridPlane::XZ;
	} else if (curUvec.xyz.z != 0.0f) {
		curPlane = GridPlane::XY;
	} else {
		curPlane = GridPlane::YZ;
	}

	const bool gridChanged = _gridPlane != curPlane
		|| _gridCenterX != static_cast<int>(_viewport->The_grid->center.xyz.x)
		|| _gridCenterY != static_cast<int>(_viewport->The_grid->center.xyz.y)
		|| _gridCenterZ != static_cast<int>(_viewport->The_grid->center.xyz.z);

	if (gridChanged) {
		_viewport->The_grid->center.xyz.x = static_cast<float>(_gridCenterX);
		_viewport->The_grid->center.xyz.y = static_cast<float>(_gridCenterY);
		_viewport->The_grid->center.xyz.z = static_cast<float>(_gridCenterZ);

		switch (_gridPlane) {
		case GridPlane::XY:
			_viewport->The_grid->gmatrix.vec.fvec = vmd_x_vector;
			_viewport->The_grid->gmatrix.vec.rvec = vmd_y_vector;
			break;
		case GridPlane::XZ:
			// fvec/rvec must be ordered so that uvec = fvec x rvec points +Y, matching
			// create_default_grid(); the reverse (X, Z) yields a -Y normal that flips the
			// grid plane and turns the orbit camera upside down.
			_viewport->The_grid->gmatrix.vec.fvec = vmd_z_vector;
			_viewport->The_grid->gmatrix.vec.rvec = vmd_x_vector;
			break;
		case GridPlane::YZ:
			_viewport->The_grid->gmatrix.vec.fvec = vmd_y_vector;
			_viewport->The_grid->gmatrix.vec.rvec = vmd_z_vector;
			break;
		}

		modify_grid(_viewport->The_grid);
	}

	return true;
}

void PreferencesDialogModel::reject() {
	// Nothing to do — data sources are not modified until apply()
}

bool PreferencesDialogModel::getOfferAutosaveRecovery() const { return _offerAutosaveRecovery; }
void PreferencesDialogModel::setOfferAutosaveRecovery(bool value) { modify(_offerAutosaveRecovery, value); }

int  PreferencesDialogModel::getAutosaveIntervalSeconds() const { return _autosaveIntervalSeconds; }
void PreferencesDialogModel::setAutosaveIntervalSeconds(int value) { modify(_autosaveIntervalSeconds, value); }

int  PreferencesDialogModel::getSexpNumberEveryN() const { return _sexpNumberEveryN; }
void PreferencesDialogModel::setSexpNumberEveryN(int value) { modify(_sexpNumberEveryN, value); }

bool PreferencesDialogModel::getCreateBakOnSave() const { return _createBakOnSave; }
void PreferencesDialogModel::setCreateBakOnSave(bool value) { modify(_createBakOnSave, value); }

bool PreferencesDialogModel::getMoveShipsWhenUndocking() const { return _moveShipsWhenUndocking; }
void PreferencesDialogModel::setMoveShipsWhenUndocking(bool value) { modify(_moveShipsWhenUndocking, value); }

bool PreferencesDialogModel::getAlwaysSaveDisplayNames() const { return _alwaysSaveDisplayNames; }
void PreferencesDialogModel::setAlwaysSaveDisplayNames(bool value) { modify(_alwaysSaveDisplayNames, value); }

bool PreferencesDialogModel::getCheckPotentialIssues() const { return _checkPotentialIssues; }
void PreferencesDialogModel::setCheckPotentialIssues(bool value) { modify(_checkPotentialIssues, value); }

bool PreferencesDialogModel::getApplyAutoCorrections() const { return _applyAutoCorrections; }
void PreferencesDialogModel::setApplyAutoCorrections(bool value) { modify(_applyAutoCorrections, value); }

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

bool PreferencesDialogModel::getDarkMode() const { return _darkMode; }
void PreferencesDialogModel::setDarkMode(bool value) { modify(_darkMode, value); }

int  PreferencesDialogModel::getToolbarIconSize() const { return _toolbarIconSize; }
void PreferencesDialogModel::setToolbarIconSize(int size) { modify(_toolbarIconSize, size); }

int  PreferencesDialogModel::getOutlineLod() const { return _outlineLod; }
void PreferencesDialogModel::setOutlineLod(int value) { modify(_outlineLod, value); }

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

bool PreferencesDialogModel::getInvertOrbitX() const { return _invertOrbitX; }
void PreferencesDialogModel::setInvertOrbitX(bool value) { modify(_invertOrbitX, value); }

bool PreferencesDialogModel::getInvertOrbitY() const { return _invertOrbitY; }
void PreferencesDialogModel::setInvertOrbitY(bool value) { modify(_invertOrbitY, value); }

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
