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


#ifndef TSArray_h
#define TSArray_h


//# \component	Utility Library
//# \prefix		Utilities/


#include "TSBasic.h"


#define TERATHON_ARRAY 1


namespace Terathon
{
	//# \class	Array	A container class that holds an array of objects.
	//
	//# The $Array$ class represents a dynamically resizable array of objects
	//# for which any entry can be accessed in constant time.
	//
	//# \def	template <typename type, int32 baseCount = 0> class Array final : public ImmutableArray<type>
	//
	//# \tparam		type			The type of the class that can be stored in the array.
	//# \tparam		baseCount		The minimum number of array elements for which storage is available inside the $Array$ object itself.
	//
	//# \ctor	explicit Array(int32 count = 0);
	//
	//# \param	count	The number of array elements for which space is initially reserved in the array's storage.
	//
	//# \desc
	//# The $Array$ class represents a homogeneous array of objects whose type is given by the
	//# $type$ template parameter. Upon construction, the initial size of the array is zero, but
	//# space is reserved for the number of objects given by the $count$ parameter. The array is
	//# stored contiguously in memory, allowing constant-time random access to its elements.
	//#
	//# As elements are added to the array (using the $@Array::AppendArrayElement@$ function), the storage
	//# size is automatically increased to a size somewhat larger than that needed to store the new
	//# element. The cost of adding an element is thus amortized linear time.
	//#
	//# If the $baseCount$ template parameter is zero (the default), then storage space for the array
	//# elements is always allocated on the heap separately from the $Array$ object. If the value of
	//# $baseCount$ is greater than zero, then space for that number of array elements is built into the
	//# structure of the $Array$ object so that no separate allocations need to be made until the size
	//# of the array exceeds the value of $baseCount$.
	//#
	//# The $count$ parameter can only be specified if the $baseCount$ template parameter is zero.
	//#
	//# An $Array$ object can be implicitly converted to a pointer to its first element. This allows the
	//# use of the $[]$ operator to access individual elements of the array.
	//#
	//# It is possible to iterate over the elements of an array using a range-based for loop.
	//# This is illustrated by the following code, where $array$ is a variable of type $Array<type>$.
	//
	//# \source
	//# for (type& element : array)\n
	//# {\n
	//# \t...\n
	//# }
	//
	//# \privbase	ImmutableArray<type>	Used internally.
	//
	//# \also	$@List@$


	//# \function	Array::GetArrayElementCount		Returns the current size of an array.
	//
	//# \proto	int32 GetArrayElementCount(void) const;
	//
	//# \desc
	//# The $GetArrayElementCount$ function returns the number of objects currently stored in an array.
	//# When an array is constructed, its initial element count is zero.
	//
	//# \also	$@Array::SetArrayElementCount@$
	//# \also	$@Array::AppendArrayElement@$
	//# \also	$@Array::InsertArrayElement@$
	//# \also	$@Array::RemoveArrayElement@$
	//# \also	$@Array::RemoveLastArrayElement@$


	//# \function	Array::SetArrayElementCount		Sets the current size of an array.
	//
	//# \proto	void SetArrayElementCount(int32 count);
	//# \proto	void SetArrayElementCount(int32 count, const type& init);
	//
	//# \param	count	The new size of the array.
	//# \param	init	A reference to an object that is used to copy-construct new objects in the array.
	//
	//# \desc
	//# The $SetArrayElementCount$ function sets the number of objects currently stored in an array.
	//# If $count$ is greater than the current size of the array, then space is allocated for
	//# $count$ objects and each new object is copy-constructed from the value of the $init$ parameter.
	//# If $count$ is less than the current size of the array, then the logical size of the array
	//# is reduced, and each object beyond the new size of the array is destroyed in reverse order.
	//#
	//# If the $init$ parameter is omitted, then any new objects created are default-constructed if the
	//# type of object stored in the array is a non-POD type. If the type of object stored in the array is
	//# a POD type, then any new objects created are left uninitialized.
	//
	//# \also	$@Array::GetArrayElementCount@$
	//# \also	$@Array::AppendArrayElement@$
	//# \also	$@Array::InsertArrayElement@$
	//# \also	$@Array::RemoveArrayElement@$
	//# \also	$@Array::RemoveLastArrayElement@$


