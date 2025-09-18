#pragma once

#include <mission/dialogs/ShipEditor/ShipSpecialStatsDialogModel.h>

#include <QtWidgets/QDialog>

namespace fso::fred::dialogs {

namespace Ui {
class ShipSpecialStatsDialog;
}

class ShipSpecialStatsDialog : public QDialog {
	Q_OBJECT

  public:
	explicit ShipSpecialStatsDialog(QWidget* parent, EditorViewport* viewport);
	~ShipSpecialStatsDialog() override;

	void accept() override;
	void reject() override;

  protected:
	void closeEvent(QCloseEvent*) override;

  private slots:

	void on_buttonBox_accepted();
	void on_buttonBox_rejected();
	void on_explodeCheckBox_toggled(bool);
	void on_damageSpinBox_valueChanged(int);
	void on_blastSpinBox_valueChanged(int);
	void on_iRadiusSpinBox_valueChanged(int);
	void on_oRadiusSpinBox_valueChanged(int);
	void on_createShockwaveCheckBox_toggled(bool);
	void on_shockwaveSpeedSpinBox_valueChanged(int);
	void on_deathRollCheckBox_toggled(bool);
	void on_rollTimeSpinBox_valueChanged(int);

	void on_specialShieldCheckBox_toggled(bool);
	void on_shieldSpinBox_valueChanged(int);
	void on_specialHullCheckbox_toggled(bool);
	void on_hullSpinBox_valueChanged(int);

  private:// NOLINT(readability-redundant-access-specifiers)
	std::unique_ptr<Ui::ShipSpecialStatsDialog> ui;
	std::unique_ptr<ShipSpecialStatsDialogModel> _model;
	EditorViewport* _viewport;

	void updateUI(bool first = false);
};
} // namespace fso::fred::dialogs