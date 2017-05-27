
#include "fredGlobals.h"

namespace fso {
namespace fred {

QtFredGlobals QtFredGlobals::instance;

QtFredGlobals* fredGlobals = &QtFredGlobals::getInstance();

QtFredGlobals::QtFredGlobals() {
}
void QtFredGlobals::qtInit() {
	connect(this, &QtFredGlobals::initializeComplete, [this]() { _initializeEmitted = true; });
}
void QtFredGlobals::runAfterInit(std::function<void()>&& action) {
	if (_initializeEmitted) {
		action();
	} else {
		connect(this, &QtFredGlobals::initializeComplete, action);
	}
}

}
}
