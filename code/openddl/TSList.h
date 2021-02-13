//
// This file is part of the Terathon Common Library, by Eric Lengyel.
// Copyright 1999-2021, Terathon Software LLC
//
// This software is licensed under the GNU General Public License version 3.
// Separate proprietary licenses are available from Terathon Software.
//
// THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER
// EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. 
//


#ifndef TSList_h
#define TSList_h


//# \component	Utility Library
//# \prefix		Utilities/


#include "TSPlatform.h"


#define TERATHON_LIST 1


namespace Terathon
{
	class ListBase;

	template <class>
	class List;


	class ListElementBase
	{
		friend class ListBase;

		private:

			ListElementBase		*prevListElement;
			ListElementBase		*nextListElement;
			ListBase			*owningList;

			ListElementBase(const ListElementBase&) = delete;
			ListElementBase& operator =(const ListElementBase&) = delete;

		protected:

			ListElementBase()
			{
				prevListElement = nullptr;
				nextListElement = nullptr;
				owningList = nullptr;
			}

			TERATHON_API virtual ~ListElementBase();

			ListElementBase *GetPreviousListElement(void) const
			{
				return (prevListElement);
			}

			ListElementBase *GetNextListElement(void) const
			{
				return (nextListElement);
			}

			ListBase *GetOwningList(void) const
			{
				return (owningList);
			}

		public:

			TERATHON_API int32 GetListIndex(void) const;

			TERATHON_API virtual void Detach(void);
	};


	class ListBase
	{
		friend class ListElementBase;

		private:

			ListElementBase		*firstListElement;
			ListElementBase		*lastListElement;

			ListBase(const ListBase&) = delete;
			ListBase& operator =(const ListBase&) = delete;

		protected:

			ListBase()
			{
				firstListElement = nullptr;
				lastListElement = nullptr;
			}

			TERATHON_API ~ListBase();

			TERATHON_API ListElementBase *operator [](machine index) const;

			ListElementBase *GetFirstListElement(void) const
			{
				return (firstListElement);
			}

			ListElementBase *GetLastListElement(void) const
			{
				return (lastListElement);
			}

			bool Member(const ListElementBase *element) const
			{
				return (element->owningList == this);
			}

			TERATHON_API void PrependListElement(ListElementBase *element);
			TERATHON_API void AppendListElement(ListElementBase *element);

			TERATHON_API void InsertListElementBefore(ListElementBase *element, ListElementBase *before);
			TERATHON_API void InsertListElementAfter(ListElementBase *element, ListElementBase *after);

			TERATHON_API void RemoveListElement(ListElementBase *element);

		public:

			bool Empty(void) const
			{
				return (!firstListElement);
			}

			TERATHON_API int32 GetListElementCount(void) const;

			TERATHON_API void RemoveAllListElements(void);
			TERATHON_API void PurgeList(void);
	};


	//# \class	ListElement		The base class for objects that can be stored in a list.
	//
	//# Objects inherit from the $ListElement$ class so that they can be stored in a list.
	//
	//# \def	template <class type> class ListElement : public ListElementBase
	//
	//# \tparam		type	The type of the class that can be stored in a list. This parameter should be the
	//#						type of the class that inherits directly from the $ListElement$ class.
	//
	//# \ctor	ListElement();
	//
	//# \desc
	//# The $ListElement$ class should be declared as a base class for objects that need to be stored in a list.
	//# The $type$ template parameter should match the class type of such objects, and these objects can be
	//# stored in a $@List@$ container declared with the same $type$ template parameter.
	//
	//# \privbase	ListElementBase		Used internally to encapsulate common functionality that is independent
	//#									of the template parameter.
	//
	//# \also	$@List@$


	//# \function	ListElement::GetPreviousListElement		Returns the previous element in a list.
	//
	//# \proto	type *GetPreviousListElement(void) const;
	//
	//# \desc
	//# The $GetPreviousListElement$ function returns a pointer to the element immediately preceding an object
	//# in its owning list. If the object is the first element in a list, or the object does not belong to a
	//# list, then the return value is $nullptr$.
	//
	//# \also	$@ListElement::GetNextListElement@$


