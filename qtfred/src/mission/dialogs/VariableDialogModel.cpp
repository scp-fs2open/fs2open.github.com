#include "VariableDialogModel.h"

#include "parse/sexp.h"

#include <algorithm>
#include <climits>
#include <unordered_set>

namespace fso::fred::dialogs {

VariableDialogModel::VariableDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	initializeData();
}

void VariableDialogModel::reject()
{
	// Simply discard the working copies
	m_variables.clear();
	m_containers.clear();
}

bool VariableDialogModel::apply()
{
	if (!VariableDialogModel::checkValidity()) {
		return false;
	}
	
	// Apply Variable Changes
	SCP_vector<std::pair<int, SCP_string>> renamedVariables;

	// Loop through our new list of deleted variables and remove them from the global state.
	for (const auto& original_name : m_deleted_variables) {
		for (int i = 0; i < MAX_SEXP_VARIABLES; ++i) {
			if (!stricmp(Sexp_variables[i].variable_name, original_name.c_str())) {
				sexp_variable_delete(i);
				break;
			}
		}
	}

	for (const auto& var_info : m_variables) {
		bool found = false;
		// Existing variable that might be modified or deleted
		if (!var_info.originalName.empty()) {
			for (int i = 0; i < MAX_SEXP_VARIABLES; ++i) {
				if (!stricmp(Sexp_variables[i].variable_name, var_info.originalName.c_str())) {
					if (var_info.name != var_info.originalName) {
						renamedVariables.emplace_back(i, var_info.originalName);
					}
					strcpy_s(Sexp_variables[i].variable_name, var_info.name.c_str());
					Sexp_variables[i].type = var_info.flags;

					if (var_info.is_string) {
						strcpy_s(Sexp_variables[i].text, var_info.stringValue.c_str());
						Sexp_variables[i].type |= SEXP_VARIABLE_STRING;
					} else {
						strcpy_s(Sexp_variables[i].text, std::to_string(var_info.numberValue).c_str());
						Sexp_variables[i].type |= SEXP_VARIABLE_NUMBER;
					}
					found = true;
					break;
				}
			}
		}

		// New variable
		if (!found) {
			int flags = var_info.flags;
			const char* value_str;
			SCP_string num_val_str;

			if (var_info.is_string) {
				flags |= SEXP_VARIABLE_STRING;
				value_str = var_info.stringValue.c_str();
			} else {
				flags |= SEXP_VARIABLE_NUMBER;
				num_val_str = std::to_string(var_info.numberValue);
				value_str = num_val_str.c_str();
			}
			sexp_add_variable(value_str, var_info.name.c_str(), flags, -1);
		}
	}

	// Apply Container Changes
	SCP_vector<sexp_container> new_containers;
	SCP_unordered_map<SCP_string, SCP_string, SCP_string_lcase_hash, SCP_string_lcase_equal_to> renamed_containers;

	for (const auto& cont_info : m_containers) {
		new_containers.push_back(createSexpContainerFromInfo(cont_info));

		if (!cont_info.originalName.empty() && cont_info.name != cont_info.originalName) {
			renamed_containers[cont_info.originalName] = cont_info.name;
		}
	}

	update_sexp_containers(new_containers, renamed_containers);

	return true;
}

void VariableDialogModel::initializeData()
{
	m_variables.clear();
	m_containers.clear();

	// Load variables
	for (auto& var : Sexp_variables) {
		if (!(var.type & SEXP_VARIABLE_NOT_USED)) {
			auto& item = m_variables.emplace_back();
			item.name = var.variable_name;
			item.originalName = item.name;
			item.flags = var.type;

			if (var.type & SEXP_VARIABLE_STRING) {
				item.is_string = true;
				item.stringValue = var.text;
			} else {
				item.is_string = false;
				item.numberValue = atoi(var.text);
			}
		}
	}

	// Load containers
	const auto& containers = get_all_sexp_containers();
	for (const auto& container : containers) {
		auto& item = m_containers.emplace_back();
		item.name = container.container_name;
		item.originalName = item.name;
		item.flags = 0; // Flags are derived from the 'type' field in sexp_container

		item.is_list = container.is_list();
		item.values_are_strings = any(container.type & ContainerType::STRING_DATA);
		item.keys_are_strings = any(container.type & ContainerType::STRING_KEYS);

		// Reconstruct SEXP_VARIABLE flags for consistency
		if (any(container.type & ContainerType::NETWORK))
			item.flags |= SEXP_VARIABLE_NETWORK;
		if (any(container.type & ContainerType::SAVE_TO_PLAYER_FILE))
			item.flags |= SEXP_VARIABLE_SAVE_TO_PLAYER_FILE;
		if (any(container.type & ContainerType::SAVE_ON_MISSION_CLOSE))
			item.flags |= SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE;
		if (any(container.type & ContainerType::SAVE_ON_MISSION_PROGRESS))
			item.flags |= SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS;

		// Load data
		if (item.is_list) {
			if (item.values_are_strings) {
				for (const auto& val_str : container.list_data) {
					item.stringValues.push_back(val_str);
				}
			} else {
				for (const auto& val_str : container.list_data) {
					item.numberValues.push_back(atoi(val_str.c_str()));
				}
			}
		} else { // Is a map
			for (const auto& pair : container.map_data) {
				item.keys.push_back(pair.first);
				if (item.values_are_strings) {
					item.stringValues.push_back(pair.second);
				} else {
					item.numberValues.push_back(atoi(pair.second.c_str()));
				}
			}
		}
	}
}

