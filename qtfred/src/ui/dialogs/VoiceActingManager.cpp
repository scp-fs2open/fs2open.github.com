#include "VoiceActingManager.h"

#include "ui_VoiceActingManager.h"

#include "missioneditor/common.h"

#include <QMessageBox>
#include <QFileDialog>


namespace fso::fred::dialogs {

VoiceActingManager::VoiceActingManager(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), _viewport(viewport), ui(new Ui::VoiceActingManager()),
	  _model(new VoiceActingManagerModel(this, viewport))
{
    ui->setupUi(this);

	// Install this dialog as the event filter on the abbrev fields
	ui->abbrevBriefingLineEdit->installEventFilter(this);
	ui->abbrevCampaignLineEdit->installEventFilter(this);
	ui->abbrevCommandBriefingLineEdit->installEventFilter(this);
	ui->abbrevDebriefingLineEdit->installEventFilter(this);
	ui->abbrevMessageLineEdit->installEventFilter(this);
	ui->abbrevMissionLineEdit->installEventFilter(this);

	populatePersonaCombo();
	populateSuffixCombo();
	initializeUi();
	updateUi();
	
	// Resize the dialog to the minimum size
	resize(QDialog::sizeHint());
}

VoiceActingManager::~VoiceActingManager() = default;

void VoiceActingManager::closeEvent(QCloseEvent* e)
{
	_model->apply();
	e->accept(); //close
}

bool VoiceActingManager::eventFilter(QObject* obj, QEvent* ev)
{
	if (ev->type() == QEvent::FocusIn) {
		// Only react for our four main abbrev fields
		if (obj == ui->abbrevBriefingLineEdit || obj == ui->abbrevCommandBriefingLineEdit || 
			obj == ui->abbrevDebriefingLineEdit || obj == ui->abbrevMessageLineEdit) {

			if (obj == ui->abbrevCommandBriefingLineEdit) {
				_model->setAbbrevSelection(ExportSelection::CommandBriefings);
			}

			if (obj == ui->abbrevBriefingLineEdit) {
				_model->setAbbrevSelection(ExportSelection::Briefings);
			}

			if (obj == ui->abbrevDebriefingLineEdit) {
				_model->setAbbrevSelection(ExportSelection::Debriefings);
			}

			if (obj == ui->abbrevMessageLineEdit) {
				_model->setAbbrevSelection(ExportSelection::Messages);
			}
			refreshExampleFilename();
		}
	}
	// Let normal processing continue
	return QDialog::eventFilter(obj, ev);
}

void VoiceActingManager::initializeUi()
{
	// Abbreviations
	ui->abbrevBriefingLineEdit->setText(QString::fromStdString(_model->abbrevBriefing()));
	ui->abbrevCampaignLineEdit->setText(QString::fromStdString(_model->abbrevCampaign()));
	ui->abbrevCommandBriefingLineEdit->setText(QString::fromStdString(_model->abbrevCommandBriefing()));
	ui->abbrevDebriefingLineEdit->setText(QString::fromStdString(_model->abbrevDebriefing()));
	ui->abbrevMessageLineEdit->setText(QString::fromStdString(_model->abbrevMessage()));
	ui->abbrevMissionLineEdit->setText(QString::fromStdString(_model->abbrevMission()));

	// Filename settings
	ui->includeSenderCheckBox->setChecked(_model->includeSenderInFilename());
	ui->replaceCheckBox->setChecked(_model->noReplace());
	ui->suffixComboBox->setCurrentIndex(suffixToIndex(_model->suffix()));

	// Script export
	ui->scriptEntryFormatPlainTextEdit->setPlainText(QString::fromStdString(_model->scriptEntryFormat()));
	ui->exportAllRadio->setChecked(_model->exportSelection() == ExportSelection::Everything);
	ui->exportCmdBriefingRadio->setChecked(_model->exportSelection() == ExportSelection::CommandBriefings);
	ui->exportBriefingRadio->setChecked(_model->exportSelection() == ExportSelection::Briefings);
	ui->exportDebriefingRadio->setChecked(_model->exportSelection() == ExportSelection::Debriefings);
	ui->exportMessageRadio->setChecked(_model->exportSelection() == ExportSelection::Messages);
	ui->groupMessagesCheckBox->setChecked(_model->groupMessages());

	ui->scriptLegendLabel->setText(QString::fromStdString(Voice_script_instructions_string));

	// Persona sync
	ui->personaSyncComboBox->setCurrentIndex(_model->whichPersonaToSync());
}

void VoiceActingManager::updateUi()
{
	refreshExampleFilename();
}

void VoiceActingManager::refreshExampleFilename()
{
	const auto ex = _model->buildExampleFilename();
	ui->exampleFilenameLabel->setText(QString::fromStdString(ex));
}

void VoiceActingManager::populatePersonaCombo()
{
	for (const auto& p : _model->personaChoices()) {
		ui->personaSyncComboBox->addItem(QString::fromStdString(p));
	}
}

void VoiceActingManager::populateSuffixCombo()
{
	for (const auto& s : _model->fileChoices()) {
		ui->suffixComboBox->addItem(QString::fromStdString(s));
	}
}

void VoiceActingManager::syncGroupMessagesEnabled()
{
	const auto sel = _model->exportSelection();
	const bool enable = (sel == ExportSelection::Everything || sel == ExportSelection::Messages);
	ui->groupMessagesCheckBox->setEnabled(enable);
}

int VoiceActingManager::exportSelectionToIndex(ExportSelection sel)
{
	switch (sel) {
	case ExportSelection::Everything:
		return 0;
	case ExportSelection::CommandBriefings:
		return 1;
	case ExportSelection::Briefings:
		return 2;
	case ExportSelection::Debriefings:
		return 3;
	case ExportSelection::Messages:
		return 4;
	default:
		Assertion(false, "Invalid export selection!");
		return 0;
	}
}

int VoiceActingManager::suffixToIndex(Suffix s)
{
	switch (s) {
		case Suffix::WAV:
			return 0;
		case Suffix::OGG:
			return 1;
		default:
			Assertion(false, "Invalid file type selected!");
			return 0;
	}
}

Suffix VoiceActingManager::indexToSuffix(int idx)
{
	switch (idx) {
		case 0:
			return Suffix::WAV;
		case 1:
			return Suffix::OGG;
		default:
			Assertion(false, "Invalid file type selected!");
			return Suffix::WAV;
	}
}

void VoiceActingManager::on_abbrevBriefingLineEdit_textEdited(const QString& text)
{
	_model->setAbbrevBriefing(text.toUtf8().constData());
	refreshExampleFilename();
}
void VoiceActingManager::on_abbrevCampaignLineEdit_textEdited(const QString& text)
{
	_model->setAbbrevCampaign(text.toUtf8().constData());
	refreshExampleFilename();
}
void VoiceActingManager::on_abbrevCommandBriefingLineEdit_textEdited(const QString& text)
{
	_model->setAbbrevCommandBriefing(text.toUtf8().constData());
	refreshExampleFilename();
}
void VoiceActingManager::on_abbrevDebriefingLineEdit_textEdited(const QString& text)
{
	_model->setAbbrevDebriefing(text.toUtf8().constData());
	refreshExampleFilename();
}
void VoiceActingManager::on_abbrevMessageLineEdit_textEdited(const QString& text)
{
	_model->setAbbrevMessage(text.toUtf8().constData());
	refreshExampleFilename();
}
void VoiceActingManager::on_abbrevMissionLineEdit_textEdited(const QString& text)
{
	_model->setAbbrevMission(text.toUtf8().constData());
	refreshExampleFilename();
}

void VoiceActingManager::on_includeSenderCheckBox_toggled(bool checked)
{
	_model->setIncludeSenderInFilename(checked);
	refreshExampleFilename();
}
void VoiceActingManager::on_noReplaceCheckBox_toggled(bool checked)
{
	_model->setNoReplace(checked);
}
void VoiceActingManager::on_suffixComboBox_currentIndexChanged(int index)
{
	_model->setSuffix(indexToSuffix(index));
	refreshExampleFilename();
}

void VoiceActingManager::on_scriptEntryFormatPlainTextEdit_textChanged()
{
	_model->setScriptEntryFormat(ui->scriptEntryFormatPlainTextEdit->toPlainText().toUtf8().constData());
}

void VoiceActingManager::on_exportAllRadio_toggled(bool checked)
{
	if (checked) {
		_model->setExportSelection(ExportSelection::Everything);
		syncGroupMessagesEnabled();
	}
}

void VoiceActingManager::on_exportCmdBriefingRadio_toggled(bool checked)
{
	if (checked) {
		_model->setExportSelection(ExportSelection::CommandBriefings);
		syncGroupMessagesEnabled();
	}
}

void VoiceActingManager::on_exportBriefingRadio_toggled(bool checked)
{
	if (checked) {
		_model->setExportSelection(ExportSelection::Briefings);
		syncGroupMessagesEnabled();
	}
}

void VoiceActingManager::on_exportDebriefingRadio_toggled(bool checked)
{
	if (checked) {
		_model->setExportSelection(ExportSelection::Debriefings);
		syncGroupMessagesEnabled();
	}
}

void VoiceActingManager::on_exportMessageRadio_toggled(bool checked)
{
	if (checked) {
		_model->setExportSelection(ExportSelection::Messages);
		syncGroupMessagesEnabled();
	}
}

void VoiceActingManager::on_groupMessagesCheckBox_toggled(bool checked)
{
	_model->setGroupMessages(checked);
}

void VoiceActingManager::on_personaSyncComboBox_currentIndexChanged(int index)
{
	_model->setWhichPersonaToSync(index);
}

void VoiceActingManager::on_generateFilenamesButton_clicked()
{
	const int count = _model->generateFilenames();
	QMessageBox::information(this, tr("Generate Filenames"), tr("%1 filename(s) updated.").arg(count));
	refreshExampleFilename();
}
void VoiceActingManager::on_generateScriptButton_clicked()
{
	const QString path = QFileDialog::getSaveFileName(this,
		tr("Export Voice Script"),
		QString(),
		tr("Text files (*.txt);;All files (*)"));
	if (path.isEmpty())
		return;

	const bool ok = _model->generateScript(path.toUtf8().constData());
	if (ok) {
		QMessageBox::information(this, tr("Export"), tr("Script exported:\n%1").arg(path));
	} else {
		QMessageBox::warning(this, tr("Export Failed"), tr("Could not open:\n%1").arg(path));
	}
}
void VoiceActingManager::on_copyMsgToShipsButton_clicked()
{
	const int n = _model->copyMessagePersonasToShips();
	QMessageBox::information(this, tr("Copy"), tr("Personas copied to %1 ship(s).").arg(n));
}
void VoiceActingManager::on_copyShipsToMsgsButton_clicked()
{
	const int n = _model->copyShipPersonasToMessages();
	QMessageBox::information(this, tr("Copy"), tr("Personas copied to %1 message(s).").arg(n));
}
void VoiceActingManager::on_clearNonSendersButton_clicked()
{
	const int n = _model->clearPersonasFromNonSenders();
	QMessageBox::information(this, tr("Clear"), tr("Cleared %1 ship(s).").arg(n));
}
void VoiceActingManager::on_setHeadAnisButton_clicked()
{
	const int n = _model->setHeadAnisUsingMessagesTbl();
	QMessageBox::information(this, tr("Set Head ANIs"), tr("Updated %1 message(s).").arg(n));
}
void VoiceActingManager::on_checkAnyWingmanButton_clicked()
{
	const auto res = _model->checkAnyWingmanPersonas();
	if (!res.anyWingmanFound) {
		QMessageBox::information(this, tr("Check <any wingman>"), tr("No \"<any wingman>\" messages found."));
		return;
	}
	if (res.issueCount == 0) {
		QMessageBox::information(this, tr("Check <any wingman>"), tr("All \"<any wingman>\" messages look good."));
	} else {
		QMessageBox::warning(this,
			tr("Check <any wingman>"),
			tr("Issues found (%1):\n%2").arg(res.issueCount).arg(QString::fromStdString(res.report)));
	}
}

} // namespace fso::fred::dialogs
