#include "ShipCustomWarpDialog.h"

#include "ui_ShipCustomWarpDialog.h"

#include "mission/util.h"

#include <ship/shipfx.h>
#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>

namespace fso::fred::dialogs {
ShipCustomWarpDialog::ShipCustomWarpDialog(QDialog* parent, EditorViewport* viewport, bool departure)
	: QDialog(parent), ui(new Ui::ShipCustomWarpDialog()),
	  _model(new ShipCustomWarpDialogModel(this, viewport, departure)), _viewport(viewport)
{
	ui->setupUi(this);
	connect(_model.get(), &AbstractDialogModel::modelChanged, this, [this]() { updateUI(false); });

	if (departure) {
		this->setWindowTitle("Edit Warp-Out Parameters");
	} else {
		this->setWindowTitle("Edit Warp-In Parameters");
	}
	updateUI(true);
	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

// Wing mode constructor
ShipCustomWarpDialog::ShipCustomWarpDialog(QDialog* parent,
	EditorViewport* viewport,
	bool departure,
	int wingIndex,
	bool wingMode)
	: QDialog(parent), ui(new Ui::ShipCustomWarpDialog()), _viewport(viewport)
{
	ui->setupUi(this);

	if (wingMode) {
		_model.reset(new ShipCustomWarpDialogModel(this,
			viewport,
			departure,
			ShipCustomWarpDialogModel::Target::Wing,
			wingIndex));
	} else {
		_model.reset(new ShipCustomWarpDialogModel(this, viewport, departure));
	}

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, [this]() { updateUI(false); });

	if (departure) {
		this->setWindowTitle("Edit Warp-Out Parameters");
	} else {
		this->setWindowTitle("Edit Warp-In Parameters");
	}
	updateUI(true);
	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}
void ShipCustomWarpDialog::on_buttonBox_accepted()
{
	accept();
}
void ShipCustomWarpDialog::on_buttonBox_rejected()
{
	reject();
}
void ShipCustomWarpDialog::on_comboBoxType_currentIndexChanged(int index)
{
	_model->setType(index);
}
void ShipCustomWarpDialog::on_lineEditStartSound_editingFinished()
{ // String wrangling reqired in order to avoid crashes when directly converting from Qstring to std::string on some
	// enviroments
	QString temp(ui->lineEditStartSound->text());
	if (!temp.isEmpty()) {
		_model->setStartSound(temp.toLatin1().constData());
	} else {
		_model->setStartSound("");
	}
}
void ShipCustomWarpDialog::on_lineEditEndSound_editingFinished()
{ // String wrangling reqired in order to avoid crashes when directly converting from Qstring to std::string on some
	// enviroments
	QString temp(ui->lineEditEndSound->text());
	if (!temp.isEmpty()) {
		_model->setEndSound(temp.toLatin1().constData());
	} else {
		_model->setEndSound("");
	}
}
void ShipCustomWarpDialog::on_doubleSpinBoxEngage_valueChanged(double value)
{
	_model->setEngageTime(value);
}
void ShipCustomWarpDialog::on_doubleSpinBoxSpeed_valueChanged(double value)
{
	_model->setSpeed(value);
}
void ShipCustomWarpDialog::on_doubleSpinBoxTime_valueChanged(double value)
{
	_model->setTime(value);
}
void ShipCustomWarpDialog::on_doubleSpinBoxExponent_valueChanged(double value)
{
	_model->setExponent(value);
}
void ShipCustomWarpDialog::on_doubleSpinBoxRadius_valueChanged(double value)
{
	_model->setRadius(value);
}
void ShipCustomWarpDialog::on_lineEditAnim_editingFinished()
{ // String wrangling reqired in order to avoid crashes when directly converting from Qstring to std::string on some
	// enviroments
	QString temp(ui->lineEditAnim->text());
	if (!temp.isEmpty()) {
		_model->setAnim(temp.toLatin1().constData());
	} else {
		_model->setAnim("");
	}
}
void ShipCustomWarpDialog::on_checkBoxSupercap_toggled(bool state)
{
	_model->setSupercap(state);
}
void ShipCustomWarpDialog::on_doubleSpinBoxPlayerSpeed_valueChanged(double value)
{
	_model->setPlayerSpeed(value);
}

ShipCustomWarpDialog::~ShipCustomWarpDialog() = default;
void ShipCustomWarpDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}
void ShipCustomWarpDialog::reject()
{ // Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}
void ShipCustomWarpDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
}
void ShipCustomWarpDialog::updateUI(const bool firstrun)
{
	util::SignalBlockers blockers(this);
	if (firstrun) {
		for (int i = 0; i < Num_warp_types; ++i) {
			SCP_string name = Warp_types[i];
			if (i == WT_HYPERSPACE) {
				name = "Star Wars";
			}
			ui->comboBoxType->addItem(name.c_str());
		}
		for (const auto& fi : Fireball_info) {
			ui->comboBoxType->addItem(fi.unique_id);
		}
		ui->comboBoxType->setCurrentIndex(_model->getType());

		ui->lineEditStartSound->setText(_model->getStartSound().c_str());
		ui->lineEditEndSound->setText(_model->getEndSound().c_str());
		if (!_model->departMode()) {
			ui->doubleSpinBoxEngage->setEnabled(false);
			ui->labelEngageTime->setEnabled(false);
		} else {
			ui->doubleSpinBoxEngage->setValue(_model->getEngageTime());
		}

		ui->doubleSpinBoxSpeed->setValue(_model->getSpeed());
		ui->doubleSpinBoxTime->setValue(_model->getTime());

		if (_model->departMode()) {
			ui->labelExponent->setText(tr("Deceleration Exponent:"));
		} else {
			ui->labelExponent->setText(tr("Acceleration Exponent:"));
		}
		ui->doubleSpinBoxExponent->setValue(_model->getExponent());
		ui->doubleSpinBoxRadius->setValue(_model->getRadius());
		ui->lineEditAnim->setText(_model->getAnim().c_str());
		if (_model->getSupercap()) {
			ui->checkBoxSupercap->setChecked(true);
		}
		if (!_model->isPlayer() || !_model->departMode()) {
			ui->labelPlayerSpeed->setEnabled(false);
			ui->doubleSpinBoxPlayerSpeed->setEnabled(false);
		} else {
			ui->doubleSpinBoxPlayerSpeed->setValue(_model->getPlayerSpeed());
		}
	}

	if (ui->comboBoxType->currentIndex() == WT_SWEEPER) {
		ui->lineEditAnim->setEnabled(true);
		ui->labelAnim->setEnabled(true);
	} else {
		ui->lineEditAnim->setEnabled(false);
		ui->labelAnim->setEnabled(false);
	}

	if (ui->comboBoxType->currentIndex() == WT_HYPERSPACE) {
		ui->doubleSpinBoxExponent->setEnabled(true);
		ui->labelExponent->setEnabled(true);
	} else {
		ui->doubleSpinBoxExponent->setEnabled(false);
		ui->labelExponent->setEnabled(false);
	}
}
} // namespace fso::fred::dialogs