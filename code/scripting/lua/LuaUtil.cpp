
#include "LuaUtil.h"

namespace luacpp {
namespace util {
const char* getValueName(ValueType type) {
	switch (type) {
		case ValueType::NONE:
			return "none";
		case ValueType::NIL:
			return "nil";
		case ValueType::BOOLEAN:
			return "boolean";
		case ValueType::LIGHTUSERDATA:
			return "light userdata";
		case ValueType::STRING:
			return "string";
		case ValueType::NUMBER:
			return "number";
		case ValueType::TABLE:
			return "table";
		case ValueType::FUNCTION:
			return "function";
		case ValueType::USERDATA:
			return "userdata";
		case ValueType::THREAD:
			return "thread";
		default:
			return "unknown";
	}
}
}
}
