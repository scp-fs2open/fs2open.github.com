#ifndef SHIPGOALSDIALOG_H
#define SHIPGOALSDIALOG_H

#include <mission/dialogs/ShipGoalsDialogModel.h>

#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QSpinBox>

#include "ShipEditorDialog.h"

namespace fso {
namespace fred {
namespace dialogs {
namespace Ui {
class ShipGoalsDialog;
}
class ShipEditorDialog;
class ShipGoalsDialog : public QDialog {
	Q_OBJECT
  public:
	explicit ShipGoalsDialog(QWidget* parent, EditorViewport* viewport);
	~ShipGoalsDialog() override;

	void showEvent(QShowEvent* e);

  protected:
	void closeEvent(QCloseEvent*) override;

  private:
	std::unique_ptr<Ui::ShipGoalsDialog> ui;
	std::unique_ptr<ShipGoalsDialogModel> _model;
	EditorViewport* _viewport;

	QComboBox* behaviors[ED_MAX_GOALS];
	QComboBox* objects[ED_MAX_GOALS];
	QComboBox* subsys[ED_MAX_GOALS];
	QComboBox* docks[ED_MAX_GOALS];
	QSpinBox* priority[ED_MAX_GOALS];

	void updateUI();
	bool WingMode;
	ShipEditorDialog* parentDialog = nullptr;
};
} // namespace dialogs
} // namespace fred
} // namespace fso
#endif // !SHIPGOALSDIALOG_H