#pragma once

#include "scripting/lua/LuaTypes.h"

namespace scripting {
namespace api {

/**
 * @brief A context that manages an asynchronous operation
 *
 * This is the interface used by a LuaPromise for running pending promises. It gets passed a callback that should be
 * called when a promise has resolved.
 */
class resolve_context {
  public:
	virtual ~resolve_context();

	using Resolver = std::function<void(const luacpp::LuaValueList& resolveVals)>;

	/**
	 * @brief Sets the function which is called when an asynchronous operation finishes
	 *
	 * @note For future compatibility, the operation represented by a subclass of this should not start executing until
	 * this function is called.
	 *
	 * @param resolver The function to call to resolve a coroutine
	 */
	virtual void setResolver(Resolver resolver) = 0;
};

class LuaPromise {
  public:
	using ContinuationFunction = std::function<luacpp::LuaValueList(const luacpp::LuaValueList& resolveVals)>;

	/**
	 * @brief Initializes an invalid promise
	 */
	LuaPromise();

	/**
	 * @brief Creates a resolved promise
	 * @param resolveValue The value with which the promise resolves
	 */
	explicit LuaPromise(luacpp::LuaValueList resolveValue);

	/**
	 * @brief Creates a pending promise
	 * @param resolveContext The context to register on
	 */
	explicit LuaPromise(std::shared_ptr<resolve_context> resolveContext);

	LuaPromise(const LuaPromise&);
	LuaPromise& operator=(const LuaPromise&);

	LuaPromise(LuaPromise&&) noexcept;
	LuaPromise& operator=(LuaPromise&&) noexcept;

	LuaPromise then(ContinuationFunction continuation);

	bool isValid() const;

	bool isResolved() const;

	const luacpp::LuaValueList& resolveValue() const;

  private:
	struct internal_state;
	std::shared_ptr<internal_state> m_state;
};

} // namespace api
} // namespace scripting
