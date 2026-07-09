#include "CommandBriefingDialog.h"
#include "ui_CommandBriefingDialog.h"
#include "ui/Theme.h"
#include "mission/util.h"
#include <globalincs/globals.h>
#include <globalincs/linklist.h>
#include <mission/commands/FredCommands.h>
#include <ui/util/default_dir.h>
#include <ui/util/DialogUndo.h>
#include <ui/util/SignalBlockers.h>
#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>

namespace fso::fred::dialogs {

CommandBriefingDialog::CommandBriefingDialog(FredView* parent, EditorViewport* viewport)
: QDialog(parent), ui(new Ui::CommandBriefingDialog()), _model(new CommandBriefingDialogModel(this, viewport)),
_viewport(viewport), _fredView(parent)
{
	this->setFocus();
	ui->setupUi(this);

	_dialogStack = new QUndoStack(this);
	_fredView->undoGroup()->addStack(_dialogStack);
	util::setupDialogUndo(this, _fredView->undoGroup(), _dialogStack, tr("Command Briefing"));

	ui->speechFilename->setMaxLength(NAME_LENGTH - 1);
	ui->animationFilename->setMaxLength(NAME_LENGTH - 1);
	ui->actionHighResolutionFilenameEdit->setMaxLength(NAME_LENGTH - 1);
	ui->actionLowResolutionFilenameEdit->setMaxLength(NAME_LENGTH - 1);

	initializeUi();
	updateUi();

	resize(QDialog::sizeHint());
}

CommandBriefingDialog::~CommandBriefingDialog() = default;

void CommandBriefingDialog::accept()
{
	QByteArray stateBefore = _model->captureState();
	if (_model->apply()) {
		QByteArray stateAfter = _model->captureState();
		_model->setParent(nullptr);
		_fredView->mainUndoStack()->push(
			new ApplyDialogCommand(std::move(_model), stateBefore, stateAfter,
			                       _viewport->editor, tr("Edit Command Briefing")));
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::accept();
	}
}

void CommandBriefingDialog::reject()
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

void CommandBriefingDialog::closeEvent(QCloseEvent* e)
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

void CommandBriefingDialog::pushWorkingStateSnapshot(const QByteArray& before, const QString& label)
{
	const QByteArray after = _model->captureWorkingState();
	if (after == before)
		return;

	_dialogStack->push(new DialogSnapshotCommand(before, after,
		[this](const QByteArray& blob) {
			_model->restoreWorkingState(blob);
			updateUi();
		},
		label));
}

void CommandBriefingDialog::initializeUi()
{
	fso::fred::bindStandardIcon(ui->actionTestSpeechFileButton, QStyle::SP_MediaPlay);
	auto list = _model->getTeamList();

	ui->actionChangeTeams->clear();

	for (const auto& team : list) {
		ui->actionChangeTeams->addItem(QString::fromStdString(team.first), team.second);
	}
}

void CommandBriefingDialog::updateUi()
{

	util::SignalBlockers blockers(this);

	ui->actionChangeTeams->setCurrentIndex(ui->actionChangeTeams->findData(_model->getCurrentTeam()));

	ui->actionBriefingTextEditor->setPlainText(_model->getBriefingText().c_str());
	ui->animationFilename->setText(_model->getAnimationFilename().c_str());
	ui->speechFilename->setText(_model->getSpeechFilename().c_str());
	ui->actionLowResolutionFilenameEdit->setText(_model->getLowResolutionFilename().c_str());
	ui->actionHighResolutionFilenameEdit->setText(_model->getHighResolutionFilename().c_str());

	SCP_string stages = "No Stages";
	int total = _model->getTotalStages();
	int current = _model->getCurrentStage() + 1; // internal is 0 based, ui is 1 based
	if (total > 0) {
		stages = "Stage ";
		stages += std::to_string(current);
		stages += " of ";
		stages += std::to_string(total);
	}
	ui->currentStageLabel->setText(stages.c_str());

	enableDisableControls();
}

void CommandBriefingDialog::enableDisableControls()
{
	int total_stages = _model->getTotalStages();
	int current = _model->getCurrentStage();
	
	ui->actionPrevStage->setEnabled(total_stages > 0 && current > 0);
	ui->actionNextStage->setEnabled(total_stages > 0 && current < total_stages - 1);
	ui->actionAddStage->setEnabled(total_stages < CMD_BRIEF_STAGES_MAX);
	ui->actionInsertStage->setEnabled(total_stages < CMD_BRIEF_STAGES_MAX);
	ui->actionDeleteStage->setEnabled(total_stages > 0);

	ui->actionChangeTeams->setEnabled(_model->getMissionIsMultiTeam());
	ui->actionCopyToOtherTeams->setEnabled(_model->getMissionIsMultiTeam());

	ui->animationFilename->setEnabled(total_stages > 0);
	ui->actionBrowseAnimation->setEnabled(total_stages > 0);
	ui->speechFilename->setEnabled(total_stages > 0);
	ui->actionBrowseSpeechFile->setEnabled(total_stages > 0);
	ui->actionTestSpeechFileButton->setEnabled(total_stages > 0 && !_model->getSpeechFilename().empty());

	ui->actionLowResolutionFilenameEdit->setEnabled(total_stages > 0);
	ui->actionLowResolutionBrowse->setEnabled(total_stages > 0);
	ui->actionHighResolutionFilenameEdit->setEnabled(total_stages > 0);
	ui->actionHighResolutionBrowse->setEnabled(total_stages > 0);
}

void CommandBriefingDialog::on_okAndCancelButtons_accepted()
{
	accept();
}

void CommandBriefingDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

void CommandBriefingDialog::on_actionPrevStage_clicked() 
{ 
	_model->gotoPreviousStage();
	updateUi();
}

void CommandBriefingDialog::on_actionNextStage_clicked() 
{ 
	_model->gotoNextStage();
	updateUi();
}

void CommandBriefingDialog::on_actionAddStage_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	_model->addStage();
	pushWorkingStateSnapshot(before, tr("Add Stage"));
	updateUi();
}

