#include "CommandBriefingDialog.h"
#include "ui_CommandBriefingDialog.h"

#include <globalincs/linklist.h>
#include <ui/util/SignalBlockers.h>
#include <QCloseEvent>
#include <qfiledialog.h>

namespace fso {
namespace fred {
namespace dialogs {


	CommandBriefingDialog::CommandBriefingDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::CommandBriefingDialog()), _model(new CommandBriefingDialogModel(this, viewport)),
	_viewport(viewport)
	{
		this->setFocus();
		ui->setupUi(this);

		connect(_model.get(), &AbstractDialogModel::modelChanged, this, &CommandBriefingDialog::updateUI);
		connect(this, &QDialog::accepted, _model.get(), &CommandBriefingDialogModel::apply);
		connect(viewport->editor, &Editor::currentObjectChanged, _model.get(), &CommandBriefingDialogModel::apply);
		connect(viewport->editor, &Editor::objectMarkingChanged, _model.get(), &CommandBriefingDialogModel::apply);
		connect(this, &QDialog::rejected, _model.get(), &CommandBriefingDialogModel::reject);

		connect(ui->actionChangeTeams,
			static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
			this,
			&CommandBriefingDialog::on_actionChangeTeams_clicked);

		connect(ui->actionBriefingTextEditor,
			&QPlainTextEdit::textChanged,
			this,
			&CommandBriefingDialog::briefingTextChanged);

		connect(ui->animationFileName,
			static_cast<void (QLineEdit::*)(const QString&)>(&QLineEdit::textChanged),
			this,
			&CommandBriefingDialog::animationFilenameChanged);

		connect(ui->speechFileName,
			static_cast<void (QLineEdit::*)(const QString&)>(&QLineEdit::textChanged),
			this,
			&CommandBriefingDialog::speechFilenameChanged);

		connect(ui->actionLowResolutionFilenameEdit,
			static_cast<void (QLineEdit::*)(const QString&)>(&QLineEdit::textChanged),
			this,
			&CommandBriefingDialog::lowResolutionFilenameChanged);

		connect(ui->actionHighResolutionFilenameEdit,
			static_cast<void (QLineEdit::*)(const QString&)>(&QLineEdit::textChanged),
			this,
			&CommandBriefingDialog::highResolutionFilenameChanged);

		resize(QDialog::sizeHint());

		_model->requestInitialUpdate();		
	}

	void CommandBriefingDialog::on_actionPrevStage_clicked() 
	{ 
		_model->gotoPreviousStage(); 
	}

	void CommandBriefingDialog::on_actionNextStage_clicked() 
	{ 
		_model->gotoNextStage(); 
	}

	void CommandBriefingDialog::on_actionAddStage_clicked() 
	{ 
		_model->addStage(); 
	}

	void CommandBriefingDialog::on_actionInsertStage_clicked() 
	{ 
		_model->insertStage();  
	}

	void CommandBriefingDialog::on_actionDeleteStage_clicked() 
	{ 
		_model->deleteStage(); 
	}

	void CommandBriefingDialog::on_actionChangeTeams_clicked ()
	{
		// not yet supported
	}

	void CommandBriefingDialog::on_actionCopyToOtherTeams_clicked()
	{
		// not yet supported.
	}

	void CommandBriefingDialog::on_actionBrowseAnimation_clicked()
	{
		QString filename;

		if (CommandBriefingDialog::browseFile(&filename)) {
			_model->setAnimationFilename(filename.toStdString());
		}
	}

	void CommandBriefingDialog::on_actionBrowseSpeechFile_clicked()
	{
		QString filename;

		if (CommandBriefingDialog::browseFile(&filename)) {
			_model->setSpeechFilename(filename.toStdString());
		}
	}
	
	void CommandBriefingDialog::on_actionTestSpeechFileButton_clicked()
	{
		_model->testSpeech();
	}

	void CommandBriefingDialog::on_actionLowResolutionBrowse_clicked()
	{
		QString filename;

		if (CommandBriefingDialog::browseFile(&filename)) {
			_model->setLowResolutionFilename(filename.toStdString());
		}
	}

	void CommandBriefingDialog::on_actionHighResolutionBrowse_clicked()
	{
		QString filename;

		if (CommandBriefingDialog::browseFile(&filename)) {
			_model->setHighResolutionFilename(filename.toStdString());
		}
	}

