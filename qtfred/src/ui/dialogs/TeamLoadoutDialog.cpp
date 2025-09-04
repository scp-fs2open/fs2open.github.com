#include "TeamLoadoutDialog.h"
#include "ui_TeamLoadoutDialog.h"

#include <QtWidgets/QMenuBar>
#include <qlist.h>
#include <qtablewidget.h>
#include <QListWidget>
#include <QMessageBox>
#include <mission/util.h>
#include "ui/util/SignalBlockers.h"
#include "ui/widgets/NoWheelSpinBox.h"
#include "ui/widgets/NoWheelComboBox.h"

constexpr int TABLE_MODE = 0;
constexpr int VARIABLE_MODE = 1;

namespace fso::fred::dialogs {

enum {
	COL_ENABLED = 0,
	COL_NAME = 1,
	COL_INWINGS_OR_VARVALUE = 2,
	COL_EXTRA = 3,
	COL_COUNTVAR = 4,
	COL_REQUIRED = 5
};

enum class Role : int {
	ClassIndex = Qt::UserRole + 1,    // ships/weapons class id
	StringVarIndex = Qt::UserRole + 2 // var id (vars tables)
};

TeamLoadoutDialog::TeamLoadoutDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::TeamLoadoutDialog()), _model(new TeamLoadoutDialogModel(this, viewport)), _viewport(viewport)
{
	this->setFocus();
	ui->setupUi(this);

	initializeUi();
	updateUi();
}

TeamLoadoutDialog::~TeamLoadoutDialog() = default;

void TeamLoadoutDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void TeamLoadoutDialog::reject()
{
	// Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void TeamLoadoutDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
}

void TeamLoadoutDialog::initializeUi()
{
	util::SignalBlockers blockers(this);
	
	auto list = _model->getTeamList();
	ui->currentTeamComboBox->clear();
	for (const auto& team : list) {
		ui->currentTeamComboBox->addItem(QString::fromStdString(team.first), team.second);
	}

	// quickly enable or disable the team spin box (must not get to multiple teams if in SP!)
	if (The_mission.game_type & MISSION_TYPE_MULTI) {
		ui->currentTeamComboBox->setEnabled(true); // TODO make an enable/disable function for all the controls
		ui->copyLoadoutToOtherTeamsButton->setEnabled(true);
	} else {
		ui->currentTeamComboBox->setEnabled(false);
		ui->copyLoadoutToOtherTeamsButton->setEnabled(false);
	}

	// Populate the variable comboboxes
	auto& numberVarList = _model->getNumberVarList();
	for (const auto& item : numberVarList) {
		ui->extraVarShipCombo->addItem(item.first.c_str(), item.second);
		ui->extraVarWeaponCombo->addItem(item.first.c_str(), item.second);
		ui->extraVarShipVarCombo->addItem(item.first.c_str(), item.second);
		ui->extraVarWeaponVarCombo->addItem(item.first.c_str(), item.second);
	}

	// set up the table headers
	populateShipsList();
	populateWeaponsList();
	populateShipVarsList();
	populateWeaponVarsList();
}

void TeamLoadoutDialog::updateUi()
{
	util::SignalBlockers blockers(this);

	ui->weaponValidationCheckbox->setChecked(_model->getSkipValidation());
}

