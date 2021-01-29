/*
* z64555's Undo system, created for the FreeSpace Source Code project
*/

#include "globalincs/pstypes.h"
#include "globalincs/undosys.h"
#include "globalincs/vmallocator.h"

// Pure virtual deconstructors must be defined.
Undo_item_base::~Undo_item_base() = default;

Undo_system::Undo_system()
	: max_undos(10) {};

Undo_system::Undo_system(size_t _undos)
	: max_undos(_undos) {};

void Undo_system::clamp_stacks() {
	while (undo_stack.size() > max_undos) {
		// If we're past the stack limit, de-construct and remove until we're in the limit
		// If max_undos is held constant (as it should be), the loop isn't necassary. 
		//   But, it also isn't that much different from an if{} when assembled, so there's no harm in using it
		delete undo_stack.front();
		undo_stack.pop_front();
	}

	// This while() shouldn't ever be triggered, but this is a safety to prevent any weird edge-cases.
	while (redo_stack.size() > max_undos) {
		delete redo_stack.front();
		undo_stack.pop_front();
	}
}

void Undo_system::clear() {
	clear_redo();

	for (auto &element : undo_stack) {
		delete element;
	}
	undo_stack.clear();
}

void Undo_system::clear_redo() {
	for (auto &element : redo_stack) {
		delete element;
	}
	redo_stack.clear();
}

size_t Undo_system::save_stack(Undo_stack& stack) {
	if (stack.size() == 0) {
		return 0;
	}
	
	clear_redo();

	// Copy the stack onto the heap, and push it into our undo_stack as a single item
	Undo_item_base *new_item = new Undo_stack(stack);

	Assert(new_item != nullptr);
	undo_stack.push_back(new_item);

	// Input stack is told to untrack the items, so that there's only one deletion handler for them
	stack.untrack();

	clamp_stacks();

	return undo_stack.size();
}

std::pair<const void*, const void*> Undo_system::redo() {
	std::pair<const void*, const void*> retval;

	if (redo_stack.empty()) {
		// Nothing to redo
		return {nullptr, nullptr};
	}

	auto &item = redo_stack.back();
	retval = item->restore();
	undo_stack.push_back(item);
	redo_stack.pop_back();

	return retval;
};

std::pair<const void*, const void*> Undo_system::undo() {
	std::pair<const void*, const void*> retval;
	
	if (undo_stack.empty()) {
		// Nothing to undo
		return {nullptr, nullptr};
	}

	auto &item = undo_stack.back();
	retval = item->restore();
	redo_stack.push_back(item);
	undo_stack.pop_back();

	return retval;
};

bool Undo_system::empty() {
	return undo_stack.empty();
}

bool Undo_system::empty_redo() {
	return redo_stack.empty();
}
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
	clear();
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
		for (auto &element : stack) {
			retval = element->restore();
		}
	}

	reverse = !reverse;
	return retval;
}

size_t Undo_stack::size() {
	return stack.size();
}

void Undo_stack::untrack() {
	stack.clear();
}

void Undo_stack::clear() {
	for (auto &element : stack) {
		delete element;
	}
	stack.clear();
}
