#pragma once

#include <iosfwd>

namespace util {

/**
 * @brief A generic class for creating type safe handle types
 *
 * This type should be used when a function returns a handle to an internal resource instead of using an int.
 * Example:
 * @code{.cpp}
 * struct gamesnd_id_tag{};
 * typedef util::ID<gamesnd_id_tag, int, -1> gamesnd_id;
 * @endcode
 *
 * The implementation value should be very light-weight since this class is designed to be passed around by-value.
 *
 * @note This class was copied from http://www.ilikebigbits.com/blog/2014/5/6/type-safe-identifiers-in-c
 *
 * @tparam Tag A unique type tag which is used for distinguishing between two types which use the same Impl type
 * @tparam Impl The internal representation of the handle.
 * @tparam default_value The default value of the handle. If the handle has this value it is considered invalid.
 */
template<class Tag, class Impl, Impl default_value>
class ID
{
public:
	typedef Tag tag_type;
	typedef Impl impl_type;

	static ID invalid() { return ID(); }

	// Defaults to ID::invalid()
	ID() : m_val(default_value) { }

	// Explicit constructor:
	explicit ID(Impl val) : m_val(val) { }

	inline Impl value() const { return m_val; }

	friend bool operator==(ID a, ID b) { return a.m_val == b.m_val; }
	friend bool operator!=(ID a, ID b) { return a.m_val != b.m_val; }
	friend std::ostream& operator<< (std::ostream& stream, const ID& id) {
		stream << id.value();
		return stream;
	}

	inline bool isValid() const { return m_val != default_value; }

protected:
	Impl m_val;
};

}
