#pragma once

#include <utility>

namespace util
{

/**
 * @brief A trivially-copyable value whose MOVE resets the source to a sentinel.
 *
 * Intended for raw owning handles (heap pointers, bitmap/sound handles) inside
 * structs that are stored by value in containers.  An implicitly-generated move
 * of such a struct copies raw handles without resetting the source, so a
 * moved-from element still aliases the live resource and a later destructor
 * releases it twice.  Wrapping the handle in reset_on_move<> makes the
 * containing struct's implicit (or defaulted) move operations transfer
 * ownership correctly, without hand-writing memberwise moves.
 *
 * Copies remain shallow by design: the wrapper does not know how to duplicate
 * the underlying resource.  Types whose copies must deep-copy (or never copy)
 * should delete or implement their own copy operations.
 *
 * @tparam T        the handle type (e.g. some_type*, int)
 * @tparam NullVal  the sentinel a moved-from handle is reset to
 */
template <typename T, T NullVal = T{}>
struct reset_on_move
{
	T value = NullVal;

	reset_on_move() = default;
	reset_on_move(T v) : value(v) {}

	reset_on_move(const reset_on_move &) = default;
	reset_on_move &operator=(const reset_on_move &) = default;

	reset_on_move(reset_on_move &&other) noexcept : value(std::exchange(other.value, NullVal)) {}

	reset_on_move &operator=(reset_on_move &&other) noexcept
	{
		value = std::exchange(other.value, NullVal);
		return *this;
	}

	reset_on_move &operator=(T v)
	{
		value = v;
		return *this;
	}

	operator T() const { return value; }

	// only valid (and only instantiated) when T is a pointer type
	T operator->() const { return value; }
};

} // namespace util
