#include "DebriefingDialog.h"
#include "ui_DebriefingDialog.h"
#include "ui/Theme.h"
#include "mission/util.h"
#include <gamesnd/eventmusic.h>
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

DebriefingDialog::DebriefingDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), SexpTreeEditorInterface(flagset<TreeFlags>()),
	  ui(new Ui::DebriefingDialog()), _model(new DebriefingDialogModel(this, viewport)),
	  _viewport(viewport), _fredView(parent)
{
	this->setFocus();
	ui->setupUi(this);

	_dialogStack = new QUndoStack(this);
	_fredView->undoGroup()->addStack(_dialogStack);
	util::setupDialogUndo(this, _fredView->undoGroup(), _dialogStack, tr("Debriefing"));

	ui->voiceFileLineEdit->setMaxLength(MAX_FILENAME_LEN - 1);

	initializeUi();
	updateUi();

	resize(QDialog::sizeHint());
}

DebriefingDialog::~DebriefingDialog() = default;

void DebriefingDialog::accept()
{
	QByteArray stateBefore = _model->captureState();
	if (_model->apply()) {
		ui->successMusicWidget->stopPlayback();
		ui->averageMusicWidget->stopPlayback();
		ui->failureMusicWidget->stopPlayback();
		QByteArray stateAfter = _model->captureState();
		_model->setParent(nullptr);
		_fredView->mainUndoStack()->push(
			new ApplyDialogCommand(std::move(_model), stateBefore, stateAfter,
			                       _viewport->editor, tr("Edit Debriefing")));
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::accept();
	}
}

void DebriefingDialog::reject()
{
	if (!_model) {
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::reject();
		return;
	}
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		ui->successMusicWidget->stopPlayback();
		ui->averageMusicWidget->stopPlayback();
		ui->failureMusicWidget->stopPlayback();
		_dialogStack->clear();
		_fredView->undoGroup()->setActiveStack(_fredView->mainUndoStack());
		QDialog::reject();
	}
}

void DebriefingDialog::closeEvent(QCloseEvent* e)
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

void DebriefingDialog::initializeUi()
{
	fso::fred::bindStandardIcon(ui->voiceFilePlayButton, QStyle::SP_MediaPlay);

	util::SignalBlockers blockers(this);

	auto list = _model->getTeamList();

	ui->actionChangeTeams->clear();

	for (const auto& team : list) {
		ui->actionChangeTeams->addItem(QString::fromStdString(team.first), team.second);
	}

	// Initialize the formula tree editor
	ui->formulaTreeView->initializeEditor(_viewport->editor, this, _viewport, _fredView);
	_model->setTreeControl(ui->formulaTreeView);
	connect(ui->formulaTreeView, &sexp_tree_view::modified, this, &DebriefingDialog::onFormulaTreeModified);
}

void DebriefingDialog::onFormulaTreeModified()
{
	// Tree edits only reach the working copy via commitCurrentFormula(), so
	// at this point the WIP still holds the pre-edit formula: capture the
	// before-state first, then commit the widget's tree into the WIP.
	const QByteArray before = _model->captureWorkingState();
	_model->commitCurrentFormula();
	_model->setModified();
	pushWorkingStateSnapshot(before, tr("Edit Stage Formula"));
}

void DebriefingDialog::pushWorkingStateSnapshot(const QByteArray& before, const QString& label)
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

void DebriefingDialog::updateUi()
{

	util::SignalBlockers blockers(this);

	ui->actionChangeTeams->setCurrentIndex(ui->actionChangeTeams->findData(_model->getCurrentTeam()));

	ui->debriefingTextEdit->setPlainText(QString::fromStdString(_model->getStageText()));
	ui->recommendationTextEdit->setPlainText(QString::fromStdString(_model->getRecommendationText()));
	ui->voiceFileLineEdit->setText(QString::fromStdString(_model->getSpeechFilename()));

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

	// SEXP tree formula
	ui->formulaTreeView->load_tree(_model->getFormula());
	ui->formulaTreeView->expandAll();
	if (ui->formulaTreeView->select_sexp_node != -1) {
		ui->formulaTreeView->hilite_item(ui->formulaTreeView->select_sexp_node);
	}

	// Music tracks: model uses Spooled_music index (-1 = None), widget uses the same convention
	ui->successMusicWidget->setCurrentMusicIndex(_model->getSuccessMusicTrack());
	ui->averageMusicWidget->setCurrentMusicIndex(_model->getAverageMusicTrack());
	ui->failureMusicWidget->setCurrentMusicIndex(_model->getFailureMusicTrack());

	enableDisableControls();
}

