#pragma once

#include "globalincs/pstypes.h"

// Our Assert conflicts with the definitions inside libRocket
#pragma push_macro("Assert")
#undef Assert

#include <Rocket/Core/Element.h>
#include <Rocket/Core/Lua/LuaSystemInterface.h>

#pragma pop_macro("Assert")

namespace scpui {

class RocketLuaSystemInterface : public Rocket::Core::Lua::LuaSystemInterface {
  public:
	RocketLuaSystemInterface();
	~RocketLuaSystemInterface() override;

	void PrepareFunction(lua_State* L, int funcIdx, Rocket::Core::Element* context) override;

	void ReportError(lua_State* L, const Rocket::Core::String& location) override;

	int ErrorHandler(lua_State* L) override;
};

} // namespace scpui