void VariableDialogModel::sortMap(int containerIndex)
{
	if (!SCP_vector_inbounds(m_containers, containerIndex)) {
		return;
	}

	auto& cont = m_containers[containerIndex];
	if (cont.is_list || cont.keys.size() < 2) {
		return; // No need to sort
	}

	// 1. Create a list of indices (0, 1, 2, ...) that we can sort.
	std::vector<size_t> indices(cont.keys.size());
	std::iota(indices.begin(), indices.end(), 0);

	// 2. Sort the indices based on the key values they point to.
	std::sort(indices.begin(), indices.end(), [&](size_t a, size_t b) {
		if (cont.keys_are_strings) {
			return stricmp(cont.keys[a].c_str(), cont.keys[b].c_str()) < 0;
		} else {
			return atoi(cont.keys[a].c_str()) < atoi(cont.keys[b].c_str());
		}
	});

	// 3. Use the sorted indices to build newly ordered data vectors.
	SCP_vector<SCP_string> sorted_keys;
	SCP_vector<SCP_string> sorted_string_values;
	SCP_vector<int> sorted_number_values;

	sorted_keys.reserve(cont.keys.size());
	if (cont.values_are_strings) {
		sorted_string_values.reserve(cont.stringValues.size());
	} else {
		sorted_number_values.reserve(cont.numberValues.size());
	}

	for (size_t original_idx : indices) {
		sorted_keys.push_back(cont.keys[original_idx]);
		if (cont.values_are_strings) {
			sorted_string_values.push_back(cont.stringValues[original_idx]);
		} else {
			sorted_number_values.push_back(cont.numberValues[original_idx]);
		}
	}

	// 4. Move the new, correctly sorted data back into the container.
	cont.keys = std::move(sorted_keys);
	if (cont.values_are_strings) {
		cont.stringValues = std::move(sorted_string_values);
	} else {
		cont.numberValues = std::move(sorted_number_values);
	}
}

bool VariableDialogModel::checkValidity()
{
	SCP_string error_message;
	std::unordered_set<SCP_string> names_taken;

	// Validate Variables
	for (const auto& var : m_variables) {
		if (var.name.empty()) {
			error_message += "A variable has an empty name.\n";
			break; // One is enough
		}
		if (!names_taken.insert(var.name).second) {
			error_message += "Duplicate variable name found: \"" + var.name + "\"\n";
		}
	}

	if (m_variables.size() > MAX_SEXP_VARIABLES) {
		error_message += "There are more than the maximum of 250 variables.\n";
	}

	// Validate Containers
	names_taken.clear();
	for (const auto& cont : m_containers) {
		if (cont.name.empty()) {
			error_message += "A container has an empty name.\n";
			break;
		}
		if (!names_taken.insert(cont.name).second) {
			error_message += "Duplicate container name found: \"" + cont.name + "\"\n";
		}

		if (!cont.is_list) { // Map validation
			std::unordered_set<SCP_string> keys_taken;
			for (const auto& key : cont.keys) {
				if (key.empty()) {
					error_message += "Container \"" + cont.name + "\" has an empty key.\n";
				}
				if (!keys_taken.insert(key).second) {
					error_message += "Container \"" + cont.name + "\" has a duplicate key: \"" + key + "\"\n";
				}
				if (!cont.keys_are_strings && key != trimIntegerString(key)) {
					error_message += "Container \"" + cont.name + "\" has a non-numeric key: \"" + key + "\"\n";
				}
			}
		}
	}

	if (error_message.empty()) {
		return true;
	}

	_viewport->dialogProvider->showButtonDialog(DialogType::Error,
		"Error",
		"Please correct the following issues before saving:\n\n" + error_message,
		{DialogButton::Ok});

	return false;
}

const SCP_vector<VariableInfo>& VariableDialogModel::getVariables() const
{
	return m_variables;
}
const SCP_vector<ContainerInfo>& VariableDialogModel::getContainers() const
{
	return m_containers;
}

bool VariableDialogModel::variableListHasSpace() const
{
	return m_variables.size() < MAX_SEXP_VARIABLES;
}

