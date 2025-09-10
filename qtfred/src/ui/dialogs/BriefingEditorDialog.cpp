#include "BriefingEditorDialog.h"
#include "ui_BriefingEditorDialog.h"

#include "mission/util.h"

#include <globalincs/linklist.h>
#include <ui/util/SignalBlockers.h>

#include <QCloseEvent>
#include <QFileDialog>

namespace fso::fred::dialogs {

BriefingEditorDialog::BriefingEditorDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), SexpTreeEditorInterface(flagset<TreeFlags>()), ui(new Ui::BriefingEditorDialog()),
	  _model(new BriefingEditorDialogModel(this, viewport)), _viewport(viewport)
{
	this->setFocus();
	ui->setupUi(this);

	initializeUi();
	updateUi();

	resize(QDialog::sizeHint());
}

BriefingEditorDialog::~BriefingEditorDialog() = default;

void BriefingEditorDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void BriefingEditorDialog::reject()
{
	// Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void BriefingEditorDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
}

void BriefingEditorDialog::initializeUi()
{
	util::SignalBlockers blockers(this);

	auto list = _model->getTeamList();

	ui->teamComboBox->clear();

	for (const auto& team : list) {
		ui->teamComboBox->addItem(QString::fromStdString(team.first), team.second);
	}

	// Initialize the formula tree editor
	ui->formulaTreeView->initializeEditor(_viewport->editor, this);
}

void BriefingEditorDialog::updateUi()
{

	util::SignalBlockers blockers(this);

	ui->teamComboBox->setCurrentIndex(ui->teamComboBox->findData(_model->getCurrentTeam()));

	ui->stageTextPlainTextEdit->setPlainText(QString::fromStdString(_model->getStageText()));
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

	enableDisableControls();
}

void BriefingEditorDialog::enableDisableControls()
{
	int total_stages = _model->getTotalStages();
	int current = _model->getCurrentStage();
	const bool stage_exists = total_stages > 0 && current >= 0;

	ui->prevStageButton->setEnabled(stage_exists && current > 0);
	ui->nextStageButton->setEnabled(stage_exists && current < total_stages - 1);
	ui->addStageButton->setEnabled(total_stages < MAX_DEBRIEF_STAGES);
	ui->insertStageButton->setEnabled(stage_exists && total_stages < MAX_DEBRIEF_STAGES);
	ui->deleteStageButton->setEnabled(stage_exists);

	ui->teamComboBox->setEnabled(_model->getMissionIsMultiTeam());
	ui->copyToOtherTeamsButton->setEnabled(_model->getMissionIsMultiTeam());

	ui->stageTextPlainTextEdit->setEnabled(stage_exists);
	ui->voiceFileLineEdit->setEnabled(stage_exists);
	ui->voiceFileBrowseButton->setEnabled(stage_exists);
	ui->voiceFilePlayButton->setEnabled(stage_exists && !_model->getSpeechFilename().empty());
	ui->formulaTreeView->setEnabled(stage_exists);
}

void BriefingEditorDialog::on_okAndCancelButtons_accepted()
{
	accept();
}

void BriefingEditorDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

void BriefingEditorDialog::on_prevStageButton_clicked()
{
	_model->gotoPreviousStage();
	updateUi();
}

void BriefingEditorDialog::on_nextStageButton_clicked()
{
	_model->gotoNextStage();
	updateUi();
}

void BriefingEditorDialog::on_addStageButton_clicked()
{
	_model->addStage();
	updateUi();
}

void BriefingEditorDialog::on_insertStageButton_clicked()
{
	_model->insertStage();
	updateUi();
}

void BriefingEditorDialog::on_deleteStageButton_clicked()
{
	_model->deleteStage();
	updateUi();
}

void BriefingEditorDialog::on_copyToOtherTeamsButton_clicked()
{
	_model->copyToOtherTeams();
}

void BriefingEditorDialog::on_teamComboBox_currentIndexChanged(int index)
{
	_model->setCurrentTeam(ui->teamComboBox->itemData(index).toInt());
	updateUi();
}

void BriefingEditorDialog::on_stageTextPlainTextEdit_textChanged()
{
	_model->setStageText(ui->stageTextPlainTextEdit->toPlainText().toUtf8().constData());
}

void BriefingEditorDialog::on_voiceFileLineEdit_textChanged(const QString& string)
{
	_model->setSpeechFilename(string.toUtf8().constData());
}

void BriefingEditorDialog::on_voiceFileBrowseButton_clicked()
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

void BriefingEditorDialog::on_voiceFilePlayButton_clicked()
{
	_model->testSpeech();
}

void BriefingEditorDialog::on_formulaTreeView_nodeChanged(int newTree)
{
	_model->setFormula(newTree);
}

} // namespace fso::fred::dialogs