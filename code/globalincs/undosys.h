/*
 * z64555's Undo system, created for the FreeSpace Source Code project
 */

#include "globalincs/pstypes.h"
#include "globalincs/vmallocator.h"

#include <utility>

/**********************************************************
Usage:
First, you need an instance of the Undo_system within your module:
	{
		Undo_styem Undo_controls;
	}

From now on, you can save and restore items. (Just be sure to save them before you change them!)
	{
		Undo_controls.save(Control_config[z]);             // 1. Save the item!!
		Control_config[z].bind(cid(CID_KEYBOARD, KEY_UP)); // 2. Make changes to the item!!
		Undo_controls.undo();                              // 3. Undo's the changes you did in step 2!
		Undo_controls.redo();                              // 4. Redo's the changes you did in step 2!
	}
The undo system can save any type of data, you don't have to have an instance of a system per data type.
!!NOTE!! C style arrays and strings should be wrapped in a std::array when saving. Simply passing the array/string's head won't save the entire array.
{
	Undo_controls.save(std::array<int, JOY_AXIS_ACTIONS>(Axis_map_to));
}

In several cases, the client code would like to know exactly what was changed. The undo system provides a mechanism for doing this:
	{
		Undo_controls.save(Control_config[z], &Control_config[0]);   // Save the item, and a reference to its container. Here, we use the location of the first item in Control_config as our reference
		Undo_controls.save(Axis_map_to[z], Axis_map_to);             // Saves an axis mapping, here, we use the array's head pointer as our reference (since a int[] is the same as a int*)

		std::pair<const void*, const void*> ref = Undo_controls.undo();    // ::undo() returns a std::pair<const void*, const void*>, the first member referencing the item that changed, and the second referencing the item's container (if we provided it)

		// Once we've stored the return value from ::undo, we can found out what the item is by comparing the second member with containers that we've been saving
		if (ref.second == &Control_config[0]) {
			cout << "I'm a button!";
			Tab = static_cast<Config_item>(ref.first).tab;  // Sets the selected tab within the Controls Config menu. We do a static cast here so we can access the Config_item::tab member
		} else if (ref.second == Axis_map_to) {
			cout << "I'm an axis!";
			Tab = SHIP_TAB;                                 // Sets the selected tab within the Controls config menu. Can't do a cast on this one, since it's a simple C array. But we already know where the axes are kept (on the ship tab)
		} else {
			cout << "I don't know what I am!!! D:";         // Usually an error condition. Depending on the client code this can be fatal or just a minor setback
		}
	}

Lastly, you can make stacks of undo operations, so that a single op in the system can undo/redo multiple item changes.
	{
		Undo_stack stack;  // We'll want to save multiple changes as a single operation. So we have to make a stack.

		for (int i = 0; i < JOY_AXIS_ACTIONS; ++i) {
			stack.save(Axis_map_to[i], Axis_map_to);		// This example saves a C style array. Does the same thing without a wrapper, but wasteful
		
		Undo_controls.save_stack(stack) // Save the stack as a single item!
	}

	std::pair<const void*, const void*> ref = Undo_controls.undo();
	// Undo rolls through the items as they were saved, should your data operate on the same location, this will apply the changes in the correct sequence
	ref.second == Axis_map_to;
	ref.first == &Axis_map_to[0];     // ::first references the _last_ item that the undo stack changed

	//Also, performing an undo reverses the stack. So when you do a system undo, the changes are applied in reverse
	ref = Undo_controls.redo();
	ref.second == Axis_map_to;
	ref.first == &Axis_map_to[JOY_AXIS_ACTIONS - 1];      // ::first references the _last_ item that the undo stack changed

***********************************************************/

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
 *   std::vector or a std::deque of this class. Undo and Redo stacks would be separate. Before restoring data from
 *   either, you'd save a copy of the item in the opposite stack.
 *
 *   For example, if you're about to restore from the undo stack, you'd first save a copy of the current data to the
 *   redo stack, then do the undo. The Undo_item in the undo stack can then be popped off the stack.
 *
 * @note If you want to save changes to an entire C-style array (such as 'int array[]'), try wrapping it in a std::array first.
 *
 * @sa Undo_item_base
 */
