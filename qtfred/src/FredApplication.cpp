
#include <QtCore/QAbstractEventDispatcher>
#include <QDebug>
#include <QTimer>
#include <QtGui/QtEvents>

#include <mission/editor.h>
#include <mission/management.h>

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

	// This will call our function in regular increments which allows us to do mission simulation stuff
	auto idleTimer = new QTimer(this);
	connect(idleTimer, &QTimer::timeout, this, &FredApplication::idleFunction);
	idleTimer->start(5);

	fredApp = this;

	// When there are issues with event handling, enable this define to see the events in the log and stdout
#ifdef EVENT_DEBUGGING
	qGuiApp->installEventFilter(this);
#endif
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
	// emit the public signal
	onIdle();
}
bool FredApplication::eventFilter(QObject* watched, QEvent* event) {
	// Skip timer events since there are a lot of those
	if (event->type() != QEvent::Timer) {
		qDebug() << watched << " " << event;
	}
	return QObject::eventFilter(watched, event);
}
}
}
