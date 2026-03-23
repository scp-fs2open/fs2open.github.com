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
	bool getMoveShipsWhenUndocking() const;
	void setMoveShipsWhenUndocking(bool value);

	bool getAlwaysSaveDisplayNames() const;
	void setAlwaysSaveDisplayNames(bool value);

	bool getErrorCheckerChecksForPotentialIssues() const;
	void setErrorCheckerChecksForPotentialIssues(bool value);

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

	// Controls
	QKeySequence getControlKey(ControlAction action) const;
	void setControlKey(ControlAction action, const QKeySequence& sequence);
	void resetControlDefaults();

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
	bool _moveShipsWhenUndocking;
	bool _alwaysSaveDisplayNames;
	bool _errorCheckerChecksForPotentialIssues;
	bool _showSexpHelpMissionEvents;
	bool _showSexpHelpMissionGoals;
	bool _showSexpHelpMissionCutscenes;
	bool _showSexpHelpShipEditor;
	bool _showSexpHelpWingEditor;

	// Controls
	std::map<ControlAction, QKeySequence> _controlKeys;

	// Grid
	int _gridCenterX;
	int _gridCenterY;
	int _gridCenterZ;
	GridPlane _gridPlane;
};

} // namespace fso::fred::dialogs
