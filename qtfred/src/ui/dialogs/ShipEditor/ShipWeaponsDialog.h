#ifndef SHIPWEAPONSDIALOG_H
#define SHIPWEAPONSDIALOG_H

#include <mission/dialogs/ShipEditor/ShipWeaponsDialogModel.h>
#include <ui/FredView.h>

#include <QtWidgets/QDialog>
#include <QtCore/QItemSelection>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class ShipWeaponsDialog;
}
namespace WeaponsDialog {
class BankTreeItem {
  public:
	explicit BankTreeItem(BankTreeItem* parentItem = nullptr);
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
	virtual Qt::ItemFlags getFlags(int column, int type) = 0;
	QList<BankTreeItem*> m_childItems;

  protected:
	QString name;

  private:
	BankTreeItem* m_parentItem;
};
class BankTreeRoot : public BankTreeItem {
	bool setData(int column, const QVariant& value) override;
	QVariant data(int column) const override;
	Qt::ItemFlags getFlags(int column, int type) override;
};
class BankTreeBank : public BankTreeItem {
  public:
	explicit BankTreeBank(Bank* bank, BankTreeItem* parentItem = nullptr);
	void setWeapon(int id);
	void setAmmo(int value);
	int getId() const;
	bool setData(int column, const QVariant& value) override;
	QVariant data(int column) const override;
	Qt::ItemFlags getFlags(int column, int type) override;

  private:
	Bank* bank;
};
class BankTreeLabel : public BankTreeItem {
  public:
	explicit BankTreeLabel(const QString& name, Banks* banks, BankTreeItem* parentItem = nullptr);
	void setAIClass(int value);
	bool setData(int column, const QVariant& value) override;
	QVariant data(int column) const override;
	Qt::ItemFlags getFlags(int column, int type) override;

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
	bool
	canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const;
	bool
	dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
	void setWeapon(const QModelIndex& index, int data) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	BankTreeItem* getItem(const QModelIndex index) const;
	int typeSelected = -1;
	BankTreeItem* rootItem;

  private:
	static void setupModelData(const SCP_vector<Banks*>& data, BankTreeItem* parent);
};

struct WeaponItem {
	WeaponItem(const int id, const SCP_string& name);
	const SCP_string name;
	const int id;
};
class WeaponModel : public QAbstractListModel {
	Q_OBJECT
  public:
	WeaponModel(int type);
	~WeaponModel();
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QMimeData* mimeData(const QModelIndexList& indexes) const;
	SCP_vector<WeaponItem*> weapons;
};


class ShipWeaponsDialog : public QDialog {
	Q_OBJECT

  public:
	explicit ShipWeaponsDialog(QDialog* parent, EditorViewport* viewport, bool isMultiEdit);
	~ShipWeaponsDialog() override;

  protected:
	void closeEvent(QCloseEvent*) override;

  private:
	std::unique_ptr<Ui::ShipWeaponsDialog> ui;
	std::unique_ptr<ShipWeaponsDialogModel> _model;
	void modeChanged(const bool enabled, const int mode);
	EditorViewport* _viewport;
	void updateUI();
	BankTreeModel* bankModel;
	int dialogMode;
	void processMultiSelect(const QItemSelection& selected, const QItemSelection& deselected);
	QAbstractListModel* weapons;
};
} // namespace WeaponsDialog
} // namespace dialogs
} // namespace fred
} // namespace fso
#endif