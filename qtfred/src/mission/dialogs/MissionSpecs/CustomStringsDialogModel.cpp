#include "CustomStringsDialogModel.h"

namespace fso::fred::dialogs {

CustomStringsDialogModel::CustomStringsDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
	
}

bool CustomStringsDialogModel::apply()
{
	// No direct application; this model is used to collect custom strings
	// and the actual application is handled by the MissionSpecDialogModel.
	return true;
}

void CustomStringsDialogModel::reject()
{
	// No direct rejection; this model is used to collect custom strings
	// and the actual rejection is handled by the MissionSpecDialogModel.
}

void CustomStringsDialogModel::setInitial(const SCP_vector<custom_string>& in)
{
	_items = in;
}

bool CustomStringsDialogModel::add(const custom_string& e, SCP_string* errorOut)
{
	// validation
	if (!validateKeySyntax(e.name, errorOut))
		return false;
	if (!validateValue(e.value, errorOut))
		return false;
	if (!validateText(e.text, errorOut))
		return false;

	if (!keyIsUnique(e.name)) {
		if (errorOut)
			*errorOut = "Key must be unique.";
		return false;
	}

	_items.push_back(e);
	set_modified();
	return true;
}

bool CustomStringsDialogModel::updateAt(size_t index, const custom_string& e, SCP_string* errorOut)
{
	if (index >= _items.size()) {
		if (errorOut)
			*errorOut = "Invalid index.";
		return false;
	}
	// validation
	if (!validateKeySyntax(e.name, errorOut))
		return false;
	if (!validateValue(e.value, errorOut))
		return false;
	if (!validateText(e.text, errorOut))
		return false;

	if (!keyIsUnique(e.name, index)) {
		if (errorOut)
			*errorOut = "Key must be unique.";
		return false;
	}

	if (_items[index].name == e.name && _items[index].value == e.value && _items[index].text == e.text) {
		return true; // no change
	}

	_items[index] = e;
	set_modified();
	return true;
}

bool CustomStringsDialogModel::removeAt(size_t index)
{
	if (index >= _items.size())
		return false;
	_items.erase(_items.begin() + index);
	set_modified();
	return true;
}

bool CustomStringsDialogModel::hasKey(const SCP_string& key) const
{
	return std::any_of(_items.begin(), _items.end(), [&](const custom_string& it) {
		return stricmp(it.name.c_str(), key.c_str()) == 0;
	});
}

std::optional<size_t> CustomStringsDialogModel::indexOfKey(const SCP_string& key) const
{
	for (size_t i = 0; i < _items.size(); ++i) {
		if (stricmp(_items[i].name.c_str(), key.c_str()) == 0)
			return i;
	}
	return std::nullopt;
}

bool CustomStringsDialogModel::validateKeySyntax(const SCP_string& key, SCP_string* errorOut)
{
	if (key.empty()) {
		if (errorOut)
			*errorOut = "Key cannot be empty.";
		return false;
	}
	// No whitespace allowed
	if (key.find_first_of(" \t\r\n") != SCP_string::npos) {
		if (errorOut)
			*errorOut = "Key cannot contain whitespace.";
		return false;
	}
	return true;
}

bool CustomStringsDialogModel::validateValue(const SCP_string& value, SCP_string* errorOut)
{
	if (value.empty()) {
		if (errorOut)
			*errorOut = "Value cannot be empty.";
		return false;
	}
	return true;
}

bool CustomStringsDialogModel::validateText(const SCP_string& text, SCP_string* errorOut)
{
	if (text.empty()) {
		if (errorOut)
			*errorOut = "Text cannot be empty.";
		return false;
	}
	return true;
}

bool CustomStringsDialogModel::keyIsUnique(const SCP_string& key, std::optional<size_t> ignoreIndex) const
{
	for (size_t i = 0; i < _items.size(); ++i) {
		if (ignoreIndex && *ignoreIndex == i)
			continue;
		if (stricmp(_items[i].name.c_str(), key.c_str()) == 0)
			return false;
	}
	return true;
}

} // namespace fso::fred::dialogs
