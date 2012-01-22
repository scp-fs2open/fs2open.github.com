#ifndef _VMALLOCATOR_H_INCLUDED_
#define _VMALLOCATOR_H_INCLUDED_

/* SCP_vm_allocator - maintained by portej05 (i.e. please don't patch this one yourself!) */

#include <vector>
#include <list>
#include <map>
#include <string>
#include <queue>

#if defined(_MSC_VER) && _MSC_VER >= 1400 || !defined(_MSC_VER)

#define DESTROY( type, p ) (p)->~type( )

template< typename T >
class SCP_vm_allocator
{
public:
	typedef size_t		size_type;
	typedef T*			pointer;
	typedef const T*	const_pointer;
	typedef T			value_type;
	typedef ptrdiff_t	difference_type;
	typedef value_type& reference;
	typedef const value_type& const_reference;

	/* portej05 does not like this particular function. */
	void construct( pointer p, const T& value )
	{
		::new (p) T(value);
	}

	void destroy( pointer p )
	{
		DESTROY( T, p );
	}

	pointer allocate( size_type n )
	{
		if ( n == 0 )
			return NULL;
		return (pointer)vm_malloc( n * sizeof( T ) );
	}

	template<class Other>
	pointer allocate( size_type n, const Other* hint = NULL )
	{
		(hint);
		return allocate( n );
	}

	void deallocate( pointer p, size_type )
	{
		if ( p )
			vm_free( p );
	}

	template< class U >
	struct rebind
	{
		typedef SCP_vm_allocator< U > other;
	};

	size_type max_size( ) const
	{	// Estimate process used by MS STL
		size_type _Count = ( size_type )( -1 ) / sizeof ( T );
		return ( 0 < _Count ? _Count : 1 );
	}

	template< class T2 >
	SCP_vm_allocator< T >& operator=( const SCP_vm_allocator<T2>& )
	{
		return (*this);
	}

	SCP_vm_allocator( )
	{
	}

	SCP_vm_allocator( const SCP_vm_allocator< T >& )
	{
	}

	template< class T2 >
	SCP_vm_allocator( const SCP_vm_allocator< T2 >& )
	{
	}
};

template< typename T >
class SCP_vector : public std::vector< T, SCP_vm_allocator< T > > { };

template< typename T >
class SCP_list : public std::list< T, SCP_vm_allocator< T > > { };

typedef std::basic_string<char, std::char_traits<char>, SCP_vm_allocator<char> > SCP_string;

typedef std::basic_stringstream<char, std::char_traits<char>, SCP_vm_allocator<char> > SCP_stringstream;

template< typename T, typename U >
class SCP_map : public std::map<T, U, std::less<T>, SCP_vm_allocator<std::pair<const T, U> > > { };

template< typename T, typename U >
class SCP_multimap : public std::multimap<T, U, std::less<T>, SCP_vm_allocator<std::pair<const T, U> > > { };

template< typename T >
class SCP_queue : public std::queue< T, std::deque< T, SCP_vm_allocator< T > > > { };

template <class T1, class T2>
bool operator==(const SCP_vm_allocator<T1>&, const SCP_vm_allocator<T2>&) throw()
{
	return true;
}

template <class T1, class T2>
bool operator!=(const SCP_vm_allocator<T1>&, const SCP_vm_allocator<T2>&) throw()
{
	return false;
}


#else

#define SCP_string std::string
#define SCP_stringstream std::stringstream
#define SCP_map std::map
#define SCP_multimap std::multimap
#define SCP_queue std::queue
#define SCP_vector std::vector
#define SCP_list std::list

#endif

#endif // _VMALLOCATOR_H_INCLUDED_