void DebriefingDialog::enableDisableControls()
{
	int total_stages = _model->getTotalStages();
	int current = _model->getCurrentStage();
	const bool stage_exists = total_stages > 0 && current >= 0;
	
	ui->actionPrevStage->setEnabled(stage_exists && current > 0);
	ui->actionNextStage->setEnabled(stage_exists && current < total_stages - 1);
	ui->actionAddStage->setEnabled(total_stages < MAX_DEBRIEF_STAGES);
	ui->actionInsertStage->setEnabled(stage_exists && total_stages < MAX_DEBRIEF_STAGES);
	ui->actionDeleteStage->setEnabled(stage_exists);

	ui->actionChangeTeams->setEnabled(_model->getMissionIsMultiTeam());
	ui->actionCopyToOtherTeams->setEnabled(_model->getMissionIsMultiTeam());

	ui->debriefingTextEdit->setEnabled(stage_exists);
	ui->recommendationTextEdit->setEnabled(stage_exists);
	ui->voiceFileLineEdit->setEnabled(stage_exists);
	ui->voiceFileBrowseButton->setEnabled(stage_exists);
	ui->voiceFilePlayButton->setEnabled(stage_exists && !_model->getSpeechFilename().empty());
	ui->formulaTreeView->setEnabled(stage_exists);

	// Music is global to the mission, not per stage
	ui->musicLayout->setEnabled(true);
}

void DebriefingDialog::on_okAndCancelButtons_accepted()
{
	accept();
}

void DebriefingDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

void DebriefingDialog::on_actionPrevStage_clicked() 
{ 
	_model->gotoPreviousStage();
	updateUi();
}

void DebriefingDialog::on_actionNextStage_clicked() 
{ 
	_model->gotoNextStage();
	updateUi();
}

void DebriefingDialog::on_actionAddStage_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	_model->addStage();
	pushWorkingStateSnapshot(before, tr("Add Stage"));
	updateUi();
}

void DebriefingDialog::on_actionInsertStage_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	_model->insertStage();
	pushWorkingStateSnapshot(before, tr("Insert Stage"));
	updateUi();
}

void DebriefingDialog::on_actionDeleteStage_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	_model->deleteStage();
	pushWorkingStateSnapshot(before, tr("Delete Stage"));
	updateUi();
}

void DebriefingDialog::on_actionCopyToOtherTeams_clicked()
{
	const QByteArray before = _model->captureWorkingState();
	_model->copyToOtherTeams();
	pushWorkingStateSnapshot(before, tr("Copy To Other Teams"));
}

