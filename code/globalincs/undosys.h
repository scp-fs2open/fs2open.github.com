/*
 * z64555's Undo system, created for the FreeSpace Source Code project
 *
 * Released under Creative Commons Attribution-ShareAlike 4.0 International 
 * https://creativecommons.org/licenses/by-sa/4.0/
 */

#include "globalincs/pstypes.h"
#include "globalincs/vmallocator.h"

#include <utility>

/*!
 * @brief Base class of Undo_item. Use this when making an undo stack!
 * @sa Undo_item
 */
class Undo_item_base
{
public:
	virtual ~Undo_item_base() = 0;
	virtual std::pair<const void*, const void*> restore() = 0;
};


/*!
 * @brief Class which handles undo operations.
 * @details This class works with a single data item, to make a undo or redo stack you'd use something like a
 *   std::vector or a std::deque of this class. Undo and Redo stacks would be seperate. Before restoring data from
 *   either, you'd save a copy of the item in the opposite stack.
 *
 *   For example, if you're about to restore from the undo stack, you'd first save a copy of the current data to the
 *   redo stack, then do the undo. The Undo_item in the undo stack can then be popped off the stack.
 *
 * @note If you want to save changes to an entire C-style array (such as 'int array[]'), try wrapping it in a std::array first.
 *
 * @sa Undo_item_base
 */
template<typename T> class Undo_item : Undo_item_base
{
public:
	Undo_item()
		: dest(nullptr), data(nullptr) {};

	Undo_item(T& _data) {
		save(_data);
	};

	Undo_item(T& _data, T* _cont) {
		save(_data, _cont);
	};

	~Undo_item() {
		if (data != nullptr) {
			delete data;
		}
	};

	/**
	 * @brief Restores the saved data
	 *
	 * @returns A pair of const void* referencing the affected item and its container (if it was provided)
	 * @details If successful, the data at dest has been swapped. So if this was an undo item, it's now a redo item
	 */
	std::pair<const void*, const void*> restore() {
		Assert((dest != nullptr) && (data != nullptr));

		T copy = *static_cast<T*>(dest);
		*static_cast<T*>(dest) = *static_cast<T*>(data);
		*static_cast<T*>(data) = copy;

		return std::pair<const void*, const void*>(dest, container);
	};

	/*!
	 * @brief Saves data and its destination as an undo item
	 *
	 * @param[in] _data The data to save
	 * @param[in] _cont The container this data is originally found in
	 * @details The explicit definition is here to ensure type safety
	 */
	void save(T& _data, T* _cont = nullptr) {
		Assert(_data != nullptr);

		dest = *_data;
		data = new T(_data);
		container = _cont;
	};

private:
	T* dest;        //!< Destination of the data
	T* data;        //!< Reference to a copy of data on the heap
	T* container;   //!< Optional reference to this item's container
};


/*!
 * @brief Generic Undo/Redo system. Save whatever, restore whatever!
 *
 * @details Currently uses a pair of deques. Would ideally use a ring container so that the actual Undo_item instances don't go anywhere.
 */
class Undo_system
{
public:
	Undo_system();

	Undo_system(uint _undos);

	/*!
	 * @brief Saves the item onto the undo stack
	 *
	 * @param[in] item      The item to save
	 * @param[in] container (Optional) The container wherein this item is located
	 * @note Call this _before_ you do your operation on the item
	 */
	template<typename T>
	size_t save(T& item, T* container = nullptr);

	/*!
	 * @brief Undo's the last changed item and save the changes into the Redo stack
	 *
	 * @returns A pair of const void* which reference the affected item, and its container (if it was provided)
	 */
	std::pair<const void*, const void*> undo();

	/*!
	 * @brief Redo's the last changed item and save the changes into the undo
	 *
	 * @returns A pair of const void* which reference the affected item, and its container (if it was provided)
	 */
	std::pair<const void*, const void*> redo();

	/*!
	 * @brief Returns the size of the undo stack
	 */
	size_t size();

	/*!
	 * @brief Returns the size of the redo stack
	 */
	size_t size_redo();

private:
	uint max_undos;	//!< Max number of Undo's available

	/*! 
	 * @brief The undo and redo stacks. These are containers of pointers, because we can't simply store the base class.
	 *   If we did, then the data of the Undo_item instances would be sliced out.
	 * @{
	 */
	SCP_deque<Undo_item_base*> undo_stack;
	SCP_deque<Undo_item_base*> redo_stack;
	//! @}
};