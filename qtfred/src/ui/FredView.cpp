#include "FredView.h"
#include "ui_FredView.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QSettings>

#include <project.h>

#include <qevent.h>
#include <FredApplication.h>
#include <io/key.h>

#include "mission/editor.h"

#include "widgets/ColorComboBox.h"

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

	// Forward all focus related events to the render widget since we don't do much ourself
	setFocusProxy(ui->centralWidget);

	// This is not possible to do with the designer
	ui->actionNew->setShortcuts(QKeySequence::New);
	ui->actionOpen->setShortcuts(QKeySequence::Open);
	ui->actionSave->setShortcuts(QKeySequence::Save);
	ui->actionSave_As->setShortcuts(QKeySequence::SaveAs);
	ui->actionExit->setShortcuts(QKeySequence::Quit);
	ui->actionUndo->setShortcuts(QKeySequence::Undo);
	ui->actionDelete->setShortcuts(QKeySequence::Delete);

	setFocusPolicy(Qt::WheelFocus);

	// A combo box cannot be added by the designer so we do that manually here
	_shipClassBox.reset(new ColorComboBox());

	ui->toolBar->addWidget(_shipClassBox.get());

	connect(ui->actionOpen, &QAction::triggered, this, &FredView::openLoadMissionDIalog);
	connect(ui->actionNew, &QAction::triggered, this, &FredView::newMission);

	connect(fredApp, &FredApplication::onIdle, this, &FredView::updateUI);

	updateRecentFileList();

	initializeStatusBar();
	initializePopupMenus();

	installEventFilter(this);
}

FredView::~FredView() {
}

void FredView::setEditor(Editor* editor, FredRenderer* renderer) {
	Assertion(fred == nullptr, "Resetting the editor is currently not supported!");
	Assertion(_renderer == nullptr, "Resetting the renderer is currently not supported!");

	fred = editor;
	_renderer = renderer;

	ui->centralWidget->setEditor(editor, renderer);

	connect(fred, &Editor::missionLoaded, this, &FredView::on_mission_loaded);

	// Sets the initial window title
	on_mission_loaded("");

	syncViewOptions();
	connect(this, &FredView::viewIdle, this, &FredView::onUpdateConstrains);
	connect(this, &FredView::viewIdle, this, &FredView::onUpdateEditingMode);
	connect(this, &FredView::viewIdle, this, &FredView::onUpdateViewSpeeds);
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

	loadMissionFile(pathName);
}

void FredView::on_actionExit_triggered(bool) {
	close();
}

