#ifndef _VMALLOCATOR_H_INCLUDED_
#define _VMALLOCATOR_H_INCLUDED_

/* SCP_vm_allocator - maintained by portej05 (i.e. please don't patch this one yourself!) */

#include <vector>
#include <list>
#include <map>
#include <string>
#include <queue>
#include <deque>
#include <unordered_map>

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

template< typename Key, typename T, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key> >
class SCP_unordered_map : public std::unordered_map< Key, T, Hash, KeyEqual, std::allocator< std::pair<const Key, T> > > { };


#endif // _VMALLOCATOR_H_INCLUDED_
