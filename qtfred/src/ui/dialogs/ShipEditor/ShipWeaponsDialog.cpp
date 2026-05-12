#include "ShipWeaponsDialog.h"

#include "ui_ShipWeaponsDialog.h"

#include <mission/util.h>
#include <ui/dialogs/TableViewerDialog.h>
#include <ui/util/SignalBlockers.h>
#include <weapon/weapon.h>

#include <QCloseEvent>
#include <QHeaderView>

namespace fso::fred::dialogs {

ShipWeaponsDialog::ShipWeaponsDialog(QDialog* parent, EditorViewport* viewport, bool isMultiEdit)
	: QDialog(parent), ui(new Ui::ShipWeaponsDialog()),
	  _model(new ShipWeaponsDialogModel(this, viewport, isMultiEdit)), _viewport(viewport)
{
	ui->setupUi(this);

	if (_model->getPrimaryBanks().empty() && _model->getSecondaryBanks().empty()) {
		Error("No Valid Weapon banks on ship");
	}

	// Tertiary banks are not implemented in the game engine so hide the placeholder tab until they are.
	ui->tabWidget->removeTab(2);

	initTab(_primary, Primary);
	initTab(_secondary, Secondary);

	// Default to the first tab that has banks.
	if (_model->getPrimaryBanks().empty() && !_model->getSecondaryBanks().empty()) {
		ui->tabWidget->setCurrentIndex(1);
	}

	updateUI();
}

ShipWeaponsDialog::~ShipWeaponsDialog()
{
	delete _primary.bankModel;
	delete _primary.weapons;
	delete _secondary.bankModel;
	delete _secondary.weapons;
}

void ShipWeaponsDialog::initTab(TabState& tab, Mode mode)
{
	tab.mode = mode;

	if (mode == Primary) {
		tab.tree = ui->primaryTreeBanks;
		tab.list = ui->primaryListWeapons;
		tab.setAllButton = ui->primarySetAllButton;
		tab.tblButton = ui->primaryTblButton;
		tab.aiButton = ui->primaryAiButton;
		tab.aiCombo = ui->primaryAiCombo;
		tab.aiGroup = ui->primaryAiGroup;
	} else {
		tab.tree = ui->secondaryTreeBanks;
		tab.list = ui->secondaryListWeapons;
		tab.setAllButton = ui->secondarySetAllButton;
		tab.tblButton = ui->secondaryTblButton;
		tab.aiButton = ui->secondaryAiButton;
		tab.aiCombo = ui->secondaryAiCombo;
		tab.aiGroup = ui->secondaryAiGroup;
	}

	const util::SignalBlockers blockers(this);

	tab.bankModel = new QStandardItemModel(this);
	tab.weapons = new WeaponModel(static_cast<int>(mode), _model->getShipClass(), _model->isBigShip());
	loadBankModel(tab);
	tab.tree->setModel(tab.bankModel);
	tab.list->setModel(tab.weapons);
	tab.tree->expandAll();
	tab.tree->setHeaderHidden(true);
	tab.tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

	tab.aiCombo->clear();
	for (int i = 0; i < Num_ai_classes; i++) {
		tab.aiCombo->addItem(Ai_class_names[i], QVariant(i));
	}

	connect(tab.tree->selectionModel(), &QItemSelectionModel::selectionChanged, this,
		[this, &tab](const QItemSelection&, const QItemSelection&) { updateTabUI(tab); });
	connect(tab.list->selectionModel(), &QItemSelectionModel::selectionChanged, this,
		[this, &tab](const QItemSelection&, const QItemSelection&) { updateTabUI(tab); });
	connect(tab.setAllButton, &QPushButton::clicked, this, [this, &tab]() { onSetAllClicked(tab); });
	connect(tab.aiButton, &QPushButton::clicked, this, [this, &tab]() { onAiButtonClicked(tab); });
	connect(tab.tblButton, &QPushButton::clicked, this, [this, &tab]() { onTblButtonClicked(tab); });
	connect(tab.aiCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		[this, &tab](int idx) { onAiComboChanged(tab, idx); });
	connect(tab.bankModel, &QStandardItemModel::itemChanged, this,
		[this, &tab](QStandardItem* item) { onBankItemChanged(tab, item); });

	const auto banks = banksForMode(mode);
	const int tabIndex = (mode == Primary) ? 0 : 1;
	ui->tabWidget->setTabEnabled(tabIndex, !banks.empty());
}

SCP_vector<Banks*> ShipWeaponsDialog::banksForMode(Mode mode) const
{
	switch (mode) {
	case Primary:
		return _model->getPrimaryBanks();
	case Secondary:
		return _model->getSecondaryBanks();
	default:
		return {};
	}
}

SCP_string ShipWeaponsDialog::banksLabel(const Banks* banks) const
{
	if (banks->getName() == "Pilot") {
		return banks->getName();
	}
	return banks->getName() + " ( " + Ai_class_names[banks->getAiClass()] + " ) ";
}

void ShipWeaponsDialog::loadBankModel(TabState& tab)
{
	const util::SignalBlockers blockers(this);
	tab.internalUpdate = true;
	tab.bankModel->removeRows(0, tab.bankModel->rowCount());
	tab.bankModel->setColumnCount(2);
	for (auto banks : banksForMode(tab.mode)) {
		auto nameItem = new QStandardItem();
		const SCP_string name = banksLabel(banks);
		nameItem->setData(name.c_str(), Qt::DisplayRole);
		nameItem->setData(true, Qt::UserRole + 2);
		nameItem->setData(banks->getId(), Qt::UserRole + 3);
		nameItem->setData(banks->getAiClass(), Qt::UserRole + 6);
		nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
		auto labelAmmoItem = new QStandardItem();
		labelAmmoItem->setFlags(Qt::NoItemFlags);
		tab.bankModel->appendRow({nameItem, labelAmmoItem});
		for (auto bank : banks->getBanks()) {
			auto weaponItem = new QStandardItem();
			QString weaponName;
			switch (bank->getWeaponId()) {
			case -2:
				weaponName = "CONFLICT";
				break;
			case -1:
				weaponName = "None";
				break;
			default:
				weaponName = Weapon_info[bank->getWeaponId()].name;
			}
			weaponItem->setData(weaponName, Qt::DisplayRole);
			weaponItem->setData(bank->getWeaponId(), Qt::UserRole);
			weaponItem->setData(false, Qt::UserRole + 2);
			weaponItem->setData(bank->getBankId(), Qt::UserRole + 3);
			weaponItem->setData(bank->getAmmo(), Qt::UserRole + 4);
			weaponItem->setData(bank->getMaxAmmo(), Qt::UserRole + 5);
			weaponItem->setFlags(weaponItem->flags() & ~Qt::ItemIsEditable);

			auto ammoItem = new QStandardItem();
			if (bank->getMaxAmmo() > 0) {
				ammoItem->setData(bank->getAmmo(), Qt::DisplayRole);
				ammoItem->setData(bank->getAmmo(), Qt::EditRole);
				ammoItem->setFlags(ammoItem->flags() | Qt::ItemIsEditable);
			} else {
				ammoItem->setFlags(Qt::NoItemFlags);
			}
			nameItem->appendRow({weaponItem, ammoItem});
		}
	}
	tab.internalUpdate = false;
}

void ShipWeaponsDialog::accept()
{
	if (_model->apply()) {
		QDialog::accept();
	}
}

void ShipWeaponsDialog::reject()
{
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject();
	}
}

