#ifndef _VMALLOCATOR_H_INCLUDED_
#define _VMALLOCATOR_H_INCLUDED_

/* SCP_vm_allocator - maintained by portej05 (i.e. please don't patch this one yourself!) */

#include <deque>
#include <list>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

template< typename T >
using SCP_vector = std::vector< T, std::allocator< T > >;

template< typename T >
using SCP_list = std::list< T, std::allocator< T > >;

typedef std::basic_string<char, std::char_traits<char>, std::allocator<char> > SCP_string;

typedef std::basic_stringstream<char, std::char_traits<char>, std::allocator<char> > SCP_stringstream;

template< typename T, typename U >
using SCP_map = std::map<T, U, std::less<T>, std::allocator<std::pair<const T, U> > >;

template< typename T, typename U >
using SCP_multimap = std::multimap<T, U, std::less<T>, std::allocator<std::pair<const T, U> > >;

template< typename T >
using SCP_queue = std::queue< T, std::deque< T, std::allocator< T > > >;

template< typename T >
using SCP_deque = std::deque< T, std::allocator< T > >;

#if __cplusplus < 201402L
template <class T, bool>
struct enum_hasher_util {
	inline size_t operator()(const T& elem) { return std::hash<T>()(elem); }
};

template <class T>
struct enum_hasher_util<T, true> {
	inline size_t operator()(const T& elem)
	{
		typedef typename std::underlying_type<T>::type enumType;
		return std::hash<enumType>()(static_cast<enumType>(elem));
	}
};

/**
 * @brief An enum class compatible hashing class
 *
 * This is the same as std::hash except that is can handle enum classes properly using their underlying type. This is
 * only required for C++ < 14 since after that the standard mandates that enum classes should be hashable.
 *
 * @tparam T The type that should be hashed
 */
template <class T>
struct SCP_hash {
	inline size_t operator()(const T& elem) const { return enum_hasher_util<T, std::is_enum<T>::value>()(elem); }
};

#else
// No need for the special hash class so just use the standard hash
template <typename T>
using SCP_hash = std::hash<T>;
#endif

template <typename Key, typename T, typename Hash = SCP_hash<Key>, typename KeyEqual = std::equal_to<Key>>
using SCP_unordered_map = std::unordered_map<Key, T, Hash, KeyEqual, std::allocator<std::pair<const Key, T>>>;

#endif // _VMALLOCATOR_H_INCLUDED_
