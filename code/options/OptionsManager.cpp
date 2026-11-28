//Internal use manager to handle seting option data and
//correctly find options from config or save them back to config

#include "OptionsManager.h"
#include "Option.h"
#include "cmdline/cmdline.h"
#include "mod_table/mod_table.h"
#include "osapi/osregistry.h"
#include <optional>

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
std::optional<std::unique_ptr<json_t>> OptionsManager::getValueFromConfig(const SCP_string& key) const
{
	// An override always wins. The options UI disables the control for an overridden option, and
	// setConfigValue() rejects writes to one, so no edit can be pending here anyway.
	auto override_iter = _config_overrides.find(key);
	if (override_iter != _config_overrides.end()) {
		// We return a reference to an existing object so we need to increment the reference count
		json_incref(override_iter->second.value.get());
		// coverity[multiple_init_smart_ptr:FALSE] - according to m!m, this is most likely a false positive: "I think Coverity does not understand the usage of default_delete" in jansson.h, line 23
		return std::unique_ptr<json_t>(override_iter->second.value.get());
	}

	auto changed_iter = _changed_values.find(key);
	if (changed_iter != _changed_values.end()) {
		json_incref(changed_iter->second.get());
		// coverity[multiple_init_smart_ptr:FALSE] - according to m!m, this is most likely a false positive: "I think Coverity does not understand the usage of default_delete" in jansson.h, line 23
		return std::unique_ptr<json_t>(changed_iter->second.get());
	}

	return getSavedValue(key);
}

