#pragma once

#include <QtWidgets/QComboBox>

namespace fso::fred {

class PropComboBox : public QComboBox {
	Q_OBJECT

 public:
	explicit PropComboBox(QWidget* parent = nullptr);

	void selectPropClass(int prop_class);
	void initialize();

 signals:
	void propClassSelected(int prop_class);

 private:
	void indexChanged(int index);
};

}
