#ifndef LUATABLE_H
#define LUATABLE_H
#pragma once

#include "LuaConvert.h"
#include "LuaValue.h"

#include <iterator>

namespace luacpp {
class LuaTable;

/**
 * @brief Utility class for iterating over the contents of a lua table
 *
 * The elements are returned as key-value pairs of the table
 *
 * @warning Never manipulate the lua stack while using an instance of this class! It's ok to change the stack but it must
 * be in the exact same state when you call the next method of this class.
 */
class LuaTableIterator {
	lua_State* _luaState = nullptr;

	int _stackTop = 0;

	bool _hasElement = false;

	std::pair<LuaValue, LuaValue> _currentVal;
 public:
	/**
	 * @brief Initializes the iterator with a table
	 * @param t The table to iterate iver
	 */
	explicit LuaTableIterator(const LuaTable& t);
	~LuaTableIterator();

	LuaTableIterator(const LuaTableIterator&) = delete;
	LuaTableIterator& operator=(const LuaTableIterator&) = delete;

	/**
	 * @brief Test if there is a valid element
	 * @return @c true if there is a value, @c false otherwise
	 */
	bool hasElement();

	/**
	 * @brief Advances the iterator to the next element
	 */
	void toNextElement();

	/**
	 * @brief Gets the key-value pair of the current element
	 * @return The key-value pair
	 */
	std::pair<LuaValue, LuaValue> getElement();
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
	 * @brief STL adaptor for LuaTableIterator
	 *
	 * This wraps a shared pointer to an iterator so that this class can be copied like the iterator interface expects
	 */
	class iterator: public std::iterator<std::input_iterator_tag, std::pair<LuaValue, LuaValue>> {
	 public:
		explicit iterator(const LuaTable& _parent);
		iterator();

		bool operator==(const iterator& other);
		bool operator!=(const iterator& other);

		iterator& operator++();
		iterator& operator++(int);

		std::pair<LuaValue, LuaValue> operator*();
	 private:
		std::shared_ptr<LuaTableIterator> _iter;
		bool _atEnd = false;
	};

	typedef iterator iterator_type;

	/**
    * @brief Creates a new empty table.
    */
	static LuaTable create(lua_State* state);

	/**
     * @brief Default constructor
     */
	LuaTable();

	/**
     * Dereferences the stored reference to the table if it exists.
     */
	~LuaTable() override;

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
	void setReference(const LuaReference& ref) override;

	/**
     * @brief Adds a value to this lua table.
     *
     * @param index The index value to use.
     * @param value The value to set at the index.
     */
	template<class IndexType, class ValueType>
	void addValue(IndexType&& index, ValueType&& value) {
		// Push the table onto the stack by using the reference
		this->pushValue(_luaState);

		// Push the index and value onto the stac by using the template functions
		convert::pushValue(_luaState, std::forward<IndexType>(index));
		convert::pushValue(_luaState, std::forward<ValueType>(value));

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
	bool getValue(const IndexType& index, ValueType& target) const {
		this->pushValue(_luaState);

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
	ValueType getValue(const IndexType& index) const {
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
	size_t getLength() const;

	/**
	 * @brief Returns an iterator to the begin of this table
	 *
	 * This can be used with STL algorithms or with the C++11 range-base-for-loop
	 *
	 * @warning Never manipulate the lua stack while using this iterator!
	 *
	 * @return The iterator to the begin of the table
	 */
	iterator begin() const;

	/**
	 * @brief Returns an iterator to the end of this table
	 *
	 * This can be used with STL algorithms or with the C++11 range-base-for-loop
	 *
	 * @return The iterator to the end of the table
	 */
	iterator end() const;

};

namespace convert {

bool popValue(lua_State* luaState, LuaTable& target, int stackposition = -1, bool remove = true);

}
}

#endif
