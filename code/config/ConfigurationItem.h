#pragma once

#include "config/serializers.h"
#include "ConfigurationManager.h"

namespace config {

/**
 * @brief Non-template base class of ConfigurationItem
 */
class ConfigurationItemBase {
 protected:
	const char* _section;
	const char* _name;

 public:
	ConfigurationItemBase(const char* section, const char* name);

	/**
	 * @brief Gets the section name of this config item
	 * @return The section name
	 */
	const char* getSection() const;
	/**
	 * @brief Gets the name of this config item
	 * @return The item name
	 */
	const char* getName() const;

	/**
	 * @brief Called to deserialize the given string value
	 *
	 * @details This should initialize the value contained in this item to correspond to the value contained in the
	 * string. This may throw an exception if deserialization fails (e.g. if the input is malformed).
	 *
	 * @param value The value to deserialize
	 */
	virtual void deserialize(const SCP_string& value) = 0;
	/**
	 * @brief Serializes the current value into a string value
	 * @return The serialized string value
	 */
	virtual SCP_string serialize() const = 0;
};

/**
 * @brief A configuration item of a specific type
 *
 * @tparam T The type of the value contained in this item
 * @tparam Serializer The type responsible for serializing and deserializing
 */
template<typename T, typename Serializer = ::config::serializer<T>>
class ConfigurationItem: public ConfigurationItemBase {
	T _value;
	bool _has_value = false; //!< @c true if the value above was loaded from the config file
 public:
	/**
	 * @brief Initializes this config item.
	 * @param section The section of this item
	 * @param name The name in the section of this item
	 * @param default_value The default value that will be returned if no value exists in the config file
	 */
	ConfigurationItem(const char* section, const char* name, T&& default_value) : ConfigurationItemBase(section, name),
																				  _value(default_value) {
	}

	/**
	 * @brief Deserializes the value using Serializer and sets it as the current value
	 * @param value The value to deserialize
	 */
	void deserialize(const SCP_string& value) override {
		setValue(Serializer::deserialize(value));
	}
	/**
	 * @brief Serializes the current value using Serializer
	 * @return The serialized value
	 */
	SCP_string serialize() const override {
		return Serializer::serialize(_value);
	}

	/**
	 * @brief Gets the current value of this item
	 *
	 * @details This will lazily load the value from the config if it hasn't been loaded yet
	 *
	 * @return The current value
	 */
	T getValue() {
		if (!_has_value) {
			ConfigurationManager::getInstance().loadConfigItem(this);
		}
		return _value;
	}
	/**
	 * @brief Sets the value of this item to the specified value
	 * @param value The new value of this item
	 */
	void setValue(const T& value) {
		_value = value;
		_has_value = true;
	}
};

}

