#ifndef SHIPWEAPONSDIALOG_H
#define SHIPWEAPONSDIALOG_H

#include <mission/dialogs/ShipWeaponsDialogModel.h>
#include <ui/FredView.h>

#include <QtWidgets/QDialog>

namespace fso {
namespace fred {
namespace dialogs {

namespace ui {
class ShipWeaponsDialog;
}

class BankTreeItem {
  public:
	explicit BankTreeItem(const QString& type, const QString& name, Banks* banks, BankTreeItem* parentItem = nullptr);
	explicit BankTreeItem(const QString& type, Bank* bank, BankTreeItem* parentItem = nullptr);
	explicit BankTreeItem(BankTreeItem* parentItem = nullptr);
	~BankTreeItem();
	QVariant data(int column) const;
	void appendChild(BankTreeItem* child);
	BankTreeItem* child(int row);
	int childCount() const;
	int childNumber() const;
	BankTreeItem* parentItem();
	bool insertChild(int position, const QString& type, const QString& name, Banks* banks);
	bool insertChild(int position, const QString& type, Bank* bank);

	//setters
	bool setWeapon(int id);
	void setAIClass(int value);
	//getters
	QString getType() const;
	int getId() const;
	QString getName() const;

	bool setData(int column, const QVariant& value);

  private:
	QList<BankTreeItem*> m_childItems;
	BankTreeItem* m_parentItem;
	Bank* bank;
	Banks* banks;
	QString name;
	QString type;
	int ammo;
};

class BankTreeModel : public QAbstractItemModel {
	Q_OBJECT
  public:
	BankTreeModel(const SCP_vector<Banks*>& data, QObject* parent = nullptr);
	~BankTreeModel() override;
	void SetAIClass(QModelIndexList& indexs, int value);
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
	bool setWeapon(const QModelIndex& index, int data);

  private:
	void setupModelData(const SCP_vector<Banks*>& data, BankTreeItem* parent);
	BankTreeItem* getItem(const QModelIndex& index) const;
	BankTreeItem* rootItem;
};

class ShipWeaponsDialog : public QDialog {
	Q_OBJECT

  public:
	explicit ShipWeaponsDialog(QDialog* parent, EditorViewport* viewport);
	~ShipWeaponsDialog() override;

  protected:
	void closeEvent(QCloseEvent*) override;
	void showEvent(QShowEvent*) override;

  private:
	std::unique_ptr<Ui::ShipWeaponsDialog> ui;
	std::unique_ptr<ShipWeaponsDialogModel> _model;
	ShipEditorDialog* parentDialog;
	void modeChanged(bool enabled, int mode);
	EditorViewport* _viewport;
	void updateUI();
	BankTreeModel* bankModel;
};

} // namespace dialogs
} // namespace fred
} // namespace fso
#endif