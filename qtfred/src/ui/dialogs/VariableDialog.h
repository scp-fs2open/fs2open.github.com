#pragma once

#include <mission/dialogs/VariableDialogModel.h>
#include <ui/FredView.h>

#include <QDialog>

namespace fso::fred::dialogs {

namespace Ui {
class VariableEditorDialog;
}

class VariableDialog : public QDialog {
	Q_OBJECT

  public:
	explicit VariableDialog(FredView* parent, EditorViewport* viewport);
	~VariableDialog() override;

	void accept() override;
	void reject() override;

  protected:
	void closeEvent(QCloseEvent* e) override; // funnel all Window X presses through reject()

  private slots:
	// Dialog Controls
	void on_okAndCancelButtons_accepted();
	void on_okAndCancelButtons_rejected();

	// Table Selection and Edits
	void on_variablesTable_itemSelectionChanged();
	void on_variablesTable_cellChanged(int row, int column);
	void on_variablesFilterLineEdit_textChanged(const QString& text);
	void on_containersTable_itemSelectionChanged();
	void on_containersTable_cellChanged(int row, int column);
	void on_containerFilterLineEdit_textChanged(const QString& text);
	void on_containerContentsTable_itemSelectionChanged();
	void on_containerContentsTable_cellChanged(int row, int column);
	void on_containerItemFilterLineEdit_textChanged(const QString& text);

	// Variable Buttons
	void on_addVariableButton_clicked();
	void on_copyVariableButton_clicked();
	void on_deleteVariableButton_clicked();

	// Variable Type Radios
	void on_setVariableAsStringRadio_toggled(bool checked);
	void on_setVariableAsNumberRadio_toggled(bool checked);

	// Variable Persistence Radios
	void on_doNotSaveVariableRadio_toggled(bool checked);
	void on_saveVariableOnMissionCompletedRadio_toggled(bool checked);
	void on_saveVariableOnMissionCloseRadio_toggled(bool checked);
	void on_networkVariableCheckbox_toggled(bool checked);
	void on_setVariableAsEternalcheckbox_toggled(bool checked);

	// Container Buttons
	void on_addContainerButton_clicked();
	void on_copyContainerButton_clicked();
	void on_deleteContainerButton_clicked();

	// Container Type Radios
	void on_setContainerAsListRadio_toggled(bool checked);
	void on_setContainerAsMapRadio_toggled(bool checked);
	void on_setContainerKeyAsStringRadio_toggled(bool checked);
	void on_setContainerKeyAsNumberRadio_toggled(bool checked);
	void on_setContainerAsStringRadio_toggled(bool checked);
	void on_setContainerAsNumberRadio_toggled(bool checked);

	// Container Persistence Radios
	void on_doNotSaveContainerRadio_toggled(bool checked);
	void on_saveContainerOnMissionCompletedRadio_toggled(bool checked);
	void on_saveContainerOnMissionCloseRadio_toggled(bool checked);
	void on_networkContainerCheckbox_toggled(bool checked);
	void on_setContainerAsEternalCheckbox_toggled(bool checked);

	// Container Item Buttons
	void on_addContainerItemButton_clicked();
	void on_copyContainerItemButton_clicked();
	void on_deleteContainerItemButton_clicked();
	void on_shiftItemUpButton_clicked();
	void on_shiftItemDownButton_clicked();
	void on_swapKeysAndValuesButton_clicked();

  private: // NOLINT(readability-redundant-access-specifiers)
	// Core UI and Model
	EditorViewport* _viewport = nullptr;
	std::unique_ptr<Ui::VariableEditorDialog> ui;
	std::unique_ptr<VariableDialogModel> _model;

	// State trackers
	int m_currentVariableIndex = -1;
	int m_currentContainerIndex = -1;
	int m_currentItemIndex = -1;

	void initializeUi();
	void updateUi();

	// Granular update methods
	void updateVariableList();
	void updateVariableControls();
	void updateContainerList();
	void updateContainerControls();
	void updateItemList();
	void updateItemControls();

	void enableDisableControls();

	static QString formatContainerTypeString(const ContainerInfo& cont);
};

} // namespace fso::fred::dialogs