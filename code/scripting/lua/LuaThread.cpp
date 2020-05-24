#include "LuaThread.h"

#include "LuaException.h"

#include <utility>

namespace luacpp {
LuaThread LuaThread::create(lua_State* L, const LuaFunction& func)
{
	auto luaThread = lua_newthread(L);
	func.pushValue(L);
	// Move the main function to the thread (I have no idea what this actually does but the Lua code does the same...)
	lua_xmove(L, luaThread, 1);

	LuaThread thread(L, luaThread);
	thread.setReference(UniqueLuaReference::create(L));

	lua_pop(L, 1);

	return thread;
}

LuaThread::LuaThread() = default;
LuaThread::LuaThread(lua_State* luaState, lua_State* thread) : LuaValue(luaState), _thread(thread) {}

LuaThread::LuaThread(const LuaThread&) = default;
LuaThread& LuaThread::operator=(const LuaThread&) = default;

// These should be noexcept but Visual Studio doesn't like that yet in a recent enough version
LuaThread::LuaThread(LuaThread&&) = default; // NOLINT(performance-noexcept-move-constructor)
LuaThread& LuaThread::operator=(LuaThread&&) = default; // NOLINT(performance-noexcept-move-constructor)

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