void TeamLoadoutDialog::populateShipsList()
{
	util::SignalBlockers blockers(this);
	
	QTableWidget* tbl = ui->shipsList;

	// Hardening: read-only & nice selection
	tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tbl->setSelectionBehavior(QAbstractItemView::SelectRows);
	tbl->setSortingEnabled(false);
	tbl->clearContents();

	const auto& rows = _model->getShips(); // const SCP_vector<LoadoutItem>&
	tbl->setRowCount(static_cast<int>(rows.size()));

	for (int r = 0; r < static_cast<int>(rows.size()); ++r) {
		const auto& it = rows[r];

		// 0) Enabled (user-editable checkbox with wing-only behavior handled in slot)
		{
			auto* item = new QTableWidgetItem();
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
			item->setCheckState(Qt::Unchecked);
			item->setData(static_cast<int>(Role::ClassIndex), it.infoIndex); // stash class index
			tbl->setItem(r, COL_ENABLED, item);
		}

		// 1) Ship (name)
		{
			auto* item = new QTableWidgetItem(QString::fromUtf8(it.name.c_str()));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(static_cast<int>(Role::ClassIndex), it.infoIndex);
			tbl->setItem(r, COL_NAME, item);
		}

		// 2) In Wings
		{
			auto* item = new QTableWidgetItem(QString::number(it.countInWings));
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(static_cast<int>(Role::ClassIndex), it.infoIndex);
			tbl->setItem(r, COL_INWINGS_OR_VARVALUE, item);
		}

		// 3) Extra (literal pool, not including wings)
		{
			// Underlying item carries the value for sorting + the change signal
			auto* item = new QTableWidgetItem(QString::number(it.extraAllocated));
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			item->setData(Qt::EditRole, it.extraAllocated);
			item->setData(static_cast<int>(Role::ClassIndex), it.infoIndex);
			// We don't set Qt::ItemIsEditable: edits happen via the spinbox, not the item editor
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			tbl->setItem(r, COL_EXTRA, item);

			// Visual editor
			auto* spin = new NoWheelSpinBox(tbl);
			spin->setMinimum(0);
			spin->setMaximum(_model->getLoadoutMaxValue());
			spin->setAccelerated(true);
			spin->setKeyboardTracking(false);

			tbl->setCellWidget(r, COL_EXTRA, spin);

			// Keep wiring minimal: spinbox only updates the item and fires itemChanged
			connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this, [tbl, spin](int v) {
				// Resolve row at runtime (robust to reordering)
				const QModelIndex idx = tbl->indexAt(spin->mapTo(tbl, QPoint(1, 1)));
				if (!idx.isValid())
					return;
				auto* extraItem = tbl->item(idx.row(), idx.column());
				if (!extraItem)
					return;
				extraItem->setData(Qt::EditRole, v);
				extraItem->setText(QString::number(v));
			});
		}

		// 4) Extra Variable (number-var name, if any)
		{
			// Backing item holds roles + the chosen var index in EditRole
			auto* back = new QTableWidgetItem();
			back->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			back->setData(static_cast<int>(Role::ClassIndex), it.infoIndex);
			back->setData(Qt::EditRole, it.varCountIndex); // -1 means “None”
			tbl->setItem(r, COL_COUNTVAR, back);

			// Build the combo as the cell widget
			auto* combo = new NoWheelComboBox(tbl);

			const auto& nums = _model->getNumberVarList(); // vector<pair<name, index>>
			for (const auto& p : nums) {
				combo->addItem(QString::fromUtf8(p.first.c_str()), p.second);
			}

			combo->setCurrentIndex(0);

			// Put it in the cell
			tbl->setCellWidget(r, COL_COUNTVAR, combo);

			// When user changes selection, reflect into backing item (fires itemChanged)
			connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [tbl, combo](int) {
				const QModelIndex idx = tbl->indexAt(combo->mapTo(tbl, QPoint(1, 1)));
				if (!idx.isValid())
					return;
				if (auto* cell = tbl->item(idx.row(), idx.column())) {
					const int varIdx = combo->currentData(Qt::UserRole).toInt(); // -1 for None
					cell->setData(Qt::EditRole, varIdx);
				}
			});
		}

		// Now set the data from the model
		rebuildShipRowFromModel(r);
	}

	// Tighten columns
	auto* hh = tbl->horizontalHeader();
	hh->setSectionsClickable(false);
	hh->setHighlightSections(false);
	hh->setSectionsMovable(false);
	hh->setSectionResizeMode(QHeaderView::ResizeToContents);
	tbl->resizeColumnsToContents();
}