	//# \function	Array::AppendArrayElement		Adds an object to the end of an array.
	//
	//# \proto	template <typename T> type *AppendArrayElement(T&& element);
	//
	//# \param	element		The new element to add to the array.
	//
	//# \desc
	//# The $AppendArrayElement$ function increases the size of an array by one and either copy-constructs
	//# or move-constructs the new element using the object referenced by the $element$ parameter,
	//# depending on whether an lvalue reference or rvalue reference is passed to the function.
	//# The return value is a pointer to the newly appended element in the array.
	//
	//# \also	$@Array::InsertArrayElement@$
	//# \also	$@Array::RemoveArrayElement@$
	//# \also	$@Array::RemoveLastArrayElement@$
	//# \also	$@Array::GetArrayElementCount@$
	//# \also	$@Array::SetArrayElementCount@$


	//# \function	Array::InsertArrayElement		Inserts an object into an array.
	//
	//# \proto	template <typename T> void InsertArrayElement(int32 index, T&& element);
	//
	//# \param	index		The location at which the object is to be inserted.
	//# \param	element		The new element to insert into the array.
	//
	//# \desc
	//# The $InsertArrayElement$ function increases the size of an array by one, moves all of the existing
	//# elements at location $index$ or greater up by one, and either copy-constructs or move-constructs
	//# the new element into the array using the object referenced by the $element$ parameter, depending on
	//# whether an lvalue reference or rvalue reference is passed to the function. When the existing elements
	//# are moved, they are move-constructed in their new locations, and the old objects are destroyed.
	//#
	//# If the $index$ parameter is greater than or equal to the current size of the array, then the
	//# array is enlarged to the size $index&#x202F;+&#x202F;1$. In this case, elements between the old size and
	//# new size are default-constructed if the type of object stored in the array is a non-POD type, and the
	//# elements are left uninitialized if the type of object stored in the array is a POD type.
	//
	//# \also	$@Array::RemoveArrayElement@$
	//# \also	$@Array::AppendArrayElement@$
	//# \also	$@Array::GetArrayElementCount@$
	//# \also	$@Array::SetArrayElementCount@$


	//# \function	Array::RemoveArrayElement		Removes an object from an array.
	//
	//# \proto	void RemoveArrayElement(int32 index);
	//
	//# \param	index	The location at which to remove an object.
	//
	//# \desc
	//# The $RemoveArrayElement$ function decreases the size of an array by one, destroys the object at location
	//# $index$, and moves all of the existing elements at location $index&#x202F;+&#x202F;1$ or greater down by one.
	//# When the existing elements are moved, they are move-constructed to their new locations, and the old
	//# objects are destroyed.
	//#
	//# If the $index$ parameter is greater than or equal to the current size of the array, then
	//# calling the $RemoveArrayElement$ function has no effect.
	//
	//# \also	$@Array::RemoveLastArrayElement@$
	//# \also	$@Array::InsertArrayElement@$
	//# \also	$@Array::AppendArrayElement@$
	//# \also	$@Array::GetArrayElementCount@$
	//# \also	$@Array::SetArrayElementCount@$


	//# \function	Array::RemoveLastArrayElement		Removes the last object from an array.
	//
	//# \proto	void RemoveLastArrayElement(void);
	//
	//# \desc
	//# The $RemoveLastArrayElement$ function decreases the size of an array by one and destroys the object at the
	//# original end of the array.
	//#
	//# If the array is empty, then calling the $RemoveLastArrayElement$ function has no effect.
	//
	//# \also	$@Array::RemoveArrayElement@$
	//# \also	$@Array::InsertArrayElement@$
	//# \also	$@Array::AppendArrayElement@$
	//# \also	$@Array::GetArrayElementCount@$
	//# \also	$@Array::SetArrayElementCount@$


	//# \function	Array::ClearArray		Removes all objects from an array.
	//
	//# \proto	void ClearArray(void);
	//
	//# \desc
	//# The $ClearArray$ function destroys all objects in an array (in reverse order) and sets the size of
	//# the array to zero. The storage for the array is not deallocated, so this function is best used
	//# when the array is likely to be filled with a similar amount of data again. To both destroy all
	//# objects in an array and deallocate the storage, call the $@Array::PurgeArray@$ function.
	//
	//# \also	$@Array::PurgeArray@$
	//# \also	$@Array::RemoveArrayElement@$
	//# \also	$@Array::RemoveLastArrayElement@$
	//# \also	$@Array::SetArrayElementCount@$


