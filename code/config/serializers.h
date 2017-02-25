#pragma once

#include "globalincs/pstypes.h"

#include <type_traits>

namespace config {

class serialization_error: public std::runtime_error {
 public:
	serialization_error() : std::runtime_error("Serialization error") {
	}

	serialization_error(const std::string& excuse) : std::runtime_error(excuse) {
	}

	~serialization_error() throw() {
	}
};

// Integer serializer functions
template<typename T>
typename std::enable_if<std::is_integral<T>::value, SCP_string>::type serialize(const T& value) {
	SCP_stringstream stream;
	stream << value;
	return stream.str();
}
template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type deserialize(const SCP_string& value) {
	SCP_stringstream stream(value);
	T parsed;
	stream >> parsed;

	if (!stream.good()) {
		throw serialization_error("Failed to deserialize value '" + value + "'");
	}

	return parsed;
}

// string serializer functions, simply passes the string through
SCP_string serialize(const SCP_string& str);
SCP_string deserialize(const SCP_string& str);

/**
 * @brief Default serializer that uses predefined, free functions
 * @tparam T The type of the object that should be serialized
 */
template<typename T>
struct serializer {
	// The default implementation uses free functions since I can't figure out how to do this only with structs...

	/**
	 * @brief Serializes the given raw value to a string representation
	 * @param value The value to serialize
	 * @return The serialized string representation
	 */
	static SCP_string serialize(const T& value) {
		return ::config::serialize<T>(value);
	}
	/**
	 * @brief Converts a string back into the raw value
	 * @param value The string value to deserialize
	 * @return The deserialized, raw value
	 */
	static T deserialize(const SCP_string& value) {
		return ::config::deserialize<T>(value);
	}
};

}