void ShipWeaponsDialog::closeEvent(QCloseEvent* event)
{
	reject();
	event->ignore();
}

void ShipWeaponsDialog::on_okAndCancelButtons_accepted()
{
	accept();
}

void ShipWeaponsDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

void ShipWeaponsDialog::onSetAllClicked(TabState& tab)
{
	const QModelIndex weaponIdx = tab.list->currentIndex();
	if (!weaponIdx.isValid()) {
		return;
	}
	const int weaponId = weaponIdx.data(Qt::UserRole).toInt();
	bool anyChanged = false;
	for (const QModelIndex& index : tab.tree->selectionModel()->selectedIndexes()) {
		Bank* bank = bankForIndex(tab, index);
		if (bank == nullptr) {
			continue;
		}
		if (bank->getWeaponId() == weaponId) {
			continue;
		}
		bank->setWeapon(weaponId);
		refreshBankItem(tab, index);
		anyChanged = true;
	}
	if (anyChanged) {
		_model->notifyChanged();
	}
}

void ShipWeaponsDialog::onAiButtonClicked(TabState& tab)
{
	bool anyChanged = false;
	for (const QModelIndex& index : tab.tree->selectionModel()->selectedIndexes()) {
		Banks* banks = banksForIndex(tab, index);
		if (banks == nullptr) {
			continue;
		}
		if (banks->getAiClass() == tab.currentAI) {
			continue;
		}
		banks->setAiClass(tab.currentAI);
		refreshBankItem(tab, index);
		anyChanged = true;
	}
	if (anyChanged) {
		_model->notifyChanged();
	}
}

