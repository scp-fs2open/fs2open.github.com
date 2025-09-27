#ifndef SHIPFLAGDIALOG_H
#define SHIPFLAGDIALOG_H

#include <mission/dialogs/ShipEditor/ShipFlagsDialogModel.h>
#include <ui/FredView.h>

#include <QtWidgets/QDialog>

namespace fso::fred::dialogs {

namespace Ui {
class ShipFlagsDialog;
}
class ShipFlagsDialog : public QDialog {
	Q_OBJECT

  public:
	explicit ShipFlagsDialog(QWidget* parent, EditorViewport* viewport);
	~ShipFlagsDialog() override;
	void accept() override;
	void reject() override;

  protected:
	void closeEvent(QCloseEvent*) override;
  private slots:
	void on_okAndCancelButtons_accepted();
	void on_okAndCancelButtons_rejected();
	void on_destroySecondsSpinBox_valueChanged(int);
	void on_escortPrioritySpinBox_valueChanged(int);
	void on_kamikazeDamageSpinBox_valueChanged(int);

  private: // NOLINT(readability-redundant-access-specifiers)
	std::unique_ptr<Ui::ShipFlagsDialog> ui;
	std::unique_ptr<ShipFlagsDialogModel> _model;
	EditorViewport* _viewport;
	void updateUI();
};
} // namespace fso::fred::dialogs

#endif // !SHIPFLAGDIALOG_H