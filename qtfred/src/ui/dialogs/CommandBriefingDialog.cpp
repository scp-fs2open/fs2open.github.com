#include "CommandBriefingDialog.h"
#include "ui_CommandBriefingDialog.h"
#include "mission/util.h"
#include <globalincs/linklist.h>
#include <ui/util/SignalBlockers.h>
#include <QCloseEvent>
#include <QFileDialog>

namespace fso::fred::dialogs {

CommandBriefingDialog::CommandBriefingDialog(FredView* parent, EditorViewport* viewport)
: QDialog(parent), ui(new Ui::CommandBriefingDialog()), _model(new CommandBriefingDialogModel(this, viewport)),
_viewport(viewport)
{
	this->setFocus();
	ui->setupUi(this);

	initializeUi();
	updateUi();

	resize(QDialog::sizeHint());

}

CommandBriefingDialog::~CommandBriefingDialog() = default;

void CommandBriefingDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void CommandBriefingDialog::reject()
{
	// Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void CommandBriefingDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
}

void CommandBriefingDialog::initializeUi()
{
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
	ui->animationFileName->setText(_model->getAnimationFilename().c_str());
	ui->speechFileName->setText(_model->getSpeechFilename().c_str());
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

	ui->animationFileName->setEnabled(total_stages > 0);
	ui->actionBrowseAnimation->setEnabled(total_stages > 0);
	ui->speechFileName->setEnabled(total_stages > 0);
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
	_model->addStage();
	updateUi();
}

void CommandBriefingDialog::on_actionInsertStage_clicked() 
{ 
	_model->insertStage();
	updateUi();
}

void CommandBriefingDialog::on_actionDeleteStage_clicked() 
{ 
	_model->deleteStage();
	updateUi();
}

void CommandBriefingDialog::on_actionCopyToOtherTeams_clicked()
{
	_model->copyToOtherTeams();
}

void CommandBriefingDialog::on_actionBrowseAnimation_clicked()
{
	QString filename;

	if (CommandBriefingDialog::browseFile(&filename)) {
		_model->setAnimationFilename(filename.toUtf8().constData());
	}
	updateUi();
}

void CommandBriefingDialog::on_actionBrowseSpeechFile_clicked()
{
	QString filename;

	if (CommandBriefingDialog::browseFile(&filename)) {
		_model->setSpeechFilename(filename.toUtf8().constData());
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

	if (CommandBriefingDialog::browseFile(&filename)) {
		_model->setLowResolutionFilename(filename.toUtf8().constData());
	}
	updateUi();
}

void CommandBriefingDialog::on_actionHighResolutionBrowse_clicked()
{
	QString filename;

	if (CommandBriefingDialog::browseFile(&filename)) {
		_model->setHighResolutionFilename(filename.toUtf8().constData());
	}
	updateUi();
}

void CommandBriefingDialog::on_actionChangeTeams_currentIndexChanged(int index)
{
	_model->setCurrentTeam(ui->actionChangeTeams->itemData(index).toInt());
	updateUi();
}

void CommandBriefingDialog::on_actionBriefingTextEditor_textChanged()
{
	_model->setBriefingText(ui->actionBriefingTextEditor->document()->toPlainText().toUtf8().constData());
}

void CommandBriefingDialog::on_animationFilename_textChanged(const QString& string)
{
	_model->setAnimationFilename(string.toUtf8().constData());
}

void CommandBriefingDialog::on_speechFilename_textChanged(const QString& string)
{
	_model->setSpeechFilename(string.toUtf8().constData());
}

void CommandBriefingDialog::on_actionLowResolutionFilenameEdit_textChanged(const QString& string)
{
	_model->setLowResolutionFilename(string.toStdString());
}

void CommandBriefingDialog::on_actionHighResolutionFilenameEdit_textChanged(const QString& string)
{
	_model->setHighResolutionFilename(string.toStdString());
}

// string in returns the file name, and the function returns true for success or false for fail.
bool CommandBriefingDialog::browseFile(QString* stringIn) 
{
	QFileInfo fileInfo(QFileDialog::getOpenFileName());
	*stringIn = fileInfo.fileName();

	if (stringIn->length() >= CF_MAX_FILENAME_LENGTH) {
		ReleaseWarning(LOCATION, "No filename in FSO can be %d characters or longer.", CF_MAX_FILENAME_LENGTH);
		return false;
	} else if (stringIn->isEmpty()) {
		return false;
	}

	return true;
}

} // namespace fso::fred::dialogs