void ShipWeaponsDialog::onTblButtonClicked(TabState& tab)
{
	const int wc = tab.list->currentIndex().data(Qt::UserRole).toInt();
	if (wc >= 0) {
		auto dialog = new TableViewerDialog(this, _viewport, "Weapon TBL Data", "weapons.tbl", "*-wep.tbm",
			Weapon_info[wc].name);
		dialog->show();
	}
}

void ShipWeaponsDialog::onAiComboChanged(TabState& tab, int index)
{
	tab.currentAI = tab.aiCombo->itemData(index).toInt();
}

void ShipWeaponsDialog::updateUI()
{
	updateTabUI(_primary);
	updateTabUI(_secondary);
}

void ShipWeaponsDialog::updateTabUI(TabState& tab)
{
	const util::SignalBlockers blockers(this);

	tab.tree->expandAll();

	const auto selType = tab.tree->getSelectionType();
	tab.setAllButton->setEnabled(selType == bankTree::SelectionType::Weapon);

	// Pilot AI maps to Ships[].weapons.ai_class, which the Ship Editor also owns. Keep that single
	// point of truth. A slight regression from old FRED but a more clear separation of responsibilities.
	bool aiEditable = (selType == bankTree::SelectionType::Bank);
	if (selType == bankTree::SelectionType::Bank) {
		Banks* firstBanks = nullptr;
		for (const QModelIndex& idx : tab.tree->selectionModel()->selectedIndexes()) {
			if (idx.column() != 0) {
				continue;
			}
			Banks* banks = banksForIndex(tab, idx);
			if (banks == nullptr) {
				continue;
			}
			if (firstBanks == nullptr) {
				firstBanks = banks;
			}
			if (banks->getName() == "Pilot") {
				aiEditable = false;
			}
		}
		if (firstBanks != nullptr) {
			tab.currentAI = firstBanks->getAiClass();
		}
	}
	tab.aiGroup->setEnabled(aiEditable);
	tab.aiCombo->setCurrentIndex(tab.aiCombo->findData(tab.currentAI));

	const bool hasWeaponSelection = tab.list->selectionModel()->hasSelection() &&
		tab.list->currentIndex().data(Qt::UserRole).toInt() != -1;
	tab.tblButton->setEnabled(hasWeaponSelection);
}

Banks* ShipWeaponsDialog::banksForIndex(const TabState& tab, const QModelIndex& idx) const
{
	if (!idx.isValid()) {
		return nullptr;
	}
	if (!idx.data(Qt::UserRole + 2).toBool()) {
		return nullptr;
	}
	const int banksId = idx.data(Qt::UserRole + 3).toInt();
	for (Banks* banks : banksForMode(tab.mode)) {
		if (banks->getId() == banksId) {
			return banks;
		}
	}
	return nullptr;
}

