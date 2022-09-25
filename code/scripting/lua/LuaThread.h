#pragma once

#include "LuaConvert.h"
#include "LuaTypes.h"
#include "LuaValue.h"
#include "LuaFunction.h"

#include "globalincs/vmallocator.h"

#include <iterator>

namespace luacpp {

/**
 * @brief Class to improve handling of lua threads (aka coroutines).
 *
 * This class provides a high-level interface to lua threads.
 *
 * @see LuaConvert
 */
class LuaThread : public LuaValue {

	LuaReference deleterUserdata;
	LuaTable deleterTable;
	LuaFunction deleterFunc;
  public:
	using ErrorCallback = std::function<bool(lua_State* mainState, lua_State* thread)>;
	struct ResumeState
	{
		bool completed = false;
		LuaValueList returnVals;
	};

	/**
	 * @brief Creates a new empty thread.
	 */
	static LuaThread create(lua_State* L, const LuaFunction& func);

	/**
	 * @brief Default constructor
	 */
	LuaThread();

	//Copying threads is VERY illegal
	LuaThread(const LuaThread&) = delete;
	LuaThread& operator=(const LuaThread&) = delete;

	LuaThread(LuaThread&&) noexcept; 
	LuaThread& operator=(LuaThread&&) noexcept;

	~LuaThread() override;

	/**
	 * @brief Sets a new reference.
	 * This overload checks if the passed reference is a thread
	 *
	 * @param ref The new reference
	 * @return void
	 */
	void setReference(const LuaReference& ref) override;

	void setErrorCallback(ErrorCallback errorCallback);

	ResumeState resume(const LuaValueList& params) const;

	lua_State* getThreadHandle() const;
  private:
	LuaThread(lua_State* luaState, lua_State* threadHandle);

	lua_State* _thread = nullptr;
	ErrorCallback _errorCallback;
};

namespace convert {

bool popValue(lua_State* luaState, LuaThread& target, int stackposition = -1, bool remove = true);

}
} // namespace luacpp