	//# \function	Array::PurgeArray		Removes all objects from an array and deallocates storage.
	//
	//# \proto	void PurgeArray(void);
	//
	//# \desc
	//# The $PurgeArray$ function destroys all objects in an array (in reverse order) and sets the size of
	//# the array to zero. The storage for the array is also deallocated, returning the array to its
	//# initial state. To destory all objects in an array without deallocating the storage, call the
	//# $@Array::ClearArray@$ function.
	//
	//# \also	$@Array::ClearArray@$
	//# \also	$@Array::RemoveArrayElement@$
	//# \also	$@Array::RemoveLastArrayElement@$
	//# \also	$@Array::SetArrayElementCount@$


	//# \function	Array::FindArrayElementIndex		Finds a specific element in an array.
	//
	//# \proto	int32 FindArrayElementIndex(const type& element) const;
	//
	//# \param	element		The value of the element to find.
	//
	//# \desc
	//# The $FindArrayElementIndex$ function searches an array for the first element matching the value
	//# passed into the $element$ parameter based on the $==$ operator. If a match is found, its index
	//# is returned. If no match is found, then the return value is &minus;1. The running time of this
	//# function is <i>O</i>(<i>n</i>), where <i>n</i> is the number of elements in the array.


	template <typename type>
	class ImmutableArray
	{
		protected:

			int32		elementCount;
			int32		reservedCount;

			type		*arrayPointer;

			inline ImmutableArray() = default;
			ImmutableArray(const ImmutableArray& array) {}
			inline ~ImmutableArray() = default;

		public:

			operator type *(void)
			{
				return (arrayPointer);
			}

			operator type *(void) const
			{
				return (arrayPointer);
			}

			type *begin(void) const
			{
				return (arrayPointer);
			}

			type *end(void) const
			{
				return (arrayPointer + elementCount);
			}

			bool Empty(void) const
			{
				return (elementCount == 0);
			}

			int32 GetArrayElementCount(void) const
			{
				return (elementCount);
			}

			int32 FindArrayElementIndex(const type& element) const;
	};

	template <typename type>
	int32 ImmutableArray<type>::FindArrayElementIndex(const type& element) const
	{
		for (machine a = 0; a < elementCount; a++)
		{
			if (arrayPointer[a] == element)
			{
				return (a);
			}
		}

		return (-1);
	}


	template <typename type, int32 baseCount = 0>
	class Array final : public ImmutableArray<type>
	{
		private:

			using ImmutableArray<type>::elementCount;
			using ImmutableArray<type>::reservedCount;
			using ImmutableArray<type>::arrayPointer;

			char		arrayStorage[baseCount * sizeof(type)];

			void SetReservedCount(int32 count);

		public:

			explicit Array();
			Array(const Array& array);
			Array(Array&& array);
			~Array();

			void ClearArray(void);
			void PurgeArray(void);
			void ReserveArrayElementCount(int32 count);

			void SetArrayElementCount(int32 count);
			void SetArrayElementCount(int32 count, const type& init);
			type *AppendArrayElement(void);

			template <typename T>
			type *AppendArrayElement(T&& element);

			template <typename T>
			void InsertArrayElement(int32 index, T&& element);

			void RemoveArrayElement(int32 index);
			void RemoveLastArrayElement(void);
	};


	template <typename type, int32 baseCount>
	Array<type, baseCount>::Array()
	{
		elementCount = 0;
		reservedCount = baseCount;
		arrayPointer = reinterpret_cast<type *>(arrayStorage);
	}

	template <typename type, int32 baseCount>
	Array<type, baseCount>::Array(const Array& array)
	{
		elementCount = array.elementCount;
		reservedCount = array.reservedCount;

		if (elementCount > baseCount)
		{
			arrayPointer = reinterpret_cast<type *>(new char[sizeof(type) * reservedCount]);
		}
		else
		{
			arrayPointer = reinterpret_cast<type *>(arrayStorage);
		}

		for (machine a = 0; a < elementCount; a++)
		{
			new(&arrayPointer[a]) type(array.arrayPointer[a]);
		}
	}

	template <typename type, int32 baseCount>
	Array<type, baseCount>::Array(Array&& array)
	{
		elementCount = array.elementCount;
		reservedCount = array.reservedCount;

		if (elementCount > baseCount)
		{
			arrayPointer = array.arrayPointer;
		}
		else
		{
			arrayPointer = reinterpret_cast<type *>(arrayStorage);

			type *pointer = array.arrayPointer;
			for (machine a = 0; a < elementCount; a++)
			{
				new(&arrayPointer[a]) type(static_cast<type&&>(pointer[a]));
				pointer[a].~type();
			}
		}

		array.elementCount = 0;
		array.reservedCount = baseCount;
		array.arrayPointer = reinterpret_cast<type *>(array.arrayStorage);
	}

