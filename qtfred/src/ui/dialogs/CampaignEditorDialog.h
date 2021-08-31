#ifndef CAMPAIGNEDITORDIALOG_H
#define CAMPAIGNEDITORDIALOG_H

#include <QDialog>
#include <QtWidgets/QMenuBar>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QTextDocument>

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

class CampaignEditorDialog : public QDialog
{
	Q_OBJECT

public:
	explicit CampaignEditorDialog(QWidget *parent, EditorViewport *viewport);
	~CampaignEditorDialog() override;

private:
	std::unique_ptr<Ui::CampaignEditorDialog> ui;
	std::unique_ptr<CampaignEditorDialogModel> model;

	void setModel(CampaignEditorDialogModel *model = nullptr);

	QWidget *const parent;
	EditorViewport *const viewport;

	bool questionSaveChanges();

public slots:
	void reject() override; //onClose for dialogs

	void updateUISpec();
	void updateUIMission(bool updateBranch = true);
	void updateUIBranch(int idx = -1);

	inline void updateUIAll(){updateUISpec(); updateUIMission(); updateUIBranch();}

private slots:
	void fileNew();
	void fileOpen();
	bool fileSave();
	bool fileSaveAs();
	void fileSaveCopyAs();

	void lstMissionsClicked(const QModelIndex &idx);
	void mnLinkMenu(const QPoint &pos);

	void btnBranchUpClicked();
	void btnBranchDownClicked();

	void btnErrorCheckerClicked();
};

}
}
}

#endif // CAMPAIGNEDITORDIALOG_H
