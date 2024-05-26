#include "ShipWeaponsDialog.h"

#include "ui_ShipWeaponsDialog.h"
#include "WeaponsTBLViewer.h"

#include <ui/util/SignalBlockers.h>
#include <weapon/weapon.h>

#include <QCloseEvent>
#include <QStringListModel>
namespace fso {
namespace fred {
namespace dialogs {
ShipWeaponsDialog::ShipWeaponsDialog(QDialog* parent, EditorViewport* viewport, bool isMultiEdit)
	: QDialog(parent), ui(new Ui::ShipWeaponsDialog()), _model(new ShipWeaponsDialogModel(this, viewport, isMultiEdit)),
	  _viewport(viewport)
{
	ui->setupUi(this);

	connect(ui->radioPrimary, &QRadioButton::toggled, this, [this](bool param) { modeChanged(param, 0); });
	connect(ui->radioSecondary, &QRadioButton::toggled, this, [this](bool param) { modeChanged(param, 1); });
	connect(ui->radioTertiary, &QRadioButton::toggled, this, [this](bool param) { modeChanged(param, 2); });


	connect(this, &QDialog::accepted, _model.get(), &ShipWeaponsDialogModel::apply);
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
	connect(ui->treeBanks->selectionModel(),
		&QItemSelectionModel::selectionChanged,
		this,
		&ShipWeaponsDialog::updateUI);
	connect(ui->AICombo,
		static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		this,
		&ShipWeaponsDialog::aiClassChanged);
	ui->listWeapons->setModel(weapons);
	ui->treeBanks->expandAll();
	connect(ui->listWeapons->selectionModel(),
		&QItemSelectionModel::selectionChanged,
		this,
		&ShipWeaponsDialog::updateUI);
	ui->treeBanks->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	updateUI();
}

ShipWeaponsDialog::~ShipWeaponsDialog() {
	delete bankModel;
	delete weapons;
}

void ShipWeaponsDialog::closeEvent(QCloseEvent* event)
{
	accept();
	QDialog::closeEvent(event);
}
void ShipWeaponsDialog::on_setAllButton_clicked()
{
	int test = 0;
	for (auto& index : ui->treeBanks->selectionModel()->selectedIndexes()) {
		bankModel->setWeapon(index, ui->listWeapons->currentIndex().data(Qt::UserRole).toInt());
	}
}
void ShipWeaponsDialog::on_TBLButton_clicked() {
	if (ui->listWeapons->currentIndex().data(Qt::UserRole).toInt() >= 0) {
		auto dialog = new WeaponsTBLViewer(this, _viewport, ui->listWeapons->currentIndex().data(Qt::UserRole).toInt());
		dialog->show();
	} else {
		return;
	}
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
				"Somehow an Illegal mode has been set. Get a coder.\n Illegal mode is " + mode,
				{DialogButton::Ok});
			ui->radioPrimary->toggled(true);
			bankModel = new BankTreeModel(_model->getPrimaryBanks(), this);
			dialogMode = 0;
		}
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
	ui->radioPrimary->setEnabled(!_model->getPrimaryBanks().empty());
	ui->radioSecondary->setEnabled(!_model->getSecondaryBanks().empty());
	ui->radioTertiary->setEnabled(false);
	ui->treeBanks->expandAll();

	if (ui->treeBanks->getTypeSelected() == 0) {
		ui->setAllButton->setEnabled(true);
	} else {
		ui->setAllButton->setEnabled(false);
	}

	if (ui->treeBanks->getTypeSelected() == 1) {
		ui->AIButton->setEnabled(true);
	} else {
		ui->AIButton->setEnabled(false);
	}
	ui->AICombo->clear();
	for (int i = 0; i < Num_ai_classes; i++) {
		ui->AICombo->addItem(Ai_class_names[i], QVariant(i));
	}
	ui->AICombo->setCurrentIndex(ui->AICombo->findData(m_currentAI));
	if (ui->listWeapons->selectionModel()->hasSelection() &&
		ui->listWeapons->currentIndex().data(Qt::UserRole).toInt() != -1) {
		ui->TBLButton->setEnabled(true);
	} else {
		ui->TBLButton->setEnabled(false);
	}
}

void ShipWeaponsDialog::aiClassChanged(const int index) {
	m_currentAI = ui->AICombo->itemData(index).toInt();
}

void ShipWeaponsDialog::on_AIButton_clicked() {
	for (auto& index : ui->treeBanks->selectionModel()->selectedIndexes()) {
		bankModel->setData(index, m_currentAI);
	}
}

} // namespace dialogs
} // namespace fred
} // namespace fso