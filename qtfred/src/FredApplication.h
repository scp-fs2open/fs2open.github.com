#pragma once

#include <QGuiApplication>

#include <functional>
#include <QtCore/QElapsedTimer>

namespace fso {
namespace fred {

class FredApplication: public QObject {
 Q_OBJECT
 private:
	bool _initializeEmitted = false;

 public:
	FredApplication();
	~FredApplication() override;

	bool isInitializeComplete() const;

	/**
	 * @brief Runs the action after all game data has been loaded
	 *
	 * This should be used for actions that require that all game data has been loaded. If initialization is already
	 * done then this function will run the action immediately.
	 *
	 * @param action The function to execute
	 */
	void runAfterInit(std::function<void()>&& action);

	bool eventFilter(QObject* watched, QEvent* event) override;
 signals:
	void initializeComplete();

	void onIdle();

private slots:
	void shutdown();

	void lateShutdown();

	void idleFunction();
};

extern FredApplication* fredApp;

}
}
