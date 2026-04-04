#include "FredView.h"
#include "ui_FredView.h"

#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>
#include <QKeyEvent>
#include <QProcess>
#include <QSettings>

#include <project.h>

#include <qevent.h>
#include <FredApplication.h>
#include <ui/dialogs/ShipEditor/ShipEditorDialog.h>
#include <ui/dialogs/WingEditorDialog.h>
#include <ui/dialogs/PropEditorDialog.h>
#include <ui/dialogs/MissionEventsDialog.h>
#include <mission/dialogs/MissionEventsDialogModel.h>
#include <ui/dialogs/AsteroidEditorDialog.h>
#include <ui/dialogs/VolumetricNebulaDialog.h>
#include <ui/dialogs/BriefingEditorDialog.h>
#include <ui/dialogs/WaypointEditorDialog.h>
#include <ui/dialogs/JumpNodeEditorDialog.h>
#include <ui/dialogs/CampaignEditorDialog.h>
#include <ui/dialogs/MissionGoalsDialog.h>
#include <ui/dialogs/ObjectOrientEditorDialog.h>
#include <ui/dialogs/MissionSpecDialog.h>
#include <ui/dialogs/MissionCutscenesDialog.h>
#include <ui/dialogs/FormWingDialog.h>
#include <ui/dialogs/AboutDialog.h>
#include <ui/dialogs/MissionStatsDialog.h>
#include <ui/dialogs/BackgroundEditorDialog.h>
#include <ui/dialogs/ShieldSystemDialog.h>
#include <ui/dialogs/GlobalShipFlagsDialog.h>
#include <ui/dialogs/VoiceActingManager.h>
#include <globalincs/linklist.h>
#include <ui/dialogs/SelectionDialog.h>
#include <ui/dialogs/FictionViewerDialog.h>
#include <ui/dialogs/CommandBriefingDialog.h>
#include <ui/dialogs/DebriefingDialog.h>
#include <ui/dialogs/ReinforcementsEditorDialog.h>
#include <ui/dialogs/TeamLoadoutDialog.h>
#include <ui/dialogs/VariableDialog.h>
#include <ui/dialogs/MusicPlayerDialog.h>
#include <ui/dialogs/RelativeCoordinatesDialog.h>
#include <ui/dialogs/SaveAsTemplateDialog.h>
#include <ui/dialogs/TemplateBrowserDialog.h>
#include <ui/dialogs/PreferencesDialog.h>
#include <ui/dialogs/LayerManagerDialog.h>
#include <ui/ControlBindings.h>
#include <iff_defs/iff_defs.h>

#include "mission/Editor.h"
#include "mission/management.h"
#include "mission/missionparse.h"
#include "missioneditor/missionsave.h"

#include "widgets/ColorComboBox.h"

#include "util.h"
#include "mission/object.h"

// Forward-declare global-scope function before entering any namespace
SCP_string cmdline_build_string();

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

	connect(ui->actionOpen, &QAction::triggered, this, &FredView::openLoadMissionDialog);
	connect(ui->actionNew, &QAction::triggered, this, &FredView::newMission);

	// Save Format actions are mutually exclusive
	auto* saveFormatGroup = new QActionGroup(this);
	saveFormatGroup->addAction(ui->actionFS2_Open);
	saveFormatGroup->addAction(ui->actionFS2_Retail);
	saveFormatGroup->addAction(ui->actionFS2_Compatibility);

	connect(fredApp, &FredApplication::onIdle, this, &FredView::updateUI);

	// TODO: Hook this up with the modified state of the mission
	setWindowModified(false);

	updateRecentFileList();

	initializeStatusBar();
	initializePopupMenus();

	initializeGroupActions();

	auto propsAction = new QAction(tr("Props"), this);
	connect(propsAction, &QAction::triggered, this, &FredView::on_actionProps_triggered);
	ui->menuObjects->insertAction(ui->actionWaypoint_Paths, propsAction);

	connect(ui->actionPreferences, &QAction::triggered, this, [this]() {
		dialogs::PreferencesDialog preferencesDialog(this, _viewport);
		preferencesDialog.exec();
	});

	connect(ui->actionManage_Layers, &QAction::triggered, this, [this]() { openLayerManagerDialog(); });
	connect(ui->actionUnhide_Layers, &QAction::triggered, this, [this]() {
		if (_viewport != nullptr) {
			_viewport->showAllLayers();
		}
	});
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

	auto propLabel = new QLabel(tr("Props"), ui->toolBar);
	ui->toolBar->addWidget(propLabel);
	_propClassBox.reset(new PropComboBox(nullptr));
	ui->toolBar->addWidget(_propClassBox.get());
	connect(_propClassBox.get(), &PropComboBox::propClassSelected, this, &FredView::onPropClassSelected);

	connect(fred, &Editor::missionLoaded, this, &FredView::on_mission_loaded);
	connect(fred, &Editor::missionChanged, this, [this]() { _missionModified = true; });

	// Sets the initial window title
	on_mission_loaded("");

	syncViewOptions();

	connect(this, &FredView::viewIdle, this, &FredView::onUpdateConstrains);
	connect(this, &FredView::viewIdle, this, &FredView::onUpdateEditingMode);
	connect(this, &FredView::viewIdle, this, &FredView::onUpdateViewSpeeds);
	connect(this, &FredView::viewIdle, this, &FredView::onUpdateCameraControlActions);
	connect(this, &FredView::viewIdle, this, &FredView::onUpdateSelectionLock);
	connect(this, &FredView::viewIdle, this, &FredView::onUpdateShipClassBox);
	connect(this, &FredView::viewIdle, this, &FredView::onUpdatePropClassBox);
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
	connect(this, &FredView::viewIdle, this, [this]() { ui->actionRevert->setEnabled(!saveName.isEmpty()); });
	connect(this, &FredView::viewIdle, this, [this]() { ui->actionUndo->setEnabled(fred->undoAvailable != 0); });
	connect(this, &FredView::viewIdle, this, [this]() { ui->actionDisable_Undo->setChecked(fred->autosaveDisabled != 0); });

}

