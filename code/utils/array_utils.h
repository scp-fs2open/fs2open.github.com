#pragma once

// General-purpose array/vector manipulation utilities.
// These are type-agnostic operations (insert, remove, move) that can be
// reused across different element types.

#include <algorithm>
#include <utility>
#include <vector>

#include "globalincs/pstypes.h"
#include "math/floating.h"

// ---------------------------------------------------------------------------
// Raw array overloads (array + count + max_size)
// ---------------------------------------------------------------------------

// Open a slot at `index` by shifting elements [index, count) right by one.
// Increments count.  Returns false if the array is already full.
template <typename T>
bool array_insert_slot(T *arr, int &count, int max_size, int index)
{
	Assertion(index >= 0 && index <= count, "array_insert_slot: index %d out of range [0, %d]", index, count);
	if (count >= max_size)
		return false;
	for (int i = count; i > index; i--)
		arr[i] = std::move(arr[i - 1]);
	count++;
	return true;
}

// Close a slot at `index` by shifting elements (index, count) left by one.
// Decrements count.
template <typename T>
void array_remove_slot(T *arr, int &count, int index)
{
	Assertion(index >= 0 && index < count, "array_remove_slot: index %d out of range [0, %d)", index, count);
	for (int i = index; i < count - 1; i++)
		arr[i] = std::move(arr[i + 1]);
	count--;
}

// Move element at `from` to `to`, shifting intermediate elements.
template <typename T>
void array_move_element(T *arr, int count, int from, int to)
{
	Assertion(from >= 0 && from < count, "array_move_element: from %d out of range [0, %d)", from, count);
	Assertion(to >= 0 && to < count, "array_move_element: to %d out of range [0, %d)", to, count);
	if (from == to)
		return;
	T temp = std::move(arr[from]);
	if (from < to) {
		for (int i = from; i < to; i++)
			arr[i] = std::move(arr[i + 1]);
	} else {
		for (int i = from; i > to; i--)
			arr[i] = std::move(arr[i - 1]);
	}
	arr[to] = std::move(temp);
}

// ---------------------------------------------------------------------------
// Swap-based raw array overloads (array + count + max_size)
//
// For element types that hold raw owning pointers (e.g. brief_stage's icons
// and lines buffers), the assignment-based helpers above would duplicate a
// pointer across two slots and orphan another buffer.  These variants
// rearrange slots exclusively by swapping adjacent elements, so every slot
// keeps a distinct buffer.  swap() is found via ADL, falling back to
// std::swap.
// ---------------------------------------------------------------------------

// Open a slot at `index` by rotating elements [index, count] right by one.
// The slot at `index` receives the former one-past-end element, so the
// caller must reset its fields.  Increments count.  Returns false if the
// array is already full.
template <typename T>
bool array_insert_slot_swap(T *arr, int &count, int max_size, int index)
{
	Assertion(index >= 0 && index <= count, "array_insert_slot_swap: index %d out of range [0, %d]", index, count);
	if (count >= max_size)
		return false;
	using std::swap;
	for (int i = count; i > index; i--)
		swap(arr[i], arr[i - 1]);
	count++;
	return true;
}

// Close the slot at `index` by rotating elements [index, count) left by one.
// The removed element is parked at the new one-past-end slot rather than
// destroyed.  Decrements count.
template <typename T>
void array_remove_slot_swap(T *arr, int &count, int index)
{
	Assertion(index >= 0 && index < count, "array_remove_slot_swap: index %d out of range [0, %d)", index, count);
	using std::swap;
	for (int i = index; i < count - 1; i++)
		swap(arr[i], arr[i + 1]);
	count--;
}

// Move element at `from` to `to` by rotating the elements in between.
template <typename T>
void array_move_element_swap(T *arr, int count, int from, int to)
{
	Assertion(from >= 0 && from < count, "array_move_element_swap: from %d out of range [0, %d)", from, count);
	Assertion(to >= 0 && to < count, "array_move_element_swap: to %d out of range [0, %d)", to, count);
	using std::swap;
	if (from < to) {
		for (int i = from; i < to; i++)
			swap(arr[i], arr[i + 1]);
	} else {
		for (int i = from; i > to; i--)
			swap(arr[i], arr[i - 1]);
	}
}

// ---------------------------------------------------------------------------
// Vector overloads
// ---------------------------------------------------------------------------

// Open a slot at `index` by shifting elements [index, count) right by one.
// Grows the vector if needed.  Increments count.
template <typename T>
void array_insert_slot(SCP_vector<T> &vec, int &count, int index)
{
	Assertion(index >= 0 && index <= count, "array_insert_slot: index %d out of range [0, %d]", index, count);
	if (count >= sz2i(vec.size()))
		vec.resize(count + 1);
	for (int i = count; i > index; i--)
		vec[i] = std::move(vec[i - 1]);
	count++;
}

// Close a slot at `index` by shifting elements (index, count) left by one.
// Decrements count.  Does not shrink the vector.
template <typename T>
void array_remove_slot(SCP_vector<T> &vec, int &count, int index)
{
	Assertion(index >= 0 && index < count, "array_remove_slot: index %d out of range [0, %d)", index, count);
	for (int i = index; i < count - 1; i++)
		vec[i] = std::move(vec[i + 1]);
	count--;
}

// Move element at `from` to `to`, shifting intermediate elements.
template <typename T>
void array_move_element(SCP_vector<T> &vec, int from, int to)
{
	Assertion(vec.in_bounds(from), "array_move_element: from %d out of range [0, " SIZE_T_ARG ")", from, vec.size());
	Assertion(vec.in_bounds(to), "array_move_element: to %d out of range [0, " SIZE_T_ARG ")", to, vec.size());
	if (from == to)
		return;
	T temp = std::move(vec[from]);
	if (from < to) {
		for (int i = from; i < to; i++)
			vec[i] = std::move(vec[i + 1]);
	} else {
		for (int i = from; i > to; i--)
			vec[i] = std::move(vec[i - 1]);
	}
	vec[to] = std::move(temp);
}
