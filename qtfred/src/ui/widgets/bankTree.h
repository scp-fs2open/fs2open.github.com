#pragma once

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QTreeView>
namespace fso::fred {
// MIME type for drags originating from the weapons-list view into the bank tree.
// Payload: a single int (weapon id) written via QDataStream.
constexpr const char* MIME_WEAPON_ID = "application/weaponid";

// Custom data roles stored on items of the bank tree's QStandardItemModel.
// (Qt::UserRole itself is used for the weapon-id on weapon-slot rows.)
constexpr int BankItemIsLabelRole = Qt::UserRole + 2; // bool: true on bank-label rows, false on weapon-slot rows
constexpr int BankItemIdRole = Qt::UserRole + 3;      // Banks::getId() on labels, Bank::getBankId() on slots
constexpr int BankItemMaxAmmoRole = Qt::UserRole + 5; // weapon's max ammo on the bank, 0 if not applicable

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
