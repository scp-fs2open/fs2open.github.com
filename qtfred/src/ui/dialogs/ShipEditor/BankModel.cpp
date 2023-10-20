#include "ShipWeaponsDialog.h"

#include <qmimedata.h>
#include <weapon/weapon.h>
namespace fso {
namespace fred {
namespace dialogs {
namespace WeaponsDialog {
BankTreeItem::BankTreeItem(BankTreeItem* parentItem) : m_parentItem(parentItem) {}
BankTreeItem::~BankTreeItem()
{
	qDeleteAll(m_childItems);
}
void BankTreeItem::appendChild(BankTreeItem* item)
{
	m_childItems.append(item);
}
BankTreeItem* BankTreeItem::child(int row) const
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

Qt::ItemFlags BankTreeBank::getFlags(int column, int type)
{
	switch (column) {
	case 0:
		if (type == 0 || type == -1) {
			return Qt::ItemIsDropEnabled | Qt::ItemIsSelectable;
		} else {
			return Qt::ItemIsDropEnabled;
		}
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

Qt::ItemFlags BankTreeLabel::getFlags(int column, int type)
{
	if (type == 1 || type == -1) {
		return Qt::ItemIsSelectable;
	} else {
		return {};
	}
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
	return {};
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

BankTreeItem* BankTreeModel::getItem(const QModelIndex index) const
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
	defaultFlags.setFlag(Qt::ItemIsSelectable, false);

	if (index.isValid()) {
		auto* item = static_cast<BankTreeItem*>(index.internalPointer());
		return item->getFlags(index.column(), typeSelected) | defaultFlags;
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
	const QModelIndex& parent) const
{
	Q_UNUSED(action);
	Q_UNUSED(row);
	Q_UNUSED(parent);

	if (!data->hasFormat("application/weaponid"))
		return false;
	BankTreeItem* item = this->getItem(parent);
	Qt::ItemFlags flags = item->getFlags(column, typeSelected);
	return flags.testFlag(Qt::ItemIsDropEnabled);
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

void BankTreeModel::setWeapon(const QModelIndex& index, int data) const
{
	auto item = dynamic_cast<BankTreeBank*>(this->getItem(index));
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
Qt::ItemFlags BankTreeRoot::getFlags(int column, int type)
{
	Q_UNUSED(type)
	return {};
}
} // namespace WeaponsDialog
} // namespace dialogs
} // namespace fred
} // namespace fso