std::optional<std::unique_ptr<json_t>> OptionsManager::getSavedValue(const SCP_string& key) const
{
	auto parts = parse_key(key);
	if (parts.first.empty()) {
		// Invalid key
		throw std::runtime_error("Invalid key");
	}

	auto value = os_config_read_string(parts.first.c_str(), parts.second.c_str(), (const char*)0, true);

	if (value == nullptr) {
		// Signal that there is no value for this key
		return std::nullopt;
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
	// An overridden option is not editable. Accepting the edit here would write it to the config file and
	// live-apply it through valueChanged(), but getValue() would keep returning the override, so the game
	// and the options menu would disagree for the rest of the session.
	if (isOverridden(key)) {
		return;
	}

	_changed_values[key] = std::move(value);
}

bool OptionsManager::hasPendingEdit(const SCP_string& key) const
{
	return _changed_values.count(key) > 0;
}

//Provides a method for overriding a built-in option setting
//Generally used for commandline settings
void OptionsManager::setOverride(const SCP_string& key,
	const SCP_string& json,
	const SCP_string& reason,
	OverrideSource source)
{
	json_error_t err;
	auto el = json_loads(json.c_str(), JSON_DECODE_ANY, &err);
	if (el == nullptr) {
		return;
	}
	setOverride(key, std::unique_ptr<json_t>(el), reason, source);
}

void OptionsManager::setOverride(const SCP_string& key,
	std::unique_ptr<json_t>&& value,
	const SCP_string& reason,
	OverrideSource source)
{
	if (value == nullptr) {
		return;
	}
	// insert_or_assign (rather than emplace) so a later override for the same key, e.g. from a second cmdline
	// flag targeting the same option, replaces the earlier one instead of being silently ignored.
	_config_overrides.insert_or_assign(key, OverrideEntry{std::move(value), reason, source});

	// An override wins over a pending edit, so drop any edit that is already pending for this key.
	_changed_values.erase(key);
}

void OptionsManager::clearOverrides() { _config_overrides.clear(); }

bool OptionsManager::isOverridden(const SCP_string& key) const
{
	return _config_overrides.find(key) != _config_overrides.end();
}

std::optional<SCP_string> OptionsManager::getOverrideReason(const SCP_string& key) const
{
	auto iter = _config_overrides.find(key);
	if (iter == _config_overrides.end()) {
		return std::nullopt;
	}
	return iter->second.reason;
}

std::optional<OverrideSource> OptionsManager::getOverrideSource(const SCP_string& key) const
{
	auto iter = _config_overrides.find(key);
	if (iter == _config_overrides.end()) {
		return std::nullopt;
	}
	return iter->second.source;
}

//Adds an option to the options vector
const OptionBase* OptionsManager::addOption(std::shared_ptr<const OptionBase>&& option)
{
	_options.emplace_back(std::move(option));
	_optionsSorted = false; // Order got invalidated by adding a new option

	auto ptr = _options.back().get();
	_optionsMapping.emplace(ptr->getConfigKey(), ptr);
	return ptr;
}

//Removes an option from the options vector
void OptionsManager::removeOption(const std::shared_ptr<const OptionBase>& option)
{
	if (_optionsMapping.find(option->getConfigKey()) == _optionsMapping.end())
		return;

	_optionsMapping.erase(option->getConfigKey());
	_options.erase(
	    std::remove_if(_options.begin(), _options.end(),
	                   [option](const std::shared_ptr<const OptionBase>& ptr) { return ptr == option; }));
}

// Returns an option with the specified name
const OptionBase* OptionsManager::getOptionByKey(SCP_string key)
{
	for (size_t i = 0; i < _options.size(); i++) {
		if (_options[i].get()->getConfigKey() == key) {
			return _options[i].get();
		}
	}
	return nullptr;
}

void OptionsManager::enforceOption(const SCP_string& key)
{
	_enforcedOptions.insert(key);
}

void OptionsManager::unenforceOption(const SCP_string& key)
{
	_enforcedOptions.erase(key);
}

bool OptionsManager::isOptionEnforced(const SCP_string& key) const
{
	return _enforcedOptions.count(key) > 0;
}

//Returns a table of all built-in options available
const SCP_vector<std::shared_ptr<const options::OptionBase>>& OptionsManager::getOptions()
{
	if (!_optionsSorted) {
		// Keep options sorted by only sorting them when necessary

		std::sort(_options.begin(), _options.end(),
		          [](const std::shared_ptr<const OptionBase>& left, const std::shared_ptr<const OptionBase>& right) {
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

	os_config_write_string(parts.first.c_str(), parts.second.c_str(), val.c_str(), true);

	auto changed = option->valueChanged(iter->second.get());

	// Always erase even if it wasn't actually changed so later persistChanges() calls don't try to save the value again
	_changed_values.erase(iter);

	// This call can only report on its own option, so anything else the hooks could not apply only reaches the log
	for (const auto* unapplied : runPostPersistHooks({option})) {
		if (unapplied == option) {
			changed = false;
		} else {
			mprintf(("%s did not take effect after %s was persisted\n",
				unapplied->getConfigKey().c_str(),
				option->getConfigKey().c_str()));
		}
	}

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

		os_config_write_string(parts.first.c_str(), parts.second.c_str(), val.c_str(), true);
	}
	SCP_vector<const options::OptionBase*> unchanged;
	SCP_vector<const options::OptionBase*> notified;

	// Only notify options once all values have been written to config
	for (auto& entry : _changed_values) {
		auto iter = _optionsMapping.find(entry.first);
		if (iter != _optionsMapping.end()) {
			notified.push_back(iter->second);
			if (!iter->second->valueChanged(entry.second.get())) {
				unchanged.push_back(iter->second);
			}
		}
	}
	_changed_values.clear();

	const auto unapplied = runPostPersistHooks(notified);
	unchanged.insert(unchanged.end(), unapplied.begin(), unapplied.end());

	return unchanged;
}

int OptionsManager::addPostPersistHook(PostPersistHook hook)
{
	const int id = _next_post_persist_hook++;
	_post_persist_hooks.emplace(id, std::move(hook));
	return id;
}

void OptionsManager::removePostPersistHook(int id)
{
	_post_persist_hooks.erase(id);
}

SCP_vector<const options::OptionBase*> OptionsManager::runPostPersistHooks(
	const SCP_vector<const options::OptionBase*>& changed) const
{
	SCP_vector<const options::OptionBase*> unapplied;
	for (const auto& hook : _post_persist_hooks) {
		auto result = hook.second(changed);
		unapplied.insert(unapplied.end(), result.begin(), result.end());
	}
	return unapplied;
}

//Discard changes and restore the option to initial value
void OptionsManager::discardChanges() { _changed_values.clear(); }

void OptionsManager::rememberOptionsForLauncher()
{
	_describedAtLoad = getOptions();
	for (const auto& opt : _describedAtLoad) {
		opt->rememberDefaultBeforeLoad();
	}
}

//Get the initial values of the option as stored on disk
void OptionsManager::loadInitialValues()
{
	// only -get_options describes options, so normal startup does no extra work here
	_describedAtLoad.clear();
	if (Cmdline_get_options) {
		rememberOptionsForLauncher();
	}

	// Collect options to remove in a separate container
	SCP_vector<std::shared_ptr<const OptionBase>> optionsToRemove;

	for (const auto& opt : _options) { // Iterate forward to collect
		if (isOptionEnforced(opt->getConfigKey())) {
			optionsToRemove.push_back(opt);
		} else {
			opt->loadInitial();
		}
	}

	// Now remove the collected options.
	for (const auto& optToRemove : optionsToRemove) {
		removeOption(optToRemove);
	}
}

json_t* json_text(const SCP_string& text)
{
	json_t* value = json_string(text.c_str());
	if (value != nullptr) {
		return value;
	}
	// not UTF-8 (a legacy-encoded translation or folder name): read it as Latin-1 rather than drop the key
	SCP_string utf8;
	for (unsigned char c : text) {
		if (c < 0x80) {
			utf8 += static_cast<char>(c);
		} else {
			utf8 += static_cast<char>(0xC0 | (c >> 6));
			utf8 += static_cast<char>(0x80 | (c & 0x3F));
		}
	}
	return json_string(utf8.c_str());
}

bool json_write_line(const json_t* root, FILE* out, size_t flags)
{
	// a launcher may read the reply a line at a time
	return json_dumpf(root, out, flags) == 0 && fputc('\n', out) != EOF && fflush(out) == 0;
}

void OptionsManager::describeValues(const OptionBase& option, json_t* default_value, json_t* entry) const
{
	if (!Using_in_game_options) {
		// the game does not read the settings file then
		json_object_set_new(entry, "saved", json_null());
		json_object_set_new(entry, "effective", json_null());
		return;
	}
	const auto& key = option.getConfigKey();
	// read as saved, not through the option, which would turn a device this process didn't list into none
	std::optional<std::unique_ptr<json_t>> saved;
	try {
		saved = getSavedValue(key);
	} catch (const std::exception&) {
		// a stored value that is not JSON: loading ignores it too
	}
	if (saved.has_value()) {
		json_object_set(entry, "saved", saved->get());
	} else {
		json_object_set_new(entry, "saved", json_null());
	}
	auto override_iter = _config_overrides.find(key);
	const bool overridden = override_iter != _config_overrides.end();
	json_t* effective = default_value;
	// in the order the game settles it: a mod's HUD tables lock the option after loading, an enforced option is
	// never loaded and keeps the table's value, and loading takes a command-line override over the saved value
	if (overridden && override_iter->second.source == OverrideSource::Mod) {
		effective = override_iter->second.value.get();
	} else if (!isOptionEnforced(key)) {
		json_t* loaded = overridden ? override_iter->second.value.get() : nullptr;
		if (loaded == nullptr && saved.has_value()) {
			loaded = saved->get();
		}
		if (loaded != nullptr && option.acceptsStoredValue(loaded)) {
			effective = loaded;
		}
	}
	json_object_set(entry, "effective", effective);
}

json_t* OptionsManager::describeOptions()
{
	// a serialized value is itself JSON, so it goes in as a value, not as a string holding JSON
	auto value_of = [](const SCP_string& serialized) {
		json_error_t err;
		json_t* value = json_loads(serialized.c_str(), JSON_DECODE_ANY, &err);
		return (value != nullptr) ? value : json_text(serialized);
	};
	const auto& described = _describedAtLoad.empty() ? getOptions() : _describedAtLoad;
	json_t* list = json_array();
	for (const auto& option : described) {
		const auto& key = option->getConfigKey();
		json_t* default_value = value_of(option->getDescribedDefault());
		json_t* entry = json_object();
		json_object_set_new(entry, "key", json_text(key));
		json_object_set_new(entry, "title", json_text(option->getTitle()));
		json_object_set_new(entry, "description", json_text(option->getDescription()));
		json_object_set_new(entry, "category", json_text(option->getCategory()));
		if (option->getType() == OptionType::Range) {
			json_object_set_new(entry, "type", json_string("range"));
			auto range = option->getRangeValues();
			json_t* bounds = json_object();
			json_object_set_new(bounds, "min", json_real(range.first));
			json_object_set_new(bounds, "max", json_real(range.second));
			// an int option rejects a real number even when the menu flag for whole-number sliders is unset
			bool integer = option->getFlags()[OptionFlags::RangeTypeInteger] || json_is_integer(default_value);
			json_object_set_new(bounds, "integer", json_boolean(integer));
			json_object_set_new(entry, "range", bounds);
		} else {
			json_object_set_new(entry, "type", json_string("selection"));
			json_t* values = json_array();
			for (const auto& desc : option->getValidValues()) {
				json_t* value = json_object();
				json_object_set_new(value, "value", value_of(desc.serialized));
				json_object_set_new(value, "display", json_text(desc.display));
				json_array_append_new(values, value);
			}
			json_object_set_new(entry, "values", values);
		}
		json_object_set_new(entry, "default", default_value);
		describeValues(*option, default_value, entry);
		auto source = getOverrideSource(key);
		if (source.has_value()) {
			json_object_set_new(entry,
				"overridden_by",
				json_string(*source == OverrideSource::CommandLine ? "cmdline" : "mod"));
			json_object_set_new(entry, "override_reason", json_text(getOverrideReason(key).value_or("")));
		} else {
			json_object_set_new(entry, "overridden_by", json_null());
			json_object_set_new(entry, "override_reason", json_null());
		}
		json_object_set_new(entry, "enforced", json_boolean(isOptionEnforced(key)));
		json_object_set_new(entry, "restart_needed", json_boolean(!option->getIsLive()));
		auto requirement = _requirements.find(key);
		if (requirement != _requirements.end()) {
			json_object_set_new(entry, "requires", json_text(requirement->second.first));
			json_object_set_new(entry, "requires_text", json_text(requirement->second.second));
			auto found = _requirementSupport.find(requirement->second.first);
			const auto support = (found != _requirementSupport.end()) ? found->second : RequirementSupport::Unknown;
			const char* support_name = "unknown";
			if (support == RequirementSupport::Supported) {
				support_name = "supported";
			} else if (support == RequirementSupport::Unsupported) {
				support_name = "unsupported";
			}
			json_object_set_new(entry, "requires_support", json_string(support_name));
		} else {
			json_object_set_new(entry, "requires", json_null());
			json_object_set_new(entry, "requires_text", json_null());
			json_object_set_new(entry, "requires_support", json_null());
		}
		json_array_append_new(list, entry);
	}
	return list;
}

void OptionsManager::setRequirement(const SCP_string& key, const SCP_string& requirement, const SCP_string& explanation)
{
	_requirements[key] = std::make_pair(requirement, explanation);
}

void OptionsManager::setRequirementSupport(const SCP_string& requirement, RequirementSupport support)
{
	_requirementSupport[requirement] = support;
}

void OptionsManager::forgetLoadedOptions()
{
	for (const auto& item : _describedAtLoad) {
		item->forgetDefaultBeforeLoad();
		if (getOptionByKey(item->getConfigKey()) == nullptr) {
			auto option = item;
			addOption(std::move(option));
		}
	}
	_describedAtLoad.clear();
	_requirements.clear();
	_requirementSupport.clear();
}

void OptionsManager::printValues()
{
	mprintf(("Printing in-game options values!\n"));
	for (auto& opt : _options) {
		// If we're not using in-game options and the option is not a retail option, then skip
		// This ensures we only log options that are actually impacting the current game instance
		if (!Using_in_game_options && !(opt->getFlags()[options::OptionFlags::RetailBuiltinOption])){
			continue;
		}

		// Log the option
		mprintf(("Option.%s: %s\n",
			opt->getConfigKey().c_str(),
			opt->getCurrentValueDescription().display.c_str()));
	}
}

//Sets the value saved within the option, but does not actually change the variables tied to the option
//Used for persistence and UI updates
void OptionsManager::set_ingame_binary_option(SCP_string key, bool value)
{
	if (!Using_in_game_options) {
		return;
	}

	const OptionBase* thisOpt = getOptionByKey(std::move(key));
	if (thisOpt != nullptr) {
		auto val = thisOpt->getCurrentValueDescription();
		SCP_string newVal = value ? "true" : "false"; // OptionsManager stores values as serialized strings
		thisOpt->setValueDescription({val.display, newVal.c_str()});
	}
}

//Sets the value saved within the option, but does not actually change the variables tied to the option
//Used for persistence and UI updates
void OptionsManager::set_ingame_multi_option(SCP_string key, int value)
{
	if (!Using_in_game_options) {
		return;
	}

	const OptionBase* thisOpt = getOptionByKey(std::move(key));
	if (thisOpt != nullptr) {
		auto values = thisOpt->getValidValues();
		thisOpt->setValueDescription(values[value]);
	}
}

//Sets value saved within the option, but does not actually change the variables tied to the option
//Used for persistence and UI updates
void OptionsManager::set_ingame_range_option(SCP_string key, int value)
{
	if (!Using_in_game_options) {
		return;
	}

	const OptionBase* thisOpt = getOptionByKey(std::move(key));
	if (thisOpt != nullptr) {
		SCP_string newVal = std::to_string(value); // OptionsManager stores values as serialized strings
		thisOpt->setValueDescription({newVal.c_str(), newVal.c_str()});
	}
}

//Sets the value saved within the option, but does not actually change the variables tied to the option
//Used for persistence and UI updates
void OptionsManager::set_ingame_range_option(SCP_string key, float value)
{
	if (!Using_in_game_options) {
		return;
	}

	const OptionBase* thisOpt = getOptionByKey(std::move(key));
	if (thisOpt != nullptr) {
		SCP_string newVal = std::to_string(value); // OptionsManager stores values as serialized strings
		thisOpt->setValueDescription({newVal.c_str(), newVal.c_str()});
	}
}

} // namespace options
