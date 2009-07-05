#ifndef _VMALLOCATOR_H_INCLUDED_
#define _VMALLOCATOR_H_INCLUDED_

/* SCP_vm_allocator - maintained by portej05 (i.e. please don't patch this one yourself!) */

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

	/* portej05 does not like this particular function.
	 */
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

	template< class T >
	struct rebind
	{
		typedef SCP_vm_allocator< T > other;
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


#endif // _VMALLOCATOR_H_INCLUDED_