	template <typename type, int32 baseCount>
	Array<type, baseCount>::~Array()
	{
		type *pointer = arrayPointer + elementCount;
		for (machine a = elementCount - 1; a >= 0; a--)
		{
			(--pointer)->~type();
		}

		char *ptr = reinterpret_cast<char *>(arrayPointer);
		if (ptr != arrayStorage)
		{
			delete[] ptr;
		}
	}

	template <typename type, int32 baseCount>
	void Array<type, baseCount>::ClearArray(void)
	{
		type *pointer = arrayPointer + elementCount;
		for (machine a = elementCount - 1; a >= 0; a--)
		{
			(--pointer)->~type();
		}

		elementCount = 0;
	}

	template <typename type, int32 baseCount>
	void Array<type, baseCount>::PurgeArray(void)
	{
		type *pointer = arrayPointer + elementCount;
		for (machine a = elementCount - 1; a >= 0; a--)
		{
			(--pointer)->~type();
		}

		char *ptr = reinterpret_cast<char *>(arrayPointer);
		if (ptr != arrayStorage)
		{
			delete[] ptr;
		}

		elementCount = 0;
		reservedCount = baseCount;
		arrayPointer = reinterpret_cast<type *>(arrayStorage);
	}

	template <typename type, int32 baseCount>
	void Array<type, baseCount>::SetReservedCount(int32 count)
	{
		reservedCount = Max(Max(count, 4), reservedCount + Max((reservedCount / 2 + 3) & ~3, baseCount));
		type *newPointer = reinterpret_cast<type *>(new char[sizeof(type) * reservedCount]);

		type *pointer = arrayPointer;
		for (machine a = 0; a < elementCount; a++)
		{
			new(&newPointer[a]) type(static_cast<type&&>(*pointer));
			pointer->~type();
			pointer++;
		}

		char *ptr = reinterpret_cast<char *>(arrayPointer);
		if (ptr != arrayStorage)
		{
			delete[] ptr;
		}

		arrayPointer = newPointer;
	}

	template <typename type, int32 baseCount>
	void Array<type, baseCount>::ReserveArrayElementCount(int32 count)
	{
		if (count > reservedCount)
		{
			SetReservedCount(count);
		}
	}

	template <typename type, int32 baseCount>
	void Array<type, baseCount>::SetArrayElementCount(int32 count)
	{
		if (count > reservedCount)
		{
			SetReservedCount(count);
		}

		if (count > elementCount)
		{
			type *pointer = arrayPointer + (elementCount - 1);
			for (machine a = elementCount; a < count; a++)
			{
				new(++pointer) type;
			}
		}
		else if (count < elementCount)
		{
			type *pointer = arrayPointer + elementCount;
			for (machine a = elementCount - 1; a >= count; a--)
			{
				(--pointer)->~type();
			}
		}

		elementCount = count;
	}

	template <typename type, int32 baseCount>
	void Array<type, baseCount>::SetArrayElementCount(int32 count, const type& init)
	{
		if (count > reservedCount)
		{
			SetReservedCount(count);
		}

		if (count > elementCount)
		{
			type *pointer = arrayPointer + (elementCount - 1);
			for (machine a = elementCount; a < count; a++)
			{
				new(++pointer) type(init);
			}
		}
		else if (count < elementCount)
		{
			type *pointer = arrayPointer + elementCount;
			for (machine a = elementCount - 1; a >= count; a--)
			{
				(--pointer)->~type();
			}
		}

		elementCount = count;
	}

	template <typename type, int32 baseCount>
	type *Array<type, baseCount>::AppendArrayElement(void)
	{
		if (elementCount >= reservedCount)
		{
			SetReservedCount(elementCount + 1);
		}

		type *pointer = arrayPointer + elementCount;
		new(pointer) type;

		elementCount++;
		return (pointer);
	}

	template <typename type, int32 baseCount>
	template <typename T>
	type *Array<type, baseCount>::AppendArrayElement(T&& element)
	{
		if (elementCount >= reservedCount)
		{
			SetReservedCount(elementCount + 1);
		}

		type *pointer = arrayPointer + elementCount;
		new(pointer) type(static_cast<T&&>(element));

		elementCount++;
		return (pointer);
	}