void TeamLoadoutDialog::populateWeaponsList()
{
	util::SignalBlockers blockers(this);
	
	QTableWidget* tbl = ui->weaponsList;

	tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tbl->setSelectionBehavior(QAbstractItemView::SelectRows);
	tbl->setSortingEnabled(false);
	tbl->clearContents();

	const auto& rows = _model->getWeapons();
	tbl->setRowCount((int)rows.size());

	for (int r = 0; r < (int)rows.size(); ++r) {
		const auto& it = rows[r];
		const bool present = it.enabled && (it.extraAllocated > 0 || it.varCountIndex != -1);
		const bool wingOnly = !present && (it.countInWings > 0);

		// 0) Enabled (visual glyph)
		{
			auto* item = new QTableWidgetItem();
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);

			item->setCheckState(present ? Qt::Checked : wingOnly ? Qt::PartiallyChecked : Qt::Unchecked);
			item->setData(static_cast<int>(Role::ClassIndex), it.infoIndex); // stash class index

			if (wingOnly)
				item->setToolTip("In starting wings (pool cleared)");
			tbl->setItem(r, COL_ENABLED, item);
		}
		// 1) Name
		{
			auto* item = new QTableWidgetItem(QString::fromUtf8(it.name.c_str()));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(static_cast<int>(Role::ClassIndex), it.infoIndex);
			tbl->setItem(r, COL_NAME, item);
		}
		// 2) Required
		{
			auto* item = new QTableWidgetItem(it.required ? QString::fromUtf8("Yes") : QString());
			item->setTextAlignment(Qt::AlignCenter);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(static_cast<int>(Role::ClassIndex), it.infoIndex);
			tbl->setItem(r, COL_REQUIRED, item);
		}
		// 3) In Wings
		{
			auto* item = new QTableWidgetItem(QString::number(it.countInWings));
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(static_cast<int>(Role::ClassIndex), it.infoIndex);
			tbl->setItem(r, COL_INWINGS_OR_VARVALUE, item);
		}
		// 4) Extra
		{
			auto* item = new QTableWidgetItem(QString::number(it.extraAllocated));
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(static_cast<int>(Role::ClassIndex), it.infoIndex);
			tbl->setItem(r, COL_EXTRA, item);
		}
		// 5) Amount Variable
		{
			QString varName = _model->getVariableName(it.varCountIndex).c_str();
			auto* item = new QTableWidgetItem(varName);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(static_cast<int>(Role::ClassIndex), it.infoIndex);
			tbl->setItem(r, COL_COUNTVAR, item);
		}
	}

	auto* hh = tbl->horizontalHeader();
	hh->setSectionsClickable(false);
	hh->setHighlightSections(false);
	hh->setSectionsMovable(false);
	hh->setSectionResizeMode(QHeaderView::ResizeToContents);
	tbl->resizeColumnsToContents();
}

void TeamLoadoutDialog::populateShipVarsList()
{
	util::SignalBlockers blockers(this);
	
	QTableWidget* tbl = ui->shipVarsList;

	tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tbl->setSelectionBehavior(QAbstractItemView::SelectRows);
	tbl->setSortingEnabled(false);
	tbl->clearContents();

	const auto rows = _model->getVarShips(); // snapshot
	tbl->setRowCount((int)rows.size());

	for (int r = 0; r < (int)rows.size(); ++r) {
		const auto& it = rows[r];

		// 0) Enabled
		{
			auto* item = new QTableWidgetItem();
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
			item->setCheckState(it.enabled ? Qt::Checked : Qt::Unchecked);
			item->setData(static_cast<int>(Role::StringVarIndex), it.infoIndex); // stash class index
			tbl->setItem(r, COL_ENABLED, item);
		}
		// 1) Var name
		{
			auto* item = new QTableWidgetItem(QString::fromUtf8(it.name.c_str()));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(static_cast<int>(Role::StringVarIndex), it.infoIndex);
			tbl->setItem(r, COL_NAME, item);
		}
		{
			SCP_string name;
			name += " [" + _model->getVariableValueAsString(it.infoIndex) + "]";
			auto* item = new QTableWidgetItem(QString::fromUtf8(name.c_str()));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(static_cast<int>(Role::StringVarIndex), it.infoIndex);
			tbl->setItem(r, COL_INWINGS_OR_VARVALUE, item);
		}
		// 2) Extra
		{
			auto* item = new QTableWidgetItem(QString::number(it.extraAllocated));
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(static_cast<int>(Role::StringVarIndex), it.infoIndex);
			tbl->setItem(r, COL_EXTRA, item);
		}
		// 3) Amount var
		{
			QString varName = _model->getVariableName(it.varCountIndex).c_str();
			auto* item = new QTableWidgetItem(varName);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(static_cast<int>(Role::StringVarIndex), it.infoIndex);
			tbl->setItem(r, COL_COUNTVAR, item);
		}
	}

	auto* hh = tbl->horizontalHeader();
	hh->setSectionsClickable(false);
	hh->setHighlightSections(false);
	hh->setSectionsMovable(false);
	hh->setSectionResizeMode(QHeaderView::ResizeToContents);
	tbl->resizeColumnsToContents();
	tbl->setSortingEnabled(true);
}