	void CommandBriefingDialog::updateUI()
	{

		util::SignalBlockers blockers(this);
		disableTeams(); // until supported, keep it from blowing things up

		// once supported, set the team here

		// only do this when necessary because the cursor will get moved around if this is not handled properly.
		if (_model->briefingUpdateRequired()) {
				ui->actionBriefingTextEditor->setPlainText(_model->getBriefingText().c_str());
		}

		// these line edits always seems to work fine without having to check with the model first.
		ui->animationFileName->setText(_model->getAnimationFilename().c_str());
		ui->speechFileName->setText(_model->getSpeechFilename().c_str());
		ui->actionLowResolutionFilenameEdit->setText(_model->getLowResolutionFilename().c_str());
		ui->actionHighResolutionFilenameEdit->setText(_model->getHighResolutionFilename().c_str());

		// needs to go at the end.
		enableDisableControls();
	}

	void CommandBriefingDialog::enableDisableControls()
	{

		if (_model->stageNumberUpdateRequired()) {
			int max_stage = _model->getTotalStages();

			if (max_stage == 0) {
				ui->actionPrevStage->setEnabled(false);
				ui->actionNextStage->setEnabled(false);
				ui->actionChangeTeams->setEnabled(false);
				ui->actionInsertStage->setEnabled(false);
				ui->actionDeleteStage->setEnabled(false);
				ui->actionBrowseAnimation->setEnabled(false);
				ui->animationFileName->setEnabled(false);
				ui->actionBrowseSpeechFile->setEnabled(false);
				ui->speechFileName->setEnabled(false);
				ui->actionTestSpeechFileButton->setEnabled(false);

				ui->currentStageLabel->setText("No Stages");
				return;
			}
			else {
				int current_stage = _model->getCurrentStage() + 1;

				if (current_stage == 1) {
					ui->actionPrevStage->setEnabled(false);
				}
				else {
					ui->actionPrevStage->setEnabled(true);
				}

				if (current_stage == max_stage) {
					ui->actionNextStage->setEnabled(false);
				}
				else {
					ui->actionNextStage->setEnabled(true);
				}

				if (max_stage >= CMD_BRIEF_STAGES_MAX) {
					ui->actionAddStage->setEnabled(false);
					ui->actionInsertStage->setEnabled(false);
				}
				else {
					ui->actionAddStage->setEnabled(true);
					ui->actionInsertStage->setEnabled(true);
				}

				ui->actionDeleteStage->setEnabled(true);
				ui->actionBrowseAnimation->setEnabled(true);
				ui->animationFileName->setEnabled(true);
				ui->actionBrowseSpeechFile->setEnabled(true);
				ui->speechFileName->setEnabled(true);



				SCP_string to_ui_string = "Stage ";
				to_ui_string += std::to_string(current_stage);
				to_ui_string += " of ";
				to_ui_string += std::to_string(max_stage);

				ui->currentStageLabel->setText(to_ui_string.c_str());
			}
		}

		if (_model->soundTestUpdateRequired()) {
			if (_model->getSpeechInstanceNumber() >= 0) {
				ui->actionTestSpeechFileButton->setEnabled(true);
			}
			else {
				ui->actionTestSpeechFileButton->setEnabled(false);
			}
		}
	}

	void CommandBriefingDialog::briefingTextChanged()
	{
		_model->setBriefingText(ui->actionBriefingTextEditor->document()->toPlainText().toStdString());
	}

	void CommandBriefingDialog::animationFilenameChanged(const QString& string)
	{
		_model->setAnimationFilename(string.toStdString());
	}

	void CommandBriefingDialog::speechFilenameChanged(const QString& string)
	{
		_model->setSpeechFilename(string.toStdString());
	}

	void CommandBriefingDialog::lowResolutionFilenameChanged(const QString& string)
	{
		_model->setLowResolutionFilename(string.toStdString());
	}

	void CommandBriefingDialog::highResolutionFilenameChanged(const QString& string)
	{
		_model->setHighResolutionFilename(string.toStdString());
	}

	// literally just here to keep things from blowing up until we have team Command Brief Support.
	void CommandBriefingDialog::disableTeams()
	{
		ui->actionChangeTeams->setEnabled(false);
		ui->actionCopyToOtherTeams->setEnabled(false);
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


	CommandBriefingDialog::~CommandBriefingDialog() {}; //NOLINT

	void CommandBriefingDialog::closeEvent(QCloseEvent*){}

} // dialogs
} // fred
} // fso