#include "LuaThread.h"

#include "LuaException.h"

#include <utility>

namespace luacpp {
LuaThread LuaThread::create(lua_State* L, const LuaFunction& func)
{
	auto luaThread = lua_newthread(L);

	LuaThread thread(L, luaThread);
	thread.setReference(UniqueLuaReference::create(L));

	lua_pop(L, 1);

	//Make sure that the C++-side reference of the LuaThread is cleared when its parents references are auto-garbage collected by lua for whatever reason (usually due to the parent thread dying).
	//To do this, register an object with a __gc method in the thread (usually we'd want to directly attach it to the thread, but only userdata will have __gc methods called).
	//Usually we'd want to do this when the childs references are GC'd, but creating tables on child threads from C causes tests to fail for some reason.
	auto threadRef = std::weak_ptr<UniqueLuaReference>(thread.getReference());
	int stack = lua_gettop(L);

	//Make sure to create the function that clears the LuaThread reference BEFORE creating the userdata value, otherwise the function will be garbage-collected itself before it's called.
	thread.deleterFunc = LuaFunction::createFromStdFunction(L, [threadRef](lua_State*, const LuaValueList&) -> LuaValueList {
		if(!threadRef.expired())
			threadRef.lock()->removeReference();
		return {};
		});

	//NOW, create the dummy userdata, and its metatable
	lua_newuserdata(L, sizeof(bool));
	thread.deleterUserdata = UniqueLuaReference::create(L);
	thread.deleterTable = LuaTable::create(L);

	//Since we hold references to all we need, tidy up the stack.
	lua_settop(L, stack);

	//Push the values in the correct order for assembly. Userdata, then table, then func
	thread.deleterUserdata->pushValue(L);
	thread.deleterTable.pushValue(L);
	thread.deleterFunc.pushValue(L);
	lua_setfield(L, -2, "__gc"); //Takes the top value (func) and assigns it to the second to last one (table) as __gc, and pops the top value
	lua_setmetatable(L, -2); //Takes the top value (table) and assigns it as the metadata table of the second to last one (userdata), and pops the last value
	lua_pop(L, 1); //Pop the userdata

	func.pushValue(L);
	// Move the main function to the thread (I have no idea what this actually does but the Lua code does the same...)
	lua_xmove(L, luaThread, 1);

	return thread;
}

LuaThread::LuaThread() = default;
LuaThread::LuaThread(lua_State* luaState, lua_State* thread) : LuaValue(luaState), _thread(thread) {}

LuaThread::LuaThread(LuaThread&&) = default;
LuaThread& LuaThread::operator=(LuaThread&&) = default;

LuaThread::~LuaThread() = default;

void LuaThread::setReference(const LuaReference& ref)
{
	lua_State* L = ref->getState();

	ref->pushValue(L);

	if (lua_type(L, -1) != LUA_TTHREAD) {
		lua_pop(L, 1);
		throw LuaException("Reference does not refer to a thread!");
	} else {
		lua_pop(L, 1);
		LuaValue::setReference(ref);
	}
}
LuaThread::ResumeState LuaThread::resume(const LuaValueList& params) const
{
	int nargs = static_cast<int>(params.size());
	for (const auto& val : params) {
		val.pushValue(_luaState);
	}

	// Move parameters to the thread
	lua_xmove(_luaState, _thread, nargs);

	// now resume
	const auto result = lua_resume(_thread, nargs);

	if (result != 0 && result != LUA_YIELD) {
		if (_errorCallback) {
			if (_errorCallback(_luaState, _thread)) {
				// If the error was handled just pretend that we resumed successfully
				ResumeState resume;
				resume.completed = true;
				return resume;
			}
		}

		throw LuaException("Lua coroutine failed to resume!");
	}

	LuaValueList returnVals;
	auto numRet = lua_gettop(_thread);
	returnVals.reserve(numRet);

	// Move the values back to the main state so that it uses the right state
	auto previousStack = lua_gettop(_luaState); // Keep this for cleaning up later
	lua_xmove(_thread, _luaState, numRet);

	auto retValsStart = previousStack + 1; // We need to start at + 1 since that is where the first return val would be
	for (int i = retValsStart; i <= previousStack + numRet; ++i) {
		LuaValue val;
		convert::popValue(_luaState, val, i, false);
		returnVals.push_back(std::move(val));
	}

	// Cleanup the stack
	lua_settop(_luaState, previousStack);

	ResumeState state;
	state.completed  = result != LUA_YIELD;
	state.returnVals = std::move(returnVals);
	return state;
}
void LuaThread::setErrorCallback(LuaThread::ErrorCallback errorCallback) { _errorCallback = std::move(errorCallback); }

lua_State* LuaThread::getThreadHandle() const { return _thread; }

bool convert::popValue(lua_State* luaState, LuaThread& target, int stackposition, bool remove)
{
	if (!internal::isValidIndex(luaState, stackposition)) {
		return false;
	}

	if (!lua_isthread(luaState, stackposition)) {
		return false;
	} else {
		target.setReference(UniqueLuaReference::create(luaState, stackposition));

		if (remove) {
			lua_remove(luaState, stackposition);
		}

		return true;
	}
}

} // namespace luacpp
