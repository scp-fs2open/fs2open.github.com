//
//

#include "ConfigurationManager.h"
#include "ConfigurationItem.h"

#include "osapi/osregistry.h"

namespace config {

ConfigurationManager::ConfigurationManager() {
}

ConfigurationManager& ConfigurationManager::getInstance() {
	// Use the static initializer trick to avoid initialization issues
	static ConfigurationManager manager;

	return manager;
}
void ConfigurationManager::addConfigItem(ConfigurationItemBase* item) {
	Assertion(item != nullptr, "Invalid item pointer detected!");

#ifndef NDEBUG
	// Check if this section:name combination already exists
	for (auto& existing : _config_items) {
		bool sameSection;
		if (existing->getSection() == item->getSection()) {
			sameSection = true;
		} else if (existing->getSection() == nullptr) {
			sameSection = false;
		} else if (item->getSection() == nullptr) {
			sameSection = false;
		} else {
			sameSection = !strcmp(existing->getSection(), item->getSection());
		}

		auto sameName = strcmp(existing->getName(), item->getName());

		Assertion(!(sameSection && sameName),
				  "A config item with name '%s:%s' already exists!",
				  existing->getSection(),
				  existing->getName());
	}
#endif

	_config_items.push_back(item);
}
void ConfigurationManager::loadConfigItem(ConfigurationItemBase* item) {
	Assertion(item != nullptr, "Invalid item pointer detected!");

	auto data = os_config_read_string(item->getSection(), item->getName(), nullptr);

	if (data != nullptr) {
		try {
			item->deserialize(data);
		} catch (const std::exception& e) {
			mprintf(("Failed to read config item %s:%s! Error: '%s'. Value was '%s'.\n", item->getSection(), item->getName(), e.what(), data));
		}
	}
}
void ConfigurationManager::saveSettings() {
	for (auto item : _config_items) {
		auto value = item->serialize();
		os_config_write_string(item->getSection(), item->getName(), value.c_str());
	}
}

}