void FredView::on_mission_loaded(const std::string& filepath) {
	QString filename = "Untitled";
	if (!filepath.empty()) {
		filename = QFileInfo(QString::fromStdString(filepath)).fileName();
	}

	auto title = tr("%1 - qtFRED v%2 - FreeSpace 2 Mission Editor").arg(filename, FS_VERSION_FULL);

	setWindowTitle(title);

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
bool FredView::eventFilter(QObject* watched, QEvent* event) {
	if (event->type() == QEvent::ShortcutOverride) {
		auto keyEvent = static_cast<QKeyEvent*>(event);
		// Only use shortcuts on the keyboard since the keypad is needed for the camera controls
		// This currently only affects the shortcuts using 1-8
		if (keyEvent->modifiers().testFlag(Qt::KeypadModifier)) {
			keyEvent->accept();
			return true;
		}
	}
	return false;
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
	connectActionToViewSetting(ui->actionShow_Ships, &_renderer->view.Show_ships);
	connectActionToViewSetting(ui->actionShow_Player_Starts, &_renderer->view.Show_starts);
	connectActionToViewSetting(ui->actionShow_Waypoints, &_renderer->view.Show_waypoints);

	// TODO: Dynamically handle the Show teams option

	connectActionToViewSetting(ui->actionShow_Ship_Models, &_renderer->view.Show_ship_models);
	connectActionToViewSetting(ui->actionShow_Outlines, &_renderer->view.Show_outlines);
	connectActionToViewSetting(ui->actionShow_Ship_Info, &_renderer->view.Show_ship_info);
	connectActionToViewSetting(ui->actionShow_Coordinates, &_renderer->view.Show_coordinates);
	connectActionToViewSetting(ui->actionShow_Grid_Positions, &_renderer->view.Show_grid_positions);
	connectActionToViewSetting(ui->actionShow_Distances, &_renderer->view.Show_distances);
	connectActionToViewSetting(ui->actionShow_Model_Paths, &_renderer->view.Show_paths_fred);
	connectActionToViewSetting(ui->actionShow_Model_Dock_Points, &_renderer->view.Show_dock_points);

	connectActionToViewSetting(ui->actionShow_Grid, &_renderer->view.Show_grid);
	connectActionToViewSetting(ui->actionShow_Horizon, &_renderer->view.Show_horizon);
	connectActionToViewSetting(ui->actionDouble_Fine_Gridlines, &double_fine_gridlines);
	connectActionToViewSetting(ui->actionAnti_Aliased_Gridlines, &_renderer->view.Aa_gridlines);
	connectActionToViewSetting(ui->actionShow_3D_Compass, &_renderer->view.Show_compass);
	connectActionToViewSetting(ui->actionShow_Background, &_renderer->view.Show_stars);

	connectActionToViewSetting(ui->actionLighting_from_Suns, &_renderer->view.Lighting_on);
}
void FredView::initializeStatusBar() {
	_statusBarUnitsLabel = new QLabel();
	statusBar()->addPermanentWidget(_statusBarUnitsLabel);
}
void FredView::updateUI() {
	if (!_renderer) {
		// The following code requires a valid renderer
		return;
	}

	_statusBarUnitsLabel->setText(tr("Units = %1 Meters").arg(_renderer->The_grid->square_size));

	viewIdle();
}
void FredView::connectActionToViewSetting(QAction* option, bool* destination) {
	Q_ASSERT(option->isCheckable());

	// Use our view idle function for updating the action status whenever possible
	// TODO: Maybe this could be improved with an event based property system but that would need to be implemented
	connect(this, &FredView::viewIdle, [option, destination]() {
		option->setChecked(*destination);
	});

	// then connect the signal to a handler for updating the view setting
	// The pointer should be valid as long as this signal is active since it should be pointing inside the renderer (I hope...)
	connect(option, &QAction::triggered, [this, destination](bool value) {
		*destination = value;

		// View settings have changed so we need to update the window
		_renderer->scheduleUpdate();
	});
}
void FredView::showContextMenu(const QPoint& globalPos) {
	_viewPopup->exec(globalPos);
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
	_viewPopup->addMenu(ui->menuViewpoint);
	_viewPopup->addSeparator();
}

void FredView::onUpdateConstrains() {
	ui->actionConstrainX->setChecked(
		_renderer->Constraint.xyz.x && !_renderer->Constraint.xyz.y && !_renderer->Constraint.xyz.z);
	ui->actionConstrainY->setChecked(
		!_renderer->Constraint.xyz.x && _renderer->Constraint.xyz.y && !_renderer->Constraint.xyz.z);
	ui->actionConstrainZ->setChecked(
		!_renderer->Constraint.xyz.x && !_renderer->Constraint.xyz.y && _renderer->Constraint.xyz.z);
	ui->actionConstrainXZ->setChecked(
		_renderer->Constraint.xyz.x && !_renderer->Constraint.xyz.y && _renderer->Constraint.xyz.z);
	ui->actionConstrainXY->setChecked(
		_renderer->Constraint.xyz.x && _renderer->Constraint.xyz.y && !_renderer->Constraint.xyz.z);
	ui->actionConstrainYZ->setChecked(
		!_renderer->Constraint.xyz.x && _renderer->Constraint.xyz.y && _renderer->Constraint.xyz.z);
}
void FredView::on_actionConstrainX_triggered(bool enabled) {
	if (enabled) {
		vm_vec_make(&_renderer->Constraint, 1.0f, 0.0f, 0.0f);
		vm_vec_make(&_renderer->Anticonstraint, 0.0f, 1.0f, 1.0f);
		_renderer->Single_axis_constraint = true;
	}
}
void FredView::on_actionConstrainY_triggered(bool enabled) {
	if (enabled) {
		vm_vec_make(&_renderer->Constraint, 0.0f, 1.0f, 0.0f);
		vm_vec_make(&_renderer->Anticonstraint, 1.0f, 0.0f, 1.0f);
		_renderer->Single_axis_constraint = true;
	}
}
void FredView::on_actionConstrainZ_triggered(bool enabled) {
	if (enabled) {
		vm_vec_make(&_renderer->Constraint, 0.0f, 0.0f, 1.0f);
		vm_vec_make(&_renderer->Anticonstraint, 1.0f, 1.0f, 0.0f);
		_renderer->Single_axis_constraint = true;
	}
}
void FredView::on_actionConstrainXZ_triggered(bool enabled) {
	if (enabled) {
		vm_vec_make(&_renderer->Constraint, 1.0f, 0.0f, 1.0f);
		vm_vec_make(&_renderer->Anticonstraint, 0.0f, 1.0f, 0.0f);
		_renderer->Single_axis_constraint = false;
	}
}
void FredView::on_actionConstrainXY_triggered(bool enabled) {
	if (enabled) {
		vm_vec_make(&_renderer->Constraint, 1.0f, 1.0f, 0.0f);
		vm_vec_make(&_renderer->Anticonstraint, 0.0f, 0.0f, 1.0f);
		_renderer->Single_axis_constraint = false;
	}
}
void FredView::on_actionConstrainYZ_triggered(bool enabled) {
	if (enabled) {
		vm_vec_make(&_renderer->Constraint, 0.0f, 1.0f, 1.0f);
		vm_vec_make(&_renderer->Anticonstraint, 1.0f, 0.0f, 0.0f);
		_renderer->Single_axis_constraint = false;
	}
}
RenderWidget* FredView::getRenderWidget() {
	return ui->centralWidget;
}
void FredView::on_actionSelect_triggered(bool enabled) {
	if (enabled) {
		_renderer->Editing_mode = CursorMode::Selecting;
	}
}
void FredView::on_actionSelectMove_triggered(bool enabled) {
	if (enabled) {
		_renderer->Editing_mode = CursorMode::Moving;
	}
}
void FredView::on_actionSelectRotate_triggered(bool enabled) {
	if (enabled) {
		_renderer->Editing_mode = CursorMode::Rotating;
	}
}
void FredView::onUpdateEditingMode() {
	ui->actionSelect->setChecked(_renderer->Editing_mode == CursorMode::Selecting);
	ui->actionSelectMove->setChecked(_renderer->Editing_mode == CursorMode::Moving);
	ui->actionSelectRotate->setChecked(_renderer->Editing_mode == CursorMode::Rotating);

	ui->centralWidget->setCursorMode(_renderer->Editing_mode);
}
bool FredView::event(QEvent* event) {
	switch(event->type()) {
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

	_renderer->Cursor_over = -1;
}
void FredView::windowDeactivated() {
	key_lost_focus();

	_renderer->Cursor_over = -1;
}
void FredView::on_actionHide_Marked_Objects_triggered(bool enabled) {
	fred->hideMarkedObjects();
}
void FredView::on_actionShow_All_Hidden_Objects_triggered(bool enabled) {
	fred->showHiddenObjects();
}
void FredView::onUpdateViewSpeeds() {
	ui->actionx1->setChecked(_renderer->physics_speed == 1);
	ui->actionx2->setChecked(_renderer->physics_speed == 2);
	ui->actionx3->setChecked(_renderer->physics_speed == 3);
	ui->actionx5->setChecked(_renderer->physics_speed == 5);
	ui->actionx8->setChecked(_renderer->physics_speed == 8);
	ui->actionx10->setChecked(_renderer->physics_speed == 10);
	ui->actionx50->setChecked(_renderer->physics_speed == 50);
	ui->actionx100->setChecked(_renderer->physics_speed == 100);

	ui->actionRotx1->setChecked(_renderer->physics_rot == 2);
	ui->actionRotx5->setChecked(_renderer->physics_rot == 10);
	ui->actionRotx12->setChecked(_renderer->physics_rot == 25);
	ui->actionRotx25->setChecked(_renderer->physics_rot == 50);
	ui->actionRotx50->setChecked(_renderer->physics_rot == 100);
}
void FredView::on_actionx1_triggered(bool enabled) {
	if (enabled) {
		_renderer->physics_speed = 1;
	}
}
void FredView::on_actionx2_triggered(bool enabled) {
	if (enabled) {
		_renderer->physics_speed = 2;
	}
}
void FredView::on_actionx3_triggered(bool enabled) {
	if (enabled) {
		_renderer->physics_speed = 3;
	}
}
void FredView::on_actionx5_triggered(bool enabled) {
	if (enabled) {
		_renderer->physics_speed = 5;
	}
}
void FredView::on_actionx8_triggered(bool enabled) {
	if (enabled) {
		_renderer->physics_speed = 8;
	}
}
void FredView::on_actionx10_triggered(bool enabled) {
	if (enabled) {
		_renderer->physics_speed = 10;
	}
}
void FredView::on_actionx50_triggered(bool enabled) {
	if (enabled) {
		_renderer->physics_speed = 50;
	}
}
void FredView::on_actionx100_triggered(bool enabled) {
	if (enabled) {
		_renderer->physics_speed = 100;
	}
}
void FredView::on_actionRotx1_triggered(bool enabled) {
	if (enabled) {
		_renderer->physics_rot = 2;
	}
}
void FredView::on_actionRotx5_triggered(bool enabled) {
	if (enabled) {
		_renderer->physics_rot = 10;
	}
}
void FredView::on_actionRotx12_triggered(bool enabled) {
	if (enabled) {
		_renderer->physics_rot = 25;
	}
}
void FredView::on_actionRotx25_triggered(bool enabled) {
	if (enabled) {
		_renderer->physics_rot = 50;
	}
}
void FredView::on_actionRotx50_triggered(bool enabled) {
	if (enabled) {
		_renderer->physics_rot = 100;
	}
}

} // namespace fred
} // namespace fso

