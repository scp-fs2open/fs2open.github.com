#include "DebriefingDialog.h"
#include "ui_DebriefingDialog.h"
#include "mission/util.h"
#include <globalincs/linklist.h>
#include <ui/util/SignalBlockers.h>
#include <QCloseEvent>
#include <QFileDialog>

namespace fso::fred::dialogs {

DebriefingDialog::DebriefingDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), SexpTreeEditorInterface(flagset<TreeFlags>()), 
	  ui(new Ui::DebriefingDialog()), _model(new DebriefingDialogModel(this, viewport)), _viewport(viewport)
{
	this->setFocus();
	ui->setupUi(this);

	initializeUi();
	updateUi();

	resize(QDialog::sizeHint());

}

DebriefingDialog::~DebriefingDialog() = default;

void DebriefingDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void DebriefingDialog::reject()
{
	// Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void DebriefingDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
}

void DebriefingDialog::initializeUi()
{
	util::SignalBlockers blockers(this);
	
	auto list = _model->getTeamList();

	ui->actionChangeTeams->clear();

	for (const auto& team : list) {
		ui->actionChangeTeams->addItem(QString::fromStdString(team.first), team.second);
	}

	auto musicList = _model->getMusicList();
	QStringList qMusicList;
	for (const auto& track : musicList) {
		qMusicList << QString::fromStdString(track);
	}
	ui->successMusicComboBox->clear();
	ui->successMusicComboBox->addItems(qMusicList);
	ui->averageMusicComboBox->clear();
	ui->averageMusicComboBox->addItems(qMusicList);
	ui->failureMusicComboBox->clear();
	ui->failureMusicComboBox->addItems(qMusicList);

	// Initialize the formula tree editor
	ui->formulaTreeView->initializeEditor(_viewport->editor, this);
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
	if (ui->formulaTreeView->select_sexp_node != -1) {
		ui->formulaTreeView->hilite_item(ui->formulaTreeView->select_sexp_node);
	}

	// Music tracks (data is index, UI is index + 1 to account for "None")
	ui->successMusicComboBox->setCurrentIndex(_model->getSuccessMusicTrack() + 1);
	ui->averageMusicComboBox->setCurrentIndex(_model->getAverageMusicTrack() + 1);
	ui->failureMusicComboBox->setCurrentIndex(_model->getFailureMusicTrack() + 1);

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
	_model->addStage();
	updateUi();
}

void DebriefingDialog::on_actionInsertStage_clicked() 
{ 
	_model->insertStage();
	updateUi();
}

void DebriefingDialog::on_actionDeleteStage_clicked() 
{ 
	_model->deleteStage();
	updateUi();
}

void DebriefingDialog::on_actionCopyToOtherTeams_clicked()
{
	_model->copyToOtherTeams();
}

void DebriefingDialog::on_actionChangeTeams_currentIndexChanged(int index)
{
	_model->setCurrentTeam(ui->actionChangeTeams->itemData(index).toInt());
	updateUi();
}

void DebriefingDialog::on_debriefingTextEdit_textChanged()
{
	_model->setStageText(ui->debriefingTextEdit->toPlainText().toUtf8().constData());
}

void DebriefingDialog::on_recommendationTextEdit_textChanged()
{
	_model->setRecommendationText(ui->recommendationTextEdit->toPlainText().toUtf8().constData());
}

void DebriefingDialog::on_voiceFileLineEdit_textChanged(const QString& string)
{
	_model->setSpeechFilename(string.toUtf8().constData());
}

void DebriefingDialog::on_voiceFileBrowseButton_clicked()
{
	int dir_pushed = cfile_push_chdir(CF_TYPE_VOICE_DEBRIEFINGS);

	QFileDialog dlg(this, "Select Voice File", "", "Voice Files (*.ogg *.wav)");
	if (dlg.exec() == QDialog::Accepted) {
		QStringList files = dlg.selectedFiles();
		if (!files.isEmpty()) {
			QFileInfo fileInfo(files.first());
			_model->setSpeechFilename(fileInfo.fileName().toUtf8().constData());
			updateUi();
		}
	}

	if (dir_pushed) {
		cfile_pop_dir();
	}
}
	
void DebriefingDialog::on_voiceFilePlayButton_clicked()
{
	_model->testSpeech();
}

void DebriefingDialog::on_formulaTreeView_nodeChanged(int newTree)
{
	_model->setFormula(newTree);
}

void DebriefingDialog::on_successMusicComboBox_currentIndexChanged(int index)
{
	_model->setSuccessMusicTrack(index - 1); // -1 to account for "None"
}

void DebriefingDialog::on_averageMusicComboBox_currentIndexChanged(int index)
{
	_model->setAverageMusicTrack(index - 1); // -1 to account for "None"
}

void DebriefingDialog::on_failureMusicComboBox_currentIndexChanged(int index)
{
	_model->setFailureMusicTrack(index - 1); // -1 to account for "None"
}

} // namespace fso::fred::dialogs