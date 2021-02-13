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


#ifndef TSTools_h
#define TSTools_h


//# \component	Utility Library
//# \prefix		Utilities/


#include "TSPlatform.h"


#define TERATHON_TOOLS 1


namespace Terathon
{
	#ifdef TERATHON_DEBUG

		TERATHON_API void Fatal(const char *c);
		TERATHON_API void Assert(bool b, const char *c);

	#else

		inline void Fatal(const char *)
		{
		}

/*		#if defined(_MSC_VER)

			#define Assert(b, c) __assume(b)

		#else

			inline void Assert(bool, const char *) {}

		#endif
*/
	#endif


	//# \class	Range	Encapsulates a range of values.
	//
	//# The $Range$ class template encapsulates a range of values.
	//
	//# \def	template <typename type> struct Range
	//
	//# \tparam		type	The type of value used to represent the beginning and end of a range.
	//
	//# \data	Range
	//
	//# \ctor	Range();
	//# \ctor	Range(const type& x, const type& y);
	//
	//# \param	x	The beginning of the range.
	//# \param	y	The end of the range.
	//
	//# \desc
	//# The $Range$ class template encapsulates a range of values of the type given by the
	//# $type$ class template.
	//#
	//# The default constructor leaves the beginning and end values of the range undefined.
	//# If the values $x$ and $y$ are supplied, then they are assigned to the beginning and
	//# end of the range, respectively.
	//
	//# \operator	type& operator [](machine index);
	//#				Returns a reference to the minimum value if $index$ is 0, and returns a reference to the maximum value if $index$ is 1.
	//#				The $index$ parameter must be 0 or 1.
	//
	//# \operator	const type& operator [](machine index) const;
	//#				Returns a constant reference to the minimum value if $index$ is 0, and returns a constant reference to the maximum value if $index$ is 1.
	//#				The $index$ parameter must be 0 or 1.
	//
	//# \operator	bool operator ==(const Range& range) const;
	//#				Returns a boolean value indicating whether two ranges are equal.
	//
	//# \operator	bool operator !=(const Range& range) const;
	//#				Returns a boolean value indicating whether two ranges are not equal.


	//# \function	Range::Set		Sets the beginning and end of a range.
	//
	//# \proto	Range& Set(const type& x, const type& y);
	//
	//# \param	x	The new beginning of the range.
	//# \param	y	The new end of the range.
	//
	//# \desc
	//# The $Set$ function sets the beginning and end of a range to the values given by the
	//# $x$ and $y$ parameters, respectively.


	//# \member		Range

	template <typename type>
	struct Range
	{
		type	min;		//## The beginning of the range.
		type	max;		//## The end of the range.

		inline Range() = default;

		Range(const Range& range)
		{
			min = range.min;
			max = range.max;
		}

		Range(const type& x, const type& y)
		{
			min = x;
			max = y;
		}

		type& operator [](machine index)
		{
			return ((&min)[index]);
		}

		const type& operator [](machine index) const
		{
			return ((&min)[index]);
		}

		Range& operator =(const Range& range)
		{
			min = range.min;
			max = range.max;
			return (*this);
		}

		Range& Set(const type& x, const type& y)
		{
			min = x;
			max = y;
			return (*this);
		}

		bool operator ==(const Range& range) const
		{
			return ((min == range.min) && (max == range.max));
		}

		bool operator !=(const Range& range) const
		{
			return ((min != range.min) || (max != range.max));
		}
	};


	class Buffer
	{
		private:

			char		*bufferStorage;
			uint32		bufferSize;

			Buffer(const Buffer& buffer) = delete;
			Buffer& operator =(const Buffer& s) = delete;
			Buffer& operator =(Buffer&& s) = delete;

		public:

			Buffer()
			{
				bufferSize = 0;
				bufferStorage = nullptr;
			}

			explicit Buffer(uint32 size)
			{
				bufferSize = size;
				bufferStorage = new char[size];
			}

			Buffer(Buffer&& buffer)
			{
				bufferStorage = buffer.bufferStorage;
				bufferSize = buffer.bufferSize;
				buffer.bufferStorage = nullptr;
				buffer.bufferSize = 0;
			}

			~Buffer()
			{
				delete[] bufferStorage;
			}

			operator void *(void) const
			{
				return (bufferStorage);
			}

			template <typename type>
			type *GetPointer(void) const
			{
				return (reinterpret_cast<type *>(bufferStorage));
			}

			void AllocateBuffer(uint32 size)
			{
				if (size != bufferSize)
				{
					bufferSize = size;
					delete[] bufferStorage;
					bufferStorage = new char[size];
				}
			}
	};


	//# \class	Holder		A helper class the wraps a pointer to an object.
	//
	//# \def	template <class type> class Holder
	//
	//# \tparam	type	The type of object to which the $Holder$ object refers.
	//
	//# \ctor	explicit Holder(type *ptr);
	//
	//# \param	ptr		A pointer to the object that is wrapped by the $Holder$ object.
	//
	//# \desc
	//# The $Holder$ class template is a helper class that wraps a pointer to an object.
	//# When a $Holder$ object is destroyed (usually by going out of scope), the object
	//# that was passed into the $ptr$ parameter when the $Holder$ object was constructed
	//# is automatically deleted with the $delete$ operator.
	//#
	//# A $Holder$ object behaves like a pointer to an object of the type given by the
	//# $type$ template parameter. A $Holder$ object can be passed as a function parameter
	//# wherever a pointer to $type$ is expected, and the $->$ operator can be used to access
	//# members of the object that the $Holder$ object wraps.
	//
	//# \also	$@AutoRelease@$


	template <class type>
	class Holder
	{
		private:

			type	*pointer;

