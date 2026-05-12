#include "bankTree.h"

#include <QDrag>

namespace fso::fred {

namespace {
constexpr const char* MIME_WEAPON_ID = "application/weaponid";
constexpr const char* MIME_BANK_TREE_WEAPON = "application/banktreeweapon";
} // namespace

bankTree::bankTree(QWidget* parent) : QTreeView(parent)
{
	setAcceptDrops(true);
}

void bankTree::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat(MIME_WEAPON_ID) || event->mimeData()->hasFormat(MIME_BANK_TREE_WEAPON)) {
		event->acceptProposedAction();
	} else {
		event->ignore();
	}
}

void bankTree::dragMoveEvent(QDragMoveEvent* event)
{
	const QModelIndex index = indexAt(event->pos());
	if (!index.isValid() || index.data(BankItemIsLabelRole).toBool()) {
		event->ignore();
		return;
	}
	event->acceptProposedAction();
}

void bankTree::dropEvent(QDropEvent* event)
{
	QModelIndex target = indexAt(event->pos());
	if (!target.isValid() || target.data(BankItemIsLabelRole).toBool()) {
		event->ignore();
		return;
	}
	if (target.column() != 0) {
		target = target.sibling(target.row(), 0);
	}

	const QMimeData* mime = event->mimeData();
	if (mime->hasFormat(MIME_BANK_TREE_WEAPON)) {
		QByteArray bytes = mime->data(MIME_BANK_TREE_WEAPON);
		QDataStream stream(&bytes, QIODevice::ReadOnly);
		int weaponId = 0;
		int sourceBanksId = 0;
		int sourceBankId = 0;
		stream >> weaponId >> sourceBanksId >> sourceBankId;
		const bool isCopy = (event->keyboardModifiers() & Qt::ControlModifier) != 0;
		weaponMoved(target, weaponId, sourceBanksId, sourceBankId, isCopy);
		event->acceptProposedAction();
	} else if (mime->hasFormat(MIME_WEAPON_ID)) {
		QByteArray bytes = mime->data(MIME_WEAPON_ID);
		QDataStream stream(&bytes, QIODevice::ReadOnly);
		int weaponId = 0;
		stream >> weaponId;
		weaponDroppedFromList(target, weaponId);
		event->acceptProposedAction();
	} else {
		event->ignore();
	}
}

void bankTree::startDrag(Qt::DropActions /*supportedActions*/)
{
	QModelIndex source;
	for (const QModelIndex& idx : selectionModel()->selectedIndexes()) {
		if (idx.column() != 0 || !idx.isValid()) {
			continue;
		}
		if (idx.data(BankItemIsLabelRole).toBool()) {
			continue;
		}
		source = idx;
		break;
	}
	if (!source.isValid()) {
		return;
	}
	const int weaponId = source.data(Qt::UserRole).toInt();
	// "None" (-1) and "CONFLICT" (-2) slots have no weapon to drag.
	if (weaponId < 0) {
		return;
	}

	const QModelIndex parent = source.parent();
	if (!parent.isValid()) {
		return;
	}
	const int sourceBanksId = parent.data(BankItemIdRole).toInt();
	const int sourceBankId = source.data(BankItemIdRole).toInt();

	auto* mime = new QMimeData();
	QByteArray bytes;
	QDataStream stream(&bytes, QIODevice::WriteOnly);
	stream << weaponId << sourceBanksId << sourceBankId;
	mime->setData(MIME_BANK_TREE_WEAPON, bytes);

	auto* drag = new QDrag(this);
	drag->setMimeData(mime);
	drag->exec(Qt::MoveAction | Qt::CopyAction, Qt::MoveAction);
}

void bankTree::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	QTreeView::selectionChanged(selected, deselected);

	if (m_autoFiltering) {
		return;
	}

	const auto newlySelected = selected.indexes();
	if (newlySelected.isEmpty()) {
		return;
	}

	QModelIndex pivot;
	for (const QModelIndex& idx : newlySelected) {
		if (idx.column() == 0 && idx.isValid()) {
			pivot = idx;
			break;
		}
	}
	if (!pivot.isValid()) {
		return;
	}
	const bool pivotIsBank = pivot.data(BankItemIsLabelRole).toBool();

	QItemSelectionModel* sm = selectionModel();
	QItemSelection toDeselect;
	for (const QModelIndex& idx : sm->selectedIndexes()) {
		if (idx.column() != 0) {
			continue;
		}
		if (idx.data(BankItemIsLabelRole).toBool() != pivotIsBank) {
			toDeselect.select(idx, idx);
		}
	}

	if (!toDeselect.isEmpty()) {
		m_autoFiltering = true;
		sm->select(toDeselect, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
		m_autoFiltering = false;
	}
}

bankTree::SelectionType bankTree::getSelectionType() const
{
	const auto* sm = selectionModel();
	if (sm == nullptr) {
		return SelectionType::None;
	}
	for (const QModelIndex& idx : sm->selectedIndexes()) {
		if (idx.column() != 0 || !idx.isValid()) {
			continue;
		}
		return idx.data(BankItemIsLabelRole).toBool() ? SelectionType::Bank : SelectionType::Weapon;
	}
	return SelectionType::None;
}
} // namespace fso::fred
