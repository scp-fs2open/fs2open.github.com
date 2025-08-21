#pragma once

#include <QDialog>

#include "mission/EditorViewport.h"
#include "mission/dialogs/MissionCutscenesDialogModel.h"

#include "ui/widgets/sexp_tree.h"

namespace fso::fred::dialogs {

namespace Ui {
class MissionCutscenesDialog;
}

class MissionCutscenesDialog : public QDialog, public SexpTreeEditorInterface {
    Q_OBJECT

public:
	explicit MissionCutscenesDialog(QWidget* parent, EditorViewport* viewport);
  ~MissionCutscenesDialog() override;

	void accept() override;
	void reject() override;

 protected:
	void closeEvent(QCloseEvent* event) override;

private slots:
	void on_okAndCancelButtons_accepted();
	void on_okAndCancelButtons_rejected();

	void on_displayTypeCombo_currentIndexChanged(int index);
	void on_cutsceneTypeCombo_currentIndexChanged(int index);
	void on_cutsceneFilename_textChanged(const QString& text);
	void on_newCutsceneBtn_clicked();

	void on_cutsceneEventTree_selectedRootChanged(int formula);
	void on_cutsceneEventTree_rootNodeDeleted(int node);
	void on_cutsceneEventTree_rootNodeFormulaChanged(int old, int node);
	void on_cutsceneEventTree_helpChanged(const QString& help);

 private: // NOLINT(readability-redundant-access-specifiers)
	void updateUi();
	void createNewCutscene();
	void changeCutsceneCategory(int type);
	void populateCutsceneCombos();
	void setCutsceneTypeDescription();

    std::unique_ptr<Ui::MissionCutscenesDialog> ui;
	std::unique_ptr<MissionCutscenesDialogModel> _model;

    EditorViewport* _viewport = nullptr;
	void load_tree();
	void recreate_tree();
};

} // namespace fso::fred::dialogs