void CommandBriefingDialog::on_actionInsertStage_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	_model->insertStage();
	pushWorkingStateSnapshot(before, tr("Insert Stage"));
	updateUi();
}

void CommandBriefingDialog::on_actionDeleteStage_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	_model->deleteStage();
	pushWorkingStateSnapshot(before, tr("Delete Stage"));
	updateUi();
}

void CommandBriefingDialog::on_actionCopyToOtherTeams_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	_model->copyToOtherTeams();
	pushWorkingStateSnapshot(before, tr("Copy To Other Teams"));
}

void CommandBriefingDialog::on_actionBrowseAnimation_clicked()
{
	QString filename;

	if (browseFile(&filename, "commandBriefing/animation", util::fredDefaultDir(CF_TYPE_INTERFACE), "FSO Animations (*.ani *.eff *.png);;All Files (*.*)")) {
		changeAnimationFilename(filename.toUtf8().constData());
	}
	updateUi();
}

void CommandBriefingDialog::on_actionBrowseSpeechFile_clicked()
{
	QString filename;

	if (browseFile(&filename, "commandBriefing/speechFile", util::fredDefaultDir(CF_TYPE_VOICE), "Voice Files (*.ogg *.wav);;All Files (*.*)")) {
		changeSpeechFilename(filename.toUtf8().constData());
	}
	updateUi();
}

void CommandBriefingDialog::on_actionTestSpeechFileButton_clicked()
{
	_model->testSpeech();
}

void CommandBriefingDialog::on_actionLowResolutionBrowse_clicked()
{
	QString filename;

	if (browseFile(&filename, "commandBriefing/lowRes", util::fredDefaultDir(CF_TYPE_INTERFACE), "FSO Animations (*.ani *.eff *.png);;All Files (*.*)")) {
		// Update the line edit unblocked; the resulting textChanged applies
		// the value to the model and pushes the undo entry.
		ui->actionLowResolutionFilenameEdit->setText(filename);
	}
	updateUi();
}

void CommandBriefingDialog::on_actionHighResolutionBrowse_clicked()
{
	QString filename;

	if (browseFile(&filename, "commandBriefing/highRes", util::fredDefaultDir(CF_TYPE_INTERFACE), "FSO Animations (*.ani *.eff *.png);;All Files (*.*)")) {
		ui->actionHighResolutionFilenameEdit->setText(filename);
	}
	updateUi();
}

