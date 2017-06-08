
#include <QtCore/QAbstractEventDispatcher>
#include <QDebug>
#include <QTimer>

#include <mission/editor.h>

#include "globalincs/pstypes.h"

#ifdef WIN32
#include "globalincs/mspdb_callstack.h"
#endif

#include "FredApplication.h"

namespace fso {
namespace fred {

FredApplication* fredApp = nullptr;

FredApplication::FredApplication() {
	Assertion(fredApp == nullptr, "Only one instances of FredApplication may be instantiated!");

	connect(this, &FredApplication::initializeComplete, [this]() { _initializeEmitted = true; });

	// Put our shutdown code into a slot connected to the quit signal. That's the recommended way of doing cleanup
	connect(qGuiApp, &QCoreApplication::aboutToQuit, this, &FredApplication::shutdown);

	_elapsedTimer.start();

	auto idleTimer = new QTimer(this);
	// This will call our function whenever the processing loop is idle, allowing us to do regular operations
	connect(idleTimer, &QTimer::timeout, this, &FredApplication::idleFunction);
	idleTimer->start(0);

	fredApp = this;
}
FredApplication::~FredApplication() {
	fredApp = nullptr;
}

bool FredApplication::isInitializeComplete() const {
	return _initializeEmitted;
}

void FredApplication::runAfterInit(std::function<void()>&& action) {
	if (_initializeEmitted) {
		action();
	} else {
		connect(this, &FredApplication::initializeComplete, action);
	}
}

void FredApplication::shutdown() {
	// Clean up resources after we are done
	fso::fred::shutdown();

#ifdef WIN32
	SCP_mspdbcs_Cleanup();
#endif
}

void FredApplication::idleFunction() {
	// We limit the number of idle events per second to reduce the CPU usage
	if (_elapsedTimer.elapsed() > 5) {
		// emit the public signal
		onIdle();

		_elapsedTimer.restart();
	}
}
}
}
