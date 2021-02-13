//
// This file is part of the Terathon OpenDDL Library, by Eric Lengyel.
// Copyright 1999-2021, Terathon Software LLC
//
// This software is licensed under the GNU General Public License version 3.
// Separate proprietary licenses are available from Terathon Software.
//
// THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER
// EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. 
//


#ifndef TSData_h
#define TSData_h


//# \component	System Utilities
//# \prefix		System/


#include "TSArray.h"
#include "TSString.h"
#include "TSHalf.h"
#include "TSTools.h"


#define TERATHON_DATA 1


namespace Terathon
{
	typedef uint32	DataType;
	typedef uint32	DataResult;


	//# \enum	DataType

	enum : DataType
	{
		kDataBool							= 'BOOL',		//## Boolean.
		kDataInt8							= 'INT8',		//## 8-bit signed integer.
		kDataInt16							= 'IN16',		//## 16-bit signed integer.
		kDataInt32							= 'IN32',		//## 32-bit signed integer.
		kDataInt64							= 'IN64',		//## 64-bit signed integer.
		kDataUInt8							= 'UIN8',		//## 8-bit unsigned integer.
		kDataUInt16							= 'UI16',		//## 16-bit unsigned integer.
		kDataUInt32							= 'UI32',		//## 32-bit unsigned integer.
		kDataUInt64							= 'UI64',		//## 64-bit unsigned integer.
		kDataHalf							= 'HALF',		//## 16-bit floating-point.
		kDataFloat							= 'FLOT',		//## 32-bit floating-point.
		kDataDouble							= 'DOUB',		//## 64-bit floating-point.
		kDataString							= 'STRG',		//## String.
		kDataRef							= 'RFNC',		//## Reference.
		kDataType							= 'TYPE',		//## Type.
		kDataBase64							= 'BS64'		//## Base64 data.
	};


	//# \enum	DataResult

	enum : DataResult
	{
		kDataOkay							= 0,
		kDataSyntaxError					= 'SYNT',		//## The syntax is invalid.
		kDataIdentifierEmpty				= 'IDEM',		//## No identifier was found where one was expected.
		kDataIdentifierIllegalChar			= 'IDIC',		//## An identifier contains an illegal character.
		kDataStringInvalid					= 'STIV',		//## A string literal is invalid.
		kDataStringIllegalChar				= 'STIC',		//## A string literal contains an illegal character.
		kDataStringIllegalEscape			= 'STIE',		//## A string literal contains an illegal escape sequence.
		kDataStringEndOfFile				= 'STEF',		//## The end of file was reached inside a string literal.
		kDataCharIllegalChar				= 'CHIC',		//## A character literal contains an illegal character.
		kDataCharIllegalEscape				= 'CHIE',		//## A character literal contains an illegal escape sequence.
		kDataCharEndOfFile					= 'CHEF',		//## The end of file was reached inside a character literal.
		kDataBoolInvalid					= 'BLIV',		//## A boolean value is not "true", "false", "0", or "1".
		kDataTypeInvalid					= 'TYIV',		//## A data type value does not name a primitive type.
		kDataBase64Invalid					= 'BSIV',		//## Base64 data is invalid.
		kDataIntegerOverflow				= 'INOV',		//## An integer value lies outside the range of representable values for the number of bits in its underlying type.
		kDataFloatOverflow					= 'FLOV',		//## A hexadecimal or binary literal used to represent a floating-point value contains more bits than the underlying type.
		kDataFloatInvalid					= 'FLIV',		//## A floating-point literal has an invalid format.
		kDataReferenceInvalid				= 'RFIV',		//## A reference uses an invalid syntax.
		kDataStructNameExists				= 'STNE',		//## A structure name is equal to a previously used structure name.
		kDataPropertySyntaxError			= 'PPSE',		//## A property list contains a syntax error.
		kDataPropertyInvalidType			= 'PPIT',		//## A property has specified an invalid type. This error is generated if the $@Structure::ValidateProperty@$ function does not specify a recognized data type.
		kDataPrimitiveSyntaxError			= 'PMSE',		//## A primitive data structure contains a syntax error.
		kDataPrimitiveIllegalArraySize		= 'PMAS',		//## A primitive data array size is too large.
		kDataPrimitiveInvalidFormat			= 'PMIF',		//## A primitive data structure contains data in an invalid format.
		kDataPrimitiveArrayUnderSize		= 'PMUS',		//## A primitive array contains too few elements.
		kDataPrimitiveArrayOverSize			= 'PMOS',		//## A primitive array contains too many elements.
		kDataPrimitiveInvalidState			= 'PMST',		//## A state identifier contained in primitive array data is not recognized. This error is generated when the $@Structure::GetStateValue@$ function returns $false$.
		kDataInvalidStructure				= 'IVST'		//## A structure contains a substructure of an invalid type, or a structure of an invalid type appears at the top level of the file. This error is generated when either the $@Structure::ValidateSubstructure@$ function or $@DataDescription::ValidateTopLevelStructure@$ function returns $false$.
	};