void CommandBriefingDialog::on_actionChangeTeams_currentIndexChanged(int index)
{
	const int before = _model->getCurrentTeam();
	const int after  = ui->actionChangeTeams->itemData(index).toInt();
	if (before == after)
		return;

	_model->setCurrentTeam(after);
	updateUi();

	auto* cmd = new FieldEditCommand<int>(FieldId::CmdBrief_CurrentTeam, nullptr, tr("Change Team"), true);
	cmd->addEntry(before, after, [this](const int& v) {
		_model->setCurrentTeam(v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void CommandBriefingDialog::on_actionBriefingTextEditor_textChanged()
{
	const int team  = _model->getCurrentTeam();
	const int stage = _model->getCurrentStage();
	const SCP_string before = _model->getBriefingText();
	const SCP_string after  = ui->actionBriefingTextEditor->document()->toPlainText().toUtf8().constData();
	if (before == after)
		return;

	_model->setBriefingText(after);

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::CmdBrief_StageText + team * CMD_BRIEF_STAGES_MAX + stage,
	    nullptr, tr("Change Briefing Text"), true);
	cmd->addEntry(before, after, [this, team, stage](const SCP_string& v) {
		_model->setBriefingTextAt(team, stage, v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

// Shared by typing and the browse button: applies the animation filename and
// pushes one merging command per stage.
void CommandBriefingDialog::changeAnimationFilename(const SCP_string& newFilename)
{
	const int team  = _model->getCurrentTeam();
	const int stage = _model->getCurrentStage();
	const SCP_string before = _model->getAnimationFilename();
	if (before == newFilename)
		return;

	_model->setAnimationFilename(newFilename);

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::CmdBrief_AniFile + team * CMD_BRIEF_STAGES_MAX + stage,
	    nullptr, tr("Change Animation File"), true);
	cmd->addEntry(before, newFilename, [this, team, stage](const SCP_string& v) {
		_model->setAnimationFilenameAt(team, stage, v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void CommandBriefingDialog::changeSpeechFilename(const SCP_string& newFilename)
{
	const int team  = _model->getCurrentTeam();
	const int stage = _model->getCurrentStage();
	const SCP_string before = _model->getSpeechFilename();
	if (before == newFilename)
		return;

	_model->setSpeechFilename(newFilename);
	enableDisableControls(); // the speech test button follows the filename

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::CmdBrief_WaveFile + team * CMD_BRIEF_STAGES_MAX + stage,
	    nullptr, tr("Change Speech File"), true);
	cmd->addEntry(before, newFilename, [this, team, stage](const SCP_string& v) {
		_model->setSpeechFilenameAt(team, stage, v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void CommandBriefingDialog::on_animationFilename_textChanged(const QString& string)
{
	changeAnimationFilename(string.toUtf8().constData());
}

void CommandBriefingDialog::on_speechFilename_textChanged(const QString& string)
{
	changeSpeechFilename(string.toUtf8().constData());
}

void CommandBriefingDialog::on_actionLowResolutionFilenameEdit_textChanged(const QString& string)
{
	const int team = _model->getCurrentTeam();
	const SCP_string before = _model->getLowResolutionFilename();
	const SCP_string after  = string.toStdString();
	if (before == after)
		return;

	_model->setLowResolutionFilename(after);

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::CmdBrief_LowResBg + team, nullptr, tr("Change Low Resolution Background"), true);
	cmd->addEntry(before, after, [this, team](const SCP_string& v) {
		_model->setLowResolutionFilenameAt(team, v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void CommandBriefingDialog::on_actionHighResolutionFilenameEdit_textChanged(const QString& string)
{
	const int team = _model->getCurrentTeam();
	const SCP_string before = _model->getHighResolutionFilename();
	const SCP_string after  = string.toStdString();
	if (before == after)
		return;

	_model->setHighResolutionFilename(after);

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::CmdBrief_HighResBg + team, nullptr, tr("Change High Resolution Background"), true);
	cmd->addEntry(before, after, [this, team](const SCP_string& v) {
		_model->setHighResolutionFilenameAt(team, v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

// string in returns the file name, and the function returns true for success or false for fail.
bool CommandBriefingDialog::browseFile(QString* stringIn, const QString& settingsKey, const QString& defaultDir, const QString& filter)
{
	const QString lastDir = util::getLastDir(settingsKey, defaultDir);

	const QFileInfo fileInfo(QFileDialog::getOpenFileName(this, QString(), lastDir, filter));
	*stringIn = fileInfo.fileName();

	if (stringIn->length() >= CF_MAX_FILENAME_LENGTH) {
		ReleaseWarning(LOCATION, "No filename in FSO can be %d characters or longer.", CF_MAX_FILENAME_LENGTH);
		return false;
	} else if (stringIn->isEmpty()) {
		return false;
	}

	util::saveLastDir(settingsKey, fileInfo.absoluteFilePath());
	return true;
}

} // namespace fso::fred::dialogs
