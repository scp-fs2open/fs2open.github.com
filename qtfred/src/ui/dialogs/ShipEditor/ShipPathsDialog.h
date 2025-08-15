#pragma once

#include "mission/dialogs/ShipEditor/ShipPathsDialogModel.h"
#include <QListWidgetItem>
#include <QtWidgets/QDialog>
namespace fso {
namespace fred {
namespace dialogs {
namespace Ui {
class ShipPathsDialog;
}
class ShipPathsDialog : public QDialog {
	Q_OBJECT
  public:
	explicit ShipPathsDialog(QWidget* parent,
		EditorViewport* viewport,
		const int ship,
		const int target_class,
		const bool departure);
	~ShipPathsDialog() override;

  protected:
	void closeEvent(QCloseEvent*) override;
	void rejectHandler();

  private:
	std::unique_ptr<Ui::ShipPathsDialog> ui;
	std::unique_ptr<ShipPathsDialogModel> _model;
	EditorViewport* _viewport;
	void updateUI();
	void changed(QListWidgetItem* changeditem);
};
} // namespace dialogs
} // namespace fred
} // namespace fso