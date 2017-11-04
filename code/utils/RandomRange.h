#pragma once

#include "globalincs/pstypes.h"
#include "parse/parselo.h"

#include <random>
#include <type_traits>
#include <limits>

namespace util {

/**
 * @defgroup randomUtils Random Utilities
 *
 * Utility functions for handling random values
 */

namespace {

/**
 * @brief Parses a generic list of numbers
 * @param list The array where the numbers should be stored.
 * @return The amount of parsed numbers
 *
 * @ingroup randomUtils
 */
template<typename T, size_t N>
size_t parse_number_list(T (& list)[N]) {
	float helpList[N];
	auto num = stuff_float_list(helpList, N);

	for (size_t i = 0; i < num; ++i) {
		list[i] = static_cast<T>(helpList[i]);
	}

	return num;
}
}

/**
 * @brief Generic class for generating numbers in a specific range
 *
 * This allows to use a generic value, distribution and generator type. It's valid to only use one value for the range
 * in which case the returned value is constant.
 *
 * @ingroup randomUtils
 */
template<typename Value, typename Distribution, typename Generator>
class RandomRange {
 public:
	typedef Distribution DistributionType;
	typedef Generator GeneratorType;
	typedef Value ValueType;

 private:
	GeneratorType m_generator;
	DistributionType m_distribution;

	bool m_constant;
	ValueType m_minValue;
	ValueType m_maxValue;

 public:
	template<typename... Ts>
	RandomRange(ValueType param1, ValueType param2, Ts&& ... distributionParameters) :
		m_generator(std::random_device()()),
		m_distribution(param1, param2, distributionParameters...) {
		m_minValue = static_cast<ValueType>(0.0);
		m_maxValue = static_cast<ValueType>(0.0);
		m_constant = false;
	}

	explicit RandomRange(const ValueType& val) : RandomRange() {
		m_minValue = val;
		m_maxValue = val;
		m_constant = true;
	}

	RandomRange() :
		m_generator(std::random_device()()),
		m_distribution() {
		m_minValue = static_cast<ValueType>(0.0);
		m_maxValue = static_cast<ValueType>(0.0);
		m_constant = true;
	}

	/**
	 * @brief Determines the next random number of this range
	 * @return The random number
	 */
	ValueType next() {
		if (m_constant) {
			return m_minValue;
		}

		return m_distribution(m_generator);
	}

	ValueType min() {
		return m_minValue;
	}

	ValueType max() {
		return m_minValue;
	}
};

/**
 * @brief A random range with a normal distribution
 *
 * The range parameters are passed directly to std::normal_distribution
 *
 * @ingroup randomUtils
 */
template<typename Value>
using NormalRange = RandomRange<Value,
								std::normal_distribution<Value>,
								std::minstd_rand>;

/**
 * @brief A normal range which uses floats
 *
 * @ingroup randomUtils
 */
typedef NormalRange<float> NormalFloatRange;

/**
 * @brief A function for parsing a normal range
 * @return The parsed normal range
 *
 * @ingroup randomUtils
 */
template<typename Value>
NormalRange<Value> parseNormalRange() {
	Value valueList[2];
	auto num = parse_number_list(valueList);

	if (num == 0) {
		error_display(0, "Need at least one value to form a random range!");
		return NormalRange<Value>();
	}
	else if (num == 1) {
		return NormalRange<Value>(valueList[0]);
	}

	return NormalRange<Value>(valueList[0], valueList[1]);
}

/**
 * @brief A generic random range which uses a uniform distribution
 *
 * @ingroup randomUtils
 */
template<typename Value>
using UniformRange = RandomRange<Value,
								 typename std::conditional<std::is_integral<Value>::value,
														   std::uniform_int_distribution<Value>,
														   std::uniform_real_distribution<Value>>::type,
								 std::minstd_rand>;

/**
 * @brief A uniform range which uses floats
 *
 * @ingroup randomUtils
 */
typedef UniformRange<float> UniformFloatRange;

/**
 * @brief A uniform range which uses ints
 *
 * @ingroup randomUtils
 */
typedef UniformRange<int> UniformIntRange;

/**
 * @brief A uniform range which uses uints
 *
 * @ingroup randomUtils
 */
typedef UniformRange<uint> UniformUIntRange;

/**
 * @brief Parses a generic uniform range
 * @param allowNegativ If @c true, negative values will be allowed
 * @param min The minimum value the random range may return
 * @param max The maximum value the random range may return
 *
 * @return The parsed uniform range
 *
 * @ingroup randomUtils
 */
template<typename Value>
UniformRange<Value> parseUniformRange(Value min = std::numeric_limits<Value>::min(),
									  Value max = std::numeric_limits<Value>::max()) {
	Assertion(min <= max, "Invalid min-max values specified!");

	Value valueList[2];
	auto num = parse_number_list(valueList);

	if (num == 0) {
		error_display(0, "Need at least one value to form a random range!");
		return UniformRange<Value>();
	}
	else if (num == 1) {
		return UniformRange<Value>(valueList[0]);
	}

	if (valueList[0] > valueList[1]) {
		error_display(0, "Minimum value %f is more than maximum value %f!", (float) valueList[0], (float) valueList[1]);
		std::swap(valueList[0], valueList[1]);
	}

	if (valueList[0] < min) {
		error_display(0, "First value (%f) is less than the minimum %f!", (float) valueList[0], (float) min);
		valueList[0] = min;
	}
	if (valueList[0] > max) {
		error_display(0, "First value (%f) is greater than the maximum %f!", (float) valueList[0], (float) max);
		valueList[0] = max;
	}

	if (valueList[1] < min) {
		error_display(0, "Second value (%f) is less than the minimum %f!", (float) valueList[1], (float) min);
		valueList[1] = min;
	}
	if (valueList[1] > max) {
		error_display(0, "Second value (%f) is greater than the maximum %f!", (float) valueList[1], (float) max);
		valueList[1] = max;
	}

	if (valueList[0] == valueList[1]) {
		// If the two values are equal then this is slightly more efficient
		return UniformRange<Value>(valueList[0]);
	} else {
		return UniformRange<Value>(valueList[0], valueList[1]);
	}
}
}
