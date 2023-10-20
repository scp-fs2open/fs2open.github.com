#include "ShipWeaponsDialog.h"

#include "ui_ShipWeaponsDialog.h"

#include <ui/util/SignalBlockers.h>
#include <weapon/weapon.h>

#include <QCloseEvent>
#include <QStringListModel>
#include <QMimeData>

namespace fso {
namespace fred {
namespace dialogs {
namespace WeaponsDialog {
ShipWeaponsDialog::ShipWeaponsDialog(QDialog* parent, EditorViewport* viewport, bool isMultiEdit)
	: QDialog(parent), ui(new Ui::ShipWeaponsDialog()), _model(new ShipWeaponsDialogModel(this, viewport, isMultiEdit)),
	  _viewport(viewport)
{
	ui->setupUi(this);

	connect(ui->radioPrimary, &QRadioButton::toggled, this, [this](bool param) { modeChanged(param, 0); });
	connect(ui->radioSecondary, &QRadioButton::toggled, this, [this](bool param) { modeChanged(param, 1); });
	connect(ui->radioTertiary, &QRadioButton::toggled, this, [this](bool param) { modeChanged(param, 2); });

	// Used to prevent user from selecting subsystems and weapons at the same time
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
	ui->listWeapons->setModel(weapons);
	connect(ui->treeBanks->selectionModel(),
		&QItemSelectionModel::selectionChanged,
		this,
		&ShipWeaponsDialog::processMultiSelect);
	ui->treeBanks->expandAll();
	updateUI();
	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
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
void ShipWeaponsDialog::modeChanged(const bool enabled, const int mode)
{
	if (enabled) {
		if (mode == 0) {
			bankModel = new BankTreeModel(_model->getPrimaryBanks(), this);
			dialogMode = 0;
		} else if (mode == 1) {
			bankModel = new BankTreeModel(_model->getSecondaryBanks(), this);
			dialogMode = 1;
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
		ui->treeBanks->setModel(bankModel);
		connect(ui->treeBanks->selectionModel(),
			&QItemSelectionModel::selectionChanged,
			this,
			&ShipWeaponsDialog::processMultiSelect);
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
}

void ShipWeaponsDialog::processMultiSelect(const QItemSelection& selected, const QItemSelection& deselected)
{
	auto indexes = new QModelIndexList(ui->treeBanks->selectionModel()->selectedIndexes());
	if (!indexes->isEmpty()) {
		BankTreeItem* item = bankModel->getItem(indexes->first());
		if (item) {
			bankModel->typeSelected = -1;
			auto bankTest = dynamic_cast<BankTreeBank*>(item);
			auto labelTest = dynamic_cast<BankTreeLabel*>(item);
			if (bankTest) {
				bankModel->typeSelected = 0;
			} else if (labelTest) {
				bankModel->typeSelected = 1;
			} else {
				bankModel->typeSelected = -1;
			}
		}
	} else {
		bankModel->typeSelected = -1;
	}
}
WeaponModel::WeaponModel(int type)
{
	if (type == 0) {
		for (int i = 0; i < Weapon_info.size(); i++) {
			auto& w = Weapon_info[i];
			if (w.subtype == WP_LASER || w.subtype == WP_BEAM) {
				auto newWeapon = new WeaponItem(i, w.name);
				weapons.push_back(newWeapon);
			}
		}
	} else if (type == 1) {
		for (int i = 0; i < Weapon_info.size(); i++) {
			auto& w = Weapon_info[i];
			if (w.subtype == WP_MISSILE) {
				auto newWeapon = new WeaponItem(i, w.name);
				weapons.push_back(newWeapon);
			}
		}
	}
}
WeaponModel::~WeaponModel() {
	for (auto pointer : weapons) {
		delete pointer;
	}
}
int WeaponModel::rowCount(const QModelIndex& parent) const
{
	return static_cast<int>(weapons.size());
}
QVariant WeaponModel::data(const QModelIndex& index, int role) const
{
	if (role == Qt::DisplayRole) {
		const QString out = weapons[index.row()]->name.c_str();
		return out;
	}
	if (role == Qt::UserRole) {
		const int id = weapons[index.row()]->id;
		return id;
	}
	return {};
}
QMimeData* WeaponModel::mimeData(const QModelIndexList& indexes) const
{
	auto mimeData = new QMimeData();
	QByteArray encodedData;
	QDataStream stream(&encodedData, QIODevice::WriteOnly);

	for (const QModelIndex& index : indexes) {
		if (index.isValid()) {
			int id = data(index, Qt::UserRole).toInt();
			stream << id;
		}
	}

	mimeData->setData("application/weaponid", encodedData);

	return mimeData;

}
WeaponItem::WeaponItem(const int id, const SCP_string &name) : name(name), id(id) {}
} // namespace WeaponsDialog
} // namespace dialogs
} // namespace fred
} // namespace fso