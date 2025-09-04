#include "TeamLoadoutDialog.h"
#include "ui_TeamLoadoutDialog.h"

#include <QtWidgets/QMenuBar>
#include <qlist.h>
#include <qtablewidget.h>
#include <QListWidget>
#include <QMessageBox>
#include <QLineEdit>
#include <QTimer>
#include <mission/util.h>
#include "ui/util/SignalBlockers.h"
#include "ui/widgets/NoWheelSpinBox.h"
#include "ui/widgets/NoWheelComboBox.h"

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

	// set up the table headers
	populateShipsList();
	populateWeaponsList();
	populateShipVarsList();
	populateWeaponVarsList();

	ui->tabWidget->setCurrentIndex(0); // Always start on the static tab
}

void TeamLoadoutDialog::updateUi()
{
	util::SignalBlockers blockers(this);

	ui->weaponValidationCheckbox->setChecked(_model->getSkipValidation());
	ui->currentTeamComboBox->setCurrentIndex(ui->currentTeamComboBox->findData(_model->getCurrentTeam()));

	// Refresh Ships List
	QList<int> shipRows;
	for (int i = 0; i < ui->shipsList->rowCount(); ++i) {
		shipRows.append(i);
	}
	if (!shipRows.isEmpty()) {
		refreshShipRows(shipRows);
	}

	// Refresh Weapons List
	QList<int> weaponRows;
	for (int i = 0; i < ui->weaponsList->rowCount(); ++i) {
		weaponRows.append(i);
	}
	if (!weaponRows.isEmpty()) {
		refreshWeaponRows(weaponRows);
	}

	// Refresh Ship Vars List
	QList<int> shipVarRows;
	for (int i = 0; i < ui->shipVarsList->rowCount(); ++i) {
		shipVarRows.append(i);
	}
	if (!shipVarRows.isEmpty()) {
		refreshShipVarRows(shipVarRows);
	}

	// Refresh Weapon Vars List
	QList<int> weaponVarRows;
	for (int i = 0; i < ui->weaponVarsList->rowCount(); ++i) {
		weaponVarRows.append(i);
	}
	if (!weaponVarRows.isEmpty()) {
		refreshWeaponVarRows(weaponVarRows);
	}
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

		setupEnabledCell(tbl, r, it.infoIndex);
		setupNameCell(tbl, r, it.infoIndex, QString::fromUtf8(it.name.c_str()));
		setupWingOrVarValueCell(tbl, r, it.infoIndex, QString::number(it.countInWings));
		setupExtraCountCell(tbl, r, it.infoIndex);
		setupVarCountCell(tbl, r, it.infoIndex);

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
		
		setupEnabledCell(tbl, r, it.infoIndex);
		setupNameCell(tbl, r, it.infoIndex, QString::fromUtf8(it.name.c_str()));
		setupWingOrVarValueCell(tbl, r, it.infoIndex, QString::number(it.countInWings));
		setupExtraCountCell(tbl, r, it.infoIndex);
		setupVarCountCell(tbl, r, it.infoIndex);
		setupRequiredCell(tbl, r, it.infoIndex);

		// Now set the data from the model
		rebuildWeaponRowFromModel(r);
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

		setupEnabledCell(tbl, r, it.infoIndex);
		setupNameCell(tbl, r, it.infoIndex, QString::fromUtf8(it.name.c_str()));
		setupWingOrVarValueCell(tbl, r, it.infoIndex, QString::fromUtf8(_model->getVariableValueAsString(it.infoIndex).c_str()));
		setupExtraCountCell(tbl, r, it.infoIndex);
		setupVarCountCell(tbl, r, it.infoIndex);

		// Now set the data from the model
		rebuildShipVarRowFromModel(r);
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

		setupEnabledCell(tbl, r, it.infoIndex);
		setupNameCell(tbl, r, it.infoIndex, QString::fromUtf8(it.name.c_str()));
		setupWingOrVarValueCell(tbl, r, it.infoIndex, QString::fromUtf8(_model->getVariableValueAsString(it.infoIndex).c_str()));
		setupExtraCountCell(tbl, r, it.infoIndex);
		setupVarCountCell(tbl, r, it.infoIndex);

		// Now set the data from the model
		rebuildWeaponVarRowFromModel(r);
	}

	auto* hh = tbl->horizontalHeader();
	hh->setSectionsClickable(false);
	hh->setHighlightSections(false);
	hh->setSectionsMovable(false);
	hh->setSectionResizeMode(QHeaderView::ResizeToContents);
	tbl->resizeColumnsToContents();
	tbl->setSortingEnabled(true);
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
	const auto it = std::find_if(ships.begin(), ships.end(), [&](const LoadoutItem& li) { return li.infoIndex == classIdx; });
	if (it == ships.end())
		return;

	// Call the refresh helpers for each complex column
	refreshEnabledCell(tbl, row, *it);
	refreshExtraCountCell(tbl, row, *it);
	refreshVarCountCell(tbl, row, *it);
}

