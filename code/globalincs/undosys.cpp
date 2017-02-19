/*
* z64555's Undo system, created for the FreeSpace Source Code project
*
* Released under Creative Commons Attribution-ShareAlike 4.0 International
* https://creativecommons.org/licenses/by-sa/4.0/
*/

#include "globalincs/pstypes.h"
#include "globalincs/undosys.h"
#include "globalincs/vmallocator.h"

Undo_system::Undo_system()
	: max_undos(10) {};

Undo_system::Undo_system(uint _undos)
	: max_undos(_undos) {};

template<typename T>
size_t Undo_system::save(T& item, T* container) {
	// De-construct all intances of Undo_item on the redo stack, then clear the stack
	for (auto it = redo_stack.begin(); it != redo_stack.end(); ++it) {
		delete *it;
	}
	redo_stack.clear();

	// Create a new instance of Undo_tem, with the correct type reference
	Undo_item_base *new_item = new Undo_item<T>(item, container);

	// If operator new fails, then we've got bigger problems!
	// If needed, You can modify this to be tolerate of the issue by throwing an error before proceding
	Assert(new_item != nullptr);
	undo_stack.push_back(new_item);

	while (undo_stack.size() > max_undos) {
		// If we're past the stack limit, de-construct and remove until we're in the limit
		// If max_undos is held constant (as it should be), the loop isn't necassary. 
		//   But, it also isn't that much different from an if{} when assembled, so there's no harm in using it
		delete undo_stack.front();
		undo_stack.pop_front();
	}

	return undo_stack.size();
};

size_t Undo_system::save_stack(Undo_stack& stack) {
	for (auto it = redo_stack.begin(); it != redo_stack.end(); ++it) {
		delete *it;
	}
	redo_stack.clear();

	Undo_item_base *new_item = new Undo_stack(stack);

	Assert(new_item != nullptr);
	undo_stack.push_back(new_item);

	while (undo_stack.size() > max_undos) {
		delete undo_stack.front();
		undo_stack.pop_front();
	}

	return undo_stack.size();
}

std::pair<const void*, const void*> Undo_system::redo() {
	std::pair<const void*, const void*> retval;

	if (redo_stack.empty()) {
		// Nothing to redo
		return std::pair<const void*, const void*>(nullptr, nullptr);
	}

	auto &item = redo_stack.back();
	retval = item->restore();
	undo_stack.push_back(item);
	redo_stack.pop_back();

	while (undo_stack.size() > max_undos) {
		delete undo_stack.front();
		undo_stack.pop_front();
	}

	return retval;
};

std::pair<const void*, const void*> Undo_system::undo() {
	std::pair<const void*, const void*> retval;
	
	if (undo_stack.empty()) {
		// Nothing to undo
		return std::pair<const void*, const void*>(nullptr, nullptr);
	}

	auto &item = undo_stack.back();
	retval = item->restore();
	redo_stack.push_back(item);
	undo_stack.pop_back();

	while (redo_stack.size() > max_undos) {
		delete redo_stack.front();
		redo_stack.pop_front();
	}

	return retval;
};

size_t Undo_system::size() {
	return undo_stack.size();
}

size_t Undo_system::size_redo() {
	return redo_stack.size();
}


Undo_stack::Undo_stack()
	: reverse(false) {
}

Undo_stack::Undo_stack(size_t size)
	: reverse(false) {
	stack.reserve(size);
}

Undo_stack::~Undo_stack() {
	for (auto it = stack.begin(); it != stack.end(); ++it) {
		delete *it;
	}
	stack.clear();
}

void Undo_stack::reserve(size_t size) {
	stack.reserve(size);
}

std::pair<const void*, const void*> Undo_stack::restore() {
	std::pair<const void*, const void*> retval;

	if (reverse) {
		for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
			retval = (*it)->restore();
		}
	} else {
		for (auto it = stack.begin(); it != stack.end(); ++it) {
			retval = (*it)->restore();
		}
	}

	reverse = !reverse;
	return retval;
}

template<typename T>
size_t Undo_stack::save(T& item, T* container) {
	// Create a new instance of Undo_tem, with the correct type reference
	Undo_item_base *new_item = new Undo_item<T>(item, container);

	// If operator new fails, then we've got bigger problems!
	// If needed, You can modify this to be tolerate of the issue by throwing an error before proceding
	Assert(new_item != nullptr);
	stack.push_back(new_item);

	return stack.size();
};