	template <typename type, int32 baseCount>
	template <typename T>
	void Array<type, baseCount>::InsertArrayElement(int32 index, T&& element)
	{
		if (index >= elementCount)
		{
			int32 count = index + 1;
			if (count > reservedCount)
			{
				SetReservedCount(count);
			}

			type *pointer = &arrayPointer[elementCount - 1];
			for (machine a = elementCount; a < index; a++)
			{
				new(++pointer) type;
			}

			new (++pointer) type(static_cast<T&&>(element));
			elementCount = count;
		}
		else
		{
			int32 count = elementCount + 1;
			if (count > reservedCount)
			{
				SetReservedCount(count);
			}

			type *pointer = &arrayPointer[elementCount];
			for (machine a = elementCount; a > index; a--)
			{
				new(pointer) type(static_cast<type&&>(pointer[-1]));
				(--pointer)->~type();
			}

			new (&arrayPointer[index]) type(static_cast<T&&>(element));
			elementCount = count;
		}
	}

	template <typename type, int32 baseCount>
	void Array<type, baseCount>::RemoveArrayElement(int32 index)
	{
		if (index < elementCount)
		{
			type *pointer = &arrayPointer[index];
			pointer->~type();

			for (machine a = index + 1; a < elementCount; a++)
			{
				new(pointer) type(static_cast<type&&>(pointer[1]));
				(++pointer)->~type();
			}

			elementCount--;
		}
	}

	template <typename type, int32 baseCount>
	void Array<type, baseCount>::RemoveLastArrayElement(void)
	{
		int32 index = elementCount - 1;
		if (index >= 0)
		{
			type *pointer = &arrayPointer[index];
			pointer->~type();

			elementCount = index;
		}
	}


	template <typename type>
	class Array<type, 0> final : public ImmutableArray<type>
	{
		private:

			using ImmutableArray<type>::elementCount;
			using ImmutableArray<type>::reservedCount;
			using ImmutableArray<type>::arrayPointer;

			void SetReservedCount(int32 count);

		public:

			explicit Array(int32 count = 0);
			Array(const Array& array);
			Array(Array&& array);
			~Array();

			void ClearArray(void);
			void PurgeArray(void);
			void ReserveArrayElementCount(int32 count);

			void SetArrayElementCount(int32 count);
			void SetArrayElementCount(int32 count, const type& init);
			type *AppendArrayElement(void);

			template <typename T>
			type *AppendArrayElement(T&& element);

			template <typename T>
			void InsertArrayElement(int32 index, T&& element);

			void RemoveArrayElement(int32 index);
			void RemoveLastArrayElement(void);
	};


	template <typename type>
	Array<type, 0>::Array(int32 count)
	{
		elementCount = 0;
		reservedCount = count;

		arrayPointer = (count > 0) ? reinterpret_cast<type *>(new char[sizeof(type) * count]) : nullptr;
	}

	template <typename type>
	Array<type, 0>::Array(const Array& array)
	{
		elementCount = array.elementCount;
		reservedCount = array.reservedCount;

		if (reservedCount > 0)
		{
			arrayPointer = reinterpret_cast<type *>(new char[sizeof(type) * reservedCount]);
			for (machine a = 0; a < elementCount; a++)
			{
				new(&arrayPointer[a]) type(array.arrayPointer[a]);
			}
		}
		else
		{
			arrayPointer = nullptr;
		}
	}

	template <typename type>
	Array<type, 0>::Array(Array&& array)
	{
		elementCount = array.elementCount;
		reservedCount = array.reservedCount;
		arrayPointer = array.arrayPointer;

		array.elementCount = 0;
		array.reservedCount = 0;
		array.arrayPointer = nullptr;
	}

	template <typename type>
	Array<type, 0>::~Array()
	{
		type *pointer = arrayPointer + elementCount;
		for (machine a = elementCount - 1; a >= 0; a--)
		{
			(--pointer)->~type();
		}

		delete[] reinterpret_cast<char *>(arrayPointer);
	}

	template <typename type>
	void Array<type, 0>::ClearArray(void)
	{
		type *pointer = arrayPointer + elementCount;
		for (machine a = elementCount - 1; a >= 0; a--)
		{
			(--pointer)->~type();
		}

		elementCount = 0;
	}

	template <typename type>
	void Array<type, 0>::PurgeArray(void)
	{
		type *pointer = arrayPointer + elementCount;
		for (machine a = elementCount - 1; a >= 0; a--)
		{
			(--pointer)->~type();
		}

		delete[] reinterpret_cast<char *>(arrayPointer);

		elementCount = 0;
		reservedCount = 0;
		arrayPointer = nullptr;
	}

