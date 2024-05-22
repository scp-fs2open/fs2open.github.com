#pragma once
#include <QTreeView>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDragMoveEvent>
#include "ui/dialogs/ShipEditor/BankModel.h"
#include <ui/util/SignalBlockers.h>
namespace fso {
namespace fred {
class bankTree : public QTreeView {
	Q_OBJECT
  public:
	bankTree(QWidget*);
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	int getTypeSelected() const;
  protected:
	void dragEnterEvent(QDragEnterEvent*);
	void dropEvent(QDropEvent* event);
	void dragMoveEvent(QDragMoveEvent*);
	int typeSelected = -1;
};
} // namespace fred
} // namespace fso