#include "TeamLoadoutDialog.h"
#include "ui_TeamLoadoutDialog.h"

#include <QtWidgets/QMenuBar>
#include <qlist.h>
#include <qtablewidget.h>
#include <QListWidget>
#include <QMessageBox>
#include <mission/util.h>
#include "ui/util/SignalBlockers.h"

constexpr int TABLE_MODE = 0;
constexpr int VARIABLE_MODE = 1;

namespace fso::fred::dialogs {

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
	QTableWidget* tbl = ui->shipsList;

	// Hardening: read-only & nice selection
	tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tbl->setSelectionBehavior(QAbstractItemView::SelectRows);
	tbl->setSelectionMode(QAbstractItemView::SingleSelection);
	tbl->setSortingEnabled(false);
	tbl->clearContents();

	// Column indices (align with your Designer headers)
	enum {
		COL_ENABLED = 0,
		COL_NAME = 1,
		COL_INWINGS = 2,
		COL_EXTRA = 3,
		COL_COUNTVAR = 4
	};
	constexpr int kRoleClassIndex = Qt::UserRole + 1;

	const auto& rows = _model->getShips(); // const SCP_vector<LoadoutItem>&
	tbl->setRowCount(static_cast<int>(rows.size()));

	for (int r = 0; r < static_cast<int>(rows.size()); ++r) {
		const auto& it = rows[r];
		const bool present = it.enabled && (it.extraAllocated > 0 || it.varCountIndex != -1);
		const bool wingOnly = !present && (it.countInWings > 0);

		// 0) Enabled (user-editable checkbox with wing-only behavior handled in slot)
		{
			auto* item = new QTableWidgetItem();
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);

			item->setCheckState(present ? Qt::Checked : wingOnly ? Qt::PartiallyChecked : Qt::Unchecked);
			item->setData(kRoleClassIndex, it.infoIndex); // stash class index

			if (wingOnly)
				item->setToolTip("In starting wings (pool cleared)");
			tbl->setItem(r, COL_ENABLED, item);
		}

		// 1) Ship (name)
		{
			auto* item = new QTableWidgetItem(QString::fromUtf8(it.name.c_str()));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(kRoleClassIndex, it.infoIndex);
			tbl->setItem(r, COL_NAME, item);
		}

		// 2) In Wings
		{
			auto* item = new QTableWidgetItem(QString::number(it.countInWings));
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(kRoleClassIndex, it.infoIndex);
			tbl->setItem(r, COL_INWINGS, item);
		}

		// 3) Extra (literal pool, not including wings)
		{
			auto* item = new QTableWidgetItem(QString::number(it.extraAllocated));
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(kRoleClassIndex, it.infoIndex);
			tbl->setItem(r, COL_EXTRA, item);
		}

		// 4) Extra Variable (number-var name, if any)
		{
			QString varName = _model->getVariableName(it.varCountIndex).c_str();
			auto* item = new QTableWidgetItem(varName);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(kRoleClassIndex, it.infoIndex);
			tbl->setItem(r, COL_COUNTVAR, item);
		}
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
	QTableWidget* tbl = ui->weaponsList;

	tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tbl->setSelectionBehavior(QAbstractItemView::SelectRows);
	tbl->setSelectionMode(QAbstractItemView::SingleSelection);
	tbl->setSortingEnabled(false);
	tbl->clearContents();

	enum {
		COL_ENABLED = 0,
		COL_NAME = 1,
		COL_INWINGS = 2,
		COL_EXTRA = 3,
		COL_COUNTVAR = 4,
		COL_REQUIRED = 5
	};
	constexpr int kRoleClassIndex = Qt::UserRole + 1;

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
			item->setData(kRoleClassIndex, it.infoIndex); // stash class index