	//# \namespace	Data	Contains functions used to parse the Open Data Description Language (OpenDDL).
	//
	//# The $Data$ namespace contains functions used to parse the Open Data Description Language (OpenDDL).
	//# Many of the functions are also applicable to generic text parsing tasks.
	//
	//# \def	namespace Terathon { namespace Data {...} }
	//
	//# \also	$@Structure@$
	//# \also	$@DataDescription@$


	//# \function	Data::GetWhitespaceLength		Returns the number of whitespace characters at the beginning of a text string.
	//
	//# \proto	int32 GetWhitespaceLength(const char *text);
	//
	//# \param	text	A pointer to a text string.
	//
	//# \desc
	//# The $GetWhitespaceLength$ function returns the number of whitespace characters at the beginning of
	//# the text string specified by the $text$ parameter. All characters having an ASCII value between
	//# 1 and 32, inclusive, are considered whitespace by this function.
	//#
	//# The $GetWhitespaceLength$ function also treats comments as whitespace. If this function encounters two
	//# consecutive forward slashes, then it treats all of the characters between the slashes and the next newline
	//# character as a comment. If this function encounters a forward slash followed by an asterisk, then all of
	//# the characters between that occurrence and the next atserisk followed by a forward slash are treated as a
	//# comment. That is, all C++ style comments are considered to be whitespace using the same rules as in C++.
	//
	//# \also	$@Data::ReadIdentifier@$


	//# \function	Data::ReadIdentifier		Reads an identifier from a text string.
	//
	//# \proto	DataResult ReadIdentifier(const char *text, int32 *textLength);
	//# \proto	DataResult ReadIdentifier(const char *text, int32 *textLength, char *restrict identifier);
	//
	//# \param	text			A pointer to a text string.
	//# \param	textLength		A pointer to the location that receives the length of the identifier.
	//# \param	identifier		A pointer to a buffer that will receive the identifier string. If this is omitted, then the identifier string itself is not returned.
	//
	//# \desc
	//# The $ReadIdentifier$ function reads an identifier from the text string specified by the $text$ parameter
	//# and returns the number of characters that were read in the location specified by the $textLength$ parameter.
	//#
	//# An identifier is made up of a sequence of consecutive characters from the set {A-Z, a-z, 0-9, _}, that is,
	//# all uppercase and lowercase letters, all numbers, and the underscore. An identifier may not begin with a number.
	//#
	//# If no error occurs, then the return value is $kDataOkay$. Otherwise, the $ReadIdentifier$ function returns
	//# $kDataIdentifierEmpty$ if there are no characters in the identifier or $kDataIdentifierIllegalChar$ if an illegal
	//# character is encountered (or if the first character is a number). The return value depends only upon the contents
	//# of the text string.
	//#
	//# If the $identifier$ parameter is not $nullptr$, then the characters composing the identifier are copied into the
	//# buffer that it points to, and a zero byte terminator is added to the end. The $ReadIdentifier$ function can be
	//# called with the $identifier$ parameter set to $nullptr$ to determine the length of the identifier, and then it
	//# can be called a second time after allocating a buffer of the necessary size to actually read the identifier.
	//# Note that the returned length does not include the zero byte terminator, so any buffer allocated should be one
	//# byte larger than the returned length.
	//
	//# \also	$@Data::GetWhitespaceLength@$


	namespace Data
	{
		extern const int8 identifierCharState[256];

		TERATHON_API int32 GetWhitespaceLength(const char *text);