void TeamLoadoutDialog::refreshShipRows(const QList<int>& tableRows)
{
	QTableWidget* tbl = ui->shipsList;
	QSignalBlocker blockTbl(tbl); // no recursive itemChanged
	const bool wasSorting = tbl->isSortingEnabled();
	tbl->setSortingEnabled(false);
	for (int r : tableRows)
		rebuildShipRowFromModel(r);
	tbl->setSortingEnabled(wasSorting);
}

void TeamLoadoutDialog::rebuildWeaponRowFromModel(int row)
{
	QTableWidget* tbl = ui->weaponsList;
	if (row < 0 || row >= tbl->rowCount())
		return;

	// Block re-entrancy while we poke items in this row
	util::SignalBlockers blockers(this);

	auto* idItem = tbl->item(row, COL_ENABLED);
	if (!idItem)
		return;

	const int classIdx = idItem->data(static_cast<int>(Role::ClassIndex)).toInt();

	// Pull fresh data
	const auto& weapons = _model->getWeapons();
	const auto it = std::find_if(weapons.begin(), weapons.end(), [&](const LoadoutItem& li) { return li.infoIndex == classIdx; });
	if (it == weapons.end())
		return;

	// Call the refresh helpers for each complex column
	refreshEnabledCell(tbl, row, *it);
	refreshExtraCountCell(tbl, row, *it);
	refreshVarCountCell(tbl, row, *it);
	refreshRequiredCell(tbl, row, *it);
}

void TeamLoadoutDialog::refreshWeaponRows(const QList<int>& tableRows)
{
	QTableWidget* tbl = ui->weaponsList;
	QSignalBlocker blockTbl(tbl); // no recursive itemChanged
	const bool wasSorting = tbl->isSortingEnabled();
	tbl->setSortingEnabled(false);
	for (int r : tableRows)
		rebuildWeaponRowFromModel(r);
	tbl->setSortingEnabled(wasSorting);
}

void TeamLoadoutDialog::rebuildShipVarRowFromModel(int row)
{
	QTableWidget* tbl = ui->shipVarsList;
	if (row < 0 || row >= tbl->rowCount())
		return;

	// Block re-entrancy while we poke items in this row
	util::SignalBlockers blockers(this);

	auto* idItem = tbl->item(row, COL_ENABLED);
	if (!idItem)
		return;

	const int classIdx = idItem->data(static_cast<int>(Role::ClassIndex)).toInt();

	// Pull fresh data
	const auto& varShips = _model->getVarShips();
	const auto it = std::find_if(varShips.begin(), varShips.end(), [&](const LoadoutItem& li) { return li.infoIndex == classIdx; });
	if (it == varShips.end())
		return;

	// Call the refresh helpers for each complex column
	refreshEnabledCell(tbl, row, *it);
	refreshExtraCountCell(tbl, row, *it);
	refreshVarCountCell(tbl, row, *it);
}

void TeamLoadoutDialog::refreshShipVarRows(const QList<int>& tableRows)
{
	QTableWidget* tbl = ui->shipVarsList;
	QSignalBlocker blockTbl(tbl); // no recursive itemChanged
	const bool wasSorting = tbl->isSortingEnabled();
	tbl->setSortingEnabled(false);
	for (int r : tableRows)
		rebuildShipVarRowFromModel(r);
	tbl->setSortingEnabled(wasSorting);
}

