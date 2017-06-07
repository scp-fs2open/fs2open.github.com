#pragma once

#include <QMainWindow>
#include <QAction>

#include <QtGui/QSurfaceFormat>
#include <QtWidgets/QLabel>

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

	void showContextMenu(const QPoint& globalPos);

public slots:
    void openLoadMissionDIalog();

	void newMission();

private slots:
	void on_actionExit_triggered(bool);

signals:
	/**
	 * @brief Special version of FredApplication::onIdle which is limited to the lifetime of this object
	 */
	void viewIdle();

protected:
	void keyPressEvent(QKeyEvent* event) override;

	void keyReleaseEvent(QKeyEvent* event) override;

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
};


} // namespace fred
} // namespace fso
