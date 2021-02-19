
#include "RocketLuaSystemInterface.h"

#include "scripting/lua/LuaTable.h"

#include <Rocket/Core/ElementDocument.h>
#include <Rocket/Core/ValueReference.h>

namespace scpui {

namespace {

const char* LuaEnvironmentIdentifier = "document_lua_environment";

class LuaTableReference : public Rocket::Core::ValueReference {
  public:
	LuaTableReference(const luacpp::LuaTable& envtable) : _envtable(envtable) {}
	~LuaTableReference() override = default;

	const luacpp::LuaTable& getEnvTable() const
	{
		return _envtable;
	}

  private:
	luacpp::LuaTable _envtable;
};

luacpp::LuaTable createEnvironment(lua_State* L)
{
	// local env = {}
	auto env = luacpp::LuaTable::create(L);

	// local metatable = {}
	auto metatable = luacpp::LuaTable::create(L);

	// metatable.__index = _G (non-local lookups go to the global table)
	metatable.pushValue(L);
	lua_getfield(L, LUA_GLOBALSINDEX, "_G");
	lua_setfield(L, -2, "__index");

	lua_pop(L, 1);

	// Use that simply metatable for the environment
	env.setMetatable(metatable);

	return env;
}

} // namespace

RocketLuaSystemInterface::RocketLuaSystemInterface() = default;
RocketLuaSystemInterface::~RocketLuaSystemInterface() = default;
void RocketLuaSystemInterface::PrepareFunction(lua_State* L, int funcIdx, Rocket::Core::Element* context)
{
	if (context == nullptr) {
		// Nothing we can do in this case
		return;
	}

	// To ensure consistent stack indices even for relative funcIdx values
	auto top = lua_gettop(L);
	lua_pushvalue(L, funcIdx);

	auto envTable =
		static_cast<LuaTableReference*>(context->GetOwnerDocument()->GetUserValue(LuaEnvironmentIdentifier));

	if (envTable == nullptr) {
		auto reference = std::unique_ptr<LuaTableReference>(new LuaTableReference(createEnvironment(L)));
		envTable = reference.get();

		context->GetOwnerDocument()->PutUserValue(LuaEnvironmentIdentifier, std::move(reference));
	}

	// Set the table as the environment of the function
	envTable->getEnvTable().pushValue(L);
	lua_setfenv(L, -2);

	lua_settop(L, top);
}
void RocketLuaSystemInterface::ReportError(lua_State* L, const Rocket::Core::String& location)
{
	if (!location.Empty()) {
		LuaError(L, "Location: %s", location.CString());
	} else {
		LuaError(L);
	}
}
int RocketLuaSystemInterface::ErrorHandler(lua_State* L) {
	return scripting::ade_friendly_error(L);
}

} // namespace scpui