	//# \function	ListElement::GetNextListElement		Returns the next element in a list.
	//
	//# \proto	type *GetNextListElement(void) const;
	//
	//# \desc
	//# The $GetNextListElement$ function returns a pointer to the element immediately succeeding an object
	//# in its owning list. If the object is the last element in a list, or the object does not belong to a
	//# list, then the return value is $nullptr$.
	//
	//# \also	$@ListElement::GetPreviousListElement@$


	//# \function	ListElement::GetOwningList		Returns the list to which an object belongs.
	//
	//# \proto	List<type> *GetOwningList(void) const;
	//
	//# \desc
	//# The $GetOwningList$ function returns a pointer to the $@List@$ container to which an object belongs.
	//# If the object is not a member of a list, then the return value is $nullptr$.
	//
	//# \also	$@List::Member@$


	//# \function	ListElement::Detach		Removes an object from any list to which it belongs.
	//
	//# \proto	virtual void Detach(void);
	//
	//# \desc
	//# The $Detach$ function removes an object from its owning list. If the object is not a member of
	//# a list, then the $Detach$ function has no effect.
	//
	//# \also	$@List::RemoveListElement@$


	template <class type>
	class ListElement : public ListElementBase
	{
		protected:

			ListElement() {}

		public:

			type *GetPreviousListElement(void) const
			{
				return (static_cast<type *>(static_cast<ListElement<type> *>(ListElementBase::GetPreviousListElement())));
			}

			type *GetNextListElement(void) const
			{
				return (static_cast<type *>(static_cast<ListElement<type> *>(ListElementBase::GetNextListElement())));
			}

			List<type> *GetOwningList(void) const
			{
				return (static_cast<List<type> *>(ListElementBase::GetOwningList()));
			}
	};


	template <class type>
	class ListIterator
	{
		private:

			type		*iteratorElement;

		public:

			ListIterator(type *element) : iteratorElement(element) {}

			type *operator *(void) const
			{
				return (iteratorElement);
			}

			ListIterator& operator ++(void)
			{
				iteratorElement = iteratorElement->ListElement<type>::GetNextListElement();
				return (*this);
			}

			bool operator ==(const ListIterator& iterator) const
			{
				return (iteratorElement == iterator.iteratorElement);
			}

			bool operator !=(const ListIterator& iterator) const
			{
				return (iteratorElement != iterator.iteratorElement);
			}
	};


	//# \class	List	A container class that holds a list of objects.
	//
	//# The $List$ class encapsulates a doubly-linked list.
	//
	//# \def	template <class type> class List : public ListBase
	//
	//# \tparam		type	The type of the class that can be stored in the list. The class specified
	//#						by this parameter should inherit directly from the $@ListElement@$ class
	//#						using the same template parameter.
	//
	//# \ctor	List();
	//
	//# \desc
	//# The $List$ class template is a container used to store a homogeneous doubly-linked list of objects.
	//# The class type of objects that are to be stored in the list must be a subclass of the $@ListElement@$
	//# class template using the same template parameter as the $List$ container. A particular object can be a
	//# member of only one list at a time.
	//#
	//# Upon construction, a $List$ object is empty. When a $List$ object is destroyed, all of the members
	//# of the list are also destroyed. To avoid deleting the members of a list when a $List$ object is
	//# destroyed, first call the $@List::RemoveAllListElements@$ function to remove all of the list's members.
	//#
	//# It is possible to iterate over the elements of a list using a range-based for loop.
	//# This is illustrated by the following code, where $list$ is a variable of type $List<type>$.
	//
	//# \source
	//# for (type *element : list)\n
	//# {\n
	//# \t...\n
	//# }
	//
	//# \operator	type *operator [](machine index) const;
	//#				Returns the element of a list whose zero-based index is $index$. If $index$ is
	//#				greater than or equal to the number of elements in the list, then the return
	//#				value is $nullptr$.
	//
	//# \privbase	ListBase	Used internally to encapsulate common functionality that is independent
	//#							of the template parameter.
	//
	//# \also	$@ListElement@$
	//# \also	$@Array@$


