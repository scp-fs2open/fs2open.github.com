#ifndef MISSIONGOALSDIALOG_H
#define MISSIONGOALSDIALOG_H

#include <QDialog>

#include "mission/EditorViewport.h"
#include "mission/dialogs/MissionGoalsDialogModel.h"

#include "ui/widgets/sexp_tree.h"

#include <memory>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class MissionGoalsDialog;
}

class MissionGoalsDialog : public QDialog, public SexpTreeEditorInterface
{
    Q_OBJECT

public:
    explicit MissionGoalsDialog(QWidget *parent, EditorViewport* viewport);
    ~MissionGoalsDialog() override;

 protected:
	void closeEvent(QCloseEvent* event) override;

 private:
	void updateUI();

	void createNewObjective();

	void changeGoalCategory(int type);

    std::unique_ptr<Ui::MissionGoalsDialog> ui;
    std::unique_ptr<MissionGoalsDialogModel> _model;

    EditorViewport* _viewport = nullptr;
	void load_tree();
	void recreate_tree();
};

}
}
}

#endif // MISSIONGOALSDIALOG_H