template<typename T> class Undo_item : public Undo_item_base
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

	~Undo_item() override {
		if (data != nullptr) {
			delete data;
		}
	};

	/**
	 * @brief Restores the saved data
	 *
	 * @returns A pair of const void* referencing the affected item and its container (if it was provided)
	 * @details swaps the data with the destination's, effectively making an undo item into a redo item, and vice versa
	 */
	std::pair<const void*, const void*> restore() override {
		Assert((dest != nullptr) && (data != nullptr));

		std::swap(*static_cast<T*>(dest),*static_cast<T*>(data));

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

		dest = &_data;
		data = new T(_data);
		container = _cont;
	};

private:
	T* dest;        //!< Destination of the data
	T* data;        //!< Reference to a copy of data on the heap
	T* container;   //!< Optional reference to this item's container
};


/*!
 * @brief Class which handles multiple undo operations as a single op within the Undo_system
 * @note Caution: Upon deconstruction the tracked Undo_items are deleted.
 */
class Undo_stack : public Undo_item_base
{
public:
	Undo_stack();

	Undo_stack(size_t size);

	~Undo_stack() override;

	/*!
	 * @brief Restores all items within the undo stack
	 *
	 * @details Maintains an internal flag which will reverse direction on the next call, thereby re-doing
	 * 
	 * @returns A pair of references to the last item restored
	 */
	std::pair<const void*, const void*> restore() override;

	/*!
	 * @brief Saves the item onto the undo stack
	 *
	 * @param[in] item      The item to save
	 * @param[in] container (Optional) The container wherein this item is located
	 * @note Call this _before_ you do your operation on the item
	 */
	template<typename T>
	size_t save(T& item, T* container = nullptr) {
		// Create a new instance of Undo_tem, with the correct type reference
		Undo_item_base *new_item = new Undo_item<T>(item, container);

		stack.push_back(new_item);

		return stack.size();
	};

	/*!
	 * @brief Calls ::reserve() on the internal vector
	 */
	void reserve(size_t size);

	/*!
	 * @brief Returns the size of the stack
	 */
	 size_t size();

	 /*!
	  * @brief Deletes all tracked items in the internal vector
	  */
	 void clear();

protected:
	friend class Undo_system;

	/*!
	 * @brief Calls ::clear() on the internal vector
	 * @note Caution: This does not delete the tracked Undo_items
	 */
	void untrack();

private:
	bool reverse;   // Direction to walk the stack. forward = false, reverse = True

	std::vector<Undo_item_base*> stack;
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

	Undo_system(size_t _undos);

	/*!
	 * @brief Deletes all undo and redo data
	 */
	void clear();
	
	/*!
	 * @brief Saves the item onto the undo stack
	 *
	 * @param[in] item      The item to save
	 * @param[in] container (Optional) The container wherein this item is located
	 * @note Call this _before_ you do your operation on the item
	 */
	template<typename T>
	size_t save(T& item, T* container = nullptr) {
		// De-construct all intances of Undo_item on the redo stack, then clear the stack
		clear_redo();

		// Create a new instance of Undo_tem, with the correct type reference
		Undo_item_base *new_item = new Undo_item<T>(item, container);

		undo_stack.push_back(new_item);

		clamp_stacks();

		return undo_stack.size();
	};

	/*!
	 * @brief Saves a stack of undo-items as a single undo-item within the system
	 *
	 * @param[in,out] stack  The undo stack to save to the undo system.  Data in the stack is "unspecified" after this operation
	 *
	 * @details  The undo system effectively moves the stack into its internal containers, claiming ownership of the
	 *  Undo_items and deleting them upon going out of scope.  The input stack is told to untrack the Undo_items in
	 *  the process, so that there is only ever one reference to the Undo_item like a std::unique_ptr
	 */
	size_t save_stack(Undo_stack& stack);

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

	/*!
	 * @brief True if undo stack size = 0
	 */
	bool empty();

	/*!
	 * @brief True if redo stack size = 0
	 */
	 bool empty_redo();

protected:
	/*!
	 * @brief Deletes all redo data
	 */
	void clear_redo();

	/*!
	 * @brief Clamp the stack sizes to be <= max_undos, deleting data as needed
	 */
	void clamp_stacks();

private:
	size_t max_undos;	//!< Max number of Undo's available

	/*! 
	 * @brief The undo and redo stacks. These are containers of pointers, because we can't simply store the base class.
	 *   If we did, then the data of the Undo_item instances would be sliced out.
	 * @{
	 */
	SCP_deque<Undo_item_base*> undo_stack;
	SCP_deque<Undo_item_base*> redo_stack;
	//! @}
};