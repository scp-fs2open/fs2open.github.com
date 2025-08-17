#pragma once
#include "mission/dialogs/MissionSpecs/SoundEnvironmentDialogModel.h"

#include <ui/FredView.h>

#include "sound/sound.h" // for sound_env

#include <QDialog>

class EditorViewport;

namespace fso::fred::dialogs {

namespace Ui {
class SoundEnvironmentDialog;
}

class SoundEnvironmentDialog final : public QDialog {
	Q_OBJECT
  public:
	SoundEnvironmentDialog(QWidget* parent, EditorViewport* viewport);
	~SoundEnvironmentDialog() override;

	void accept() override;
	void reject() override;

	void setInitial(const sound_env& env); // seed from Mission Specs
	sound_env items() const;               // pull back edited data

  protected:
	void closeEvent(QCloseEvent* e) override;

  private slots:
	void on_environmentComboBox_currentIndexChanged(int index);
	void on_volumeSpinBox_valueChanged(double value);
	void on_dampingSpinBox_valueChanged(double value);
	void on_decaySpinBox_valueChanged(double value);

	void on_browseButton_clicked();
	void on_playButton_clicked();

	// Dialog buttons
	void on_okAndCancelButtons_accepted();
	void on_okAndCancelButtons_rejected();

  private: // NOLINT(readability-redundant-access-specifiers)
	void enableOrDisableFields();
	void populatePresets();
	void applyPresetFields();
	void closeWave();
	static void disableEnvPreview();

	std::unique_ptr<Ui::SoundEnvironmentDialog> ui;
	std::unique_ptr<SoundEnvironmentDialogModel> _model;
	EditorViewport* _viewport;

	int _waveId = -1;
};

} // namespace fso::fred::dialogs
