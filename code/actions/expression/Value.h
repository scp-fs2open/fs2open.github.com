#pragma once

#include "globalincs/pstypes.h"

#include <mpark/variant.hpp>

namespace actions {
namespace expression {

namespace detail {
template <typename ToType>
struct implicit_conversion {
	template <typename T>
	ToType operator()(const T&)
	{
		throw std::runtime_error("Invalid implicit conversion.");
	}
};

template<>
struct implicit_conversion<float> {
	inline float operator()(int val) {
		return i2fl(val);
	}

	template <typename T>
	float operator()(const T&)
	{
		throw std::runtime_error("Invalid implicit conversion.");
	}
};

} // namespace detail

enum class ValueType
{
	Invalid,
	Integer,
	Float,
	Vector,
	String,
};

template <typename T>
struct ValueTypeTraits;

template <>
struct ValueTypeTraits<int> {
	static constexpr ValueType type = ValueType::Integer;
};
template <>
struct ValueTypeTraits<float> {
	static constexpr ValueType type = ValueType::Float;
};
template <>
struct ValueTypeTraits<vec3d> {
	static constexpr ValueType type = ValueType::Vector;
};
template <>
struct ValueTypeTraits<SCP_string> {
	static constexpr ValueType type = ValueType::String;
};

class Value {
  public:
	Value() = default;

	explicit Value(int val);
	explicit Value(float val);
	explicit Value(const vec3d& val);
	explicit Value(SCP_string val);

	ValueType getType() const;

	int getInteger() const;
	float getFloat() const;
	vec3d getVector() const;
	SCP_string getIdentifier() const;

	template <typename T>
	T get() const
	{
		const auto currentType = getType();
		const auto expectedType = ValueTypeTraits<T>::type;

		if (currentType == expectedType) {
			// No conversion necessary
			return mpark::get<T>(m_value);
		}

		return mpark::visit(detail::implicit_conversion<T>{}, m_value);
	}

	friend bool operator==(const Value& lhs, const Value& rhs);
	friend bool operator!=(const Value& lhs, const Value& rhs);

	friend std::ostream& operator<<(std::ostream& os, const Value& value);

  private:
	mpark::variant<mpark::monostate, int, float, vec3d, SCP_string> m_value;
};

} // namespace expression
} // namespace actions
