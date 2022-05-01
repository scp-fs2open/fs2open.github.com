#pragma once

#include <QMainWindow>
#include <QAction>
#include <QtGui/QSurfaceFormat>
#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtGui/QSurface>

#include <mission/FredRenderer.h>
#include <mission/IDialogProvider.h>

#include <memory>
#include <ui/widgets/ColorComboBox.h>

namespace fso {
namespace fred {

class Editor;
class RenderWidget;

namespace Ui {
class FredView;
}

class FredView: public QMainWindow, public IDialogProvider {
 Q_OBJECT

 public:
	explicit FredView(QWidget* parent = nullptr);
	~FredView() override;
	void setEditor(Editor* editor, EditorViewport* viewport);

	void loadMissionFile(const QString& pathName);

	QSurface* getRenderSurface();
	RenderWidget* getRenderWidget();

	void showContextMenu(const QPoint& globalPos);

 public slots:
	void openLoadMissionDIalog();

	void newMission();

 private slots:
	 void on_actionSave_As_triggered(bool);
	 void on_actionSave_triggered(bool);
	void on_actionExit_triggered(bool);

	void on_actionConstrainX_triggered(bool enabled);
	void on_actionConstrainXY_triggered(bool enabled);
	void on_actionConstrainY_triggered(bool enabled);
	void on_actionConstrainZ_triggered(bool enabled);
	void on_actionConstrainXZ_triggered(bool enabled);
	void on_actionConstrainYZ_triggered(bool enabled);

	void on_actionSelect_triggered(bool enabled);
	void on_actionSelectMove_triggered(bool enabled);
	void on_actionSelectRotate_triggered(bool enabled);

	void on_actionHide_Marked_Objects_triggered(bool enabled);
	void on_actionShow_All_Hidden_Objects_triggered(bool enabled);

	void on_actionx1_triggered(bool enabled);
	void on_actionx2_triggered(bool enabled);
	void on_actionx3_triggered(bool enabled);
	void on_actionx5_triggered(bool enabled);
	void on_actionx8_triggered(bool enabled);
	void on_actionx10_triggered(bool enabled);
	void on_actionx50_triggered(bool enabled);
	void on_actionx100_triggered(bool enabled);

	void on_actionRotx1_triggered(bool enabled);
	void on_actionRotx5_triggered(bool enabled);
	void on_actionRotx12_triggered(bool enabled);
	void on_actionRotx25_triggered(bool enabled);
	void on_actionRotx50_triggered(bool enabled);

	void on_actionCamera_triggered(bool enabled);
	void on_actionCurrent_Ship_triggered(bool enabled);

	void on_actionEvents_triggered(bool);
	void on_actionAsteroid_Field_triggered(bool);
	void on_actionBriefing_triggered(bool);
	void on_actionMission_Specs_triggered(bool);
	void on_actionWaypoint_Paths_triggered(bool);
	void on_actionObjects_triggered(bool);
	void on_actionShips_triggered(bool);
	void on_actionCommand_Briefing_triggered(bool);
	void on_actionReinforcements_triggered(bool);

	void on_actionSelectionLock_triggered(bool enabled);

	void on_actionWingForm_triggered(bool enabled);
	void on_actionWingDisband_triggered(bool enabled);

	void on_actionZoomSelected_triggered(bool);
	void on_actionZoomExtents_triggered(bool);

	void on_actionSelectionList_triggered(bool);

	void on_actionOrbitSelected_triggered(bool enabled);

	void on_actionRotateLocal_triggered(bool enabled);

	void on_actionSave_Camera_Pos_triggered(bool);
	void on_actionRestore_Camera_Pos_triggered(bool);

	void on_actionTool_Bar_triggered(bool enabled);
	void on_actionStatus_Bar_triggered(bool enabled);

	void on_actionClone_Marked_Objects_triggered(bool);
	void on_actionDelete_triggered(bool);
	void on_actionDelete_Wing_triggered(bool);

	void on_actionControl_Object_triggered(bool);
	void on_actionLevel_Object_triggered(bool);
	void on_actionAlign_Object_triggered(bool);

	void on_actionNext_Subsystem_triggered(bool);
	void on_actionPrev_Subsystem_triggered(bool);
	void on_actionCancel_Subsystem_triggered(bool);

	void on_actionMove_Ships_When_Undocking_triggered(bool);

	void on_actionError_Checker_triggered(bool);

	void on_actionAbout_triggered(bool);
	void on_actionBackground_triggered(bool);
	void on_actionShield_System_triggered(bool);
	void on_actionVoice_Acting_Manager_triggered(bool);
	void on_actionFiction_Viewer_triggered(bool);
	void on_actionMission_Objectives_triggered(bool);
 signals:
	/**
	 * @brief Special version of FredApplication::onIdle which is limited to the lifetime of this object
	 */
	void viewIdle();

	/**
	 * @brief This is emitted when the view window is activated after being deactivated
	 */
	void viewWindowActivated();
 protected:
	bool event(QEvent* event) override;

	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

	void mouseDoubleClickEvent(QMouseEvent* event) override;

 private:
	void on_mission_loaded(const std::string& filepath);

	void connectActionToViewSetting(QAction* option, bool* destination);
	void connectActionToViewSetting(QAction* option, std::vector<bool>* vector, size_t idx);

	void on_actionControlModeCamera_triggered(bool enabled);
	void on_actionControlModeCurrentShip_triggered(bool enabled);

	void addToRecentFiles(const QString& path);
	void updateRecentFileList();

	void recentFileOpened();

	/**
	 * @brief Synchronize the view options in the renderer and the state of the view check boxes in the menu
	 */
	void syncViewOptions();
	void updateUI();

	void initializeStatusBar();
	void initializePopupMenus();

	void onGroupSelected(int group);
	void onSetGroup(int group);

	QLabel* _statusBarViewmode = nullptr;
	QLabel* _statusBarUnitsLabel = nullptr;

	QMenu* _viewPopup = nullptr;

	QMenu* _editPopup = nullptr;
	QAction* _editObjectAction = nullptr;
	QAction* _editOrientPositionAction = nullptr;
	QAction* _editWingAction = nullptr;

	QMenu* _controlModeMenu = nullptr;
	QAction* _controlModeCamera = nullptr;
	QAction* _controlModeCurrentShip = nullptr;

	QString saveName = nullptr;

	std::unique_ptr<Ui::FredView> ui;

	std::unique_ptr<ColorComboBox> _shipClassBox;

	Editor* fred = nullptr;
	EditorViewport* _viewport = nullptr;

	bool _inKeyPressHandler = false;
	bool _inKeyReleaseHandler = false;

	void onUpdateConstrains();
	void onUpdateEditingMode();
	void onUpdateViewSpeeds();
	void onUpdateCameraControlActions();
	void onUpdateSelectionLock();
	void onUpdateShipClassBox();
	void onUpdateEditorActions();
	void onUpdateWingActionStatus();

	void onShipClassSelected(int ship_class);

	void windowActivated();
	void windowDeactivated();

	void editObjectTriggered();
	void orientEditorTriggered();

	void handleObjectEditor(int objNum);

 public:
	DialogButton showButtonDialog(DialogType type,
								  const SCP_string& title,
								  const SCP_string& message,
								  const flagset<DialogButton>& buttons) override;

	std::unique_ptr<IDialog<dialogs::FormWingDialogModel>> createFormWingDialog() override;

	bool showModalDialog(IBaseDialog* dlg) override;
	void initializeGroupActions();
};

} // namespace fred
} // namespace fso
