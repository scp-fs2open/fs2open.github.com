#include "IconFromShipDialog.h"

#include "mission/dialogs/BriefingEditorDialogModel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>

namespace fso::fred::dialogs {

IconFromShipDialog::IconFromShipDialog(QWidget* parent, BriefingEditorDialogModel* model)
	: QDialog(parent), _model(model)
{
	setWindowTitle("Make Icon From Ship");
	setMinimumSize(350, 450);

	auto* layout = new QVBoxLayout(this);

	// Search/filter
	auto* filterLayout = new QHBoxLayout();
	filterLayout->addWidget(new QLabel("Filter:", this));
	_filter = new QLineEdit(this);
	_filter->setPlaceholderText("Search ships...");
	_filter->setClearButtonEnabled(true);
	filterLayout->addWidget(_filter);
	layout->addLayout(filterLayout);

	// Tree widget
	_tree = new QTreeWidget(this);
	_tree->setHeaderLabel("Ships");
	_tree->setSelectionMode(QAbstractItemView::SingleSelection);
	layout->addWidget(_tree);

	// Buttons
	auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	layout->addWidget(buttons);

	populateTree();

	connect(_filter, &QLineEdit::textChanged, this, &IconFromShipDialog::onFilterTextChanged);
	connect(_tree, &QTreeWidget::itemDoubleClicked, this, &IconFromShipDialog::onItemDoubleClicked);
	connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
		auto* item = _tree->currentItem();
		if (item && item->data(0, Qt::UserRole).toInt() >= 0) {
			_selectedShipIndex = item->data(0, Qt::UserRole).toInt();
			accept();
		}
	});
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

	setLayout(layout);
}

int IconFromShipDialog::selectedShipIndex() const
{
	return _selectedShipIndex;
}

void IconFromShipDialog::populateTree()
{
	_tree->clear();

	auto wingTree = BriefingEditorDialogModel::getWingShipTree();

	for (const auto& entry : wingTree) {
		if (entry.wingName.empty()) {
			// Ungrouped ships — add at root level
			for (const auto& ship : entry.ships) {
				auto* item = new QTreeWidgetItem(_tree);
				item->setText(0, QString::fromStdString(ship.name));
				item->setData(0, Qt::UserRole, ship.shipIndex);
			}
		} else {
			// Wing group
			auto* wingItem = new QTreeWidgetItem(_tree);
			wingItem->setText(0, QString::fromStdString(entry.wingName) + " (Wing)");
			wingItem->setData(0, Qt::UserRole, -1); // Not a selectable ship
			wingItem->setFlags(wingItem->flags() & ~Qt::ItemIsSelectable);

			for (const auto& ship : entry.ships) {
				auto* shipItem = new QTreeWidgetItem(wingItem);
				shipItem->setText(0, QString::fromStdString(ship.name));
				shipItem->setData(0, Qt::UserRole, ship.shipIndex);
			}

			wingItem->setExpanded(true);
		}
	}
}

void IconFromShipDialog::applyFilter(const QString& text)
{
	for (int i = 0; i < _tree->topLevelItemCount(); ++i) {
		auto* topItem = _tree->topLevelItem(i);

		if (topItem->childCount() == 0) {
			// Root-level ship
			bool matches = text.isEmpty() ||
				topItem->text(0).contains(text, Qt::CaseInsensitive);
			topItem->setHidden(!matches);
		} else {
			// Wing group — show if any child matches
			bool anyChildVisible = false;
			for (int j = 0; j < topItem->childCount(); ++j) {
				auto* child = topItem->child(j);
				bool matches = text.isEmpty() ||
					child->text(0).contains(text, Qt::CaseInsensitive);
				child->setHidden(!matches);
				if (matches)
					anyChildVisible = true;
			}
			// Also show wing if wing name itself matches
			if (!anyChildVisible && !text.isEmpty()) {
				anyChildVisible = topItem->text(0).contains(text, Qt::CaseInsensitive);
				if (anyChildVisible) {
					// Show all children
					for (int j = 0; j < topItem->childCount(); ++j)
						topItem->child(j)->setHidden(false);
				}
			}
			topItem->setHidden(!anyChildVisible);
			if (anyChildVisible)
				topItem->setExpanded(true);
		}
	}
}

void IconFromShipDialog::onFilterTextChanged(const QString& text)
{
	applyFilter(text);
}

void IconFromShipDialog::onItemDoubleClicked(QTreeWidgetItem* item, int /*column*/)
{
	if (item && item->data(0, Qt::UserRole).toInt() >= 0) {
		_selectedShipIndex = item->data(0, Qt::UserRole).toInt();
		accept();
	}
}

} // namespace fso::fred::dialogs
