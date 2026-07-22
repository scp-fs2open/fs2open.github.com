#pragma once

#include <QMenu>
#include <QString>

#include <functional>
#include <vector>

class QWidget;

namespace fso {
namespace fred {
namespace util {

int propagate_disabled_status(QMenu* top);

struct SelectMenuEntry {
	QString name;
	int id;
};

// Adds a "Select" menu to an object-editor dialog's menu bar, creating a slim
// menu bar via the dialog's top-level layout if one isn't already present and
// reusing an existing bar otherwise.
void installSelectMenu(QWidget* dialog,
	std::function<std::vector<SelectMenuEntry>()> gather,
	std::function<int()> currentId,
	std::function<void(int)> onChosen,
	const QString& menuTitle);

}
}
}
