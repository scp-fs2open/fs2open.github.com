#pragma once

#include "mission/dialogs/AbstractDialogModel.h"
#include "ui/ControlBindings.h"

namespace fso::fred::dialogs {

enum class GridPlane { XY, XZ, YZ };

class PreferencesDialogModel : public AbstractDialogModel {
	Q_OBJECT

public:
	PreferencesDialogModel(QObject* parent, EditorViewport* viewport);
	~PreferencesDialogModel() override = default;

	bool apply() override;
	void reject() override;

	// General
	bool getOfferAutosaveRecovery() const;
	void setOfferAutosaveRecovery(bool value);

	int  getAutosaveIntervalSeconds() const;
	void setAutosaveIntervalSeconds(int value);

	int  getSexpNumberEveryN() const;
	void setSexpNumberEveryN(int value);

	bool getCreateBakOnSave() const;
	void setCreateBakOnSave(bool value);

	bool getMoveShipsWhenUndocking() const;
	void setMoveShipsWhenUndocking(bool value);

	bool getAlwaysSaveDisplayNames() const;
	void setAlwaysSaveDisplayNames(bool value);

	// Error Checker
	bool getCheckPotentialIssues() const;
	void setCheckPotentialIssues(bool value);

	bool getApplyAutoCorrections() const;
	void setApplyAutoCorrections(bool value);

	bool getShowSexpHelpMissionEvents() const;
	void setShowSexpHelpMissionEvents(bool value);
	bool getShowSexpHelpMissionGoals() const;
	void setShowSexpHelpMissionGoals(bool value);
	bool getShowSexpHelpMissionCutscenes() const;
	void setShowSexpHelpMissionCutscenes(bool value);
	bool getShowSexpHelpShipEditor() const;
	void setShowSexpHelpShipEditor(bool value);
	bool getShowSexpHelpWingEditor() const;
	void setShowSexpHelpWingEditor(bool value);

	bool getDarkMode() const;
	void setDarkMode(bool value);

	int  getToolbarIconSize() const;
	void setToolbarIconSize(int size);

	int  getOutlineLod() const;
	void setOutlineLod(int value);

	// Controls
	QKeySequence getControlKey(ControlAction action) const;
	void setControlKey(ControlAction action, const QKeySequence& sequence);
	void resetControlDefaults();

	bool getInvertOrbitX() const;
	void setInvertOrbitX(bool value);
	bool getInvertOrbitY() const;
	void setInvertOrbitY(bool value);

	// Grid
	int getGridCenterX() const;
	int getGridCenterY() const;
	int getGridCenterZ() const;
	void setGridCenterX(int value);
	void setGridCenterY(int value);
	void setGridCenterZ(int value);

	GridPlane getGridPlane() const;
	void setGridPlane(GridPlane plane);
	void resetGrid();

private:
	// General
	bool _offerAutosaveRecovery;
	int  _autosaveIntervalSeconds;
	int  _sexpNumberEveryN;
	bool _createBakOnSave;
	bool _moveShipsWhenUndocking;
	bool _alwaysSaveDisplayNames;
	bool _checkPotentialIssues;
	bool _applyAutoCorrections;
	bool _showSexpHelpMissionEvents;
	bool _showSexpHelpMissionGoals;
	bool _showSexpHelpMissionCutscenes;
	bool _showSexpHelpShipEditor;
	bool _showSexpHelpWingEditor;
	bool _darkMode;
	int  _toolbarIconSize;
	int  _outlineLod;

	// Controls
	std::map<ControlAction, QKeySequence> _controlKeys;
	bool _invertOrbitX;
	bool _invertOrbitY;

	// Grid
	int _gridCenterX;
	int _gridCenterY;
	int _gridCenterZ;
	GridPlane _gridPlane;
};

} // namespace fso::fred::dialogs
