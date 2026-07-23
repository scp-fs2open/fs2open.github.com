#pragma once

#include <QMenu>
#include <QString>

#include <functional>
#include <vector>

class QWidget;

namespace fso {
namespace fred {

class EditorViewport;

namespace util {

int propagate_disabled_status(QMenu* top);

struct SelectMenuEntry {
	QString name;
	int id;
};

// Adds a "Select" menu to an object-editor dialog's menu bar, creating a slim
// menu bar via the dialog's top-level layout if one isn't already present and
// reusing an existing bar otherwise. The list is presented with the viewport's
// preferred data menu style (columns vs. searchable).
void installSelectMenu(QWidget* dialog,
	EditorViewport* viewport,
	std::function<std::vector<SelectMenuEntry>()> gather,
	std::function<int()> currentId,
	std::function<void(int)> onChosen,
	const QString& menuTitle);

}
}
}