void TeamLoadoutDialog::rebuildWeaponVarRowFromModel(int row)
{
	QTableWidget* tbl = ui->weaponVarsList;
	if (row < 0 || row >= tbl->rowCount())
		return;

	// Block re-entrancy while we poke items in this row
	util::SignalBlockers blockers(this);

	auto* idItem = tbl->item(row, COL_ENABLED);
	if (!idItem)
		return;

	const int classIdx = idItem->data(static_cast<int>(Role::ClassIndex)).toInt();

	// Pull fresh data
	const auto& varWeapons = _model->getVarWeapons();
	const auto it = std::find_if(varWeapons.begin(), varWeapons.end(), [&](const LoadoutItem& li) { return li.infoIndex == classIdx; });
	if (it == varWeapons.end())
		return;

	// Call the refresh helpers for each complex column
	refreshEnabledCell(tbl, row, *it);
	refreshExtraCountCell(tbl, row, *it);
	refreshVarCountCell(tbl, row, *it);
}

void TeamLoadoutDialog::refreshWeaponVarRows(const QList<int>& tableRows)
{
	QTableWidget* tbl = ui->weaponVarsList;
	QSignalBlocker blockTbl(tbl); // no recursive itemChanged
	const bool wasSorting = tbl->isSortingEnabled();
	tbl->setSortingEnabled(false);
	for (int r : tableRows)
		rebuildWeaponVarRowFromModel(r);
	tbl->setSortingEnabled(wasSorting);
}

void TeamLoadoutDialog::setupEnabledCell(QTableWidget* tbl, int row, int classIndex)
{
	auto* item = new QTableWidgetItem();
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
	item->setCheckState(Qt::Unchecked); // Initial state, will be updated by refresh
	item->setData(static_cast<int>(Role::ClassIndex), classIndex);
	tbl->setItem(row, COL_ENABLED, item);
}

void TeamLoadoutDialog::setupNameCell(QTableWidget* tbl, int row, int classIndex, const QString& name)
{
	auto* item = new QTableWidgetItem(name);
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	item->setData(static_cast<int>(Role::ClassIndex), classIndex);
	tbl->setItem(row, COL_NAME, item);
}

void TeamLoadoutDialog::setupWingOrVarValueCell(QTableWidget* tbl, int row, int classIndex, const QString& value)
{
	auto* item = new QTableWidgetItem(value);
	item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	item->setData(static_cast<int>(Role::ClassIndex), classIndex);
	tbl->setItem(row, COL_INWINGS_OR_VARVALUE, item);
}

void TeamLoadoutDialog::setupExtraCountCell(QTableWidget* tbl, int row, int classIndex)
{
	// Backing item
	auto* item = new QTableWidgetItem("0");
	item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	item->setData(Qt::EditRole, 0);
	item->setData(static_cast<int>(Role::ClassIndex), classIndex);
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	tbl->setItem(row, COL_EXTRA, item);

	// Cell widget
	auto* spin = new NoWheelSpinBox(tbl);
	spin->setFocusPolicy(Qt::StrongFocus); // Prevents table selection changing during interaction
	spin->setMinimum(0);
	spin->setMaximum(_model->getLoadoutMaxValue());
	spin->setAccelerated(true);
	tbl->setCellWidget(row, COL_EXTRA, spin);

	// Connection
	connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this, [tbl, spin](int v) {
		const QModelIndex idx = tbl->indexAt(spin->mapTo(tbl, QPoint(1, 1)));
		if (!idx.isValid())
			return;
		if (auto* cell = tbl->item(idx.row(), idx.column())) {
			cell->setData(Qt::EditRole, v);
		}
	});
}

void TeamLoadoutDialog::setupVarCountCell(QTableWidget* tbl, int row, int classIndex)
{
	// Backing item
	auto* item = new QTableWidgetItem();
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	item->setData(static_cast<int>(Role::ClassIndex), classIndex);
	item->setData(Qt::EditRole, -1);
	tbl->setItem(row, COL_COUNTVAR, item);

	// Cell widget
	auto* combo = new NoWheelComboBox(tbl);
	combo->setFocusPolicy(Qt::NoFocus); // Prevents table selection changing during interaction
	combo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	const auto& nums = _model->getNumberVarList();
	for (const auto& p : nums) {
		combo->addItem(QString::fromStdString(p.first), p.second);
	}
	tbl->setCellWidget(row, COL_COUNTVAR, combo);

	// Connection
	connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [tbl, combo](int) {
		const QModelIndex idx = tbl->indexAt(combo->mapTo(tbl, QPoint(1, 1)));
		if (!idx.isValid())
			return;
		if (auto* cell = tbl->item(idx.row(), idx.column())) {
			cell->setData(Qt::EditRole, combo->currentData(Qt::UserRole));
		}
	});
}

