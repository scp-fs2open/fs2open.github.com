#include "ShipWeaponsDialog.h"

#include "ui_ShipWeaponsDialog.h"

#include <qmimedata.h>
#include <ui/util/SignalBlockers.h>
#include <weapon/weapon.h>

#include <QCloseEvent>
#include <QStringListModel>
#include "ShipEditorDialog.h"

namespace fso {
namespace fred {
namespace dialogs {
ShipWeaponsDialog::ShipWeaponsDialog(QDialog* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::ShipWeaponsDialog()), _model(new ShipWeaponsDialogModel(this, viewport)),
	  _viewport(viewport)
{
	parentDialog = dynamic_cast<ShipEditorDialog*>(parent);
	Assert(parentDialog);
	ui->setupUi(this);

	connect(ui->radioPrimary, &QRadioButton::toggled, this, [this](bool param) { modeChanged(param, 0); });
	connect(this, &QDialog::accepted, _model.get(), &ShipWeaponsDialogModel::apply);
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

void ShipWeaponsDialog::showEvent(QShowEvent* event)
{
	_model->initializeData(parentDialog->getIfMultipleShips());

	QDialog::showEvent(event);
}
void ShipWeaponsDialog::modeChanged(bool enabled, int mode)
{
	if (enabled) {
		if (mode == 0) {
			bankModel = new BankTreeModel(_model->getPrimaryBanks(), this);
		} else if (mode == 1) {
			bankModel = new BankTreeModel(_model->getSecondaryBanks(), this);
		} else if (mode == 2) {
			bankModel = new BankTreeModel(_model->getTertiaryBanks(), this);
		} else {
			_viewport->dialogProvider->showButtonDialog(DialogType::Error,
				"Illegal Mode",
				"Somehow an Illegal mode has been set. Get a coder.\n Illegal mode is " + mode,
				{DialogButton::Ok});
		}
	}
}
void ShipWeaponsDialog::updateUI()
{
	util::SignalBlockers blockers(this);
}

BankTreeItem::BankTreeItem(const QString& type, const QString& name, Banks* banks, BankTreeItem* parentItem)
	: m_parentItem(parentItem)
{
	this->type = type;
	this->name = name + " (" + Ai_class_names[banks->aiClass] + ")";
	this->banks = banks;
	this->name = name;
	bank = nullptr;
}
BankTreeItem::BankTreeItem(const QString& type, Bank* bank, BankTreeItem* parentItem) : m_parentItem(parentItem)
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
	this->type = type;
}
BankTreeItem::BankTreeItem(BankTreeItem* parentItem) : m_parentItem(parentItem) {}
BankTreeItem::~BankTreeItem()
{
	qDeleteAll(m_childItems);
}
QVariant BankTreeItem::data(int column) const
{
	switch (column) {
	case 0:
		return name;
		break;
	case 1:
		return ammo;
		break;
	default:
		return {};
	}
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

bool BankTreeItem::insertChild(int position, const QString& type, const QString& name, Banks* banks)
{
	if (position < 0 || position > m_childItems.size())
		return false;

	auto* item = new BankTreeItem(type, name, banks, this);
	m_childItems.insert(position, item);

	return true;
}

bool BankTreeItem::insertChild(int position, const QString& type, Bank* bank)
{
	if (position < 0 || position > m_childItems.size())
		return false;

	auto* item = new BankTreeItem(type, bank, this);
	m_childItems.insert(position, item);

	return true;
}

QString BankTreeItem::getType() const
{
	return type;
}

int BankTreeItem::getId() const
{
	return bank->getWeaponId();
}

bool BankTreeItem::setWeapon(int id)
{
	Assert(type != "LABEL");
	bank->setWeapon(id);
	if (id == -1) {
		name = "None";
	} else {
		name = Weapon_info[id].name;
	}
}

void BankTreeItem::setAIClass(int value)
{
	Assert(banks != nullptr);
	banks->aiClass = value;
}

QString BankTreeItem::getName() const
{
	return name;
}
bool BankTreeItem::setData(int column, const QVariant& value)
{
	switch (column) {
	case 1:
		setAmmo(value);
		return true;
		break;
	default:
		return false;
	}
}
BankTreeModel::BankTreeModel(const SCP_vector<Banks*>& data, QObject* parent) : QAbstractItemModel(parent)
{
	rootItem = new BankTreeItem();

	setupModelData(data, rootItem);
}

void BankTreeModel::setupModelData(const SCP_vector<Banks*>& data, BankTreeItem* parent)
{
	for (auto banks : data) {
		parent->insertChild(parent->childCount(), "LABEL", banks->name.c_str(), banks);
		BankTreeItem* currentParent = parent->child(parent->childCount() - 1);
		for (auto bank : banks->banks) {
			currentParent->insertChild(currentParent->childCount(), "BANK", bank);
		}
	}
}

BankTreeModel::~BankTreeModel()
{
	delete rootItem;
}

void BankTreeModel::SetAiClass(QModelIndexList& indexs, int value)
{
	for (auto& index : indexs) {
		if (index.isValid()) {
			auto item = this->getItem(index);
			if (item->getType() == "LABEL") {
				item->setAiClass(value);
			}
		}
	}
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
		if (item->getType() == "BANK" && index.column() == 0) {
			return Qt::ItemIsDropEnabled | Qt::ItemIsSelectable | defaultFlags;
		} else if (item->getType() == "BANK" && index.column() == 1) {
			return Qt::ItemIsEditable | defaultFlags;

		} else if (item->getType() == "LABEL") {
			return Qt::ItemIsSelectable | defaultFlags;
		} else {
			return defaultFlags;
		}
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

	if (column > 0)
		return false;

	return true;
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

bool BankTreeModel::setWeapon(const QModelIndex& index, int data)
{
	BankTreeItem* item = this->getItem(index);
	item->setWeapon(data);
}
} // namespace dialogs
} // namespace fred
} // namespace fso