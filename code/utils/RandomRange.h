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

  public:
	template <typename T, typename... Ts, typename = typename std::enable_if<sizeof... (Ts) >=1 || !std::is_same<ValueType, typename std::remove_reference<typename std::remove_cv<T>::type>::type>::value, int>::type>
	RandomRange(T&& distributionFirstParameter, Ts&&... distributionParameters)
		: m_generator(std::random_device()()), m_distribution(distributionFirstParameter, distributionParameters...)
	{
		m_minValue = static_cast<ValueType>(m_distribution.min());
		m_maxValue = static_cast<ValueType>(m_distribution.max());
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
		}

		return m_distribution(m_generator);
	}

	/**
	 * @brief Gets the minimum value that may be returned by this random range
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
	 * @return The maximum value
	 */
	ValueType max() const
	{
		return m_maxValue;
	}

	void seed(typename GeneratorType::result_type new_seed) const {
		if (m_constant)
			return;

		m_generator.seed(new_seed);
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

class BoundedNormalDistribution {
  public:
	using result_type = float;
	using param_type = struct {
		std::normal_distribution<float>::param_type normal_parameters;
		float min;
		float max;
	};
	param_type m_param;
	inline BoundedNormalDistribution() : BoundedNormalDistribution(param_type{std::normal_distribution<float>::param_type{0.5f, 1.f}, 0.f, 1.f}) {}
	inline BoundedNormalDistribution(param_type curve) : m_param(curve) {}
	inline void reset() {}
	inline param_type param() const
	{
		return m_param;
	}
	inline void param(param_type p)
	{
		m_param = p;
	}
	template <typename Generator>
	inline result_type operator()(Generator& generator, const param_type& param)
	{
		float unbounded = std::normal_distribution<float>()(generator, param.normal_parameters);
		CLAMP(unbounded, param.min, param.max);
		return unbounded;
	}
	template <typename Generator>
	inline result_type operator()(Generator& generator)
	{
		return this->operator()(generator, m_param);
	}
	inline result_type min() const
	{
		return m_param.min;
	}
	inline result_type max() const
	{
		return m_param.max;
	}
	inline bool operator==(const BoundedNormalDistribution& other) const
	{
		return (m_param.normal_parameters == other.m_param.normal_parameters && fl_equal(m_param.min, other.m_param.min) &&
				fl_equal(m_param.max, other.m_param.max));
	}
	inline bool operator!=(const BoundedNormalDistribution& other) const
	{
		return !(*this == other);
	}
};

using BoundedNormalFloatRange = RandomRange<float, BoundedNormalDistribution, std::minstd_rand>;

/**
 * @brief A function for parsing a normal range
 * @return The parsed normal range
 *
 * @ingroup randomUtils
 */
inline BoundedNormalFloatRange parseNormalFloatRange(float min = std::numeric_limits<float>::lowest()/2.1f, float max = std::numeric_limits<float>::max()/2.1f)
{
	float valueList[2];
	auto num = parse_number_list(valueList);

	float parsed_min = min;
	float parsed_max = max;

	if (num == 0) {
		error_display(0, "Need at least one value to form a random range!");
		return {};
	} else if (num == 1) {
		return BoundedNormalFloatRange(BoundedNormalDistribution::param_type{std::normal_distribution<float>::param_type(valueList[0]), parsed_min, parsed_max});
	}

	stuff_float_optional(&parsed_min);
	stuff_float_optional(&parsed_max);

	if (parsed_min > parsed_max) {
	error_display(0, "Minimum value %f is more than maximum value %f!", (float)parsed_min, (float)parsed_max);
	std::swap(parsed_min, parsed_max);
	}

	if (parsed_min < min) {
		error_display(0, "First value (%f) is less than the minimum %f!", (float)parsed_min, (float)min);
		parsed_min = min;
	}
	if (parsed_min > max) {
		error_display(0, "First value (%f) is greater than the maximum %f!", (float)parsed_min, (float)max);
		parsed_min = max;
	}

	if (parsed_max < min) {
		error_display(0, "Second value (%f) is less than the minimum %f!", (float)parsed_max, (float)min);
		parsed_max = min;
	}
	if (parsed_max > max) {
		error_display(0, "Second value (%f) is greater than the maximum %f!", (float)parsed_max, (float)max);
		parsed_max = max;
	}

	return BoundedNormalFloatRange(BoundedNormalDistribution::param_type{std::normal_distribution<float>::param_type(valueList[0], valueList[1]), parsed_min, parsed_max});
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
UniformRange<Value> parseUniformRange(Value min = std::numeric_limits<float>::lowest()/2.1f,
	Value max = std::numeric_limits<float>::max()/2.1f)
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
  public:
	using result_type = float;
	using param_type = struct {
		int curve;
		float min;
		float max;
	};
	param_type m_param;
	inline CurveNumberDistribution() : CurveNumberDistribution(param_type{-1, NAN, NAN}) {}
	inline CurveNumberDistribution(param_type curve) : m_param(curve) {}
	inline void reset() {}
	inline param_type param() const
	{
		return m_param;
	}
	inline void param(param_type p)
	{
		m_param = p;
	}
	template <typename Generator>
	inline result_type operator()(Generator& generator, const param_type& param)
	{
		if (param.curve < 0) {
			return 0.f;
		}
		float lower_bound = Curves[param.curve].keyframes.front().pos.x;
		float upper_bound = Curves[param.curve].keyframes.back().pos.x;

		float max_integral = Curves[param.curve].GetValueIntegrated(Curves[param.curve].keyframes.back().pos.x);
		float rand =
			std::generate_canonical<float, std::numeric_limits<float>::digits, Generator>(generator) * max_integral;
		float curve_min = lower_bound;
		float curve_max = upper_bound;
		
		for (size_t count = 0; count < 16; count++) {
			float current_pos = (lower_bound + upper_bound) / 2.f;
			float current_value = Curves[param.curve].GetValueIntegrated(current_pos);
			if (fl_equal(current_value, rand, max_integral * 0.01f)) {
				// remap the values to the distribution's range
				return (current_pos - curve_min) / (curve_max - curve_min) * (param.max - param.min) + param.min;
			}
			if (current_value > rand) {
				upper_bound = current_pos;
			} else {
				lower_bound = current_pos;
			}
		}
		// remap the values to the distribution's range
		return (((lower_bound + upper_bound) / 2.f) - curve_min) / (curve_max - curve_min) * (param.max - param.min) +
			   param.min;
	}
	template <typename Generator>
	inline result_type operator()(Generator& generator)
	{
		return this->operator()(generator, m_param);
	}
	inline result_type min() const
	{
		return Curves[m_param.curve].keyframes.front().pos.x;
	}
	inline result_type max() const
	{
		return Curves[m_param.curve].keyframes.back().pos.x;
	}
	inline bool operator==(const CurveNumberDistribution& other) const
	{
		return (m_param.curve == other.m_param.curve && fl_equal(m_param.min, other.m_param.min) && fl_equal(m_param.max, other.m_param.max));
	}
	inline bool operator!=(const CurveNumberDistribution& other) const
	{
		return !(*this == other);
	}
};

using CurveFloatRange = RandomRange<float, CurveNumberDistribution, std::minstd_rand>;

inline CurveFloatRange parseCurveFloatRange(float min = std::numeric_limits<float>::lowest()/2.1f, float max = std::numeric_limits<float>::max()/2.1f) {
	CurveNumberDistribution::param_type curve_params;

	SCP_string curve_name;
	stuff_string(curve_name, F_NAME);
	curve_params.curve = curve_get_by_name(curve_name);

	optional_string("(");
	stuff_float_optional(&curve_params.min);
	stuff_float_optional(&curve_params.max);
	optional_string(")");

	if (curve_params.curve < 0) {
		error_display(0, "Curve %s not found! Random distributions using this curve will return 0.", curve_name.c_str());
		return {curve_params};
	} else {
		bool y_below_0 = false;
		bool no_y_above_0 = true;

		for (const auto& kframe : Curves[curve_params.curve].keyframes) {
			if (kframe.pos.y < 0.f) {
				y_below_0 = true;
			}
			if (kframe.pos.y > 0.f) {
				no_y_above_0 = false;
			}
		}

		if (y_below_0) {
			error_display(0,
				"Curve %s goes below zero along the Y axis. Random distributions using this curve will return 0.", curve_name.c_str());
			curve_params.curve = -1;
			return {curve_params};
		}
		if (no_y_above_0) {
			error_display(0,
				"Curve %s has no values above zero along the Y axis. Random distributions using this curve will return 0.", curve_name.c_str());
			curve_params.curve = -1;
			return {curve_params};
		}
	}

	if (fl_is_nan(curve_params.min) || fl_is_nan(curve_params.max)) {
		if (!fl_is_nan(curve_params.min)) {
			error_display(0, "Minimum value but no maximum value specified for curve distribution %s!", curve_name.c_str());
		}
		curve_params.min = Curves[curve_params.curve].keyframes.front().pos.x;
		curve_params.max = Curves[curve_params.curve].keyframes.back().pos.x;
	}

	if (curve_params.min > curve_params.max) {
		error_display(0, "Minimum value %f is more than maximum value %f!", (float)curve_params.min, (float)curve_params.max);
		std::swap(curve_params.min, curve_params.max);
	}

	if (curve_params.min < min) {
		error_display(0, "First value (%f) is less than the minimum %f!", (float)curve_params.min, (float)min);
		curve_params.min = min;
	}
	if (curve_params.min > max) {
		error_display(0, "First value (%f) is greater than the maximum %f!", (float)curve_params.min, (float)max);
		curve_params.min = max;
	}

	if (curve_params.max < min) {
		error_display(0, "Second value (%f) is less than the minimum %f!", (float)curve_params.max, (float)min);
		curve_params.max = min;
	}
	if (curve_params.max > max) {
		error_display(0, "Second value (%f) is greater than the maximum %f!", (float)curve_params.max, (float)max);
		curve_params.max = max;
	}

	return {curve_params};
}

template<typename result_type>

class ParsedRandomRange {
  public:
	  using variant = mpark::variant<UniformFloatRange, BoundedNormalFloatRange, CurveFloatRange>;
  private:
	variant m_random_range;

	// these could have been one-line auto lambdas if we had C++ 14
	struct next_helper {
		template <typename T>
		inline float operator()(T& range) {
			return range.next();
		}
	};

	struct min_helper {
		template<typename T>
		inline float operator()(T& range) {
			return range.min();
		}
	};

	struct max_helper {
		template <typename T>
		inline float operator()(T& range) {
			return range.max();
		}
	};

	struct seed_helper {
		unsigned int new_seed;

		template <typename T>
		inline void operator()(T& range) {
			range.seed(new_seed);
		}
	};

  public:
	  template<typename T>
	ParsedRandomRange(T&& random_range)
	{
		m_random_range = std::forward<T>(random_range);
	}
	ParsedRandomRange() {
		m_random_range = UniformFloatRange();
	};
	inline result_type next() const {
		return static_cast<result_type>(mpark::visit(next_helper{}, m_random_range));
	}
	inline result_type min() const {
		return static_cast<result_type>(mpark::visit(min_helper{}, m_random_range));
	}
	inline result_type max() const {
		return static_cast<result_type>(mpark::visit(max_helper{}, m_random_range));
	}
	inline void seed(unsigned int new_seed) const {
		mpark::visit(seed_helper{new_seed}, m_random_range);
	}
	static ParsedRandomRange parseRandomRange(float min = std::numeric_limits<float>::lowest()/2.1f, float max = std::numeric_limits<float>::max()/2.1f) {
		switch (optional_string_either("NORMAL", "CURVE")) {
			case 0: {
				return ParsedRandomRange(parseNormalFloatRange(min, max));
			}
			case 1: {
				return ParsedRandomRange(parseCurveFloatRange(min, max));
			}
			default: {
				return ParsedRandomRange(parseUniformRange<float>(min, max));
			}
		}
	}
	template<typename T>
	ParsedRandomRange& operator=(T&& random_range) {
		m_random_range = std::forward<T>(random_range);
		return *this;
	}
};

using ParsedRandomFloatRange = ParsedRandomRange<float>;
using ParsedRandomUintRange = ParsedRandomRange<uint>;

}