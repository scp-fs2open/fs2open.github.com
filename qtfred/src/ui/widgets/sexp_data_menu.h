#pragma once

#include "mission/EditorViewport.h"

#include <QString>
#include <functional>
#include <vector>

class QMenu;

namespace fso::fred {

struct SexpDataMenuItem {
	QString text;
	int dataIdx;
};

// Populates the OPF data list portion of an "Add Data" / "Replace Data" submenu
// using the chosen style. Pre-existing items in `menu` (Number, String,
// separator) are left intact; the data list is appended after them.
//
// onActivate is invoked with the chosen item's dataIdx when the user selects
// an entry. Callers are responsible for resolving that index against the
// original sexp_list_item linked list.
void populateSexpDataSubmenu(QMenu* menu,
	const std::vector<SexpDataMenuItem>& items,
	SexpDataMenuStyle style,
	std::function<void(int)> onActivate);

} // namespace fso::fred
