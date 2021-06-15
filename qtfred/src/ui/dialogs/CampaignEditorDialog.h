#ifndef CAMPAIGNEDITORDIALOG_H
#define CAMPAIGNEDITORDIALOG_H

#include <QDialog>
#include <QtWidgets/QMenuBar>
#include <QListWidgetItem>
#include <QMessageBox>

#include <memory>

#include <mission/dialogs/CampaignEditorDialogModel.h>
#include <ui/widgets/sexp_tree.h>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
	class CampaignEditorDialog;
}

class CampaignEditorDialogModel;

class CampaignEditorDialog : public QDialog, public SexpTreeEditorInterface
{
	Q_OBJECT

public:
	explicit CampaignEditorDialog(QWidget *parent, EditorViewport *viewport);
	~CampaignEditorDialog();

	bool requireCampaignOperators() const override {return true;}

private:
	std::unique_ptr<Ui::CampaignEditorDialog> ui;
	std::unique_ptr<CampaignEditorDialogModel> model;

	void setModel(CampaignEditorDialogModel *model = nullptr);

	QWidget *const _parent;
	EditorViewport *const _viewport;

	bool questionSaveChanges();

public slots:
	void reject() override; //onClose for dialogs
	void updateUI();
	void updateUIBranch();

private slots:
	void fileNew();
	void fileOpen();
	bool fileSave();
	bool fileSaveAs();
	void fileSaveCopyAs();

	void createTree(bool enabled = true);

	void btnBranchUpClicked();
	void btnBranchDownClicked();

	void btnBrLoopAnimClicked();
	void btnBrLoopVoiceClicked();

	void btnErrorCheckerClicked();
	void btnRealignClicked();
	void btnFredMissionClicked();
};

}
}
}

#endif // CAMPAIGNEDITORDIALOG_H
