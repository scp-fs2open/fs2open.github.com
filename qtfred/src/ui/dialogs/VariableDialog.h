#pragma once

#include <mission/dialogs/VariableDialogModel.h>
#include <QDialog>
#include <QtWidgets/QDialog>
#include <ui/FredView.h>

namespace fso {
namespace fred {
namespace dialogs {

namespace Ui {
class VariableEditorDialog;
}

class VariableDialog : public QDialog {
	Q_OBJECT

  public:
	explicit VariableDialog(FredView* parent, EditorViewport* viewport);
	~VariableDialog() override;

  private:
	std::unique_ptr<Ui::VariableEditorDialog> ui;
	std::unique_ptr<VariableDialogModel> _model;
	EditorViewport* _viewport;

	// basically UpdateUI, but called when there is an inconsistency between model and UI
	void applyModel();
	// Helper functions for this
	void updateVariableOptions();
	void updateContainerOptions();
	void updateContainerDataOptions(bool list);

	void onVariablesTableUpdated();
	void onVariablesSelectionChanged();
	void onContainersTableUpdated();
	void onContainersSelectionChanged();
	void onContainerContentsTableUpdated();
	void onContainerContentsSelectionChanged();
	void onAddVariableButtonPressed();
	void onDeleteVariableButtonPressed();
	void onCopyVariableButtonPressed();
	void onSetVariableAsStringRadioSelected();
	void onSetVariableAsNumberRadioSelected();
	void onSaveVariableOnMissionCompleteRadioSelected();
	void onSaveVariableOnMissionCloseRadioSelected();
	void onSaveVariableAsEternalCheckboxClicked();
	void onNetworkVariableCheckboxClicked();

	void onAddContainerButtonPressed();
	void onDeleteContainerButtonPressed();
	void onCopyContainerButtonPressed();
	void onSetContainerAsMapRadioSelected();
	void onSetContainerAsListRadioSelected();
	void onSetContainerAsStringRadioSelected();
	void onSetContainerAsNumberRadioSelected();
	void onSetContainerKeyAsStringRadioSelected();
	void onSetContainerKeyAsNumberRadioSelected()
	void onSaveContainerOnMissionClosedRadioSelected();
	void onSaveContainerOnMissionCompletedRadioSelected();
	void onNetworkContainerCheckboxClicked();
	void onSetContainerAsEternalCheckboxClicked();
	void onAddContainerItemButtonPressed();
	void onCopyContainerItemButtonPressed();
	void onDeleteContainerItemButtonPressed();

	SCP_string trimNumberString(SCP_string source);

	bool _applyingModel = false;
	SCP_string _currentVariable = "";
	SCP_string _currentContainer = "";
	SCP_string _currentContainerItem = "";
};





} // namespace dialogs
} // namespace fred
} // namespace fso