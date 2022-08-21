#ifndef REINFORCEMENTEDITORDIALOG_H
#define REINFORCEMENTEDITORDIALOG_H

#include <QtWidgets/QDialog>
#include <mission/dialogs/ReinforcementsEditorDialogModel.h>
#include <ui/FredView.h>
#include <globalincs/pstypes.h>

// not sure if I need these yet.
//#include <QAbstractButton>

namespace fso {
namespace fred {
namespace dialogs {

	namespace Ui {
		class ReinforcementsDialog;
	}

class ReinforcementsDialog : public QDialog {
	
	Q_OBJECT
	
public:

	explicit ReinforcementsDialog(FredView* parent, EditorViewport* viewport);
	~ReinforcementsDialog() override; // NOLINT

protected:
	void closeEvent(QCloseEvent*) override;

private slots:
	void on_chosenShipsList_clicked();
	void on_actionAddShip_clicked();
	void on_actionRemoveShip_clicked();
	void on_moveSelectionUp_clicked();
	void on_moveSelectionDown_clicked();

private:
	std::unique_ptr<Ui::ReinforcementsDialog> ui;
	std::unique_ptr<ReinforcementsDialogModel> _model;
	EditorViewport* _viewport;

	void updateUI();
	void enableDisableControls();

	void onReinforcementItemChanged();
	void onUseCountChanged();
	void onDelayChanged();

};

}
}
}
#endif
