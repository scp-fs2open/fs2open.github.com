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


#ifndef TSText_h
#define TSText_h


//# \component	Utility Library
//# \prefix		Utilities/


#include "TSPlatform.h"


#define TERATHON_TEXT 1


namespace Terathon
{
	//# \namespace	Text	Contains miscellaneous text functions.
	//
	//# The $Text$ namespace contains miscellaneous text functions.
	//
	//# \def	namespace Terathon { namespace Text {...} }
	//
	//# \also	$@String@$


	//# \function	Text::GetTextLength		Returns the length of a character string.
	//
	//# \proto	int32 GetTextLength(const char *text);
	//
	//# \param	text	A pointer to a null-terminated character string.
	//
	//# \desc
	//# The $GetTextLength$ function returns the number of bytes occupied by the string specified by the
	//# $text$ parameter.
	//#
	//# The string must be null-terminated by a zero byte. The null terminator is not included in the length.
	//
	//# \also	$@Text::GetUnicodeCharCount@$


	//# \function	Text::GetUnicodeCharCount		Returns the number of UTF-8 characters in a string.
	//
	//# \proto	int32 GetUnicodeCharCount(const char *text);
	//
	//# \param	text	A pointer to a null-terminated character string.
	//
	//# \desc
	//# The $GetUnicodeCharCount$ function returns the number of individual characters encoded as UTF-8 in the string
	//# specified by the $text$ parameter. This number is at most the number of bytes occupied by the string, but
	//# it is less for strings containing multi-byte characters. Each character is counted as one unit regardless
	//# of the number of bytes it occupies.
	//#
	//# The string must be null-terminated by a zero byte. The null terminator is not included in the length.
	//
	//# \also	$@Text::GetTextLength@$


	//# \function	Text::Hash		Calculates the hash value for a character string.
	//
	//# \proto	uint32 Hash(const char *text);
	//
	//# \param	text	A pointer to a null-terminated character string.
	//
	//# \desc
	//# The $Hash$ function returns a 32-bit hash value for the character string specified by the $text$ parameter.
	//# The hash algorithm is designed so that different strings appear to have random hash values even if the strings
	//# only differ by a small amount.
	//#
	//# The hash value returned for the empty string is zero.
	//
	//# \note
	//# The $Hash$ function is <i>not</i> intended for cryptographic applications and does not produce a secure hash value.
	//# It should not be used for things like password storage.
	//
	//# \also	$@Utilities/HashTable@$


	//# \function	Text::FindChar		Searches for a particular byte value in a string.
	//
	//# \proto	int32 FindChar(const char *text, uint32 k);
	//# \proto	int32 FindChar(const char *text, uint32 k, int32 max);
	//
	//# \param	text	A pointer to a null-terminated character string.
	//# \param	k		An 8-bit bytes value to search for.
	//# \param	max		The maximum number of bytes to search.
	//
	//# \desc
	//# The $FindChar$ function searches the string specified by the $text$ parameter for the byte value given by the $k$ parameter.
	//# If the $max$ parameter is included, then it specifies the maximum number of bytes to search from the beginning of the string.
	//# Otherwise, the search continues until a zero byte is encountered. The string does not need to be null-terminated if the $max$
	//# parameter is included, but the search will still end if a zero byte is encountered before $max$ bytes have been searched.
	//#
	//# If the character given by $k$ is found, then the byte position within the string of the first occurrence is returned.
	//# If it is not found, then the return value is &minus;1.
	//
	//# \also	$@Text::FindUnquotedChar@$


