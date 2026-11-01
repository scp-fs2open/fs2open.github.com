#include "ShipWeaponsDialog.h"

#include "ui_ShipWeaponsDialog.h"

#include <mission/util.h>
#include <ui/dialogs/TableViewerDialog.h>
#include <ui/util/SignalBlockers.h>
#include <weapon/weapon.h>

#include <QBrush>
#include <QCloseEvent>
#include <QHeaderView>
#include <QMimeData>
#include <QSpinBox>
#include <QStyledItemDelegate>

namespace fso::fred::dialogs {

namespace {
// QStandardItem collapses Qt::EditRole into Qt::DisplayRole, so we can't have a formatted string
// in DisplayRole alongside an int in EditRole. Use a custom role for the spinbox's value.
constexpr int AmmoValueRole = Qt::UserRole + 7;

QString formatAmmoDisplay(int current, int max)
{
	return QString::number(current) + "/" + QString::number(max);
}

QString formatAmmoConflict(int max)
{
	return QStringLiteral("--/") + QString::number(max);
}

// bankTree's drop handler expects the "application/weaponid" MIME type with a single int payload.
// QStandardItemModel's built-in mimeData uses application/x-qabstractitemmodeldatalist instead, so
// we override here to keep the existing contract.
class WeaponListModel : public QStandardItemModel {
  public:
	using QStandardItemModel::QStandardItemModel;

	QStringList mimeTypes() const override
	{
		return {QLatin1String(MIME_WEAPON_ID)};
	}

	QMimeData* mimeData(const QModelIndexList& indexes) const override
	{
		auto* mime = new QMimeData();
		QByteArray bytes;
		QDataStream stream(&bytes, QIODevice::WriteOnly);
		for (const QModelIndex& index : indexes) {
			if (index.isValid()) {
				stream << index.data(Qt::UserRole).toInt();
			}
		}
		mime->setData(QLatin1String(MIME_WEAPON_ID), bytes);
		return mime;
	}
};

class AmmoSpinBoxDelegate : public QStyledItemDelegate {
  public:
	using QStyledItemDelegate::QStyledItemDelegate;

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/,
		const QModelIndex& index) const override
	{
		auto* editor = new QSpinBox(parent);
		editor->setMinimum(0);
		const QModelIndex col0 = index.sibling(index.row(), 0);
		editor->setMaximum(col0.data(BankItemMaxAmmoRole).toInt());
		editor->setFrame(false);
		editor->setAutoFillBackground(true);
		return editor;
	}

	void setEditorData(QWidget* editor, const QModelIndex& index) const override
	{
		static_cast<QSpinBox*>(editor)->setValue(index.data(AmmoValueRole).toInt());
	}

	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
	{
		auto* spin = static_cast<QSpinBox*>(editor);
		spin->interpretText();
		model->setData(index, spin->value(), AmmoValueRole);
	}
};
} // namespace

ShipWeaponsDialog::ShipWeaponsDialog(QDialog* parent, EditorViewport* viewport, bool isMultiEdit)
	: QDialog(parent), ui(new Ui::ShipWeaponsDialog()),
	  _model(new ShipWeaponsDialogModel(this, viewport, isMultiEdit)), _viewport(viewport)
{
	ui->setupUi(this);

	if (_model->getPrimaryBanks().empty() && _model->getSecondaryBanks().empty()) {
		Error("No Valid Weapon banks on ship");
	}

	// Tertiary banks are not implemented in the game engine yet, but the UI scaffolding (the
	// tertiaryPage in the .ui file, the Mode::Tertiary enum value, the default branch in
	// banksForMode) is intentionally preserved so that wiring the engine side later is a small
	// follow-up rather than a re-build.
	ui->tabWidget->removeTab(2);

	initTab(_primary, Primary);
	initTab(_secondary, Secondary);

	// Default to the first tab that has banks.
	if (_model->getPrimaryBanks().empty() && !_model->getSecondaryBanks().empty()) {
		ui->tabWidget->setCurrentIndex(1);
	}

	updateUI();
}

