#pragma once

#include "mission/dialogs/MissionSpecs/SupportRearmDialogModel.h"

#include <QDialog>

namespace fso::fred::dialogs {

namespace Ui {
class SupportRearmDialog;
}

class SupportRearmDialog : public QDialog {
	Q_OBJECT
  public:
	explicit SupportRearmDialog(QWidget* parent, EditorViewport* viewport);
	~SupportRearmDialog() override;

	void setInitial(const SupportRearmSettings& settings);
	SupportRearmSettings settings() const;

	void accept() override;
	void reject() override;

  protected:
	void closeEvent(QCloseEvent* e) override;

  private slots:
	void on_okAndCancelButtons_accepted();
	void on_okAndCancelButtons_rejected();
	void on_weaponList_currentRowChanged(int currentRow);
	void on_setAmountButton_clicked();
	void on_setUnlimitedButton_clicked();
	void on_setZeroButton_clicked();
	void on_setAllAmountButton_clicked();
	void on_setAllUnlimitedButton_clicked();
	void on_setAllZeroButton_clicked();
	void on_supportEnabledCheck_toggled(bool);
	void on_repairHullCheck_toggled(bool);
	void on_disallowRearmCheck_toggled(bool);
	void on_limitPoolCheck_toggled(bool);
	void on_fromLoadoutCheck_toggled(bool);
	void on_precedenceCheck_toggled(bool);
	void on_hullRepairSpin_valueChanged(double);
	void on_subsysRepairSpin_valueChanged(double);
	void on_poolTeamCombo_currentIndexChanged(int);

  private: // NOLINT(readability-redundant-access-specifiers)
	void populateWeaponList();
	void updateFromSelection();
	void updateControlStates();
	int selectedWeaponClass() const;
	void setCurrentWeaponAmount(int amount);
	void setAllVisibleWeaponAmounts(int amount);
	QString weaponEntryText(int weaponClass) const;

	std::unique_ptr<Ui::SupportRearmDialog> ui;
	std::unique_ptr<SupportRearmDialogModel> _model;
	EditorViewport* _viewport;

	int _activePoolTeam = 0;
};

} // namespace fso::fred::dialogs
