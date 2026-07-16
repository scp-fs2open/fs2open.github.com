/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#ifndef _LINKLIST_H
#define _LINKLIST_H

#include <cstddef>	// for std::ptrdiff_t
#include <iterator>	// for std::bidirectional_iterator_tag
#include <type_traits>	// for std::remove_pointer_t

// Initializes a list of zero elements
#define list_init( head )					\
do {												\
	(head)->next = (head);					\
	(head)->prev = (head);					\
} while (0)

// Inserts element onto the front of the list
#define list_insert( head, elem )		\
do {												\
	(elem)->next = (head)->next;			\
	(head)->next->prev = (elem);			\
	(head)->next = (elem);					\
	(elem)->prev = (head);					\
} while (0)

// Inserts new_elem before elem
#define list_insert_before(elem, new_elem)		\
do {															\
	(elem)->prev->next	= (new_elem);				\
	(new_elem)->prev		= (elem)->prev;			\
	(elem)->prev			= (new_elem);				\
	(new_elem)->next		= (elem);					\
} while (0)	

// Appends an element on to the tail of the list
#define list_append( head, elem )		\
do	{												\
	(elem)->prev = (head)->prev;			\
	(elem)->next = (head);					\
	(head)->prev->next = (elem);			\
	(head)->prev = (elem);					\
} while (0)

// Adds list b onto the end of list a
#define list_merge( a, b )					\
do {												\
	(a)->prev->next = (b)->next;			\
	(b)->next->prev = (a)->prev;			\
	(a)->prev = (b)->prev;					\
	(b)->prev->next = (a);					\
} while (0)

// Removes an element from the list
#define list_remove( head, elem )		\
do {												\
	(elem)->prev->next = (elem)->next;	\
	(elem)->next->prev = (elem)->prev;	\
	(elem)->next = nullptr;					\
	(elem)->prev = nullptr;					\
} while(false)

// Moves elem to be after head
#define list_move_append(head, elem)		\
do {												\
	(elem)->prev->next = (elem)->next;				\
	(elem)->next->prev = (elem)->prev;				\
	(elem)->prev = (head)->prev;					\
	(elem)->next = (head);							\
	(head)->prev->next = (elem);					\
	(head)->prev = (elem);							\
} while (0)

#define GET_FIRST(head)		((head)->next)
#define GET_LAST(head)		((head)->prev)
#define GET_NEXT(elem) 		((elem)->next)
#define GET_PREV(elem) 		((elem)->prev)
#define END_OF_LIST(head)	(head)
#define NOT_EMPTY(head)		((head)->next != (head))
#define EMPTY(head)			((head) == nullptr || (head)->next == (head))

// Adapter for iterating over the sentinel-headed circular lists managed by the macros
// above using a range-based for loop, e.g. for (auto so : list_range(&Ship_obj_list)),
// or a non-modifying <algorithm> function, e.g. std::find_if.
//
// Since these lists are intrusive, dereferencing an iterator using operator* returns a
// pointer to the element rather than a reference to it.  (Consequently the iterators do
// not strictly meet the standard's ForwardIterator requirements, but all mainstream
// standard libraries accept them in non-modifying algorithms.)
//
// NOTE: An element may not be removed from the list (nor freed) inside the loop body,
// because the iterator reads that element's next pointer when it advances.  Loops that
// need to do this must use the GET_FIRST/GET_NEXT macros and save the next pointer
// before the removal, as the object and ship code does.
template <class T>
class volition_linked_list
{
	T* head;

public:
	// U is either T or const T
	template <class U>
	class basic_iterator
	{
		U* ptr;

	public:
		// iterator traits, to allow use with <algorithm>
		// (note that value_type is a pointer, and operator* returns it by value)
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = U*;
		using difference_type = std::ptrdiff_t;
		using pointer = U*;
		using reference = U*;

		basic_iterator()
			: ptr(nullptr)
		{}

		explicit basic_iterator(U* x)
			: ptr(x)
		{}

		// Implicit conversion from iterator to const_iterator, as standard
		// containers provide.  Enabled only when U is const V for non-const V,
		// so it cannot be used to convert const_iterator to iterator, and does
		// not duplicate the copy constructor.
		template <class V, std::enable_if_t<std::is_same_v<U, const V> && !std::is_const_v<V>, int> = 0>
		basic_iterator(const basic_iterator<V>& it)
			: ptr(*it)
		{}

		basic_iterator& operator++()
		{
			ptr = GET_NEXT(ptr);
			return *this;
		}

		basic_iterator operator++(int)
		{
			basic_iterator tmp(*this);
			operator++();
			return tmp;
		}

		basic_iterator& operator--()
		{
			ptr = GET_PREV(ptr);
			return *this;
		}

		basic_iterator operator--(int)
		{
			basic_iterator tmp(*this);
			operator--();
			return tmp;
		}

		// Comparisons accept both iterator and const_iterator, as standard
		// containers allow (the underlying pointer comparison handles the
		// mixed-constness case).
		template <class V>
		bool operator==(const basic_iterator<V>& rhs) const
		{
			return ptr == *rhs;
		}

		template <class V>
		bool operator!=(const basic_iterator<V>& rhs) const
		{
			return ptr != *rhs;
		}

		U* operator*() const
		{
			return ptr;
		}

		U* operator->() const
		{
			return ptr;
		}
	};

	using iterator = basic_iterator<T>;
	using const_iterator = basic_iterator<const T>;

	iterator begin() const
	{
		return iterator(GET_FIRST(head));
	}

	iterator end() const
	{
		return iterator(END_OF_LIST(head));
	}

	const_iterator cbegin() const
	{
		return const_iterator(GET_FIRST(head));
	}

	const_iterator cend() const
	{
		return const_iterator(END_OF_LIST(head));
	}

	explicit volition_linked_list(T* ptr)
		: head(ptr)
	{}
};

template <class T>
auto list_range(T* head)
{
	// Deduce the node type from the next pointer rather than from head itself,
	// so that a list headed by a derived sentinel (e.g. ship_subsys_sentinel)
	// iterates as its node type.  decltype yields the member's declared type,
	// which never carries the head's constness, so restore that separately.
	using node_t = std::remove_pointer_t<decltype(head->next)>;
	using list_node_t = std::conditional_t<std::is_const_v<T>, const node_t, node_t>;
	return volition_linked_list<list_node_t>(head);
}

#endif
