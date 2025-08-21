#pragma once
#include <mission/dialogs/ShipEditor/ShipAltShipClassModel.h>

#include <QtWidgets/QDialog>
#include <qstandarditemmodel.h>
#include<QSortFilterProxyModel>
namespace fso::fred::dialogs {
namespace Ui {
class ShipAltShipClass;
}
/**
 * @brief QtFRED's Alternate Ship Class Editor
 */

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
	/**
	 * @brief Constructor
	 * @param [in] parent The parent dialog.
	 * @param [in] viewport The viewport this dialog is attacted to.
	 */
	explicit ShipAltShipClass(QDialog* parent, EditorViewport* viewport);
	~ShipAltShipClass() override;

	void accept() override;
	void reject() override;

  protected:
	/**
	 * @brief Overides the Dialogs Close event to add a confermation dialog
	 * @param [in] *e The event.
	 */
	void closeEvent(QCloseEvent*) override;

  private:
	std::unique_ptr<Ui::ShipAltShipClass> ui;
	std::unique_ptr<ShipAltShipClassModel> _model;
	EditorViewport* _viewport;

	void initUI();

	/**
	 * @brief Populates the UI
	 */
	void updateUI();

	QStandardItemModel* alt_pool;

	void classListChanged(const QModelIndex& current);
	static QStandardItem* generate_item(const int classid, const int variable, const bool default_ship);
	static QString generate_name(const int classid, const int variable);

	void sync_data();

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