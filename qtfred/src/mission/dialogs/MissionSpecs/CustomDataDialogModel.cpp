#include "CustomDataDialogModel.h"

using namespace fso::fred::dialogs;

CustomDataDialogModel::CustomDataDialogModel(QObject* parent, EditorViewport* viewport)
	: AbstractDialogModel(parent, viewport)
{
}

bool CustomDataDialogModel::apply()
{
	// No direct application; this model is used to collect custom strings
	// and the actual application is handled by the MissionSpecDialogModel.
	return true;
}

void CustomDataDialogModel::reject()
{
	// No direct rejection; this model is used to collect custom strings
	// and the actual rejection is handled by the MissionSpecDialogModel.
}

void CustomDataDialogModel::setInitial(const SCP_map<SCP_string, SCP_string>& in)
{
	_items = in;
}

bool CustomDataDialogModel::add(const std::pair<SCP_string, SCP_string>& e, SCP_string* errorOut)
{
	// validation
	if (!validateKeySyntax(e.first, errorOut))
		return false;
	if (!validateValue(e.second, errorOut))
		return false;

	if (!keyIsUnique(e.first)) {
		if (errorOut)
			*errorOut = "Key must be unique.";
		return false;
	}

	_items.emplace(e);
	set_modified();
	return true;
}

static inline bool
advance_to_index(SCP_map<SCP_string, SCP_string>& m, size_t index, SCP_map<SCP_string, SCP_string>::iterator& out)
{
	if (index >= m.size())
		return false;
	out = m.begin();
	std::advance(out, static_cast<long>(index));
	return true;
}

bool CustomDataDialogModel::updateAt(size_t index, const std::pair<SCP_string, SCP_string>& e, SCP_string* errorOut)
{
	// Bounds check
	SCP_map<SCP_string, SCP_string>::iterator it;
	if (!advance_to_index(_items, index, it)) {
		if (errorOut)
			*errorOut = "Invalid index.";
		return false;
	}

	// validation
	if (!validateKeySyntax(e.first, errorOut))
		return false;
	if (!validateValue(e.second, errorOut))
		return false;

	// uniqueness (case-insensitive) ignoring this index
	if (!keyIsUnique(e.first, index)) {
		if (errorOut)
			*errorOut = "Key must be unique.";
		return false;
	}

	// No change?
	if (stricmp(it->first.c_str(), e.first.c_str()) == 0 && it->second == e.second) {
		return true; // no change
	}

	// If the key is unchanged (case-insensitive), just update the value
	if (stricmp(it->first.c_str(), e.first.c_str()) == 0) {
		if (it->second != e.second) {
			it->second = e.second;
			set_modified();
		}
		return true;
	}

	// Key changed: erase old and insert new pair
	_items.erase(it);
	_items.emplace(e);
	set_modified();
	return true;
}

bool CustomDataDialogModel::removeAt(size_t index)
{
	SCP_map<SCP_string, SCP_string>::iterator it;
	if (!advance_to_index(_items, index, it))
		return false;

	_items.erase(it);
	set_modified();
	return true;
}

bool CustomDataDialogModel::hasKey(const SCP_string& key) const
{
	return std::any_of(_items.begin(), _items.end(), [&key](const auto& kv) {
		return stricmp(kv.first.c_str(), key.c_str()) == 0;
	});
}

std::optional<size_t> CustomDataDialogModel::indexOfKey(const SCP_string& key) const
{
	size_t i = 0;
	for (const auto& kv : _items) {
		if (stricmp(kv.first.c_str(), key.c_str()) == 0)
			return i;
		++i;
	}
	return std::nullopt;
}

bool CustomDataDialogModel::validateKeySyntax(const SCP_string& key, SCP_string* errorOut)
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

bool CustomDataDialogModel::validateValue(const SCP_string& value, SCP_string* errorOut)
{
	if (value.empty()) {
		if (errorOut)
			*errorOut = "Value cannot be empty.";
		return false;
	}
	return true;
}

bool CustomDataDialogModel::keyIsUnique(const SCP_string& key, std::optional<size_t> ignoreIndex) const
{
	size_t i = 0;
	for (const auto& kv : _items) {
		if (ignoreIndex && *ignoreIndex == i) {
			++i;
			continue;
		}
		if (stricmp(kv.first.c_str(), key.c_str()) == 0)
			return false;
		++i;
	}
	return true;
}