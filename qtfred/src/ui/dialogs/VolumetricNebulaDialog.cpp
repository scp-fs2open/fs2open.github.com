#include "ui/dialogs/VolumetricNebulaDialog.h"
#include "ui/util/default_dir.h"
#include "ui/util/SignalBlockers.h"

#include "ui_VolumetricNebulaDialog.h"
#include <globalincs/globals.h>
#include <mission/commands/FredCommands.h>
#include <mission/util.h>

#include <ui/util/DialogUndo.h>

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

namespace fso::fred::dialogs {

VolumetricNebulaDialog::VolumetricNebulaDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent), _viewport(viewport), ui(new Ui::VolumetricNebulaDialog()),
	_model(new VolumetricNebulaDialogModel(this, viewport)), _fredView(parent)
{
	this->setFocus();
	ui->setupUi(this);

	_dialogStack = new QUndoStack(this);
	_fredView->undoGroup()->addStack(_dialogStack);
	util::setupDialogUndo(this, _fredView->undoGroup(), _dialogStack, tr("Volumetric Nebula"));

	ui->setModelLineEdit->setMaxLength(MAX_FILENAME_LEN - 1);

	// set our internal values, update the UI
	initializeUi();
	updateUi();
}

VolumetricNebulaDialog::~VolumetricNebulaDialog() = default;

void VolumetricNebulaDialog::accept()
{
	QByteArray stateBefore = _model->captureState();
	if (_model->apply()) {
		QByteArray stateAfter = _model->captureState();
		_model->setParent(nullptr);
		_fredView->mainUndoStack()->push(
			new ApplyDialogCommand(std::move(_model), stateBefore, stateAfter,
			                       _viewport->editor, tr("Edit Volumetric Nebula")));
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::accept();
	}
}

void VolumetricNebulaDialog::reject()
{
	if (!_model) {
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::reject();
		return;
	}
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::reject();
	}
}

void VolumetricNebulaDialog::closeEvent(QCloseEvent* e)
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


void VolumetricNebulaDialog::initializeUi()
{
	util::SignalBlockers blockers(this); // block signals while we set up the UI
	
	// Set ranges
	ui->opacityDoubleSpinBox->setRange(_model->getOpacityLimit().first, _model->getOpacityLimit().second);
	ui->opacityDistanceDoubleSpinBox->setRange(_model->getOpacityDistanceLimit().first, _model->getOpacityDistanceLimit().second);
	ui->renderQualityStepsSpinBox->setRange(_model->getStepsLimit().first, _model->getStepsLimit().second);
	ui->resolutionSpinBox->setRange(_model->getResolutionLimit().first, _model->getResolutionLimit().second);
	ui->oversamplingSpinBox->setRange(_model->getOversamplingLimit().first, _model->getOversamplingLimit().second);
	ui->smoothingDoubleSpinBox->setRange(_model->getSmoothingLimit().first, _model->getSmoothingLimit().second);
	ui->henyeyGreensteinCoeffDoubleSpinBox->setRange(_model->getHenyeyGreensteinLimit().first, _model->getHenyeyGreensteinLimit().second);
	ui->sunFalloffFactorDoubleSpinBox->setRange(_model->getSunFalloffFactorLimit().first, _model->getSunFalloffFactorLimit().second);
	ui->sunQualityStepsSpinBox->setRange(_model->getSunStepsLimit().first, _model->getSunStepsLimit().second);
	ui->emissiveLightDoubleSpinBox->setRange(_model->getEmissiveSpreadLimit().first, _model->getEmissiveSpreadLimit().second);
	ui->emissiveLightIntensityDoubleSpinBox->setRange(_model->getEmissiveIntensityLimit().first, _model->getEmissiveIntensityLimit().second);
	ui->emissiveLightFalloffDoubleSpinBox->setRange(_model->getEmissiveFalloffLimit().first, _model->getEmissiveFalloffLimit().second);
	ui->noiseScaleBaseDoubleSpinBox->setRange(_model->getNoiseScaleBaseLimit().first, _model->getNoiseScaleBaseLimit().second);
	ui->noiseScaleSubDoubleSpinBox->setRange(_model->getNoiseScaleSubLimit().first, _model->getNoiseScaleSubLimit().second);
	ui->noiseIntensityDoubleSpinBox->setRange(_model->getNoiseIntensityLimit().first, _model->getNoiseIntensityLimit().second);
	ui->noiseResolutionSpinBox->setRange(_model->getNoiseResolutionLimit().first, _model->getNoiseResolutionLimit().second);
}