void VariableDialogModel::addNewVariable()
{
	if (!variableListHasSpace()) {
		return; // No room for more variables
	}

	VariableInfo new_var;
	new_var.name = "newVar"; // Base name

	int count = 1;
	while (true) {
		SCP_string candidate_name = "newVar" + std::to_string(count);
		if (isVariableNameUnique(candidate_name, -1)) {
			new_var.name = candidate_name;
			break;
		}
		count++;
	}
	
	// add it
	m_variables.push_back(new_var);
	set_modified();
}

void VariableDialogModel::copyVariable(int index)
{
	if (!SCP_vector_inbounds(m_variables, index) || !variableListHasSpace()) {
		return;
	}

	// copy the original
	VariableInfo new_var_data = m_variables[index];
	new_var_data.originalName.clear();

	// Find a unique name for the copy
	int count = 1;
	const SCP_string base_name = new_var_data.name.substr(0, TOKEN_LENGTH - 5);
	while (true) {
		SCP_string candidate_name = base_name + "_" + std::to_string(count);
		if (isVariableNameUnique(candidate_name, -1)) {
			new_var_data.name = candidate_name;
			break;
		}
		count++;
	}

	// add it
	m_variables.push_back(new_var_data);
	set_modified();
}

void VariableDialogModel::markVariableForDeletion(int index)
{
	if (!SCP_vector_inbounds(m_variables, index)) {
		return;
	}

	const auto& var_to_delete = m_variables[index];

	// If the variable had an original name, it's a real variable that
	// needs to be deleted from the global state on apply.
	if (!var_to_delete.originalName.empty()) {
		// Check if the name is already in our list of deleted variables.
		// This really shouldn't be possible but let's be safe.
		auto it = std::find(m_deleted_variables.begin(), m_deleted_variables.end(), var_to_delete.originalName);

		if (it == m_deleted_variables.end()) {
			m_deleted_variables.push_back(var_to_delete.originalName);
		}
	}

	// Now, remove it from the active list.
	m_variables.erase(m_variables.begin() + index);
	set_modified();
}

bool VariableDialogModel::isVariableNameUnique(const SCP_string& name, int variableIndex) const
{
	for (int i = 0; i < static_cast<int>(m_variables.size()); ++i) {
		// Don't compare the variable against itself
		if (i == variableIndex) {
			continue;
		}

		const auto& var = m_variables[i];

		// If we find a case-insensitive match, the name is not unique
		if (!stricmp(var.name.c_str(), name.c_str())) {
			return false;
		}
	}

	return true; // No conflicts found
}

SCP_string VariableDialogModel::getVariableName(int index) const
{
	if (SCP_vector_inbounds(m_variables, index)) {
		return m_variables[index].name;
	}
	return "";
}

void VariableDialogModel::setVariableName(int index, const SCP_string& newName)
{
	if (SCP_vector_inbounds(m_variables, index)) {
		SCP_string truncated_name = newName.substr(0, TOKEN_LENGTH - 1);

		if (isVariableNameUnique(truncated_name, index)) {
			modify(m_variables[index].name, truncated_name);
		}
	}
}

// true for string, false for number
bool VariableDialogModel::getVariableType(int index) const
{
	if (SCP_vector_inbounds(m_variables, index)) {
		return m_variables[index].is_string;
	}
	return true; // Default to string if out of bounds
}

void VariableDialogModel::setVariableType(int index, bool is_string)
{
	if (!SCP_vector_inbounds(m_variables, index)) {
		return;
	}

	auto& var = m_variables[index];
	if (var.is_string == is_string) {
		return; // No change needed
	}

	// When changing type, attempt a best-effort conversion.
	// The original value is preserved in its respective member variable
	// in case the user toggles the type back and forth.
	if (is_string) {
		// Converting from Number to String
		modify(var.stringValue, std::to_string(var.numberValue));
	} else {
		// Converting from String to Number
		modify(var.numberValue, atoi(var.stringValue.c_str()));
	}

	modify(var.is_string, is_string);
}

SCP_string VariableDialogModel::getVariableValue(int index) const
{
	if (SCP_vector_inbounds(m_variables, index)) {
		const auto& var = m_variables[index];
		if (var.is_string) {
			return var.stringValue;
		} else {
			return std::to_string(var.numberValue);
		}
	}
	return "";
}

void VariableDialogModel::setVariableValue(int index, const SCP_string& value)
{
	if (!SCP_vector_inbounds(m_variables, index)) {
		return;
	}

	auto& var = m_variables[index];
	if (var.is_string) {
		SCP_string truncated_name = value.substr(0, TOKEN_LENGTH - 1);
		modify(var.stringValue, truncated_name);
	} else {
		// Use our helper to sanitize the input before converting to an integer
		modify(var.numberValue, atoi(trimIntegerString(value).c_str()));
	}
}

bool VariableDialogModel::getVariableNetwork(int index) const
{
	if (SCP_vector_inbounds(m_variables, index)) {
		return (m_variables[index].flags & SEXP_VARIABLE_NETWORK) != 0;
	}
	return false;
}

