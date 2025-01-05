#pragma once
#include <mission/dialogs/ShipEditor/ShipCustomWarpDialogModel.h>

#include <QtWidgets/QDialog>

namespace fso::fred::dialogs {
namespace Ui {
class ShipCustomWarpDialog;
}
/**
 * @brief QtFRED's Custom Warp Editor
 */
class ShipCustomWarpDialog : public QDialog {
	Q_OBJECT
  public:
	/**
	 * @brief Constructor
	 * @param [in] parent The parent dialog.
	 * @param [in] viewport The viewport this dialog is attacted to.
	 * @param [in] departure Whether the dialog is changeing warp-in or warp-out.
	 */
	explicit ShipCustomWarpDialog(QDialog* parent, EditorViewport* viewport, const bool departure = false);
	~ShipCustomWarpDialog() override;

  protected:
	/**
	 * @brief Overides the Dialogs Close event to add a confermation dialog
	 * @param [in] *e The event.
	 */
	void closeEvent(QCloseEvent*) override;

  private:
	std::unique_ptr<Ui::ShipCustomWarpDialog> ui;
	std::unique_ptr<ShipCustomWarpDialogModel> _model;
	EditorViewport* _viewport;
	/**
	 * @brief Populates the UI
	 * @param [in] firstRun If this is the first run.
	 */
	void updateUI(const bool firstRun = false);
	/**
	 * @brief Check if the user meant to cancel
	 */
	void rejectHandler();

	/**
	 * @brief Update model with the contents of the start sound text box
	 */
	void startSoundChanged();
	/**
	 * @brief Update model with the contents of the end sound text box
	 */
	void endSoundChanged();
	/**
	 * @brief Update model with the contents of the anim text box
	 */
	void animChanged();
};
} // namespace fso::fred::dialogs