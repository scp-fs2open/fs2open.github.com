#pragma once

#include <QMainWindow>
#include <QAction>

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
protected:
	void keyPressEvent(QKeyEvent* event) override;

	void keyReleaseEvent(QKeyEvent* event) override;

	bool event(QEvent* event) override;

private:
	void on_mission_loaded(const std::string& filepath);

	template<typename T>
	void connectActionToViewSetting(QAction* option, T* destination) {
		Q_ASSERT(option->isCheckable());

		// First copy the existing value to the action
		// Double negate to promote an integer to a real boolean
		option->setChecked(!!(*destination));

		// then connect the signal to a handler for updating the view setting
		// The pointer should be valid as long as this signal is active since it should be pointing inside the renderer (I hope...)
		connect(option, &QAction::triggered, [this,destination](bool value) {
			*destination = value;

			// View settings have changed so we need to update the window
			_renderer->scheduleUpdate();
		});
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
