#pragma once

#include <QtWidgets/QDialog>

#include <mission/dialogs/ShieldSystemDialogModel.h>
#include <ui/FredView.h>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class ShieldSystemDialog;
}

class ShieldSystemDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShieldSystemDialog(FredView* parent, EditorViewport* viewport);
	~ShieldSystemDialog() override;

protected:
	void keyPressEvent(QKeyEvent* event) override;
	void closeEvent(QCloseEvent*) override;
private:
	void updateUI();
	void updateTeam();
	void updateType();

	void teamSelectionChanged(int index);
	void typeSelectionChanged(int index);

	EditorViewport * _viewport = nullptr;
    std::unique_ptr<Ui::ShieldSystemDialog> ui;
	std::unique_ptr<ShieldSystemDialogModel> _model;
};

}
}
}
