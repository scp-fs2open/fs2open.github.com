#pragma once

#include <QMainWindow>

#include <QtGui/QSurfaceFormat>

#include <memory>
#include <QtWidgets/QComboBox>
#include <mission/FredRenderer.h>

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
    void setEditor(Editor* editor, FredRenderer* renderer);

	void loadMissionFile(const QString& pathName);

	RenderWindow* getRenderWindow();

public slots:
    void openLoadMissionDIalog();

	void newMission();

private slots:
	void on_actionExit_triggered(bool);


	void on_actionShow_Ships_triggered(bool checked);

	void on_actionShow_Player_Starts_triggered(bool checked);

	void on_actionShow_Waypoints_triggered(bool checked);

	void on_actionShow_Ship_Models_triggered(bool checked);

	void on_actionShow_Outlines_triggered(bool checked);

	void on_actionShow_Ship_Info_triggered(bool checked);

	void on_actionShow_Coordinates_triggered(bool checked);

	void on_actionShow_Grid_Positions_triggered(bool checked);

	void on_actionShow_Distances_triggered(bool checked);

	void on_actionShow_Model_Paths_triggered(bool checked);

	void on_actionShow_Model_Dock_Points_triggered(bool checked);

	void on_actionShow_Grid_triggered(bool checked);

	void on_actionShow_Horizon_triggered(bool checked);

	void on_actionDouble_Fine_Gridlines_triggered(bool checked);

	void on_actionAnti_Aliased_Gridlines_triggered(bool checked);

	void on_actionShow_3D_Compass_triggered(bool checked);

    void on_actionShow_Background_triggered(bool checked);

	void on_actionLighting_from_Suns_triggered(bool checked);

protected:
	void keyPressEvent(QKeyEvent* event) override;

	void keyReleaseEvent(QKeyEvent* event) override;

	bool event(QEvent* event) override;

private:
	void on_mission_loaded(const std::string& filepath);

	template<typename T>
	void handleViewSettingsUpdate(T* viewVariable, bool enabled) {
		*viewVariable = enabled;

		// View settings have changed so we need to update the window
		_renderer->scheduleUpdate();
	}

	void addToRecentFiles(const QString& path);
	void updateRecentFileList();

	void recentFileOpened();

	/**
	 * @brief Synchronize the view options in the renderer and the state of the view check boxes in the menu
	 */
	void syncViewOptions();

	std::unique_ptr<Ui::FredView> ui;

	std::unique_ptr<QComboBox> _shipClassBox;

    Editor* fred = nullptr;
	FredRenderer* _renderer = nullptr;
};


} // namespace fred
} // namespace fso
