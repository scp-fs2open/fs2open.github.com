//
//

#include "menu.h"


namespace fso {
namespace fred {
namespace util {

int propagate_disabled_status(QMenu* top) {
	auto count = 0;

	for (auto& action : top->actions()) {
		if (action->menu() != nullptr) {
			if (propagate_disabled_status(action->menu()) != 0) {
				++count;
			} else {
				action->setEnabled(false);
			}
		} else {
			if (!action->isSeparator() && action->isEnabled()) {
				++count;
			}
		}
	}

	return count;
}

}
}
}
