//
//

#include "ColorComboBox.h"

#include <ship/ship.h>

namespace fso {
namespace fred {

class ShipClassListModel: public QAbstractListModel {
 public:
	ShipClassListModel(QObject* parent = nullptr) : QAbstractListModel(parent) {
	}

	int rowCount(const QModelIndex& parent) const override {
		return (int) Ship_info.size();
	}
	QVariant data(const QModelIndex& index, int role) const override {
		if (!index.isValid()) {
			return QVariant();
		}

		if (role == Qt::DisplayRole) {
			return QString::fromUtf8(Ship_info[index.row()].name);
		}

		if (role == Qt::ForegroundRole ) {
			species_info* sinfo = &Species_info[Ship_info[index.row()].species];
			return QBrush(QColor(sinfo->fred_color.rgb.r, sinfo->fred_color.rgb.g, sinfo->fred_color.rgb.b));
		}

		return QVariant();
	}
};

ColorComboBox::ColorComboBox(QWidget* parent) : QComboBox(parent) {
	setModel(new ShipClassListModel(this));
	setCurrentIndex(0);
}

}
}
