#ifndef CAMPAIGNEDITORDIALOG_H
#define CAMPAIGNEDITORDIALOG_H

#include <QDialog>
#include <QtWidgets/QMenuBar>
#include <QListWidgetItem>

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
	explicit CampaignEditorDialog(QWidget *parent = nullptr);
	~CampaignEditorDialog();

private:
	Ui::CampaignEditorDialog *ui;
	QMenuBar *menubar;
	bool changes = false;

public slots:
	void reject();

private slots:
	void fileNew();
	void fileOpen();
	void fileSave();
	void fileSaveAs();

	void otherErrorChecker();

	void initialShips();
	void initialWeapons();

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
	void btnBranchLoopClicked();

	void txaLoopDescrChanged();
	void txtLoopAnimChanged(const QString changed);
	void btnBrLoopAnimClicked();
	void txtLoopVoiceChanged(const QString changed);
	void btnBrLoopVoiceClicked();

	void btnRealignClicked();
	void btnLoadMissionClicked();
};

}
}
}

#endif // CAMPAIGNEDITORDIALOG_H
