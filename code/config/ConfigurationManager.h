#pragma once

#include "globalincs/pstypes.h"

namespace config {

class ConfigurationItemBase;

/**
 * @brief Class for managing known configuration items
 *
 * This class is a static singleton which keeps track of all registered config items and allows to save them when the
 * application exits.
 */
class ConfigurationManager {
	SCP_vector<ConfigurationItemBase*> _config_items;

	ConfigurationManager();
 public:
	// Disallow copying since this is a singleton
	ConfigurationManager(const ConfigurationManager&) = delete;
	ConfigurationManager& operator=(const ConfigurationManager&) = delete;

	// Disallow moving since this is a singleton
	ConfigurationManager(ConfigurationManager&&) = delete;
	ConfigurationManager& operator=(ConfigurationManager&&) = delete;

	/**
	 * @brief Gets the manager instance
	 *
	 * @details This uses a local static variable so you can call this from static initializers of other classes in
	 * other files and it will still work.
	 *
	 * @return The manager instance
	 */
	static ConfigurationManager& getInstance();

	/**
	 * @brief Adds a config item to this manager
	 *
	 * Added items will be saved when the manager instance is saved. A new item can be added at runtime.
	 *
	 * @param item The item to add
	 */
	void addConfigItem(ConfigurationItemBase* item);

	/**
	 * @brief Loads the value for the specified item
	 * @param item The item to load the data for
	 */
	void loadConfigItem(ConfigurationItemBase* item);

	/**
	 * @brief Save the values currently stored in the configuration items
	 */
	void saveSettings();
};

}

