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


#ifndef TSString_h
#define TSString_h


//# \component	Utility Library
//# \prefix		Utilities/


#include "TSBasic.h"
#include "TSText.h"


#define TERATHON_STRING 1


namespace Terathon
{
	//# \class	String		Encapsulates a character string.
	//
	//# The $String$ class template encapsulates a character string having a fixed maximum length.
	//
	//# \def	template <int32 len = 0> class String
	//
	//# \tparam		len		The maximum length of the string, excluding the null terminator.
	//
	//# \ctor	String();
	//# \ctor	String(const String<len>& s);
	//# \ctor	String(const char *s);
	//# \ctor	String(const char *s, int32 length);
	//# \ctor	explicit String(int32 n);
	//# \ctor	explicit String(float n);
	//
	//# \param	s		A reference to another $String$ object or a pointer to a null-terminated
	//#					string that initializes the new $String$ object.
	//# \param	length	If specified, then the string is initialized with the character string pointed
	//#					to by the $s$ parameter, but at most $length$ characters are copied.
	//# \param	n		A signed integer or floating-point value that is converted to a decimal string
	//#					to initialize the new $String$ object.
	//
	//# \desc
	//# The $String$ class template can be used to store and manipulate strings. The $len$
	//# parameter establishes the maximum length of the string, excluding the null terminator.
	//# If the $len$ parameter is zero (the default if the parameter is not specified), then
	//# the string has unlimited length, and allocation for the memory used by the string is
	//# automatically managed.
	//#
	//# If the $len$ parameter is not zero, then the default constructor leaves the contents of the string
	//# undefined (and unterminated). If the $len$ parameter is zero, then the $String$ object is initially
	//# set to the empty string.
	//#
	//# $String$ objects can always be implicitly converted to a pointer to $char$ and thus can
	//# be accepted by any function expecting a parameter of type $char *$.
	//
	//# In addition to the member functions of the $String$ class, the $@Text@$ namespace contains several
	//# functions that are useful for manipulating character strings.
	//
	//# \operator	operator char *(void);
	//#				A $String$ object can be implicitly converted to an array of $char$.
	//
	//# \operator	operator const char *(void) const;
	//#				A $const String$ object can be implicitly converted to an array of $const char$.
	//
	//# \operator	String& operator =(const char *s);
	//#				Sets the contents of the $String$ object to the string pointed to by $s$.
	//#				If necessary, the string is truncated to the maximum length given by $len$.
	//
	//# \operator	String& operator +=(const char *s);
	//#				Appends the string pointed to by $s$ to the $String$ object, truncating to
	//#				the maximum length given by $len$.
	//
	//# \operator	String& operator +=(int32 n);
	//#				Converts the signed integer $n$ to a decimal string and appends it to the
	//#				$String$ object, truncating to the maximum length given by $len$.
	//
	//# \operator	String operator +(const char *s) const;
	//#				Returns a new $String$ object containing the concatenation with the string
	//#				pointed to by $s$. If necessary, the new string is truncated to the maximum
	//#				length given by $len$.
	//
	//# \operator	String operator +(int32 n) const;
	//#				Returns a new $String$ object containing the concatenation with the decimal
	//#				string corresponding to &n&. If necessary, the new string is truncated to the
	//#				maximum length given by $len$.
	//
	//# \operator	bool operator ==(const char *s) const;
	//#				Returns a boolean value indicating whether two strings have equal contents.
	//
	//# \operator	bool operator !=(const char *s) const;
	//#				Returns a boolean value indicating whether two strings have differing contents.
	//
	//# \operator	bool operator <(const char *s) const;
	//#				Returns a boolean value indicating whether the text in the $String$ object
	//#				precedes the text pointed to by $s$ in lexicographical order.
	//
	//# \operator	bool operator >(const char *s) const;
	//#				Returns a boolean value indicating whether the text in the $String$ object
	//#				follows the text pointed to by $s$ in lexicographical order.
	//
	//# \operator	bool operator <=(const char *s) const;
	//#				Returns a boolean value indicating whether the text in the $String$ object
	//#				precedes the text pointed to by $s$ in lexicographical order
	//#				or the two strings are equal.
	//
	//# \operator	bool operator >=(const char *s) const;
	//#				Returns a boolean value indicating whether the text in the $String$ object
	//#				follows the text pointed to by $s$ in lexicographical order
	//#				or the two strings are equal.
	//
	//# \also	$@Text@$