void VolumetricNebulaDialog::updateUi()
{
	util::SignalBlockers blockers(this); // block signals while we update the UI

	enableDisableControls();

	ui->enabled->setChecked(_model->getEnabled());
	
	ui->setModelLineEdit->setText(QString::fromStdString(_model->getHullPof()));
	ui->positionXSpinBox->setValue(_model->getPosX());
	ui->positionYSpinBox->setValue(_model->getPosY());
	ui->positionZSpinBox->setValue(_model->getPosZ());
	ui->colorRSpinBox->setValue(_model->getColorR());
	ui->colorGSpinBox->setValue(_model->getColorG());
	ui->colorBSpinBox->setValue(_model->getColorB());

	ui->opacityDoubleSpinBox->setValue(_model->getOpacity());
	ui->opacityDistanceDoubleSpinBox->setValue(_model->getOpacityDistance());
	ui->renderQualityStepsSpinBox->setValue(_model->getSteps());
	ui->resolutionSpinBox->setValue(_model->getResolution());
	ui->oversamplingSpinBox->setValue(_model->getOversampling());
	ui->smoothingDoubleSpinBox->setValue(_model->getSmoothing());
	ui->henyeyGreensteinCoeffDoubleSpinBox->setValue(_model->getHenyeyGreenstein());
	ui->sunFalloffFactorDoubleSpinBox->setValue(_model->getSunFalloffFactor());
	ui->sunQualityStepsSpinBox->setValue(_model->getSunSteps());

	ui->emissiveLightDoubleSpinBox->setValue(_model->getEmissiveSpread());
	ui->emissiveLightIntensityDoubleSpinBox->setValue(_model->getEmissiveIntensity());
	ui->emissiveLightFalloffDoubleSpinBox->setValue(_model->getEmissiveFalloff());

	ui->enableNoiseCheckBox->setChecked(_model->getNoiseEnabled());
	ui->noiseColorRSpinBox->setValue(_model->getNoiseColorR());
	ui->noiseColorGSpinBox->setValue(_model->getNoiseColorG());
	ui->noiseColorBSpinBox->setValue(_model->getNoiseColorB());
	ui->noiseScaleBaseDoubleSpinBox->setValue(_model->getNoiseScaleBase());
	ui->noiseScaleSubDoubleSpinBox->setValue(_model->getNoiseScaleSub());
	ui->noiseIntensityDoubleSpinBox->setValue(_model->getNoiseIntensity());
	ui->noiseResolutionSpinBox->setValue(_model->getNoiseResolution());

	updateColorSwatch();
	updateNoiseColorSwatch();
}

void VolumetricNebulaDialog::enableDisableControls()
{
	bool enabled = _model->getEnabled();

	ui->setModelButton->setEnabled(enabled);
	ui->setModelLineEdit->setEnabled(enabled);
	ui->positionXSpinBox->setEnabled(enabled);
	ui->positionYSpinBox->setEnabled(enabled);
	ui->positionZSpinBox->setEnabled(enabled);
	ui->colorRSpinBox->setEnabled(enabled);
	ui->colorGSpinBox->setEnabled(enabled);
	ui->colorBSpinBox->setEnabled(enabled);
	ui->opacityDoubleSpinBox->setEnabled(enabled);
	ui->opacityDistanceDoubleSpinBox->setEnabled(enabled);
	ui->renderQualityStepsSpinBox->setEnabled(enabled);
	ui->resolutionSpinBox->setEnabled(enabled);
	ui->oversamplingSpinBox->setEnabled(enabled);
	ui->smoothingDoubleSpinBox->setEnabled(enabled);
	ui->henyeyGreensteinCoeffDoubleSpinBox->setEnabled(enabled);
	ui->sunFalloffFactorDoubleSpinBox->setEnabled(enabled);
	ui->sunQualityStepsSpinBox->setEnabled(enabled);
	ui->emissiveLightDoubleSpinBox->setEnabled(enabled);
	ui->emissiveLightIntensityDoubleSpinBox->setEnabled(enabled);
	ui->emissiveLightFalloffDoubleSpinBox->setEnabled(enabled);

	ui->enableNoiseCheckBox->setEnabled(enabled);

	bool noiseEnabled = enabled && _model->getNoiseEnabled();

	ui->noiseColorRSpinBox->setEnabled(noiseEnabled);
	ui->noiseColorGSpinBox->setEnabled(noiseEnabled);
	ui->noiseColorBSpinBox->setEnabled(noiseEnabled);
	ui->noiseScaleBaseDoubleSpinBox->setEnabled(noiseEnabled);
	ui->noiseScaleSubDoubleSpinBox->setEnabled(noiseEnabled);
	ui->noiseIntensityDoubleSpinBox->setEnabled(noiseEnabled);
	ui->noiseResolutionSpinBox->setEnabled(noiseEnabled);
	ui->setBaseNoiseFunctionButton->setEnabled(noiseEnabled);
	ui->setSubNoiseFunctionButton->setEnabled(noiseEnabled);

}