void TeamLoadoutDialog::populateWeaponVarsList()
{
	util::SignalBlockers blockers(this);
	
	QTableWidget* tbl = ui->weaponVarsList;

	tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tbl->setSelectionBehavior(QAbstractItemView::SelectRows);
	tbl->setSortingEnabled(false);
	tbl->clearContents();

	const auto rows = _model->getVarWeapons(); // snapshot
	tbl->setRowCount((int)rows.size());

	for (int r = 0; r < (int)rows.size(); ++r) {
		const auto& it = rows[r];

		// 0) Enabled
		{
			auto* item = new QTableWidgetItem();
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
			item->setCheckState(it.enabled ? Qt::Checked : Qt::Unchecked);
			item->setData(static_cast<int>(Role::StringVarIndex), it.infoIndex); // stash class index
			tbl->setItem(r, COL_ENABLED, item);
		}
		// 1) Var name
		{
			auto* item = new QTableWidgetItem(QString::fromUtf8(it.name.c_str()));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(static_cast<int>(Role::StringVarIndex), it.infoIndex);
			tbl->setItem(r, COL_NAME, item);
		}
		{
			SCP_string name;
			name += " [" + _model->getVariableValueAsString(it.infoIndex) + "]";
			auto* item = new QTableWidgetItem(QString::fromUtf8(name.c_str()));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(static_cast<int>(Role::StringVarIndex), it.infoIndex);
			tbl->setItem(r, COL_INWINGS_OR_VARVALUE, item);
		}
		// 2) Extra
		{
			auto* item = new QTableWidgetItem(QString::number(it.extraAllocated));
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(static_cast<int>(Role::StringVarIndex), it.infoIndex);
			tbl->setItem(r, COL_EXTRA, item);
		}
		// 3) Amount var
		{
			QString varName = _model->getVariableName(it.varCountIndex).c_str();
			auto* item = new QTableWidgetItem(varName);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(static_cast<int>(Role::StringVarIndex), it.infoIndex);
			tbl->setItem(r, COL_COUNTVAR, item);
		}
	}

	auto* hh = tbl->horizontalHeader();
	hh->setSectionsClickable(false);
	hh->setHighlightSections(false);
	hh->setSectionsMovable(false);
	hh->setSectionResizeMode(QHeaderView::ResizeToContents);
	tbl->resizeColumnsToContents();
	tbl->setSortingEnabled(true);
}

