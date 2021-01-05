
#include "Value.h"
#include "math/vecmat.h"

namespace actions {
namespace expression {

Value::Value(int val) : m_value(val) {}
Value::Value(float val) : m_value(val) {}
Value::Value(const vec3d& val) : m_value(val) {}
Value::Value(SCP_string val) : m_value(std::move(val)) {}

ValueType actions::expression::Value::getType() const
{
	switch (m_value.index()) {
	case 1:
		return ValueType::Integer;
	case 2:
		return ValueType::Float;
	case 3:
		return ValueType::Vector;
	case 4:
		return ValueType::Identifier;
	default:
		return ValueType::Invalid;
	}
}

int Value::getInteger() const
{
	return mpark::get<int>(m_value);
}
float Value::getFloat() const
{
	return mpark::get<float>(m_value);
}
vec3d Value::getVector() const
{
	return mpark::get<vec3d>(m_value);
}
SCP_string Value::getIdentifier() const
{
	return mpark::get<SCP_string>(m_value);
}
bool operator==(const Value& lhs, const Value& rhs)
{
	return lhs.m_value == rhs.m_value;
}
bool operator!=(const Value& lhs, const Value& rhs)
{
	return !(rhs == lhs);
}

} // namespace expression
} // namespace actions
