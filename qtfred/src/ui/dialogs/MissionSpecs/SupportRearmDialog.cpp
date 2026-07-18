#include "SupportRearmDialog.h"

#include "ui_SupportRearmDialog.h"

#include "ui/util/SignalBlockers.h"
#include "weapon/weapon.h"

#include <QCloseEvent>

namespace fso::fred::dialogs {

SupportRearmDialog::SupportRearmDialog(QWidget* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::SupportRearmDialog()), _model(new SupportRearmDialogModel(this, viewport))
{
	ui->setupUi(this);
	populateTeamCombo();
}

SupportRearmDialog::~SupportRearmDialog() = default;

void SupportRearmDialog::setInitial(const SupportRearmSettings& settings)
{
	_model->setInitial(settings);
	_activePoolTeam = 0;
	syncUiFromModel();
}

SupportRearmSettings SupportRearmDialog::settings() const
{
	return _model->settings();
}

void SupportRearmDialog::closeEvent(QCloseEvent* e)
{
	reject();
	// reject() hides the dialog when it actually closes. Let that close
	// proceed (so a dialog created with WA_DeleteOnClose is destroyed),
	// and only veto it when reject() decided to keep the dialog open (e.g.
	// the user cancelled the unsaved-changes prompt).
	if (isVisible()) {
		e->ignore();
	} else {
		e->accept();
	}
}

QString SupportRearmDialog::weaponEntryText(int weaponClass) const
{
	const auto& wi = Weapon_info[weaponClass];
	if (wi.disallow_rearm) {
		return QString::fromStdString(SCP_string(wi.name) + " - 0 (disabled by weapon settings)");
	}
	const int amount = _model->settings().rearmWeaponPool[_activePoolTeam][weaponClass];
	if (amount < 0) {
		return QString::fromStdString(SCP_string(wi.name) + " - Unlimited");
	}
	return QString::fromStdString(SCP_string(wi.name) + " - " + std::to_string(amount));
}

void SupportRearmDialog::populateTeamCombo()
{
	util::SignalBlockers blockers(this);
	ui->poolTeamCombo->clear();
	for (int i = 0; i < Num_teams && i < MAX_TVT_TEAMS; ++i) {
		ui->poolTeamCombo->addItem(QString("Team %1").arg(i + 1), i);
	}
	if (ui->poolTeamCombo->count() > 0) {
		ui->poolTeamCombo->setCurrentIndex(_activePoolTeam);
	}
}

void SupportRearmDialog::syncUiFromModel()
{
	const auto& s = _model->settings();
	{
		util::SignalBlockers blockers(this);
		ui->supportEnabledCheck->setChecked(!s.disallowSupportShips);
		ui->repairHullCheck->setChecked(s.supportRepairsHull);
		ui->hullRepairSpin->setValue(s.maxHullRepair);
		ui->subsysRepairSpin->setValue(s.maxSubsysRepair);
		ui->disallowRearmCheck->setChecked(s.disallowSupportRearm);
		ui->limitPoolCheck->setChecked(s.limitRearmToPool);
		ui->fromLoadoutCheck->setChecked(s.rearmPoolFromLoadout);
		ui->precedenceCheck->setChecked(s.allowWeaponPrecedence);
		if (ui->poolTeamCombo->count() > 0) {
			ui->poolTeamCombo->setCurrentIndex(_activePoolTeam);
		}
	}
	populateWeaponList();
	if (ui->weaponList->count() > 0) {
		ui->weaponList->setCurrentRow(0);
	}
	updateFromSelection();
	updateControlStates();
}

void SupportRearmDialog::populateWeaponList()
{
	auto prev = selectedWeaponClass();
	ui->weaponList->clear();
	for (auto cls : _model->visibleWeaponClasses()) {
		auto* item = new QListWidgetItem(weaponEntryText(cls));
		item->setData(Qt::UserRole, cls);
		if (Weapon_info[cls].disallow_rearm) {
			item->setForeground(palette().color(QPalette::Disabled, QPalette::Text));
		}
		ui->weaponList->addItem(item);
	}
	for (int i = 0; i < ui->weaponList->count(); ++i) {
		if (ui->weaponList->item(i)->data(Qt::UserRole).toInt() == prev) {
			ui->weaponList->setCurrentRow(i);
			break;
		}
	}
}

int SupportRearmDialog::selectedWeaponClass() const
{
	auto* item = ui->weaponList->currentItem();
	if (!item)
		return -1;
	return item->data(Qt::UserRole).toInt();
}

void SupportRearmDialog::updateFromSelection()
{
	const int cls = selectedWeaponClass();
	if (cls < 0 || cls >= weapon_info_size()) {
		ui->poolAmountSpin->setValue(0);
	} else if (Weapon_info[cls].disallow_rearm) {
		ui->poolAmountSpin->setValue(0);
	} else {
		ui->poolAmountSpin->setValue(_model->settings().rearmWeaponPool[_activePoolTeam][cls]);
	}
}

void SupportRearmDialog::updateControlStates()
{
	const auto& s = _model->settings();
	const bool supportEnabled = !s.disallowSupportShips;
	const bool rearmAllowed = supportEnabled && !s.disallowSupportRearm;
	const bool poolEnabled = rearmAllowed && s.limitRearmToPool && !s.rearmPoolFromLoadout;
	const int cls = selectedWeaponClass();
	const bool selectedDisallowed = (cls >= 0 && cls < weapon_info_size() && Weapon_info[cls].disallow_rearm);
	const bool rightEnabled = poolEnabled && !selectedDisallowed;

	ui->repairHullCheck->setEnabled(supportEnabled);
	ui->hullRepairSpin->setEnabled(supportEnabled);
	ui->subsysRepairSpin->setEnabled(supportEnabled);
	ui->disallowRearmCheck->setEnabled(supportEnabled);

	const bool limitedPoolEnabled = rearmAllowed && s.limitRearmToPool;

	ui->limitPoolCheck->setEnabled(rearmAllowed);
	ui->fromLoadoutCheck->setEnabled(limitedPoolEnabled);
	ui->precedenceCheck->setEnabled(limitedPoolEnabled);

	ui->weaponList->setEnabled(poolEnabled);
	ui->poolTeamCombo->setEnabled(poolEnabled);
	ui->poolAmountSpin->setEnabled(rightEnabled);
	ui->setAmountButton->setEnabled(rightEnabled);
	ui->setUnlimitedButton->setEnabled(rightEnabled);
	ui->setZeroButton->setEnabled(rightEnabled);
	ui->setAllAmountButton->setEnabled(rightEnabled);
	ui->setAllUnlimitedButton->setEnabled(rightEnabled);
	ui->setAllZeroButton->setEnabled(rightEnabled);
}

void SupportRearmDialog::on_okAndCancelButtons_accepted()
{
	accept();
}
void SupportRearmDialog::on_okAndCancelButtons_rejected()
{
	reject();
}
void SupportRearmDialog::on_weaponList_currentRowChanged(int)
{
	updateFromSelection();
	updateControlStates();
}
void SupportRearmDialog::on_setAmountButton_clicked()
{
	_model->setWeaponPoolEntry(_activePoolTeam, selectedWeaponClass(), ui->poolAmountSpin->value());
	populateWeaponList();
	updateFromSelection();
	updateControlStates();
}
void SupportRearmDialog::on_setUnlimitedButton_clicked()
{
	_model->setWeaponPoolEntry(_activePoolTeam, selectedWeaponClass(), -1);
	populateWeaponList();
	updateFromSelection();
	updateControlStates();
}
void SupportRearmDialog::on_setZeroButton_clicked()
{
	_model->setWeaponPoolEntry(_activePoolTeam, selectedWeaponClass(), 0);
	populateWeaponList();
	updateFromSelection();
	updateControlStates();
}
void SupportRearmDialog::on_setAllAmountButton_clicked()
{
	_model->setAllVisibleWeaponPoolEntries(_activePoolTeam, ui->poolAmountSpin->value());
	populateWeaponList();
	updateFromSelection();
	updateControlStates();
}
void SupportRearmDialog::on_setAllUnlimitedButton_clicked()
{
	_model->setAllVisibleWeaponPoolEntries(_activePoolTeam, -1);
	populateWeaponList();
	updateFromSelection();
	updateControlStates();
}
void SupportRearmDialog::on_setAllZeroButton_clicked()
{
	_model->setAllVisibleWeaponPoolEntries(_activePoolTeam, 0);
	populateWeaponList();
	updateFromSelection();
	updateControlStates();
}
void SupportRearmDialog::on_supportEnabledCheck_toggled(bool checked)
{
	_model->setDisallowSupportShips(!checked);
	updateControlStates();
}
void SupportRearmDialog::on_repairHullCheck_toggled(bool checked)
{
	_model->setSupportRepairsHull(checked);
}
void SupportRearmDialog::on_disallowRearmCheck_toggled(bool checked)
{
	_model->setDisallowSupportRearm(checked);
	updateControlStates();
}
void SupportRearmDialog::on_limitPoolCheck_toggled(bool checked)
{
	_model->setLimitRearmToPool(checked);
	updateControlStates();
}
void SupportRearmDialog::on_fromLoadoutCheck_toggled(bool checked)
{
	_model->setRearmPoolFromLoadout(checked);
	updateControlStates();
}
void SupportRearmDialog::on_precedenceCheck_toggled(bool checked)
{
	_model->setAllowWeaponPrecedence(checked);
	updateControlStates();
}
void SupportRearmDialog::on_hullRepairSpin_valueChanged(double value)
{
	_model->setMaxHullRepair(static_cast<float>(value));
}
void SupportRearmDialog::on_subsysRepairSpin_valueChanged(double value)
{
	_model->setMaxSubsysRepair(static_cast<float>(value));
}

void SupportRearmDialog::on_poolTeamCombo_currentIndexChanged(int index)
{
	if (index < 0 || index >= ui->poolTeamCombo->count()) {
		return;
	}

	_activePoolTeam = ui->poolTeamCombo->itemData(index).toInt();
	populateWeaponList();
	if (ui->weaponList->count() > 0) {
		ui->weaponList->setCurrentRow(0);
	}
	updateFromSelection();
	updateControlStates();
}

} // namespace fso::fred::dialogs
