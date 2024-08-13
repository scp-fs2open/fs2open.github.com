#pragma once

#include "globalincs/pstypes.h"
#include "parse/parselo.h"
#include "math/curve.h"

#include <mpark/variant.hpp>

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
template <typename T, size_t N>
size_t parse_number_list(T (&list)[N])
{
	ignore_white_space();

	if (*Mp != '(') {
		// Probably one a single value so stuff that and don't parse a list. This makes it easier to specify single
		// values
		float val;
		stuff_float(&val);

		list[0] = static_cast<T>(val);
		return 1;
	}

	float helpList[N];
	auto num = stuff_float_list(helpList, N);

	for (size_t i = 0; i < num; ++i) {
		list[i] = static_cast<T>(helpList[i]);
	}

	return num;
}
} // namespace

/**
 * @brief Generic class for generating numbers in a specific range
 *
 * This allows to use a generic value, distribution and generator type. It's valid to only use one value for the range
 * in which case the returned value is constant.
 *
 * @ingroup randomUtils
 */
template <typename Value, typename Distribution, typename Generator>
class RandomRange {
  public:
	typedef Distribution DistributionType;
	typedef Generator GeneratorType;
	typedef Value ValueType;

  private:
	mutable GeneratorType m_generator;
	mutable DistributionType m_distribution;

	bool m_constant;
	ValueType m_minValue;
	ValueType m_maxValue;
	int m_curve = -1;

  public:
	template <typename... Ts>
	RandomRange(ValueType param1, ValueType param2, Ts&&... distributionParameters)
		: m_generator(std::random_device()()), m_distribution(param1, param2, distributionParameters...)
	{
		m_minValue = static_cast<ValueType>(param1);
		m_maxValue = static_cast<ValueType>(param2);
		m_constant = false;
	}

	explicit RandomRange(const ValueType& val) : RandomRange()
	{
		m_minValue = val;
		m_maxValue = val;
		m_constant = true;
	}

	RandomRange() : m_generator(std::random_device()()), m_distribution()
	{
		m_minValue = static_cast<ValueType>(0.0);
		m_maxValue = static_cast<ValueType>(0.0);
		m_constant = true;
	}

	/**
	 * @brief Determines the next random number of this range
	 * @return The random number
	 */
	ValueType next() const
	{
		if (m_constant) {
			return m_minValue;
		} else if (m_curve >= 0) {
			float interp = Curves[m_curve].GetValue(frand());
			return (interp * (m_maxValue - m_minValue)) + m_minValue;
		}

		return m_distribution(m_generator);
	}

	/**
	 * @brief Gets the minimum value that may be returned by this random range
	 *
	 * @warning This is not valid for normal distribution ranges since those do not have a definite minimum value.
	 *
	 * @return The minimum value
	 */
	ValueType min() const
	{
		return m_minValue;
	}

	/**
	 * @brief Gets the maximum value that may be returned by this random range
	 *
	 * @warning This is not valid for normal distribution ranges since those do not have a definite maximum value.
	 *
	 * @return The maximum value
	 */
	ValueType max()
	{
		return m_maxValue;
	}
};

/**
 * @brief A random range with a normal distribution
 *
 * The range parameters are passed directly to std::normal_distribution
 *
 * @ingroup randomUtils
 */
template <typename Value>
using NormalRange = RandomRange<Value, std::normal_distribution<Value>, std::minstd_rand>;

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
template <typename Value>
NormalRange<Value> parseNormalRange()
{
	Value valueList[2];
	auto num = parse_number_list(valueList);

	if (num == 0) {
		error_display(0, "Need at least one value to form a random range!");
		return NormalRange<Value>();
	} else if (num == 1) {
		return NormalRange<Value>(valueList[0]);
	}

	return NormalRange<Value>(valueList[0], valueList[1]);
}

/**
 * @brief A generic random range which uses a uniform distribution
 *
 * @ingroup randomUtils
 */
