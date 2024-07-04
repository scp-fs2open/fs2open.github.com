//
//

#include "LuaEventCallback.h"
#include "scripting/lua/LuaConvert.h"

using namespace luacpp;

namespace scripting {
namespace api {
namespace util {

void convert_arg(lua_State* L, luacpp::LuaValueList& out, object* objp)
{
	LuaValue val;
	ade_set_object_with_breed(L, OBJ_INDEX(objp));
	convert::popValue(L, val);

	out.push_back(val);
}

} // namespace util
} // namespace api
} // namespace scripting
