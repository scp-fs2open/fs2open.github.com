//
//

#if defined(_MSC_VER) && _MSC_VER <= 1920
	// work around MSVC 2015 and 2017 compiler bug
	// https://bugreports.qt.io/browse/QTBUG-72073
	#define QT_NO_FLOAT16_OPERATORS
#endif

#include "ColorComboBox.h"

#include <ship/ship.h>

#include <QtGui/QtGui>

#include <FredApplication.h>

namespace fso {
namespace fred {

ColorComboBox::ColorComboBox(QWidget* parent, EditorViewport* viewport) : QComboBox(parent), _viewport(viewport) {
	// This needs to be done after init since the ship classes aren't initialized yet
	fredApp->runAfterInit([this]() {
		setModel(getShipClassModel());
	});

	connect(this,
			static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this,
			&ColorComboBox::indexChanged);
}

QStandardItemModel* ColorComboBox::getShipClassModel() {
	auto itemModel = new QStandardItemModel();

	for (auto it = Ship_info.cbegin(); it != Ship_info.cend(); ++it) {
		// don't add the pirate ship
		if (it->flags[Ship::Info_Flags::No_fred]) {
			continue;
		}
		QStandardItem* item = new QStandardItem(it->name);
		species_info* sinfo = &Species_info[it->species];
		auto brush = QBrush(QColor(sinfo->fred_color.rgb.r, sinfo->fred_color.rgb.g, sinfo->fred_color.rgb.b));
		item->setData(brush, Qt::ForegroundRole);
		item->setData((int)std::distance(Ship_info.cbegin(), it), Qt::UserRole);
		itemModel->appendRow(item);
	}

	// Add a separator after the ship classes
	QStandardItem* item = new QStandardItem();
	item->setData("separator", Qt::AccessibleDescriptionRole);
	itemModel->appendRow(item);

	item = new QStandardItem("Jump Node");
	item->setData(_viewport->editor->Id_select_type_jump_node, Qt::UserRole);
	itemModel->appendRow(item);

	item = new QStandardItem("Waypoint");
	item->setData(_viewport->editor->Id_select_type_waypoint, Qt::UserRole);
	itemModel->appendRow(item);

	return itemModel;
}
void ColorComboBox::selectShipClass(int ship_class) {
	auto index = findData(ship_class);
	setCurrentIndex(index);
}
void ColorComboBox::indexChanged(int index) {
	if (index < 0) {
		// Invalid index
		return;
	}

	auto shipClass = itemData(index).value<int>();
	shipClassSelected(shipClass);
}

}
}
