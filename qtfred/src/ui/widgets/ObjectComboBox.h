#pragma once

#include <QtWidgets/QComboBox>
#include <QtGui/QStandardItemModel>

#include <mission/EditorViewport.h>

namespace fso::fred {

class ObjectComboBox : public QComboBox {
	Q_OBJECT

public:
	explicit ObjectComboBox(QWidget* parent = nullptr);

	void initForShips();
	void initForProps();
	void initForOther();

	void selectClass(int class_index);

signals:
	void classSelected(int class_index);

private:
	void buildShipsModel();
	void buildPropsModel();
	void buildOtherModel();
	void indexChanged(int index);
};

} // namespace fso::fred