	//# \function	String::GetStringLength		Returns the length of a string.
	//
	//# \proto	int32 GetStringLength(void) const;
	//
	//# \desc
	//# The $GetStringLength$ function returns the length of the text contained in a $String$ object.
	//# The contents of the $String$ object must be defined so that the text is null-terminated.
	//# (The overloaded operators that manipulate the $String$ object all maintain the null terminator.)


	//# \function	String::ConvertToLowerCase		Converts each alphabetic character to lower case.
	//
	//# \proto	String<0>& ConvertToLowerCase(void);
	//
	//# \desc
	//# The $ConvertToLowerCase$ function examines each character in the string and converts any
	//# in the range $'A'$ to $'Z'$ to lower case.
	//#
	//# \note
	//# The $ConvertToLowerCase$ function is available only when the $len$ template parameter for the $String$
	//# class is the default value of zero.
	//
	//# \also	$@String::ConvertToUpperCase@$


	//# \function	String::ConvertToUpperCase		Converts each alphabetic character to upper case.
	//
	//# \proto	String<0>& ConvertToUpperCase(void);
	//
	//# \desc
	//# The $ConvertToUpperCase$ function examines each character in the string and converts any
	//# in the range $'a'$ to $'z'$ to upper case.
	//#
	//# \note
	//# The $ConvertToUpperCase$ function is available only when the $len$ template parameter for the $String$
	//# class is the default value of zero.
	//
	//# \also	$@String::ConvertToLowerCase@$


	template <int32 len = 0>
	class String
	{
		private:

			char	c[len + 1];

		public:

			inline String() = default;

			String(const String& s)
			{
				Text::CopyText(s.c, c, len);
			}

			String(const char *s)
			{
				Text::CopyText(s, c, len);
			}

			String(const char *s, int32 length)
			{
				Text::CopyText(s, c, Min(length, len));
			}

			explicit String(int32 n)
			{
				Text::IntegerToString(n, c, len);
			}

			explicit String(uint32 n)
			{
				Text::IntegerToString(n, c, len);
			}

			explicit String(int64 n)
			{
				Text::Integer64ToString(n, c, len);
			}

			explicit String(float n)
			{
				Text::FloatToString(n, c, len);
			}

			String(char c1, char c2, char c3, char c4)
			{
				c[0] = c1;
				c[1] = c2;
				c[2] = c3;
				c[3] = c4;
				c[4] = 0;
			}

			String& Set(const char *s, int32 length)
			{
				Text::CopyText(s, c, Min(length, len));
				return (*this);
			}

			operator char *(void)
			{
				return (c);
			}

			operator const char *(void) const
			{
				return (c);
			}

			String& operator =(const String& s)
			{
				Text::CopyText(s.c, c, len);
				return (*this);
			}

			String& operator =(const char *s)
			{
				Text::CopyText(s, c, len);
				return (*this);
			}

			String& operator =(int32 n)
			{
				Text::IntegerToString(n, c, len);
				return (*this);
			}

			String& operator =(uint32 n)
			{
				Text::IntegerToString(n, c, len);
				return (*this);
			}

			String& operator =(int64 n)
			{
				Text::Integer64ToString(n, c, len);
				return (*this);
			}

			String& operator =(float n)
			{
				Text::FloatToString(n, c, len);
				return (*this);
			}

			String& operator +=(const char *s)
			{
				int32 l = GetStringLength();
				Text::CopyText(s, &c[l], len - l);
				return (*this);
			}

			String& operator +=(char k)
			{
				int32 l = GetStringLength();
				if (l < len)
				{
					c[l] = k;
					c[l + 1] = 0;
				}

				return (*this);
			}

			String& operator +=(int32 n)
			{
				int32 l = GetStringLength();
				Text::IntegerToString(n, &c[l], len - l);
				return (*this);
			}

			String& operator +=(uint32 n)
			{
				int32 l = GetStringLength();
				Text::IntegerToString(n, &c[l], len - l);
				return (*this);
			}