			Holder(const Holder&) = delete;
			Holder& operator =(const Holder&) = delete;

		public:

			Holder()
			{
				pointer = nullptr;
			}

			Holder(type *ptr)
			{
				pointer = ptr;
			}

			Holder(Holder&& holder)
			{
				pointer = holder.pointer;
				holder.pointer = nullptr;
			}

			~Holder()
			{
				delete pointer;
			}

			operator type *(void) const
			{
				return (pointer);
			}

			type *operator ->(void) const
			{
				return (pointer);
			}

			type& operator *(void) const
			{
				return (*pointer);
			}

			Holder& operator =(type *ptr)
			{
				pointer = ptr;
				return (*this);
			}
	};


	template <class type>
	class Holder<type[]>
	{
		private:

			type	*pointer;

			Holder(const Holder&) = delete;
			Holder& operator =(const Holder&) = delete;

		public:

			Holder()
			{
				pointer = nullptr;
			}

			Holder(type *ptr)
			{
				pointer = ptr;
			}

			Holder(Holder&& holder)
			{
				pointer = holder.pointer;
				holder.pointer = nullptr;
			}

			~Holder()
			{
				delete[] pointer;
			}

			operator type *(void) const
			{
				return (pointer);
			}

			type *operator ->(void) const
			{
				return (pointer);
			}

			type& operator *(void) const
			{
				return (*pointer);
			}

			Holder& operator =(type *ptr)
			{
				pointer = ptr;
				return (*this);
			}
	};


	//# \class	Shared		The base class for reference-counted objects.
	//
	//# Objects inherit from the $Shared$ class when they contain shared data and
	//# need to be reference counted.
	//
	//# \def	class Shared
	//
	//# \ctor	Shared();
	//
	//# \desc
	//# The $Shared$ class encapsulates a reference count for objects that can be shared.
	//# Upon construction, the object's reference count is initialized to 1.
	//
	//# \important
	//# The destructor of the $Shared$ class does not have public access, and the destructors
	//# of any subclasses of the $Shared$ class should not have public access. Shared objects
	//# must be released by calling the $@Shared::Release@$ function.
	//
	//# \also	$@AutoRelease@$


	//# \function	Shared::GetReferenceCount		Returns an object's current reference count.
	//
	//# \proto		int32 GetReferenceCount(void) const;
	//
	//# \desc
	//# The $GetReferenceCount$ function returns the current reference count.
	//# When a shared object is constructed, its initial reference count is 1.
	//
	//# \also		$@Shared::Retain@$
	//# \also		$@Shared::Release@$


	//# \function	Shared::Retain		Increments an object's reference count.
	//
	//# \proto		int32 Retain(void);
	//
	//# \desc
	//# The $Retain$ function increments the reference count. For each call to $Retain$
	//# made for a particular shared object, a balancing call to $@Shared::Release@$
	//# decrements the reference count without destroying the object. The return value
	//# of the $Retain$ function is the new reference count.
	//
	//# \also		$@Shared::GetReferenceCount@$
	//# \also		$@Shared::Release@$


	//# \function	Shared::Release		Decrements an object's reference count.
	//
	//# \proto		virtual int32 Release(void);
	//
	//# \desc
	//# The $Release$ function decrements the reference count. If the reference count
	//# becomes zero, then the object is destroyed. The return value of the $Release$
	//# function is the new reference count.
	//
	//# \also		$@Shared::GetReferenceCount@$
	//# \also		$@Shared::Retain@$


	class Shared
	{
		private:

			int32		referenceCount = 1;

			Shared(const Shared&) = delete;
			Shared& operator =(const Shared&) = delete;

		protected:

			inline Shared() = default;
			virtual ~Shared() = default;

		public:

			int32 GetReferenceCount(void) const
			{
				return (referenceCount);
			}

			int32 Retain(void)
			{
				return (++referenceCount);
			}

			virtual int32 Release(void)
			{
				int32 count = --referenceCount;
				if (count == 0)
				{
					delete this;
				}

				return (count);
			}
	};


	//# \class	AutoRelease		A helper class the wraps a pointer to a shared object.
	//
	//# \def	template <class type> class AutoRelease
	//
	//# \tparam	type	The type of object to which the $AutoRelease$ object refers.
	//
	//# \ctor	explicit AutoRelease(type *ptr);
	//
	//# \param	ptr		A pointer to the shared object that is wrapped by the $AutoRelease$ object.
	//
	//# \desc
	//# The $AutoRelease$ class template is a helper class that wraps a pointer to a shared
	//# object. When an $AutoRelease$ object is destroyed (usually by going out of scope),
	//# the $Release$ function is automatically called for the object that was passed into
	//# the $ptr$ parameter when the $AutoRelease$ object was constructed.
	//#
	//# An $AutoRelease$ object behaves like a pointer to an object of the type given by the
	//# $type$ template parameter. An $AutoRelease$ object can be passed as a function parameter
	//# wherever a pointer to $type$ is expected, and the $->$ operator can be used to access
	//# members of the object that the $AutoRelease$ object wraps.
	//
	//# \also	$@Shared@$
	//# \also	$@Holder@$


	template <class type>
	class AutoRelease
	{
		private:

			type	*reference;

		public:

			explicit AutoRelease(type *ptr)
			{
				reference = ptr;
			}

			AutoRelease(const AutoRelease& ar)
			{
				reference = ar.reference;
				reference->Retain();
			}

			~AutoRelease()
			{
				reference->Release();
			}

			operator type *(void) const
			{
				return (reference);
			}

			type *const *operator &(void) const
			{
				return (&reference);
			}

			type *operator ->(void) const
			{
				return (reference);
			}
	};
}


#endif