void VolumetricNebulaDialog::updateColorSwatch()
{
	const int r = _model->getColorR();
	const int g = _model->getColorG();
	const int b = _model->getColorB();
	ui->colorPreview->setStyleSheet(QString("background: rgb(%1,%2,%3);"
										 "border: 1px solid #444; border-radius: 3px;")
			.arg(r)
			.arg(g)
			.arg(b));
}

void VolumetricNebulaDialog::updateNoiseColorSwatch()
{
	const int r = _model->getNoiseColorR();
	const int g = _model->getNoiseColorG();
	const int b = _model->getNoiseColorB();
	ui->noiseColorPreview->setStyleSheet(QString("background: rgb(%1,%2,%3);"
											"border: 1px solid #444; border-radius: 3px;")
			.arg(r)
			.arg(g)
			.arg(b));
}

void VolumetricNebulaDialog::on_okAndCancelButtons_accepted()
{
	accept();
}

void VolumetricNebulaDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

void VolumetricNebulaDialog::on_enabled_toggled(bool checked)
{
	const bool before = _model->getEnabled();
	_model->setEnabled(checked);
	enableDisableControls();
	pushValueCommand(FieldId::Neb_Enabled, tr("Toggle Volumetric Nebula"), before, checked,
		[this](const bool& v) { _model->setEnabled(v); });
}

void VolumetricNebulaDialog::changeHullPof(const SCP_string& name)
{
	const SCP_string before = _model->getHullPof();
	_model->setHullPof(name);
	pushValueCommand(FieldId::Neb_HullPof, tr("Change Nebula Model"), before, _model->getHullPof(),
		[this](const SCP_string& v) { _model->setHullPof(v); });
}

void VolumetricNebulaDialog::on_setModelButton_clicked()
{
	const QString lastDir = util::getLastDir("volumetricNebula/pofModel", CF_TYPE_MODELS);

	const QString path = QFileDialog::getOpenFileName(this,
		"Select POF File",
		lastDir,
		"Freespace 2 Model Files (*.pof);;All Files (*)");
	if (path.isEmpty())
		return;

	util::saveLastDir("volumetricNebula/pofModel", path);
	changeHullPof(QFileInfo(path).fileName().toUtf8().constData());
	updateUi();
}

void VolumetricNebulaDialog::on_setModelLineEdit_textChanged(const QString& text)
{
	changeHullPof(text.toUtf8().constData());
}

void VolumetricNebulaDialog::on_positionXSpinBox_valueChanged(int v)
{
	const float before = _model->getPosX();
	_model->setPosX(static_cast<float>(v));
	pushValueCommand(FieldId::Neb_PosX, tr("Change Nebula Position"), before, _model->getPosX(),
		[this](const float& val) { _model->setPosX(val); });
}

void VolumetricNebulaDialog::on_positionYSpinBox_valueChanged(int v)
{
	const float before = _model->getPosY();
	_model->setPosY(static_cast<float>(v));
	pushValueCommand(FieldId::Neb_PosY, tr("Change Nebula Position"), before, _model->getPosY(),
		[this](const float& val) { _model->setPosY(val); });
}

void VolumetricNebulaDialog::on_positionZSpinBox_valueChanged(int v)
{
	const float before = _model->getPosZ();
	_model->setPosZ(static_cast<float>(v));
	pushValueCommand(FieldId::Neb_PosZ, tr("Change Nebula Position"), before, _model->getPosZ(),
		[this](const float& val) { _model->setPosZ(val); });
}

void VolumetricNebulaDialog::on_colorRSpinBox_valueChanged(int v)
{
	const int before = _model->getColorR();
	_model->setColorR(v);
	updateColorSwatch();
	pushValueCommand(FieldId::Neb_ColorR, tr("Change Nebula Color"), before, _model->getColorR(),
		[this](const int& val) { _model->setColorR(val); });
}

void VolumetricNebulaDialog::on_colorGSpinBox_valueChanged(int v)
{
	const int before = _model->getColorG();
	_model->setColorG(v);
	updateColorSwatch();
	pushValueCommand(FieldId::Neb_ColorG, tr("Change Nebula Color"), before, _model->getColorG(),
		[this](const int& val) { _model->setColorG(val); });
}