void TeamLoadoutDialog::setupRequiredCell(QTableWidget* tbl, int row, int classIndex)
{
	auto* item = new QTableWidgetItem();
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
	item->setCheckState(Qt::Unchecked); // Set a default state
	item->setData(static_cast<int>(Role::ClassIndex), classIndex);
	tbl->setItem(row, COL_REQUIRED, item);
}

void TeamLoadoutDialog::refreshEnabledCell(QTableWidget* tbl, int row, const LoadoutItem& modelItem)
{
	if (auto* item = tbl->item(row, COL_ENABLED)) {
		const bool present = modelItem.enabled && (modelItem.extraAllocated > 0 || modelItem.varCountIndex != -1);
		const bool wingOnly = !present && (modelItem.countInWings > 0);
		item->setCheckState(present ? Qt::Checked : wingOnly ? Qt::PartiallyChecked : Qt::Unchecked);
		item->setToolTip(wingOnly ? "In starting wings (pool cleared)" : QString());

		// Do some custom styling for the partially checked state
		// TODO this doesn't really look great. The bigger issue is a tristate checkbox doesn't seem
		// to play nice with the table view. Maybe a custom delegate or directly setting the widget
		// in the cell like we do for the spinbox and combobox would work better? For now the default,
		// barely visible style will have to do.
		/*if (item->checkState() == Qt::PartiallyChecked) {
			// You can use any QColor here
			item->setBackground(QColor(255, 240, 200)); // A light orange/yellow
		} else {
			// For all other states, clear the background to use the default table color.
			item->setBackground(QBrush());
		}*/
	}
}

void TeamLoadoutDialog::refreshExtraCountCell(QTableWidget* tbl, int row, const LoadoutItem& modelItem)
{
	if (auto* item = tbl->item(row, COL_EXTRA)) {
		item->setData(Qt::EditRole, modelItem.extraAllocated);
	}
	if (auto* spin = qobject_cast<NoWheelSpinBox*>(tbl->cellWidget(row, COL_EXTRA))) {
		const bool isEditable = (modelItem.varCountIndex == -1);
		spin->setEnabled(isEditable);
		spin->setToolTip(isEditable ? QString() : "Driven by count variable");
		spin->setValue(modelItem.extraAllocated);
		if (!isEditable) {
			if (auto* le = spin->findChild<QLineEdit*>()) {
				// Defer this call until the next event loop cycle.
				// This gives the QLineEdit time to be fully created.
				QTimer::singleShot(0, le, [le]() { le->setText("-"); });
			}
		}
	}
}

void TeamLoadoutDialog::refreshVarCountCell(QTableWidget* tbl, int row, const LoadoutItem& modelItem)
{
	if (auto* item = tbl->item(row, COL_COUNTVAR)) {
		item->setData(Qt::EditRole, modelItem.varCountIndex);
	}
	if (auto* combo = qobject_cast<NoWheelComboBox*>(tbl->cellWidget(row, COL_COUNTVAR))) {
		const int comboIndex = combo->findData(modelItem.varCountIndex);
		if (comboIndex != -1) {
			combo->setCurrentIndex(comboIndex);
		}
	}
}

void TeamLoadoutDialog::refreshRequiredCell(QTableWidget* tbl, int row, const LoadoutItem& modelItem)
{
	if (auto* item = tbl->item(row, COL_REQUIRED)) {
		item->setCheckState(modelItem.required ? Qt::Checked : Qt::Unchecked);
	}
}

void TeamLoadoutDialog::on_okAndCancelButtons_accepted()
{
	accept();
}

void TeamLoadoutDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

void TeamLoadoutDialog::on_currentTeamComboBox_currentIndexChanged(int index)
{
	if (index < 0) {
		return;
	}
	int team = ui->currentTeamComboBox->itemData(index).toInt();
	_model->setCurrentTeam(team);

	updateUi();
}

void TeamLoadoutDialog::on_copyLoadoutToOtherTeamsButton_clicked()
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

	updateUi();
}

void TeamLoadoutDialog::on_weaponValidationCheckbox_toggled(bool checked)
{
	_model->setSkipValidation(checked);
}

void TeamLoadoutDialog::on_shipsFilterLineEdit_textChanged(const QString& filterText)
{
	// Make the search case insensitive
	const QString lowerFilter = filterText.toLower();

	for (int i = 0; i < ui->shipsList->rowCount(); ++i) {
		// Get the item from the Name column
		QTableWidgetItem* item = ui->shipsList->item(i, COL_NAME);
		if (!item) {
			continue;
		}

		const bool match = item->text().toLower().contains(lowerFilter);

		// Hide the row if it doesn't match the filter
		ui->shipsList->setRowHidden(i, !match);
	}
}

