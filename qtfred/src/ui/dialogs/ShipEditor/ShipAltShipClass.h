#pragma once
#include <mission/dialogs/ShipEditor/ShipAltShipClassModel.h>

#include <QtWidgets/QDialog>

namespace fso::fred::dialogs {
namespace Ui {
class ShipAltShipClass;
}
/**
 * @brief QtFRED's Alternate Ship Class Editor
 */
class ShipAltShipClass : public QDialog {
	Q_OBJECT
  public:
	/**
	 * @brief Constructor
	 * @param [in] parent The parent dialog.
	 * @param [in] viewport The viewport this dialog is attacted to.
	 */
	explicit ShipAltShipClass(QDialog* parent, EditorViewport* viewport, bool is_several_ships);
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

	/**
	 * @brief Populates the UI
	 */
	void updateUI();

  private slots: // NOLINT(readability-redundant-access-specifiers)
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();
};
} // namespace fso::fred::dialogs