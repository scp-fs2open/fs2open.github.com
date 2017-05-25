#pragma once
#include <memory>
#include <QMainWindow>

namespace fso {
namespace fred {

class Editor;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setEditor(std::shared_ptr<Editor>);

public slots:
    void loadMission();

private slots:
    void on_actionShow_Stars_triggered(bool checked);

    void on_actionShow_Horizon_triggered(bool checked);

    void on_actionShow_Grid_triggered(bool checked);

    void on_actionShow_Distances_triggered(bool checked);

    void on_actionShow_Coordinates_triggered(bool checked);

    void on_actionShow_Outlines_triggered(bool checked);

private:
    Ui::MainWindow *ui;
    std::shared_ptr<Editor> fred;
};


} // namespace fred
} // namespace fso
