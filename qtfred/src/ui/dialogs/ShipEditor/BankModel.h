#pragma once
#include "mission/dialogs/ShipEditor/ShipWeaponsDialogModel.h"

#include <QAbstractItemModel>
namespace fso::fred {
class BankTreeItem {
  public:
	explicit BankTreeItem(BankTreeItem* parentItem = nullptr, QString inName = "");
	virtual ~BankTreeItem();
	virtual QVariant data(int column) const = 0;
	void appendChild(BankTreeItem* child);
	BankTreeItem* child(int row) const;
	int childCount() const;
	int childNumber() const;
	BankTreeItem* parentItem();
	bool insertLabel(int position, const QString& name, Banks* banks);
	bool insertBank(int position, Bank* banks);

	QString getName() const;
	virtual bool setData(int column, const QVariant& value) = 0;
	virtual Qt::ItemFlags getFlags(int column) const = 0;
	QList<BankTreeItem*> m_childItems;

  protected:
	QString name;

  private:
	BankTreeItem* m_parentItem;
};
class BankTreeRoot : public BankTreeItem {
	bool setData(int column, const QVariant& value) override;
	QVariant data(int column) const override;
	Qt::ItemFlags getFlags(int column) const override;
};
class BankTreeBank : public BankTreeItem {
  public:
	explicit BankTreeBank(Bank* inBank, BankTreeItem* parentItem = nullptr);
	void setWeapon(int id);
	void setAmmo(int value);
	int getId() const;
	bool setData(int column, const QVariant& value) override;
	QVariant data(int column) const override;
	Qt::ItemFlags getFlags(int column) const override;

  private:
	Bank* bank;
};
class BankTreeLabel : public BankTreeItem {
  public:
	explicit BankTreeLabel(const QString& name, Banks* banks, BankTreeItem* parentItem = nullptr);
	void setAIClass(int value);
	bool setData(int column, const QVariant& value) override;
	QVariant data(int column) const override;
	Qt::ItemFlags getFlags(int column) const override;

  private:
	Banks* banks;
};

class BankTreeModel : public QAbstractItemModel {
	Q_OBJECT
  public:
	BankTreeModel(const SCP_vector<Banks*>& data, QObject* parent = nullptr);
	~BankTreeModel() override;
	int columnCount(const QModelIndex& parent) const override;
	QVariant data(const QModelIndex& index, int role) const override;

	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex& index) const override;

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;

	Qt::ItemFlags flags(const QModelIndex& index) const override;

	QStringList mimeTypes() const override;
	bool canDropMimeData(const QMimeData* data,
		Qt::DropAction action,
		int row,
		int column,
		const QModelIndex& parent) const override;
	bool
	dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
	void setWeapon(const QModelIndex& index, int data);
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
	int checktype(const QModelIndex index) const;

  private:
	BankTreeItem* rootItem;
	BankTreeItem* getItem(const QModelIndex index) const;
	static void setupModelData(const SCP_vector<Banks*>& data, BankTreeItem* parent);
};
} // namespace fso::fred