void VariableDialogModel::setVariableNetwork(int index, bool enabled)
{
	if (!SCP_vector_inbounds(m_variables, index)) {
		return;
	}

	int flags = m_variables[index].flags;

	if (enabled) {
		flags |= SEXP_VARIABLE_NETWORK;
	} else {
		flags &= ~SEXP_VARIABLE_NETWORK;
	}
	modify(m_variables[index].flags, flags);
}

bool VariableDialogModel::getVariableEternal(int index) const
{
	if (SCP_vector_inbounds(m_variables, index)) {
		return (m_variables[index].flags & SEXP_VARIABLE_SAVE_TO_PLAYER_FILE) != 0;
	}
	return false;
}

void VariableDialogModel::setVariableEternal(int index, bool enabled)
{
	if (!SCP_vector_inbounds(m_variables, index)) {
		return;
	}

	int flags = m_variables[index].flags;

	if (enabled) {
		// Based on original FRED, Eternal only works if a persistence type is also set
		if (flags & (SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE | SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS)) {
			flags |= SEXP_VARIABLE_SAVE_TO_PLAYER_FILE;
		}
	} else {
		flags &= ~SEXP_VARIABLE_SAVE_TO_PLAYER_FILE;
	}
	modify(m_variables[index].flags, flags);
}

// Returns 0 for "None", 1 for "Campaign/Progress", 2 for "Player/Close"
// This really should be a flag enum but to do that properly I think we'd need
// to refactor the sexp variables flags defines as well and that's too much for now.
int VariableDialogModel::getVariablePersistenceType(int index) const
{
	if (SCP_vector_inbounds(m_variables, index)) {
		int flags = m_variables[index].flags;
		if (flags & SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE) {
			return 2; // "Player" or "Mission Close"
		}
		if (flags & SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS) {
			return 1; // "Campaign" or "Mission Progress"
		}
	}
	return 0; // "None"
}

void VariableDialogModel::setVariablePersistenceType(int index, int type)
{
	if (!SCP_vector_inbounds(m_variables, index)) {
		return;
	}

	auto flags = m_variables[index].flags;

	// These are mutually exclusive, so we always clear both bits first,
	// then set the one we want.
	flags &= ~(SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS | SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE);

	switch (type) {
		case 1: // Campaign / Progress
			flags |= SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS;
			break;
		case 2: // Player / Close
			flags |= SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE;
			break;
		case 0: // None
		default:
			// Flags are already cleared. Also, if "None" is selected, "Eternal" must be off.
			flags &= ~SEXP_VARIABLE_SAVE_TO_PLAYER_FILE;
			break;
	}
	modify(m_variables[index].flags, flags);
}

void VariableDialogModel::addNewContainer()
{
	ContainerInfo new_cont;

	// Find a unique name
	int count = 1;
	while (true) {
		SCP_string candidate_name = "newContainer" + std::to_string(count);
		if (isContainerNameUnique(candidate_name, -1)) {
			new_cont.name = candidate_name;
			break;
		}
		count++;
	}

	m_containers.push_back(new_cont);
	set_modified();
}

void VariableDialogModel::copyContainer(int index)
{
	if (!SCP_vector_inbounds(m_containers, index)) {
		return;
	}

	ContainerInfo new_cont_data = m_containers[index];
	new_cont_data.originalName.clear();

	// Find a unique name for the copy
	int count = 1;
	const SCP_string base_name = new_cont_data.name.substr(0, TOKEN_LENGTH - 5);
	while (true) {
		SCP_string candidate_name = base_name + "_" + std::to_string(count);
		if (isContainerNameUnique(candidate_name, -1)) {
			new_cont_data.name = candidate_name;
			break;
		}
		count++;
	}

	m_containers.push_back(new_cont_data);
	set_modified();
}

void VariableDialogModel::markContainerForDeletion(int index)
{
	if (!SCP_vector_inbounds(m_containers, index)) {
		return;
	}

	// Just remove it from the active list. The replacement logic in apply() handles the rest.
	m_containers.erase(m_containers.begin() + index);
	set_modified();
}

bool VariableDialogModel::isContainerNameUnique(const SCP_string& name, int containerIndex) const
{
	for (int i = 0; i < static_cast<int>(m_containers.size()); ++i) {
		// Don't compare the container against itself
		if (i == containerIndex) {
			continue;
		}

		// If we find a case-insensitive match, the name is not unique
		if (!stricmp(m_containers[i].name.c_str(), name.c_str())) {
			return false;
		}
	}
	return true; // No conflicts found
}

bool VariableDialogModel::isContainerEmpty(int index) const
{
	if (!SCP_vector_inbounds(m_containers, index)) {
		return true; // An invalid container is effectively empty
	}

	const auto& cont = m_containers[index];

	if (cont.is_list) {
		// A list is empty if both of its potential value vectors are empty.
		// We check both in case the user has toggled the type back and forth.
		return cont.stringValues.empty() && cont.numberValues.empty();
	} else {
		return cont.keys.empty();
	}
}