void TeamLoadoutDialog::rebuildShipRowFromModel(int row)
{
	QTableWidget* tbl = ui->shipsList;
	if (row < 0 || row >= tbl->rowCount())
		return;

	// Block re-entrancy while we poke items in this row
	util::SignalBlockers blockers(this);

	auto* idItem = tbl->item(row, COL_ENABLED);
	if (!idItem)
		return;

	const int classIdx = idItem->data(static_cast<int>(Role::ClassIndex)).toInt();

	// Pull fresh data
	const auto& ships = _model->getShips();
	const auto it =
		std::find_if(ships.begin(), ships.end(), [&](const LoadoutItem& li) { return li.infoIndex == classIdx; });
	if (it == ships.end())
		return;

	const bool present = it->enabled && (it->extraAllocated > 0 || it->varCountIndex != -1);
	const bool wingOnly = !present && (it->countInWings > 0);

	// Enabled
	if (auto* item = tbl->item(row, COL_ENABLED)) {
		item->setCheckState(present ? Qt::Checked : wingOnly ? Qt::PartiallyChecked : Qt::Unchecked);
		item->setToolTip(wingOnly ? "In starting wings (pool cleared)" : QString());
		item->setData(static_cast<int>(Role::ClassIndex), it->infoIndex);
	}

	// Extra (backing item + existing spinbox)
	if (auto* extraItem = tbl->item(row, COL_EXTRA)) {
		// Update the backing item's value/text (keeps itemChanged pathways consistent)
		extraItem->setData(Qt::EditRole, it->extraAllocated);
		extraItem->setData(static_cast<int>(Role::ClassIndex), it->infoIndex);

		// Update the existing spinbox in-place (no reconnects)
		if (auto* spin = qobject_cast<NoWheelSpinBox*>(tbl->cellWidget(row, COL_EXTRA))) {
			const bool editable = it->varCountIndex == -1;
			spin->setEnabled(editable);
			if (!editable) {
				QString tip;
				spin->setToolTip("Driven by count variable");
			} else {
				spin->setToolTip(QString());
			}

			spin->setValue(it->extraAllocated);
		}
	}

	// Count Variable
	if (auto* back = tbl->item(row, COL_COUNTVAR)) {
		// Keep roles correct
		back->setData(static_cast<int>(Role::ClassIndex), it->infoIndex);
		back->setData(Qt::EditRole, it->varCountIndex);

		if (auto* combo = qobject_cast<NoWheelComboBox*>(tbl->cellWidget(row, COL_COUNTVAR))) {
			// Select current
			int want = it->varCountIndex;
			int toSet = 0;
			for (int i = 0; i < combo->count(); ++i)
				if (combo->itemData(i, Qt::UserRole).toInt() == want) {
					toSet = i;
					break;
				}
			combo->setCurrentIndex(toSet);
		}
	}
}

void TeamLoadoutDialog::refreshShipsRows(const QList<int>& tableRows)
{
	QTableWidget* tbl = ui->shipsList;
	QSignalBlocker blockTbl(tbl); // no recursive itemChanged
	const bool wasSorting = tbl->isSortingEnabled();
	tbl->setSortingEnabled(false);
	for (int r : tableRows)
		rebuildShipRowFromModel(r);
	tbl->setSortingEnabled(wasSorting);
}

QList<int> TeamLoadoutDialog::selectedRowNumbers(QTableWidget* tbl)
{
	QList<int> rows;
	const auto idxs = tbl->selectionModel()->selectedRows(/*column=*/0);
	rows.reserve(idxs.size());
	for (const auto& idx : idxs)
		rows.append(idx.row());
	std::sort(rows.begin(), rows.end());
	rows.erase(std::unique(rows.begin(), rows.end()), rows.end());
	return rows;
}

SCP_vector<SCP_string> TeamLoadoutDialog::getSelectedShips() 
{
	/*SCP_vector<SCP_string> namesOut;

	for (int x = 0; x < ui->usedShipsList->rowCount(); ++x) {
		if (ui->usedShipsList->item(x, 0) && ui->usedShipsList->item(x,0)->isSelected()) {
			SCP_string shipName = ui->usedShipsList->item(x, 0)->text().toUtf8().constData();
			namesOut.emplace_back(shipName);
		}
	}

	return namesOut;*/
}

SCP_vector<SCP_string> TeamLoadoutDialog::getSelectedWeapons()
{
	/*SCP_vector<SCP_string> namesOut;

	for (int x = 0; x < ui->usedWeaponsList->rowCount(); ++x) {
		if (ui->usedWeaponsList->item(x, 0) && ui->usedWeaponsList->item(x, 0)->isSelected()) {
			SCP_string weaponName = ui->usedWeaponsList->item(x, 0)->text().toUtf8().constData();
			namesOut.emplace_back(weaponName);
		}
	}

	return namesOut;*/
}

void TeamLoadoutDialog::on_okAndCancelButtons_accepted()
{
	accept();
}

void TeamLoadoutDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