	//# \function	Text::FindUnquotedChar		Searches for a particular byte value in a string.
	//
	//# \proto	int32 FindUnquotedChar(const char *text, uint32 k);
	//
	//# \param	text	A pointer to a null-terminated character string.
	//# \param	k		An 8-bit bytes value to search for.
	//
	//# \desc
	//# The $FindUnquotedChar$ function searches the string specified by the $text$ parameter for the byte value given by the $k$ parameter.
	//# Any text enclosed in double quotes (ASCII character 34) is ignored during the search. The quotes themselves are also ignored, so
	//# it is not possible to search for a quote character using this function.
	//#
	//# If the character given by $k$ is found outside of double quotes, then the byte position within the string of the first occurrence
	//# is returned. If it is not found, then the return value is &minus;1.
	//
	//# \also	$@Text::FindChar@$


	//# \function	Text::CompareText		Compares two strings for equality.
	//
	//# \proto	bool CompareText(const char *s1, const char *s2);
	//# \proto	bool CompareText(const char *s1, const char *s2, int32 max);
	//
	//# \param	s1		A pointer to the first character string.
	//# \param	s2		A pointer to the second character string.
	//# \param	max		The maximum number of bytes to compare.
	//
	//# \desc
	//# The $CompareText$ function returns a boolean value indicating whether the two strings specified by the
	//# $s1$ and $s2$ parameters are equal. The comparison is case-sensitive.
	//#
	//# In general, both strings should be null-terminated by a zero byte. The comparison only goes as far as the
	//# length of the shorter string. If the $max$ parameter is specified, then at most $max$ bytes of each string
	//# are compared, even if both strings are longer.
	//
	//# \also	$@Text::CompareTextCaseless@$
	//# \also	$@Text::CompareTextLessThan@$
	//# \also	$@Text::CompareTextLessThanCaseless@$
	//# \also	$@Text::CompareTextLessEqual@$
	//# \also	$@Text::CompareTextLessEqualCaseless@$


	//# \function	Text::CompareTextCaseless		Compares two strings for equality, ignoring case.
	//
	//# \proto	bool CompareTextCaseless(const char *s1, const char *s2);
	//# \proto	bool CompareTextCaseless(const char *s1, const char *s2, int32 max);
	//
	//# \param	s1		A pointer to the first character string.
	//# \param	s2		A pointer to the second character string.
	//# \param	max		The maximum number of bytes to compare.
	//
	//# \desc
	//# The $CompareTextCaseless$ function returns a boolean value indicating whether the two strings specified by the
	//# $s1$ and $s2$ parameters are equal when case is ignored. All lowercase characters between $a$ and $z$ are considered
	//# equal to their corresponding uppercase characters between $A$ and $Z$.
	//#
	//# In general, both strings should be null-terminated by a zero byte. The comparison only goes as far as the
	//# length of the shorter string. If the $max$ parameter is specified, then at most $max$ bytes of each string
	//# are compared, even if both strings are longer.
	//
	//# \also	$@Text::CompareText@$
	//# \also	$@Text::CompareTextLessThan@$
	//# \also	$@Text::CompareTextLessThanCaseless@$
	//# \also	$@Text::CompareTextLessEqual@$
	//# \also	$@Text::CompareTextLessEqualCaseless@$


	//# \function	Text::CompareTextLessThan		Determines whether one string precedes another.
	//
	//# \proto	bool CompareTextLessThan(const char *s1, const char *s2);
	//# \proto	bool CompareTextLessThan(const char *s1, const char *s2, int32 max);
	//
	//# \param	s1		A pointer to the first character string.
	//# \param	s2		A pointer to the second character string.
	//# \param	max		The maximum number of bytes to compare.
	//
	//# \desc
	//# The $CompareTextLessThan$ function returns a boolean value indicating whether the string specified by the $s1$
	//# parameter precedes the string specified by the $s2$ parameter in lexicographical order.
	//#
	//# In general, both strings should be null-terminated by a zero byte. The comparison only goes as far as the
	//# length of the shorter string. If the $max$ parameter is specified, then at most $max$ bytes of each string
	//# are compared, even if both strings are longer.
	//
	//# \also	$@Text::CompareTextLessThanCaseless@$
	//# \also	$@Text::CompareTextLessEqual@$
	//# \also	$@Text::CompareTextLessEqualCaseless@$
	//# \also	$@Text::CompareTextCaseless@$
	//# \also	$@Text::CompareText@$


