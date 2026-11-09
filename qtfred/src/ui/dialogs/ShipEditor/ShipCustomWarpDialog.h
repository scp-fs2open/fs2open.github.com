#pragma once
#include <mission/dialogs/ShipEditor/ShipCustomWarpDialogModel.h>

#include <QtWidgets/QDialog>

namespace fso::fred::dialogs {
namespace Ui {
class ShipCustomWarpDialog;
}
class ShipCustomWarpDialog : public QDialog {
	Q_OBJECT
  public:
	explicit ShipCustomWarpDialog(QDialog* parent, EditorViewport* viewport, bool departure = false);
	ShipCustomWarpDialog(QDialog* parent, EditorViewport* viewport, bool departure, int wingIndex, bool wingMode);
	~ShipCustomWarpDialog() override;

	void accept() override;
	void reject() override;

  protected:
	void closeEvent(QCloseEvent*) override;
  private slots:
	void on_buttonBox_accepted();
	void on_buttonBox_rejected();
	void on_comboBoxType_currentIndexChanged(int);
	void on_lineEditStartSound_editingFinished();
	void on_lineEditEndSound_editingFinished();
	void on_doubleSpinBoxEngage_valueChanged(double);
	void on_doubleSpinBoxSpeed_valueChanged(double);
	void on_doubleSpinBoxTime_valueChanged(double);
	void on_doubleSpinBoxExponent_valueChanged(double);
	void on_doubleSpinBoxRadius_valueChanged(double);
	void on_lineEditAnim_editingFinished();
	void on_checkBoxSupercap_toggled(bool);
	void on_doubleSpinBoxPlayerSpeed_valueChanged(double);
  private: // NOLINT(readability-redundant-access-specifiers)
	std::unique_ptr<Ui::ShipCustomWarpDialog> ui;
	std::unique_ptr<ShipCustomWarpDialogModel> _model;
	EditorViewport* _viewport;
	void updateUi(bool firstRun = false);
};
} // namespace fso::fred::dialogs
