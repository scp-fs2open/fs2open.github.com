#include "ui/dialogs/VolumetricNebulaDialog.h"
#include "ui/util/SignalBlockers.h"

#include "ui_VolumetricNebulaDialog.h"
#include <mission/util.h>

#include <QFileDialog>
#include <QMessageBox>

namespace fso::fred::dialogs {

VolumetricNebulaDialog::VolumetricNebulaDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent), _viewport(viewport), ui(new Ui::VolumetricNebulaDialog()),
	_model(new VolumetricNebulaDialogModel(this, viewport))
{
	this->setFocus();
	ui->setupUi(this);

	// set our internal values, update the UI
	initializeUi();
	updateUi();
}

VolumetricNebulaDialog::~VolumetricNebulaDialog() = default;

void VolumetricNebulaDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void VolumetricNebulaDialog::reject()
{
	// Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void VolumetricNebulaDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
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
	_model->setEnabled(checked);
	enableDisableControls();
}

void VolumetricNebulaDialog::on_setModelButton_clicked()
{
	const QString path = QFileDialog::getOpenFileName(this,
		"Select POF File",
		QString(),
		"Freespace 2 Model Files (*.pof);;All Files (*)");
	if (path.isEmpty())
		return;

	const QString filename = QFileInfo(path).fileName();
	_model->setHullPof(filename.toUtf8().constData());
	updateUi();
}

void VolumetricNebulaDialog::on_setModelLineEdit_textChanged(const QString& text)
{
	_model->setHullPof(text.toUtf8().constData());
}

void VolumetricNebulaDialog::on_positionXSpinBox_valueChanged(int v)
{
	_model->setPosX(static_cast<float>(v));
}

void VolumetricNebulaDialog::on_positionYSpinBox_valueChanged(int v)
{
	_model->setPosY(static_cast<float>(v));
}

void VolumetricNebulaDialog::on_positionZSpinBox_valueChanged(int v)
{
	_model->setPosZ(static_cast<float>(v));
}

void VolumetricNebulaDialog::on_colorRSpinBox_valueChanged(int v)
{
	_model->setColorR(v);
	updateColorSwatch();
}

void VolumetricNebulaDialog::on_colorGSpinBox_valueChanged(int v)
{
	_model->setColorG(v);
	updateColorSwatch();
}

void VolumetricNebulaDialog::on_colorBSpinBox_valueChanged(int v)
{
	_model->setColorB(v);
	updateColorSwatch();
}

void VolumetricNebulaDialog::on_opacityDoubleSpinBox_valueChanged(double v)
{
	_model->setOpacity(static_cast<float>(v));
}

void VolumetricNebulaDialog::on_opacityDistanceDoubleSpinBox_valueChanged(double v)
{
	_model->setOpacityDistance(static_cast<float>(v));
}

void VolumetricNebulaDialog::on_renderQualityStepsSpinBox_valueChanged(int v)
{
	_model->setSteps(v);
}

void VolumetricNebulaDialog::on_resolutionSpinBox_valueChanged(int v)
{
	_model->setResolution(v);
}

void VolumetricNebulaDialog::on_oversamplingSpinBox_valueChanged(int v)
{
	_model->setOversampling(v);
}

void VolumetricNebulaDialog::on_smoothingDoubleSpinBox_valueChanged(double v)
{
	_model->setSmoothing(static_cast<float>(v));
}

void VolumetricNebulaDialog::on_henyeyGreensteinCoeffDoubleSpinBox_valueChanged(double v)
{
	_model->setHenyeyGreenstein(static_cast<float>(v));
}

void VolumetricNebulaDialog::on_sunFalloffFactorDoubleSpinBox_valueChanged(double v)
{
	_model->setSunFalloffFactor(static_cast<float>(v));
}

void VolumetricNebulaDialog::on_sunQualityStepsSpinBox_valueChanged(int v)
{
	_model->setSunSteps(v);
}

void VolumetricNebulaDialog::on_emissiveLightDoubleSpinBox_valueChanged(double v)
{
	_model->setEmissiveSpread(static_cast<float>(v));
}

void VolumetricNebulaDialog::on_emissiveLightIntensityDoubleSpinBox_valueChanged(double v)
{
	_model->setEmissiveIntensity(static_cast<float>(v));
}

void VolumetricNebulaDialog::on_emissiveLightFalloffDoubleSpinBox_valueChanged(double v)
{
	_model->setEmissiveFalloff(static_cast<float>(v));
}

void VolumetricNebulaDialog::on_enableNoiseCheckBox_toggled(bool enabled)
{
	_model->setNoiseEnabled(enabled);
	enableDisableControls();
}

void VolumetricNebulaDialog::on_noiseColorRSpinBox_valueChanged(int v)
{
	_model->setNoiseColorR(v);
	updateNoiseColorSwatch();
}

void VolumetricNebulaDialog::on_noiseColorGSpinBox_valueChanged(int v)
{
	_model->setNoiseColorG(v);
	updateNoiseColorSwatch();
}

void VolumetricNebulaDialog::on_noiseColorBSpinBox_valueChanged(int v)
{
	_model->setNoiseColorB(v);
	updateNoiseColorSwatch();
}

void VolumetricNebulaDialog::on_noiseScaleBaseDoubleSpinBox_valueChanged(double v)
{
	_model->setNoiseScaleBase(static_cast<float>(v));
}

void VolumetricNebulaDialog::on_noiseScaleSubDoubleSpinBox_valueChanged(double v)
{
	_model->setNoiseScaleSub(static_cast<float>(v));
}

void VolumetricNebulaDialog::on_noiseIntensityDoubleSpinBox_valueChanged(double v)
{
	_model->setNoiseIntensity(static_cast<float>(v));
}

void VolumetricNebulaDialog::on_noiseResolutionSpinBox_valueChanged(int v)
{
	_model->setNoiseResolution(v);
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
