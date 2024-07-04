#include "SignalBlockers.h"

#include <QWidget>

namespace fso {
namespace fred {
namespace util {

SignalBlockers::SignalBlockers(QObject* parent) {
	_blockers.emplace_back(parent);
	for (QWidget* widget : parent->findChildren<QWidget*>()) {
		_blockers.emplace_back(widget);
	}
}
}
}
}
