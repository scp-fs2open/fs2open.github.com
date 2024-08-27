#pragma once
#include <mission/dialogs/ShipEditor/ShipCustomWarpDialogModel.h>

#include <QtWidgets/QDialog>

namespace fso {
namespace fred {
namespace dialogs {
namespace Ui {
class ShipCustomWarpDialog;
}

class ShipCustomWarpDialog : public QDialog {
	Q_OBJECT
  public:
	explicit ShipCustomWarpDialog(QDialog* parent, EditorViewport* viewport, const bool departure = false);
	~ShipCustomWarpDialog() override;

  protected:
	void closeEvent(QCloseEvent*) override;

  private:
	std::unique_ptr<Ui::ShipCustomWarpDialog> ui;
	std::unique_ptr<ShipCustomWarpDialogModel> _model;
	EditorViewport* _viewport;
	void updateUI(const bool firstRun = false);
	void rejectHandler();

	void startSoundChanged();
	void endSoundChanged();
	void animChanged();
};
} // namespace dialogs
} // namespace fred
} // namespace fso