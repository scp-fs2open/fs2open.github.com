#pragma once
#include "ui/dialogs/ShipEditor/BankModel.h"

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

  signals:
	void weaponDroppedFromList(const QModelIndex& target, int weaponId);
	void weaponMoved(const QModelIndex& target, int weaponId, int sourceBanksId, int sourceBankId, bool isCopy);

  protected:
	void dragEnterEvent(QDragEnterEvent*) override;
	void dropEvent(QDropEvent* event) override;
	void dragMoveEvent(QDragMoveEvent*) override;
	void startDrag(Qt::DropActions supportedActions) override;

  private:
	bool m_autoFiltering = false;
};
} // namespace fso::fred
