
#include "fredGlobals.h"

#include <globalincs/pstypes.h>

namespace fso {
namespace fred {

QtFredGlobals* fredGlobals = nullptr;

QtFredGlobals::QtFredGlobals() {
	Assertion(fredGlobals == nullptr, "Only one instances of QtFredGlobals may be instantiated!");

	connect(this, &QtFredGlobals::initializeComplete, [this]() { _initializeEmitted = true; });

	fredGlobals = this;
}
QtFredGlobals::~QtFredGlobals() {
	fredGlobals = nullptr;
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
