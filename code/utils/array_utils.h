#pragma once

// General-purpose array/vector manipulation utilities.

#include <algorithm>

#include "globalincs/vmallocator.h"

// Adjusts an index stored elsewhere after the element at `removed_index` has been erased from its array.
// Returns false if the index referred to the removed element, in which case it is left unchanged.
template <typename T>
bool array_adjust_index_for_removal(T &index, T removed_index)
{
	if (index == removed_index)
		return false;
	if (index > removed_index)
		--index;
	return true;
}

// Adjusts an index stored elsewhere after an element has been inserted into its array at `inserted_index`.
template <typename T>
void array_adjust_index_for_insertion(T &index, T inserted_index)
{
	if (index >= inserted_index)
		++index;
}

// Drops the entries of a list of indexes that referred to the erased element at `removed_index`, and adjusts the rest.
inline void array_remove_index_from_list(SCP_vector<int> &indexes, int removed_index)
{
	indexes.erase(std::remove(indexes.begin(), indexes.end(), removed_index), indexes.end());
	for (auto &index : indexes)
		array_adjust_index_for_removal(index, removed_index);
}