void TeamLoadoutDialog::on_weaponsFilterLineEdit_textChanged(const QString& filterText)
{
	// Make the search case insensitive
	const QString lowerFilter = filterText.toLower();

	for (int i = 0; i < ui->weaponsList->rowCount(); ++i) {
		// Get the item from the Name column
		QTableWidgetItem* item = ui->weaponsList->item(i, COL_NAME);
		if (!item) {
			continue;
		}

		const bool match = item->text().toLower().contains(lowerFilter);

		// Hide the row if it doesn't match the filter
		ui->weaponsList->setRowHidden(i, !match);
	}
}

void TeamLoadoutDialog::on_shipsVarFilterLineEdit_textChanged(const QString& filterText)
{
	// Make the search case insensitive
	const QString lowerFilter = filterText.toLower();

	for (int i = 0; i < ui->shipVarsList->rowCount(); ++i) {
		// Get the item from the Name column
		QTableWidgetItem* item = ui->shipVarsList->item(i, COL_NAME);
		if (!item) {
			continue;
		}

		const bool match = item->text().toLower().contains(lowerFilter);

		// Hide the row if it doesn't match the filter
		ui->shipVarsList->setRowHidden(i, !match);
	}
}

void TeamLoadoutDialog::on_weaponsVarFilterLineEdit_textChanged(const QString& filterText)
{
	// Make the search case insensitive
	const QString lowerFilter = filterText.toLower();

	for (int i = 0; i < ui->weaponVarsList->rowCount(); ++i) {
		// Get the item from the Name column
		QTableWidgetItem* item = ui->weaponVarsList->item(i, COL_NAME);
		if (!item) {
			continue;
		}

		const bool match = item->text().toLower().contains(lowerFilter);

		// Hide the row if it doesn't match the filter
		ui->weaponVarsList->setRowHidden(i, !match);
	}
}

void TeamLoadoutDialog::on_clearShipsListButton_clicked()
{
	ui->shipsList->clearSelection();
}

void TeamLoadoutDialog::on_clearWeaponsListButton_clicked()
{
	ui->weaponsList->clearSelection();
}

void TeamLoadoutDialog::on_clearShipsVarListButton_clicked()
{
	ui->shipVarsList->clearSelection();
}

void TeamLoadoutDialog::on_clearWeaponsVarListButton_clicked()
{
	ui->weaponVarsList->clearSelection();
}

void TeamLoadoutDialog::on_selectAllShipsListButton_clicked()
{
	ui->shipsList->selectAll();
}

void TeamLoadoutDialog::on_selectAllWeaponsListButton_clicked()
{
	ui->weaponsList->selectAll();
}

void TeamLoadoutDialog::on_selectAllShipsVarListButton_clicked()
{
	ui->shipVarsList->selectAll();
}

void TeamLoadoutDialog::on_selectAllWeaponsVarListButton_clicked()
{
	ui->weaponVarsList->selectAll();
}

void TeamLoadoutDialog::on_shipsMultiSelectCheckBox_toggled(bool checked)
{
	if (checked) {
		ui->shipsList->setSelectionMode(QAbstractItemView::MultiSelection);
	} else {
		ui->shipsList->setSelectionMode(QAbstractItemView::SingleSelection);
		if (selectedRowNumbers(ui->shipsList).size() > 1) {
			ui->shipsList->clearSelection();
		}
	}
}

void TeamLoadoutDialog::on_weaponsMultiSelectCheckBox_toggled(bool checked)
{
	if (checked) {
		ui->weaponsList->setSelectionMode(QAbstractItemView::MultiSelection);
	} else {
		ui->weaponsList->setSelectionMode(QAbstractItemView::SingleSelection);
		if (selectedRowNumbers(ui->weaponsList).size() > 1) {
			ui->weaponsList->clearSelection();
		}
	}
}

void TeamLoadoutDialog::on_shipsVarMultiSelectCheckBox_toggled(bool checked)
{
	if (checked) {
		ui->shipVarsList->setSelectionMode(QAbstractItemView::MultiSelection);
	} else {
		ui->shipVarsList->setSelectionMode(QAbstractItemView::SingleSelection);
		if (selectedRowNumbers(ui->shipVarsList).size() > 1) {
			ui->shipVarsList->clearSelection();
		}
	}
}

