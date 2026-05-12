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
/* void bankTree::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	auto indexes = selected.indexes();
	if (!indexes.isEmpty()) {
		auto& first = indexes.first();
		if (first.isValid()) {
			if (model()->data(first, Qt::UserRole + 2) == true) {
				for (auto& index :
					model()->match(model()->index(0, 0), Qt::UserRole + 2, false, -1, Qt::MatchRecursive)) {
					if (index.isValid()) {
						auto item = dynamic_cast<QStandardItemModel*>(model())->itemFromIndex(index);
						Qt::ItemFlags flags = item->flags();
						flags &= ~Qt::ItemIsSelectable;
						item->setFlags(flags);
					}
				}
			} else {
				for (auto& index :
					model()->match(model()->index(0, 0), Qt::UserRole + 2, true, -1, Qt::MatchRecursive)) {
					if (index.isValid()) {
						auto item = dynamic_cast<QStandardItemModel*>(model())->itemFromIndex(index);
						Qt::ItemFlags flags = item->flags();
						flags &= ~Qt::ItemIsSelectable;
						item->setFlags(flags);
					}
				}
			}
		}
	}
	QTreeView::selectionChanged(selected, deselected);
}*/

void bankTree::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	// 1. Update the internal selection state first
	QTreeView::selectionChanged(selected, deselected);

	QItemSelectionModel* sm = selectionModel();
	QStandardItemModel* m = qobject_cast<QStandardItemModel*>(model());
	if (!m)
		return;

	// Helper lambda to update the selectable flag across the whole tree
	auto updateTreeFlags = [&](bool isFiltering, bool filterForBank) {
		std::function<void(const QModelIndex&)> traverse = [&](const QModelIndex& parent) {
			for (int r = 0; r < m->rowCount(parent); ++r) {
				QModelIndex idx = m->index(r, 0, parent);
				QStandardItem* item = m->itemFromIndex(idx);
				if (!item)
					continue;

				if (!isFiltering) {
					// Reset mode: Everything becomes selectable
					item->setFlags(item->flags() | Qt::ItemIsSelectable);
				} else {
					// Filter mode: Only items matching the 'bank' status stay selectable
					bool itemIsBank = m->data(idx, Qt::UserRole + 2).toBool();
					if (itemIsBank == filterForBank) {
						item->setFlags(item->flags() | Qt::ItemIsSelectable);
					} else {
						item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
					}
				}

				if (m->hasChildren(idx))
					traverse(idx);
			}
		};
		traverse(QModelIndex());
	};

	// 2. Handle the "Last Item Unselected" case (prevents the crash)
		updateTreeFlags(false, false); // Disable filtering, reset all to selectable

	// 3. We have a selection, so determine the current "Master Type"
	// safe because we checked hasSelection()
	if (!sm->selectedIndexes().empty()) {
		QModelIndex first = sm->selectedIndexes().first();
		if (first.isValid()) {
			currentSelectionIsNotBank = m->data(first, Qt::UserRole + 2).toBool();
			updateTreeFlags(true, currentSelectionIsNotBank);
		}
	}
}
bool bankTree::getTypeSelected() const
{
	return currentSelectionIsNotBank;
}
} // namespace fso::fred