SCP_string VariableDialogModel::getContainerName(int index) const
{
	if (SCP_vector_inbounds(m_containers, index)) {
		return m_containers[index].name;
	}
	return "";
}

void VariableDialogModel::setContainerName(int index, const SCP_string& newName)
{
	if (!SCP_vector_inbounds(m_containers, index)) {
		return;
	}

	// Ensure name does not exceed the engine's limit
	modify(m_containers[index].name, newName.substr(0, TOKEN_LENGTH - 1));
}

// true = List, false = Map
bool VariableDialogModel::getContainerType(int index) const
{
	if (SCP_vector_inbounds(m_containers, index)) {
		return m_containers[index].is_list;
	}
	return true; // Default to list if out of bounds
}

void VariableDialogModel::setContainerType(int index, bool is_list)
{
	if (!SCP_vector_inbounds(m_containers, index)) {
		return;
	}

	auto& cont = m_containers[index];
	if (cont.is_list == is_list) {
		return; // No change
	}

	// The UI is responsible for warning the user about potential data changes.
	// The model simply performs a reasonable default conversion.
	if (is_list) {
		// Converting from Map to List: Values are kept, keys are discarded.
		// No data movement is needed, just changing the flag.
	} else {
		// Converting from List to Map: List items become values, default keys are generated.
		cont.keys.clear();
		size_t num_items = cont.values_are_strings ? cont.stringValues.size() : cont.numberValues.size();
		for (size_t i = 0; i < num_items; ++i) {
			cont.keys.push_back(std::to_string(i));
		}
	}

	modify(cont.is_list, is_list);
}

// true = String data/keys, false = Number data/keys
bool VariableDialogModel::getContainerValueType(int index) const
{
	if (SCP_vector_inbounds(m_containers, index)) {
		return m_containers[index].values_are_strings;
	}
	return true; // Default to string if out of bounds
}

void VariableDialogModel::setContainerValueType(int index, bool values_are_strings)
{
	if (!SCP_vector_inbounds(m_containers, index)) {
		return;
	}

	auto& cont = m_containers[index];
	if (cont.values_are_strings == values_are_strings) {
		return; // No change
	}

	// Attempt a best-effort conversion, preserving the original data
	// in case the user toggles the type back.
	if (values_are_strings) {
		// Converting from Number to String values
		cont.stringValues.clear();
		for (int val : cont.numberValues) {
			cont.stringValues.push_back(std::to_string(val));
		}
	} else {
		// Converting from String to Number values
		cont.numberValues.clear();
		for (const auto& val : cont.stringValues) {
			cont.numberValues.push_back(atoi(val.c_str()));
		}
	}

	modify(cont.values_are_strings, values_are_strings);
}

// true = String keys, false = Number keys (maps only)
bool VariableDialogModel::getContainerKeyType(int index) const
{
	if (SCP_vector_inbounds(m_containers, index)) {
		return m_containers[index].keys_are_strings;
	}
	return true; // Default to string keys if out of bounds
}

void VariableDialogModel::setContainerKeyType(int index, bool keys_are_strings)
{
	if (!SCP_vector_inbounds(m_containers, index)) {
		return;
	}

	auto& cont = m_containers[index];
	if (cont.is_list || cont.keys_are_strings == keys_are_strings) {
		return; // No change needed
	}

	// This is just a flag change. The checkValidity() function will enforce
	// that keys are numeric if keys_are_strings is false.
	modify(cont.keys_are_strings, keys_are_strings);
}

bool VariableDialogModel::getContainerNetwork(int index) const
{
	if (SCP_vector_inbounds(m_containers, index)) {
		return (m_containers[index].flags & SEXP_VARIABLE_NETWORK) != 0;
	}
	return false;
}

void VariableDialogModel::setContainerNetwork(int index, bool enabled)
{
	if (!SCP_vector_inbounds(m_containers, index)) {
		return;
	}

	int flags = m_containers[index].flags;
	if (enabled) {
		flags |= SEXP_VARIABLE_NETWORK;
	} else {
		flags &= ~SEXP_VARIABLE_NETWORK;
	}
	modify(m_containers[index].flags, flags);
}

bool VariableDialogModel::getContainerEternal(int index) const
{
	if (SCP_vector_inbounds(m_containers, index)) {
		return (m_containers[index].flags & SEXP_VARIABLE_SAVE_TO_PLAYER_FILE) != 0;
	}
	return false;
}