void TeamLoadoutDialog::on_weaponsVarMultiSelectCheckBox_toggled(bool checked)
{
	if (checked) {
		ui->weaponVarsList->setSelectionMode(QAbstractItemView::MultiSelection);
	} else {
		ui->weaponVarsList->setSelectionMode(QAbstractItemView::SingleSelection);
		if (selectedRowNumbers(ui->weaponVarsList).size() > 1) {
			ui->weaponVarsList->clearSelection();
		}
	}
}

void TeamLoadoutDialog::on_shipsList_itemChanged(QTableWidgetItem* changed)
{
	QTableWidget* tbl = ui->shipsList;
	if (!changed || changed->tableWidget() != tbl)
		return;

	const int col = changed->column();
	const int originRow = changed->row();

	// Snapshot selection before mutating
	auto selRows = selectedRowNumbers(tbl);
	const bool applyToSelection = selRows.size() > 1 && selRows.contains(originRow);
	if (!applyToSelection)
		selRows = {originRow};

	switch (col) {
		case COL_ENABLED: {
			const Qt::CheckState desired = changed->checkState();
			for (int r : selRows) {
				if (auto* chk = tbl->item(r, COL_ENABLED)) {
					const int classIdx = chk->data(static_cast<int>(Role::ClassIndex)).toInt();
					_model->setShipEnabled(classIdx, desired != Qt::Unchecked);
				}
			}
			break;
		}
		case COL_EXTRA: {
			const int newVal = changed->data(Qt::EditRole).toInt();
			const int clamped = std::max(0, newVal);

			for (int r : selRows) {
				if (auto* extraItem = tbl->item(r, COL_EXTRA)) {
					const int classIdx = extraItem->data(static_cast<int>(Role::ClassIndex)).toInt();
					_model->setShipExtra(classIdx, clamped);
				}
			}
			break;
		}
		case COL_COUNTVAR: {
			const int newVarIdx = changed->data(Qt::EditRole).toInt();
			for (int r : selRows) {
				if (auto* idItem = tbl->item(r, COL_COUNTVAR)) {
					const int classIdx = idItem->data(static_cast<int>(Role::ClassIndex)).toInt();
					_model->setShipCountVar(classIdx, newVarIdx);
				}
			}
			break;
		}
		default:
			return; // not a column we handle
	}

	// Now repaint only the affected rows from the model’s *current* state.
	refreshShipRows(selRows);
}

void TeamLoadoutDialog::on_weaponsList_itemChanged(QTableWidgetItem* changed)
{
	QTableWidget* tbl = ui->weaponsList;
	if (!changed || changed->tableWidget() != tbl)
		return;

	const int col = changed->column();
	const int originRow = changed->row();

	// Snapshot selection before mutating
	auto selRows = selectedRowNumbers(tbl);
	const bool applyToSelection = selRows.size() > 1 && selRows.contains(originRow);
	if (!applyToSelection)
		selRows = {originRow};

	switch (col) {
		case COL_ENABLED: {
			const Qt::CheckState desired = changed->checkState();
			for (int r : selRows) {
				if (auto* chk = tbl->item(r, COL_ENABLED)) {
					const int classIdx = chk->data(static_cast<int>(Role::ClassIndex)).toInt();
					_model->setWeaponEnabled(classIdx, desired != Qt::Unchecked);
				}
			}
			break;
		}
		case COL_EXTRA: {
			const int newVal = changed->data(Qt::EditRole).toInt();
			const int clamped = std::max(0, newVal);

			for (int r : selRows) {
				if (auto* extraItem = tbl->item(r, COL_EXTRA)) {
					const int classIdx = extraItem->data(static_cast<int>(Role::ClassIndex)).toInt();
					_model->setWeaponExtra(classIdx, clamped);
				}
			}
			break;
		}
		case COL_COUNTVAR: {
			const int newVarIdx = changed->data(Qt::EditRole).toInt();
			for (int r : selRows) {
				if (auto* idItem = tbl->item(r, COL_COUNTVAR)) {
					const int classIdx = idItem->data(static_cast<int>(Role::ClassIndex)).toInt();
					_model->setWeaponCountVar(classIdx, newVarIdx);
				}
			}
			break;
		}
		case COL_REQUIRED: {
			const bool isRequired = (changed->checkState() == Qt::Checked);
			for (int r : selRows) {
				if (auto* item = tbl->item(r, COL_REQUIRED)) {
					const int classIdx = item->data(static_cast<int>(Role::ClassIndex)).toInt();
					_model->setWeaponRequired(classIdx, isRequired); // Assumes this model function exists
				}
			}
			break;
		}
		default:
			return; // not a column we handle
	}

	// Now repaint only the affected rows from the model’s *current* state.
	refreshWeaponRows(selRows);
}

