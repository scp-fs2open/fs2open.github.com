//
//

#include "ColorComboBox.h"

#include <ship/ship.h>
#include <QtGui/QtGui>

namespace fso {
namespace fred {

class ShipClassListModel: public QAbstractListModel {
	EditorViewport* _viewport = nullptr;

 public:
	ShipClassListModel(QObject* parent, EditorViewport* viewport) : QAbstractListModel(parent), _viewport(viewport) {
	}

	int rowCount(const QModelIndex& parent) const override {
		return (int) Ship_info.size() + 3; // One separator and two for waypoint and jumpnode
	}
	QVariant data(const QModelIndex& index, int role) const override {
		if (!index.isValid()) {
			return QVariant();
		}

		if (index.row() < (int) Ship_info.size()) {
			// Handle normal ship classes here
			if (role == Qt::AccessibleDescriptionRole) {
				return "item";
			}

			if (role == Qt::DisplayRole) {
				return QString::fromUtf8(Ship_info[index.row()].name);
			}

			if (role == Qt::ForegroundRole) {
				species_info* sinfo = &Species_info[Ship_info[index.row()].species];
				return QBrush(QColor(sinfo->fred_color.rgb.r, sinfo->fred_color.rgb.g, sinfo->fred_color.rgb.b));
			}

			return QVariant();
		}

		// Handle the special entries
		if (index.row() == (int) Ship_info.size()) {
			// separator
			if (role == Qt::AccessibleDescriptionRole) {
				return "separator";
			}
			return QVariant();
		} else {
			if (role == Qt::ForegroundRole) {
				// Use that standard text brush
				return qGuiApp->palette().text();
			}
			if (role == Qt::AccessibleDescriptionRole) {
				return "item";
			}
			if (role == Qt::DisplayRole) {
				// Account for the separator item above
				if (index.row() - 1 == _viewport->editor->Id_select_type_waypoint) {
					return QString::fromUtf8("Waypoint");
				} else {
					return QString::fromUtf8("Jump Node");
				}
			}
			return QVariant();
		}
	}
};

ColorComboBox::ColorComboBox(QWidget* parent, EditorViewport* viewport) : QComboBox(parent), _viewport(viewport) {
	setModel(new ShipClassListModel(this, _viewport));

	connect(this,
			static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this,
			&ColorComboBox::indexChanged);
}
void ColorComboBox::selectShipClass(int ship_class) {
	setCurrentIndex(ship_class); // TODO: Fix index
}
void ColorComboBox::indexChanged(int index) {
	if (index < 0) {
		// Invalid index
		return;
	}

	if (index < (int)Ship_info.size()) {
		// Already a valid index
		shipClassSelected(index);
		return;
	}

	// The rest of the combobox are special items but there is a separator item which changes the index slightly
	shipClassSelected(index - 1);
}

}
}
