#pragma once

#include <QDialog>
#include <QtWidgets/QMenuBar>
#include <QListWidgetItem>
#include <QTextDocument>

#include <memory>

#include "mission/dialogs/CampaignEditorDialogModel.h"
#include "ui/widgets/sexp_tree.h"

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
	class CampaignEditorDialog;
}

class CampaignEditorDialogModel;

namespace CampaignEditorUtil {
	struct WarningMsg
	{
		QString title{};
		QString msg{};
		QString type{};

		WarningMsg() = default;
		WarningMsg(QString &&title, QString &&msg, QString &&type);
	};
	class WarningVec : public QObject, public QVector<WarningMsg>
	{
		Q_OBJECT
	signals:
		void gotMsg();
	public:
		inline void addMsg(WarningMsg &&msg){
			append(msg);
			gotMsg();
		}
	};
} // namespace CampaignEditorUtil

class CampaignEditorDialog : public QDialog
{
	Q_OBJECT
	static CampaignEditorUtil::WarningVec warnings;

public:
	explicit CampaignEditorDialog(QWidget *parent, EditorViewport *viewport);
	~CampaignEditorDialog() override;

	static inline void uiWarn(QString title, QString msg, QString type = ""){
		warnings.addMsg(CampaignEditorUtil::WarningMsg{std::move(title), std::move(msg), std::move(type)});
	}

private:
	std::unique_ptr<Ui::CampaignEditorDialog> ui;
	std::unique_ptr<CampaignEditorDialogModel> model;
	// no graphical (tree chart) view implemented

	/**
	 * @brief takes ownership of a model, sets the UI components to their submodels and connects to model slots
	 */
	void setModel(CampaignEditorDialogModel *model = nullptr);

	QWidget *const parent;
	EditorViewport *const viewport;

	/**
	 * @brief reopen branch in mission view after changes
	 * @param branch the branch
	 */
	inline void restoreBranchOpen(int branch);

	/**
	 * @brief shows a save changes dialog (if any) and handles its results
	 * @return whether it is safe to dismiss or replace the current model
	 * @returns true if changes were successfully saved, discarded or no changes were made
	 * @returns false if saving failed or was cancelled
	 */
	bool questionSaveChanges();
        
public slots:
	/**
	 * @brief onClose for dialogs. Checks for changes and prompts to save them.
	 * @note Dialog will be deleted after closing
	 */
	void reject() override;

	// these reset part or all of the view to the model
	/**
	 * @brief reset view for general/misc campaign data
	 */
	void updateUISpec();
	/**
	 * @brief reset view to current mission
	 * @param updateBranch whether to also reset branch view to mission's first branch
	 */
	void updateUIMission(bool updateBranch = true);
	/**
	 * @brief reset view to a branch of current mission
	 * @param idx which branch to display
	 */
	void updateUIBranch(int idx = -1);

	inline void updateUIAll(){updateUISpec(); updateUIMission(); updateUIBranch();}

private slots:
	// handlers for file menu operations
	void fileNew();
	void fileOpen();
	bool fileSave();
	bool fileSaveAs();
	void fileSaveCopyAs();

	/**
	 * @brief changes selection to the clicked mission list item
	 * @param idx index of the item
	 */
	void lstMissionsClicked(const QModelIndex &idx);
	/**
	 * @brief opens a right-click menu on a mission list item for creating mission links
	 * @param pos the clicked position
	 */
	void mnLinkMenu(const QPoint &pos);
};

} // namespace dialogs
} // namespace fred
} // namespace fso