			String& operator +=(int64 n)
			{
				int32 l = GetStringLength();
				Text::Integer64ToString(n, &c[l], len - l);
				return (*this);
			}

			String& operator +=(uint64 n)
			{
				int32 l = GetStringLength();
				Text::Integer64ToString(n, &c[l], len - l);
				return (*this);
			}

			String operator +(const char *s) const
			{
				return (String(c) += s);
			}

			String operator +(int32 n) const
			{
				return (String(c) += n);
			}

			String operator +(uint32 n) const
			{
				return (String(c) += n);
			}

			String operator +(int64 n) const
			{
				return (String(c) += n);
			}

			bool operator ==(const char *s) const
			{
				return (Text::CompareTextCaseless(c, s));
			}

			bool operator !=(const char *s) const
			{
				return (!Text::CompareTextCaseless(c, s));
			}

			bool operator <(const char *s) const
			{
				return (Text::CompareTextLessThanCaseless(c, s));
			}

			bool operator >=(const char *s) const
			{
				return (!Text::CompareTextLessThanCaseless(c, s));
			}

			bool operator <=(const char *s) const
			{
				return (Text::CompareTextLessEqualCaseless(c, s));
			}

			bool operator >(const char *s) const
			{
				return (!Text::CompareTextLessEqualCaseless(c, s));
			}

			int32 GetStringLength(void) const
			{
				return (Text::GetTextLength(c));
			}

			String& SetStringLength(int32 length)
			{
				c[length] = 0;
				return (*this);
			}

			String& AppendString(const char *s, int32 length)
			{
				int32 l = GetStringLength();
				Text::CopyText(s, &c[l], Min(len - l, length));
				return (*this);
			}
	};


	template <>
	class String<0>
	{
		private:

			enum
			{
				kStringAllocSize	= 64,
				kStringLocalSize	= 16
			};

			int32		logicalSize;
			int32		physicalSize;
			char		*stringPointer;
			char		localString[kStringLocalSize];

			TERATHON_API String(const char *s1, const char *s2);
			TERATHON_API String(int32 n, const char *s1);
			TERATHON_API String(uint32 n, const char *s1);
			TERATHON_API String(int64 n, const char *s1);

			static uint32 GetPhysicalSize(uint32 size);

			void Resize(int32 size);

		public:

			TERATHON_API String();
			TERATHON_API ~String();

			TERATHON_API String(const String& s);
			TERATHON_API String(String&& s);
			TERATHON_API String(const char *s);
			TERATHON_API String(const char *s, int32 length);
			TERATHON_API String(const uint16 *s);
			TERATHON_API explicit String(int32 n);
			TERATHON_API explicit String(uint32 n);
			TERATHON_API explicit String(int64 n);
			TERATHON_API explicit String(float n);

			operator char *(void)
			{
				return (stringPointer);
			}

			operator const char *(void) const
			{
				return (stringPointer);
			}

			bool operator ==(const char *s) const
			{
				return (Text::CompareTextCaseless(stringPointer, s));
			}

			bool operator !=(const char *s) const
			{
				return (!Text::CompareTextCaseless(stringPointer, s));
			}

			bool operator <(const char *s) const
			{
				return (Text::CompareTextLessThanCaseless(stringPointer, s));
			}

			bool operator >=(const char *s) const
			{
				return (!Text::CompareTextLessThanCaseless(stringPointer, s));
			}

			bool operator <=(const char *s) const
			{
				return (Text::CompareTextLessEqualCaseless(stringPointer, s));
			}

			bool operator >(const char *s) const
			{
				return (!Text::CompareTextLessEqualCaseless(stringPointer, s));
			}

			String operator +(const char *s) const
			{
				return (String(stringPointer, s));
			}

			String operator +(int32 n) const
			{
				return (String(n, stringPointer));
			}

			String operator +(uint32 n) const
			{
				return (String(n, stringPointer));
			}

			String operator +(int64 n) const
			{
				return (String(n, stringPointer));
			}

			int32 GetStringLength(void) const
			{
				return (logicalSize - 1);
			}

			TERATHON_API void PurgeString(void);
			TERATHON_API String& Set(const char *s, int32 length);

