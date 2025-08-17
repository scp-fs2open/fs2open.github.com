#include "SoundEnvironmentDialog.h"

#include "ui_SoundEnvironmentDialog.h"

#include "cfile/cfile.h"
#include "sound/audiostr.h"
#include "sound/ds.h"
#include "sound/sound.h"

#include <ui/util/SignalBlockers.h>

#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>

using namespace fso::fred::dialogs;

SoundEnvironmentDialog::SoundEnvironmentDialog(QWidget* parent, EditorViewport* viewport)
	: QDialog(parent), ui(std::make_unique<Ui::SoundEnvironmentDialog>()),
	  _model(new SoundEnvironmentDialogModel(this, viewport)), _viewport(viewport)
{
	ui->setupUi(this);

	populatePresets();

	applyPresetFields();
}

SoundEnvironmentDialog::~SoundEnvironmentDialog() = default;

void SoundEnvironmentDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close

	closeWave();
	disableEnvPreview();
}

void SoundEnvironmentDialog::reject()
{
	// Custom reject or close logic because we need to handle talkback to Mission Specs
	if (_model->query_modified()) {
		auto button = _viewport->dialogProvider->showButtonDialog(fso::fred::DialogType::Question,
			"Changes detected",
			"Do you want to keep your changes?",
			{fso::fred::DialogButton::Yes, fso::fred::DialogButton::No, fso::fred::DialogButton::Cancel});

		if (button == fso::fred::DialogButton::Yes) {
			accept();
		}
		if (button == fso::fred::DialogButton::No) {
			_model->reject();
			QDialog::reject(); // actually close

			closeWave();
			disableEnvPreview();
		}
	} else {
		_model->reject();
		QDialog::reject();

		closeWave();
		disableEnvPreview();
	}
}

void SoundEnvironmentDialog::closeEvent(QCloseEvent* e)
{
	reject();
	closeWave();
	disableEnvPreview();
	e->ignore(); // Don't let the base class close the window
}

void SoundEnvironmentDialog::enableOrDisableFields()
{
	const bool enabled = ui->environmentComboBox->currentIndex() > 0;
	ui->volumeSpinBox->setEnabled(enabled);
	ui->dampingSpinBox->setEnabled(enabled);
	ui->decaySpinBox->setEnabled(enabled);
	ui->browseButton->setEnabled(enabled);
	ui->playButton->setEnabled(enabled && _waveId >= 0);
}

void SoundEnvironmentDialog::populatePresets()
{
	util::SignalBlockers blockers(this);

	ui->environmentComboBox->clear();
	ui->environmentComboBox->addItem(QString("<none>")); // index 0 = none
	for (auto& preset : EFX_presets) {
		ui->environmentComboBox->addItem(QString::fromStdString(preset.name));
	}
}

void SoundEnvironmentDialog::applyPresetFields()
{
	util::SignalBlockers blockers(this);

	int presetIndex = _model->getId();

	if (presetIndex >= 0) {
		const auto& p = EFX_presets[presetIndex];
		ui->volumeSpinBox->setValue(p.flGain);
		ui->dampingSpinBox->setValue(p.flDecayHFRatio);
		ui->decaySpinBox->setValue(p.flDecayTime);
	} else { // <none>
		ui->volumeSpinBox->setValue(0.0);
		ui->dampingSpinBox->setValue(0.1);
		ui->decaySpinBox->setValue(0.1);
	}

	enableOrDisableFields();
}

void SoundEnvironmentDialog::setInitial(const sound_env& env)
{
	_model->setInitial(env);

	util::SignalBlockers blockers(this);

	ui->environmentComboBox->setCurrentIndex(env.id + 1);
	ui->volumeSpinBox->setValue(env.volume);
	ui->dampingSpinBox->setValue(env.damping);
	ui->decaySpinBox->setValue(env.decay);

	enableOrDisableFields();
}

sound_env SoundEnvironmentDialog::items() const
{
	return _model->params();
}

void SoundEnvironmentDialog::on_environmentComboBox_currentIndexChanged(int index)
{
	_model->setId(index - 1);
	applyPresetFields();
}

void SoundEnvironmentDialog::on_volumeSpinBox_valueChanged(double value)
{
	_model->setVolume(static_cast<float>(value));
}

void SoundEnvironmentDialog::on_dampingSpinBox_valueChanged(double value)
{
	_model->setDamping(static_cast<float>(value));
}

void SoundEnvironmentDialog::on_decaySpinBox_valueChanged(double value)
{
	_model->setDecay(static_cast<float>(value));
}

void SoundEnvironmentDialog::on_browseButton_clicked()
{
	closeWave();

	const int pushed = cfile_push_chdir(CF_TYPE_DATA);
	const QString filter = "Voice Files (*.ogg *.wav);;Ogg Vorbis Files (*.ogg);;Wave Files (*.wav)";
	const auto path = QFileDialog::getOpenFileName(this, tr("Choose sound"), QString(), filter);

	if (!path.isEmpty()) {
		const QString justName = QFileInfo(path).fileName();
		_waveId = audiostream_open(justName.toUtf8().constData(), ASF_SOUNDFX);
		ui->fileSelectionLabel->setText(justName);
	}
	if (!pushed)
		cfile_pop_dir();

	enableOrDisableFields();
}

void SoundEnvironmentDialog::on_playButton_clicked()
{
	if (!sound_env_supported()) {
		QMessageBox::warning(this, tr("Error"), tr("Sound environment effects are not available! Unable to preview!"));
		return;
	}
	if (_waveId < 0) {
		on_browseButton_clicked();
		if (_waveId < 0)
			return;
	}

	// Build a temp env from current fields and set it active for preview.
	sound_env temp = _model->params();

	if (temp.id >= 0) {
		sound_env_set(&temp);
	} else {
		sound_env_disable();
	}

	audiostream_play(_waveId, 1.0f, 0); // simple preview
}

void SoundEnvironmentDialog::on_okAndCancelButtons_accepted()
{
	accept(); // Mission Specs will read model->items() and commit during its own Apply
}

void SoundEnvironmentDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

void SoundEnvironmentDialog::closeWave()
{
	if (_waveId >= 0) {
		audiostream_close_file(_waveId, false);
		ui->fileSelectionLabel->setText(QString("<No File Selected>")); // clear label
		_waveId = -1;
	}
}

void SoundEnvironmentDialog::disableEnvPreview()
{
	sound_env_disable();
}
