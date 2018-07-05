#pragma once


#include <QtWidgets/QComboBox>
#include <QtGui/QStandardItemModel>

#include <mission/EditorViewport.h>

namespace fso {
namespace fred {

class ColorComboBox: public QComboBox {
	Q_OBJECT

	EditorViewport* _viewport = nullptr;

 public:
	ColorComboBox(QWidget* parent, EditorViewport* viewport);

	void selectShipClass(int ship_class);

 private:
	void indexChanged(int index);

	QStandardItemModel* getShipClassModel();

 signals:
	void shipClassSelected(int ship_class);
};

}
}