		TERATHON_API DataResult ReadDataType(const char *text, int32 *textLength, DataType *value);
		TERATHON_API DataResult ReadIdentifier(const char *text, int32 *textLength);
		TERATHON_API DataResult ReadIdentifier(const char *text, int32 *textLength, char *restrict identifier);
		TERATHON_API DataResult ReadStringLiteral(const char *text, int32 *textLength, int32 *stringLength, char *restrict string = nullptr);
		TERATHON_API DataResult ReadBoolLiteral(const char *text, int32 *textLength, bool *value);
		TERATHON_API DataResult ReadIntegerLiteral(const char *text, int32 *textLength, uint64 *value);

		template <typename type>
		TERATHON_API DataResult ReadFloatLiteral(const char *text, int32 *textLength, type *value);
	}


	//# \class	StructureRef		Represents a structure reference in an OpenDDL file.
	//
	//# The $StructureRef$ class represents a structure reference in an OpenDDL file.
	//
	//# \def	class StructureRef
	//
	//# \ctor	StructureRef(bool global = true);
	//
	//# \param	global		A boolean value that indicates whether the reference is global.
	//
	//# \desc
	//# The $StructureRef$ class holds an array of structure names that compose an OpenDDL reference.
	//# A reference can be global or local, depending on whether the first name in the sequence is a
	//# global name or local name. Only the first name can be a global name, and the rest, if any,
	//# are always local names.
	//#
	//# The $@StructureRef::GetNameArray@$ function can be used to retrieve the array of
	//# $@Utilities/String@$ objects containing the sequence of names stored in the reference. For a
	//# null reference, this array is empty. For non-null references, the $@StructureRef::GetGlobalRefFlag@$
	//# function can be called to determine whether a reference is global or local.
	//#
	//# The $@StructureRef::AddName@$ function is used to add names to a reference. Initially,
	//# a $StructureRef$ object is a null reference, and thus its name array is empty.
	//
	//# \also	$@DataDescription::FindStructure@$
	//# \also	$@Structure::FindStructure@$


	//# \function	StructureRef::GetNameArray		Returns the array of names stored in a reference.
	//
	//# \proto	const ImmutableArray<String<>>& GetNameArray(void) const;
	//
	//# \desc
	//# The $GetNameArray$ function returns the array of names stored in a structure reference.
	//# The $@Utilities/Array::GetArrayElementCount@$ function can be used to retrieve the number of names
	//# in the array, and the $[]$ operator can be used to retrieve each individual name in the array.
	//# For a null reference, the name array is empty (i.e., has zero elements).
	//
	//# \also	$@StructureRef::GetGlobalRefFlag@$
	//# \also	$@StructureRef::AddName@$
	//# \also	$@StructureRef::Reset@$
	//# \also	$@Utilities/Array@$


	//# \function	StructureRef::GetGlobalRefFlag		Returns a boolean value indicating whether a reference is global.
	//
	//# \proto	bool GetGlobalRefFlag(void) const;
	//
	//# \desc
	//# The $GetGlobalRefFlag$ function returns $true$ if the structure reference is global, and it returns
	//# $false$ if the structure reference is local. A structure reference is global if the first name in its
	//# name array is a global name, as specified in the OpenDDL file.
	//
	//# \also	$@StructureRef::GetNameArray@$
	//# \also	$@StructureRef::AddName@$
	//# \also	$@StructureRef::Reset@$


	//# \function	StructureRef::AddName		Adds a name to a reference.
	//
	//# \proto	void AddName(String<>&& name);
	//
	//# \param	name	The name to add to the sequence stored in the reference. The dollar sign or percent sign should be omitted.
	//
	//# \desc
	//# The $AddName$ function adds the name specified by the $name$ parameter to the array of names stored
	//# in a structure reference. A move constructor is used to add the name to the array, so the string object
	//# passed in to this function becomes the empty string upon return.
	//
	//# \also	$@StructureRef::GetNameArray@$
	//# \also	$@StructureRef::GetGlobalRefFlag@$
	//# \also	$@StructureRef::Reset@$


	//# \function	StructureRef::Reset		Resets a reference to an empty sequence of names.
	//
	//# \proto	void Reset(bool global = true);
	//
	//# \param	global		A boolean value that indicates whether the reference is global.
	//
	//# \desc
	//# The $Reset$ function removes all of the names stored in a structure reference, making the name
	//# array empty. Upon return, the structure reference is a null reference. New names can be added
	//# to the reference by calling the $@StructureRef::AddName@$ function. The $global$ parameter
	//# specifies whether the first name added is a global name or local name.
	//
	//# \also	$@StructureRef::AddName@$
	//# \also	$@StructureRef::GetNameArray@$
	//# \also	$@StructureRef::GetGlobalRefFlag@$


