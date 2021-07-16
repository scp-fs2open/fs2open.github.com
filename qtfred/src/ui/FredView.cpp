#include "FredView.h"
#include "ui_FredView.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QSettings>

#include <project.h>
#include <io/key.h>

#include <qevent.h>
#include <FredApplication.h>
#include <ui/dialogs/ShipEditorDialog.h>
#include <ui/dialogs/EventEditorDialog.h>
#include <ui/dialogs/AsteroidEditorDialog.h>
#include <ui/dialogs/BriefingEditorDialog.h>
#include <ui/dialogs/WaypointEditorDialog.h>
#include <ui/dialogs/MissionGoalsDialog.h>
#include <ui/dialogs/ObjectOrientEditorDialog.h>
#include <ui/dialogs/MissionSpecDialog.h>
#include <ui/dialogs/FormWingDialog.h>
#include <ui/dialogs/AboutDialog.h>
#include <ui/dialogs/BackgroundEditorDialog.h>
#include <ui/dialogs/ShieldSystemDialog.h>
#include <ui/dialogs/VoiceActingManager.h>
#include <globalincs/linklist.h>
#include <ui/dialogs/SelectionDialog.h>
#include <ui/dialogs/FictionViewerDialog.h>
#include <iff_defs/iff_defs.h>

#include "mission/Editor.h"
#include "mission/management.h"
#include "mission/missionsave.h"

#include "widgets/ColorComboBox.h"

#include "util.h"
#include "mission/object.h"

namespace {

template<typename T>
void copyActionSettings(QAction* action, T* target) {
	Q_ASSERT(action->isCheckable());

	// Double negate so that integers get promoted to a "true" boolean
	action->setChecked(!!(*target));
}

}

