#pragma once

#include "globalincs/pstypes.h"
#include "libs/jansson.h"
#include <memory>
#include <optional>

namespace options {

// Forward declaration
class OptionBase;

class OptionsManager {
	OptionsManager();

	SCP_unordered_map<SCP_string, std::unique_ptr<json_t>> _config_overrides;

	SCP_unordered_map<SCP_string, std::unique_ptr<json_t>> _changed_values;

	SCP_unordered_map<SCP_string, const OptionBase*> _optionsMapping;

	// Enforced options are hidden from the player and do not load values from user settings
	SCP_unordered_set<SCP_string> _enforcedOptions;

	SCP_vector<std::shared_ptr<const OptionBase>> _options;
	bool _optionsSorted = false;

  public:
	static OptionsManager* instance();

	~OptionsManager();

	std::optional<std::unique_ptr<json_t>> getValueFromConfig(const SCP_string& key) const;

	void setConfigValue(const SCP_string& key, std::unique_ptr<json_t>&& value);

	void setOverride(const SCP_string& key, const SCP_string& json);

	const OptionBase* addOption(std::shared_ptr<const OptionBase>&& option);

	void removeOption(const std::shared_ptr<const OptionBase>& option);

	const OptionBase* getOptionByKey(SCP_string name);

	void enforceOption(const SCP_string& key);

	void unenforceOption(const SCP_string& key);

	bool isOptionEnforced(const SCP_string& key) const;

	const SCP_vector<std::shared_ptr<const options::OptionBase>>& getOptions();

	bool persistOptionChanges(const options::OptionBase* option);

	/**
	 * @brief Persists changes to the disk
	 *
	 * This writes all the outstanding changes to the config file and returns a vector of all the options that indicated
	 * that they could not change the value. If the return value is empty then all changes were applied but if it is not
	 * empty then a restart is required for all the options to take effect.
	 *
	 * @return A vector of options that did not support changing the value
	 */
	SCP_vector<const options::OptionBase*> persistChanges();

	void discardChanges();

	void loadInitialValues();

	void printValues();

	void set_ingame_binary_option(SCP_string key, bool value);
	void set_ingame_multi_option(SCP_string key, int value);
	void set_ingame_range_option(SCP_string key, int value);
	void set_ingame_range_option(SCP_string key, float value);
};

}
