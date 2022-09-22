#include "LuaThread.h"

#include "LuaException.h"

#include <utility>

#include "scripting/scripting.h"

namespace luacpp {

SCP_unordered_set<LuaThread*> LuaThread::threads;
SCP_unordered_set<LuaThread*>& LuaThread::registerThreadList(lua_State* mainThread) {
	script_state::GetScriptState(mainThread)->OnStateDestroy.add([](lua_State* L) {
		for (LuaThread* thread : threads)
			thread->getReference()->removeReference();
		threads.clear();
		});
	return threads;
}

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
LuaThread::LuaThread(lua_State* luaState, lua_State* thread) : LuaValue(luaState), _thread(thread) {
	//This is kinda hacky and assumes that the first thread created must always be from the main thread.
	static auto& threadList = registerThreadList(luaState);
	threadList.emplace(this);
}

LuaThread::LuaThread(LuaThread&& other) noexcept : LuaValue(other) {
	*this = std::move(other);
}

LuaThread& LuaThread::operator=(LuaThread&& other) noexcept {
	LuaValue::operator=(other);
	std::swap(_errorCallback, other._errorCallback);
	std::swap(_thread, other._thread);
	threads.emplace(this);
	return *this;
}

LuaThread::~LuaThread() {
	//Safety check since erasing from the half-dead threads set is gonna cause segfaults when the whole program deallocates, as dangling threads outlive the static threads map
	if(!threads.empty())
		threads.erase(this);
}

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