void VariableDialogModel::setContainerEternal(int index, bool enabled)
{
	if (!SCP_vector_inbounds(m_containers, index)) {
		return;
	}

	int flags = m_containers[index].flags;
	if (enabled) {
		// Eternal only works if a persistence type is also set
		if (flags & (SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE | SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS)) {
			flags |= SEXP_VARIABLE_SAVE_TO_PLAYER_FILE;
		}
	} else {
		flags &= ~SEXP_VARIABLE_SAVE_TO_PLAYER_FILE;
	}
	modify(m_containers[index].flags, flags);
}

// Returns 0 for "None", 1 for "Campaign/Progress", 2 for "Player/Close"
// Again this should probably return an enum but that's a bigger refactor
// of the sexp variable flags than I want to do right now.
int VariableDialogModel::getContainerPersistenceType(int index) const
{
	if (SCP_vector_inbounds(m_containers, index)) {
		int flags = m_containers[index].flags;
		if (flags & SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE) {
			return 2; // "Player" or "Mission Close"
		}
		if (flags & SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS) {
			return 1; // "Campaign" or "Mission Progress"
		}
	}
	return 0; // "None"
}

void VariableDialogModel::setContainerPersistenceType(int index, int type)
{
	if (!SCP_vector_inbounds(m_containers, index)) {
		return;
	}

	auto flags = m_containers[index].flags;

	// These are mutually exclusive, so we clear both bits first
	flags &= ~(SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS | SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE);

	switch (type) {
	case 1: // Campaign / Progress
		flags |= SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS;
		break;
	case 2: // Player / Close
		flags |= SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE;
		break;
	case 0: // None
	default:
		// If persistence is set to "None", the "Eternal" flag must also be cleared.
		flags &= ~SEXP_VARIABLE_SAVE_TO_PLAYER_FILE;
		break;
	}
	modify(m_containers[index].flags, flags);
}

void VariableDialogModel::addListItem(int containerIndex)
{
	if (!SCP_vector_inbounds(m_containers, containerIndex)) {
		return;
	}

	auto& cont = m_containers[containerIndex];
	if (!cont.is_list) {
		return; // Can only add list items to a list
	}

	if (cont.values_are_strings) {
		cont.stringValues.emplace_back("New Item");
	} else {
		cont.numberValues.push_back(0);
	}

	set_modified();
}

void VariableDialogModel::removeListItem(int containerIndex, int itemIndex)
{
	if (!SCP_vector_inbounds(m_containers, containerIndex)) {
		return;
	}

	auto& cont = m_containers[containerIndex];
	if (!cont.is_list) {
		return;
	}

	if (cont.values_are_strings) {
		if (SCP_vector_inbounds(cont.stringValues, itemIndex)) {
			cont.stringValues.erase(cont.stringValues.begin() + itemIndex);
			set_modified();
		}
	} else {
		if (SCP_vector_inbounds(cont.numberValues, itemIndex)) {
			cont.numberValues.erase(cont.numberValues.begin() + itemIndex);
			set_modified();
		}
	}
}

SCP_string VariableDialogModel::getListItemValue(int containerIndex, int itemIndex) const
{
	if (!SCP_vector_inbounds(m_containers, containerIndex)) {
		return "";
	}
	const auto& cont = m_containers[containerIndex];
	if (!cont.is_list) {
		return "";
	}
	if (cont.values_are_strings) {
		if (SCP_vector_inbounds(cont.stringValues, itemIndex)) {
			return cont.stringValues[itemIndex];
		}
	} else {
		if (SCP_vector_inbounds(cont.numberValues, itemIndex)) {
			return std::to_string(cont.numberValues[itemIndex]);
		}
	}
	return "";
}

void VariableDialogModel::setListItemValue(int containerIndex, int itemIndex, const SCP_string& value)
{
	if (!SCP_vector_inbounds(m_containers, containerIndex)) {
		return;
	}

	auto& cont = m_containers[containerIndex];
	if (!cont.is_list) {
		return;
	}

	if (cont.values_are_strings) {
		if (SCP_vector_inbounds(cont.stringValues, itemIndex)) {
			modify(cont.stringValues[itemIndex], value.substr(0, TOKEN_LENGTH - 1));
		}
	} else {
		if (SCP_vector_inbounds(cont.numberValues, itemIndex)) {
			modify(cont.numberValues[itemIndex], atoi(trimIntegerString(value).c_str()));
		}
	}
}

void VariableDialogModel::moveListItem(int containerIndex, int itemIndex, bool up)
{
	if (!SCP_vector_inbounds(m_containers, containerIndex)) {
		return;
	}

	auto& cont = m_containers[containerIndex];
	if (!cont.is_list) {
		return;
	}

	int targetIndex = up ? itemIndex - 1 : itemIndex + 1;

	if (cont.values_are_strings) {
		if (SCP_vector_inbounds(cont.stringValues, itemIndex) && SCP_vector_inbounds(cont.stringValues, targetIndex)) {
			std::swap(cont.stringValues[itemIndex], cont.stringValues[targetIndex]);
			set_modified();
		}
	} else {
		if (SCP_vector_inbounds(cont.numberValues, itemIndex) && SCP_vector_inbounds(cont.numberValues, targetIndex)) {
			std::swap(cont.numberValues[itemIndex], cont.numberValues[targetIndex]);
			set_modified();
		}
	}
}

