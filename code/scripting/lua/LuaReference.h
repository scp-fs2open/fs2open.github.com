#ifndef LUA_REFERENCE_H
#define LUA_REFERENCE_H
#pragma once

#include "LuaHeaders.h"

#include <memory>

namespace luacpp {
class UniqueLuaReference;

/**
 * @brief A lua reference
 *
 * Use this if you want to
 */
typedef std::shared_ptr<UniqueLuaReference> LuaReference;

/**
 * @brief A lua-value reference.
 *
 * Wraps a reference to a lua-value. This object may only be owned by one owner. If you need reference sharing then
 * LuaReference should be used.
 */
class UniqueLuaReference {
 private:
	lua_State* _luaState;
	int _reference;

 public:
	/**
    * @brief Initializes a lua reference.
    *
    * Sets the lua_State which will be used to hold the reference and the actual reference value.
    *
    * @param state The lua_State where the reference points to a value.
    * @param reference The reference value, should be >= 0.
    *
    * @warning Do not call this directly. Use UniqueLuaReference::create instead.
    */
	UniqueLuaReference(lua_State* state, int reference);

	/**
    * @brief Creates a lua-reference.
    *
    * Copies the value at @c position and creates a reference to it. Also replaces
    * the value at that position with the copied value.
    *
    * @param state The state to create the reference in.
    * @param position The stack position of the value, defaults to the top of the stack (-1).
    * @return The UniqueLuaReference instance which got created.
    */
	static LuaReference create(lua_State* state, int position = -1);

	/**
     * @brief Copies another lua reference
     * There is no copy-constructor as unintentional copying could lead to excessive creation and deletion of lua references
     *
     * @param other The other pointer
     * @return luacpp::UniqueLuaReferencePtr The new reference pointer
     */
	static LuaReference copy(const LuaReference& other);

	/**
    * @brief Default constructor, initializes an invalid reference
    */
	UniqueLuaReference();

	/**
    * @brief Releases the lua reference.
    */
	~UniqueLuaReference();

	UniqueLuaReference(const UniqueLuaReference&) = delete;
	UniqueLuaReference& operator=(const UniqueLuaReference&) = delete;

	UniqueLuaReference(UniqueLuaReference&& other) noexcept;
	UniqueLuaReference& operator=(UniqueLuaReference&& other) noexcept;

	lua_State* getState() { return _luaState; }

	/**
    * @brief Gets the actual reference number.
    * @return The reference number
    */
	int getReference() const;

	/**
    * @brief Checks if the reference is valid.
    * @return @c true when valid, @c false otherwise.
    */
	bool isValid() const;

	/**
    * @brief Removes the Lua reference
    * @return @c true when the reference was removed @c false otherwise
    */
	bool removeReference();

	/**
    * @brief Pushes the referenced value onto the stack.
    * @param thread A specific thread state to push the value to. nullptr for the default state of this reference
    */
	void pushValue(lua_State* thread, bool guaranteeStack = false) const;
};
}

#endif // LUA_REFERENCE_H
