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
	~FredApplication();

	bool isInitializeComplete() const;

	void runAfterInit(std::function<void()>&& action);
 signals:
	void initializeComplete();

	void onIdle();

private slots:
	void shutdown();

	void idleFunction();
};

extern FredApplication* fredApp;

}
}