void TeamLoadoutDialog::on_shipVarsList_itemChanged(QTableWidgetItem* changed)
{
	QTableWidget* tbl = ui->shipVarsList;
	if (!changed || changed->tableWidget() != tbl)
		return;

	const int col = changed->column();
	const int originRow = changed->row();

	// Snapshot selection before mutating
	auto selRows = selectedRowNumbers(tbl);
	const bool applyToSelection = selRows.size() > 1 && selRows.contains(originRow);
	if (!applyToSelection)
		selRows = {originRow};

	switch (col) {
		case COL_ENABLED: {
			const Qt::CheckState desired = changed->checkState();
			for (int r : selRows) {
				if (auto* chk = tbl->item(r, COL_ENABLED)) {
					const int classIdx = chk->data(static_cast<int>(Role::ClassIndex)).toInt();
					_model->setShipVarEnabled(classIdx, desired != Qt::Unchecked);
				}
			}
			break;
		}
		case COL_EXTRA: {
			const int newVal = changed->data(Qt::EditRole).toInt();
			const int clamped = std::max(0, newVal);

			for (int r : selRows) {
				if (auto* extraItem = tbl->item(r, COL_EXTRA)) {
					const int classIdx = extraItem->data(static_cast<int>(Role::ClassIndex)).toInt();
					_model->setShipVarExtra(classIdx, clamped);
				}
			}
			break;
		}
		case COL_COUNTVAR: {
			const int newVarIdx = changed->data(Qt::EditRole).toInt();
			for (int r : selRows) {
				if (auto* idItem = tbl->item(r, COL_COUNTVAR)) {
					const int classIdx = idItem->data(static_cast<int>(Role::ClassIndex)).toInt();
					_model->setShipVarCountVar(classIdx, newVarIdx);
				}
			}
			break;
		}
		default:
			return; // not a column we handle
	}

	// Now repaint only the affected rows from the model’s *current* state.
	refreshShipVarRows(selRows);
}

void TeamLoadoutDialog::on_weaponVarsList_itemChanged(QTableWidgetItem* changed)
{
	QTableWidget* tbl = ui->weaponVarsList;
	if (!changed || changed->tableWidget() != tbl)
		return;

	const int col = changed->column();
	const int originRow = changed->row();

	// Snapshot selection before mutating
	auto selRows = selectedRowNumbers(tbl);
	const bool applyToSelection = selRows.size() > 1 && selRows.contains(originRow);
	if (!applyToSelection)
		selRows = {originRow};

	switch (col) {
		case COL_ENABLED: {
			const Qt::CheckState desired = changed->checkState();
			for (int r : selRows) {
				if (auto* chk = tbl->item(r, COL_ENABLED)) {
					const int classIdx = chk->data(static_cast<int>(Role::ClassIndex)).toInt();
					_model->setWeaponVarEnabled(classIdx, desired != Qt::Unchecked);
				}
			}
			break;
		}
		case COL_EXTRA: {
			const int newVal = changed->data(Qt::EditRole).toInt();
			const int clamped = std::max(0, newVal);

			for (int r : selRows) {
				if (auto* extraItem = tbl->item(r, COL_EXTRA)) {
					const int classIdx = extraItem->data(static_cast<int>(Role::ClassIndex)).toInt();
					_model->setWeaponVarExtra(classIdx, clamped);
				}
			}
			break;
		}
		case COL_COUNTVAR: {
			const int newVarIdx = changed->data(Qt::EditRole).toInt();
			for (int r : selRows) {
				if (auto* idItem = tbl->item(r, COL_COUNTVAR)) {
					const int classIdx = idItem->data(static_cast<int>(Role::ClassIndex)).toInt();
					_model->setWeaponVarCountVar(classIdx, newVarIdx);
				}
			}
			break;
		}
		default:
			return; // not a column we handle
	}

	// Now repaint only the affected rows from the model’s *current* state.
	refreshWeaponVarRows(selRows);
}

} // namespace fso::fred::dialogs
