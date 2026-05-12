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
	enum class SelectionType { None, Bank, Weapon };

	bankTree(QWidget*);
	void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;
	SelectionType getSelectionType() const;

  protected:
	void dragEnterEvent(QDragEnterEvent*) override;
	void dropEvent(QDropEvent* event) override;
	void dragMoveEvent(QDragMoveEvent*) override;

  private:
	bool m_autoFiltering = false;
};
} // namespace fso::fred
