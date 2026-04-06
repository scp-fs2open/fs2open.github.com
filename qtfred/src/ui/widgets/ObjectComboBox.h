#pragma once

#include <QtWidgets/QComboBox>
#include <QtGui/QStandardItemModel>

#include <mission/EditorViewport.h>

namespace fso::fred {

class ObjectComboBox : public QComboBox {
	Q_OBJECT

	EditorViewport* _viewport = nullptr;

public:
	explicit ObjectComboBox(QWidget* parent = nullptr);

	void initForShips(EditorViewport* viewport);
	void initForProps();

	void selectClass(int class_index);

signals:
	void classSelected(int class_index);

private:
	void buildShipsModel();
	void buildPropsModel();
	void indexChanged(int index);
};

} // namespace fso::fred