namespace fso {
namespace fred {

FredView::FredView(QWidget* parent) : QMainWindow(parent), ui(new Ui::FredView()) {
	ui->setupUi(this);

	setFocusPolicy(Qt::NoFocus);
	setFocusProxy(ui->centralWidget);

	// This is not possible to do with the designer
	ui->actionNew->setShortcuts(QKeySequence::New);
	ui->actionOpen->setShortcuts(QKeySequence::Open);
	ui->actionSave->setShortcuts(QKeySequence::Save);
	ui->actionExit->setShortcuts(QKeySequence::Quit);
	ui->actionUndo->setShortcuts(QKeySequence::Undo);
	ui->actionDelete->setShortcuts(QKeySequence::Delete);

	connect(ui->actionOpen, &QAction::triggered, this, &FredView::openLoadMissionDIalog);
	connect(ui->actionNew, &QAction::triggered, this, &FredView::newMission);

	connect(fredApp, &FredApplication::onIdle, this, &FredView::updateUI);

	// TODO: Hook this up with the modified state of the mission
	setWindowModified(false);

	updateRecentFileList();

	initializeStatusBar();
	initializePopupMenus();

	initializeGroupActions();
}

FredView::~FredView() {
}

void FredView::setEditor(Editor* editor, EditorViewport* viewport) {
	Assertion(fred == nullptr, "Resetting the editor is currently not supported!");
	Assertion(_viewport == nullptr, "Resetting the viewport is currently not supported!");

	fred = editor;
	_viewport = viewport;

	// Let the viewport use us for displaying dialogs
	_viewport->dialogProvider = this;

	ui->centralWidget->setEditor(editor, _viewport);

	// A combo box cannot be added by the designer so we do that manually here
	// This needs to be done since the viewport pointer is not valid earlier
	_shipClassBox.reset(new ColorComboBox(nullptr, _viewport));
	ui->toolBar->addWidget(_shipClassBox.get());
	connect(_shipClassBox.get(), &ColorComboBox::shipClassSelected, this, &FredView::onShipClassSelected);

	connect(fred, &Editor::missionLoaded, this, &FredView::on_mission_loaded);

	// Sets the initial window title
	on_mission_loaded("");

	syncViewOptions();

	connect(this, &FredView::viewIdle, this, &FredView::onUpdateConstrains);
	connect(this, &FredView::viewIdle, this, &FredView::onUpdateEditingMode);
	connect(this, &FredView::viewIdle, this, &FredView::onUpdateViewSpeeds);
	connect(this, &FredView::viewIdle, this, &FredView::onUpdateCameraControlActions);
	connect(this, &FredView::viewIdle, this, &FredView::onUpdateSelectionLock);
	connect(this, &FredView::viewIdle, this, &FredView::onUpdateShipClassBox);
	connect(this, &FredView::viewIdle, this, &FredView::onUpdateEditorActions);
	connect(this, &FredView::viewIdle, this, &FredView::onUpdateWingActionStatus);
	connect(this,
			&FredView::viewIdle,
			this,
			[this]() { ui->actionZoomSelected->setEnabled(query_valid_object(fred->currentObject)); });
	connect(this, &FredView::viewIdle, this, [this]() { ui->actionOrbitSelected->setChecked(_viewport->Lookat_mode); });
	connect(this, &FredView::viewIdle, this, [this]() { ui->actionRotateLocal->setChecked(_viewport->Group_rotate); });
	connect(this,
			&FredView::viewIdle,
			this,
			[this]() { ui->actionRestore_Camera_Pos->setEnabled(!IS_VEC_NULL(&_viewport->saved_cam_orient.vec.fvec)); });

	// The Show teams actions need to be initialized after everything has been set up since the IFFs may not have been
	// initialized yet
	fredApp->runAfterInit([this]() {
		for (auto i = 0; i < Num_iffs; ++i) {
			auto action = new QAction(QString::fromUtf8(Iff_info[i].iff_name), ui->menuDisplay_Filter);
			action->setCheckable(true);
			connectActionToViewSetting(action, &_viewport->view.Show_iff[i]);

			ui->menuDisplay_Filter->addAction(action);
		}
	});
}

void FredView::loadMissionFile(const QString& pathName) {
	statusBar()->showMessage(tr("Loading mission %1").arg(pathName));
	try {
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

		fred->loadMission(pathName.toStdString());

		QApplication::restoreOverrideCursor();
	} catch (const fso::fred::mission_load_error&) {
		QApplication::restoreOverrideCursor();

		QMessageBox::critical(this, tr("Failed loading mission."), tr("Could not parse the mission."));
		statusBar()->clearMessage();
	}
}

void FredView::openLoadMissionDIalog() {
	qDebug() << "Loading from directory:" << QDir::currentPath();
	QString pathName = QFileDialog::getOpenFileName(this, tr("Load mission"), QString(), tr("FS2 missions (*.fs2)"));

	if (pathName.isEmpty()) {
		return;
	}

	loadMissionFile(pathName.replace('/',DIR_SEPARATOR_CHAR));
}

void FredView::on_actionExit_triggered(bool) {
	close();
}
void FredView::on_actionSave_As_triggered(bool) {
	CFred_mission_save save(_viewport);

	QString saveName = QFileDialog::getSaveFileName(this, tr("Save mission"), QString(), tr("FS2 missions (*.fs2)"));

	if (saveName.isEmpty()) {
		return;
	}

	save.save_mission_file(saveName.replace('/',DIR_SEPARATOR_CHAR).toUtf8().constData());
}

void FredView::on_mission_loaded(const std::string& filepath) {
	QString filename = "Untitled";
	if (!filepath.empty()) {
		filename = QFileInfo(QString::fromStdString(filepath)).fileName();
	}

	// The "[*]" is the placeholder for showing the modified state of the window
	auto title = tr("%1[*]").arg(filename);

	setWindowTitle(title);
	// This will add some additional features on platforms that make use of this information
	setWindowFilePath(QString::fromStdString(filepath));

	if (!filepath.empty()) {
		addToRecentFiles(QString::fromStdString(filepath));
	}
}

QSurface* FredView::getRenderSurface() {
	return ui->centralWidget->getRenderSurface();
}
void FredView::newMission() {
	fred->createNewMission();
}
void FredView::addToRecentFiles(const QString& path) {
	// First get the list of existing files
	QSettings settings;
	auto recentFiles = settings.value("FredView/recentFiles").toStringList();

	if (recentFiles.contains(path)) {
		// If this file is already here then remove it since we don't want duplicate entries
		recentFiles.removeAll(path);
	}
	// Add the path to the start
	recentFiles.prepend(path);

	// Only keep the last 8 entries
	while (recentFiles.size() > 8) {
		recentFiles.removeLast();
	}

	settings.setValue("FredView/recentFiles", recentFiles);
	updateRecentFileList();
}

void FredView::updateRecentFileList() {
	QSettings settings;
	auto recentFiles = settings.value("FredView/recentFiles").toStringList();

	if (recentFiles.empty()) {
		// If there are no files, clear the menu and disable it
		ui->menuRe_cent_Files->clear();
		ui->menuRe_cent_Files->setEnabled(false);
	} else {
		// Reset the menu in case there was something there before and enable it
		ui->menuRe_cent_Files->clear();
		ui->menuRe_cent_Files->setEnabled(true);

		// Now add the individual files as actions
		for (auto& path : recentFiles) {
			auto action = new QAction(path, this);
			connect(action, &QAction::triggered, this, &FredView::recentFileOpened);

			ui->menuRe_cent_Files->addAction(action);
		}
	}
}

void FredView::recentFileOpened() {
	auto sender = qobject_cast<QAction*>(QObject::sender());

	Q_ASSERT(sender != nullptr);

	auto path = sender->text();
	loadMissionFile(path);
}
void FredView::syncViewOptions() {
	connectActionToViewSetting(ui->actionShow_Ships, &_viewport->view.Show_ships);
	connectActionToViewSetting(ui->actionShow_Player_Starts, &_viewport->view.Show_starts);
	connectActionToViewSetting(ui->actionShow_Waypoints, &_viewport->view.Show_waypoints);

	// TODO: Dynamically handle the Show teams option

	connectActionToViewSetting(ui->actionShow_Ship_Models, &_viewport->view.Show_ship_models);
	connectActionToViewSetting(ui->actionShow_Outlines, &_viewport->view.Show_outlines);
	connectActionToViewSetting(ui->actionShow_Ship_Info, &_viewport->view.Show_ship_info);
	connectActionToViewSetting(ui->actionShow_Coordinates, &_viewport->view.Show_coordinates);
	connectActionToViewSetting(ui->actionShow_Grid_Positions, &_viewport->view.Show_grid_positions);
	connectActionToViewSetting(ui->actionShow_Distances, &_viewport->view.Show_distances);
	connectActionToViewSetting(ui->actionShow_Model_Paths, &_viewport->view.Show_paths_fred);
	connectActionToViewSetting(ui->actionShow_Model_Dock_Points, &_viewport->view.Show_dock_points);
	connectActionToViewSetting(ui->actionHighlight_Selectable_Subsystems, &_viewport->view.Highlight_selectable_subsys);

	connectActionToViewSetting(ui->actionShow_Grid, &_viewport->view.Show_grid);
	connectActionToViewSetting(ui->actionShow_Horizon, &_viewport->view.Show_horizon);
	connectActionToViewSetting(ui->actionDouble_Fine_Gridlines, &double_fine_gridlines);
	connectActionToViewSetting(ui->actionAnti_Aliased_Gridlines, &_viewport->view.Aa_gridlines);
	connectActionToViewSetting(ui->actionShow_3D_Compass, &_viewport->view.Show_compass);
	connectActionToViewSetting(ui->actionShow_Background, &_viewport->view.Show_stars);

	connectActionToViewSetting(ui->actionLighting_from_Suns, &_viewport->view.Lighting_on);


	connectActionToViewSetting(ui->actionShowDistances, &_viewport->view.Show_distances);
}
void FredView::initializeStatusBar() {
	_statusBarViewmode = new QLabel();
	statusBar()->addPermanentWidget(_statusBarViewmode);

	_statusBarUnitsLabel = new QLabel();
	statusBar()->addPermanentWidget(_statusBarUnitsLabel);
}
void FredView::updateUI() {
	if (!_viewport) {
		// The following code requires a valid viewport
		return;
	}

	_statusBarUnitsLabel->setText(tr("Units = %1 Meters").arg(_viewport->The_grid->square_size));

	if (_viewport->viewpoint == 1) {
		_statusBarViewmode->setText(tr("Viewpoint: %1").arg(object_name(_viewport->view_obj)));
	} else {
		_statusBarViewmode->setText(tr("Viewpoint: Camera"));
	}

	viewIdle();
}
void FredView::connectActionToViewSetting(QAction* option, bool* destination) {
	Q_ASSERT(option->isCheckable());

	// Use our view idle function for updating the action status whenever possible
	// TODO: Maybe this could be improved with an event based property system but that would need to be implemented
	connect(this, &FredView::viewIdle, this, [option, destination]() {
		option->setChecked(*destination);
	});

	// then connect the signal to a handler for updating the view setting
	// The pointer should be valid as long as this signal is active since it should be pointing inside the renderer (I hope...)
	connect(option, &QAction::triggered, this, [this, destination](bool value) {
		*destination = value;

		// View settings have changed so we need to update the window
		_viewport->needsUpdate();
	});
}

void FredView::showContextMenu(const QPoint& globalPos) {
	auto localPos = ui->centralWidget->mapFromGlobal(globalPos);

	auto obj = _viewport->select_object(localPos.x(), localPos.y());
	if (obj >= 0) {
		fred->selectObject(obj);

		// There is an object under the cursor
		SCP_string objName;
		if (fred->getNumMarked() > 1) {
			objName = "Marked Ships";
		} else {
			objName = object_name(obj);
		}

		_editObjectAction->setText(tr("Edit %1").arg(objName.c_str()));

		_editPopup->exec(globalPos);
	} else {
		// Nothing is here...
		_viewPopup->exec(globalPos);
	}
}
void FredView::initializePopupMenus() {
	_viewPopup = new QMenu(this);

	_viewPopup->addAction(ui->actionShow_Ship_Models);
	_viewPopup->addAction(ui->actionShow_Outlines);
	_viewPopup->addAction(ui->actionShow_Ship_Info);
	_viewPopup->addAction(ui->actionShow_Coordinates);
	_viewPopup->addAction(ui->actionShow_Grid_Positions);
	_viewPopup->addAction(ui->actionShow_Distances);
	_viewPopup->addSeparator();

	_controlModeMenu = new QMenu(tr("Control Mode"), _viewPopup);
	_controlModeCamera = new QAction(tr("Camera"), _controlModeMenu);
	_controlModeCamera->setCheckable(true);
	connect(_controlModeCamera, &QAction::triggered, this, &FredView::on_actionControlModeCamera_triggered);
	_controlModeMenu->addAction(_controlModeCamera);

	_controlModeCurrentShip = new QAction(tr("Current Ship"), _controlModeMenu);
	_controlModeCurrentShip->setCheckable(true);
	connect(_controlModeCurrentShip, &QAction::triggered, this, &FredView::on_actionControlModeCurrentShip_triggered);
	_controlModeMenu->addAction(_controlModeCurrentShip);

	_viewPopup->addMenu(_controlModeMenu);
	_viewPopup->addMenu(ui->menuViewpoint);
	_viewPopup->addSeparator();

	// Begin construction edit popup
	_editPopup = new QMenu(this);

	_editObjectAction = new QAction(tr("Edit !Object!"), _editPopup);
	connect(_editObjectAction, &QAction::triggered, this, &FredView::editObjectTriggered);
	_editPopup->addAction(_editObjectAction);

	_editOrientPositionAction = new QAction(tr("Edit Position and Orientation"), _editPopup);
	connect(_editOrientPositionAction, &QAction::triggered, this, &FredView::orientEditorTriggered);
	_editPopup->addAction(_editOrientPositionAction);

	_editWingAction = new QAction(tr("Edit Wing"), _editPopup);
	_editPopup->addAction(_editWingAction);
}

void FredView::onUpdateConstrains() {
	ui->actionConstrainX->setChecked(
		_viewport->Constraint.xyz.x && !_viewport->Constraint.xyz.y && !_viewport->Constraint.xyz.z);
	ui->actionConstrainY->setChecked(
		!_viewport->Constraint.xyz.x && _viewport->Constraint.xyz.y && !_viewport->Constraint.xyz.z);
	ui->actionConstrainZ->setChecked(
		!_viewport->Constraint.xyz.x && !_viewport->Constraint.xyz.y && _viewport->Constraint.xyz.z);
	ui->actionConstrainXZ->setChecked(
		_viewport->Constraint.xyz.x && !_viewport->Constraint.xyz.y && _viewport->Constraint.xyz.z);
	ui->actionConstrainXY->setChecked(
		_viewport->Constraint.xyz.x && _viewport->Constraint.xyz.y && !_viewport->Constraint.xyz.z);
	ui->actionConstrainYZ->setChecked(
		!_viewport->Constraint.xyz.x && _viewport->Constraint.xyz.y && _viewport->Constraint.xyz.z);
}
void FredView::on_actionConstrainX_triggered(bool enabled) {
	if (enabled) {
		vm_vec_make(&_viewport->Constraint, 1.0f, 0.0f, 0.0f);
		vm_vec_make(&_viewport->Anticonstraint, 0.0f, 1.0f, 1.0f);
		_viewport->Single_axis_constraint = true;
	}
}
void FredView::on_actionConstrainY_triggered(bool enabled) {
	if (enabled) {
		vm_vec_make(&_viewport->Constraint, 0.0f, 1.0f, 0.0f);
		vm_vec_make(&_viewport->Anticonstraint, 1.0f, 0.0f, 1.0f);
		_viewport->Single_axis_constraint = true;
	}
}
void FredView::on_actionConstrainZ_triggered(bool enabled) {
	if (enabled) {
		vm_vec_make(&_viewport->Constraint, 0.0f, 0.0f, 1.0f);
		vm_vec_make(&_viewport->Anticonstraint, 1.0f, 1.0f, 0.0f);
		_viewport->Single_axis_constraint = true;
	}
}
void FredView::on_actionConstrainXZ_triggered(bool enabled) {
	if (enabled) {
		vm_vec_make(&_viewport->Constraint, 1.0f, 0.0f, 1.0f);
		vm_vec_make(&_viewport->Anticonstraint, 0.0f, 1.0f, 0.0f);
		_viewport->Single_axis_constraint = false;
	}
}
void FredView::on_actionConstrainXY_triggered(bool enabled) {
	if (enabled) {
		vm_vec_make(&_viewport->Constraint, 1.0f, 1.0f, 0.0f);
		vm_vec_make(&_viewport->Anticonstraint, 0.0f, 0.0f, 1.0f);
		_viewport->Single_axis_constraint = false;
	}
}
void FredView::on_actionConstrainYZ_triggered(bool enabled) {
	if (enabled) {
		vm_vec_make(&_viewport->Constraint, 0.0f, 1.0f, 1.0f);
		vm_vec_make(&_viewport->Anticonstraint, 1.0f, 0.0f, 0.0f);
		_viewport->Single_axis_constraint = false;
	}
}
RenderWidget* FredView::getRenderWidget() {
	return ui->centralWidget;
}
void FredView::on_actionSelect_triggered(bool enabled) {
	if (enabled) {
		_viewport->Editing_mode = CursorMode::Selecting;
	}
}
void FredView::on_actionSelectMove_triggered(bool enabled) {
	if (enabled) {
		_viewport->Editing_mode = CursorMode::Moving;
	}
}
void FredView::on_actionSelectRotate_triggered(bool enabled) {
	if (enabled) {
		_viewport->Editing_mode = CursorMode::Rotating;
	}
}
void FredView::onUpdateEditingMode() {
	ui->actionSelect->setChecked(_viewport->Editing_mode == CursorMode::Selecting);
	ui->actionSelectMove->setChecked(_viewport->Editing_mode == CursorMode::Moving);
	ui->actionSelectRotate->setChecked(_viewport->Editing_mode == CursorMode::Rotating);

	ui->centralWidget->setCursorMode(_viewport->Editing_mode);
}
bool FredView::event(QEvent* event) {
	switch (event->type()) {
	case QEvent::WindowActivate:
		windowActivated();
		return true;
	case QEvent::WindowDeactivate:
		windowDeactivated();
		return true;
	default:
		return QMainWindow::event(event);
	}
}
void FredView::windowActivated() {
	key_got_focus();

	_viewport->Cursor_over = -1;

	// Track the last active viewport
	fred->setActiveViewport(_viewport);

	viewWindowActivated();
}
void FredView::windowDeactivated() {
	key_lost_focus();

	_viewport->Cursor_over = -1;
}
void FredView::on_actionHide_Marked_Objects_triggered(bool  /*enabled*/) {
	fred->hideMarkedObjects();
}
void FredView::on_actionShow_All_Hidden_Objects_triggered(bool  /*enabled*/) {
	fred->showHiddenObjects();
}
void FredView::onUpdateViewSpeeds() {
	ui->actionx1->setChecked(_viewport->physics_speed == 1);
	ui->actionx2->setChecked(_viewport->physics_speed == 2);
	ui->actionx3->setChecked(_viewport->physics_speed == 3);
	ui->actionx5->setChecked(_viewport->physics_speed == 5);
	ui->actionx8->setChecked(_viewport->physics_speed == 8);
	ui->actionx10->setChecked(_viewport->physics_speed == 10);
	ui->actionx50->setChecked(_viewport->physics_speed == 50);
	ui->actionx100->setChecked(_viewport->physics_speed == 100);

	ui->actionRotx1->setChecked(_viewport->physics_rot == 2);
	ui->actionRotx5->setChecked(_viewport->physics_rot == 10);
	ui->actionRotx12->setChecked(_viewport->physics_rot == 25);
	ui->actionRotx25->setChecked(_viewport->physics_rot == 50);
	ui->actionRotx50->setChecked(_viewport->physics_rot == 100);
}
void FredView::on_actionx1_triggered(bool enabled) {
	if (enabled) {
		_viewport->physics_speed = 1;
		_viewport->resetViewPhysics();
	}
}
void FredView::on_actionx2_triggered(bool enabled) {
	if (enabled) {
		_viewport->physics_speed = 2;
		_viewport->resetViewPhysics();
	}
}
void FredView::on_actionx3_triggered(bool enabled) {
	if (enabled) {
		_viewport->physics_speed = 3;
		_viewport->resetViewPhysics();
	}
}
void FredView::on_actionx5_triggered(bool enabled) {
	if (enabled) {
		_viewport->physics_speed = 5;
		_viewport->resetViewPhysics();
	}
}
void FredView::on_actionx8_triggered(bool enabled) {
	if (enabled) {
		_viewport->physics_speed = 8;
		_viewport->resetViewPhysics();
	}
}
void FredView::on_actionx10_triggered(bool enabled) {
	if (enabled) {
		_viewport->physics_speed = 10;
		_viewport->resetViewPhysics();
	}
}
void FredView::on_actionx50_triggered(bool enabled) {
	if (enabled) {
		_viewport->physics_speed = 50;
		_viewport->resetViewPhysics();
	}
}
void FredView::on_actionx100_triggered(bool enabled) {
	if (enabled) {
		_viewport->physics_speed = 100;
		_viewport->resetViewPhysics();
	}
}
void FredView::on_actionRotx1_triggered(bool enabled) {
	if (enabled) {
		_viewport->physics_rot = 2;
		_viewport->resetViewPhysics();
	}
}
void FredView::on_actionRotx5_triggered(bool enabled) {
	if (enabled) {
		_viewport->physics_rot = 10;
		_viewport->resetViewPhysics();
	}
}
void FredView::on_actionRotx12_triggered(bool enabled) {
	if (enabled) {
		_viewport->physics_rot = 25;
		_viewport->resetViewPhysics();
	}
}
void FredView::on_actionRotx25_triggered(bool enabled) {
	if (enabled) {
		_viewport->physics_rot = 50;
		_viewport->resetViewPhysics();
	}
}
void FredView::on_actionRotx50_triggered(bool enabled) {
	if (enabled) {
		_viewport->physics_rot = 100;
		_viewport->resetViewPhysics();
	}
}
void FredView::onUpdateCameraControlActions() {
	ui->actionCamera->setChecked(_viewport->viewpoint == 0);
	ui->actionCurrent_Ship->setChecked(_viewport->viewpoint == 1);

	_controlModeCamera->setChecked(_viewport->Control_mode == 0);
	_controlModeCurrentShip->setChecked(_viewport->Control_mode == 1);
}
void FredView::on_actionCamera_triggered(bool enabled) {
	if (enabled) {
		_viewport->viewpoint = 0;

		_viewport->needsUpdate();
	}
}
void FredView::on_actionCurrent_Ship_triggered(bool enabled) {
	if (enabled) {
		_viewport->viewpoint = 1;
		_viewport->view_obj = fred->currentObject;

		_viewport->needsUpdate();
	}
}
void FredView::on_actionControlModeCamera_triggered(bool enabled) {
	if (enabled) {
		_viewport->Control_mode = 0;
	}
}
void FredView::on_actionControlModeCurrentShip_triggered(bool enabled) {
	if (enabled) {
		_viewport->Control_mode = 1;
	}
}

void FredView::keyPressEvent(QKeyEvent* event) {
	if (_inKeyPressHandler) {
		return;
	}
	_inKeyPressHandler = true;

	qGuiApp->sendEvent(ui->centralWidget, event);

	_inKeyPressHandler = false;
}
void FredView::keyReleaseEvent(QKeyEvent* event) {
	if (_inKeyReleaseHandler) {
		return;
	}
	_inKeyReleaseHandler = true;

	qGuiApp->sendEvent(ui->centralWidget, event);

	_inKeyReleaseHandler = false;
}
void FredView::on_actionEvents_triggered(bool) {
	auto eventEditor = new dialogs::EventEditorDialog(this, _viewport);
	eventEditor->show();
}
void FredView::on_actionSelectionLock_triggered(bool enabled) {
	_viewport->Selection_lock = enabled;
}
void FredView::onUpdateSelectionLock() {
	ui->actionSelectionLock->setChecked(_viewport->Selection_lock);
}
void FredView::onUpdateShipClassBox() {
	_shipClassBox->selectShipClass(_viewport->cur_model_index);
}
void FredView::onShipClassSelected(int ship_class) {
	_viewport->cur_model_index = ship_class;
}
void FredView::on_actionAsteroid_Field_triggered(bool) {
	auto asteroidFieldEditor = new dialogs::AsteroidEditorDialog(this, _viewport);
	asteroidFieldEditor->show();
}
void FredView::on_actionBriefing_triggered(bool) {
	auto eventEditor = new dialogs::BriefingEditorDialog(this);
	eventEditor->show();
}
void FredView::on_actionMission_Specs_triggered(bool) {
	auto missionSpecEditor = new dialogs::MissionSpecDialog(this, _viewport);
	missionSpecEditor->show();
}
void FredView::on_actionWaypoint_Paths_triggered(bool) {
	auto editorDialog = new dialogs::WaypointEditorDialog(this, _viewport);
	editorDialog->show();
}
void FredView::on_actionShips_triggered(bool)
{
	auto editorDialog = new dialogs::ShipEditorDialog(this, _viewport);
	editorDialog->show();

}
void FredView::on_actionObjects_triggered(bool) {
	orientEditorTriggered();
}
DialogButton FredView::showButtonDialog(DialogType type,
										const SCP_string& title,
										const SCP_string& message,
										const flagset<DialogButton>& buttons) {
	QMessageBox dialog(this);

	dialog.setWindowTitle(QString::fromStdString(title));
	dialog.setText(QString::fromStdString(message));

	QMessageBox::StandardButtons qtButtons{};
	QMessageBox::StandardButton defaultButton = QMessageBox::NoButton;
	if (buttons[DialogButton::Yes]) {
		qtButtons |= QMessageBox::Yes;
		defaultButton = QMessageBox::Yes;
	}
	if (buttons[DialogButton::No]) {
		qtButtons |= QMessageBox::No;
		defaultButton = QMessageBox::No;
	}
	if (buttons[DialogButton::Cancel]) {
		qtButtons |= QMessageBox::Cancel;
		defaultButton = QMessageBox::Cancel;
	}
	if (buttons[DialogButton::Ok]) {
		qtButtons |= QMessageBox::Ok;
		defaultButton = QMessageBox::Ok;
	}
	dialog.setStandardButtons(qtButtons);
	dialog.setDefaultButton(defaultButton);

	QMessageBox::Icon dialogIcon = QMessageBox::Critical;
	switch (type) {
	case DialogType::Error:
		dialogIcon = QMessageBox::Critical;
		break;
	case DialogType::Warning:
		dialogIcon = QMessageBox::Warning;
		break;
	case DialogType::Information:
		dialogIcon = QMessageBox::Information;
		break;
	case DialogType::Question:
		dialogIcon = QMessageBox::Question;
		break;
	}
	dialog.setIcon(dialogIcon);

	auto ret = dialog.exec();

	switch (ret) {
	case QMessageBox::Yes:
		return DialogButton::Yes;
	case QMessageBox::No:
		return DialogButton::No;
	case QMessageBox::Cancel:
		return DialogButton::Cancel;
	case QMessageBox::Ok:
		return DialogButton::Ok;
	default:
		return DialogButton::Cancel;
	}
}
void FredView::editObjectTriggered() {
	handleObjectEditor(fred->currentObject);
}
void FredView::handleObjectEditor(int objNum) {
	if (fred->getNumMarked() > 1) {
		on_actionShips_triggered(false);
	} else {
		Assertion(objNum >= 0, "Popup object is not valid when editObjectTriggered was called!");

		if ((Objects[objNum].type == OBJ_START) || (Objects[objNum].type == OBJ_SHIP)) {
			on_actionShips_triggered(false);
		} else if (Objects[objNum].type == OBJ_JUMP_NODE || Objects[objNum].type == OBJ_WAYPOINT) {

			// Select the object before displaying the dialog
			fred->selectObject(objNum);

			// Use the existing slot for this to avoid duplicating code
			on_actionWaypoint_Paths_triggered(false);
		} else if (Objects[objNum].type == OBJ_POINT) {
			return;
		} else {
			UNREACHABLE("Unhandled object type!");
		}
	}
}
void FredView::mouseDoubleClickEvent(QMouseEvent* event) {
	auto viewLocal = ui->centralWidget->mapFromGlobal(event->globalPos());
	auto obj = _viewport->select_object(viewLocal.x(), viewLocal.y());

	if (obj >= 0) {
		handleObjectEditor(obj);
	} else {
		// Ignore event
		QWidget::mouseDoubleClickEvent(event);
	}
}
void FredView::orientEditorTriggered() {
	auto dialog = new dialogs::ObjectOrientEditorDialog(this, _viewport);
	// This is a modal dialog
	dialog->exec();
}
void FredView::onUpdateEditorActions() {
	ui->actionObjects->setEnabled(query_valid_object(fred->currentObject));
}
void FredView::on_actionWingForm_triggered(bool  /*enabled*/) {
	object* ptr = GET_FIRST(&obj_used_list);
	bool found = false;
	while (ptr != END_OF_LIST(&obj_used_list)) {
		if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START)) && (ptr->flags[Object::Object_Flags::Marked])) {
			if (Ships[ptr->instance].flags[Ship::Ship_Flags::Reinforcement]) {
				found = true;
				break;
			}
		}