	//# \function	List::GetFirstListElement		Returns the first element in a list.
	//
	//# \proto	type *GetFirstListElement(void) const;
	//
	//# \desc
	//# The $GetFirstListElement$ function returns a pointer to the first element in a list. If the list is empty,
	//# then this function returns $nullptr$.
	//
	//# \also	$@List::GetLastListElement@$


	//# \function	List::GetLastListElement		Returns the last element in a list.
	//
	//# \proto	type *GetLastListElement(void) const;
	//
	//# \desc
	//# The $GetLastListElement$ function returns a pointer to the last element in a list. If the list is empty,
	//# then this function returns $nullptr$.
	//
	//# \also	$@List::GetFirstListElement@$


	//# \function	List::Member		Returns a boolean value indicating whether a particular object is
	//#									a member of a list.
	//
	//# \proto	bool Member(const ListElement<type> *element) const;
	//
	//# \param	element		A pointer to the object to test for membership.
	//
	//# \desc
	//# The $Member$ function returns $true$ if the object specified by the $element$ parameter is
	//# a member of the list, and $false$ otherwise.
	//
	//# \also	$@ListElement::GetOwningList@$


	//# \function	List::Empty		Returns a boolean value indicating whether a list is empty.
	//
	//# \proto	bool Empty(void) const;
	//
	//# \desc
	//# The $Empty$ function returns $true$ if the list contains no elements, and $false$ otherwise.
	//
	//# \also	$@List::GetFirstListElement@$
	//# \also	$@List::GetLastListElement@$


	//# \function	List::GetListElementCount		Returns the number of elements in a list.
	//
	//# \proto	int32 GetListElementCount(void) const;
	//
	//# \desc
	//# The $GetListElementCount$ function iterates through the members of a list and returns the count.
	//
	//# \also	$@List::Empty@$


	//# \function	List::PrependListElement	Adds an object to the beginning of a list.
	//
	//# \proto	void PrependListElement(ListElement<type> *element);
	//
	//# \param	element		A pointer to the object to add to the list.
	//
	//# \desc
	//# The $PrependListElement$ function adds the object specified by the $element$ parameter to the beginning of
	//# a list. If the object is already a member of the list, then it is moved to the beginning.
	//#
	//# If the object being added is already a member of a different list of the same type, then it is first
	//# removed from that list before being added to the new list.
	//
	//# \also	$@List::AppendListElement@$
	//# \also	$@List::InsertListElementBefore@$
	//# \also	$@List::InsertListElementAfter@$


	//# \function	List::AppendListElement		Adds an object to the end of a list.
	//
	//# \proto	void AppendListElement(ListElement<type> *element);
	//
	//# \param	element		A pointer to the object to add to the list.
	//
	//# \desc
	//# The $ListElement$ function adds the object specified by the $element$ parameter to the end of
	//# a list. If the object is already a member of the list, then it is moved to the end.
	//#
	//# If the object being added is already a member of a different list of the same type, then it is first
	//# removed from that list before being added to the new list.
	//
	//# \also	$@List::PrependListElement@$
	//# \also	$@List::InsertListElementBefore@$
	//# \also	$@List::InsertListElementAfter@$


	//# \function	List::InsertListElementBefore		Inserts an object before an existing element of a list.
	//
	//# \proto	void InsertListElementBefore(ListElement<type> *element, ListElement<type> *before);
	//
	//# \param	element		A pointer to the object to add to the list.
	//# \param	before		A pointer to the object before which the new object is inserted.
	//#						This parameter must point to an object that is already a member of the list.
	//
	//# \desc
	//# The $InsertListElementBefore$ function adds the object specified by the $element$ parameter to a list at the
	//# position before the object specified by the $before$ parameter. If the object is already a member
	//# of the list, then it is moved to the new position. If the $before$ parameter is $nullptr$, then the
	//# node is added to the end of the list. Otherwise, the $before$ parameter must specify an object that
	//# is already a member of the list for which this function is called.
	//#
	//# If the object being added is already a member of a different list of the same type, then it is first
	//# removed from that list before being added to the new list.
	//
	//# \also	$@List::InsertListElementAfter@$
	//# \also	$@List::PrependListElement@$
	//# \also	$@List::AppendListElement@$