void TeamLoadoutDialog::on_shipsList_itemChanged(QTableWidgetItem* changed)
{
	QTableWidget* tbl = ui->shipsList;
	if (!changed || changed->tableWidget() != tbl)
		return;

	const int col = changed->column();
	if (col != COL_ENABLED && col != COL_EXTRA && col != COL_COUNTVAR)
		return;

	const int originRow = changed->row();

	// Snapshot selection before mutating
	auto selRows = selectedRowNumbers(tbl);
	const bool applyToSelection = selRows.size() > 1 && selRows.contains(originRow);
	if (!applyToSelection)
		selRows = {originRow};

	// Mutate the model based on the user's action
	if (col == COL_ENABLED) {
		const Qt::CheckState desired = changed->checkState();
		for (int r : selRows) {
			auto* chk = tbl->item(r, COL_ENABLED);
			if (!chk)
				continue;
			const int classIdx = chk->data(static_cast<int>(Role::ClassIndex)).toInt();

			_model->setShipEnabled(classIdx, desired != Qt::Unchecked);
		}
	} else if (col == COL_EXTRA) {
		const int newVal =
			changed->data(Qt::EditRole).isValid() ? changed->data(Qt::EditRole).toInt() : changed->text().toInt();
		const int clamped = std::max(0, newVal);

		for (int r : selRows) {
			auto* extraItem = tbl->item(r, COL_EXTRA);
			if (!extraItem)
				continue;
			const int classIdx = extraItem->data(static_cast<int>(Role::ClassIndex)).toInt();

			_model->setShipExtra(classIdx, clamped);
		}
	} else if (col == COL_COUNTVAR) {
		// New value chosen in the combo (stored by our combo handler)
		const int newVarIdx = changed->data(Qt::EditRole).toInt();

		for (int r : selRows) {
			auto* idItem = tbl->item(r, COL_ENABLED);
			if (!idItem)
				continue;

			const int classIdx = idItem->data(static_cast<int>(Role::ClassIndex)).toInt();

			// Update model
			_model->setShipCountVar(classIdx, newVarIdx); // validates inside model
		}
	}

	// Now repaint only the affected rows from the model’s *current* state.
	refreshShipsRows(selRows);
}

void TeamLoadoutDialog::on_weaponsList_itemChanged(QTableWidgetItem* changed)
{
	QTableWidget* tbl = ui->weaponsList;
	if (!changed || changed->tableWidget() != tbl || changed->column() != COL_ENABLED)
		return;

	const int rowChanged = changed->row();
	const Qt::CheckState desired = changed->checkState();

	// Snapshot selection BEFORE any changes
	auto selRows = selectedRowNumbers(tbl);
	const bool applyToSelection = selRows.size() > 1 && selRows.contains(rowChanged);
	if (!applyToSelection)
		selRows = {rowChanged};

	QSignalBlocker blockTbl(tbl);
	const bool wasSorting = tbl->isSortingEnabled();
	tbl->setSortingEnabled(false);

	// Snapshot model state we need for wing-only logic
	const auto& weapsSnapshot = _model->getWeapons();

	for (int r : selRows) {
		auto* cell = tbl->item(r, COL_ENABLED);
		if (!cell)
			continue;

		const int classIdx = cell->data(static_cast<int>(Role::ClassIndex)).toInt();

		auto it = std::find_if(weapsSnapshot.begin(), weapsSnapshot.end(), [&](const LoadoutItem& li) {
			return li.infoIndex == classIdx;
		});
		if (it == weapsSnapshot.end())
			continue;

		const bool present = it->enabled && (it->extraAllocated > 0 || it->varCountIndex != -1);
		const bool wingOnly = !present && (it->countInWings > 0);

		if (desired == Qt::Unchecked) {
			if (wingOnly) {
				cell->setCheckState(Qt::PartiallyChecked);
				cell->setToolTip("In starting wings (pool cleared)");
				continue;
			}
			_model->setWeaponEnabled(classIdx, false);
			const bool stillWingOnly = (it->countInWings > 0);
			cell->setCheckState(stillWingOnly ? Qt::PartiallyChecked : Qt::Unchecked);
			cell->setToolTip(stillWingOnly ? "In starting wings (pool cleared)" : QString());
		} else {
			// Treat Checked and PartiallyChecked clicks as "enable"
			_model->setWeaponEnabled(classIdx, true);
			cell->setCheckState(Qt::Checked);
			cell->setToolTip(QString());
		}
	}

	tbl->setSortingEnabled(wasSorting);
}