int VariableDialogModel::copyListItem(int containerIndex, int itemIndex)
{
	if (!SCP_vector_inbounds(m_containers, containerIndex)) {
		return -1;
	}

	auto& cont = m_containers[containerIndex];
	if (!cont.is_list) {
		return -1;
	}

	const int new_index = itemIndex + 1;

	if (cont.values_are_strings) {
		if (SCP_vector_inbounds(cont.stringValues, itemIndex)) {
			cont.stringValues.insert(cont.stringValues.begin() + new_index, cont.stringValues[itemIndex]);
			set_modified();
			return new_index;
		}
	} else {
		if (SCP_vector_inbounds(cont.numberValues, itemIndex)) {
			cont.numberValues.insert(cont.numberValues.begin() + new_index, cont.numberValues[itemIndex]);
			set_modified();
			return new_index;
		}
	}

	return -1;
}

void VariableDialogModel::addMapItem(int containerIndex)
{
	if (SCP_vector_inbounds(m_containers, containerIndex)) {
		auto& container = m_containers[containerIndex];
		if (container.is_list)
			return;

		// Find a unique key
		SCP_string new_key_base = container.keys_are_strings ? "newKey" : "";
		int count = 0;
		SCP_string final_key;
		bool key_taken;

		do {
			key_taken = false;
			final_key = new_key_base + std::to_string(count);
			for (const auto& key : container.keys) {
				if (key == final_key) {
					key_taken = true;
					count++;
					break;
				}
			}
		} while (key_taken);

		container.keys.push_back(final_key);
		if (container.values_are_strings) {
			container.stringValues.emplace_back("");
		} else {
			container.numberValues.push_back(0);
		}

		sortMap(containerIndex);
		set_modified();
	}
}

void VariableDialogModel::removeMapItem(int containerIndex, int itemIndex)
{
	if (!SCP_vector_inbounds(m_containers, containerIndex)) {
		return;
	}

	auto& cont = m_containers[containerIndex];
	if (cont.is_list || !SCP_vector_inbounds(cont.keys, itemIndex)) {
		return;
	}

	// Remove the key first
	cont.keys.erase(cont.keys.begin() + itemIndex);

	// Then remove the corresponding value from the active value vector
	if (cont.values_are_strings) {
		if (itemIndex < static_cast<int>(cont.stringValues.size())) {
			cont.stringValues.erase(cont.stringValues.begin() + itemIndex);
		}
	} else {
		if (itemIndex < static_cast<int>(cont.numberValues.size())) {
			cont.numberValues.erase(cont.numberValues.begin() + itemIndex);
		}
	}

	set_modified();
}

SCP_string VariableDialogModel::getMapItemKey(int containerIndex, int itemIndex) const
{
	if (!SCP_vector_inbounds(m_containers, containerIndex)) {
		return "";
	}
	const auto& cont = m_containers[containerIndex];
	if (cont.is_list || !SCP_vector_inbounds(cont.keys, itemIndex)) {
		return "";
	}
	return cont.keys[itemIndex];
}

void VariableDialogModel::setMapItemKey(int containerIndex, int itemIndex, const SCP_string& key)
{
	if (!SCP_vector_inbounds(m_containers, containerIndex)) {
		return;
	}

	auto& cont = m_containers[containerIndex];
	if (cont.is_list || !SCP_vector_inbounds(cont.keys, itemIndex)) {
		return;
	}

	SCP_string sanitized_key = key.substr(0, TOKEN_LENGTH - 1);
	if (!cont.keys_are_strings) {
		sanitized_key = trimIntegerString(sanitized_key);
	}

	modify(cont.keys[itemIndex], sanitized_key);
	sortMap(containerIndex);
}

SCP_string VariableDialogModel::getMapItemValue(int containerIndex, int itemIndex) const
{
	if (!SCP_vector_inbounds(m_containers, containerIndex)) {
		return "";
	}
	const auto& cont = m_containers[containerIndex];
	if (cont.is_list || !SCP_vector_inbounds(cont.keys, itemIndex)) {
		return "";
	}
	if (cont.values_are_strings) {
		if (itemIndex < static_cast<int>(cont.stringValues.size())) {
			return cont.stringValues[itemIndex];
		}
	} else {
		if (itemIndex < static_cast<int>(cont.numberValues.size())) {
			return std::to_string(cont.numberValues[itemIndex]);
		}
	}
	return "";
}

