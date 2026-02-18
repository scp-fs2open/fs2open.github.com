#if defined(_MSC_VER) && _MSC_VER <= 1920
	#define QT_NO_FLOAT16_OPERATORS
#endif

#include "PropComboBox.h"

#include <FredApplication.h>
#include <prop/prop.h>

#include <QtGui/QBrush>
#include <QtGui/QColor>

namespace fso::fred {

PropComboBox::PropComboBox(QWidget* parent) : QComboBox(parent) {
	fredApp->runAfterInit([this]() {
		initialize();
	});

	connect(this,
			static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this,
			&PropComboBox::indexChanged);
}

void PropComboBox::initialize() {
	clear();

	for (int i = 0; i < prop_info_size(); ++i) {
		if (Prop_info[i].flags[Prop::Info_Flags::No_fred]) {
			continue;
		}

		addItem(QString::fromStdString(Prop_info[i].name), i);

		auto category = prop_get_category(Prop_info[i].category_index);
		if (category != nullptr) {
			setItemData(count() - 1,
						QBrush(QColor(category->list_color.red, category->list_color.green, category->list_color.blue)),
						Qt::ForegroundRole);
		}
	}
}

void PropComboBox::selectPropClass(int prop_class) {
	auto index = findData(prop_class);
	setCurrentIndex(index);
}

void PropComboBox::indexChanged(int index) {
	if (index < 0) {
		return;
	}

	auto propClass = itemData(index).value<int>();
	Q_EMIT propClassSelected(propClass);
}

}
