#include "ShipTBLViewerModel.h"

#include <ship/ship.h>
#include <utils/table_viewer.h>

namespace fso {
namespace fred {
namespace dialogs {
ShipTBLViewerModel::ShipTBLViewerModel(QObject* parent, EditorViewport* viewport, int sc)
	: AbstractDialogModel(parent, viewport)
{
	initializeData(sc);
}
bool ShipTBLViewerModel::apply()
{
	return true;
}
void ShipTBLViewerModel::reject()
{
}
void ShipTBLViewerModel::initializeData(const int ship_class)
{
	const ship_info* sip = &Ship_info[ship_class];

	text.clear();

	if (!sip) {
		return;
	}

	text = table_viewer::get_table_entry_text("ships.tbl", "*-shp.tbm", sip->name);
	modelChanged();
}
SCP_string ShipTBLViewerModel::getText() const
{
	return text;
}
} // namespace dialogs
} // namespace fred
} // namespace fso