void VariableDialogModel::setMapItemValue(int containerIndex, int itemIndex, const SCP_string& value)
{
	if (!SCP_vector_inbounds(m_containers, containerIndex)) {
		return;
	}

	auto& cont = m_containers[containerIndex];
	if (cont.is_list) {
		return;
	}

	if (cont.values_are_strings) {
		if (SCP_vector_inbounds(cont.stringValues, itemIndex)) {
			modify(cont.stringValues[itemIndex], value.substr(0, TOKEN_LENGTH - 1));
		}
	} else {
		if (SCP_vector_inbounds(cont.numberValues, itemIndex)) {
			modify(cont.numberValues[itemIndex], atoi(trimIntegerString(value).c_str()));
		}
	}
}

void VariableDialogModel::swapMapKeysAndValues(int containerIndex)
{
	if (!SCP_vector_inbounds(m_containers, containerIndex)) {
		return;
	}

	auto& cont = m_containers[containerIndex];
	if (cont.is_list) {
		return;
	}

	// Create a temporary vector of strings from the current values
	SCP_vector<SCP_string> temp_values_as_strings;
	if (cont.values_are_strings) {
		temp_values_as_strings = cont.stringValues;
	} else {
		for (int val : cont.numberValues) {
			temp_values_as_strings.push_back(std::to_string(val));
		}
	}

	// Create a temporary vector of the appropriate type from the current keys
	if (cont.keys_are_strings) {
		cont.stringValues = cont.keys;
	} else {
		cont.numberValues.clear();
		for (const auto& key : cont.keys) {
			cont.numberValues.push_back(atoi(key.c_str()));
		}
	}

	// Move the temp values into the keys vector
	cont.keys = std::move(temp_values_as_strings);

	// Swap the type flags
	std::swap(cont.keys_are_strings, cont.values_are_strings);

	set_modified();
}

sexp_container VariableDialogModel::createSexpContainerFromInfo(const ContainerInfo& info)
{
	sexp_container sc;
	sc.container_name = info.name;

	// Set type flags
	sc.type = static_cast<ContainerType>(0); // Start fresh
	if (info.is_list)
		sc.type |= ContainerType::LIST;
	else
		sc.type |= ContainerType::MAP;

	if (info.values_are_strings)
		sc.type |= ContainerType::STRING_DATA;
	else
		sc.type |= ContainerType::NUMBER_DATA;

	if (info.keys_are_strings)
		sc.type |= ContainerType::STRING_KEYS;
	else
		sc.type |= ContainerType::NUMBER_KEYS;

	if (info.flags & SEXP_VARIABLE_NETWORK)
		sc.type |= ContainerType::NETWORK;
	if (info.flags & SEXP_VARIABLE_SAVE_TO_PLAYER_FILE)
		sc.type |= ContainerType::SAVE_TO_PLAYER_FILE;
	if (info.flags & SEXP_VARIABLE_SAVE_ON_MISSION_CLOSE)
		sc.type |= ContainerType::SAVE_ON_MISSION_CLOSE;
	if (info.flags & SEXP_VARIABLE_SAVE_ON_MISSION_PROGRESS)
		sc.type |= ContainerType::SAVE_ON_MISSION_PROGRESS;

	// Populate data
	if (info.is_list) {
		if (info.values_are_strings) {
			for (const auto& val_str : info.stringValues) {
				sc.list_data.push_back(val_str);
			}
		} else {
			for (int val : info.numberValues) {
				sc.list_data.push_back(std::to_string(val));
			}
		}
	} else { // is map
		for (size_t i = 0; i < info.keys.size(); ++i) {
			if (info.values_are_strings) {
				sc.map_data[info.keys[i]] = info.stringValues[i];
			} else {
				sc.map_data[info.keys[i]] = std::to_string(info.numberValues[i]);
			}
		}
	}

	return sc;
}

SCP_string VariableDialogModel::trimIntegerString(const SCP_string& source)
{
	SCP_string result;
	result.reserve(source.length());

	// Filter the string to only include valid integer characters.
	// We use a flag to handle leading zeros correctly.
	bool non_zero_found = false;
	for (char c : source) {
		if (isdigit(c)) {
			if (c == '0' && !non_zero_found && !result.empty() && result.back() != '-') {
				continue; // Skip leading zeros
			}
			result += c;
			if (c != '0') {
				non_zero_found = true;
			}
		} else if (c == '-' && result.empty()) {
			result += c; // Allow a leading negative sign
		}
	}

	// Handle edge cases
	if (result.empty() || result == "-") {
		return "0";
	}

	// Check for and clamp overflow values
	try {
		long long val = std::stoll(result);
		if (val > INT_MAX)
			return std::to_string(INT_MAX);
		if (val < INT_MIN)
			return std::to_string(INT_MIN);
	} catch (const std::out_of_range&) {
		// String represents a number too large for stoll
		if (result[0] == '-') {
			return std::to_string(INT_MIN);
		} else {
			return std::to_string(INT_MAX);
		}
	}

	return result;
}

} // namespace fso::fred::dialogs