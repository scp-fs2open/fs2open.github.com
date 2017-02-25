//
//

#include "ConfigurationItem.h"

#include "config/ConfigurationManager.h"

namespace {
config::ConfigurationItem<uint64_t> test("Test", "Section", 20);
}

namespace config {

ConfigurationItemBase::ConfigurationItemBase(const char* section, const char* name) : _section(section), _name(name) {
	ConfigurationManager::getInstance().addConfigItem(this);
}
const char* ConfigurationItemBase::getSection() const {
	return _section;
}
const char* ConfigurationItemBase::getName() const {
	return _name;
}

}
