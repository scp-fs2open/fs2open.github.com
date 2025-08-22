
#include <QCloseEvent>
#include "ui/dialogs/FictionViewerDialog.h"
#include "ui/util/SignalBlockers.h"
#include "ui_FictionViewerDialog.h"
#include "mission/util.h"

namespace fso::fred::dialogs {

FictionViewerDialog::FictionViewerDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent), _viewport(viewport), ui(new Ui::FictionViewerDialog()), _model(new FictionViewerDialogModel(this, viewport))
{

	ui->setupUi(this);

	// Initial set up of the UI
	initializeUi();
	updateUi();

	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());

	// Fiction viewer can have multiple *conditional* stages but only ever displays one during a mission.
	// So in order to properly handle multiple stages in the editor we will need to add a formula editor
	// to the dialog like goals or cutscenes. It looks like formulas already saved/parsed in the mission file
	// so this is just an editor UI limitation maybe? This should be handled in the next pass at the FV dialog
	// because the model doesn't yet support reading/writing the formula
	/*if (_model->hasMultipleStages()) {
		viewport->dialogProvider->showButtonDialog(DialogType::Information, "Multiple stages detected",
			"This mission has multiple fiction viewer stages defined.  Currently, qtFRED will only allow you to edit the first stage.",
			{ DialogButton::Ok});
	}*/
}
FictionViewerDialog::~FictionViewerDialog() = default;

void FictionViewerDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void FictionViewerDialog::reject()
{
	// Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void FictionViewerDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
}

void FictionViewerDialog::initializeUi()
{
	ui->storyFileEdit->setMaxLength(_model->getMaxStoryFileLength());
	ui->fontFileEdit->setMaxLength(_model->getMaxFontFileLength());
	ui->voiceFileEdit->setMaxLength(_model->getMaxVoiceFileLength());

	updateMusicComboBox();
}

void FictionViewerDialog::updateUi() {
	util::SignalBlockers blockers(this);

	ui->storyFileEdit->setText(QString::fromStdString(_model->getStoryFile()));
	ui->fontFileEdit->setText(QString::fromStdString(_model->getFontFile()));
	ui->voiceFileEdit->setText(QString::fromStdString(_model->getVoiceFile()));
	ui->musicComboBox->setCurrentIndex(ui->musicComboBox->findData(_model->getFictionMusic()));
}

void FictionViewerDialog::updateMusicComboBox()
{
	util::SignalBlockers blockers(this);
	
	ui->musicComboBox->clear();

	const auto& musicOptions = _model->getMusicOptions();

	if (musicOptions.empty()) {
		ui->musicComboBox->setEnabled(false);
		return;
	}

	ui->musicComboBox->setEnabled(true);
	for (const auto& option : musicOptions) {
		ui->musicComboBox->addItem(QString::fromStdString(option.first), option.second);
	}
}

void FictionViewerDialog::on_okAndCancelButtons_accepted()
{
	accept();
}

void FictionViewerDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

void FictionViewerDialog::on_storyFileEdit_textChanged(const QString& text)
{
	_model->setStoryFile(text.toUtf8().constData());
}

void FictionViewerDialog::on_fontFileEdit_textChanged(const QString& text)
{
	_model->setFontFile(text.toUtf8().constData());
}

void FictionViewerDialog::on_voiceFileEdit_textChanged(const QString& text)
{
	_model->setVoiceFile(text.toUtf8().constData());
}

void FictionViewerDialog::on_musicComboBox_currentIndexChanged(int index)
{
	_model->setFictionMusic(ui->musicComboBox->itemData(index).value<int>());
}

} // namespace fso::fred::dialogs
