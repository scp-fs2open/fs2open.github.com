#pragma once

#include "globalincs/pstypes.h"
#include "libs/jansson.h"
#include <memory>

namespace options {

// Forward declaration
class OptionBase;

class OptionsManager {
	OptionsManager();

	SCP_unordered_map<SCP_string, std::unique_ptr<json_t>> _config_overrides;

	SCP_unordered_map<SCP_string, std::unique_ptr<json_t>> _changed_values;

	SCP_unordered_map<SCP_string, const OptionBase*> _optionsMapping;

	SCP_vector<std::unique_ptr<const OptionBase>> _options;
	bool _optionsSorted = false;

  public:
	static OptionsManager* instance();

	~OptionsManager();

	std::unique_ptr<json_t> getValueFromConfig(const SCP_string& key) const;

	void setConfigValue(const SCP_string& key, std::unique_ptr<json_t>&& value);

	void setOverride(const SCP_string& key, const SCP_string& json);

	const OptionBase* addOption(std::unique_ptr<const OptionBase>&& option);

	void removeOption(const OptionBase* option);

	const SCP_vector<std::unique_ptr<const options::OptionBase>>& getOptions();

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
};

}