	//# \function	Text::CompareTextLessThanCaseless		Determines whether one string precedes another, ignoring case.
	//
	//# \proto	bool CompareTextLessThanCaseless(const char *s1, const char *s2);
	//# \proto	bool CompareTextLessThanCaseless(const char *s1, const char *s2, int32 max);
	//
	//# \param	s1		A pointer to the first character string.
	//# \param	s2		A pointer to the second character string.
	//# \param	max		The maximum number of bytes to compare.
	//
	//# \desc
	//# The $CompareTextLessThan$ function returns a boolean value indicating whether the string specified by the $s1$
	//# parameter precedes the string specified by the $s2$ parameter in lexicographical order when case is ignored.
	//# All lowercase characters between $a$ and $z$ are considered equal to their corresponding uppercase characters between $A$ and $Z$.
	//#
	//# In general, both strings should be null-terminated by a zero byte. The comparison only goes as far as the
	//# length of the shorter string. If the $max$ parameter is specified, then at most $max$ bytes of each string
	//# are compared, even if both strings are longer.
	//
	//# \also	$@Text::CompareTextLessThan@$
	//# \also	$@Text::CompareTextLessEqual@$
	//# \also	$@Text::CompareTextLessEqualCaseless@$
	//# \also	$@Text::CompareTextCaseless@$
	//# \also	$@Text::CompareText@$


	//# \function	Text::CompareTextLessEqual		Determines whether one string precedes another or the two strings are equal.
	//
	//# \proto	bool CompareTextLessEqual(const char *s1, const char *s2);
	//# \proto	bool CompareTextLessEqual(const char *s1, const char *s2, int32 max);
	//
	//# \param	s1		A pointer to the first character string.
	//# \param	s2		A pointer to the second character string.
	//# \param	max		The maximum number of bytes to compare.
	//
	//# \desc
	//# The $CompareTextLessEqual$ function returns a boolean value indicating whether the string specified by the $s1$
	//# parameter precedes the string specified by the $s2$ parameter in lexicographical order, or the two strings are equal.
	//#
	//# In general, both strings should be null-terminated by a zero byte. The comparison only goes as far as the
	//# length of the shorter string. If the $max$ parameter is specified, then at most $max$ bytes of each string
	//# are compared, even if both strings are longer.
	//
	//# \also	$@Text::CompareTextLessEqualCaseless@$
	//# \also	$@Text::CompareTextLessThan@$
	//# \also	$@Text::CompareTextLessThanCaseless@$
	//# \also	$@Text::CompareTextCaseless@$
	//# \also	$@Text::CompareText@$


	//# \function	Text::CompareTextLessEqualCaseless		Determines whether one string precedes another or the two strings are equal, ignoring case.
	//
	//# \proto	bool CompareTextLessEqualCaseless(const char *s1, const char *s2);
	//# \proto	bool CompareTextLessEqualCaseless(const char *s1, const char *s2, int32 max);
	//
	//# \param	s1		A pointer to the first character string.
	//# \param	s2		A pointer to the second character string.
	//# \param	max		The maximum number of bytes to compare.
	//
	//# \desc
	//# The $CompareTextLessEqual$ function returns a boolean value indicating whether the string specified by the $s1$
	//# parameter precedes the string specified by the $s2$ parameter in lexicographical order, or the two strings are equal, when case is ignored.
	//# All lowercase characters between $a$ and $z$ are considered equal to their corresponding uppercase characters between $A$ and $Z$.
	//#
	//# In general, both strings should be null-terminated by a zero byte. The comparison only goes as far as the
	//# length of the shorter string. If the $max$ parameter is specified, then at most $max$ bytes of each string
	//# are compared, even if both strings are longer.
	//
	//# \also	$@Text::CompareTextLessEqual@$
	//# \also	$@Text::CompareTextLessThan@$
	//# \also	$@Text::CompareTextLessThanCaseless@$
	//# \also	$@Text::CompareTextCaseless@$
	//# \also	$@Text::CompareText@$


