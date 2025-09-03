#pragma once
#include <mission/dialogs/VoiceActingManagerModel.h>

#include <QtWidgets/QDialog>

#include <ui/FredView.h>

namespace fso::fred::dialogs {

namespace Ui {
class VoiceActingManager;
}

class VoiceActingManager : public QDialog {
    Q_OBJECT
public:
    explicit VoiceActingManager(FredView* parent, EditorViewport* viewport);
	~VoiceActingManager() override;

protected:
	void closeEvent(QCloseEvent* event) override;
	bool eventFilter(QObject* obj, QEvent* ev) override;

private slots:
	// Abbrev line edits
	void on_abbrevBriefingLineEdit_textEdited(const QString& text);
	void on_abbrevCampaignLineEdit_textEdited(const QString& text);
	void on_abbrevCommandBriefingLineEdit_textEdited(const QString& text);
	void on_abbrevDebriefingLineEdit_textEdited(const QString& text);
	void on_abbrevMessageLineEdit_textEdited(const QString& text);
	void on_abbrevMissionLineEdit_textEdited(const QString& text);

	// Filename settings
	void on_includeSenderCheckBox_toggled(bool checked);
	void on_noReplaceCheckBox_toggled(bool checked);
	void on_suffixComboBox_currentIndexChanged(int index);

	// Script export
	void on_scriptEntryFormatPlainTextEdit_textChanged();
	void on_exportAllRadio_toggled(bool checked);
	void on_exportCmdBriefingRadio_toggled(bool checked);
	void on_exportBriefingRadio_toggled(bool checked);
	void on_exportDebriefingRadio_toggled(bool checked);
	void on_exportMessageRadio_toggled(bool checked);
	void on_groupMessagesCheckBox_toggled(bool checked);

	// Persona sync
	void on_personaSyncComboBox_currentIndexChanged(int index);

	// Actions
	void on_generateFilenamesButton_clicked();
	void on_generateScriptButton_clicked();
	void on_copyMsgToShipsButton_clicked();
	void on_copyShipsToMsgsButton_clicked();
	void on_clearNonSendersButton_clicked();
	void on_setHeadAnisButton_clicked();
	void on_checkAnyWingmanButton_clicked();

private: // NOLINT(readability-redundant-access-specifiers)
	EditorViewport* _viewport;
	std::unique_ptr<Ui::VoiceActingManager> ui;
	std::unique_ptr<VoiceActingManagerModel> _model;

	void initializeUi();
	void updateUi();

	void refreshExampleFilename();
	void populatePersonaCombo();
	void populateSuffixCombo();
	void syncGroupMessagesEnabled();

	// enum mappers
	static int exportSelectionToIndex(ExportSelection sel);

	static int suffixToIndex(Suffix s);
	static Suffix indexToSuffix(int idx);
};

} // namespace fso::fred::dialogs
