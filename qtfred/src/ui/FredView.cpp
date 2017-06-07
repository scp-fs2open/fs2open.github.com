#include "FredView.h"
#include "ui_FredView.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QSettings>

#include <project.h>

#include <qevent.h>

#include "mission/editor.h"

#include "widgets/ColorComboBox.h"

namespace fso {
namespace fred {

FredView::FredView(QWidget* parent) : QMainWindow(parent), ui(new Ui::FredView()) {
	ui->setupUi(this);

	// This is not possible to do with the designer
	ui->actionNew->setShortcuts(QKeySequence::New);
	ui->actionOpen->setShortcuts(QKeySequence::Open);
	ui->actionSave->setShortcuts(QKeySequence::Save);
	ui->actionSave_As->setShortcuts(QKeySequence::SaveAs);
	ui->actionExit->setShortcuts(QKeySequence::Quit);
	ui->actionUndo->setShortcuts(QKeySequence::Undo);
	ui->actionDelete->setShortcuts(QKeySequence::Delete);

	// A combo box cannot be added by the designer so we do that manually here
	_shipClassBox.reset(new ColorComboBox());

	ui->toolBar->addWidget(_shipClassBox.get());

	connect(ui->actionOpen, &QAction::triggered, this, &FredView::openLoadMissionDIalog);
	connect(ui->actionNew, &QAction::triggered, this, &FredView::newMission);

	updateRecentFileList();

	installEventFilter(this);
}

FredView::~FredView() {
}

void FredView::setEditor(Editor* editor, FredRenderer* renderer) {
	Assertion(fred == nullptr, "Resetting the editor is currently not supported!");
	Assertion(_renderer == nullptr, "Resetting the renderer is currently not supported!");

	fred = editor;
	_renderer = renderer;

	getRenderWindow()->setEditor(editor, renderer);

	connect(fred, &Editor::missionLoaded, this, &FredView::on_mission_loaded);

	// Sets the initial window title
	on_mission_loaded("");
}

void FredView::loadMissionFile(const QString& pathName) {
	statusBar()->showMessage(tr("Loading mission %1").arg(pathName));
	try {
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

		fred->loadMission(pathName.toStdString());

		QApplication::restoreOverrideCursor();

		getRenderWindow()->updateGL();
		statusBar()->showMessage(tr("Units = %1 meters").arg(_renderer->The_grid->square_size));
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

void FredView::on_actionShow_Background_triggered(bool checked) {
	_renderer->view.Show_stars = checked;

	// View has changed so we need to schedule an update
	_renderer->scheduleUpdate();
}

void FredView::on_actionShow_Horizon_triggered(bool checked) {
	_renderer->view.Show_horizon = checked;

	// View has changed so we need to schedule an update
	_renderer->scheduleUpdate();
}

void FredView::on_actionShow_Grid_triggered(bool checked) {
	_renderer->view.Show_grid = checked;

	// View has changed so we need to schedule an update
	_renderer->scheduleUpdate();
}

void FredView::on_actionShow_Distances_triggered(bool checked) {
	_renderer->view.Show_distances = checked;

	// View has changed so we need to schedule an update
	_renderer->scheduleUpdate();
}

void FredView::on_actionShow_Coordinates_triggered(bool checked) {
	_renderer->view.Show_coordinates = checked;

	// View has changed so we need to schedule an update
	_renderer->scheduleUpdate();
}

void FredView::on_actionShow_Outlines_triggered(bool checked) {
	_renderer->view.Show_outlines = checked;

	// View has changed so we need to schedule an update
	_renderer->scheduleUpdate();
}

void FredView::on_actionExit_triggered(bool) {
	close();
}

void FredView::on_mission_loaded(const std::string& filepath) {
	QString filename = "Untitled";
	if (!filepath.empty()) {
		filename = QFileInfo(QString::fromStdString(filepath)).fileName();
	}

	auto title = QString("%1 - qtFRED v%2 - FreeSpace 2 Mission Editor").arg(filename, FS_VERSION_FULL);

	setWindowTitle(title);

	if (!filepath.empty()) {
		addToRecentFiles(QString::fromStdString(filepath));
	}
}

void FredView::keyPressEvent(QKeyEvent* event) {
	// Forward all uncaught keyboard events to the render window so we don't have to focus it explicitly.
	qGuiApp->sendEvent(getRenderWindow(), event);
}

void FredView::keyReleaseEvent(QKeyEvent* event) {
	qGuiApp->sendEvent(getRenderWindow(), event);
}

RenderWindow* FredView::getRenderWindow() {
	return ui->centralWidget->getWindow();
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
	return QObject::eventFilter(watched, event);
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
} // namespace fred
} // namespace fso

