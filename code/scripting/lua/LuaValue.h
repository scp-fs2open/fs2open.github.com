
#ifndef LUAVALUE_H
#define LUAVALUE_H
#pragma once

#include "LuaConvert.h"
#include "LuaReference.h"
#include "LuaException.h"

#include "LuaHeaders.h"

#include <memory>

#include <cstdint>
#include <csetjmp>

namespace luacpp {
/**
 * @brief The values of a LuaValue
 */
enum class ValueType {
	NONE,
	NIL,
	BOOLEAN,
	LIGHTUSERDATA,
	STRING,
	NUMBER,
	TABLE,
	FUNCTION,
	USERDATA,
	THREAD,
};

/**
 * @brief Represents a Lua-value
 *
 * This class holds a reference to a lua value and provides type checking to ensure that the value is still the same.
 */
class LuaValue {
 public:
	/**
     * @brief Creates a LuaValue.
     * This is done by pushing the value onto the lua stack and creating a reference to it.
     *
     * @param state The lua state
     * @param value The value to be referenced
     * @return luacpp::LuaValue A new LuaValue instance which references the specified value
     */
	template<class ValueType>
	static LuaValue createValue(lua_State* state, ValueType&& value) {
		LuaValue retVal(state);

		convert::pushValue(state, std::forward<ValueType>(value));

		retVal.setReference(UniqueLuaReference::create(state));

		// Remove the value again
		lua_pop(state, 1);

		return retVal;
	}

	static LuaValue createNil(lua_State* L);

	/**
	 * @brief Default constructor, creates an invalid LuaValue
	 */
	LuaValue();

	/**
	 * @brief Initializes the lua value
	 *
	 * The instance does not point to a lua-value after the constructor has finished. To reference
	 * a value use #setReference(LuaReferencePtr)
	 *
	 * @param state The lua state
	 */
	LuaValue(lua_State* state);

	/**
	 * @brief Copy-constructor
	 * @param other The other LuaValue.
	 */
	LuaValue(const LuaValue& other);
	LuaValue& operator=(const LuaValue& other);

	/**
	 * @brief Move-constructor
	 * @param other The other LuaValue.
	 */
	LuaValue(LuaValue&& other) noexcept;
	LuaValue& operator=(LuaValue&& other) noexcept;

	/**
	 * @brief Releases the reference
	 */
	virtual ~LuaValue();

	/**
	 * @brief Sets a new LuaReference.
	 *
	 * @param ref The new lua reference.
	 */
	virtual void setReference(const LuaReference& ref);

	/**
     * @brief Gets the LuaReference.
     *
     * This reference is used to reference the actual lua value.
     *
     * @return The LuaReference instance.
     */
	virtual const LuaReference getReference() const;

	/**
     * @brief Gets the lua type of this value.
     * @return One of the LUA_T* defines.
     */
	virtual ValueType getValueType() const;

	/**
     * @brief Checks if the value is of the specified type.
     *
     * @return bool @c true when it is, @c false if it isn't.
     */
	inline bool is(ValueType check) const { return _luaType == check; }

	/**
     * @brief Sets a new value, possible changing the type
     *
     * @param value The new value
     */
	template<class Type>
	void setValue(const Type& value) {
		// Push the new value
		convert::pushValue(_luaState, value);

		// And create a reference for it
		setReference(UniqueLuaReference::create(_luaState));
	}

	/**
     * @brief Gets the value or throws an exception
     *
     * @return Type The value
     *
     * @exception LuaException Thrown when the conversion failed.
     */
	template<class Type>
	Type getValue() const {
		_reference->pushValue(_luaState);

		Type target;
		if (!convert::popValue(_luaState, target)) {
			lua_pop(_luaState, 1);
			throw LuaException("Failed to pop value");
		} else {
			return target;
		}
	}

	/**
	 * @brief Same as above but allows passing a reference to a value where to store the lua value
	 *
	 * @exception LuaException Thrown when the conversion failed.
	 */
	template<typename Type>
	void getValue(Type&& od) const {
		_reference->pushValue(_luaState);

		if (!convert::popValue(_luaState, std::forward<Type>(od))) {
			lua_pop(_luaState, 1);
			throw LuaException("Failed to pop value");
		}
	}

	/**
     * @brief Specifies if the lua value is valid.
     *
     * @return bool @c true if it can be used and have an underlying reference, @c false otherwise.
     */
	bool isValid() const;

	/**
	 * @brief Pushes this lua value onto the stack.
	 * @param thread The thread stack onto which this value should be pushed. May be nullptr for the default state of
	 * this value
	 */
	bool pushValue(lua_State* thread, bool guaranteeStack = false) const;

	lua_State* getLuaState() const;

  protected:
	lua_State* _luaState{nullptr}; //!< The lua state of this value.

	LuaReference _reference;

	ValueType _luaType = ValueType::NONE;
};

/**
* @brief Checks for equality of the lua values.
* @param lhs The left value
* @param rhs The right lua value.
* @return `true` when the value are equal as specified by the lua "==" operator.
*/
template<typename Type>
bool operator==(const LuaValue& lhs, const Type& rhs) {
	lhs.pushValue(lhs.getLuaState());
	convert::pushValue(lhs.getLuaState(), rhs);

	bool result = lua_equal(lhs.getLuaState(), -2, -1) != 0;

	lua_pop(lhs.getLuaState(), 2);

	return result;
}

/**
* @brief Checks if the other value is bigger than this value.
* @param lhs The left lua value.
* @param rhs The right lua value.
* @return `true` when the second value is bigger than this value as specified
* 			by lua.
*/
template<typename Type>
bool operator<(const LuaValue& lhs, const Type& rhs) {
	lhs.pushValue(lhs.getLuaState());
	convert::pushValue(lhs.getLuaState(), rhs);

	bool result = lua_lessthan(lhs.getLuaState(), -2, -1) != 0;

	lua_pop(lhs.getLuaState(), 2);

	return result;
}

template<typename Type>
bool operator!=(const LuaValue& lhs, const Type& rhs) {
	return !(lhs == rhs);
}

template<typename Type>
bool operator>(const LuaValue& lhs, const Type& rhs) {
	return !(lhs <= rhs);
}

template<typename Type>
bool operator<=(const LuaValue& lhs, const Type& rhs) {
	return (lhs == rhs) || (lhs < rhs);
}

template<typename Type>
bool operator>=(const LuaValue& lhs, const Type& rhs) {
	return !(lhs < rhs);
}

namespace convert {

void pushValue(lua_State* luaState, const LuaValue& value);

bool popValue(lua_State* luaState, LuaValue& target, int stackposition = -1, bool remove = true);

}
}

#endif
