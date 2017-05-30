#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

#include <project.h>

#include <qevent.h>

#include "mission/editor.h"
#include "renderwidget.h"

namespace fso {
namespace fred {

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow())
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setEditor(Editor* editor)
{
	Assertion(fred == nullptr, "Resetting the editor is currently not supported!");

    fred = editor;
    ui->centralwidget->getWindow()->setEditor(editor);

	connect(fred, &Editor::missionLoaded, this, &MainWindow::on_mission_loaded);

	// Sets the initial window title
	on_mission_loaded("");
}

void MainWindow::loadMission()
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

        ui->centralwidget->getWindow()->updateGL();
        statusBar()->showMessage(tr("Units = %1 meters").arg(fred->renderer()->The_grid->square_size));
    }
    catch (const fso::fred::mission_load_error &) {
		QApplication::restoreOverrideCursor();

        QMessageBox::critical(this, tr("Failed loading mission."), tr("Could not parse the mission."));
        statusBar()->clearMessage();
    }
}

void MainWindow::on_actionShow_Stars_triggered(bool checked)
{
	fred->renderer()->view.Show_stars = checked;
}

void MainWindow::on_actionShow_Horizon_triggered(bool checked)
{
	fred->renderer()->view.Show_horizon = checked;
}

void MainWindow::on_actionShow_Grid_triggered(bool checked)
{
	fred->renderer()->view.Show_grid = checked;
}

void MainWindow::on_actionShow_Distances_triggered(bool checked)
{
	fred->renderer()->view.Show_distances = checked;
}

void MainWindow::on_actionShow_Coordinates_triggered(bool checked)
{
	fred->renderer()->view.Show_coordinates = checked;
}

void MainWindow::on_actionShow_Outlines_triggered(bool checked)
{
	fred->renderer()->view.Show_outlines = checked;
}

void MainWindow::on_mission_loaded(const std::string& filepath) {
	QString filename = "Untitled";
	if (!filepath.empty()) {
		filename = QFileInfo(QString::fromStdString(filepath)).fileName();
	}

	auto title = QString("%1 - qtFRED v%2 - FreeSpace 2 Mission Editor").arg(filename, FS_VERSION_FULL);

	setWindowTitle(title);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
	// Forward all uncaught keyboard events to the render window so we don't have to focus it explicitly.
	qGuiApp->sendEvent(getRenderWindow(), event);
}

void MainWindow::keyReleaseEvent(QKeyEvent* event) {
	qGuiApp->sendEvent(getRenderWindow(), event);
}

RenderWindow* MainWindow::getRenderWindow() {
	return ui->centralwidget->getWindow();
}

} // namespace fred
} // namespace fso

