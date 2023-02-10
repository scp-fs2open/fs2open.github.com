//Internal use manager to handle seting option data and
//correctly find options from config or save them back to config

#include "OptionsManager.h"
#include "Option.h"
#include "mod_table/mod_table.h"
#include "osapi/osregistry.h"

namespace {

//Parses a full option key, separating it into section and key
std::pair<SCP_string, SCP_string> parse_key(const SCP_string& key)
{
	auto point_pos = key.find('.');
	if (point_pos == SCP_string::npos) {
		// Invalid key
		return std::pair<SCP_string, SCP_string>();
	}

	auto section     = key.substr(0, point_pos);
	auto section_key = key.substr(point_pos + 1);

	return std::make_pair(section, section_key);
}
} // namespace

namespace options {

OptionsManager::OptionsManager() = default;
OptionsManager::~OptionsManager() = default;

OptionsManager* options::OptionsManager::instance()
{
	static OptionsManager instance;
	return &instance;
}

//Gets the value of an option from the Config using the option key
std::unique_ptr<json_t> OptionsManager::getValueFromConfig(const SCP_string& key) const
{
	auto override_iter = _config_overrides.find(key);
	if (override_iter != _config_overrides.end()) {
		// We return a reference to an existing object so we need to increment the reference count
		json_incref(override_iter->second.get());
		return std::unique_ptr<json_t>(override_iter->second.get());
	}

	auto changed_iter = _changed_values.find(key);
	if (changed_iter != _changed_values.end()) {
		json_incref(changed_iter->second.get());
		return std::unique_ptr<json_t>(changed_iter->second.get());
	}

	auto parts = parse_key(key);
	if (parts.first.empty()) {
		// Invalid key
		throw std::runtime_error("Invalid key");
	}

	auto value = os_config_read_string(parts.first.c_str(), parts.second.c_str());

	if (value == nullptr) {
		// TODO: This is not really an error but I would like to avoid return nullptr here...
		throw std::runtime_error("No value available");
	}

	json_error_t err;
	auto el = json_loads(value, JSON_DECODE_ANY, &err);
	if (el == nullptr) {
		throw json_exception(err);
	}

	return std::unique_ptr<json_t>(el);
}

//Sets the value for the option
void OptionsManager::setConfigValue(const SCP_string& key,std::unique_ptr<json_t>&& value)
{
	_changed_values[key] = std::move(value);
}

//Provides a method for overriding a built-in option setting
//Generally used for commandline settings
void OptionsManager::setOverride(const SCP_string& key, const SCP_string& json)
{
	json_error_t err;
	auto el = json_loads(json.c_str(), JSON_DECODE_ANY, &err);
	if (el == nullptr) {
		return;
	}
	_config_overrides.emplace(key, std::unique_ptr<json_t>(el));
}

//Adds an option to the options vector
const OptionBase* OptionsManager::addOption(std::unique_ptr<const OptionBase>&& option)
{
	_options.emplace_back(std::move(option));
	_optionsSorted = false; // Order got invalidated by adding a new option

	auto ptr = _options.back().get();
	_optionsMapping.emplace(ptr->getConfigKey(), ptr);
	return ptr;
}

//Removes an option from the options vector
void OptionsManager::removeOption(const OptionBase* option)
{
	_optionsMapping.erase(option->getConfigKey());
	_options.erase(
	    std::remove_if(_options.begin(), _options.end(),
	                   [option](const std::unique_ptr<const OptionBase>& ptr) { return ptr.get() == option; }));
}

//Returns a table of all built-in options available
const SCP_vector<std::unique_ptr<const options::OptionBase>>& OptionsManager::getOptions()
{
	if (!_optionsSorted) {
		// Keep options sorted by only sorting them when necessary

		std::sort(_options.begin(), _options.end(),
		          [](const std::unique_ptr<const OptionBase>& left, const std::unique_ptr<const OptionBase>& right) {
			          return *left < *right;
		          });

		_optionsSorted = true;
	}
	return _options;
}

//Write the option to the registry and return if it was changed or not
bool OptionsManager::persistOptionChanges(const options::OptionBase* option)
{
	auto iter = _changed_values.find(option->getConfigKey());

	if (iter == _changed_values.end()) {
		// No changes for this option
		return true;
	}

	auto parts = parse_key(iter->first);
	if (parts.first.empty()) {
		// Invalid key
		return false;
	}

	auto val = json_dump_string(iter->second.get(), JSON_COMPACT | JSON_ENSURE_ASCII | JSON_ENCODE_ANY);

	os_config_write_string(parts.first.c_str(), parts.second.c_str(), val.c_str());

	auto changed = option->valueChanged(iter->second.get());

	// Always erase even if it wasn't actually changed so later persistChanges() calls don't try to save the value again
	_changed_values.erase(iter);

	return changed;
}

//Write value changes to disk and return vector of options that do not support changine the value
SCP_vector<const options::OptionBase*> OptionsManager::persistChanges()
{
	for (auto& entry : _changed_values) {
		auto parts = parse_key(entry.first);
		if (parts.first.empty()) {
			// Invalid key
			continue;
		}

		auto val = json_dump_string(entry.second.get(), JSON_COMPACT | JSON_ENSURE_ASCII | JSON_ENCODE_ANY);

		os_config_write_string(parts.first.c_str(), parts.second.c_str(), val.c_str());
	}
	SCP_vector<const options::OptionBase*> unchanged;

	// Only notify options once all values have been written to config
	for (auto& entry : _changed_values) {
		auto iter = _optionsMapping.find(entry.first);
		if (iter != _optionsMapping.end()) {
			if (!iter->second->valueChanged(entry.second.get())) {
				unchanged.push_back(iter->second);
			}
		}
	}
	_changed_values.clear();

	return unchanged;
}

//Discard changes and restore the option to initial value
void OptionsManager::discardChanges() { _changed_values.clear(); }

//Get the initial values of the option as stored on disk
void OptionsManager::loadInitialValues()
{
	for (auto& opt : _options) {
		opt->loadInitial();
	}
}

} // namespace options