void DebriefingDialog::on_actionChangeTeams_currentIndexChanged(int index)
{
	const int before = _model->getCurrentTeam();
	const int after  = ui->actionChangeTeams->itemData(index).toInt();
	if (before == after)
		return;

	_model->setCurrentTeam(after);
	updateUi();

	auto* cmd = new FieldEditCommand<int>(FieldId::Deb_CurrentTeam, nullptr, tr("Change Team"), true);
	cmd->addEntry(before, after, [this](const int& v) {
		_model->setCurrentTeam(v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void DebriefingDialog::on_debriefingTextEdit_textChanged()
{
	const int team  = _model->getCurrentTeam();
	const int stage = _model->getCurrentStage();
	const SCP_string before = _model->getStageText();
	const SCP_string after  = ui->debriefingTextEdit->toPlainText().toUtf8().constData();
	if (before == after)
		return;

	_model->setStageText(after);

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::Deb_StageText + team * MAX_DEBRIEF_STAGES + stage,
	    nullptr, tr("Change Debriefing Text"), true);
	cmd->addEntry(before, after, [this, team, stage](const SCP_string& v) {
		_model->setStageTextAt(team, stage, v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void DebriefingDialog::on_recommendationTextEdit_textChanged()
{
	const int team  = _model->getCurrentTeam();
	const int stage = _model->getCurrentStage();
	const SCP_string before = _model->getRecommendationText();
	const SCP_string after  = ui->recommendationTextEdit->toPlainText().toUtf8().constData();
	if (before == after)
		return;

	_model->setRecommendationText(after);

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::Deb_RecommendationText + team * MAX_DEBRIEF_STAGES + stage,
	    nullptr, tr("Change Recommendation Text"), true);
	cmd->addEntry(before, after, [this, team, stage](const SCP_string& v) {
		_model->setRecommendationTextAt(team, stage, v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void DebriefingDialog::changeVoiceFilename(const SCP_string& newFilename)
{
	const int team  = _model->getCurrentTeam();
	const int stage = _model->getCurrentStage();
	const SCP_string before = _model->getSpeechFilename();
	if (before == newFilename)
		return;

	_model->setSpeechFilename(newFilename);

	auto* cmd = new FieldEditCommand<SCP_string>(
	    FieldId::Deb_VoiceFile + team * MAX_DEBRIEF_STAGES + stage,
	    nullptr, tr("Change Voice File"), true);
	cmd->addEntry(before, newFilename, [this, team, stage](const SCP_string& v) {
		_model->setSpeechFilenameAt(team, stage, v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void DebriefingDialog::on_voiceFileLineEdit_textChanged(const QString& string)
{
	changeVoiceFilename(string.toUtf8().constData());
}

void DebriefingDialog::on_voiceFileBrowseButton_clicked()
{
	const QString lastDir = util::getLastDir("debriefing/voiceFile", CF_TYPE_VOICE_DEBRIEFINGS);

	QFileDialog dlg(this, "Select Voice File", lastDir, "Voice Files (*.ogg *.wav)");
	if (dlg.exec() == QDialog::Accepted) {
		QStringList files = dlg.selectedFiles();
		if (!files.isEmpty()) {
			const QFileInfo fileInfo(files.first());
			util::saveLastDir("debriefing/voiceFile", files.first());
			changeVoiceFilename(fileInfo.fileName().toUtf8().constData());
			updateUi();
		}
	}
}
	
void DebriefingDialog::on_voiceFilePlayButton_clicked()
{
	_model->testSpeech();
}

void DebriefingDialog::on_successMusicWidget_currentIndexChanged(int spooledMusicIdx)
{
	const int before = _model->getSuccessMusicTrack();
	if (before == spooledMusicIdx)
		return;

	_model->setSuccessMusicTrack(spooledMusicIdx);

	auto* cmd = new FieldEditCommand<int>(FieldId::Deb_SuccessMusic, nullptr, tr("Change Success Music"), true);
	cmd->addEntry(before, spooledMusicIdx, [this](const int& v) {
		_model->setSuccessMusicTrack(v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void DebriefingDialog::on_averageMusicWidget_currentIndexChanged(int spooledMusicIdx)
{
	const int before = _model->getAverageMusicTrack();
	if (before == spooledMusicIdx)
		return;

	_model->setAverageMusicTrack(spooledMusicIdx);

	auto* cmd = new FieldEditCommand<int>(FieldId::Deb_AverageMusic, nullptr, tr("Change Average Music"), true);
	cmd->addEntry(before, spooledMusicIdx, [this](const int& v) {
		_model->setAverageMusicTrack(v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void DebriefingDialog::on_failureMusicWidget_currentIndexChanged(int spooledMusicIdx)
{
	const int before = _model->getFailureMusicTrack();
	if (before == spooledMusicIdx)
		return;

	_model->setFailureMusicTrack(spooledMusicIdx);

	auto* cmd = new FieldEditCommand<int>(FieldId::Deb_FailureMusic, nullptr, tr("Change Failure Music"), true);
	cmd->addEntry(before, spooledMusicIdx, [this](const int& v) {
		_model->setFailureMusicTrack(v);
		updateUi();
	});
	_dialogStack->push(cmd);
}

void DebriefingDialog::on_successMusicWidget_playbackStarted()
{
	ui->averageMusicWidget->stopPlayback();
	ui->failureMusicWidget->stopPlayback();
}

void DebriefingDialog::on_averageMusicWidget_playbackStarted()
{
	ui->successMusicWidget->stopPlayback();
	ui->failureMusicWidget->stopPlayback();
}

void DebriefingDialog::on_failureMusicWidget_playbackStarted()
{
	ui->successMusicWidget->stopPlayback();
	ui->averageMusicWidget->stopPlayback();
}

} // namespace fso::fred::dialogs