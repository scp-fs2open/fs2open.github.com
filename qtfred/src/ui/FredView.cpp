
#include "FredView.h"
#include "ui_FredView.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

#include <project.h>

#include <qevent.h>

#include "mission/editor.h"

#include "widgets/ColorComboBox.h"

namespace fso {
namespace fred {

FredView::FredView(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FredView())
{
    ui->setupUi(this);

	// A combo box cannot be added by the designer so we do that manually here
	_shipClassBox.reset(new ColorComboBox());

	ui->toolBar->addWidget(_shipClassBox.get());

	connect(ui->actionOpen, &QAction::triggered, this, &FredView::loadMission);
	connect(ui->actionNew, &QAction::triggered, this, &FredView::newMission);
}

FredView::~FredView()
{
}

void FredView::setEditor(Editor* editor, FredRenderer* renderer)
{
	Assertion(fred == nullptr, "Resetting the editor is currently not supported!");
	Assertion(_renderer == nullptr, "Resetting the renderer is currently not supported!");

    fred = editor;
	_renderer = renderer;

    getRenderWindow()->setEditor(editor, renderer);

	connect(fred, &Editor::missionLoaded, this, &FredView::on_mission_loaded);

	// Sets the initial window title
	on_mission_loaded("");
}

void FredView::loadMission()
{
    qDebug() << "Loading from directory:" << QDir::currentPath();
    QString pathName = QFileDialog::getOpenFileName(this,
                                                    tr("Load mission"),
                                                    QString(),
                                                    tr("FS2 missions (*.fs2)"));

    if (pathName.isEmpty())
        return;

    statusBar()->showMessage(tr("Loading mission %1").arg(pathName));
    try {
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        fred->loadMission(pathName.toStdString());

		QApplication::restoreOverrideCursor();

		getRenderWindow()->updateGL();
        statusBar()->showMessage(tr("Units = %1 meters").arg(_renderer->The_grid->square_size));
    }
    catch (const fso::fred::mission_load_error &) {
		QApplication::restoreOverrideCursor();

        QMessageBox::critical(this, tr("Failed loading mission."), tr("Could not parse the mission."));
        statusBar()->clearMessage();
    }
}

void FredView::on_actionShow_Background_triggered(bool checked)
{
	_renderer->view.Show_stars = checked;

	// View has changed so we need to schedule an update
	_renderer->scheduleUpdate();
}

void FredView::on_actionShow_Horizon_triggered(bool checked)
{
	_renderer->view.Show_horizon = checked;

	// View has changed so we need to schedule an update
	_renderer->scheduleUpdate();
}

void FredView::on_actionShow_Grid_triggered(bool checked)
{
	_renderer->view.Show_grid = checked;

	// View has changed so we need to schedule an update
	_renderer->scheduleUpdate();
}

void FredView::on_actionShow_Distances_triggered(bool checked)
{
	_renderer->view.Show_distances = checked;

	// View has changed so we need to schedule an update
	_renderer->scheduleUpdate();
}

void FredView::on_actionShow_Coordinates_triggered(bool checked)
{
	_renderer->view.Show_coordinates = checked;

	// View has changed so we need to schedule an update
	_renderer->scheduleUpdate();
}

void FredView::on_actionShow_Outlines_triggered(bool checked)
{
	_renderer->view.Show_outlines = checked;

	// View has changed so we need to schedule an update
	_renderer->scheduleUpdate();
}

void FredView::on_mission_loaded(const std::string& filepath) {
	QString filename = "Untitled";
	if (!filepath.empty()) {
		filename = QFileInfo(QString::fromStdString(filepath)).fileName();
	}

	auto title = QString("%1 - qtFRED v%2 - FreeSpace 2 Mission Editor").arg(filename, FS_VERSION_FULL);

	setWindowTitle(title);
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

} // namespace fred
} // namespace fso