void VolumetricNebulaDialog::on_colorBSpinBox_valueChanged(int v)
{
	const int before = _model->getColorB();
	_model->setColorB(v);
	updateColorSwatch();
	pushValueCommand(FieldId::Neb_ColorB, tr("Change Nebula Color"), before, _model->getColorB(),
		[this](const int& val) { _model->setColorB(val); });
}

void VolumetricNebulaDialog::on_opacityDoubleSpinBox_valueChanged(double v)
{
	const float before = _model->getOpacity();
	_model->setOpacity(static_cast<float>(v));
	pushValueCommand(FieldId::Neb_Opacity, tr("Change Opacity"), before, _model->getOpacity(),
		[this](const float& val) { _model->setOpacity(val); });
}

void VolumetricNebulaDialog::on_opacityDistanceDoubleSpinBox_valueChanged(double v)
{
	const float before = _model->getOpacityDistance();
	_model->setOpacityDistance(static_cast<float>(v));
	pushValueCommand(FieldId::Neb_OpacityDistance, tr("Change Opacity Distance"), before, _model->getOpacityDistance(),
		[this](const float& val) { _model->setOpacityDistance(val); });
}

void VolumetricNebulaDialog::on_renderQualityStepsSpinBox_valueChanged(int v)
{
	const int before = _model->getSteps();
	_model->setSteps(v);
	pushValueCommand(FieldId::Neb_Steps, tr("Change Quality Steps"), before, _model->getSteps(),
		[this](const int& val) { _model->setSteps(val); });
}

void VolumetricNebulaDialog::on_resolutionSpinBox_valueChanged(int v)
{
	const int before = _model->getResolution();
	_model->setResolution(v);
	pushValueCommand(FieldId::Neb_Resolution, tr("Change Resolution"), before, _model->getResolution(),
		[this](const int& val) { _model->setResolution(val); });
}

void VolumetricNebulaDialog::on_oversamplingSpinBox_valueChanged(int v)
{
	const int before = _model->getOversampling();
	_model->setOversampling(v);
	pushValueCommand(FieldId::Neb_Oversampling, tr("Change Oversampling"), before, _model->getOversampling(),
		[this](const int& val) { _model->setOversampling(val); });
}

void VolumetricNebulaDialog::on_smoothingDoubleSpinBox_valueChanged(double v)
{
	const float before = _model->getSmoothing();
	_model->setSmoothing(static_cast<float>(v));
	pushValueCommand(FieldId::Neb_Smoothing, tr("Change Smoothing"), before, _model->getSmoothing(),
		[this](const float& val) { _model->setSmoothing(val); });
}

void VolumetricNebulaDialog::on_henyeyGreensteinCoeffDoubleSpinBox_valueChanged(double v)
{
	const float before = _model->getHenyeyGreenstein();
	_model->setHenyeyGreenstein(static_cast<float>(v));
	pushValueCommand(FieldId::Neb_HenyeyGreenstein, tr("Change Henyey-Greenstein Coefficient"), before,
		_model->getHenyeyGreenstein(), [this](const float& val) { _model->setHenyeyGreenstein(val); });
}

void VolumetricNebulaDialog::on_sunFalloffFactorDoubleSpinBox_valueChanged(double v)
{
	const float before = _model->getSunFalloffFactor();
	_model->setSunFalloffFactor(static_cast<float>(v));
	pushValueCommand(FieldId::Neb_SunFalloff, tr("Change Sun Falloff Factor"), before, _model->getSunFalloffFactor(),
		[this](const float& val) { _model->setSunFalloffFactor(val); });
}

void VolumetricNebulaDialog::on_sunQualityStepsSpinBox_valueChanged(int v)
{
	const int before = _model->getSunSteps();
	_model->setSunSteps(v);
	pushValueCommand(FieldId::Neb_SunSteps, tr("Change Sun Quality Steps"), before, _model->getSunSteps(),
		[this](const int& val) { _model->setSunSteps(val); });
}

void VolumetricNebulaDialog::on_emissiveLightDoubleSpinBox_valueChanged(double v)
{
	const float before = _model->getEmissiveSpread();
	_model->setEmissiveSpread(static_cast<float>(v));
	pushValueCommand(FieldId::Neb_EmissiveSpread, tr("Change Emissive Spread"), before, _model->getEmissiveSpread(),
		[this](const float& val) { _model->setEmissiveSpread(val); });
}

void VolumetricNebulaDialog::on_emissiveLightIntensityDoubleSpinBox_valueChanged(double v)
{
	const float before = _model->getEmissiveIntensity();
	_model->setEmissiveIntensity(static_cast<float>(v));
	pushValueCommand(FieldId::Neb_EmissiveIntensity, tr("Change Emissive Intensity"), before,
		_model->getEmissiveIntensity(), [this](const float& val) { _model->setEmissiveIntensity(val); });
}