			TERATHON_API String& operator =(String&& s);
			TERATHON_API String& operator =(const String& s);
			TERATHON_API String& operator =(const char *s);
			TERATHON_API String& operator =(int32 n);
			TERATHON_API String& operator =(uint32 n);
			TERATHON_API String& operator =(int64 n);
			TERATHON_API String& operator =(float n);
			TERATHON_API String& operator +=(const String& s);
			TERATHON_API String& operator +=(const char *s);
			TERATHON_API String& operator +=(char k);
			TERATHON_API String& operator +=(int32 n);
			TERATHON_API String& operator +=(uint32 n);
			TERATHON_API String& operator +=(int64 n);
			TERATHON_API String& operator +=(uint64 n);

			TERATHON_API String& SetStringLength(int32 length);
			TERATHON_API String& AppendString(const char *s, int32 length);

			TERATHON_API String& ConvertToLowerCase(void);
			TERATHON_API String& ConvertToUpperCase(void);
			TERATHON_API String& ReplaceChar(char x, char y);

			TERATHON_API String& EncodeEscapeSequences(void);
	};


	class ConstCharKey
	{
		private:

			const char		*ptr;

		public:

			inline ConstCharKey() = default;

			ConstCharKey(const char *c)
			{
				ptr = c;
			}

			template <int32 len>
			ConstCharKey(const String<len>& s)
			{
				ptr = s;
			}

			operator const char *(void) const
			{
				return (ptr);
			}

			ConstCharKey& operator =(const char *c)
			{
				ptr = c;
				return (*this);
			}

			bool operator ==(const char *c) const
			{
				return (Text::CompareText(ptr, c));
			}

			bool operator !=(const char *c) const
			{
				return (!Text::CompareText(ptr, c));
			}

			bool operator <(const char *c) const
			{
				return (Text::CompareTextLessThan(ptr, c));
			}
	};


	class StringKey
	{
		private:

			const char		*ptr;

		public:

			inline StringKey() = default;

			StringKey(const char *c)
			{
				ptr = c;
			}

			template <int32 len>
			StringKey(const String<len>& s)
			{
				ptr = s;
			}

			operator const char *(void) const
			{
				return (ptr);
			}

			StringKey& operator =(const char *c)
			{
				ptr = c;
				return (*this);
			}

			bool operator ==(const char *c) const
			{
				return (Text::CompareTextCaseless(ptr, c));
			}

			bool operator !=(const char *c) const
			{
				return (!Text::CompareTextCaseless(ptr, c));
			}

			bool operator <(const char *c) const
			{
				return (Text::CompareTextLessThanCaseless(ptr, c));
			}
	};


	class FileNameKey
	{
		private:

			const char		*ptr;

		public:

			inline FileNameKey() = default;

			FileNameKey(const char *c)
			{
				ptr = c;
			}

			template <int32 len>
			FileNameKey(const String<len>& s)
			{
				ptr = s;
			}

			operator const char *(void) const
			{
				return (ptr);
			}

			FileNameKey& operator =(const char *c)
			{
				ptr = c;
				return (*this);
			}

			bool operator ==(const char *c) const
			{
				return (Text::CompareTextCaseless(ptr, c));
			}

			bool operator !=(const char *c) const
			{
				return (!Text::CompareTextCaseless(ptr, c));
			}

			bool operator <(const char *c) const
			{
				return (Text::CompareNumberedTextLessThanCaseless(ptr, c));
			}
	};


	namespace Text
	{
		inline String<15> IntegerToString(int32 num)
		{
			return (String<15>(num));
		}

		inline String<31> Integer64ToString(int64 num)
		{
			return (String<31>(num));
		}

		inline String<15> FloatToString(float num)
		{
			return (String<15>(num));
		}

		TERATHON_API String<31> Integer64ToHexString16(uint64 num);
		TERATHON_API String<15> IntegerToHexString8(uint32 num);
		TERATHON_API String<7> IntegerToHexString4(uint32 num);
		TERATHON_API String<3> IntegerToHexString2(uint32 num);

		TERATHON_API uint32 StringToType(const char *string);
		TERATHON_API String<4> TypeToString(uint32 type);
		TERATHON_API String<31> TypeToHexCharString(uint32 type);
	}
}


#endif
