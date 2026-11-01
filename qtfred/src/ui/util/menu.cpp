//
//

#include "menu.h"

#include <ui/widgets/data_list_menu.h>

#include <QAction>
#include <QLayout>
#include <QMenuBar>
#include <QWidget>

#include <utility>

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

void installSelectMenu(QWidget* dialog,
	EditorViewport* viewport,
	std::function<std::vector<SelectMenuEntry>()> gather,
	std::function<int()> currentId,
	std::function<void(int)> onChosen,
	const QString& menuTitle)
{
	auto* layout = dialog->layout();
	if (layout == nullptr) {
		return; // needs a top-level layout to host a menu bar
	}

	// Reuse an existing menu bar if one is already installed, otherwise add a
	// slim one above the dialog's content.
	auto* menuBar = qobject_cast<QMenuBar*>(layout->menuBar());
	if (menuBar == nullptr) {
		menuBar = new QMenuBar(dialog);
		layout->setMenuBar(menuBar);
	}

	QMenu* menu = menuBar->addMenu(menuTitle);

	// Rebuild the list from the live scene every time the menu opens.
	QObject::connect(menu, &QMenu::aboutToShow, menu,
		[menu, viewport, gather = std::move(gather), currentId = std::move(currentId), onChosen = std::move(onChosen)]() {
			menu->clear();
			const int current = currentId ? currentId() : -1;
			const DataMenuStyle style = viewport ? viewport->Data_menu_style : DataMenuStyle::Columns;
			populateDataListMenu(menu, gather(), style, onChosen, 0, current);
		});
}

}
}
}