void FredView::loadMissionFile(const QString& pathName, int flags) {
	if (!maybePromptToSaveMissionChanges(tr("loading another mission"))) {
		return;
	}

	statusBar()->showMessage(tr("Loading mission %1").arg(pathName));
	try {
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

		// probably good to clear selections in qtFRED too
		fred->clean_up_selections();

		auto pathToLoad = pathName.toStdString();
		if (!(flags & MPF_IS_TEMPLATE))
			fred->maybeUseAutosave(pathToLoad);

		fred->loadMission(pathToLoad, flags);

		QApplication::restoreOverrideCursor();
	} catch (const fso::fred::mission_load_error&) {
		QApplication::restoreOverrideCursor();

		QMessageBox::critical(this, tr("Failed loading mission."), tr("Could not parse the mission."));
		statusBar()->clearMessage();
	}
}

void FredView::openLoadMissionDialog() {
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

void FredView::on_actionSave_triggered(bool state) {
	Q_UNUSED(state);
	saveMissionToCurrentPath();
}
void FredView::on_actionSave_As_triggered(bool) {
	saveMissionAs();
}

bool FredView::saveMissionToCurrentPath() {
	Fred_mission_save save;
	save.set_save_format(_missionSaveFormat);
	save.set_always_save_display_names(_viewport->Always_save_display_names);
	save.set_view_pos(_viewport->view_pos);
	save.set_view_orient(_viewport->view_orient);
	save.set_fred_alt_names(Fred_alt_names);
	save.set_fred_callsigns(Fred_callsigns);

	if (saveName.isEmpty()) {
		return saveMissionAs();
	}

	save.save_mission_file(saveName.replace('/', DIR_SEPARATOR_CHAR).toUtf8().constData());
	_missionModified = false;
	return true;
}
bool FredView::saveMissionAs() {
	Fred_mission_save save;
	save.set_save_format(_missionSaveFormat);
	save.set_always_save_display_names(_viewport->Always_save_display_names);
	save.set_view_pos(_viewport->view_pos);
	save.set_view_orient(_viewport->view_orient);
	save.set_fred_alt_names(Fred_alt_names);
	save.set_fred_callsigns(Fred_callsigns);

	saveName = QFileDialog::getSaveFileName(this, tr("Save mission"), QString(), tr("FS2 missions (*.fs2)"));

	if (saveName.isEmpty()) {
		return false;
	}

	save.save_mission_file(saveName.replace('/',DIR_SEPARATOR_CHAR).toUtf8().constData());
	_missionModified = false;
	return true;
}

void FredView::saveAsTemplate() {
	// Collect template metadata first
	dialogs::SaveAsTemplateDialog metaDialog(this, getUsername());
	if (metaDialog.exec() != QDialog::Accepted)
		return;

	// Default to data/missions/templates/ and create it if needed
	QString templatesDir = QDir::currentPath() + "/data/missions/templates";
	QDir().mkpath(templatesDir);

	QString templateName = QFileDialog::getSaveFileName(this,
		tr("Save As Template"),
		templatesDir,
		tr("FS2 mission templates (*.fst)"));

	if (templateName.isEmpty())
		return;

	if (!templateName.endsWith(".fst", Qt::CaseInsensitive))
		templateName += ".fst";

	Fred_mission_save save;
	save.set_always_save_display_names(_viewport->Always_save_display_names);
	save.set_fred_alt_names(Fred_alt_names);
	save.set_fred_callsigns(Fred_callsigns);
	save.set_template_info(metaDialog.templateInfo());

	save.save_template_file(templateName.replace('/', DIR_SEPARATOR_CHAR).toUtf8().constData());
}

void FredView::loadTemplate() {
	QString templatesDir = QDir::currentPath() + "/data/missions/templates";
	QDir().mkpath(templatesDir);

	dialogs::TemplateBrowserDialog browser(this, templatesDir);
	if (browser.exec() != QDialog::Accepted)
		return;

	QString templateName = browser.selectedTemplatePath();
	if (templateName.isEmpty())
		return;

	auto result = QMessageBox::question(this,
		tr("Load Template"),
		tr("This will replace all mission data. Continue?"),
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No);

	if (result != QMessageBox::Yes)
		return;

	loadMissionFile(templateName.replace('/', DIR_SEPARATOR_CHAR), MPF_IS_TEMPLATE);
}

void FredView::on_actionLoad_Template_triggered(bool) {
	loadTemplate();
}

void FredView::on_actionSave_As_Template_triggered(bool) {
	saveAsTemplate();
}

void FredView::on_actionRevert_triggered(bool) {
	if (saveName.isEmpty()) {
		QMessageBox::information(this, tr("Revert"), tr("Mission has not been saved yet."));
		return;
	}
	auto result = QMessageBox::question(this,
		tr("Revert to Last Save"),
		tr("Discard all changes and reload from disk?"),
		QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
	if (result != QMessageBox::Yes)
		return;
	// Clear modified flag so loadMissionFile doesn't prompt to save again
	_missionModified = false;
	loadMissionFile(saveName);
}

void FredView::on_actionUndo_triggered(bool) {
	// Preserve camera state and saveName because autoload() triggers missionLoaded which would overwrite them
	auto savedViewPos    = _viewport->view_pos;
	auto savedViewOrient = _viewport->view_orient;
	auto savedSaveName   = saveName;

	fred->autoload();

	_viewport->view_pos    = savedViewPos;
	_viewport->view_orient = savedViewOrient;
	saveName               = savedSaveName;
}

void FredView::on_actionDisable_Undo_triggered(bool checked) {
	fred->autosaveDisabled = checked ? 1 : 0;
}

void FredView::on_actionFS2_Open_triggered(bool) {
	_missionSaveFormat = MissionFormat::STANDARD;
}

void FredView::on_actionFS2_Retail_triggered(bool) {
	_missionSaveFormat = MissionFormat::RETAIL;
}

void FredView::on_actionFS2_Compatibility_triggered(bool) {
	_missionSaveFormat = MissionFormat::COMPATIBILITY_MODE;
}

void FredView::on_actionFS1_Mission_triggered(bool) {
	if (!maybePromptToSaveMissionChanges(tr("importing an FS1 mission"))) {
		return;
	}
	// Mark as unmodified so loadMissionFile won't prompt again after import
	_missionModified = false;

	QStringList srcPaths = QFileDialog::getOpenFileNames(this,
		tr("Select FS1 mission(s) to import"),
		QString(),
		tr("FreeSpace Missions (*.fsm)"));

	if (srcPaths.isEmpty())
		return;

	QString destDir = QFileDialog::getExistingDirectory(this,
		tr("Select destination folder for converted missions"));

	if (destDir.isEmpty())
		return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	int successes = 0;
	QString lastDestPath;
	for (const auto& qSrcPath : srcPaths) {
		SCP_string srcPath = qSrcPath.toStdString();
		if (!fred->loadMission(srcPath, MPF_IMPORT_FSM | MPF_FAST_RELOAD))
			continue;

		// Derive output filename: strip directory, replace .fsm with .fs2
		SCP_string filename = srcPath;
		auto slash = filename.find_last_of("/\\");
		if (slash != SCP_string::npos)
			filename = filename.substr(slash + 1);
		auto dot = filename.rfind('.');
		if (dot != SCP_string::npos)
			filename = filename.substr(0, dot);
		filename += ".fs2";

		SCP_string destPath = destDir.toStdString();
		if (!destPath.empty() && destPath.back() != '/' && destPath.back() != '\\')
			destPath += DIR_SEPARATOR_CHAR;
		destPath += filename;

		Fred_mission_save fileSave;
		fileSave.set_save_format(_missionSaveFormat);
		fileSave.set_always_save_display_names(_viewport->Always_save_display_names);
		fileSave.set_view_pos(_viewport->view_pos);
		fileSave.set_view_orient(_viewport->view_orient);
		fileSave.set_fred_alt_names(Fred_alt_names);
		fileSave.set_fred_callsigns(Fred_callsigns);

		if (fileSave.save_mission_file(destPath.c_str()) == 0) {
			++successes;
			lastDestPath = QString::fromStdString(destPath);
		}
	}

	QApplication::restoreOverrideCursor();

	int numFiles = srcPaths.size();

	if (numFiles > 1) {
		fred->createNewMission();
		QMessageBox::information(this, tr("Import Complete"),
			tr("Imported %1 of %2 mission(s). Check the destination folder to verify results.")
				.arg(successes).arg(numFiles));
	} else if (numFiles == 1) {
		if (successes == 1) {
			loadMissionFile(lastDestPath.replace('/', DIR_SEPARATOR_CHAR));
		} else {
			QMessageBox::warning(this, tr("Import Failed"), tr("Could not import the selected mission."));
		}
	}
}

void FredView::on_actionRun_FreeSpace_2_Open_triggered(bool) {
	if (!saveMissionToCurrentPath())
		return;

	// Try to find the FSO executable next to this one by replacing the editor name
	QFileInfo appInfo(QCoreApplication::applicationFilePath());
	QString dir = appInfo.absolutePath();

	QStringList candidates;

	// Derive a candidate by replacing editor name with fs2_open
	QString derived = appInfo.completeBaseName();
	QString derivedLower = derived.toLower();
	for (const QString& editorName : {QString("qtfred"), QString("fred2_open")}) {
		int idx = derivedLower.indexOf(editorName);
		if (idx != -1) {
			QString candidate = derived;
			candidate.replace(idx, editorName.length(), derived[idx].isUpper() ? "FS2_Open" : "fs2_open");
			candidates << dir + "/" + candidate;
			break;
		}
	}

	// Platform-aware fallback names
#ifdef WIN32
	candidates << dir + "/fs2_open.exe"
	           << dir + "/fs2_open_r.exe";
#else
	candidates << dir + "/fs2_open"
	           << dir + "/fs2_open_r";
#endif

	QString exePath;
	for (const auto& candidate : candidates) {
		if (QFileInfo::exists(candidate)) {
			exePath = candidate;
			break;
		}
	}

	if (exePath.isEmpty()) {
		QMessageBox::warning(this, tr("Run FreeSpace"),
			tr("Could not find the FreeSpace 2 Open executable next to this editor.\n"
			   "Ensure fs2_open is in the same directory as qtfred."));
		return;
	}

	QString args = QString::fromStdString(cmdline_build_string());

	if (!QProcess::startDetached(exePath, args.split(' ', QString::SkipEmptyParts))) {
		QMessageBox::warning(this, tr("Run FreeSpace"),
			tr("Failed to launch: %1").arg(exePath));
	}
}

void FredView::on_mission_loaded(const std::string& filepath) {
	// Clear browsed head ANIs so the new mission's message scan starts fresh.
	fso::fred::dialogs::MissionEventsDialogModel::clearBrowsedHeadAnis();

	if (_viewport != nullptr) {
		_viewport->reloadLayersFromMission();
	}

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
		auto qpath = QString::fromStdString(filepath);
		// Templates are loaded to edit, not saved back to their original path
		if (!qpath.endsWith(".fst", Qt::CaseInsensitive)) {
			saveName = qpath;
		}
		addToRecentFiles(qpath);
	} else {
		saveName = QString();
	}

	_missionModified = false;
}

QSurface* FredView::getRenderSurface() {
	return ui->centralWidget->getRenderSurface();
}
void FredView::newMission() {
	if (!maybePromptToSaveMissionChanges(tr("creating a new mission"))) {
		return;
	}

	fred->createNewMission();
}
void FredView::addToRecentFiles(const QString& path) {
	// Templates are not mission files; don't pollute the recent list with them
	if (path.endsWith(".fst", Qt::CaseInsensitive))
		return;

	// Backup files are internal; don't pollute the recent list with them
	if (QFileInfo(path).baseName().compare("Backup", Qt::CaseInsensitive) == 0)
		return;

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
	// Initialize the Show_iff visibility vector after IFF data is loaded
	fredApp->runAfterInit([this]() {
		for (auto i = 0; i < (int)Iff_info.size(); ++i) {
			_viewport->view.Show_iff.push_back(true);
		}
	});

	connectActionToViewSetting(ui->actionShow_Ship_Models, &_viewport->view.Show_ship_models);
	connectActionToViewSetting(ui->actionShow_Outlines, &_viewport->view.Show_outlines);
	connectActionToViewSetting(ui->actionDraw_Outlines_On_Selected_Ships, &_viewport->view.Draw_outlines_on_selected_ships);
	connectActionToViewSetting(ui->actionDraw_Outline_At_Warpin_Position, &_viewport->view.Draw_outline_at_warpin_position);
	connectActionToViewSetting(ui->actionShow_Ship_Info, &_viewport->view.Show_ship_info);
	connectActionToViewSetting(ui->actionShow_Coordinates, &_viewport->view.Show_coordinates);
	connectActionToViewSetting(ui->actionShow_Grid_Positions, &_viewport->view.Show_grid_positions);
	connectActionToViewSetting(ui->actionShow_Distances, &_viewport->view.Show_distances);
	connectActionToViewSetting(ui->actionShow_Model_Paths, &_viewport->view.Show_paths_fred);
	connectActionToViewSetting(ui->actionShow_Model_Dock_Points, &_viewport->view.Show_dock_points);
	connectActionToViewSetting(ui->actionShow_Bay_Paths, &_viewport->view.Show_bay_paths);
	connectActionToViewSetting(ui->actionHighlight_Selectable_Subsystems, &_viewport->view.Highlight_selectable_subsys);

	connectActionToViewSetting(ui->actionShow_Grid, &_viewport->view.Show_grid);
	connectActionToViewSetting(ui->actionShow_Horizon, &_viewport->view.Show_horizon);
	connectActionToViewSetting(ui->actionShow_3D_Compass, &_viewport->view.Show_compass);
	connectActionToViewSetting(ui->actionShow_Background, &_viewport->view.Show_stars);

	connectActionToViewSetting(ui->actionLighting_from_Suns, &_viewport->view.Lighting_on);
	connectActionToViewSetting(ui->actionRender_Full_Detail, &_viewport->view.FullDetail);

	connectActionToViewSetting(ui->actionShowDistances, &_viewport->view.Show_distances);

	connect(ui->actionVisibility_Layers, &QAction::triggered, this, [this]() { openLayerManagerDialog(); });
}
void FredView::initializeStatusBar() {
	_statusBarViewmode = new QLabel();
	statusBar()->addPermanentWidget(_statusBarViewmode);

	_statusBarUnitsLabel = new QLabel();
	statusBar()->addPermanentWidget(_statusBarUnitsLabel);

	statusBar()->showMessage(tr("Every great mission starts here. No pressure."));
}
void FredView::updateUI() {
	if (!_viewport) {
		// The following code requires a valid viewport
		return;
	}

	_statusBarUnitsLabel->setText(tr("Units = %1 Meters").arg(_viewport->The_grid->square_size));
	setWindowModified(isMissionModified());

	if (_viewport->viewpoint == 1) {
		_statusBarViewmode->setText(tr("Viewpoint: %1").arg(object_name(_viewport->view_obj)));
	} else {
		_statusBarViewmode->setText(tr("Viewpoint: Camera"));
	}

	viewIdle();
	ensureViewportFocus();
}
void FredView::ensureViewportFocus() {
	if (QApplication::activeWindow() != this || QApplication::activeModalWidget() != nullptr) {
		return;
	}

	auto* focusedWidget = QApplication::focusWidget();
	if (focusedWidget != nullptr) {
		auto* focusedWindow = focusedWidget->window();
		if (focusedWindow != nullptr && focusedWindow != this) {
			return;
		}
	}

	if (focusedWidget != ui->centralWidget) {
		ui->centralWidget->setFocus(Qt::OtherFocusReason);
	}
}
bool FredView::isMissionModified() const {
	return _missionModified;
}

bool FredView::maybePromptToSaveMissionChanges(const QString& actionDescription) {
	if (!isMissionModified()) {
		return true;
	}

	QMessageBox confirmationDialog(this);
	confirmationDialog.setIcon(QMessageBox::Warning);
	confirmationDialog.setWindowTitle(tr("Unsaved Changes"));
	confirmationDialog.setText(tr("The current mission has unsaved changes."));
	confirmationDialog.setInformativeText(tr("Do you want to save your changes before %1?").arg(actionDescription));
	confirmationDialog.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
	confirmationDialog.setDefaultButton(QMessageBox::Save);
	confirmationDialog.setEscapeButton(QMessageBox::Cancel);

	switch (confirmationDialog.exec()) {
	case QMessageBox::Save:
		return saveMissionToCurrentPath();
	case QMessageBox::Discard:
		return true;
	case QMessageBox::Cancel:
	default:
		return false;
	}
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
void FredView::connectActionToViewSetting(QAction* option, std::vector<bool>* vector, size_t idx) {
	Q_ASSERT(option->isCheckable());

	// Use our view idle function for updating the action status whenever possible
	// TODO: Maybe this could be improved with an event based property system but that would need to be implemented
	connect(this, &FredView::viewIdle, this, [option, vector, idx]() {
		option->setChecked((*vector)[idx]);
		});

	// then connect the signal to a handler for updating the view setting
	// The pointer should be valid as long as this signal is active since it should be pointing inside the renderer (I hope...)
	connect(option, &QAction::triggered, this, [this, vector, idx](bool value) {
		(*vector)[idx] = value;

		// View settings have changed so we need to update the window
		_viewport->needsUpdate();
		});
}

void FredView::showContextMenu(const QPoint& globalPos) {
	auto localPos = ui->centralWidget->mapFromGlobal(globalPos);

	auto obj = _viewport->select_object(localPos.x(), localPos.y());
	if (obj >= 0) {
		fred->selectObject(obj);
		const auto objType = Objects[obj].type;
		const bool canAssignLayer = (objType == OBJ_SHIP) || (objType == OBJ_START) || (objType == OBJ_PROP);
		_moveToLayerMenu->menuAction()->setVisible(canAssignLayer);
		if (canAssignLayer) {
			populateMoveToLayerMenu(obj);
		}

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
	auto* manageLayersViewAction = new QAction(tr("Manage Layers..."), _viewPopup);
	connect(manageLayersViewAction, &QAction::triggered, this, [this]() { openLayerManagerDialog(); });
	_viewPopup->addAction(manageLayersViewAction);

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
	_editPopup->addSeparator();
	_moveToLayerMenu = new QMenu(tr("Move to Layer"), _editPopup);
	_editPopup->addMenu(_moveToLayerMenu);
}

void FredView::populateMoveToLayerMenu(int targetObject) {
	_moveToLayerMenu->clear();

	const auto layerNames = _viewport->getLayerNames();
	for (const auto& layerName : layerNames) {
		bool visible = true;
		_viewport->getLayerVisibility(layerName, &visible);

		auto* layerAction = new QAction(QString::fromStdString(layerName), _moveToLayerMenu);
		layerAction->setCheckable(true);
		layerAction->setChecked(_viewport->getObjectLayerName(targetObject) == layerName);
		layerAction->setEnabled(visible);

		connect(layerAction, &QAction::triggered, this, [this, layerName, targetObject]() {
			SCP_string error;
			if (Objects[targetObject].flags[Object::Object_Flags::Marked] && fred->getNumMarked() > 1) {
				_viewport->moveMarkedObjectsToLayer(layerName, &error);
			} else {
				_viewport->moveObjectToLayer(targetObject, layerName, &error);
			}

				if (!error.empty()) {
					showButtonDialog(DialogType::Error, "Layer Error", error, { DialogButton::Ok });
				}
			});
		_moveToLayerMenu->addAction(layerAction);
	}

	_moveToLayerMenu->addSeparator();
	auto* manageAction = _moveToLayerMenu->addAction(tr("Manage Layers..."));
	connect(manageAction, &QAction::triggered, this, [this]() { openLayerManagerDialog(); });
}

void FredView::openLayerManagerDialog() {
	dialogs::LayerManagerDialog dialog(_viewport, this);
	dialog.exec();
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
	case QEvent::ShortcutOverride: {
		auto* keyEvent = static_cast<QKeyEvent*>(event);
		if (ControlBindings::instance().matches(keyEvent)) {
			event->accept();
			return true;
		}
		return QMainWindow::event(event);
	}
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
void FredView::changeEvent(QEvent* event) {
	QMainWindow::changeEvent(event);
	// Force menubar repaint when reenabled after a modal dialog closes.
	// Without this, menu items stay grey until the user mouses over them.
	if (event->type() == QEvent::EnabledChange && isEnabled()) {
		menuBar()->update();
	}
}
void FredView::closeEvent(QCloseEvent* event) {
	if (!maybePromptToSaveMissionChanges(tr("closing QtFRED"))) {
		event->ignore();
		return;
	}

	QMainWindow::closeEvent(event);
}
void FredView::windowActivated() {
	_viewport->Cursor_over = -1;

	// Track the last active viewport
	fred->setActiveViewport(_viewport);

	menuBar()->update();
	viewWindowActivated();
}
void FredView::windowDeactivated() {
	_viewport->Cursor_over = -1;
}
void FredView::on_actionLock_Marked_Objects_triggered(bool  /*enabled*/) {
	fred->lockMarkedObjects();
}
void FredView::on_actionUnlock_All_Objects_triggered(bool  /*enabled*/) {
	fred->unlockAllObjects();
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
void FredView::on_actionMission_Events_triggered(bool) {
	auto eventEditor = new dialogs::MissionEventsDialog(this, _viewport);
	eventEditor->setAttribute(Qt::WA_DeleteOnClose);
	eventEditor->show();
}
void FredView::on_actionMission_Cutscenes_triggered(bool)
{
	auto cutsceneEditor = new dialogs::MissionCutscenesDialog(this, _viewport);
	cutsceneEditor->setAttribute(Qt::WA_DeleteOnClose);
	cutsceneEditor->show();
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
void FredView::onUpdatePropClassBox() {
	if (_propClassBox) {
		if (_viewport->cur_prop_index < 0 && _propClassBox->count() > 0) {
			onPropClassSelected(_propClassBox->itemData(0).value<int>());
		}
		_propClassBox->selectPropClass(_viewport->cur_prop_index);
	}
}
void FredView::onShipClassSelected(int ship_class) {
	_viewport->cur_model_index = ship_class;
}
void FredView::onPropClassSelected(int prop_class) {
	_viewport->cur_prop_index = prop_class;
}
void FredView::on_actionAsteroid_Field_triggered(bool) {
	auto asteroidFieldEditor = new dialogs::AsteroidEditorDialog(this, _viewport);
	asteroidFieldEditor->setAttribute(Qt::WA_DeleteOnClose);
	asteroidFieldEditor->show();
}
void FredView::on_actionVolumetric_Nebula_triggered(bool)
{
	auto volumetricNebulaEditor = new dialogs::VolumetricNebulaDialog(this, _viewport);
	volumetricNebulaEditor->setAttribute(Qt::WA_DeleteOnClose);
	volumetricNebulaEditor->show();
}
void FredView::on_actionBriefing_triggered(bool) {
	auto eventEditor = new dialogs::BriefingEditorDialog(this, _viewport);
	eventEditor->setAttribute(Qt::WA_DeleteOnClose);
	eventEditor->show();
}
void FredView::on_actionMission_Specs_triggered(bool) {
	auto missionSpecEditor = new dialogs::MissionSpecDialog(this, _viewport);
	missionSpecEditor->setAttribute(Qt::WA_DeleteOnClose);
	missionSpecEditor->show();
}
void FredView::on_actionWaypoint_Paths_triggered(bool) {
	auto editorDialog = new dialogs::WaypointEditorDialog(this, _viewport);
	editorDialog->setAttribute(Qt::WA_DeleteOnClose);
	editorDialog->show();
}
void FredView::on_actionJump_Nodes_triggered(bool)
{
	auto editorDialog = new dialogs::JumpNodeEditorDialog(this, _viewport);
	editorDialog->setAttribute(Qt::WA_DeleteOnClose);
	editorDialog->show();
}
void FredView::on_actionShips_triggered(bool)
{
	if (!_shipEditorDialog) {
		_shipEditorDialog = new dialogs::ShipEditorDialog(this, _viewport);
		_shipEditorDialog->setAttribute(Qt::WA_DeleteOnClose);
		// When the user closes it, reset our pointer so we can open a new one later
		connect(_shipEditorDialog, &QObject::destroyed, this, [this]() {
			_shipEditorDialog = nullptr;
		});
		_shipEditorDialog->show();
	} else {
		_shipEditorDialog->raise();
		_shipEditorDialog->activateWindow();
	}

}
void FredView::on_actionWings_triggered(bool)
{
	if (!_wingEditorDialog) {
		_wingEditorDialog = new dialogs::WingEditorDialog(this, _viewport);
		_wingEditorDialog->setAttribute(Qt::WA_DeleteOnClose);
		// When the user closes it, reset our pointer so we can open a new one later
		connect(_wingEditorDialog, &QObject::destroyed, this, [this]() { _wingEditorDialog = nullptr; });
		_wingEditorDialog->show();
	} else {
		_wingEditorDialog->raise();
		_wingEditorDialog->activateWindow();
	}
}
void FredView::on_actionProps_triggered(bool)
{
	if (!_propEditorDialog) {
		_propEditorDialog = new dialogs::PropEditorDialog(this, _viewport);
		_propEditorDialog->setAttribute(Qt::WA_DeleteOnClose);
		connect(_propEditorDialog, &QObject::destroyed, this, [this]() { _propEditorDialog = nullptr; });
		_propEditorDialog->show();
	} else {
		_propEditorDialog->raise();
		_propEditorDialog->activateWindow();
	}
}
void FredView::on_actionCampaign_triggered(bool) {
	//TODO: Save if Changes
	auto editorCampaign = new dialogs::CampaignEditorDialog(this, _viewport);
	editorCampaign->setAttribute(Qt::WA_DeleteOnClose);
	editorCampaign->show();
}
void FredView::on_actionObjects_triggered(bool) {
	orientEditorTriggered();
}
void FredView::on_actionCommand_Briefing_triggered(bool) {
	auto editorDialog = new dialogs::CommandBriefingDialog(this, _viewport);
	editorDialog->setAttribute(Qt::WA_DeleteOnClose);
	editorDialog->show();
}
void FredView::on_actionDebriefing_triggered(bool)
{
	auto editorDialog = new dialogs::DebriefingDialog(this, _viewport);
	editorDialog->setAttribute(Qt::WA_DeleteOnClose);
	editorDialog->show();
}
void FredView::on_actionReinforcements_triggered(bool) {
	auto editorDialog = new dialogs::ReinforcementsDialog(this, _viewport);
	editorDialog->setAttribute(Qt::WA_DeleteOnClose);
	editorDialog->show();
}
void FredView::on_actionLoadout_triggered(bool) {
	auto editorDialog = new dialogs::TeamLoadoutDialog(this, _viewport);
	editorDialog->setAttribute(Qt::WA_DeleteOnClose);
	editorDialog->show();
}
void FredView::on_actionVariables_triggered(bool) {
	auto editorDialog = new dialogs::VariableDialog(this, _viewport);
	editorDialog->show();
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
			} else if (Objects[objNum].type == OBJ_PROP) {

				// Select the object before displaying the dialog
				fred->selectObject(objNum);

				on_actionProps_triggered(false);
			} else if (Objects[objNum].type == OBJ_JUMP_NODE || Objects[objNum].type == OBJ_WAYPOINT) {

			// Select the object before displaying the dialog
			fred->selectObject(objNum);

			// Use the existing slot for this to avoid duplicating code
			if (Objects[objNum].type == OBJ_JUMP_NODE) {
				on_actionJump_Nodes_triggered(false);
			} else if (Objects[objNum].type == OBJ_WAYPOINT) {
				// If this is a waypoint, we need to show the waypoint editor
				on_actionWaypoint_Paths_triggered(false);
			}
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
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	// This is a modal dialog
	dialog->exec();
}
void FredView::onUpdateEditorActions() {
	ui->actionObjects->setEnabled(query_valid_object(fred->currentObject));

	const bool validObject = query_valid_object(fred->currentObject);
	const bool hasMarked = fred->getNumMarked() > 0;
	const bool isShip = validObject && Objects[fred->currentObject].type == OBJ_SHIP;
	const bool subsysActive = fred->Render_subsys.do_render;

	ui->actionClone_Marked_Objects->setEnabled(hasMarked);
	ui->actionDelete->setEnabled(hasMarked);
	ui->actionLock_Marked_Objects->setEnabled(hasMarked);
	ui->actionDelete_Wing->setEnabled(fred->cur_wing >= 0);

	// Objects editor — requires a selected object
	ui->actionObjects->setEnabled(validObject);

	// Level/Align — require something to be selected
	ui->actionLevel_Object->setEnabled(validObject);
	ui->actionAlign_Object->setEnabled(validObject);

	// Mark Wing — only valid when the current object belongs to a wing
	ui->actionMark_Wing->setEnabled(fred->cur_wing != -1);

	// Subsystem navigation — Next requires a ship selected, Prev/Cancel require an active subsystem
	ui->actionNext_Subsystem->setEnabled(isShip);
	ui->actionPrev_Subsystem->setEnabled(subsysActive);
	ui->actionCancel_Subsystem->setEnabled(subsysActive);

	// Set Group submenu — requires at least one marked object to assign
	ui->menuSet_Group->setEnabled(hasMarked);
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
		fred->autosave("form wing");
	}
}
void FredView::on_actionWingDisband_triggered(bool  /*enabled*/) {
	if (fred->query_single_wing_marked()) {
		fred->remove_wing(fred->cur_wing);
		fred->autosave("wing disband");
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
	dialog->setAttribute(Qt::WA_DeleteOnClose);
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
void FredView::on_actionClone_Marked_Objects_triggered(bool) {
	if (fred->getNumMarked() > 0) {
		_viewport->duplicate_marked_objects();
		fred->autosave("clone marked");
	}
}
void FredView::on_actionDelete_triggered(bool) {
	if (fred->getNumMarked() > 0) {
		fred->delete_marked();
		fred->autosave("object delete");
	}
}
void FredView::on_actionDelete_Wing_triggered(bool) {
	if (fred->cur_wing >= 0) {
		fred->delete_wing(fred->cur_wing, 0);
		fred->autosave("wing delete");
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
void FredView::on_actionControl_Object_triggered(bool) {
	_viewport->Control_mode = (_viewport->Control_mode + 1) % 2;
}
void FredView::on_actionLevel_Object_triggered(bool) {
	_viewport->level_controlled();
}
void FredView::on_actionAlign_Object_triggered(bool) {
	_viewport->verticalize_controlled();
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
void FredView::on_actionNext_Object_triggered(bool) {
	fred->select_next_object();
}
void FredView::on_actionPrev_Object_triggered(bool) {
	fred->select_previous_object();
}
void FredView::on_actionMark_Wing_triggered(bool) {
	if (fred->cur_wing != -1) {
		fred->mark_wing(fred->cur_wing);
	}
}
void FredView::on_actionError_Checker_triggered(bool) {
	fred->global_error_check();
}
void FredView::on_actionAbout_triggered(bool) {
	auto dialog = new dialogs::AboutDialog(this, _viewport);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}
void FredView::on_actionMission_Statistics_triggered(bool) {
	auto dialog = new dialogs::MissionStatsDialog(this, _viewport);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}

void FredView::on_actionBackground_triggered(bool) {
	auto dialog = new dialogs::BackgroundEditorDialog(this, _viewport);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}

void FredView::on_actionShield_System_triggered(bool) {
	auto dialog = new dialogs::ShieldSystemDialog(this, _viewport);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}

void FredView::on_actionSet_Global_Ship_Flags_triggered(bool) {
	auto dialog = new dialogs::GlobalShipFlagsDialog(this, _viewport);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}

void FredView::on_actionVoice_Acting_Manager_triggered(bool) {
	auto dialog = new dialogs::VoiceActingManager(this, _viewport);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}
void FredView::on_actionMission_Goals_triggered(bool) {
	auto dialog = new dialogs::MissionGoalsDialog(this, _viewport);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}

void FredView::on_actionMusic_Player_triggered(bool)
{
	auto dialog = new dialogs::MusicPlayerDialog(this, _viewport);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}

void FredView::on_actionCalculate_Relative_Coordinates_triggered(bool) {
	auto dialog = new dialogs::RelativeCoordinatesDialog(this, _viewport);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}

void FredView::on_actionFiction_Viewer_triggered(bool) {
	auto dialog = new dialogs::FictionViewerDialog(this, _viewport);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->show();
}

} // namespace fred
} // namespace fso
