#include "SupportRearmDialog.h"

#include "ui_SupportRearmDialog.h"

#include "ui/util/SignalBlockers.h"
#include "weapon/weapon.h"

#include <QCloseEvent>

namespace fso::fred::dialogs {

SupportRearmDialog::SupportRearmDialog(QWidget* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::SupportRearmDialog()), _model(new SupportRearmDialogModel(this, viewport)),
	  _viewport(viewport)
{
	ui->setupUi(this);
	ui->poolTeamCombo->clear();
	for (int i = 0; i < Num_teams && i < MAX_TVT_TEAMS; ++i) {
		ui->poolTeamCombo->addItem(QString("Team %1").arg(i + 1), i);
	}
	_activePoolTeam = 0;
	if (ui->poolTeamCombo->count() > 0) {
		ui->poolTeamCombo->setCurrentIndex(_activePoolTeam);
	}
	populateWeaponList();
	if (ui->weaponList->count() > 0) {
		ui->weaponList->setCurrentRow(0);
	}
	updateFromSelection();
	updateControlStates();
}

SupportRearmDialog::~SupportRearmDialog() = default;

void SupportRearmDialog::setInitial(const SupportRearmSettings& settings)
{
	_model->setInitial(settings);
	{
		util::SignalBlockers blockers(this);
		ui->supportEnabledCheck->setChecked(!settings.disallowSupportShips);
		ui->repairHullCheck->setChecked(settings.supportRepairsHull);
		ui->hullRepairSpin->setValue(settings.maxHullRepair);
		ui->subsysRepairSpin->setValue(settings.maxSubsysRepair);
		ui->disallowRearmCheck->setChecked(settings.disallowSupportRearm);
		ui->limitPoolCheck->setChecked(settings.limitRearmToPool);
		ui->fromLoadoutCheck->setChecked(settings.rearmPoolFromLoadout);
		ui->precedenceCheck->setChecked(settings.allowWeaponPrecedence);
		_activePoolTeam = 0;
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

SupportRearmSettings SupportRearmDialog::settings() const
{
	auto out = _model->settings();
	out.disallowSupportShips = !ui->supportEnabledCheck->isChecked();
	out.supportRepairsHull = ui->repairHullCheck->isChecked();
	out.maxHullRepair = static_cast<float>(ui->hullRepairSpin->value());
	out.maxSubsysRepair = static_cast<float>(ui->subsysRepairSpin->value());
	out.disallowSupportRearm = ui->disallowRearmCheck->isChecked();
	out.limitRearmToPool = ui->limitPoolCheck->isChecked();
	out.rearmPoolFromLoadout = ui->fromLoadoutCheck->isChecked();
	out.allowWeaponPrecedence = ui->precedenceCheck->isChecked();
	for (int team = 0; team < MAX_TVT_TEAMS; ++team) {
		for (int i = 0; i < weapon_info_size(); ++i) {
			if (Weapon_info[i].disallow_rearm) {
				out.rearmWeaponPool[team][i] = 0;
			}
		}
	}
	return out;
}

void SupportRearmDialog::accept()
{
	if (_model->apply()) {
		QDialog::accept();
	}
}

void SupportRearmDialog::reject()
{
	QDialog::reject();
}

void SupportRearmDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore();
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
	const bool supportEnabled = ui->supportEnabledCheck->isChecked();
	const bool rearmAllowed = supportEnabled && !ui->disallowRearmCheck->isChecked();
	const bool poolEnabled = rearmAllowed && ui->limitPoolCheck->isChecked() && !ui->fromLoadoutCheck->isChecked();
	const int cls = selectedWeaponClass();
	const bool selectedDisallowed = (cls >= 0 && cls < weapon_info_size() && Weapon_info[cls].disallow_rearm);
	const bool rightEnabled = poolEnabled && !selectedDisallowed;

	ui->repairHullCheck->setEnabled(supportEnabled);
	ui->hullRepairSpin->setEnabled(supportEnabled);
	ui->subsysRepairSpin->setEnabled(supportEnabled);
	ui->disallowRearmCheck->setEnabled(supportEnabled);

	const bool limitedPoolEnabled = rearmAllowed && ui->limitPoolCheck->isChecked();

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

void SupportRearmDialog::setCurrentWeaponAmount(int amount)
{
	const int cls = selectedWeaponClass();
	if (cls < 0 || cls >= weapon_info_size())
		return;
	if (Weapon_info[cls].disallow_rearm) {
		_model->settings().rearmWeaponPool[_activePoolTeam][cls] = 0;
	} else {
		_model->settings().rearmWeaponPool[_activePoolTeam][cls] = (amount < 0) ? -1 : amount;
	}
	populateWeaponList();
	updateFromSelection();
	updateControlStates();
}

void SupportRearmDialog::setAllVisibleWeaponAmounts(int amount)
{
	for (auto cls : _model->visibleWeaponClasses()) {
		if (Weapon_info[cls].disallow_rearm) {
			_model->settings().rearmWeaponPool[_activePoolTeam][cls] = 0;
		} else {
			_model->settings().rearmWeaponPool[_activePoolTeam][cls] = (amount < 0) ? -1 : amount;
		}
	}
	populateWeaponList();
	updateFromSelection();
	updateControlStates();
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
	setCurrentWeaponAmount(ui->poolAmountSpin->value());
}
void SupportRearmDialog::on_setUnlimitedButton_clicked()
{
	setCurrentWeaponAmount(-1);
}
void SupportRearmDialog::on_setZeroButton_clicked()
{
	setCurrentWeaponAmount(0);
}
void SupportRearmDialog::on_setAllAmountButton_clicked()
{
	setAllVisibleWeaponAmounts(ui->poolAmountSpin->value());
}
void SupportRearmDialog::on_setAllUnlimitedButton_clicked()
{
	setAllVisibleWeaponAmounts(-1);
}
void SupportRearmDialog::on_setAllZeroButton_clicked()
{
	setAllVisibleWeaponAmounts(0);
}
void SupportRearmDialog::on_supportEnabledCheck_toggled(bool)
{
	updateControlStates();
}
void SupportRearmDialog::on_repairHullCheck_toggled(bool) {}
void SupportRearmDialog::on_disallowRearmCheck_toggled(bool)
{
	updateControlStates();
}
void SupportRearmDialog::on_limitPoolCheck_toggled(bool)
{
	updateControlStates();
}
void SupportRearmDialog::on_fromLoadoutCheck_toggled(bool)
{
	updateControlStates();
}
void SupportRearmDialog::on_precedenceCheck_toggled(bool)
{
	updateControlStates();
}
void SupportRearmDialog::on_hullRepairSpin_valueChanged(double) {}
void SupportRearmDialog::on_subsysRepairSpin_valueChanged(double) {}

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
