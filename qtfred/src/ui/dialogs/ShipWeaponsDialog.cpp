#include "ShipWeaponsDialog.h"

#include "ui_ShipWeaponsDialog.h"

#include <qmimedata.h>
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
		util::SignalBlockers blockers(this);
		bankModel = new BankTreeModel(_model->getPrimaryBanks(), this);
		ui->radioPrimary->setChecked(true);
		dialogMode = 0;
	} else if (!_model->getSecondaryBanks().empty()) {
		util::SignalBlockers blockers(this);
		bankModel = new BankTreeModel(_model->getSecondaryBanks(), this);
		ui->radioSecondary->setChecked(true);
		dialogMode = 1;
	} else {
		Error("No Valid Weapon banks on ship");
	}
	ui->treeBanks->setModel(bankModel);
	ui->treeBanks->expandAll();
	updateUI();
	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

ShipWeaponsDialog::~ShipWeaponsDialog() = default;

void ShipWeaponsDialog::closeEvent(QCloseEvent* event)
{
	accept();
	QDialog::closeEvent(event);
}
void ShipWeaponsDialog::modeChanged(bool enabled, int mode)
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
		ui->treeBanks->expandAll();
	}
	updateUI();
}
void ShipWeaponsDialog::updateUI()
{
	util::SignalBlockers blockers(this);
	ui->radioPrimary->setEnabled(!_model->getPrimaryBanks().empty());
	ui->radioSecondary->setEnabled(!_model->getSecondaryBanks().empty());
	ui->radioTertiary->setEnabled(false);
}

BankTreeItem::BankTreeItem(BankTreeItem* parentItem) : m_parentItem(parentItem) {}
BankTreeItem::~BankTreeItem()
{
	qDeleteAll(m_childItems);
}
void BankTreeItem::appendChild(BankTreeItem* item)
{
	m_childItems.append(item);
}
BankTreeItem* BankTreeItem::child(int row)
{
	if (row < 0 || row >= m_childItems.size())
		return nullptr;
	return m_childItems.at(row);
}
int BankTreeItem::childCount() const
{
	return m_childItems.count();
}
int BankTreeItem::childNumber() const
{
	if (m_parentItem)
		return m_parentItem->m_childItems.indexOf(const_cast<BankTreeItem*>(this));
	return 0;
}
BankTreeItem* BankTreeItem::parentItem()
{
	return m_parentItem;
}

bool BankTreeItem::insertLabel(int position, const QString& newName, Banks* newBanks)
{
	if (position < 0 || position > m_childItems.size())
		return false;

	auto* item = new BankTreeLabel(newName, newBanks, this);
	m_childItems.insert(position, item);

	return true;
}

bool BankTreeItem::insertBank(int position, Bank* newBank)
{
	if (position < 0 || position > m_childItems.size())
		return false;

	auto* item = new BankTreeBank(newBank, this);
	m_childItems.insert(position, item);

	return true;
}

QString BankTreeItem::getName() const
{
	return name;
}

int BankTreeBank::getId() const
{
	return bank->getWeaponId();
}

BankTreeBank::BankTreeBank(Bank* bank, BankTreeItem* parentItem) : BankTreeItem(parentItem)
{
	this->bank = bank;
	switch (bank->getWeaponId()) {
	case -2:
		this->name = "CONFLICT";
		break;
	case -1:
		this->name = "None";
		break;
	default:
		this->name = Weapon_info[bank->getWeaponId()].name;
	}
}

QVariant BankTreeBank::data(int column) const
{
	switch (column) {
	case 0:
		return name;
		break;
	case 1:
		return bank->getAmmo();
		break;
	default:
		return {};
	}
}

Qt::ItemFlags BankTreeBank::getFlags(int column)
{
	switch (column) {
	case 0:
		return Qt::ItemIsDropEnabled | Qt::ItemIsSelectable;
		break;
	case 1:
		return Qt::ItemIsEditable;
		break;
	default:
		return {};
	}
}

void BankTreeBank::setWeapon(int id)
{
	bank->setWeapon(id);
	if (id == -1) {
		name = "None";
	} else {
		name = Weapon_info[id].name;
	}
}

void BankTreeBank::setAmmo(int value)
{
	Assert(bank != nullptr);
	bank->setAmmo(value);
}

BankTreeLabel::BankTreeLabel(const QString& name, Banks* banks, BankTreeItem* parentItem) : BankTreeItem(parentItem)
{
	this->name = name + " (" + Ai_class_names[banks->aiClass] + ")";
	this->banks = banks;
	this->name = name;
}

QVariant BankTreeLabel::data(int column) const
{
	switch (column) {
	case 0:
		return name;
		break;
	default:
		return {};
	}
}

Qt::ItemFlags BankTreeLabel::getFlags(int column)
{
	return Qt::ItemIsSelectable;
}

void BankTreeLabel::setAIClass(int value)
{
	Assert(banks != nullptr);
	banks->aiClass = value;
}

bool BankTreeLabel::setData(int column, const QVariant& value)
{
	return false;
}