	//# \function	Text::ReadString		Reads a string of non-whitespace characters from a character string.
	//
	//# \proto	int32 ReadString(const char *text, char *string, int32 max);
	//
	//# \param	text		A pointer to a character string.
	//# \param	string		A pointer to a buffer that will receive the read characters.
	//# \param	max			The maximum length of a string that can be stored in the buffer specified by the $string$ parameter.
	//
	//# \desc
	//# The $ReadString$ function reads a sequence of non-whitespace characters from the string specified by
	//# the $text$ parameter and returns the number of characters that were read. The characters composing the
	//# string are stored in the buffer specified by the $string$ parameter. The maximum number of characters
	//# returned in this buffer is specified by the $max$ parameter. The buffer specified by the $string$ parameter
	//# should be large enough to hold $max$ characters plus a zero terminator.
	//#
	//# If the first character pointed to by the $text$ parameter is not a double quote, then the $ReadString$
	//# function reads characters until it encounters a whitespace character as defined in the description of the
	//# $@System/Data::GetWhitespaceLength@$ function. If the first character is a double quote, then the $ReadString$
	//# function reads all of the characters between the first double quote and a closing double quote, including any
	//# whitespace. (The quotes themselves are not returned in the buffer pointed to by the $string$ parameter.)
	//# Any quote preceded by a backslash is not considered a closing quote, but is instead included in the returned
	//# string without the backslash character.


	namespace Text
	{
		typedef bool TextComparator(const char *, const char *);


		TERATHON_API extern const char hexDigit[16];


		TERATHON_API int32 ValidateUnicodeChar(const char *text);
		TERATHON_API int32 ReadUnicodeChar(const char *text, uint32 *code);
		TERATHON_API int32 WriteUnicodeChar(char *text, uint32 code);

		TERATHON_API int32 GetUnicodeCharByteCount(uint32 code);
		TERATHON_API int32 GetUnicodeCharCount(const char *text);
		TERATHON_API int32 GetUnicodeCharCount(const char *text, int32 max);

		TERATHON_API int32 GetPreviousUnicodeCharByteCount(const char *text, int32 max);
		TERATHON_API int32 GetNextUnicodeCharByteCount(const char *text, int32 max);
		TERATHON_API int32 GetUnicodeCharStringByteCount(const char *text, int32 charCount);

		TERATHON_API int32 GetUnicodeStringLength(const uint16 *wideText);
		TERATHON_API void ConvertWideTextToString(const uint16 *wideText, char *string, int32 max);

		TERATHON_API int32 GetWideTextCharCount(const char *string);
		TERATHON_API void ConvertStringToWideText(const char *string, uint16 *wideText, int32 max);

		TERATHON_API int32 GetTextLength(const char *text);
		TERATHON_API uint32 Hash(const char *text);

		TERATHON_API int32 FindChar(const char *text, uint32 k);
		TERATHON_API int32 FindChar(const char *text, uint32 k, int32 max);
		TERATHON_API int32 FindUnquotedChar(const char *text, uint32 k);
		TERATHON_API int32 CountChars(const char *text, uint32 k, int32 max);

		TERATHON_API int32 CopyText(const char *source, char *dest);
		TERATHON_API int32 CopyText(const char *source, char *dest, int32 max);

