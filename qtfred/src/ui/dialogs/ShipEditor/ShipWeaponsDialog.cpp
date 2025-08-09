#include "ShipWeaponsDialog.h"

#include "WeaponsTBLViewer.h"
#include "ui_ShipWeaponsDialog.h"

#include <mission/util.h>
#include <ui/util/SignalBlockers.h>
#include <weapon/weapon.h>

#include <QCloseEvent>
#include <QStringListModel>
namespace fso::fred::dialogs {
ShipWeaponsDialog::ShipWeaponsDialog(QDialog* parent, EditorViewport* viewport, bool isMultiEdit)
	: QDialog(parent), ui(new Ui::ShipWeaponsDialog()), _model(new ShipWeaponsDialogModel(this, viewport, isMultiEdit)),
	  _viewport(viewport)
{
	ui->setupUi(this);

	// connect(this, &QDialog::accepted, _model.get(), &ShipWeaponsDialogModel::apply);

	// Build the model of ship weapons and set inital mode.
	if (!_model->getPrimaryBanks().empty()) {
		const util::SignalBlockers blockers(this);
		bankModel = new BankTreeModel(_model->getPrimaryBanks(), this);
		ui->radioPrimary->setChecked(true);
		dialogMode = 0;
		weapons = new WeaponModel(0);
	} else if (!_model->getSecondaryBanks().empty()) {
		const util::SignalBlockers blockers(this);
		bankModel = new BankTreeModel(_model->getSecondaryBanks(), this);
		ui->radioSecondary->setChecked(true);
		dialogMode = 1;
		weapons = new WeaponModel(1);
	} else {
		Error("No Valid Weapon banks on ship");
	}
	ui->treeBanks->setModel(bankModel);
	ui->listWeapons->setModel(weapons);

	connect(ui->treeBanks->selectionModel()->model(),
		&QAbstractItemModel::dataChanged,
		this,
		&ShipWeaponsDialog::updateUI);
	// Update the UI whenever selections change
	connect(ui->treeBanks->selectionModel(),
		&QItemSelectionModel::selectionChanged,
		this,
		&ShipWeaponsDialog::updateUI);
	connect(ui->listWeapons->selectionModel(),
		&QItemSelectionModel::selectionChanged,
		this,
		&ShipWeaponsDialog::updateUI);

	// Setup ai combo box
	// connect(ui->AICombo,
	// static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
	// this,
	//&ShipWeaponsDialog::aiClassChanged);

	// Resize Bank view
	ui->treeBanks->expandAll();
	ui->treeBanks->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	updateUI();
}

ShipWeaponsDialog::~ShipWeaponsDialog()
{
	delete bankModel;
	delete weapons;
}

void ShipWeaponsDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void ShipWeaponsDialog::reject()
{
	// Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void ShipWeaponsDialog::closeEvent(QCloseEvent* event)
{
	reject();
	event->ignore();
}
void ShipWeaponsDialog::on_setAllButton_clicked()
{
	for (auto& index : ui->treeBanks->selectionModel()->selectedIndexes()) {
		bankModel->setWeapon(index, ui->listWeapons->currentIndex().data(Qt::UserRole).toInt());
	}
}
void ShipWeaponsDialog::on_tblButton_clicked()
{
	if (ui->listWeapons->currentIndex().data(Qt::UserRole).toInt() >= 0) {
		auto dialog = new WeaponsTBLViewer(this, _viewport, ui->listWeapons->currentIndex().data(Qt::UserRole).toInt());
		dialog->show();
	} else {
		return;
	}
}
void ShipWeaponsDialog::on_radioPrimary_toggled(bool checked)
{
	modeChanged(checked, 0);
}
void ShipWeaponsDialog::on_radioSecondary_toggled(bool checked)
{
	modeChanged(checked, 1);
}
void ShipWeaponsDialog::on_radioTertiary_toggled(bool checked)
{
	modeChanged(checked, 2);
}
void ShipWeaponsDialog::on_aiCombo_currentIndexChanged(int index)
{
	aiClassChanged(index);
}
void ShipWeaponsDialog::modeChanged(const bool enabled, const int mode)
{
	if (enabled) {
		if (mode == 0) {
			bankModel = new BankTreeModel(_model->getPrimaryBanks(), this);
			dialogMode = 0;
			delete weapons;
			weapons = new WeaponModel(0);
			ui->listWeapons->setModel(weapons);
		} else if (mode == 1) {
			bankModel = new BankTreeModel(_model->getSecondaryBanks(), this);
			dialogMode = 1;
			delete weapons;
			weapons = new WeaponModel(1);
			ui->listWeapons->setModel(weapons);
		} else if (mode == 2) {
			// bankModel = new BankTreeModel(_model->getTertiaryBanks(), this);
			dialogMode = 2;
		} else {
			_viewport->dialogProvider->showButtonDialog(DialogType::Error,
				"Illegal Mode",
				"Somehow an Illegal mode has been set. Get a coder.\n Illegal mode is " + std::to_string(mode),
				{DialogButton::Ok});
			ui->radioPrimary->toggled(true);
			bankModel = new BankTreeModel(_model->getPrimaryBanks(), this);
			dialogMode = 0;
		}
		// Reconnect beacuse the model has changed
		connect(ui->treeBanks->selectionModel()->model(),
			&QAbstractItemModel::dataChanged,
			this,
			&ShipWeaponsDialog::updateUI);
		connect(ui->treeBanks->selectionModel(),
			&QItemSelectionModel::selectionChanged,
			this,
			&ShipWeaponsDialog::updateUI);
		connect(ui->listWeapons->selectionModel(),
			&QItemSelectionModel::selectionChanged,
			this,
			&ShipWeaponsDialog::updateUI);
		ui->treeBanks->setModel(bankModel);
		ui->treeBanks->expandAll();
	}
	updateUI();
}
void ShipWeaponsDialog::updateUI()
{
	const util::SignalBlockers blockers(this);
	// Radio Buttons
	ui->radioPrimary->setEnabled(!_model->getPrimaryBanks().empty());
	ui->radioSecondary->setEnabled(!_model->getSecondaryBanks().empty());
	ui->radioTertiary->setEnabled(false);

	ui->treeBanks->expandAll();
	// Setall button
	if (ui->treeBanks->getTypeSelected() == 0) {
		ui->setAllButton->setEnabled(true);
	} else {
		ui->setAllButton->setEnabled(false);
	}
	// Change AI Button
	if (ui->treeBanks->getTypeSelected() == 1) {
		ui->aiButton->setEnabled(true);
	} else {
		ui->aiButton->setEnabled(false);
	}
	// AI Combo Box
	ui->aiCombo->clear();
	for (int i = 0; i < Num_ai_classes; i++) {
		ui->aiCombo->addItem(Ai_class_names[i], QVariant(i));
	}
	ui->aiCombo->setCurrentIndex(ui->aiCombo->findData(m_currentAI));
	if (ui->listWeapons->selectionModel()->hasSelection() &&
		ui->listWeapons->currentIndex().data(Qt::UserRole).toInt() != -1) {
		ui->tblButton->setEnabled(true);
	} else {
		ui->tblButton->setEnabled(false);
	}
}

void ShipWeaponsDialog::aiClassChanged(const int index)
{
	m_currentAI = ui->aiCombo->itemData(index).toInt();
}

void ShipWeaponsDialog::on_aiButton_clicked()
{
	for (auto& index : ui->treeBanks->selectionModel()->selectedIndexes()) {
		bankModel->setData(index, m_currentAI);
	}
}

void ShipWeaponsDialog::on_buttonClose_clicked()
{
	accept();
}

} // namespace fso::fred::dialogs