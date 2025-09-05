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
	: QDialog(parent), _viewport(viewport), ui(new Ui::VariableEditorDialog()),
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
	auto* var_header = ui->variablesTable->horizontalHeader();
	var_header->setSectionsClickable(false);
	var_header->setHighlightSections(false);
	var_header->setSectionsMovable(false);
	var_header->setSectionResizeMode(0, QHeaderView::Stretch);
	var_header->setSectionResizeMode(1, QHeaderView::Stretch);

	// Create one delegate to be reused for all variable editing
	auto* variable_value_delegate = new LineEditDelegate(this);
	ui->variablesTable->setItemDelegateForColumn(VarValue, variable_value_delegate);
	ui->variablesTable->setItemDelegateForColumn(VarName, variable_value_delegate);

	// Configure Containers Table
	auto* cont_header = ui->containersTable->horizontalHeader();
	cont_header->setSectionsClickable(false);
	cont_header->setHighlightSections(false);
	cont_header->setSectionsMovable(false);
	cont_header->setSectionResizeMode(0, QHeaderView::Stretch);
	cont_header->setSectionResizeMode(1, QHeaderView::Stretch);

	// Create one delegate to be reused for all container editing
	auto* container_delegate = new LineEditDelegate(this);
	ui->containersTable->setItemDelegateForColumn(ContName, container_delegate);
	ui->containerContentsTable->setItemDelegate(container_delegate);

	// Configure Items Table
	auto* cont_items_header = ui->containersTable->horizontalHeader();
	cont_items_header->setSectionsClickable(false);
	cont_items_header->setHighlightSections(false);
	cont_items_header->setSectionsMovable(false);
	cont_items_header->setSectionResizeMode(0, QHeaderView::Stretch);
	cont_items_header->setSectionResizeMode(1, QHeaderView::Stretch);

	// The old UI had a format combobox, which our model doesn't support.
	// We'll hide it for now. //TODO reimplement this in the UI layer
	ui->selectFormatLabel->setVisible(false);
	ui->selectFormatCombobox->setVisible(false);

	ui->tabWidget->setCurrentIndex(0);
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
	for (int i = 0; i < static_cast<int>(variables.size()); ++i) {
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

	if (m_currentVariableIndex >= 0 && m_currentVariableIndex < static_cast<int>(variables.size())) {
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

	// Start with an empty table and build it dynamically
	ui->containersTable->setRowCount(0);

	int table_row = 0;
	for (int i = 0; i < static_cast<int>(containers.size()); ++i) {
		const auto& cont = containers[i];

		ui->containersTable->insertRow(table_row);

		// Name Item
		auto* nameItem = new QTableWidgetItem(cont.name.c_str());
		nameItem->setData(Qt::UserRole, i);
		nameItem->setData(MaxLength, TOKEN_LENGTH - 1);
		ui->containersTable->setItem(table_row, ContName, nameItem);

		// Type Item (using our new helper)
		auto* typeItem = new QTableWidgetItem(formatContainerTypeString(cont));
		typeItem->setData(Qt::UserRole, i);
		// Make the type column read-only
		typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
		ui->containersTable->setItem(table_row, ContType, typeItem);

		table_row++;
	}

	if (m_currentContainerIndex >= 0 && m_currentContainerIndex < static_cast<int>(containers.size())) {
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

	ui->containerItemFilterLineEdit->clear();
	
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

	enableDisableControls();
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

	int row_count;
	if (is_list) {
		if (container.values_are_strings) {
			row_count = static_cast<int>(container.stringValues.size());
		} else {
			row_count = static_cast<int>(container.numberValues.size());
		}
	} else {
		row_count = static_cast<int>(container.keys.size());
	}
	ui->containerContentsTable->setRowCount(row_count);

	for (int i = 0; i < row_count; ++i) {
		const bool keys_are_strings = _model->getContainerKeyType(m_currentContainerIndex);
		const bool values_are_strings = _model->getContainerValueType(m_currentContainerIndex);

		if (is_list) {
			auto* item = new QTableWidgetItem(_model->getListItemValue(m_currentContainerIndex, i).c_str());
			item->setData(Qt::UserRole, i);
			item->setData(IsStringRole, values_are_strings);
			if (values_are_strings) {
				item->setData(MaxLength, TOKEN_LENGTH - 1);
			}
			ui->containerContentsTable->setItem(i, ItemKey, item);
		} else { // Is Map
			// Key Item
			auto* keyItem = new QTableWidgetItem(_model->getMapItemKey(m_currentContainerIndex, i).c_str());
			keyItem->setData(Qt::UserRole, i);
			keyItem->setData(IsStringRole, keys_are_strings);
			if (keys_are_strings) {
				keyItem->setData(MaxLength, TOKEN_LENGTH - 1);
			}
			ui->containerContentsTable->setItem(i, ItemKey, keyItem);

			// Value Item
			auto* valItem = new QTableWidgetItem(_model->getMapItemValue(m_currentContainerIndex, i).c_str());
			valItem->setData(Qt::UserRole, i);
			valItem->setData(IsStringRole, values_are_strings);
			if (values_are_strings) {
				valItem->setData(MaxLength, TOKEN_LENGTH - 1);
			}
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

// Technically not needed but keeps the pattern consistent
void VariableDialog::updateItemControls()
{
	util::SignalBlockers blockers(this);

	enableDisableControls();
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

	// Containers
	const bool is_container_selected = (m_currentContainerIndex != -1);

	ui->containerItemFilterLineEdit->setEnabled(is_container_selected);

	// Main container actions
	ui->copyContainerButton->setEnabled(is_container_selected);
	ui->deleteContainerButton->setEnabled(is_container_selected);
	ui->containerStructurePersistenceGroupBox->setEnabled(is_container_selected);

	// The group boxes for contents and types are only enabled if a container is selected.
	ui->containerContentsGroupBox->setEnabled(is_container_selected);
	ui->containerTypesGroupBox->setEnabled(is_container_selected);

	if (is_container_selected) {
		// Get properties of the selected container for more detailed logic
		const bool is_list = _model->getContainerType(m_currentContainerIndex);
		const bool is_empty = _model->isContainerEmpty(m_currentContainerIndex);
		const bool is_item_selected = (m_currentItemIndex != -1);
		const int item_count = ui->containerContentsTable->rowCount();

		// Persistence controls
		const bool is_persistent = _model->getContainerPersistenceType(m_currentContainerIndex) != 0;
		ui->setContainerAsEternalCheckbox->setEnabled(is_persistent);

		// Item action buttons
		ui->copyContainerItemButton->setEnabled(is_list && is_item_selected);
		ui->deleteContainerItemButton->setEnabled(is_item_selected);

		// List only buttons
		ui->shiftItemUpButton->setEnabled(is_list && is_item_selected && m_currentItemIndex > 0);
		ui->shiftItemDownButton->setEnabled(is_list && is_item_selected && m_currentItemIndex < item_count - 1);

		// Map only button
		ui->swapKeysAndValuesButton->setEnabled(!is_list && !is_empty);

		// Type switching is only allowed for empty containers to prevent data loss
		ui->containerBaseTypeGroupBox->setEnabled(is_empty);
		ui->containerKeyTypeGroupBox->setEnabled(!is_list && is_empty); // Keys are only for maps
		ui->containerDataTypeGroupBox->setEnabled(is_empty);
	}
}

QString VariableDialog::formatContainerTypeString(const fso::fred::dialogs::ContainerInfo& cont)
{
	QString type_str = cont.is_list ? "List" : "Map";
	QString value_type_str = cont.values_are_strings ? "String" : "Number";

	if (cont.is_list) {
		type_str += QString(" (%1)").arg(value_type_str);
	} else {
		QString key_type_str = cont.keys_are_strings ? "String" : "Number";
		type_str += QString(" (%1 Keys, %2 Values)").arg(key_type_str, value_type_str);
	}
	return type_str;
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

void VariableDialog::on_containerFilterLineEdit_textChanged(const QString& text)
{
	const int selected_row = ui->containersTable->currentRow();

	for (int i = 0; i < ui->containersTable->rowCount(); ++i) {
		auto* item = ui->containersTable->item(i, ContName);
		if (!item) {
			continue;
		}

		const bool match = item->text().contains(text, Qt::CaseInsensitive);
		ui->containersTable->setRowHidden(i, !match);
	}

	if (selected_row != -1 && ui->containersTable->isRowHidden(selected_row)) {
		ui->containersTable->clearSelection();
	}
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

			// After changing a key, we must refresh the entire item list because the sort order may have changed.
			updateItemList();

			// Now, find the new row index for the key we just edited.
			for (int i = 0; i < ui->containerContentsTable->rowCount(); ++i) {
				SCP_string key = ui->containerContentsTable->item(i, ItemKey)->text().toUtf8().constData();
				if (key == new_text) {
					// We found it, so select its new row and stop searching.
					ui->containerContentsTable->selectRow(i);
					break;
				}
			}
		} else if (column == ItemValue) {
			_model->setMapItemValue(m_currentContainerIndex, item_idx, new_text);
		}
	}
}

void VariableDialog::on_containerItemFilterLineEdit_textChanged(const QString& text)
{
	if (m_currentContainerIndex == -1) {
		return;
	}

	const int selected_row = ui->containerContentsTable->currentRow();
	const bool is_list = _model->getContainerType(m_currentContainerIndex);

	for (int i = 0; i < ui->containerContentsTable->rowCount(); ++i) {
		bool match = false;
		if (is_list) {
			auto* item = ui->containerContentsTable->item(i, ItemKey); // Value is in the "Key" column for lists
			if (item) {
				match = item->text().contains(text, Qt::CaseInsensitive);
			}
		} else { // Is a map, check both key and value
			auto* keyItem = ui->containerContentsTable->item(i, ItemKey);
			auto* valItem = ui->containerContentsTable->item(i, ItemValue);
			if (keyItem && keyItem->text().contains(text, Qt::CaseInsensitive)) {
				match = true;
			} else if (valItem && valItem->text().contains(text, Qt::CaseInsensitive)) {
				match = true;
			}
		}
		ui->containerContentsTable->setRowHidden(i, !match);
	}

	if (selected_row != -1 && ui->containerContentsTable->isRowHidden(selected_row)) {
		ui->containerContentsTable->clearSelection();
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

	// After updating, select the new item, which is now the last row.
	const int last_row = ui->containersTable->rowCount() - 1;
	if (last_row >= 0) {
		ui->containersTable->selectRow(last_row);
	}
}

void VariableDialog::on_copyContainerButton_clicked()
{
	// Ensure a container is actually selected
	if (m_currentContainerIndex == -1) {
		return;
	}

	_model->copyContainer(m_currentContainerIndex);

	updateUi();

	// After updating, select the new item, which is now the last row.
	const int last_row = ui->containersTable->rowCount() - 1;
	if (last_row >= 0) {
		ui->containersTable->selectRow(last_row);
	}
}

void VariableDialog::on_deleteContainerButton_clicked()
{
	if (m_currentContainerIndex == -1) {
		return;
	}

	const int row_to_delete = ui->containersTable->currentRow();

	// Deleting is a destructive action, so we must ask the user for confirmation first.
	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(this,
		"Confirm Deletion",
		"Are you sure you want to delete this container? This cannot be undone!",
		QMessageBox::Yes | QMessageBox::No);

	if (reply == QMessageBox::Yes) {
		_model->markContainerForDeletion(m_currentContainerIndex);
		updateUi();

		// After the UI is updated, intelligently select the next logical row.
		const int new_row_count = ui->containersTable->rowCount();
		if (new_row_count > 0) {
			// Select the item that took the deleted item's place, or the new last item.
			ui->containersTable->selectRow(std::min(row_to_delete, new_row_count - 1));
		}
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
		if (!_model->getContainerValueType(m_currentContainerIndex)) {
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

void VariableDialog::on_copyContainerItemButton_clicked()
{
	if (m_currentContainerIndex == -1 || m_currentItemIndex == -1) {
		return;
	}

	// The model duplicates the item and returns the index of the new copy.
	int new_item_index = _model->copyListItem(m_currentContainerIndex, m_currentItemIndex);
	if (new_item_index != -1) {
		// Set our selection tracker to this new index.
		m_currentItemIndex = new_item_index;
		// Refresh the item list, which will select the new row.
		updateItemList();
	}
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