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
	connect(this, &QDialog::accepted, _model.get(), &ShipCustomWarpDialogModel::apply);

	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ShipCustomWarpDialog::rejectHandler);

	connect(_model.get(), &AbstractDialogModel::modelChanged, this, [this]() { updateUI(false); });

	connect(ui->comboBoxType,
		static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		_model.get(),
		&ShipCustomWarpDialogModel::setType);
	connect(ui->lineEditStartSound, &QLineEdit::editingFinished, this, &ShipCustomWarpDialog::startSoundChanged);
	connect(ui->lineEditEndSound, &QLineEdit::editingFinished, this, &ShipCustomWarpDialog::endSoundChanged);
	connect(ui->doubleSpinBoxEngage,
		QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		_model.get(),
		&ShipCustomWarpDialogModel::setEngageTime);
	connect(ui->doubleSpinBoxSpeed,
		QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		_model.get(),
		&ShipCustomWarpDialogModel::setSpeed);
	connect(ui->doubleSpinBoxTime,
		QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		_model.get(),
		&ShipCustomWarpDialogModel::setTime);
	connect(ui->doubleSpinBoxExponent,
		QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		_model.get(),
		&ShipCustomWarpDialogModel::setExponent);
	connect(ui->doubleSpinBoxRadius,
		QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		_model.get(),
		&ShipCustomWarpDialogModel::setRadius);
	connect(ui->checkBoxSupercap, &QCheckBox::toggled, _model.get(), [=](bool param) {
		static_cast<ShipCustomWarpDialogModel*>(_model.get())->setSupercap(param);
	});
	connect(ui->lineEditAnim, &QLineEdit::editingFinished, this, &ShipCustomWarpDialog::animChanged);
	connect(ui->doubleSpinBoxPlayerSpeed,
		QOverload<double>::of(&QDoubleSpinBox::valueChanged),
		_model.get(),
		&ShipCustomWarpDialogModel::setPlayerSpeed);

	if (departure) {
		this->setWindowTitle("Edit Warp-Out Parameters");
	} else {
		this->setWindowTitle("Edit Warp-In Parameters");
	}
	updateUI(true);
	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

ShipCustomWarpDialog::~ShipCustomWarpDialog() = default;
void ShipCustomWarpDialog::closeEvent(QCloseEvent* e)
{
	if (!rejectOrCloseHandler(this, _model.get(), _viewport)) {
		e->ignore();
	};
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
void ShipCustomWarpDialog::rejectHandler()
{
	this->close();
}

void ShipCustomWarpDialog::startSoundChanged()
{
	// String wrangling reqired in order to avoid crashes when directly converting from Qstring to std::string on some
	// enviroments
	QString temp(ui->lineEditStartSound->text());
	if (!temp.isEmpty()) {
		_model->setStartSound(temp.toLatin1().constData());
	} else {
		_model->setStartSound("");
	}
}
void ShipCustomWarpDialog::endSoundChanged()
{
	// String wrangling reqired in order to avoid crashes when directly converting from Qstring to std::string on some
	// enviroments
	QString temp(ui->lineEditEndSound->text());
	if (!temp.isEmpty()) {
		_model->setEndSound(temp.toLatin1().constData());
	} else {
		_model->setEndSound("");
	}
}
void ShipCustomWarpDialog::animChanged()
{
	// String wrangling reqired in order to avoid crashes when directly converting from Qstring to std::string on some
	// enviroments
	QString temp(ui->lineEditAnim->text());
	if (!temp.isEmpty()) {
		_model->setAnim(temp.toLatin1().constData());
	} else {
		_model->setAnim("");
	}
}
} // namespace fso::fred::dialogs