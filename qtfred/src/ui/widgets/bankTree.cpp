#include "bankTree.h"
namespace fso::fred {
bankTree::bankTree(QWidget* parent) : QTreeView(parent)
{
	setAcceptDrops(true);
}
void bankTree::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat("application/weaponid")) {
		event->acceptProposedAction();
	}
}
void bankTree::dropEvent(QDropEvent* event)
{
	auto item = indexAt(event->pos());
	if (!item.isValid()) {
		return;
	}
	bool accepted = model()->dropMimeData(event->mimeData(), Qt::CopyAction, -1, 0, item);
	if (accepted) {
		event->acceptProposedAction();
	}
}
void bankTree::dragMoveEvent(QDragMoveEvent* event)
{
	auto pos = QCursor::pos();
	auto index = indexAt(pos);
	if (!index.isValid()) {
		return;
	}
	if (model()->data(index, Qt::UserRole + 2) == false) {
		event->accept();
	} else {
		event->ignore();
	}
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
	const bool pivotIsBank = pivot.data(Qt::UserRole + 2).toBool();

	QItemSelectionModel* sm = selectionModel();
	QItemSelection toDeselect;
	for (const QModelIndex& idx : sm->selectedIndexes()) {
		if (idx.column() != 0) {
			continue;
		}
		if (idx.data(Qt::UserRole + 2).toBool() != pivotIsBank) {
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
		return idx.data(Qt::UserRole + 2).toBool() ? SelectionType::Bank : SelectionType::Weapon;
	}
	return SelectionType::None;
}
} // namespace fso::fred
