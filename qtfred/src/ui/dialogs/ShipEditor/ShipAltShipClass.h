#pragma once

#include <mission/dialogs/ShipEditor/ShipAltShipClassModel.h>

#include <QtWidgets/QDialog>
#include <qstandarditemmodel.h>
#include <QSortFilterProxyModel>

namespace fso::fred::dialogs {

namespace Ui {
class ShipAltShipClass;
}

class InverseSortFilterProxyModel : public QSortFilterProxyModel {
	Q_OBJECT
  public:
	InverseSortFilterProxyModel(QObject* parent = nullptr);

  protected:
	bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
};

class ShipAltShipClass : public QDialog {
	Q_OBJECT
  public:
	explicit ShipAltShipClass(QDialog* parent, EditorViewport* viewport);
	~ShipAltShipClass() override;

	void accept() override;
	void reject() override;

  protected:
	void closeEvent(QCloseEvent*) override;

  private:
	std::unique_ptr<Ui::ShipAltShipClass> ui;
	std::unique_ptr<ShipAltShipClassModel> _model;
	EditorViewport* _viewport;

	void initUI();
	void updateUi();

	QStandardItemModel* _altPool;

	void classListChanged(const QModelIndex& current);
	static QStandardItem* generateItem(int classid, int variable, bool defaultShip);
	static QString generateName(int classid, int variable);

	void syncData();

  private slots: // NOLINT(readability-redundant-access-specifiers)
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();
	void on_addButton_clicked();
	void on_insertButton_clicked();
	void on_deleteButton_clicked();
	void on_upButton_clicked();
	void on_downButton_clicked();
	void on_shipCombo_currentIndexChanged(int);
	void on_variableCombo_currentIndexChanged(int);
	void on_defaultCheckbox_toggled(bool);
};

} // namespace fso::fred::dialogs