ShipWeaponsDialog::~ShipWeaponsDialog() = default;

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
	tab.weapons = new WeaponListModel(this);
	loadBankModel(tab);
	loadWeaponList(tab);
	tab.tree->setModel(tab.bankModel);
	tab.list->setModel(tab.weapons);
	tab.tree->expandAll();
	tab.tree->setHeaderHidden(true);
	tab.tree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
	tab.tree->setItemDelegateForColumn(1, new AmmoSpinBoxDelegate(this));

	tab.aiCombo->clear();
	const auto aiNames = _model->getAiClassNames();
	for (int i = 0; i < static_cast<int>(aiNames.size()); i++) {
		tab.aiCombo->addItem(QString::fromUtf8(aiNames[i].c_str()), QVariant(i));
	}

	connect(tab.tree->selectionModel(), &QItemSelectionModel::selectionChanged, this,
		[this, &tab](const QItemSelection&, const QItemSelection&) { updateTabUI(tab); });
	connect(tab.list->selectionModel(), &QItemSelectionModel::selectionChanged, this,
		[this, &tab](const QItemSelection&, const QItemSelection&) { updateTabUI(tab); });
	connect(tab.setAllButton, &QPushButton::clicked, this, [this, &tab]() { onSetAllClicked(tab); });
	connect(tab.aiButton, &QPushButton::clicked, this, [this, &tab]() { onAiButtonClicked(tab); });
	connect(tab.tblButton, &QPushButton::clicked, this, [this, &tab]() { onTblButtonClicked(tab); });
	connect(tab.bankModel, &QStandardItemModel::itemChanged, this,
		[this, &tab](QStandardItem* item) { onBankItemChanged(tab, item); });
	connect(tab.tree, &bankTree::weaponDroppedFromList, this,
		[this, &tab](const QModelIndex& target, int weaponId) {
			onWeaponDroppedFromList(tab, target, weaponId);
		});
	connect(tab.tree, &bankTree::weaponMoved, this,
		[this, &tab](const QModelIndex& target, int weaponId, int sourceBanksId, int sourceBankId, bool isCopy) {
			onWeaponMoved(tab, target, weaponId, sourceBanksId, sourceBankId, isCopy);
		});

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
		// Tertiary banks are unsupported by the engine; placeholder mode returns no banks.
		return {};
	}
}