		TERATHON_API bool CompareText(const char *s1, const char *s2);
		TERATHON_API bool CompareText(const char *s1, const char *s2, int32 max);
		TERATHON_API bool CompareTextCaseless(const char *s1, const char *s2);
		TERATHON_API bool CompareTextCaseless(const char *s1, const char *s2, int32 max);
		TERATHON_API bool CompareTextLessThan(const char *s1, const char *s2);
		TERATHON_API bool CompareTextLessThan(const char *s1, const char *s2, int32 max);
		TERATHON_API bool CompareTextLessThanCaseless(const char *s1, const char *s2);
		TERATHON_API bool CompareTextLessThanCaseless(const char *s1, const char *s2, int32 max);
		TERATHON_API bool CompareTextLessEqual(const char *s1, const char *s2);
		TERATHON_API bool CompareTextLessEqual(const char *s1, const char *s2, int32 max);
		TERATHON_API bool CompareTextLessEqualCaseless(const char *s1, const char *s2);
		TERATHON_API bool CompareTextLessEqualCaseless(const char *s1, const char *s2, int32 max);
		TERATHON_API bool CompareNumberedTextLessThan(const char *s1, const char *s2);
		TERATHON_API bool CompareNumberedTextLessThanCaseless(const char *s1, const char *s2);

		TERATHON_API int32 FindText(const char *s1, const char *s2);
		TERATHON_API int32 FindTextCaseless(const char *s1, const char *s2);

		TERATHON_API int32 IntegerToString(int32 num, char *text, int32 max);
		TERATHON_API int32 Integer64ToString(int64 num, char *text, int32 max);
		TERATHON_API int32 FloatToString(float num, char *text, int32 max);

		TERATHON_API int32 GetResourceNameLength(const char *text);
		TERATHON_API int32 GetDirectoryPathLength(const char *text);

		inline int32 GetPrefixDirectoryLength(const char *text)
		{
			return (FindChar(text, '/') + 1);
		}

		TERATHON_API int32 ReadInteger(const char *text, char *number, int32 max);
		TERATHON_API int32 ReadFloat(const char *text, char *number, int32 max);
		TERATHON_API int32 ReadString(const char *text, char *string, int32 max);
		TERATHON_API int32 ReadType(const char *text, uint32 *type);


		//# \class	StaticHash		Calculates a compile-time hash value for a string.
		//
		//# The $StaticHash$ class template is used to calculate a constant hash value for a string at compile time.
		//
		//# \def	template <...> class StaticHash
		//
		//# \desc
		//# The hash value for a string of up to 17 characters in length can be calculated as a compile-time constant
		//# by specifying each of the characters separately as template parameters. The constant hash value is accessed
		//# by using the $value$ enumerant member of the $StaticHash$ class. For example, the hash value of the
		//# string "foobar" is turned into a compile-time constant with the following expression:
		//
		//# \source
		//# Text::StaticHash<'f', 'o', 'o', 'b', 'a', 'r'>::value
		//
		//# \desc
		//# Hash values are case-sensitive.
		//
		//# \also	$@WorldMgr/Model::FindNode@$


		template <char c0, char c1 = 0, char c2 = 0, char c3 = 0, char c4 = 0, char c5 = 0, char c6 = 0, char c7 = 0, char c8 = 0, char c9 = 0, char c10 = 0, char c11 = 0, char c12 = 0, char c13 = 0, char c14 = 0, char c15 = 0, char c16 = 0>
		class StaticHash
		{
			private:

				template <char c, uint32 h>
				struct H
				{
					enum : uint32
					{
						hash = (c == 0) ? h : uint32((h ^ c) * 0x6B84DF47ULL + 1)
					};
				};

			public:

				enum : uint32
				{
					value = H<c16, H<c15, H<c14, H<c13, H<c12, H<c11, H<c10, H<c9, H<c8, H<c7, H<c6, H<c5, H<c4, H<c3, H<c2, H<c1, H<c0, 0>::hash>::hash>::hash>::hash>::hash>::hash>::hash>::hash>::hash>::hash>::hash>::hash>::hash>::hash>::hash>::hash>::hash
				};
		};
	}
}


#endif
