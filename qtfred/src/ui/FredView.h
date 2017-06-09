#pragma once

#include <QMainWindow>
#include <QAction>
#include <QtGui/QSurfaceFormat>
#include <QtWidgets/QLabel>
#include <QtWidgets/QComboBox>
#include <QtGui/QSurface>

#include <mission/FredRenderer.h>

#include <memory>

namespace fso {
namespace fred {

class Editor;
class RenderWidget;

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

	QSurface* getRenderSurface();
	RenderWidget* getRenderWidget();

	void showContextMenu(const QPoint& globalPos);

public slots:
    void openLoadMissionDIalog();

	void newMission();

private slots:
	void on_actionExit_triggered(bool);

	void on_actionConstrainX_triggered(bool enabled);
	void on_actionConstrainXY_triggered(bool enabled);
	void on_actionConstrainY_triggered(bool enabled);
	void on_actionConstrainZ_triggered(bool enabled);
	void on_actionConstrainXZ_triggered(bool enabled);
	void on_actionConstrainYZ_triggered(bool enabled);

	void on_actionSelect_triggered(bool enabled);
	void on_actionSelectMove_triggered(bool enabled);
	void on_actionSelectRotate_triggered(bool enabled);

	void on_actionHide_Marked_Objects_triggered(bool enabled);
	void on_actionShow_All_Hidden_Objects_triggered(bool enabled);

	void on_actionx1_triggered(bool enabled);
	void on_actionx2_triggered(bool enabled);
	void on_actionx3_triggered(bool enabled);
	void on_actionx5_triggered(bool enabled);
	void on_actionx8_triggered(bool enabled);
	void on_actionx10_triggered(bool enabled);
	void on_actionx50_triggered(bool enabled);
	void on_actionx100_triggered(bool enabled);

	void on_actionRotx1_triggered(bool enabled);
	void on_actionRotx5_triggered(bool enabled);
	void on_actionRotx12_triggered(bool enabled);
	void on_actionRotx25_triggered(bool enabled);
	void on_actionRotx50_triggered(bool enabled);

	void on_actionCamera_triggered(bool enabled);
	void on_actionCurrent_Ship_triggered(bool enabled);
signals:
	/**
	 * @brief Special version of FredApplication::onIdle which is limited to the lifetime of this object
	 */
	void viewIdle();

protected:
	bool event(QEvent* event) override;

 private:
	void on_mission_loaded(const std::string& filepath);

	void connectActionToViewSetting(QAction* option, bool* destination);

	void addToRecentFiles(const QString& path);
	void updateRecentFileList();

	void recentFileOpened();

	/**
	 * @brief Synchronize the view options in the renderer and the state of the view check boxes in the menu
	 */
	void syncViewOptions();
	void updateUI();

	void initializeStatusBar();
	void initializePopupMenus();

	QLabel* _statusBarUnitsLabel = nullptr;

	QMenu* _viewPopup = nullptr;

	std::unique_ptr<Ui::FredView> ui;

	std::unique_ptr<QComboBox> _shipClassBox;

    Editor* fred = nullptr;
	FredRenderer* _renderer = nullptr;

	void onUpdateConstrains();
	void onUpdateEditingMode();
	void onUpdateViewSpeeds();
	void onUpdateViewpointMenu();

	void windowActivated();
	void windowDeactivated();
};


} // namespace fred
} // namespace fso