SCP_string ShipWeaponsDialog::banksLabel(const Banks* banks) const
{
	if (banks->getName() == "Pilot") {
		return banks->getName();
	}
	const int ai = banks->getAiClass();
	if (ai < 0) {
		return banks->getName() + " (Mixed AI)";
	}
	return banks->getName() + " ( " + _model->getAiClassName(ai) + " ) ";
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
		nameItem->setData(true, BankItemIsLabelRole);
		nameItem->setData(banks->getId(), BankItemIdRole);
		nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
		auto labelAmmoItem = new QStandardItem();
		labelAmmoItem->setFlags(Qt::NoItemFlags);
		tab.bankModel->appendRow({nameItem, labelAmmoItem});
		for (auto bank : banks->getBanks()) {
			auto weaponItem = new QStandardItem();
			const SCP_string weaponName = _model->getWeaponName(bank->getWeaponId());
			weaponItem->setData(QString::fromUtf8(weaponName.c_str()), Qt::DisplayRole);
			weaponItem->setData(bank->getWeaponId(), Qt::UserRole);
			weaponItem->setData(false, BankItemIsLabelRole);
			weaponItem->setData(bank->getBankId(), BankItemIdRole);
			weaponItem->setData(bank->getMaxAmmo(), BankItemMaxAmmoRole);
			Qt::ItemFlags weaponFlags = weaponItem->flags() & ~Qt::ItemIsEditable;
			// Only slots that have a real weapon can be dragged out.
			if (bank->getWeaponId() >= 0) {
				weaponFlags |= Qt::ItemIsDragEnabled;
			} else {
				weaponFlags &= ~Qt::ItemIsDragEnabled;
			}
			weaponItem->setFlags(weaponFlags);

			auto ammoItem = new QStandardItem();
			Qt::ItemFlags ammoFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
			if (bank->getMaxAmmo() > 0) {
				if (bank->getAmmo() == -2) {
					// Multi-edit ammo CONFLICT: show --/max, still editable so user can resolve.
					ammoItem->setData(formatAmmoConflict(bank->getMaxAmmo()), Qt::DisplayRole);
					ammoItem->setData(0, AmmoValueRole);
				} else {
					ammoItem->setData(formatAmmoDisplay(bank->getAmmo(), bank->getMaxAmmo()), Qt::DisplayRole);
					ammoItem->setData(bank->getAmmo(), AmmoValueRole);
				}
				ammoFlags |= Qt::ItemIsEditable;
			} else {
				ammoItem->setData(QString(), Qt::DisplayRole);
				ammoItem->setData(QVariant(), AmmoValueRole);
			}
			ammoItem->setFlags(ammoFlags);
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
	// reject() hides the dialog when it actually closes. Let that close
	// proceed (so a dialog created with WA_DeleteOnClose is destroyed),
	// and only veto it when reject() decided to keep the dialog open (e.g.
	// the user cancelled the unsaved-changes prompt).
	if (isVisible()) {
		event->ignore();
	} else {
		event->accept();
	}
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
	const int comboIdx = tab.aiCombo->currentIndex();
	if (comboIdx < 0) {
		return; // No AI picked in the combo (still blank from a mixed selection).
	}
	const int newAi = tab.aiCombo->itemData(comboIdx).toInt();
	TabState& otherTab = (&tab == &_primary) ? _secondary : _primary;
	bool anyChanged = false;
	for (const QModelIndex& index : tab.tree->selectionModel()->selectedIndexes()) {
		if (index.column() != 0) {
			continue;
		}
		Banks* banks = banksForIndex(tab, index);
		if (banks == nullptr || banks->getName() == "Pilot") {
			continue;
		}
		if (banks->getAiClass() == newAi) {
			continue;
		}
		banks->setAiClass(newAi);
		refreshBankItem(tab, index);
		// A turret with both primary and secondary banks has a Banks in each tab pointing at the
		// same ship_subsys::weapons.ai_class. Keep them in lockstep so saveShip() can't have one
		// tab silently overwrite the other, and so the other tab's label stays accurate.
		for (Banks* sibling : banksForMode(otherTab.mode)) {
			if (sibling == nullptr || sibling->getName() != banks->getName()) {
				continue;
			}
			if (sibling->getAiClass() != newAi) {
				sibling->setAiClass(newAi);
			}
			const QModelIndex siblingIdx = indexForBanks(otherTab, sibling->getId());
			if (siblingIdx.isValid()) {
				refreshBankItem(otherTab, siblingIdx);
			}
			break;
		}
		anyChanged = true;
	}
	if (anyChanged) {
		updateTabUI(otherTab);
		_model->notifyChanged();
	}
}

void ShipWeaponsDialog::onTblButtonClicked(TabState& tab)
{
	const int wc = tab.list->currentIndex().data(Qt::UserRole).toInt();
	if (wc >= 0) {
		const SCP_string name = _model->getWeaponName(wc);
		auto dialog = new TableViewerDialog(this, _viewport, "Weapon TBL Data", "weapons.tbl", "*-wep.tbm", name.c_str());
		dialog->show();
	}
}

void ShipWeaponsDialog::loadWeaponList(TabState& tab)
{
	tab.weapons->clear();
	const auto listType = (tab.mode == Primary) ? WeaponListType::Primary : WeaponListType::Secondary;
	for (const WeaponItem& item : _model->getAvailableWeapons(listType)) {
		auto* row = new QStandardItem();
		row->setData(QString::fromUtf8(item.name.c_str()), Qt::DisplayRole);
		row->setData(item.id, Qt::UserRole);
		if (!item.allowed) {
			row->setData(QBrush(Qt::gray), Qt::ForegroundRole);
			row->setData(QStringLiteral("Not in this ship class's allowed weapons list."), Qt::ToolTipRole);
		}
		row->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
		tab.weapons->appendRow(row);
	}
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
	// point of truth. Other selected banks (turrets) collectively determine the combo state:
	//  - all same AI -> show that value
	//  - any mixed across ships, or differing AIs across selected turrets -> combo blank, user picks
	bool aiEditable = (selType == bankTree::SelectionType::Bank);
	int displayedAi = -1;
	bool combinedInitialized = false;
	bool combinedMixed = false;
	if (selType == bankTree::SelectionType::Bank) {
		for (const QModelIndex& idx : tab.tree->selectionModel()->selectedIndexes()) {
			if (idx.column() != 0) {
				continue;
			}
			Banks* banks = banksForIndex(tab, idx);
			if (banks == nullptr) {
				continue;
			}
			if (banks->getName() == "Pilot") {
				aiEditable = false;
				continue;
			}
			const int ai = banks->getAiClass();
			if (ai < 0) {
				combinedMixed = true;
			} else if (!combinedInitialized) {
				displayedAi = ai;
				combinedInitialized = true;
			} else if (displayedAi != ai) {
				combinedMixed = true;
			}
		}
	}
	tab.aiGroup->setEnabled(aiEditable);
	if (aiEditable && combinedInitialized && !combinedMixed) {
		tab.aiCombo->setCurrentIndex(tab.aiCombo->findData(displayedAi));
	} else {
		// Mixed across selection, or only Pilot selected (combo greyed): clear the combo.
		tab.aiCombo->setCurrentIndex(-1);
	}

	const bool hasWeaponSelection = tab.list->selectionModel()->hasSelection() &&
		tab.list->currentIndex().data(Qt::UserRole).toInt() != -1;
	tab.tblButton->setEnabled(hasWeaponSelection);
}

Banks* ShipWeaponsDialog::banksForIndex(const TabState& tab, const QModelIndex& idx) const
{
	if (!idx.isValid()) {
		return nullptr;
	}
	if (!idx.data(BankItemIsLabelRole).toBool()) {
		return nullptr;
	}
	const int banksId = idx.data(BankItemIdRole).toInt();
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
	if (idx.data(BankItemIsLabelRole).toBool()) {
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
	const int bankId = idx.data(BankItemIdRole).toInt();
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
	if (col0.data(BankItemIsLabelRole).toBool()) {
		Banks* banks = banksForIndex(tab, col0);
		if (banks != nullptr) {
			const SCP_string name = banksLabel(banks);
			tab.bankModel->setData(col0, name.c_str(), Qt::DisplayRole);
		}
		tab.internalUpdate = false;
		return;
	}
	Bank* bank = bankForIndex(tab, col0);
	if (bank == nullptr) {
		tab.internalUpdate = false;
		return;
	}
	const SCP_string name = _model->getWeaponName(bank->getWeaponId());
	tab.bankModel->setData(col0, QString::fromUtf8(name.c_str()), Qt::DisplayRole);
	tab.bankModel->setData(col0, bank->getWeaponId(), Qt::UserRole);
	tab.bankModel->setData(col0, bank->getMaxAmmo(), BankItemMaxAmmoRole);
	if (QStandardItem* weaponItem = tab.bankModel->itemFromIndex(col0)) {
		Qt::ItemFlags f = weaponItem->flags();
		if (bank->getWeaponId() >= 0) {
			f |= Qt::ItemIsDragEnabled;
		} else {
			f &= ~Qt::ItemIsDragEnabled;
		}
		weaponItem->setFlags(f);
	}

	const QModelIndex col1 = idx.sibling(idx.row(), 1);
	if (col1.isValid()) {
		QStandardItem* ammoItem = tab.bankModel->itemFromIndex(col1);
		if (ammoItem != nullptr) {
			Qt::ItemFlags ammoFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
			if (bank->getMaxAmmo() > 0) {
				if (bank->getAmmo() == -2) {
					ammoItem->setData(formatAmmoConflict(bank->getMaxAmmo()), Qt::DisplayRole);
					ammoItem->setData(0, AmmoValueRole);
				} else {
					ammoItem->setData(formatAmmoDisplay(bank->getAmmo(), bank->getMaxAmmo()), Qt::DisplayRole);
					ammoItem->setData(bank->getAmmo(), AmmoValueRole);
				}
				ammoFlags |= Qt::ItemIsEditable;
			} else {
				ammoItem->setData(QString(), Qt::DisplayRole);
				ammoItem->setData(QVariant(), AmmoValueRole);
			}
			ammoItem->setFlags(ammoFlags);
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
	if (!col0.isValid() || col0.data(BankItemIsLabelRole).toBool()) {
		return;
	}
	Bank* bank = bankForIndex(tab, col0);
	if (bank == nullptr) {
		return;
	}
	bool ok = false;
	int requested = item->data(AmmoValueRole).toInt(&ok);
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
	item->setData(formatAmmoDisplay(clamped, bank->getMaxAmmo()), Qt::DisplayRole);
	item->setData(clamped, AmmoValueRole);
	tab.internalUpdate = false;
}

void ShipWeaponsDialog::onWeaponDroppedFromList(TabState& tab, const QModelIndex& target, int weaponId)
{
	Bank* bank = bankForIndex(tab, target);
	if (bank == nullptr) {
		return;
	}
	if (bank->getWeaponId() == weaponId) {
		return;
	}
	bank->setWeapon(weaponId);
	refreshBankItem(tab, target);
	_model->notifyChanged();
}

void ShipWeaponsDialog::onWeaponMoved(TabState& tab, const QModelIndex& target, int weaponId,
	int sourceBanksId, int sourceBankId, bool isCopy)
{
	Bank* targetBank = bankForIndex(tab, target);
	if (targetBank == nullptr) {
		return;
	}

	Bank* sourceBank = nullptr;
	for (Banks* banks : banksForMode(tab.mode)) {
		if (banks->getId() != sourceBanksId) {
			continue;
		}
		for (Bank* b : banks->getBanks()) {
			if (b->getBankId() == sourceBankId) {
				sourceBank = b;
				break;
			}
		}
		break;
	}
	if (sourceBank == nullptr || sourceBank == targetBank) {
		return;
	}

	targetBank->setWeapon(weaponId);
	if (!isCopy) {
		sourceBank->setWeapon(-1);
	}

	refreshBankItem(tab, target);
	const QModelIndex sourceIdx = indexForBank(tab, sourceBanksId, sourceBankId);
	if (sourceIdx.isValid()) {
		refreshBankItem(tab, sourceIdx);
	}
	_model->notifyChanged();
}

QModelIndex ShipWeaponsDialog::indexForBank(const TabState& tab, int banksId, int bankId)
{
	if (tab.bankModel == nullptr) {
		return {};
	}
	const int topRows = tab.bankModel->rowCount();
	for (int i = 0; i < topRows; i++) {
		const QModelIndex parent = tab.bankModel->index(i, 0);
		if (parent.data(BankItemIdRole).toInt() != banksId) {
			continue;
		}
		const int childRows = tab.bankModel->rowCount(parent);
		for (int j = 0; j < childRows; j++) {
			const QModelIndex child = tab.bankModel->index(j, 0, parent);
			if (child.data(BankItemIdRole).toInt() == bankId) {
				return child;
			}
		}
	}
	return {};
}

QModelIndex ShipWeaponsDialog::indexForBanks(const TabState& tab, int banksId)
{
	if (tab.bankModel == nullptr) {
		return {};
	}
	const int topRows = tab.bankModel->rowCount();
	for (int i = 0; i < topRows; i++) {
		const QModelIndex parent = tab.bankModel->index(i, 0);
		if (parent.data(BankItemIdRole).toInt() == banksId) {
			return parent;
		}
	}
	return {};
}

} // namespace fso::fred::dialogs
