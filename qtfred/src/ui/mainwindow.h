#pragma once

#include <QMainWindow>

#include <QtGui/QSurfaceFormat>

#include <memory>

namespace fso {
namespace fred {

class Editor;
class RenderWindow;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setEditor(Editor* editor);

	RenderWindow* getRenderWidget();

public slots:
    void loadMission();

private slots:
    void on_actionShow_Stars_triggered(bool checked);

    void on_actionShow_Horizon_triggered(bool checked);

    void on_actionShow_Grid_triggered(bool checked);

    void on_actionShow_Distances_triggered(bool checked);

    void on_actionShow_Coordinates_triggered(bool checked);

    void on_actionShow_Outlines_triggered(bool checked);

	void on_mission_loaded(const std::string& filepath);

private:
    std::unique_ptr<Ui::MainWindow> ui;
    Editor* fred = nullptr;
};


} // namespace fred
} // namespace fso