void TeamLoadoutDialog::on_shipVarsList_itemChanged(QTableWidgetItem* changed)
{
	QTableWidget* tbl = ui->shipVarsList;
	if (!changed || changed->tableWidget() != tbl || changed->column() != COL_ENABLED)
		return;

	const int rowChanged = changed->row();
	const bool enable = (changed->checkState() == Qt::Checked);

	// Snapshot selection BEFORE mutating anything
	auto selRows = selectedRowNumbers(tbl); // uses selectedRows(0) internally
	const bool applyToSelection = selRows.size() > 1 && selRows.contains(rowChanged);
	if (!applyToSelection)
		selRows = {rowChanged};

	QSignalBlocker blockTbl(tbl);
	const bool wasSorting = tbl->isSortingEnabled();
	tbl->setSortingEnabled(false);

	for (int r : selRows) {
		auto* cell = tbl->item(r, COL_ENABLED);
		if (!cell)
			continue;

		const int varIdx = cell->data(static_cast<int>(Role::StringVarIndex)).toInt();
		_model->setShipVarEnabled(varIdx, enable);

		cell->setCheckState(enable ? Qt::Checked : Qt::Unchecked);
	}

	tbl->setSortingEnabled(wasSorting);
}

void TeamLoadoutDialog::on_weaponVarsList_itemChanged(QTableWidgetItem* changed)
{
	QTableWidget* tbl = ui->weaponVarsList;
	if (!changed || changed->tableWidget() != tbl || changed->column() != COL_ENABLED)
		return;

	const int rowChanged = changed->row();
	const bool enable = (changed->checkState() == Qt::Checked);

	// Snapshot selection BEFORE mutating anything
	auto selRows = selectedRowNumbers(tbl); // uses selectedRows(0) internally
	const bool applyToSelection = selRows.size() > 1 && selRows.contains(rowChanged);
	if (!applyToSelection)
		selRows = {rowChanged};

	QSignalBlocker blockTbl(tbl);
	const bool wasSorting = tbl->isSortingEnabled();
	tbl->setSortingEnabled(false);

	for (int r : selRows) {
		auto* cell = tbl->item(r, COL_ENABLED);
		if (!cell)
			continue;

		const int varIdx = cell->data(static_cast<int>(Role::StringVarIndex)).toInt();
		_model->setWeaponVarEnabled(varIdx, enable);
		cell->setCheckState(enable ? Qt::Checked : Qt::Unchecked);
	}

	tbl->setSortingEnabled(wasSorting);
}

//void TeamLoadoutDialog::on_editVariables_clicked()
//{
//	reinterpret_cast<FredView*>(parent())->on_actionVariables_triggered(true);
//}

//void TeamLoadoutDialog::on_currentTeamComboBox_currentIndexChanged(int index)
//{
//	if (index < 0) {
//		return;
//	}
//	int team = ui->currentTeamComboBox->itemData(index).toInt();
//	_model->switchTeam(team);
//
//	updateUi();
//}

/*void TeamLoadoutDialog::on_copyLoadoutToOtherTeamsButton_clicked()
{
	QMessageBox msgBox;
	msgBox.setText("Are you sure that you want to overwrite the other team's loadout with this team's loadout?");
	msgBox.setInformativeText("This can't be undone.");
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
	msgBox.setDefaultButton(QMessageBox::Cancel);
	int ret = msgBox.exec();

	switch (ret) {
		case QMessageBox::Yes:
			_model->copyToOtherTeams();
			break;
		case QMessageBox::Cancel:
			break;
		default:
			UNREACHABLE("Bad return value from confirmation message box in the Loadout dialog editor.");
			break;
	}
}*/

} // namespace fso::fred::dialogs
