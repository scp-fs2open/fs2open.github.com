#ifndef SHIPGOALSDIALOG_H
#define SHIPGOALSDIALOG_H

#include <mission/dialogs/ShipEditor/ShipGoalsDialogModel.h>

#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QSpinBox>

namespace fso::fred::dialogs {
namespace Ui {
class ShipGoalsDialog;
}
class ShipGoalsDialog : public QDialog {
	Q_OBJECT
  public:
	explicit ShipGoalsDialog(QWidget* parent, EditorViewport* viewport, bool editMultiple, int shipID, int wingID);
	~ShipGoalsDialog() override;
	void accept() override;
	void reject() override;

  protected:
	void closeEvent(QCloseEvent*) override;

  private slots:
	void on_okButton_clicked();
	void on_cancelButton_clicked();

  private:// NOLINT(readability-redundant-access-specifiers)
	std::unique_ptr<Ui::ShipGoalsDialog> ui;
	std::unique_ptr<ShipGoalsDialogModel> _model;
	EditorViewport* _viewport;

	QComboBox* behaviors[ED_MAX_GOALS];
	QComboBox* objects[ED_MAX_GOALS];
	QComboBox* subsys[ED_MAX_GOALS];
	QComboBox* docks[ED_MAX_GOALS];
	QSpinBox* priority[ED_MAX_GOALS];

	void updateUI();
};
} // namespace fso::fred::dialogs
#endif // !SHIPGOALSDIALOG_H