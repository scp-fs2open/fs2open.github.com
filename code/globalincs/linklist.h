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

// Removes an element from listit's in
#define list_remove( head, elem )		\
do {												\
	(elem)->prev->next = (elem)->next;	\
	(elem)->next->prev = (elem)->prev;	\
	(elem)->next = NULL;						\
	(elem)->prev = NULL;						\
} while(0)

//Moves elem to be after destelem
/*
#define list_move_append(destelem, elem)		\
do {												\
	(elem)->prev->next = (elem)->next;				\
	(elem)->next->prev = (elem)->prev;				\
	(elem)->prev = (destelem);						\
	(elem)->next = (destelem)->next;				\
	(destelem)->next->prev = (elem);				\
	(destelem)->next = (elem);						\
} while (0)*/
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
#define EMPTY(head)			((head)->next == (head))

template <class StoreType> class linked_list
{
protected:
	StoreType *m_next;
	StoreType *m_prev;

	int n_elem;
public:
	linked_list()
	{
		m_next=(StoreType*)this;
		m_prev=(StoreType*)this;
		n_elem=0;
	}

	~linked_list(){n_elem=0;}

	//Getting
	StoreType *get_first(){return m_next;}
	
	//Setting
	void append(StoreType *ptr)
	{
		ptr->m_prev=m_prev;
		ptr->m_next=(StoreType*)this;
		m_prev->m_next=ptr; m_prev=ptr;
		n_elem++;
	}

	void remove(StoreType *ptr)
	{	
		ptr->m_prev->m_next=ptr->m_next;
		ptr->m_next->m_prev=ptr->m_prev;
		ptr->m_next = 0;		//These should both be 0
		ptr->m_prev = 0;		//But stupid MSVC doesn't like a NULL here
		n_elem--;
	}	

	StoreType *get_next(){return m_next;}

	//Querying
	bool is_end(StoreType *ptr){return (ptr==this);}
	int get_num_elements() { return n_elem;}
};

#endif
