#include "VariableDialog.h"
#include "ui_VariableDialog.h"
#include <ui/util/SignalBlockers.h>
#include "ui/widgets/LineEditDelegate.h"
#include <mission/util.h>
#include <QMessageBox>

// Roles to store data for the line edit delegate
const int IsStringRole = Qt::UserRole + 1;
const int MaxLength = Qt::UserRole + 2;

namespace fso::fred::dialogs {

// Simple enum for table columns for clarity
enum VarCol {
	VarName = 0,
	VarValue = 1
};
enum ContCol {
	ContName = 0,
	ContType = 1,
	ContNotes = 2
};
enum ItemCol {
	ItemKey = 0,
	ItemValue = 1
};

VariableDialog::VariableDialog(FredView* parent, EditorViewport* viewport)
	: QDialog(parent), ui(new Ui::VariableEditorDialog()), _viewport(viewport),
	  _model(new VariableDialogModel(this, viewport))
{
	this->setFocus();
	ui->setupUi(this);
	initializeUi();
	updateUi();
}

VariableDialog::~VariableDialog() = default;

void VariableDialog::accept()
{
	// If apply() returns true, close the dialog
	if (_model->apply()) {
		QDialog::accept();
	}
	// else: validation failed, don’t close
}

void VariableDialog::reject()
{
	// Asks the user if they want to save changes, if any
	// If they do, it runs _model->apply() and returns the success value
	// If they don't, it runs _model->reject() and returns true
	if (rejectOrCloseHandler(this, _model.get(), _viewport)) {
		QDialog::reject(); // actually close
	}
	// else: do nothing, don't close
}

void VariableDialog::closeEvent(QCloseEvent* e)
{
	reject();
	e->ignore(); // Don't let the base class close the window
}

void VariableDialog::initializeUi()
{
	util::SignalBlockers blockers(this);

	// Configure Variables Table
	ui->variablesTable->setColumnCount(2);
	ui->variablesTable->setHorizontalHeaderItem(VarName, new QTableWidgetItem("Name"));
	ui->variablesTable->setHorizontalHeaderItem(VarValue, new QTableWidgetItem("Value"));
	ui->variablesTable->setColumnWidth(VarName, 250);
	ui->variablesTable->setColumnWidth(VarValue, 250);

	// Create and apply the delegate for the 'Value' column
	auto* variable_value_delegate = new LineEditDelegate(this);
	ui->variablesTable->setItemDelegateForColumn(VarValue, variable_value_delegate);
	ui->variablesTable->setItemDelegateForColumn(VarName, variable_value_delegate);

	auto* var_header = ui->variablesTable->horizontalHeader();
	var_header->setSectionsClickable(false);
	var_header->setHighlightSections(false);
	var_header->setSectionsMovable(false);
	var_header->setSectionResizeMode(QHeaderView::Stretch);

	// Configure Containers Table
	ui->containersTable->setColumnCount(3);
	ui->containersTable->setHorizontalHeaderItem(ContName, new QTableWidgetItem("Name"));
	ui->containersTable->setHorizontalHeaderItem(ContType, new QTableWidgetItem("Types"));
	ui->containersTable->setHorizontalHeaderItem(ContNotes, new QTableWidgetItem("Notes"));
	ui->containersTable->setColumnWidth(ContName, 190);
	ui->containersTable->setColumnWidth(ContType, 220);
	ui->containersTable->setColumnWidth(ContNotes, 120);

	// Configure Items Table
	ui->containerContentsTable->setColumnCount(2);
	ui->containerContentsTable->setHorizontalHeaderItem(ItemKey, new QTableWidgetItem("Key"));
	ui->containerContentsTable->setHorizontalHeaderItem(ItemValue, new QTableWidgetItem("Value"));
	ui->containerContentsTable->setColumnWidth(ItemKey, 245);
	ui->containerContentsTable->setColumnWidth(ItemValue, 245);

	// The old UI had a format combobox, which our model doesn't support.
	// We'll hide it for now. //TODO reimplement this in the UI layer
	ui->selectFormatLabel->setVisible(false);
	ui->selectFormatCombobox->setVisible(false);
}

void VariableDialog::updateUi()
{
	util::SignalBlockers blockers(this);
	updateVariableList();
	updateContainerList();
	enableDisableControls();
}

void VariableDialog::updateVariableList()
{
	util::SignalBlockers blockers(this);
	
	const auto& variables = _model->getVariables();
	ui->variablesTable->clearContents();
	ui->variablesTable->setRowCount(0);

	int table_row = 0;
	for (int i = 0; i < variables.size(); ++i) {
		const auto& var = variables[i];

		ui->variablesTable->insertRow(table_row);

		auto* nameItem = new QTableWidgetItem(var.name.c_str());
		nameItem->setData(Qt::UserRole, i); // Store original model index
		nameItem->setData(MaxLength, TOKEN_LENGTH - 1);

		auto* valueItem = new QTableWidgetItem(_model->getVariableValue(i).c_str());
		valueItem->setData(Qt::UserRole, i);

		bool is_string = _model->getVariableType(i);
		valueItem->setData(IsStringRole, is_string);
		if (is_string) {
			valueItem->setData(MaxLength, TOKEN_LENGTH - 1);
		}

		ui->variablesTable->setItem(table_row, VarName, nameItem);
		ui->variablesTable->setItem(table_row, VarValue, valueItem);

		table_row++;
	}

	if (m_currentVariableIndex >= 0 && m_currentVariableIndex < variables.size()) {
		ui->variablesTable->selectRow(m_currentVariableIndex);
	} else if (!variables.empty()) {
		m_currentVariableIndex = 0;
		ui->variablesTable->selectRow(0);
	} else {
		m_currentVariableIndex = -1;
	}

	updateVariableControls();
}

void VariableDialog::updateVariableControls()
{
	util::SignalBlockers blockers(this);
	
	const bool is_selected = m_currentVariableIndex != -1;

	ui->setVariableAsStringRadio->setChecked(is_selected && _model->getVariableType(m_currentVariableIndex));
	ui->setVariableAsNumberRadio->setChecked(is_selected && !_model->getVariableType(m_currentVariableIndex));

	const int p_type = is_selected ? _model->getVariablePersistenceType(m_currentVariableIndex) : 0;
	ui->doNotSaveVariableRadio->setChecked(p_type == 0);
	ui->saveVariableOnMissionCompletedRadio->setChecked(p_type == 1);
	ui->saveVariableOnMissionCloseRadio->setChecked(p_type == 2);

	ui->networkVariableCheckbox->setChecked(is_selected && _model->getVariableNetwork(m_currentVariableIndex));
	ui->setVariableAsEternalcheckbox->setChecked(is_selected && _model->getVariableEternal(m_currentVariableIndex));

	enableDisableControls();
}

void VariableDialog::updateContainerList()
{
	util::SignalBlockers blockers(this);
	
	const auto& containers = _model->getContainers();
	ui->containersTable->clearContents();
	ui->containersTable->setRowCount(containers.size());

	for (int i = 0; i < containers.size(); ++i) {
		const auto& cont = containers[i];
		if (cont.deleted)
			continue;

		auto* nameItem = new QTableWidgetItem(cont.name.c_str());
		nameItem->setData(Qt::UserRole, i);
		ui->containersTable->setItem(i, ContName, nameItem);
		// TODO Type and Notes columns
	}

	if (m_currentContainerIndex >= 0 && m_currentContainerIndex < containers.size()) {
		ui->containersTable->selectRow(m_currentContainerIndex);
	} else if (!containers.empty()) {
		m_currentContainerIndex = 0;
		ui->containersTable->selectRow(0);
	} else {
		m_currentContainerIndex = -1;
	}

	updateContainerControls();
	updateItemList();
}

void VariableDialog::updateContainerControls()
{
	util::SignalBlockers blockers(this);
	
	const bool is_selected = m_currentContainerIndex != -1;
	ui->setContainerAsListRadio->setChecked(is_selected && _model->getContainerType(m_currentContainerIndex));
	ui->setContainerAsMapRadio->setChecked(is_selected && !_model->getContainerType(m_currentContainerIndex));
	ui->setContainerKeyAsStringRadio->setChecked(is_selected && _model->getContainerKeyType(m_currentContainerIndex));
	ui->setContainerKeyAsNumberRadio->setChecked(is_selected && !_model->getContainerKeyType(m_currentContainerIndex));
	ui->setContainerAsStringRadio->setChecked(is_selected && _model->getContainerValueType(m_currentContainerIndex));
	ui->setContainerAsNumberRadio->setChecked(is_selected && !_model->getContainerValueType(m_currentContainerIndex));

	const int p_type = is_selected ? _model->getContainerPersistenceType(m_currentContainerIndex) : 0;
	ui->doNotSaveContainerRadio->setChecked(p_type == 0);
	ui->saveContainerOnMissionCompletedRadio->setChecked(p_type == 1);
	ui->saveContainerOnMissionCloseRadio->setChecked(p_type == 2);

	ui->networkContainerCheckbox->setChecked(is_selected && _model->getContainerNetwork(m_currentContainerIndex));
	ui->setContainerAsEternalCheckbox->setChecked(is_selected && _model->getContainerEternal(m_currentContainerIndex));
}

void VariableDialog::updateItemList()
{
	util::SignalBlockers blockers(this);
	
	ui->containerContentsTable->clearContents();
	if (m_currentContainerIndex == -1) {
		ui->containerContentsTable->setRowCount(0);
		m_currentItemIndex = -1;
		updateItemControls();
		return;
	}

	const auto& container = _model->getContainers()[m_currentContainerIndex];
	const bool is_list = container.is_list;

	ui->containerContentsTable->horizontalHeaderItem(ItemKey)->setText(is_list ? "Value" : "Key");
	ui->containerContentsTable->horizontalHeaderItem(ItemValue)->setText(is_list ? "" : "Value");
	ui->containerContentsTable->setColumnHidden(ItemValue, is_list);

	const int row_count =
		is_list ? (container.values_are_strings ? container.stringValues.size() : container.numberValues.size())
				: container.keys.size();
	ui->containerContentsTable->setRowCount(row_count);

	for (int i = 0; i < row_count; ++i) {
		if (is_list) {
			auto* item = new QTableWidgetItem(_model->getListItemValue(m_currentContainerIndex, i).c_str());
			item->setData(Qt::UserRole, i);
			ui->containerContentsTable->setItem(i, ItemKey, item);
		} else { // Is Map
			auto* keyItem = new QTableWidgetItem(_model->getMapItemKey(m_currentContainerIndex, i).c_str());
			keyItem->setData(Qt::UserRole, i);
			auto* valItem = new QTableWidgetItem(_model->getMapItemValue(m_currentContainerIndex, i).c_str());
			valItem->setData(Qt::UserRole, i);
			ui->containerContentsTable->setItem(i, ItemKey, keyItem);
			ui->containerContentsTable->setItem(i, ItemValue, valItem);
		}
	}

	if (m_currentItemIndex >= 0 && m_currentItemIndex < row_count) {
		ui->containerContentsTable->selectRow(m_currentItemIndex);
	} else if (row_count > 0) {
		m_currentItemIndex = 0;
		ui->containerContentsTable->selectRow(0);
	} else {
		m_currentItemIndex = -1;
	}
	updateItemControls();
}

void VariableDialog::updateItemControls()
{
	// This function is for future use if we add item-specific editors.
	util::SignalBlockers blockers(this);
}

void VariableDialog::enableDisableControls()
{
	// Variables

	// The 'Add' button is enabled only if the model has room.
	ui->addVariableButton->setEnabled(_model->variableListHasSpace());

	// All other variable controls depend on having a row selected.
	const bool is_variable_selected = (m_currentVariableIndex != -1);

	ui->copyVariableButton->setEnabled(is_variable_selected);
	ui->deleteVariableButton->setEnabled(is_variable_selected);
	ui->variablesTypeGroupBox->setEnabled(is_variable_selected);
	ui->variablesPersistenceGroupBox->setEnabled(is_variable_selected);

	// The Eternal checkbox has a special condition: it requires both a selection
	// and a persistence type (Campaign or Player) to be active.
	if (is_variable_selected) {
		const bool is_persistent = _model->getVariablePersistenceType(m_currentVariableIndex) != 0;
		ui->setVariableAsEternalcheckbox->setEnabled(is_persistent);
	} else {
		// If nothing is selected, it must be disabled regardless.
		ui->setVariableAsEternalcheckbox->setEnabled(false);
	}

	// TODO: Implement logic for Containers section...
}

void VariableDialog::on_okAndCancelButtons_accepted()
{
	accept();
}

void VariableDialog::on_okAndCancelButtons_rejected()
{
	reject();
}

void VariableDialog::on_variablesTable_itemSelectionChanged()
{
	auto selected = ui->variablesTable->selectedItems();
	if (selected.isEmpty()) {
		m_currentVariableIndex = -1;
	} else {
		m_currentVariableIndex = selected.first()->data(Qt::UserRole).toInt();
	}

	updateVariableControls();
	enableDisableControls();
}

void VariableDialog::on_variablesTable_cellChanged(int row, int column)
{
	int model_idx = ui->variablesTable->item(row, column)->data(Qt::UserRole).toInt();
	if (column == VarName) {
		SCP_string name = ui->variablesTable->item(row, VarName)->text().toUtf8().constData();
		if (_model->isVariableNameUnique(name, model_idx)) {
			_model->setVariableName(model_idx, name);
		} else {
			QMessageBox::warning(this, "Duplicate Name", "A variable with that name already exists.");
			// Revert to previous name
			util::SignalBlockers b(this);
			ui->variablesTable->item(row, column)->setText(_model->getVariableName(model_idx).c_str());
			return;
		}
	} else if (column == VarValue) {
		bool is_string = _model->getVariableType(model_idx);

		if (!is_string) {
			// This is a numeric variable, so we must validate the input.
			bool is_numeric;
			QString text = ui->variablesTable->item(row, column)->text();
			text.toInt(&is_numeric); // Try to convert the string to an integer

			if (!is_numeric) {
				// The conversion failed, meaning the input is not a valid integer.
				QMessageBox::warning(this, "Invalid Input", "This variable only accepts integer values.");

				// Revert the cell's text to the last valid value from the model.
				util::SignalBlockers b(this);
				ui->variablesTable->item(row, column)->setText(_model->getVariableValue(model_idx).c_str());
				return;
			}
		}
		_model->setVariableValue(model_idx, ui->variablesTable->item(row, column)->text().toUtf8().constData());
	}

	updateVariableControls();
}

void VariableDialog::on_variablesFilterLineEdit_textChanged(const QString& text)
{
	const int selected_row = ui->variablesTable->currentRow();
	
	// Loop through all rows in the variables table
	for (int i = 0; i < ui->variablesTable->rowCount(); ++i) {
		// Get the item from the "Name" column
		auto* item = ui->variablesTable->item(i, VarName);
		if (!item) {
			continue;
		}

		// Check if the item's text contains the filter text (case-insensitive)
		const bool match = item->text().contains(text, Qt::CaseInsensitive);

		// Hide the row if it doesn't match, show it if it does
		ui->variablesTable->setRowHidden(i, !match);
	}

	// After filtering, if a row was selected and is now hidden...
	if (selected_row != -1 && ui->variablesTable->isRowHidden(selected_row)) {
		// ...clear the selection.
		ui->variablesTable->clearSelection();
	}
}

void VariableDialog::on_containersTable_itemSelectionChanged()
{
	auto selected = ui->containersTable->selectedItems();
	if (selected.isEmpty()) {
		m_currentContainerIndex = -1;
	} else {
		// Get the original model index we stored in the item's data
		m_currentContainerIndex = selected.first()->data(Qt::UserRole).toInt();
	}

	updateContainerControls();
	updateItemList();
	enableDisableControls();
}

void VariableDialog::on_containersTable_cellChanged(int row, int column)
{
	// We only care about edits to the name column
	if (column != ContName) {
		return;
	}

	int model_idx = ui->containersTable->item(row, column)->data(Qt::UserRole).toInt();
	if (model_idx < 0)
		return;

	_model->setContainerName(model_idx, ui->containersTable->item(row, column)->text().toUtf8().constData());

	updateContainerControls();
}

void VariableDialog::on_containerContentsTable_itemSelectionChanged()
{
	auto selected = ui->containerContentsTable->selectedItems();
	if (selected.isEmpty()) {
		m_currentItemIndex = -1;
	} else {
		m_currentItemIndex = selected.first()->data(Qt::UserRole).toInt();
	}

	util::SignalBlockers b(this);

	updateItemControls();
	enableDisableControls();
}

void VariableDialog::on_containerContentsTable_cellChanged(int row, int column)
{
	// Ensure a valid container is selected first
	if (m_currentContainerIndex == -1) {
		return;
	}

	int item_idx = ui->containerContentsTable->item(row, column)->data(Qt::UserRole).toInt();
	if (item_idx < 0)
		return;

	const SCP_string new_text = ui->containerContentsTable->item(row, column)->text().toUtf8().constData();
	const bool is_list = _model->getContainerType(m_currentContainerIndex);

	if (is_list) {
		_model->setListItemValue(m_currentContainerIndex, item_idx, new_text);
	} else { // Is a map
		if (column == ItemKey) {
			_model->setMapItemKey(m_currentContainerIndex, item_idx, new_text);
			// A key change can cause a re-sort, so we must refresh the item list
			updateItemList();
		} else if (column == ItemValue) {
			_model->setMapItemValue(m_currentContainerIndex, item_idx, new_text);
		}
	}
}

void VariableDialog::on_addVariableButton_clicked()
{
	_model->addNewVariable();
	updateUi();

	int last_row = ui->variablesTable->rowCount() - 1;
	if (last_row >= 0) {
		ui->variablesTable->selectRow(last_row);
	}
}

void VariableDialog::on_copyVariableButton_clicked()
{
	// Ensure a variable is actually selected
	if (m_currentVariableIndex == -1) {
		return;
	}

	_model->copyVariable(m_currentVariableIndex);

	updateUi();

	int last_row = ui->variablesTable->rowCount() - 1;
	if (last_row >= 0) {
		ui->variablesTable->selectRow(last_row);
	}
}

void VariableDialog::on_deleteVariableButton_clicked()
{
	if (m_currentVariableIndex != -1) {
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(this,
			"Confirm Deletion",
			"Are you sure you want to delete this variable? This cannot be undone!",
			QMessageBox::Yes | QMessageBox::No);
		if (reply == QMessageBox::Yes) {
			int index_to_delete = m_currentVariableIndex;
			_model->markVariableForDeletion(index_to_delete);
			m_currentVariableIndex = index_to_delete - 1;
			updateUi();
		}
	}
}

void VariableDialog::on_setVariableAsStringRadio_toggled(bool checked)
{
	if (checked && m_currentVariableIndex != -1) {
		_model->setVariableType(m_currentVariableIndex, true);
		updateVariableList(); // Need to update value column
	}
}

void VariableDialog::on_setVariableAsNumberRadio_toggled(bool checked)
{
	if (checked && m_currentVariableIndex != -1) {
		_model->setVariableType(m_currentVariableIndex, false);
		updateVariableList(); // Need to update value column
	}
}

void VariableDialog::on_doNotSaveVariableRadio_toggled(bool checked)
{
	if (checked && m_currentVariableIndex != -1) {
		_model->setVariablePersistenceType(m_currentVariableIndex, 0); // 0 = None
		updateVariableControls();
	}
}

void VariableDialog::on_saveVariableOnMissionCompletedRadio_toggled(bool checked)
{
	if (checked && m_currentVariableIndex != -1) {
		_model->setVariablePersistenceType(m_currentVariableIndex, 1); // 1 = Campaign/Progress
		updateVariableControls();
	}
}

void VariableDialog::on_saveVariableOnMissionCloseRadio_toggled(bool checked)
{
	if (checked && m_currentVariableIndex != -1) {
		_model->setVariablePersistenceType(m_currentVariableIndex, 2); // 2 = Player/Close
		updateVariableControls();
	}
}

void VariableDialog::on_networkVariableCheckbox_toggled(bool checked)
{
	if (m_currentVariableIndex != -1) {
		_model->setVariableNetwork(m_currentVariableIndex, checked);
		updateVariableControls();
	}
}

void VariableDialog::on_setVariableAsEternalcheckbox_toggled(bool checked)
{
	if (m_currentVariableIndex != -1) {
		_model->setVariableEternal(m_currentVariableIndex, checked);
		updateVariableControls();
	}
}

void VariableDialog::on_addContainerButton_clicked()
{
	_model->addNewContainer();
	updateUi();
}

void VariableDialog::on_copyContainerButton_clicked()
{
	// Ensure a container is actually selected
	if (m_currentContainerIndex == -1) {
		return;
	}

	_model->copyContainer(m_currentContainerIndex);

	updateUi();
}

void VariableDialog::on_deleteContainerButton_clicked()
{
	if (m_currentContainerIndex == -1) {
		return;
	}

	// Deleting is a destructive action, so we must ask the user for confirmation first.
	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(this,
		"Confirm Deletion",
		"Are you sure you want to delete this container?",
		QMessageBox::Yes | QMessageBox::No);

	if (reply == QMessageBox::Yes) {
		_model->markContainerForDeletion(m_currentContainerIndex, true);
		updateUi();
	}
}

void VariableDialog::on_setContainerAsListRadio_toggled(bool checked)
{
	if (checked && m_currentContainerIndex != -1) {
		// If the container is already a list, do nothing.
		if (_model->getContainerType(m_currentContainerIndex)) {
			return;
		}

		// Changing a map to a list is destructive (keys are lost).
		// We should ask for confirmation if the map is not empty.
		if (!_model->isContainerEmpty(m_currentContainerIndex)) {
			QMessageBox::StandardButton reply;
			reply = QMessageBox::question(this,
				"Confirm Type Change",
				"Changing a map to a list will discard all key data. Are you sure you want to continue?",
				QMessageBox::Yes | QMessageBox::No);

			if (reply == QMessageBox::No) {
				updateContainerControls(); // Revert the radio button state
				return;
			}
		}

		_model->setContainerType(m_currentContainerIndex, true); // true = is_list
		updateContainerList();
	}
}

void VariableDialog::on_setContainerAsMapRadio_toggled(bool checked)
{
	if (checked && m_currentContainerIndex != -1) {
		// Changing a list to a map is not destructive, so no confirmation is needed.
		_model->setContainerType(m_currentContainerIndex, false); // false = is_list
		updateContainerList();
	}
}

void VariableDialog::on_setContainerKeyAsStringRadio_toggled(bool checked)
{
	if (checked && m_currentContainerIndex != -1) {
		_model->setContainerKeyType(m_currentContainerIndex, true); // true = keys_are_strings
		updateContainerControls();
	}
}

void VariableDialog::on_setContainerKeyAsNumberRadio_toggled(bool checked)
{
	if (checked && m_currentContainerIndex != -1) {
		_model->setContainerKeyType(m_currentContainerIndex, false); // false = keys_are_strings
		updateContainerControls();
	}
}

void VariableDialog::on_setContainerAsStringRadio_toggled(bool checked)
{
	if (checked && m_currentContainerIndex != -1) {
		// If the type is already string, do nothing
		if (_model->getContainerValueType(m_currentContainerIndex)) {
			return;
		}

		if (!_model->isContainerEmpty(m_currentContainerIndex)) {
			QMessageBox::StandardButton reply;
			reply = QMessageBox::question(this,
				"Confirm Type Change",
				"Changing value types can result in data loss (e.g., non-numeric text becomes 0). Are you sure?",
				QMessageBox::Yes | QMessageBox::No);

			if (reply == QMessageBox::No) {
				updateContainerControls();
				return;
			}
		}

		_model->setContainerValueType(m_currentContainerIndex, true); // true = values_are_strings
		updateItemList();
	}
}

void VariableDialog::on_setContainerAsNumberRadio_toggled(bool checked)
{
	if (checked && m_currentContainerIndex != -1) {
		// If the type is already number, do nothing
		if (!_model->getContainerValueType(m_currentContainerIndex) == false) {
			return;
		}

		if (!_model->isContainerEmpty(m_currentContainerIndex)) {
			QMessageBox::StandardButton reply;
			reply = QMessageBox::question(this,
				"Confirm Type Change",
				"Changing value types can result in data loss (e.g., non-numeric text becomes 0). Are you sure?",
				QMessageBox::Yes | QMessageBox::No);

			if (reply == QMessageBox::No) {
				updateContainerControls();
				return;
			}
		}

		_model->setContainerValueType(m_currentContainerIndex, false); // false = values_are_strings
		updateItemList();
	}
}

void VariableDialog::on_doNotSaveContainerRadio_toggled(bool checked)
{
	if (checked && m_currentContainerIndex != -1) {
		_model->setContainerPersistenceType(m_currentContainerIndex, 0); // 0 = None
		updateContainerControls();
	}
}

void VariableDialog::on_saveContainerOnMissionCompletedRadio_toggled(bool checked)
{
	if (checked && m_currentContainerIndex != -1) {
		_model->setContainerPersistenceType(m_currentContainerIndex, 1); // 1 = Campaign/Progress
		updateContainerControls();
	}
}

void VariableDialog::on_saveContainerOnMissionCloseRadio_toggled(bool checked)
{
	if (checked && m_currentContainerIndex != -1) {
		_model->setContainerPersistenceType(m_currentContainerIndex, 2); // 2 = Player/Close
		updateContainerControls();
	}
}

void VariableDialog::on_networkContainerCheckbox_toggled(bool checked)
{
	if (m_currentContainerIndex != -1) {
		_model->setContainerNetwork(m_currentContainerIndex, checked);
		updateContainerControls();
	}
}

void VariableDialog::on_setContainerAsEternalCheckbox_toggled(bool checked)
{
	if (m_currentContainerIndex != -1) {
		_model->setContainerEternal(m_currentContainerIndex, checked);
		// We must update the controls here to sync the checkbox
		// with the true state, as the model might have rejected the change.
		updateContainerControls();
	}
}

void VariableDialog::on_addContainerItemButton_clicked()
{
	if (m_currentContainerIndex == -1) {
		return;
	}

	// The model knows whether it's a list or map and will add the correct item type.
	if (_model->getContainerType(m_currentContainerIndex)) {
		_model->addListItem(m_currentContainerIndex);
	} else {
		_model->addMapItem(m_currentContainerIndex);
	}

	// Refresh the item list to show the new entry
	updateItemList();
}

void VariableDialog::on_deleteContainerItemButton_clicked()
{
	if (m_currentContainerIndex == -1 || m_currentItemIndex == -1) {
		return;
	}

	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(this,
		"Confirm Deletion",
		"Are you sure you want to delete this item?",
		QMessageBox::Yes | QMessageBox::No);

	if (reply == QMessageBox::Yes) {
		if (_model->getContainerType(m_currentContainerIndex)) {
			_model->removeListItem(m_currentContainerIndex, m_currentItemIndex);
		} else {
			_model->removeMapItem(m_currentContainerIndex, m_currentItemIndex);
		}
		updateItemList();
	}
}

void VariableDialog::on_shiftItemUpButton_clicked()
{
	if (m_currentContainerIndex == -1 || m_currentItemIndex == -1) {
		return;
	}

	// This operation is only valid for lists.
	if (_model->getContainerType(m_currentContainerIndex)) {
		_model->moveListItem(m_currentContainerIndex, m_currentItemIndex, true); // true = up
		updateItemList();
	}
}

void VariableDialog::on_shiftItemDownButton_clicked()
{
	if (m_currentContainerIndex == -1 || m_currentItemIndex == -1) {
		return;
	}

	// This operation is only valid for lists.
	if (_model->getContainerType(m_currentContainerIndex)) {
		_model->moveListItem(m_currentContainerIndex, m_currentItemIndex, false); // false = down
		updateItemList();
	}
}

void VariableDialog::on_swapKeysAndValuesButton_clicked()
{
	if (m_currentContainerIndex == -1) {
		return;
	}

	// This operation is only valid for maps.
	if (!_model->getContainerType(m_currentContainerIndex)) {
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(this,
			"Confirm Swap",
			"This will swap all keys and values in the map, which may convert data types and cannot be undone easily. "
			"Are you sure?",
			QMessageBox::Yes | QMessageBox::No);

		if (reply == QMessageBox::Yes) {
			_model->swapMapKeysAndValues(m_currentContainerIndex);
			updateItemList();
		}
	}
}

} // namespace fso::fred::dialogs