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

void bankTree::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
	// Iterate QItemSelection directly as QList<QItemSelectionRange> via const ref
	// avoids calling .indexes() which returns a new QList<QModelIndex> by value
	bool hasSelection = false;
	bool isBank = false;
	for (const QItemSelectionRange& range : selected) {
		QModelIndex first = range.topLeft();
		if (first.isValid()) {
			hasSelection = true;
			isBank = (model()->data(first, Qt::UserRole + 2) == true);
			break;
		}
	}

	    if (hasSelection) {
		// Traverse model manually to avoid model()->match() returning QList by value
		bool disableValue = !isBank;
		std::function<void(const QModelIndex&)> traverse = [&](const QModelIndex& parent) {
			for (int row = 0; row < model()->rowCount(parent); ++row) {
				QModelIndex idx = model()->index(row, 0, parent);
				if (model()->data(idx, Qt::UserRole + 2) == disableValue) {
					auto* item = dynamic_cast<QStandardItemModel*>(model())->itemFromIndex(idx);
					item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
				}
				if (model()->hasChildren(idx)) {
					traverse(idx);
				}
			}
		};
		traverse(QModelIndex());
	}

	QTreeView::selectionChanged(selected, deselected);
}
	int bankTree::getTypeSelected() const
{
	return typeSelected;
}
} // namespace fso::fred