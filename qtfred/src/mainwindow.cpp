#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

#include "editor.h"

namespace fso {
namespace fred {

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setEditor(std::shared_ptr<Editor> editor)
{
    fred = editor;
    ui->centralwidget->setEditor(editor);
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
        fred->loadMission(pathName.toStdString());
        ui->centralwidget->updateGL();
        statusBar()->showMessage(tr("Units = %1 meters").arg(fred->renderer()->The_grid->square_size));
    }
    catch (const fso::fred::mission_load_error &) {
        QMessageBox::critical(this, tr("Failed loading mission."), tr("Could not parse the mission."));
        statusBar()->clearMessage();
    }
}

} // namespace fred
} // namespace fso

void fso::fred::MainWindow::on_actionShow_Stars_triggered(bool checked)
{
    fred->renderer()->view.Show_stars = checked;
}

void fso::fred::MainWindow::on_actionShow_Horizon_triggered(bool checked)
{
    fred->renderer()->view.Show_horizon = checked;
}

void fso::fred::MainWindow::on_actionShow_Grid_triggered(bool checked)
{
    fred->renderer()->view.Show_grid = checked;
}

void fso::fred::MainWindow::on_actionShow_Distances_triggered(bool checked)
{
    fred->renderer()->view.Show_distances = checked;
}

void fso::fred::MainWindow::on_actionShow_Coordinates_triggered(bool checked)
{
    fred->renderer()->view.Show_coordinates = checked;
}

void fso::fred::MainWindow::on_actionShow_Outlines_triggered(bool checked)
{
    fred->renderer()->view.Show_outlines = checked;
}
