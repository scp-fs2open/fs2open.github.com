
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
		return ValueType::String;
	default:
		return ValueType::Invalid;
	}
}

int Value::getInteger() const
{
	return get<int>();
}
float Value::getFloat() const
{
	return get<float>();
}
vec3d Value::getVector() const
{
	return get<vec3d>();
}
SCP_string Value::getIdentifier() const
{
	return get<SCP_string>();
}
bool operator==(const Value& lhs, const Value& rhs)
{
	return lhs.m_value == rhs.m_value;
}
bool operator!=(const Value& lhs, const Value& rhs)
{
	return !(rhs == lhs);
}
std::ostream& operator<<(std::ostream& os, const Value& value)
{
	os << "Value<";

	switch (value.getType()) {
	case ValueType::Invalid:
		os << "INVALID";
		break;
	case ValueType::Integer:
		os << value.get<int>();
		break;
	case ValueType::Float:
		os << value.get<float>();
		break;
	case ValueType::Vector:
		os << value.get<vec3d>();
		break;
	case ValueType::String:
		os << value.get<SCP_string>();
		break;
	}

	os << ">";
	return os;
}

} // namespace expression
} // namespace actions
