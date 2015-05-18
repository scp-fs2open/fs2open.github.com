#ifndef _VMALLOCATOR_H_INCLUDED_
#define _VMALLOCATOR_H_INCLUDED_

/* SCP_vm_allocator - maintained by portej05 (i.e. please don't patch this one yourself!) */

#include <vector>
#include <list>
#include <map>
#include <string>
#include <queue>
#include <deque>

#if defined __GNUC__
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if GCC_VERSION >= 40300
#include <tr1/unordered_map>
#define SCP_hash_map std::tr1::unordered_map
#elif GCC_VERSION < 40300 || __clang__
#include <ext/hash_map>
#define SCP_hash_map __gnu_cxx::hash_map
#endif // GCC_VERSION || __clang__
#endif // __GNUC__

#if ! defined __GNUC__
	#if defined(_MSC_VER)
		#if _MSC_VER < 1900
			#include <hash_map>
			#if _MSC_VER < 1400
			#define SCP_hash_map std::hash_map
			#else
			#define SCP_hash_map stdext::hash_map
			#endif
		#else
			#include <unordered_map>
			#define SCP_hash_map std::unordered_map
		#endif
	#endif
#endif // ! defined __GNUC__

template< typename T >
class SCP_vector : public std::vector< T, std::allocator< T > > { };

template< typename T >
class SCP_list : public std::list< T, std::allocator< T > > { };

typedef std::basic_string<char, std::char_traits<char>, std::allocator<char> > SCP_string;

typedef std::basic_stringstream<char, std::char_traits<char>, std::allocator<char> > SCP_stringstream;

template< typename T, typename U >
class SCP_map : public std::map<T, U, std::less<T>, std::allocator<std::pair<const T, U> > > { };

template< typename T, typename U >
class SCP_multimap : public std::multimap<T, U, std::less<T>, std::allocator<std::pair<const T, U> > > { };

template< typename T >
class SCP_queue : public std::queue< T, std::deque< T, std::allocator< T > > > { };

template< typename T >
class SCP_deque : public std::deque< T, std::allocator< T > > { };


#endif // _VMALLOCATOR_H_INCLUDED_
