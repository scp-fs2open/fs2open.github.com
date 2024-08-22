#include "ShipCustomWarpDialog.h"

#include "ui_ShipCustomWarpDialog.h"

#include <ship/shipfx.h>
#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>

namespace fso {
namespace fred {
namespace dialogs {
ShipCustomWarpDialog::ShipCustomWarpDialog(QDialog* parent, EditorViewport* viewport, bool departure)
	: QDialog(parent), ui(new Ui::ShipCustomWarpDialog()),
	  _model(new ShipCustomWarpDialogModel(this, viewport, departure)), _viewport(viewport)
{
	ui->setupUi(this);
	connect(this, &QDialog::accepted, _model.get(), &ShipCustomWarpDialogModel::apply);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ShipCustomWarpDialog::rejectHandler);

	updateUI(true);

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

ShipCustomWarpDialog::~ShipCustomWarpDialog() = default;
void ShipCustomWarpDialog::closeEvent(QCloseEvent* e)
{
	if (_model->query_modified()) {
		auto button = _viewport->dialogProvider->showButtonDialog(DialogType::Question,
			"Changes detected",
			"Do you want to keep your changes?",
			{DialogButton::Yes, DialogButton::No, DialogButton::Cancel});

		if (button == DialogButton::Cancel) {
			e->ignore();
			return;
		}

		if (button == DialogButton::Yes) {
			accept();
			return;
		}
		if (button == DialogButton::No) {
			_model->reject();
		}
	}

	QDialog::closeEvent(e);
}
void ShipCustomWarpDialog::updateUI(const bool firstrun)
{
	util::SignalBlockers blockers(this);
	if (firstrun) {
		for (int i = 0; i < Num_warp_types; ++i) {
			SCP_string name = Warp_types[i];
			if (name == "Hyperspace") {
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
			ui->doubleSpinBoxEngage->setVisible(false);
			ui->labelEngageTime->setVisible(false);
		} else {
			ui->doubleSpinBoxEngage->setValue(_model->getEngageTime());
		}

		ui->doubleSpinBoxSpeed->setValue(_model->getSpeed());
		ui->doubleSpinBoxTime->setValue(_model->getTime());

		if (_model->departMode()) {
			ui->labelExponent->setText(tr("Deceleration Exponent:"));
		} else {
			ui->labelExponent->setText(tr("Aceleration Exponent:"));
		}
		ui->doubleSpinBoxExponent->setValue(_model->getExponent());
		ui->doubleSpinBoxRadius->setValue(_model->getRadius());
		ui->lineEditAnim->setText(_model->getAnim().c_str());
		if (_model->getSupercap()) {
			ui->checkBoxSupercap->setChecked(true);
		}
		if (!_model->isPlayer() || !_model->departMode()) {
			ui->labelPlayerSpeed->setVisible(false);
			ui->doubleSpinBoxPlayerSpeed->setVisible(false);
		} else {
			ui->doubleSpinBoxPlayerSpeed->setValue(_model->getPlayerSpeed());
		}
	}

	if (ui->comboBoxType->itemText(ui->comboBoxType->currentIndex()) == "Homeworld")
		{
		ui->lineEditAnim->setVisible(true);
		ui->labelAnim->setVisible(true);
	} else {
		ui->lineEditAnim->setVisible(false);
		ui->labelAnim->setVisible(false);
	}

	if (ui->comboBoxType->itemText(ui->comboBoxType->currentIndex()) == "Star Wars") {
		ui->doubleSpinBoxExponent->setVisible(true);
		ui->labelExponent->setVisible(true);
	} else {
		ui->doubleSpinBoxExponent->setVisible(false);
		ui->labelExponent->setVisible(false);
	}
}
void ShipCustomWarpDialog::rejectHandler() {}
} // namespace dialogs
} // namespace fred
} // namespace fso