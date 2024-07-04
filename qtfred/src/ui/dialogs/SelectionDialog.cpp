//
//

#include "SelectionDialog.h"

#include <iff_defs/iff_defs.h>

#include "ui_SelectionDialog.h"

#include <ui/util/SignalBlockers.h>

#include <QtDebug>

namespace fso {
namespace fred {
namespace dialogs {


SelectionDialog::SelectionDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent), ui(new Ui::SelectionDialog()), _model(new SelectionDialogModel(this, viewport)) {
	ui->setupUi(this);

	connect(this, &QDialog::accepted, _model.get(), &SelectionDialogModel::apply);
	connect(this, &QDialog::rejected, _model.get(), &SelectionDialogModel::reject);


	connect(ui->checkShips,
			&QCheckBox::stateChanged,
			this,
			[this](int state) { _model->setFilterShips(state == Qt::Checked); });
	connect(ui->checkWaypoints,
			&QCheckBox::stateChanged,
			this,
			[this](int state) { _model->setFilterWaypoints(state == Qt::Checked); });
	connect(ui->checkPlayerStarts,
			&QCheckBox::stateChanged,
			this,
			[this](int state) { _model->setFilterStarts(state == Qt::Checked); });

	// Initialize IFF check boxes
	for (auto i = 0; i < (int)Iff_info.size(); ++i) {
		auto checkbox = new QCheckBox(QString::fromUtf8(Iff_info[i].iff_name), this);
		_iffCheckBoxes.push_back(checkbox);
		ui->iffSelectionContainer->addWidget(checkbox);

		connect(checkbox,
				&QCheckBox::stateChanged,
				this,
				[this, i](int state) { _model->setFilterIFFTeam(i, state == Qt::Checked); });
	}

	connect(ui->buttonAll, &QPushButton::pressed, this, [this]() { _model->selectAll(); });
	connect(ui->buttonClear, &QPushButton::pressed, this, [this]() { _model->clearSelection(); });
	connect(ui->buttonInvert, &QPushButton::pressed, this, [this]() { _model->invertSelection(); });

	connect(ui->shipSelectionList, &QListWidget::itemSelectionChanged, this, &SelectionDialog::objectSelectionChanged);
	connect(ui->wingSelectionList, &QListWidget::currentItemChanged, this, &SelectionDialog::wingSelectionChanged);

	connect(_model.get(), &SelectionDialogModel::modelChanged, this, &SelectionDialog::updateUI);
	connect(_model.get(), &SelectionDialogModel::selectionUpdated, this, &SelectionDialog::updateListSelection);

	// Initial UI update
	updateUI();

	resize(sizeHint());
}
SelectionDialog::~SelectionDialog() {

}
void SelectionDialog::updateUI() {
	util::SignalBlockers blockers(this);
	// Update check boxes
	ui->checkPlayerStarts->setChecked(_model->isFilterStarts());
	ui->checkWaypoints->setChecked(_model->isFilterWaypoints());
	ui->checkShips->setChecked(_model->isFilterShips());
	for (auto i = 0; i < (int)Iff_info.size(); ++i) {
		_iffCheckBoxes[i]->setChecked(_model->isFilterIFFTeam(i));
		_iffCheckBoxes[i]->setEnabled(_model->isFilterShips());
	}

	// Update ship list
	ui->shipSelectionList->clear();
	for (auto& entry : _model->getObjectList()) {
		auto item = new QListWidgetItem(QString::fromStdString(entry.name));
		item->setData(Qt::UserRole, entry.id);
		ui->shipSelectionList->addItem(item);
	}

	// Update wing and waypoint list
	ui->wingSelectionList->clear();
	for (auto& entry : _model->getWingList()) {
		auto item = new QListWidgetItem(QString::fromStdString(entry.name));
		item->setData(Qt::UserRole, entry.id);
		item->setData(Qt::UserRole + 1, true); // This is a wing item
		ui->wingSelectionList->addItem(item);
	}
	for (auto& entry : _model->getWaypointList()) {
		auto item = new QListWidgetItem(QString::fromStdString(entry.name));
		item->setData(Qt::UserRole, entry.id);
		item->setData(Qt::UserRole + 1, false);
		ui->wingSelectionList->addItem(item);
	}

	updateListSelection();
}
void SelectionDialog::objectSelectionChanged() {
	auto current = _model->getObjectList(); // Copy the vector so we can change the selection

	for (auto& entry : current) {
		auto items =
			ui->shipSelectionList->model()->match(ui->shipSelectionList->model()->index(0, 0), Qt::UserRole, entry.id);

		Assertion(items.size() > 0, "No items for object index found!");
		Assertion(items.size() <= 1, "Found multiple items for one object index!");

		auto item = ui->shipSelectionList->item(items[0].row());

		Assertion(item != nullptr, "Couldn't find item for index!");
		entry.selected = item->isSelected();
	}

	_model->updateObjectSelection(current);
}
void SelectionDialog::wingSelectionChanged(QListWidgetItem* current, QListWidgetItem*  /*previous*/) {
	auto isWing = current->data(Qt::UserRole + 1).value<bool>();
	auto id = current->data(Qt::UserRole).value<int>();

	if (isWing) {
		_model->selectWing(id);
	} else {
		_model->selectWaypointPath(id);
	}
}
void SelectionDialog::updateListSelection() {
	QSignalBlocker shipBlocker(ui->shipSelectionList);
	QSignalBlocker wingBlocker(ui->wingSelectionList);

	for (auto& entry : _model->getObjectList()) {
		auto items =
			ui->shipSelectionList->model()->match(ui->shipSelectionList->model()->index(0, 0), Qt::UserRole, entry.id);

		Assertion(items.size() > 0, "No items for object index found!");
		Assertion(items.size() <= 1, "Found multiple items for one object index!");

		ui->shipSelectionList->selectionModel()->select(items[0],
														entry.selected ? QItemSelectionModel::Select
																	   : QItemSelectionModel::Deselect);
	}

	auto& wingList = _model->getWingList();
	auto& waypointList = _model->getWaypointList();

	for (auto i = 0; i < ui->wingSelectionList->count(); ++i) {
		auto item = ui->wingSelectionList->item(i);
		auto id = item->data(Qt::UserRole).value<int>();
		auto isWing = item->data(Qt::UserRole + 1).value<bool>();

		if (isWing) {
			for (auto& entry : wingList) {
				if (entry.id != id) {
					continue;
				}

				item->setSelected(entry.selected);
				break;
			}
		} else {
			for (auto& entry : waypointList) {
				if (entry.id != id) {
					continue;
				}

				item->setSelected(entry.selected);
				break;
			}
		}
	}
}

}
}
}