void VolumetricNebulaDialog::on_emissiveLightFalloffDoubleSpinBox_valueChanged(double v)
{
	const float before = _model->getEmissiveFalloff();
	_model->setEmissiveFalloff(static_cast<float>(v));
	pushValueCommand(FieldId::Neb_EmissiveFalloff, tr("Change Emissive Falloff"), before, _model->getEmissiveFalloff(),
		[this](const float& val) { _model->setEmissiveFalloff(val); });
}

void VolumetricNebulaDialog::on_enableNoiseCheckBox_toggled(bool enabled)
{
	const bool before = _model->getNoiseEnabled();
	_model->setNoiseEnabled(enabled);
	enableDisableControls();
	pushValueCommand(FieldId::Neb_NoiseEnabled, tr("Toggle Nebula Noise"), before, enabled,
		[this](const bool& v) { _model->setNoiseEnabled(v); });
}

void VolumetricNebulaDialog::on_noiseColorRSpinBox_valueChanged(int v)
{
	const int before = _model->getNoiseColorR();
	_model->setNoiseColorR(v);
	updateNoiseColorSwatch();
	pushValueCommand(FieldId::Neb_NoiseColorR, tr("Change Noise Color"), before, _model->getNoiseColorR(),
		[this](const int& val) { _model->setNoiseColorR(val); });
}

void VolumetricNebulaDialog::on_noiseColorGSpinBox_valueChanged(int v)
{
	const int before = _model->getNoiseColorG();
	_model->setNoiseColorG(v);
	updateNoiseColorSwatch();
	pushValueCommand(FieldId::Neb_NoiseColorG, tr("Change Noise Color"), before, _model->getNoiseColorG(),
		[this](const int& val) { _model->setNoiseColorG(val); });
}

void VolumetricNebulaDialog::on_noiseColorBSpinBox_valueChanged(int v)
{
	const int before = _model->getNoiseColorB();
	_model->setNoiseColorB(v);
	updateNoiseColorSwatch();
	pushValueCommand(FieldId::Neb_NoiseColorB, tr("Change Noise Color"), before, _model->getNoiseColorB(),
		[this](const int& val) { _model->setNoiseColorB(val); });
}

void VolumetricNebulaDialog::on_noiseScaleBaseDoubleSpinBox_valueChanged(double v)
{
	const float before = _model->getNoiseScaleBase();
	_model->setNoiseScaleBase(static_cast<float>(v));
	pushValueCommand(FieldId::Neb_NoiseScaleBase, tr("Change Noise Base Scale"), before, _model->getNoiseScaleBase(),
		[this](const float& val) { _model->setNoiseScaleBase(val); });
}

void VolumetricNebulaDialog::on_noiseScaleSubDoubleSpinBox_valueChanged(double v)
{
	const float before = _model->getNoiseScaleSub();
	_model->setNoiseScaleSub(static_cast<float>(v));
	pushValueCommand(FieldId::Neb_NoiseScaleSub, tr("Change Noise Sub Scale"), before, _model->getNoiseScaleSub(),
		[this](const float& val) { _model->setNoiseScaleSub(val); });
}

void VolumetricNebulaDialog::on_noiseIntensityDoubleSpinBox_valueChanged(double v)
{
	const float before = _model->getNoiseIntensity();
	_model->setNoiseIntensity(static_cast<float>(v));
	pushValueCommand(FieldId::Neb_NoiseIntensity, tr("Change Noise Intensity"), before, _model->getNoiseIntensity(),
		[this](const float& val) { _model->setNoiseIntensity(val); });
}

void VolumetricNebulaDialog::on_noiseResolutionSpinBox_valueChanged(int v)
{
	const int before = _model->getNoiseResolution();
	_model->setNoiseResolution(v);
	pushValueCommand(FieldId::Neb_NoiseResolution, tr("Change Noise Resolution"), before, _model->getNoiseResolution(),
		[this](const int& val) { _model->setNoiseResolution(val); });
}

void VolumetricNebulaDialog::on_setBaseNoiseFunctionButton_clicked()
{
	QMessageBox::information(this, "Not Implemented", "Setting the base noise function is not implemented yet.");
}

void VolumetricNebulaDialog::on_setSubNoiseFunctionButton_clicked()
{
	QMessageBox::information(this, "Not Implemented", "Setting the sub noise function is not implemented yet.");
}

} // namespace fso::fred::dialogs
