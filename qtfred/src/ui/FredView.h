#pragma once

#include <QMainWindow>

#include <QtGui/QSurfaceFormat>

#include <memory>

namespace fso {
namespace fred {

class Editor;
class RenderWindow;

namespace Ui {
class FredView;
}

class FredView : public QMainWindow
{
    Q_OBJECT

public:
    explicit FredView(QWidget *parent = 0);
    ~FredView();
    void setEditor(Editor* editor);

	RenderWindow* getRenderWindow();

public slots:
    void loadMission();

private slots:
    void on_actionShow_Background_triggered(bool checked);

    void on_actionShow_Horizon_triggered(bool checked);

    void on_actionShow_Grid_triggered(bool checked);

    void on_actionShow_Distances_triggered(bool checked);

    void on_actionShow_Coordinates_triggered(bool checked);

    void on_actionShow_Outlines_triggered(bool checked);

	void on_mission_loaded(const std::string& filepath);

protected:
	void keyPressEvent(QKeyEvent* event) override;

	void keyReleaseEvent(QKeyEvent* event) override;

private:
    std::unique_ptr<Ui::FredView> ui;
    Editor* fred = nullptr;
};


} // namespace fred
} // namespace fso