template <typename Value>
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
template <typename Value>
UniformRange<Value> parseUniformRange(Value min = std::numeric_limits<Value>::min(),
	Value max = std::numeric_limits<Value>::max())
{
	Assertion(min <= max, "Invalid min-max values specified!");

	Value valueList[2];
	auto num = parse_number_list(valueList);

	if (num == 0) {
		error_display(0, "Need at least one value to form a random range!");
		return UniformRange<Value>();
	} else if (num == 1) {
		return UniformRange<Value>(valueList[0]);
	}

	if (valueList[0] > valueList[1]) {
		error_display(0, "Minimum value %f is more than maximum value %f!", (float)valueList[0], (float)valueList[1]);
		std::swap(valueList[0], valueList[1]);
	}

	if (valueList[0] < min) {
		error_display(0, "First value (%f) is less than the minimum %f!", (float)valueList[0], (float)min);
		valueList[0] = min;
	}
	if (valueList[0] > max) {
		error_display(0, "First value (%f) is greater than the maximum %f!", (float)valueList[0], (float)max);
		valueList[0] = max;
	}

	if (valueList[1] < min) {
		error_display(0, "Second value (%f) is less than the minimum %f!", (float)valueList[1], (float)min);
		valueList[1] = min;
	}
	if (valueList[1] > max) {
		error_display(0, "Second value (%f) is greater than the maximum %f!", (float)valueList[1], (float)max);
		valueList[1] = max;
	}

	if (valueList[0] == valueList[1]) {
		// If the two values are equal then this is slightly more efficient
		return UniformRange<Value>(valueList[0]);
	} else {
		return UniformRange<Value>(valueList[0], valueList[1]);
	}
}

class CurveNumberDistribution {
	int m_curve;

  public:
	using result_type = float;
	using param_type = int;
	CurveNumberDistribution() : CurveNumberDistribution(-1) {}
	CurveNumberDistribution(int curve) : m_curve(curve) {}
	void reset() {}
	inline int param()
	{
		return m_curve;
	}
	inline void param(int p)
	{
		m_curve = p;
	}
	template <typename Generator>
	result_type operator()(Generator& generator, const param_type& curve)
	{
		float max_integral = Curves[curve].GetValueIntegrated(Curves[curve].keyframes.back().pos.x);
		float rand =
			std::generate_canonical<float, std::numeric_limits<float>::digits, Generator>(generator) * max_integral;
		float lower_bound = Curves[curve].keyframes.front().pos.x;
		float upper_bound = Curves[curve].keyframes.back().pos.x;
		for (size_t count = 0; count < 16; count++) {
			float current_pos = (lower_bound + upper_bound) / 2.f;
			float current_value = Curves[curve].GetValueIntegrated(current_pos);
			if (fl_equal(current_value, rand, max_integral * 0.01f)) {
				return current_pos;
			}
			if (current_value > rand) {
				upper_bound = current_pos;
			} else {
				lower_bound = current_pos;
			}
		}
		return (lower_bound + upper_bound) / 2.f;
	}
	template <typename Generator>
	result_type operator()(Generator& generator)
	{
		return this->operator()(generator, m_curve);
	}
	inline result_type min()
	{
		return Curves[m_curve].keyframes.front().pos.x;
	}
	inline result_type max()
	{
		return Curves[m_curve].keyframes.back().pos.x;
	}
	bool operator==(const CurveNumberDistribution& other)
	{
		return m_curve == other.m_curve;
	}
	bool operator!=(const CurveNumberDistribution& other)
	{
		return !(*this == other);
	}
};

template <typename _RealType, typename _CharT, typename _Traits>
std::basic_ostream<_CharT, _Traits>& operator<<(std::basic_ostream<_CharT, _Traits>& __os,
	const CurveNumberDistribution& __x)
{
	__os << __x.param();
	return __os;
}

template <typename _RealType, typename _CharT, typename _Traits>
std::basic_istream<_CharT, _Traits>& operator>>(std::basic_istream<_CharT, _Traits>& __is,
	const CurveNumberDistribution& __x)
{
	int curve;
	__is >> curve;
	__x.param(curve);
	return __is;
}

using CurveFloatRange = RandomRange<float, CurveNumberDistribution, std::minstd_rand>;

CurveFloatRange ParseCurveFloatRange() {
	Value valueList[2];
	auto num = parse_number_list(valueList);

	if (num == 0) {
		error_display(0, "Need at least one value to form a random range!");
		return NormalRange<Value>();
	} else if (num == 1) {
		return NormalRange<Value>(valueList[0]);
	}

	return NormalRange<Value>(valueList[0], valueList[1]);
}

class ParsedRandomRange {
	mpark::variant<UniformFloatRange, NormalFloatRange, CurveFloatRange> m_random_range;
	ParsedRandomRange() {}
public:
	inline float next() {
		return mpark::visit([] (auto& range) { return range.next(); }, m_random_range);
	}
	float min() const {

	}
	float max() const {

	}
};

}