			if (wingOnly)
				item->setToolTip("In starting wings (pool cleared)");
			tbl->setItem(r, COL_ENABLED, item);
		}
		// 1) Name
		{
			auto* item = new QTableWidgetItem(QString::fromUtf8(it.name.c_str()));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(kRoleClassIndex, it.infoIndex);
			tbl->setItem(r, COL_NAME, item);
		}
		// 2) Required
		{
			auto* item = new QTableWidgetItem(it.required ? QString::fromUtf8("X") : QString());
			item->setTextAlignment(Qt::AlignCenter);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(kRoleClassIndex, it.infoIndex);
			tbl->setItem(r, COL_REQUIRED, item);
		}
		// 3) In Wings
		{
			auto* item = new QTableWidgetItem(QString::number(it.countInWings));
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(kRoleClassIndex, it.infoIndex);
			tbl->setItem(r, COL_INWINGS, item);
		}
		// 4) Extra
		{
			auto* item = new QTableWidgetItem(QString::number(it.extraAllocated));
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(kRoleClassIndex, it.infoIndex);
			tbl->setItem(r, COL_EXTRA, item);
		}
		// 5) Amount Variable
		{
			QString varName = _model->getVariableName(it.varCountIndex).c_str();
			auto* item = new QTableWidgetItem(varName);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(kRoleClassIndex, it.infoIndex);
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
	QTableWidget* tbl = ui->shipVarsList;

	tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tbl->setSelectionBehavior(QAbstractItemView::SelectRows);
	tbl->setSelectionMode(QAbstractItemView::SingleSelection);
	tbl->setSortingEnabled(false);
	tbl->clearContents();

	enum {
		COL_ENABLED = 0,
		COL_NAME = 1,
		COL_EXTRA = 2,
		COL_COUNTVAR = 3
	};
	constexpr int kRoleStringVarIndex = Qt::UserRole + 2;

	const auto rows = _model->getVarShips(); // snapshot
	tbl->setRowCount((int)rows.size());

	for (int r = 0; r < (int)rows.size(); ++r) {
		const auto& it = rows[r];

		// 0) Enabled
		{
			auto* item = new QTableWidgetItem();
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
			item->setCheckState(it.enabled ? Qt::Checked : Qt::Unchecked);
			item->setData(kRoleStringVarIndex, it.infoIndex); // stash class index
			tbl->setItem(r, COL_ENABLED, item);
		}
		// 1) Var name
		{
			SCP_string name = it.name;
			SCP_string var_text = _model->getVariableValueAsString(it.infoIndex);
			name += " [" + var_text + "]";
			auto* item = new QTableWidgetItem(QString::fromUtf8(name.c_str()));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(kRoleStringVarIndex, it.infoIndex);
			tbl->setItem(r, COL_NAME, item);
		}
		// 2) Extra
		{
			auto* item = new QTableWidgetItem(QString::number(it.extraAllocated));
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(kRoleStringVarIndex, it.infoIndex);
			tbl->setItem(r, COL_EXTRA, item);
		}
		// 3) Amount var
		{
			QString varName = _model->getVariableName(it.varCountIndex).c_str();
			auto* item = new QTableWidgetItem(varName);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(kRoleStringVarIndex, it.infoIndex);
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
	QTableWidget* tbl = ui->weaponVarsList;

	tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tbl->setSelectionBehavior(QAbstractItemView::SelectRows);
	tbl->setSelectionMode(QAbstractItemView::SingleSelection);
	tbl->setSortingEnabled(false);
	tbl->clearContents();

	enum {
		COL_ENABLED = 0,
		COL_NAME = 1,
		COL_EXTRA = 2,
		COL_COUNTVAR = 3
	};
	constexpr int kRoleStringVarIndex = Qt::UserRole + 2;

	const auto rows = _model->getVarWeapons(); // snapshot
	tbl->setRowCount((int)rows.size());

	for (int r = 0; r < (int)rows.size(); ++r) {
		const auto& it = rows[r];

		// 0) Enabled
		{
			auto* item = new QTableWidgetItem();
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
			item->setCheckState(it.enabled ? Qt::Checked : Qt::Unchecked);
			item->setData(kRoleStringVarIndex, it.infoIndex); // stash class index
			tbl->setItem(r, COL_ENABLED, item);
		}
		// 1) Var name
		{
			SCP_string name = it.name;
			SCP_string var_text = _model->getVariableValueAsString(it.infoIndex);
			name += " [" + var_text + "]";
			auto* item = new QTableWidgetItem(QString::fromUtf8(name.c_str()));
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(kRoleStringVarIndex, it.infoIndex);
			tbl->setItem(r, COL_NAME, item);
		}
		// 2) Extra
		{
			auto* item = new QTableWidgetItem(QString::number(it.extraAllocated));
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(kRoleStringVarIndex, it.infoIndex);
			tbl->setItem(r, COL_EXTRA, item);
		}
		// 3) Amount var
		{
			QString varName = _model->getVariableName(it.varCountIndex).c_str();
			auto* item = new QTableWidgetItem(varName);
			item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
			item->setData(kRoleStringVarIndex, it.infoIndex);
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
