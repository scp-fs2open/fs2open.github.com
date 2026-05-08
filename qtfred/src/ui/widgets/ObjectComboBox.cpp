#if defined(_MSC_VER) && _MSC_VER <= 1920
	#define QT_NO_FLOAT16_OPERATORS
#endif

#include "ObjectComboBox.h"

#include <ship/ship.h>
#include <prop/prop.h>
#include <species_defs/species_defs.h>

#include <QtGui/QBrush>
#include <QtGui/QColor>

#include <FredApplication.h>

namespace fso::fred {

ObjectComboBox::ObjectComboBox(QWidget* parent) : QComboBox(parent) {
	connect(this,
			static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this,
			&ObjectComboBox::indexChanged);
}

void ObjectComboBox::initForShips(EditorViewport* viewport) {
	_viewport = viewport;
	fredApp->runAfterInit([this]() {
		buildShipsModel();
	});
}

void ObjectComboBox::initForProps() {
	fredApp->runAfterInit([this]() {
		buildPropsModel();
	});
}

void ObjectComboBox::buildShipsModel() {
	auto model = new QStandardItemModel();

	for (auto it = Ship_info.cbegin(); it != Ship_info.cend(); ++it) {
		if (it->flags[Ship::Info_Flags::No_fred]) {
			continue;
		}
		auto item = new QStandardItem(it->name);
		species_info* sinfo = &Species_info[it->species];
		item->setData(QBrush(QColor(sinfo->fred_color.rgb.r, sinfo->fred_color.rgb.g, sinfo->fred_color.rgb.b)),
					  Qt::ForegroundRole);
		item->setData((int)std::distance(Ship_info.cbegin(), it), Qt::UserRole);
		model->appendRow(item);
	}

	auto separator = new QStandardItem();
	separator->setData("separator", Qt::AccessibleDescriptionRole);
	model->appendRow(separator);

	auto waypoint = new QStandardItem("Waypoint");
	waypoint->setData(_viewport->editor->Id_select_type_waypoint, Qt::UserRole);
	model->appendRow(waypoint);

	auto jumpNode = new QStandardItem("Jump Node");
	jumpNode->setData(_viewport->editor->Id_select_type_jump_node, Qt::UserRole);
	model->appendRow(jumpNode);

	setModel(model);
}

void ObjectComboBox::buildPropsModel() {
	auto model = new QStandardItemModel();

	for (int i = 0; i < prop_info_size(); ++i) {
		if (Prop_info[i].flags[Prop::Info_Flags::No_fred]) {
			continue;
		}
		auto item = new QStandardItem(QString::fromStdString(Prop_info[i].name));
		item->setData(i, Qt::UserRole);
		auto category = prop_get_category(Prop_info[i].category_index);
		if (category != nullptr) {
			item->setData(QBrush(QColor(category->list_color.red, category->list_color.green, category->list_color.blue)),
						  Qt::ForegroundRole);
		}
		model->appendRow(item);
	}

	setModel(model);
}

void ObjectComboBox::selectClass(int class_index) {
	setCurrentIndex(findData(class_index));
}

void ObjectComboBox::indexChanged(int index) {
	if (index < 0) {
		return;
	}
	Q_EMIT classSelected(itemData(index).value<int>());
}

} // namespace fso::fred
