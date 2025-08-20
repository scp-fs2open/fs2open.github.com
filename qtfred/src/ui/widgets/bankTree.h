#pragma once
#include "ui/dialogs/ShipEditor/BankModel.h"

#include <ui/util/SignalBlockers.h>

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QTreeView>
namespace fso::fred {
class bankTree : public QTreeView {
	Q_OBJECT
  public:
	bankTree(QWidget*);
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;
	int getTypeSelected() const;

  protected:
	void dragEnterEvent(QDragEnterEvent*) override;
	void dropEvent(QDropEvent* event) override;
	void dragMoveEvent(QDragMoveEvent*) override;
	int typeSelected = -1;
};
} // namespace fso::fred