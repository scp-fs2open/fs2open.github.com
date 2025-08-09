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
	if (dynamic_cast<BankTreeModel*>(model())->checktype(index) == 0) {
		event->accept();
	} else {
		event->ignore();
	}
}
void bankTree::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
	QItemSelection newlySelected;
	QItemSelection select;
	QItemSelection deselect(deselected);
	if (selected.empty()) {
		QTreeView::selectionChanged(selected, deselected);
		if (selectionModel()->selectedIndexes().empty()) {
			typeSelected = -1;
		}
		return;
	}
	for (auto& sidx : selected.indexes()) {
		bool match = false;
		for (auto& didx : deselected.indexes()) {
			if (sidx == didx) {
				match = true;
				break;
			}
		}
		if (!match) {
			QItemSelectionRange selection(sidx);
			newlySelected.append(selection);
		}
	}
	if (!newlySelected.empty()) {
		if (typeSelected == -1) {
			typeSelected = dynamic_cast<BankTreeModel*>(model())->checktype(newlySelected.indexes().first());
			for (auto& sidx : newlySelected.indexes()) {
				if (dynamic_cast<BankTreeModel*>(model())->checktype(sidx) == typeSelected) {
					QItemSelectionRange selection(sidx);
					select.append(selection);
				}
			}
		} else {
			int type = dynamic_cast<BankTreeModel*>(model())->checktype(newlySelected.indexes().first());
			if (type != typeSelected) {
				typeSelected = type;
				for (auto& sidx : selected.indexes()) {
					QItemSelectionRange selection(sidx);
					deselect.append(selection);
				}
				for (auto& sidx : newlySelected.indexes()) {
					if (dynamic_cast<BankTreeModel*>(model())->checktype(sidx) == typeSelected) {
						QItemSelectionRange selection(sidx);
						select.append(selection);
					}
				}
				selectionModel()->clear();
				typeSelected = -1;
			} else {
				for (auto& sidx : newlySelected.indexes()) {
					if (dynamic_cast<BankTreeModel*>(model())->checktype(sidx) == typeSelected) {
						QItemSelectionRange selection(sidx);
						select.append(selection);
					}
				}
			}
		}
	}
	QTreeView::selectionChanged(select, deselect);
}
int bankTree::getTypeSelected() const
{
	return typeSelected;
}
} // namespace fso::fred