bool BankTreeBank::setData(int column, const QVariant& value)
{
	switch (column) {
	case 1:
		setAmmo(value.toInt());
		return true;
		break;
	default:
		return false;
	}
}
BankTreeModel::BankTreeModel(const SCP_vector<Banks*>& data, QObject* parent) : QAbstractItemModel(parent)
{
	rootItem = new BankTreeRoot();

	setupModelData(data, rootItem);
}

void BankTreeModel::setupModelData(const SCP_vector<Banks*>& data, BankTreeItem* parent)
{
	for (auto banks : data) {
		parent->insertLabel(parent->childCount(), banks->name.c_str(), banks);
		BankTreeItem* currentParent = parent->child(parent->childCount() - 1);
		for (auto bank : banks->banks) {
			currentParent->insertBank(currentParent->childCount(), bank);
		}
	}
}

QVariant BankTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole) {
			switch (section) {
			case 0:
				return tr("Bank Name/Weapon");
			case 1:
				return tr("Ammo");
			default:
				return QString("");
			}
		}
	return QVariant();
}

BankTreeModel::~BankTreeModel()
{
	delete rootItem;
}

int BankTreeModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent);
	return 2;
}

QVariant BankTreeModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid()) {
		return {};
	}

	if (role != Qt::DisplayRole && role != Qt::EditRole)
		return {};

	BankTreeItem* item = getItem(index);

	return item->data(index.column());
}

BankTreeItem* BankTreeModel::getItem(const QModelIndex& index) const
{
	if (index.isValid()) {
		auto* item = static_cast<BankTreeItem*>(index.internalPointer());
		if (item)
			return item;
	}
	return rootItem;
}

int BankTreeModel::rowCount(const QModelIndex& parent) const
{
	if (parent.isValid() && parent.column() > 0)
		return 0;

	const BankTreeItem* parentItem = getItem(parent);

	return parentItem ? parentItem->childCount() : 0;
}

Qt::ItemFlags BankTreeModel::flags(const QModelIndex& index) const
{
	Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

	if (index.isValid()) {
		auto* item = static_cast<BankTreeItem*>(index.internalPointer());
		return item->getFlags(index.column()) | defaultFlags;
	} else {
		return Qt::NoItemFlags;
	}
}

QModelIndex BankTreeModel::index(int row, int column, const QModelIndex& parent) const
{
	if (parent.isValid() && parent.column() != 0)
		return {};

	BankTreeItem* parentItem = getItem(parent);
	if (!parentItem)
		return {};

	BankTreeItem* childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);
	return {};
}
QModelIndex BankTreeModel::parent(const QModelIndex& index) const
{
	if (!index.isValid())
		return {};
	BankTreeItem* childItem = getItem(index);
	BankTreeItem* parentItem = childItem ? childItem->parentItem() : nullptr;

	if (parentItem == rootItem || !parentItem)
		return {};
	return createIndex(parentItem->childNumber(), 0, parentItem);
}
QStringList BankTreeModel::mimeTypes() const
{
	QStringList types;
	types << "application/weaponid";
	return types;
}

bool BankTreeModel::canDropMimeData(const QMimeData* data,
	Qt::DropAction action,
	int row,
	int column,
	const QModelIndex& parent)
{
	Q_UNUSED(action);
	Q_UNUSED(row);
	Q_UNUSED(parent);

	if (!data->hasFormat("application/weaponid"))
		return false;
	BankTreeItem* item = this->getItem(parent);
	Qt::ItemFlags flags = item->getFlags(column);
	if (flags.testFlag(Qt::ItemIsDropEnabled)) {
		return true;
	} else {
		return false;
	}
}
bool BankTreeModel::dropMimeData(const QMimeData* data,
	Qt::DropAction action,
	int row,
	int column,
	const QModelIndex& parent)
{
	if (!canDropMimeData(data, action, row, column, parent))
		return false;

	if (action == Qt::IgnoreAction)
		return true;

	int beginRow;

	if (row != -1)
		beginRow = row;
	else if (parent.isValid())
		beginRow = parent.row();
	else
		return false;

	QByteArray encodedData = data->data("application/weaponid");
	QDataStream stream(&encodedData, QIODevice::ReadOnly);
	while (!stream.atEnd()) {
		int id = 0;
		stream >> id;
		setWeapon(parent, id);
	}
	return true;
}

void BankTreeModel::setWeapon(const QModelIndex& index, int data)
{
	BankTreeBank* item = dynamic_cast<BankTreeBank*>(this->getItem(index));
	Assert(item != nullptr);
	if (item != nullptr) {
		item->setWeapon(data);
	}
}


bool BankTreeRoot::setData(int column, const QVariant& value)
{
	return false;
}
QVariant BankTreeRoot::data(int column) const
{
	switch (column) {
	case 0:
		return "Name/Weapon";
		break;
	case 1:
		return "Ammo";
		break;
	default:
		return {};
	}
}
Qt::ItemFlags BankTreeRoot::getFlags(int column)
{
	return {};
}
} // namespace dialogs
} // namespace fred
} // namespace fso