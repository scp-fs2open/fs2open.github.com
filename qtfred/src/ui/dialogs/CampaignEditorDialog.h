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

	void btnBranchUpClicked();
	void btnBranchDownClicked();
	void btnBranchLoopClicked();

	void btnBrLoopAnimClicked();
	void btnBrLoopVoiceClicked();

	void btnRealignClicked();
	void btnLoadMissionClicked();
};

}
}
}

#endif // CAMPAIGNEDITORDIALOG_H
