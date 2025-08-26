#pragma once

#include <QtWidgets/QDialog>
#include <mission/dialogs/ReinforcementsEditorDialogModel.h>
#include <ui/FredView.h>
#include <globalincs/pstypes.h>

#include <QListWidget>

namespace fso::fred::dialogs {

namespace Ui {
	class ReinforcementsDialog;
}

class ReinforcementsDialog : public QDialog {
	
	Q_OBJECT
	
public:

	explicit ReinforcementsDialog(FredView* parent, EditorViewport* viewport);
	~ReinforcementsDialog() override; // NOLINT

	void accept() override;
	void reject() override;

protected:
	void closeEvent(QCloseEvent* e) override; // funnel all Window X presses through reject()

private slots:
	void on_okAndCancelButtons_accepted();
	void on_okAndCancelButtons_rejected();

	void on_actionAddShip_clicked();
	void on_actionRemoveShip_clicked();
	void on_moveSelectionUp_clicked();
	void on_moveSelectionDown_clicked();
	void on_useSpinBox_valueChanged(int val);
	void on_delaySpinBox_valueChanged(int val);
	void on_chosenShipsList_itemClicked(QListWidgetItem* /*item*/);

	void on_chosenMultiselectCheckbox_toggled(bool checked);
	void on_poolMultiselectCheckbox_toggled(bool checked);

private:  // NOLINT(readability-redundant-access-specifiers)
	std::unique_ptr<Ui::ReinforcementsDialog> ui;
	std::unique_ptr<ReinforcementsDialogModel> _model;
	EditorViewport* _viewport;

	void updateUi();
	void enableDisableControls();

};

} // namespace fso::fred::dialogs
