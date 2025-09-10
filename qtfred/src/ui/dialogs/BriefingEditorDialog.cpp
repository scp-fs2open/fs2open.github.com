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

	auto icons = _model->getIconList();
	for (const auto& icon : icons) {
		ui->iconImageComboBox->addItem(icon.second.c_str(), icon.first);
	}

	auto ships = _model->getShipList();
	for (const auto& ship : ships) {
		ui->iconShipTypeComboBox->addItem(ship.second.c_str(), ship.first);
	}

	auto iffs = _model->getIffList();
	for (const auto& iff : iffs) {
		ui->iconTeamComboBox->addItem(iff.second.c_str(), iff.first);
	}

	auto musicList = _model->getMusicList();
	for (const auto& m : musicList) {
		ui->defaultMusicComboBox->addItem(m.c_str());
		ui->musicPackComboBox->addItem(m.c_str());
	}

	// Initialize the formula tree editor
	ui->formulaTreeView->initializeEditor(_viewport->editor, this);
}

void BriefingEditorDialog::updateUi()
{

	util::SignalBlockers blockers(this);

	ui->teamComboBox->setCurrentIndex(ui->teamComboBox->findData(_model->getCurrentTeam()));

	ui->cameraTransitionTimeSpinBox->setValue(_model->getCameraTransitionTime());
	ui->cutToNextStageCheckBox->setChecked(_model->getCutToNext());
	ui->cutToPrevStageCheckBox->setChecked(_model->getCutFromPrev());
	ui->disableGridCheckBox->setChecked(_model->getDisableGrid());

	ui->stageTextPlainTextEdit->setPlainText(QString::fromStdString(_model->getStageText()));
	ui->voiceFileLineEdit->setText(QString::fromStdString(_model->getSpeechFilename()));
	ui->defaultMusicComboBox->setCurrentIndex(_model->getBriefingMusicIndex());
	ui->musicPackComboBox->setCurrentIndex(ui->musicPackComboBox->findText(_model->getSubstituteBriefingMusicName().c_str()));

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
	ui->addStageButton->setEnabled(total_stages < MAX_BRIEF_STAGES);
	ui->insertStageButton->setEnabled(stage_exists && total_stages < MAX_BRIEF_STAGES);
	ui->deleteStageButton->setEnabled(stage_exists);
	ui->saveViewButton->setEnabled(stage_exists);
	ui->gotoViewButton->setEnabled(stage_exists);

	ui->teamComboBox->setEnabled(_model->getMissionIsMultiTeam());
	ui->copyToOtherTeamsButton->setEnabled(_model->getMissionIsMultiTeam());

	ui->stageTextPlainTextEdit->setEnabled(stage_exists);
	ui->voiceFileLineEdit->setEnabled(stage_exists);
	ui->voiceFileBrowseButton->setEnabled(stage_exists);
	ui->voiceFilePlayButton->setEnabled(stage_exists && !_model->getSpeechFilename().empty());
	ui->formulaTreeView->setEnabled(stage_exists);
	
	const bool icon_selected = stage_exists && _model->getCurrentIconIndex() >= 0;
	ui->currentIconInfoGroupBox->setEnabled(icon_selected);

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

void BriefingEditorDialog::on_saveViewButton_clicked()
{
	//_model->saveStageView(pos, orient); // Get them from the renderer when the renderer exists
}

void BriefingEditorDialog::on_gotoViewButton_clicked()
{
	//const auto& view = _model->getStageView();
	// Tell the renderer to go to this view
}

void BriefingEditorDialog::on_copyViewButton_clicked()
{
	_model->copyStageViewToClipboard();
}

void BriefingEditorDialog::on_pasteViewButton_clicked()
{
	_model->pasteClipboardViewToStage();
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

void BriefingEditorDialog::on_cameraTransitionTimeSpinBox_valueChanged(int arg1)
{
	_model->setCameraTransitionTime(arg1);
}

void BriefingEditorDialog::on_cutToNextStageCheckBox_toggled(bool checked)
{
	_model->setCutToNext(checked);
}

void BriefingEditorDialog::on_cutToPrevStageCheckBox_toggled(bool checked)
{
	_model->setCutFromPrev(checked);
}

void BriefingEditorDialog::on_disableGridCheckBox_toggled(bool checked)
{
	_model->setDisableGrid(checked);
}

void BriefingEditorDialog::on_iconIdSpinBox_valueChanged(int arg1)
{
	_model->setIconId(arg1);
}

void BriefingEditorDialog::on_iconLabelLineEdit_textChanged(const QString& string)
{
	_model->setIconLabel(string.toUtf8().constData());
}

void BriefingEditorDialog::on_iconCloseupLabelLineEdit_textChanged(const QString& string)
{
	_model->setIconCloseupLabel(string.toUtf8().constData());
}

void BriefingEditorDialog::on_iconImageComboBox_currentIndexChanged(int index)
{
	_model->setIconTypeIndex(index);
}

void BriefingEditorDialog::on_iconShipTypeComboBox_currentIndexChanged(int index)
{
	_model->setIconShipTypeIndex(index);
}

void BriefingEditorDialog::on_iconTeamComboBox_currentIndexChanged(int index)
{
	_model->setIconTeamIndex(index);
}

void BriefingEditorDialog::on_iconScaleDoubleSpinBox_valueChanged(double arg1)
{
	_model->setIconScaleFactor(static_cast<float>(arg1));
}

void BriefingEditorDialog::on_drawLinesCheckBox_toggled(bool checked)
{
	_model->applyDrawLines(checked); // Assumes multi selection has already been set by the renderer
}

void BriefingEditorDialog::on_changeLocallyCheckBox_toggled(bool checked)
{
	_model->setChangeLocally(checked);
}

void BriefingEditorDialog::on_flipIconCheckBox_toggled(bool checked)
{
	_model->setIconFlipped(checked);
}

void BriefingEditorDialog::on_highlightCheckBox_toggled(bool checked)
{
	_model->setIconHighlighted(checked);
}

void BriefingEditorDialog::on_useWingCheckBox_toggled(bool checked)
{
	_model->setIconUseWing(checked);
}

void BriefingEditorDialog::on_useCargoCheckBox_toggled(bool checked)
{
	_model->setIconUseCargo(checked);
}

void BriefingEditorDialog::on_makeIconButton_clicked()
{
	//_model->makeIcon("New Icon", 0, 0, 0); // Get data from the ui when the ui exists
}

void BriefingEditorDialog::on_deleteIconButton_clicked()
{
	_model->deleteCurrentIcon();
}

void BriefingEditorDialog::on_propagateIconButton_clicked()
{
	_model->propagateCurrentIconForward();
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

void BriefingEditorDialog::on_defaultMusicComboBox_currentIndexChanged(int index)
{
	_model->setBriefingMusicIndex(index);
}

void BriefingEditorDialog::on_musicPackComboBox_currentIndexChanged(int /*index*/)
{
	_model->setSubstituteBriefingMusicName(ui->musicPackComboBox->currentText().toUtf8().constData());
}

} // namespace fso::fred::dialogs