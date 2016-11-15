#ifndef LUATABLE_H
#define LUATABLE_H
#pragma once

#include "LuaConvert.h"
#include "LuaValue.h"

namespace luacpp {
class LuaTable;

/**
* @brief An iterator for a lua table.
*
* Provides a way to iterate over the contents of a lua table:
*
* @see luacpp::util::tableListPairs for an example on how to use this
*/
class LuaTableIterator {
 public:
	/**
    * @brief Moves to the next key-value pair of the table.
    *
    * @return @c true when a key-value pair is available, @c false otehrwise
    */
	bool toNext();
 private:
	LuaTableIterator(LuaTable* _parent);

	LuaTable* _parent;

	friend class LuaTable;
};

/**
* @brief Class to improve handling of lua tables.
*
* This class provides a high-level interface to lua tables without the need to directly call
* lua-API functions.
*
* @see LuaConvert
*/
class LuaTable: public LuaValue {
 public:
	/**
    * @brief Creates a new empty table.
    */
	static LuaTable create(lua_State* state);

	/**
     * @brief Default constructor
     */
	LuaTable();

	/**
     * @brief Copy-constructor
     * @param other The other table.
     */
	LuaTable(const LuaTable& other);

	/**
     * Dereferences the stored reference to the table if it exists.
     */
	virtual ~LuaTable();

	/**
     * @brief Sets the metatable.
     *
     * The given table will be set as the metatable of this table.
     *
     * @param ref The new metatable.
     * @return Always `true`.
     */
	bool setMetatable(const LuaTable& ref);

	/**
     * @brief Sets a new reference.
     * This overload checks if the passed reference is a table
     *
     * @param ref The new reference
     * @return void
     */
	void setReference(LuaReference ref) override;

	/**
     * @brief Adds a value to this lua table.
     *
     * @param index The index value to use.
     * @param value The value to set at the index.
     */
	template<class IndexType, class ValueType>
	void addValue(const IndexType& index, const ValueType& value) {
		// Push the table onto the stack by using the reference
		this->pushValue();

		// Push the index and value onto the stac by using the template functions
		convert::pushValue(_luaState, index);
		convert::pushValue(_luaState, value);

		// Set the value in the table
		lua_settable(_luaState, -3);

		// And pop the table again
		lua_pop(_luaState, 1);
	}

	/**
     * @brief Retrieves a value from the table.
     *
     * @param index The index where the value is located.
     * @param target The target location where the value should be stored.
     * @return @c true when the value could be successfully converted, @c false otherwise
     */
	template<class IndexType, class ValueType>
	bool getValue(const IndexType& index, ValueType& target) {
		this->pushValue();

		convert::pushValue(_luaState, index);

		lua_gettable(_luaState, -2);

		bool ret = convert::popValue(_luaState, target);

		if (!ret) {
			lua_pop(_luaState, 1);
		}

		lua_pop(_luaState, 1);

		return ret;
	}

	/**
     * @brief Gets a value or throws an exception.
     * This function gets the specified value from the table and returns it
     * or throws an exception if that faild
     *
     * @param index The index of the value to retrieve
     * @return luacpp::ValueType The value
     *
     * @exception LuaException Thrown when an error occurs while converting the value
     */
	template<class ValueType, class IndexType>
	// IndexType is last so the compiler can deduce it from the argument
	ValueType getValue(const IndexType& index) {
		ValueType target;

		if (!getValue(index, target)) {
			throw LuaException("Failed to get lua value!");
		} else {
			return target;
		}
	}

	/**
     * @brief Gets the length of the table.
     *
     * Gets size of the table (the same as using the '#' operator in Lua).
     *
     * @return The size value.
     */
	size_t getLength();

	/**
     * @brief Creates LuaTableITerator to iterate over the contents of the table.
     * @return The iterator instance.
     */
	LuaTableIterator iterator();
};
}

#endif