Bank* ShipWeaponsDialog::bankForIndex(const TabState& tab, const QModelIndex& idx) const
{
	if (!idx.isValid()) {
		return nullptr;
	}
	if (idx.data(Qt::UserRole + 2).toBool()) {
		return nullptr;
	}
	const QModelIndex parent = idx.parent();
	if (!parent.isValid()) {
		return nullptr;
	}
	Banks* parentBanks = banksForIndex(tab, parent);
	if (parentBanks == nullptr) {
		return nullptr;
	}
	const int bankId = idx.data(Qt::UserRole + 3).toInt();
	for (Bank* bank : parentBanks->getBanks()) {
		if (bank->getBankId() == bankId) {
			return bank;
		}
	}
	return nullptr;
}

void ShipWeaponsDialog::refreshBankItem(TabState& tab, const QModelIndex& idx)
{
	if (!idx.isValid() || tab.bankModel == nullptr) {
		return;
	}
	const QModelIndex col0 = idx.sibling(idx.row(), 0);
	tab.internalUpdate = true;
	if (col0.data(Qt::UserRole + 2).toBool()) {
		Banks* banks = banksForIndex(tab, col0);
		if (banks != nullptr) {
			const SCP_string name = banksLabel(banks);
			tab.bankModel->setData(col0, name.c_str(), Qt::DisplayRole);
			tab.bankModel->setData(col0, banks->getAiClass(), Qt::UserRole + 6);
		}
		tab.internalUpdate = false;
		return;
	}
	Bank* bank = bankForIndex(tab, col0);
	if (bank == nullptr) {
		tab.internalUpdate = false;
		return;
	}
	QString name;
	switch (bank->getWeaponId()) {
	case -2:
		name = "CONFLICT";
		break;
	case -1:
		name = "None";
		break;
	default:
		name = Weapon_info[bank->getWeaponId()].name;
		break;
	}
	tab.bankModel->setData(col0, name, Qt::DisplayRole);
	tab.bankModel->setData(col0, bank->getWeaponId(), Qt::UserRole);
	tab.bankModel->setData(col0, bank->getAmmo(), Qt::UserRole + 4);
	tab.bankModel->setData(col0, bank->getMaxAmmo(), Qt::UserRole + 5);

	const QModelIndex col1 = idx.sibling(idx.row(), 1);
	if (col1.isValid()) {
		QStandardItem* ammoItem = tab.bankModel->itemFromIndex(col1);
		if (ammoItem != nullptr) {
			if (bank->getMaxAmmo() > 0) {
				ammoItem->setData(bank->getAmmo(), Qt::DisplayRole);
				ammoItem->setData(bank->getAmmo(), Qt::EditRole);
				ammoItem->setFlags(ammoItem->flags() | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			} else {
				ammoItem->setData(QVariant(), Qt::DisplayRole);
				ammoItem->setData(QVariant(), Qt::EditRole);
				ammoItem->setFlags(Qt::NoItemFlags);
			}
		}
	}
	tab.internalUpdate = false;
}

void ShipWeaponsDialog::onBankItemChanged(TabState& tab, QStandardItem* item)
{
	if (item == nullptr || tab.internalUpdate) {
		return;
	}
	if (item->column() != 1) {
		return;
	}
	const QModelIndex col0 = item->index().sibling(item->row(), 0);
	if (!col0.isValid() || col0.data(Qt::UserRole + 2).toBool()) {
		return;
	}
	Bank* bank = bankForIndex(tab, col0);
	if (bank == nullptr) {
		return;
	}
	bool ok = false;
	int requested = item->data(Qt::EditRole).toInt(&ok);
	if (!ok) {
		requested = bank->getAmmo();
	}
	const int clamped = std::max(0, std::min(requested, bank->getMaxAmmo()));
	if (clamped != bank->getAmmo()) {
		bank->setAmmo(clamped);
		_model->notifyChanged();
	}
	// Always write back the canonical value. This covers the case where the user typed an out-of-range
	// number and our clamp differs from what they entered.
	tab.internalUpdate = true;
	item->setData(clamped, Qt::DisplayRole);
	item->setData(clamped, Qt::EditRole);
	tab.internalUpdate = false;
}

} // namespace fso::fred::dialogs
