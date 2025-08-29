#pragma once

#include <QDialog>

#include <mission/dialogs/VolumetricNebulaDialogModel.h>
#include <ui/FredView.h>

namespace fso::fred::dialogs {

namespace Ui {
class VolumetricNebulaDialog;
}

class VolumetricNebulaDialog : public QDialog {
	Q_OBJECT
public:
	VolumetricNebulaDialog(FredView* parent, EditorViewport* viewport);
	~VolumetricNebulaDialog() override;

	void accept() override;
	void reject() override;

protected:
	void closeEvent(QCloseEvent* e) override; // funnel all Window X presses through reject()


private slots:
	// dialog controls
	void on_okAndCancelButtons_accepted();
	void on_okAndCancelButtons_rejected();

	// Master toggle
	void on_enabled_toggled(bool enabled);

	// Model
	void on_setModelButton_clicked();
	void on_setModelLineEdit_textChanged(const QString& text);

	// Position
	void on_positionXSpinBox_valueChanged(int v);
	void on_positionYSpinBox_valueChanged(int v);
	void on_positionZSpinBox_valueChanged(int v);

	// Color
	void on_colorRSpinBox_valueChanged(int v);
	void on_colorGSpinBox_valueChanged(int v);
	void on_colorBSpinBox_valueChanged(int v);

	// Visibility
	void on_opacityDoubleSpinBox_valueChanged(double v);
	void on_opacityDistanceDoubleSpinBox_valueChanged(double v);

	// Quality
	void on_renderQualityStepsSpinBox_valueChanged(int v);
	void on_resolutionSpinBox_valueChanged(int v);
	void on_oversamplingSpinBox_valueChanged(int v);
	void on_smoothingDoubleSpinBox_valueChanged(double v);

	// Lighting
	void on_henyeyGreensteinCoeffDoubleSpinBox_valueChanged(double v);
	void on_sunFalloffFactorDoubleSpinBox_valueChanged(double v);
	void on_sunQualityStepsSpinBox_valueChanged(int v);

	// Emissive
	void on_emissiveLightDoubleSpinBox_valueChanged(double v);
	void on_emissiveLightIntensityDoubleSpinBox_valueChanged(double v);
	void on_emissiveLightFalloffDoubleSpinBox_valueChanged(double v);

	// Noise toggle
	void on_enableNoiseCheckBox_toggled(bool enabled);

	// Noise params
	void on_noiseColorRSpinBox_valueChanged(int v);
	void on_noiseColorGSpinBox_valueChanged(int v);
	void on_noiseColorBSpinBox_valueChanged(int v);
	void on_noiseScaleBaseDoubleSpinBox_valueChanged(double v);
	void on_noiseScaleSubDoubleSpinBox_valueChanged(double v);
	void on_noiseIntensityDoubleSpinBox_valueChanged(double v);
	void on_noiseResolutionSpinBox_valueChanged(int v);

	// Noise functions
	void on_setBaseNoiseFunctionButton_clicked();
	void on_setSubNoiseFunctionButton_clicked();
	

private: // NOLINT(readability-redundant-access-specifiers)
	void initializeUi();
	void updateUi();
	void enableDisableControls();

	void updateColorSwatch();
	void updateNoiseColorSwatch();

	// Boilerplate
	EditorViewport* _viewport = nullptr;
	std::unique_ptr<Ui::VolumetricNebulaDialog> ui;
	std::unique_ptr<VolumetricNebulaDialogModel> _model;
};


} // namespace fso::fred::dialogs