	//# \function	List::InsertListElementAfter		Inserts an object after an existing element of a list.
	//
	//# \proto	void InsertListElementAfter(ListElement<type> *element, ListElement<type> *after);
	//
	//# \param	element		A pointer to the object to add to the list.
	//# \param	after		A pointer to the object after which the new object is inserted.
	//#						This parameter must point to an object that is already a member of the list.
	//
	//# \desc
	//# The $InsertListElementAfter$ function adds the object specified by the $element$ parameter to a list at the
	//# position after the object specified by the $after$ parameter. If the object is already a member
	//# of the list, then it is moved to the new position. If the $after$ parameter is $nullptr$, then the
	//# node is added to the beginning of the list. Otherwise, the $after$ parameter must specify an object that
	//# is already a member of the list for which this function is called.
	//#
	//# If the object being added is already a member of a different list of the same type, then it is first
	//# removed from that list before being added to the new list.
	//
	//# \also	$@List::InsertListElementBefore@$
	//# \also	$@List::PrependListElement@$
	//# \also	$@List::AppendListElement@$


	//# \function	List::RemoveListElement		Removes a particular element from a list.
	//
	//# \proto	void RemoveListElement(ListElement<type> *element);
	//
	//# \param	element		A pointer to the object to remove from the list.
	//
	//# \desc
	//# The $RemoveListElement$ function removes the object specified by the $element$ parameter from a list.
	//# The object must belong to the list for which the $RemoveListElement$ function is called, or the internal
	//# links will become corrupted.
	//
	//# \also	$@List::RemoveAllListElements@$
	//# \also	$@List::PurgeList@$
	//# \also	$@ListElement::Detach@$


	//# \function	List::RemoveAllListElements		Removes all elements from a list.
	//
	//# \proto	void RemoveAllListElements(void);
	//
	//# \desc
	//# The $RemoveAllListElements$ function removes all objects contained in a list, but does not delete them.
	//# The list is subsequently empty.
	//
	//# \also	$@List::RemoveListElement@$
	//# \also	$@List::PurgeList@$
	//# \also	$@ListElement::Detach@$


	//# \function	List::PurgeList		Deletes all elements in a list.
	//
	//# \proto	void PurgeList(void);
	//
	//# \desc
	//# The $PurgeList$ function deletes all objects contained in a list. The list is subsequently empty.
	//# To remove all elements of a list without destroying them, use the $@List::RemoveAllListElements@$ function.
	//
	//# \also	$@List::RemoveListElement@$
	//# \also	$@List::RemoveAllListElements@$
	//# \also	$@ListElement::Detach@$


	template <class type>
	class List : public ListBase
	{
		public:

			inline List() = default;

			type *operator [](machine index) const
			{
				return (static_cast<type *>(static_cast<ListElement<type> *>(ListBase::operator [](index))));
			}

			type *GetFirstListElement(void) const
			{
				return (static_cast<type *>(static_cast<ListElement<type> *>(ListBase::GetFirstListElement())));
			}

			type *GetLastListElement(void) const
			{
				return (static_cast<type *>(static_cast<ListElement<type> *>(ListBase::GetLastListElement())));
			}

			ListIterator<type> begin(void) const
			{
				return (ListIterator<type>(static_cast<type *>(static_cast<ListElement<type> *>(ListBase::GetFirstListElement()))));
			}

			ListIterator<type> end(void) const
			{
				return (ListIterator<type>(nullptr));
			}

			bool Member(const ListElement<type> *element) const
			{
				return (ListBase::Member(element));
			}

			void PrependListElement(ListElement<type> *element)
			{
				ListBase::PrependListElement(element);
			}

			void AppendListElement(ListElement<type> *element)
			{
				ListBase::AppendListElement(element);
			}

			void InsertListElementBefore(ListElement<type> *element, ListElement<type> *before)
			{
				ListBase::InsertListElementBefore(element, before);
			}

			void InsertListElementAfter(ListElement<type> *element, ListElement<type> *after)
			{
				ListBase::InsertListElementAfter(element, after);
			}

			void RemoveListElement(ListElement<type> *element)
			{
				ListBase::RemoveListElement(element);
			}
	};
}


#endif
