#ifndef SHIPWEAPONSDIALOG_H
#define SHIPWEAPONSDIALOG_H

#include "ShipEditorDialog.h"

#include <mission/dialogs/ShipWeaponsDialogModel.h>
#include <ui/FredView.h>

#include <QtWidgets/QDialog>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class ShipWeaponsDialog;
}
class BankTreeItem {
  public:
	explicit BankTreeItem(BankTreeItem* parentItem = nullptr);
	virtual ~BankTreeItem();
	virtual QVariant data(int column) const = 0;
	void appendChild(BankTreeItem* child);
	BankTreeItem* child(int row);
	int childCount() const;
	int childNumber() const;
	BankTreeItem* parentItem();
	bool insertLabel(int position, const QString& name, Banks* banks);
	bool insertBank(int position, Bank* banks);

	QString getName() const;
	virtual bool setData(int column, const QVariant& value) = 0;
	virtual Qt::ItemFlags getFlags(int column) = 0;

  protected:
	QString name;

  private:
	BankTreeItem* m_parentItem;
	QList<BankTreeItem*> m_childItems;
};
class BankTreeRoot : public BankTreeItem {
	bool setData(int column, const QVariant& value) override;
	QVariant data(int column) const override;
	Qt::ItemFlags getFlags(int column) override;
};
class BankTreeBank : public BankTreeItem {
  public:
	explicit BankTreeBank(Bank* bank, BankTreeItem* parentItem = nullptr);
	void setWeapon(int id);
	void setAmmo(int value);
	int getId() const;
	bool setData(int column, const QVariant& value) override;
	QVariant data(int column) const override;
	Qt::ItemFlags getFlags(int column) override;

  private:
	Bank* bank;
};
class BankTreeLabel : public BankTreeItem {
  public:
	explicit BankTreeLabel(const QString& name, Banks* banks, BankTreeItem* parentItem = nullptr);
	void setAIClass(int value);
	bool setData(int column, const QVariant& value) override;
	QVariant data(int column) const override;
	Qt::ItemFlags getFlags(int column) override;

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
	bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
	bool
	dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
	void setWeapon(const QModelIndex& index, int data);
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  private:
	void setupModelData(const SCP_vector<Banks*>& data, BankTreeItem* parent);
	BankTreeItem* getItem(const QModelIndex& index) const;
	BankTreeItem* rootItem;
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
	ShipEditorDialog* parentDialog;
	void modeChanged(bool enabled, int mode);
	EditorViewport* _viewport;
	void updateUI();
	BankTreeModel* bankModel;
	int dialogMode;
};

} // namespace dialogs
} // namespace fred
} // namespace fso
#endif