	template <typename type>
	void Array<type, 0>::SetReservedCount(int32 count)
	{
		reservedCount = Max(Max(count, 4), reservedCount + Max((reservedCount / 2 + 3) & ~3, 4));
		type *newPointer = reinterpret_cast<type *>(new char[sizeof(type) * reservedCount]);

		type *pointer = arrayPointer;
		if (pointer)
		{
			for (machine a = 0; a < elementCount; a++)
			{
				new(&newPointer[a]) type(static_cast<type&&>(*pointer));
				pointer->~type();
				pointer++;
			}

			delete[] reinterpret_cast<char *>(arrayPointer);
		}

		arrayPointer = newPointer;
	}

	template <typename type>
	void Array<type, 0>::ReserveArrayElementCount(int32 count)
	{
		if (count > reservedCount)
		{
			SetReservedCount(count);
		}
	}

	template <typename type>
	void Array<type, 0>::SetArrayElementCount(int32 count)
	{
		if (count > reservedCount)
		{
			SetReservedCount(count);
		}

		if (count > elementCount)
		{
			type *pointer = arrayPointer + (elementCount - 1);
			for (machine a = elementCount; a < count; a++)
			{
				new(++pointer) type;
			}
		}
		else if (count < elementCount)
		{
			type *pointer = arrayPointer + elementCount;
			for (machine a = elementCount - 1; a >= count; a--)
			{
				(--pointer)->~type();
			}
		}

		elementCount = count;
	}

	template <typename type>
	void Array<type, 0>::SetArrayElementCount(int32 count, const type& init)
	{
		if (count > reservedCount)
		{
			SetReservedCount(count);
		}

		if (count > elementCount)
		{
			type *pointer = arrayPointer + (elementCount - 1);
			for (machine a = elementCount; a < count; a++)
			{
				new(++pointer) type(init);
			}
		}
		else if (count < elementCount)
		{
			type *pointer = arrayPointer + elementCount;
			for (machine a = elementCount - 1; a >= count; a--)
			{
				(--pointer)->~type();
			}
		}

		elementCount = count;
	}

	template <typename type>
	type *Array<type, 0>::AppendArrayElement(void)
	{
		if (elementCount >= reservedCount)
		{
			SetReservedCount(elementCount + 1);
		}

		type *pointer = arrayPointer + elementCount;
		new(pointer) type;

		elementCount++;
		return (pointer);
	}

	template <typename type>
	template <typename T>
	type *Array<type, 0>::AppendArrayElement(T&& element)
	{
		if (elementCount >= reservedCount)
		{
			SetReservedCount(elementCount + 1);
		}

		type *pointer = arrayPointer + elementCount;
		new(pointer) type(static_cast<T&&>(element));

		elementCount++;
		return (pointer);
	}

	template <typename type>
	template <typename T>
	void Array<type, 0>::InsertArrayElement(int32 index, T&& element)
	{
		if (index >= elementCount)
		{
			int32 count = index + 1;
			if (count > reservedCount)
			{
				SetReservedCount(count);
			}

			type *pointer = &arrayPointer[elementCount - 1];
			for (machine a = elementCount; a < index; a++)
			{
				new(++pointer) type;
			}

			new (++pointer) type(static_cast<T&&>(element));
			elementCount = count;
		}
		else
		{
			int32 count = elementCount + 1;
			if (count > reservedCount)
			{
				SetReservedCount(count);
			}

			type *pointer = &arrayPointer[elementCount];
			for (machine a = elementCount; a > index; a--)
			{
				new(pointer) type(static_cast<type&&>(pointer[-1]));
				(--pointer)->~type();
			}

			new (&arrayPointer[index]) type(static_cast<T&&>(element));
			elementCount = count;
		}
	}

	template <typename type>
	void Array<type, 0>::RemoveArrayElement(int32 index)
	{
		if (index < elementCount)
		{
			type *pointer = &arrayPointer[index];
			pointer->~type();

			for (machine a = index + 1; a < elementCount; a++)
			{
				new(pointer) type(static_cast<type&&>(pointer[1]));
				(++pointer)->~type();
			}

			elementCount--;
		}
	}

	template <typename type>
	void Array<type, 0>::RemoveLastArrayElement(void)
	{
		int32 index = elementCount - 1;
		if (index >= 0)
		{
			type *pointer = &arrayPointer[index];
			pointer->~type();

			elementCount = index;
		}
	}
}


#endif
