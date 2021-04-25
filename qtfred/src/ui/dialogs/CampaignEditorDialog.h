#ifndef CAMPAIGNEDITORDIALOG_H
#define CAMPAIGNEDITORDIALOG_H

#include <QDialog>
#include <QtWidgets/QMenuBar>
#include <QListWidgetItem>

#include <memory>

#include <mission/dialogs/CampaignEditorDialogModel.h>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
	class CampaignEditorDialog;
}

class CampaignEditorDialog : public QDialog
{
	Q_OBJECT

public:
	explicit CampaignEditorDialog(QWidget *parent, EditorViewport *viewport);
	~CampaignEditorDialog();

private:
	std::unique_ptr<Ui::CampaignEditorDialog> ui;
	std::unique_ptr<CampaignEditorDialogModel> model;
	std::unique_ptr<QMenuBar> menubar;

	QWidget *const _parent;
	EditorViewport *const _viewport;

	bool attemptClose();

public slots:
	void reject() override; //onClose for dialogs

private slots:
	void fileNew();
	void fileOpen();
	void fileSave();
	void fileSaveAs();

	void listedMissionActivated(const QListWidgetItem *item);

	void txtNameChanged(const QString changed);
	void cmbTypeChanged(const QString changed);
	void chkTechResetChanged(const int changed);

	void txaDescrTextChanged();

	void txtBriefingCutsceneChanged(const QString changed);
	void txtMainhallChanged(const QString changed);
	void txtDebriefingPersonaChanged(const QString changed);

	void btnBranchUpClicked();
	void btnBranchDownClicked();
	void btnBranchLoopToggled(bool checked);

	void txaLoopDescrChanged();
	void txtLoopAnimChanged(const QString changed);
	void btnBrLoopAnimClicked();
	void txtLoopVoiceChanged(const QString changed);
	void btnBrLoopVoiceClicked();

	void btnErrorCheckerClicked();
	void btnRealignClicked();
	void btnLoadMissionClicked();
};

}
}
}

#endif // CAMPAIGNEDITORDIALOG_H
