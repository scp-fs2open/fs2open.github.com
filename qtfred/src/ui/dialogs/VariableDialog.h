#pragma once

#include <mission/dialogs/VariableDialogModel.h>
#include <QDialog>
#include <QtWidgets/QDialog>
#include <ui/FredView.h>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class VariableDialog;
}

class VariableDialog : public QDialog {
	Q_OBJECT

  public:
	explicit VariableDialog(FredView* parent, EditorViewport* viewport);
	~VariableDialog() override;

  private:
	std::unique_ptr<Ui::VariableDialog> ui;
	std::unique_ptr<VariableDialogModel> _model;
	EditorViewport* _viewport;

	void updateUI();

	void onVariablesTableUpdated();
	void onContainersTableUpdated();
	void onContainerContentsTableUpdated();
	void onAddVariableButtonPressed();
	void onDeleteVariableButtonPressed();
	void onSetVariableAsStringRadioSelected();
	void onSetVariableAsNumberRadioSelected();
	void onSaveVariableOnMissionCompleteRadioSelected();
	void onSaveVariableOnMissionCloseRadioSelected();
	void onSaveVariableAsEternalCheckboxClicked();

	void onAddContainerButtonPressed();
	void onDeleteContainerButtonPressed();
	void onSetContainerAsMapRadioSelected();
	void onSetContainerAsListRadioSelected();
	void onSetContainerAsStringRadioSelected();
	void onSetContainerAsNumberRadio();
	void onSaveContainerOnMissionClosedRadioSelected();
	void onSaveContainerOnMissionCompletedRadioSelected();
	void onNetworkContainerCheckboxClicked();
	void onSetContainerAsEternalCheckboxClicked();
	void onAddContainerItemButtonPressed();
	void onDeleteContainerItemButtonPressed();
};





} // namespace dialogs
} // namespace fred
} // namespace fso