	class StructureRef
	{
		private:

			Array<String<>, 1>		nameArray;
			bool					globalRefFlag;

		public:

			TERATHON_API StructureRef(bool global = true);
			TERATHON_API ~StructureRef();

			const ImmutableArray<String<>>& GetNameArray(void) const
			{
				return (nameArray);
			}

			bool GetGlobalRefFlag(void) const
			{
				return (globalRefFlag);
			}

			void AddName(String<>&& name)
			{
				nameArray.AppendArrayElement(static_cast<String<>&&>(name));
			}

			TERATHON_API void Reset(bool global = true);
	};


	struct BoolDataType
	{
		typedef bool PrimType;

		enum : DataType
		{
			kStructureType = kDataBool
		};

		static DataResult ParseValue(const char *& text, PrimType *value);
	};


	struct Int8DataType
	{
		typedef int8 PrimType;

		enum : DataType
		{
			kStructureType = kDataInt8
		};

		static DataResult ParseValue(const char *& text, PrimType *value);
	};


	struct Int16DataType
	{
		typedef int16 PrimType;

		enum : DataType
		{
			kStructureType = kDataInt16
		};

		static DataResult ParseValue(const char *& text, PrimType *value);
	};


	struct Int32DataType
	{
		typedef int32 PrimType;

		enum : DataType
		{
			kStructureType = kDataInt32
		};

		static DataResult ParseValue(const char *& text, PrimType *value);
	};


	struct Int64DataType
	{
		typedef int64 PrimType;

		enum : DataType
		{
			kStructureType = kDataInt64
		};

		static DataResult ParseValue(const char *& text, PrimType *value);
	};


	struct UInt8DataType
	{
		typedef uint8 PrimType;

		enum : DataType
		{
			kStructureType = kDataUInt8
		};

		static DataResult ParseValue(const char *& text, PrimType *value);
	};


	struct UInt16DataType
	{
		typedef uint16 PrimType;

		enum : DataType
		{
			kStructureType = kDataUInt16
		};

		static DataResult ParseValue(const char *& text, PrimType *value);
	};


	struct UInt32DataType
	{
		typedef uint32 PrimType;

		enum : DataType
		{
			kStructureType = kDataUInt32
		};

		static DataResult ParseValue(const char *& text, PrimType *value);
	};


	struct UInt64DataType
	{
		typedef uint64 PrimType;

		enum : DataType
		{
			kStructureType = kDataUInt64
		};

		static DataResult ParseValue(const char *& text, PrimType *value);
	};


	struct HalfDataType
	{
		typedef Half PrimType;

		enum : DataType
		{
			kStructureType = kDataHalf
		};

		static DataResult ParseValue(const char *& text, PrimType *value);
	};


	struct FloatDataType
	{
		typedef float PrimType;

		enum : DataType
		{
			kStructureType = kDataFloat
		};

		static DataResult ParseValue(const char *& text, PrimType *value);
	};


	struct DoubleDataType
	{
		typedef double PrimType;

		enum : DataType
		{
			kStructureType = kDataDouble
		};

		static DataResult ParseValue(const char *& text, PrimType *value);
	};


	struct StringDataType
	{
		typedef String<> PrimType;

		enum : DataType
		{
			kStructureType = kDataString
		};

		static DataResult ParseValue(const char *& text, PrimType *value);
	};


	struct RefDataType
	{
		typedef StructureRef PrimType;

		enum : DataType
		{
			kStructureType = kDataRef
		};

		static DataResult ParseValue(const char *& text, PrimType *value);
	};


	struct TypeDataType
	{
		typedef DataType PrimType;

		enum : DataType
		{
			kStructureType = kDataType
		};

		static DataResult ParseValue(const char *& text, PrimType *value);
	};


	struct Base64DataType
	{
		typedef Buffer PrimType;

		enum : DataType
		{
			kStructureType = kDataBase64
		};

		static DataResult ParseValue(const char *& text, PrimType *value);
	};


	namespace Data
	{
		TERATHON_API int32 StringToInt32(const char *text);
		TERATHON_API float StringToFloat(const char *text);
	}
}


#endif
