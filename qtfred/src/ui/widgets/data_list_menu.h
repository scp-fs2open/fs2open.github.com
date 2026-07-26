#pragma once

#include "mission/EditorViewport.h"

#include <ui/util/menu.h>

#include <QString>
#include <functional>
#include <vector>

class QMenu;

namespace fso::fred {

// Populates a menu with a flat list of name/id entries using the chosen style.
// Pre-existing items in `menu` (e.g. the sexp Number/String entries) are left
// intact; the list is appended after them. `fixedRows` is the count of those
// pre-existing rows, used when estimating whether an Auto-style menu would
// outgrow the screen. If `currentId` matches an entry's id, that entry is
// bolded and pre-highlighted.
//
// onActivate is invoked with the chosen entry's id when the user selects one.
void populateDataListMenu(QMenu* menu,
	const std::vector<util::SelectMenuEntry>& items,
	DataMenuStyle style,
	std::function<void(int)> onActivate,
	int fixedRows = 0,
	int currentId = -1);

} // namespace fso::fred