		ptr = GET_NEXT(ptr);
	}

	if (found) {
		auto button = showButtonDialog(DialogType::Warning,
									   "Reinforcement conflict",
									   "Some of the ships you selected to create a wing are marked as reinforcements. "
										   "Press Ok to clear the flag on all selected ships. Press Cancel to not create the wing.",
									   { DialogButton::Ok, DialogButton::Cancel });
		if (button == DialogButton::Ok) {
			ptr = GET_FIRST(&obj_used_list);
			while (ptr != END_OF_LIST(&obj_used_list)) {
				if (((ptr->type == OBJ_SHIP) || (ptr->type == OBJ_START))
					&& (ptr->flags[Object::Object_Flags::Marked])) {
					fred->set_reinforcement(Ships[ptr->instance].ship_name, 0);
				}

				ptr = GET_NEXT(ptr);
			}
		} else {
			return;
		}
	}

	if (fred->create_wing()) {
		// TODO: Autosave
	}
}
void FredView::on_actionWingDisband_triggered(bool  /*enabled*/) {
	if (fred->query_single_wing_marked()) {
		fred->remove_wing(fred->cur_wing);
	} else {
		showButtonDialog(DialogType::Error,
						 "Error",
						 "One and only one wing must be selected for this operation",
						 { DialogButton::Ok });
	}
}
void FredView::onUpdateWingActionStatus() {
	int count = 0;
	object* ptr;

	if (query_valid_object(fred->currentObject)) {
		ptr = GET_FIRST(&obj_used_list);
		while (ptr != END_OF_LIST(&obj_used_list)) {
			if (ptr->flags[Object::Object_Flags::Marked]) {
				if (ptr->type == OBJ_SHIP) {
					int ship_type = ship_query_general_type(ptr->instance);
					if (ship_type > -1 && (Ship_types[ship_type].flags[Ship::Type_Info_Flags::AI_can_form_wing])) {
						count++;
					}
				}

				if (ptr->type == OBJ_START) {
					count++;
				}
			}

			ptr = GET_NEXT(ptr);
		}
	}

	ui->actionWingForm->setEnabled(count > 0);
	ui->actionWingDisband->setEnabled(fred->query_single_wing_marked());
}
void FredView::on_actionZoomSelected_triggered(bool) {
	if (query_valid_object(fred->currentObject)) {
		if (fred->getNumMarked() > 1) {
			_viewport->view_universe(true);
		} else {
			_viewport->view_object(fred->currentObject);
		}
	}
}
void FredView::on_actionZoomExtents_triggered(bool) {
	_viewport->view_universe(false);
}
std::unique_ptr<IDialog<dialogs::FormWingDialogModel>> FredView::createFormWingDialog() {
	std::unique_ptr<IDialog<dialogs::FormWingDialogModel>> dialog(new dialogs::FormWingDialog(nullptr, _viewport));
	return dialog;
}
bool FredView::showModalDialog(IBaseDialog* dlg) {
	auto qdlg = dynamic_cast<QDialog*>(dlg);
	if (qdlg == nullptr) {
		return false;
	}

	// We need to temporarily reparent the dialog so it's shown in the right location
	auto prevParent = qdlg->parentWidget();
	qdlg->setParent(this, Qt::Dialog);

	auto ret = qdlg->exec();

	qdlg->setParent(prevParent, Qt::Dialog);

	return ret == QDialog::Accepted;
}
void FredView::on_actionSelectionList_triggered(bool) {
	auto dialog = new dialogs::SelectionDialog(this, _viewport);
	// This is a modal dialog
	dialog->exec();
}
void FredView::on_actionOrbitSelected_triggered(bool enabled) {
	_viewport->Lookat_mode = enabled;
	if (_viewport->Lookat_mode && query_valid_object(fred->currentObject)) {
		vec3d v, loc;
		matrix m;

		loc = Objects[fred->currentObject].pos;
		vm_vec_sub(&v, &loc, &_viewport->view_pos);

		if (v.xyz.x || v.xyz.y || v.xyz.z) {
			vm_vector_2_matrix(&m, &v, NULL, NULL);
			_viewport->view_orient = m;
		}
	}
}
void FredView::on_actionRotateLocal_triggered(bool enabled) {
	_viewport->Group_rotate = enabled;
}
void FredView::on_actionSave_Camera_Pos_triggered(bool) {
	_viewport->saved_cam_pos = _viewport->view_pos;
	_viewport->saved_cam_orient = _viewport->view_orient;
}
void FredView::on_actionRestore_Camera_Pos_triggered(bool) {
	_viewport->view_pos = _viewport->saved_cam_pos;
	_viewport->view_orient = _viewport->saved_cam_orient;

	_viewport->needsUpdate();
}
void FredView::on_actionTool_Bar_triggered(bool enabled) {
	ui->toolBar->setVisible(enabled);
}
void FredView::on_actionStatus_Bar_triggered(bool enabled) {
	statusBar()->setVisible(enabled);
}
void FredView::on_actionClone_Marked_Objects_triggered(bool) {
	if (fred->getNumMarked() > 0) {
		_viewport->duplicate_marked_objects();
	}
}
void FredView::on_actionDelete_triggered(bool) {
	if (fred->getNumMarked() > 0) {
		fred->delete_marked();
	}
}
void FredView::on_actionDelete_Wing_triggered(bool) {
	if (fred->cur_wing >= 0) {
		fred->delete_wing(fred->cur_wing, 0);
	}
}
void FredView::initializeGroupActions() {
	// This is a bit ugly but it's easier than iterating though all actions in the menu...
	connect(ui->actionGroup_1, &QAction::triggered, this, [this]() { onGroupSelected(1); });
	connect(ui->actionGroup_2, &QAction::triggered, this, [this]() { onGroupSelected(2); });
	connect(ui->actionGroup_3, &QAction::triggered, this, [this]() { onGroupSelected(3); });
	connect(ui->actionGroup_4, &QAction::triggered, this, [this]() { onGroupSelected(4); });
	connect(ui->actionGroup_5, &QAction::triggered, this, [this]() { onGroupSelected(5); });
	connect(ui->actionGroup_6, &QAction::triggered, this, [this]() { onGroupSelected(6); });
	connect(ui->actionGroup_7, &QAction::triggered, this, [this]() { onGroupSelected(7); });
	connect(ui->actionGroup_8, &QAction::triggered, this, [this]() { onGroupSelected(8); });
	connect(ui->actionGroup_9, &QAction::triggered, this, [this]() { onGroupSelected(9); });


	connect(ui->actionSetGroup_1, &QAction::triggered, this, [this]() { onSetGroup(1); });
	connect(ui->actionSetGroup_2, &QAction::triggered, this, [this]() { onSetGroup(2); });
	connect(ui->actionSetGroup_3, &QAction::triggered, this, [this]() { onSetGroup(3); });
	connect(ui->actionSetGroup_4, &QAction::triggered, this, [this]() { onSetGroup(4); });
	connect(ui->actionSetGroup_5, &QAction::triggered, this, [this]() { onSetGroup(5); });
	connect(ui->actionSetGroup_6, &QAction::triggered, this, [this]() { onSetGroup(6); });
	connect(ui->actionSetGroup_7, &QAction::triggered, this, [this]() { onSetGroup(7); });
	connect(ui->actionSetGroup_8, &QAction::triggered, this, [this]() { onSetGroup(8); });
	connect(ui->actionSetGroup_9, &QAction::triggered, this, [this]() { onSetGroup(9); });
}
void FredView::onGroupSelected(int group) {
	fred->unmark_all();
	auto objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if (objp->type == OBJ_SHIP) {
			if (Ships[objp->instance].group & group) {
				fred->markObject(OBJ_INDEX(objp));
			}
		}

		objp = GET_NEXT(objp);
	}
}
void FredView::onSetGroup(int group) {
	bool err = false;

	for (auto i = 0; i < MAX_SHIPS; i++) {
		Ships[i].group &= ~group;
	}

	auto objp = GET_FIRST(&obj_used_list);
	while (objp != END_OF_LIST(&obj_used_list)) {
		if (objp->flags[Object::Object_Flags::Marked]) {
			if (objp->type == OBJ_SHIP) {
				Ships[objp->instance].group |= group;

			} else {
				err = true;
			}
		}

		objp = GET_NEXT(objp);
	}

	if (err) {
		showButtonDialog(DialogType::Error, "Error", "Only ships can be in groups, and not players or waypoints, etc.\n"
			"These illegal objects you marked were not placed in the group", { DialogButton::Ok });
	}

	fred->updateAllViewports();
}
void FredView::on_actionLevel_Object_triggered(bool) {
	_viewport->level_controlled();
}
void FredView::on_actionAlign_Object_triggered(bool) {
	_viewport->verticalize_controlled();
}
void FredView::on_actionControl_Object_triggered(bool) {
	_viewport->Control_mode = (_viewport->Control_mode + 1) % 2;
}
void FredView::on_actionNext_Subsystem_triggered(bool) {
	fred->select_next_subsystem();
}
void FredView::on_actionPrev_Subsystem_triggered(bool) {
	fred->select_previous_subsystem();
}
void FredView::on_actionCancel_Subsystem_triggered(bool) {
	fred->cancel_select_subsystem();
}
void FredView::on_actionError_Checker_triggered(bool) {
	fred->global_error_check();
}
void FredView::on_actionAbout_triggered(bool) {
	dialogs::AboutDialog dialog(this);
	dialog.exec();
}

void FredView::on_actionBackground_triggered(bool) {
	dialogs::BackgroundEditorDialog dialog(this, _viewport);
	dialog.exec();
}

void FredView::on_actionShield_System_triggered(bool) {
	dialogs::ShieldSystemDialog dialog(this, _viewport);
	dialog.exec();
}

void FredView::on_actionVoice_Acting_Manager_triggered(bool) {
	dialogs::VoiceActingManager dialog(this, _viewport);
	dialog.exec();
}
void FredView::on_actionMission_Objectives_triggered(bool) {
	dialogs::MissionGoalsDialog dialog(this, _viewport);
	dialog.exec();
}

void FredView::on_actionFiction_Viewer_triggered(bool) {
	dialogs::FictionViewerDialog dialog(this, _viewport);
	dialog.exec();
}

} // namespace fred
} // namespace fso
