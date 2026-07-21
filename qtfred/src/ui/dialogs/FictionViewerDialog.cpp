
#include <QCloseEvent>
#include "ui/dialogs/FictionViewerDialog.h"
#include "ui/util/DialogUndo.h"
#include "ui/util/SignalBlockers.h"
#include "ui_FictionViewerDialog.h"
#include "mission/util.h"
#include <mission/commands/FredCommands.h>

namespace fso::fred::dialogs {

FictionViewerDialog::FictionViewerDialog(FredView* parent, EditorViewport* viewport) :
	QDialog(parent), _viewport(viewport), ui(new Ui::FictionViewerDialog()),
	_model(new FictionViewerDialogModel(this, viewport)), _fredView(parent)
{
	ui->setupUi(this);

	_dialogStack = new QUndoStack(this);
	_fredView->undoGroup()->addStack(_dialogStack);
	util::setupDialogUndo(this, _fredView->undoGroup(), _dialogStack, tr("Fiction Viewer"));

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
	QByteArray stateBefore = _model->captureState();
	if (_model->apply()) {
		ui->musicWidget->stopPlayback();
		QByteArray stateAfter = _model->captureState();
		_model->setParent(nullptr);
		_fredView->mainUndoStack()->push(
			new ApplyDialogCommand(std::move(_model), stateBefore, stateAfter,
			                       _viewport->editor, tr("Edit Fiction Viewer")));
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::accept();
	}
}

void FictionViewerDialog::reject()
{
	if (!_model) {
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::reject();
		return;
	}
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		ui->musicWidget->stopPlayback();
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::reject();
	}
}

void FictionViewerDialog::closeEvent(QCloseEvent* e)
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


void FictionViewerDialog::initializeUi()
{
	ui->storyFileEdit->setMaxLength(_model->getMaxStoryFileLength());
	ui->fontFileEdit->setMaxLength(_model->getMaxFontFileLength());
	ui->voiceFileEdit->setMaxLength(_model->getMaxVoiceFileLength());

}

void FictionViewerDialog::updateUi()
{
	util::SignalBlockers blockers(this);

	ui->storyFileEdit->setText(QString::fromStdString(_model->getStoryFile()));
	ui->fontFileEdit->setText(QString::fromStdString(_model->getFontFile()));
	ui->voiceFileEdit->setText(QString::fromStdString(_model->getVoiceFile()));
	ui->musicWidget->setCurrentMusicIndex(_model->getFictionMusic());
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
	const SCP_string before = _model->getStoryFile();
	_model->setStoryFile(text.toUtf8().constData());
	const SCP_string after = _model->getStoryFile();
	if (before == after)
		return;

	auto* cmd = new FieldEditCommand<SCP_string>(FieldId::FV_StoryFile, nullptr, tr("Change Story File"), true);
	cmd->addEntry(before, after, [this](const SCP_string& v) {
		_model->setStoryFile(v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void FictionViewerDialog::on_fontFileEdit_textChanged(const QString& text)
{
	const SCP_string before = _model->getFontFile();
	_model->setFontFile(text.toUtf8().constData());
	const SCP_string after = _model->getFontFile();
	if (before == after)
		return;

	auto* cmd = new FieldEditCommand<SCP_string>(FieldId::FV_FontFile, nullptr, tr("Change Font File"), true);
	cmd->addEntry(before, after, [this](const SCP_string& v) {
		_model->setFontFile(v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void FictionViewerDialog::on_voiceFileEdit_textChanged(const QString& text)
{
	const SCP_string before = _model->getVoiceFile();
	_model->setVoiceFile(text.toUtf8().constData());
	const SCP_string after = _model->getVoiceFile();
	if (before == after)
		return;

	auto* cmd = new FieldEditCommand<SCP_string>(FieldId::FV_VoiceFile, nullptr, tr("Change Voice File"), true);
	cmd->addEntry(before, after, [this](const SCP_string& v) {
		_model->setVoiceFile(v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void FictionViewerDialog::on_musicWidget_currentIndexChanged(int spooledMusicIdx)
{
	const int before = _model->getFictionMusic();
	if (before == spooledMusicIdx)
		return;

	_model->setFictionMusic(spooledMusicIdx);

	auto* cmd = new FieldEditCommand<int>(FieldId::FV_Music, nullptr, tr("Change Fiction Music"), true);
	cmd->addEntry(before, spooledMusicIdx, [this](const int& v) {
		_model->setFictionMusic(v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

} // namespace fso::fred::dialogs
