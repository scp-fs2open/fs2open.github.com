#pragma once

#include "globalincs/pstypes.h"

#include <mpark/variant.hpp>

#include <ostream>

namespace actions {
namespace expression {

enum class ValueType
{
	Invalid,
	Integer,
	Float,
	Vector,
	Identifier,
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

	template<typename T>
	const T& get() const {
		return mpark::get<T>(m_value);
	}

	friend bool operator==(const Value& lhs, const Value& rhs);
	friend bool operator!=(const Value& lhs, const Value& rhs);

	friend std::ostream& operator<<(std::ostream& os, const Value& value);

  private:
	mpark::variant<mpark::monostate, int, float, vec3d, SCP_string> m_value;
};

} // namespace expression
} // namespace actions
