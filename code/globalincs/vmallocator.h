#ifndef _VMALLOCATOR_H_INCLUDED_
#define _VMALLOCATOR_H_INCLUDED_

/* SCP_vm_allocator - maintained by portej05 (i.e. please don't patch this one yourself!) */

#include <vector>
#include <list>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <queue>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <sstream>

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
using SCP_set = std::set<T, std::less<T>, std::allocator<T> >;

template< typename T >
using SCP_queue = std::queue< T, std::deque< T, std::allocator< T > > >;

template< typename T >
using SCP_deque = std::deque< T, std::allocator< T > >;

template< typename Key, typename T, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key> >
using SCP_unordered_map = std::unordered_map< Key, T, Hash, KeyEqual, std::allocator< std::pair<const Key, T> > >;

template< typename Key, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key> >
using SCP_unordered_set = std::unordered_set< Key, Hash, KeyEqual, std::allocator<  Key > >;

#endif // _VMALLOCATOR_H_INCLUDED_
