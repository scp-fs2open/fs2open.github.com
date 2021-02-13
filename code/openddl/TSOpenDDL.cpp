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


#include "TSOpenDDL.h"
#include "TSTools.h"


using namespace Terathon;


/*
<nondigit>				::= "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" | "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z"
						  | "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" | "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" | "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z"
						  | "_"

<decimal-digit>			::= "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"

<hex-digit>				::= "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" | "A" | "B" | "C" | "D" | "E" | "F" | "a" | "b" | "c" | "d" | "e" | "f"

<octal-digit>			::= "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7"

<binary-digit>			::= "0" | "1"

<sign>					::= "" | "+" | "-"

<identifier>			::= <nondigit>
						  | <identifier><nondigit>
						  | <identifier><decimal-digit>

<global-name>			::= "$"<identifier>

<local-name>			::= "%"<identifier>

<name>					::= <global-name>
						  | <local-name>

<reference>				::= <name>
						  | <reference> <local-name>
						  | "null"

<bool-literal>			::= "false" | "0"
						  | "true" | "1"

<decimal-literal>		::= <decimal-digit>
						  | <decimal-literal><decimal-digit>
						  | <decimal-literal>"_"<decimal-digit>

<hex-literal>			::= "0x"<hex-digit>
						  | "0X"<hex-digit>
						  | <hex-literal><hex-digit>
						  | <hex-literal>"_"<hex-digit>

<octal-literal>			::= "0o"<octal-digit>
						  | "0O"<octal-digit>
						  | <octal-literal><octal-digit>
						  | <octal-literal>"_"<octal-digit>

<binary-literal>		::= "0b"<binary-digit>
						  | "0B"<binary-digit>
						  | <binary-literal><binary-digit>
						  | <binary-literal>"_"<binary-digit>

<char>					::= ASCII character in the ranges [0x0020,0x0026], [0x0028,0x005B], [0x005D,0x007E]
						  | <escape-char>

<char-seq>				::= <char>
						  | <char-seq><char>

<char-literal>			::= "'"<char-seq>"'"

<unsigned-literal>		::= <decimal-literal>
						  | <hex-literal>
						  | <octal-literal>
						  | <binary-literal>
						  | <char-literal>

<integer-literal>		::= <sign> <unsigned-literal>

<float-exponent>		::= "e"<sign><decimal-literal>
						  | "E"<sign><decimal-literal>

<float-magnitude>		::= <decimal-literal>"."<decimal-literal><float-exponent>
						  | <decimal-literal>"."<decimal-literal>
						  | <decimal-literal>"."
						  | <decimal-literal><float-exponent>
						  | <decimal-literal>
						  | "."<decimal-literal><float-exponent>
						  | "."<decimal-literal>
						  | <hex-literal>
						  | <octal-literal>
						  | <binary-literal>

<float-literal>			::= <sign> <float-magnitude>

<escape-char>			::= "\"'"' | "\'" | "\?" | "\\" | "\a" | "\b" | "\f" | "\n" | "\r" | "\t" | "\v"
						  | "\x"<hex-digit><hex-digit>

<string-char>			::= Unicode character in the ranges [0x0020,0x0021], [0x0023,0x005B], [0x005D,0x007E], [0x00A0,0xD7FF], [0xE000,0xFFFD], [0x010000,0x10FFFF]
						  | <escape-char>
						  | "\u"<hex-digit><hex-digit><hex-digit><hex-digit>
						  | "\U"<hex-digit><hex-digit><hex-digit><hex-digit><hex-digit><hex-digit>

<string-char-seq>		::= <string-char>
						  | <string-char-seq><string-char>
						  | ""

<string-literal>		::= '"'<string-char-seq>'"'
						  | <string-literal> '"'<string-char-seq>'"'

<base64-char>			::= "A" | "B" | "C" | "D" | "E" | "F" | "G" | "H" | "I" | "J" | "K" | "L" | "M" | "N" | "O" | "P" | "Q" | "R" | "S" | "T" | "U" | "V" | "W" | "X" | "Y" | "Z"
						  | "a" | "b" | "c" | "d" | "e" | "f" | "g" | "h" | "i" | "j" | "k" | "l" | "m" | "n" | "o" | "p" | "q" | "r" | "s" | "t" | "u" | "v" | "w" | "x" | "y" | "z"
						  | "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9" | "+" | "/"

<base64-char-seq>		::= <base64-char>
						  | <base64-char-seq><base64-char>
						  | ""

<base64-data>			::= <base64-char-seq>
						  | <base64-char-seq>"="
						  | <base64-char-seq>"=="

<data-type>				::= "bool" | "b"
						  | "int8" | "i8"
						  | "int16" | "i16"
						  | "int32" | "i32"
						  | "int64" | "i64"
						  | "unsigned_int8" | "uint8" | "u8"
						  | "unsigned_int16" | "uint16" | "u16"
						  | "unsigned_int32" | "uint32" | "u32"
						  | "unsigned_int64" | "uint64" | "u64"
						  | "half" | "h" | "float16" | "f16"
						  | "float" | "f" | "float32" | "f32"
						  | "double" | "d" | "float64" | "f64"
						  | "string" | "s"
						  | "ref" | "r"
						  | "type" | "t"
						  | "base64" | "z"

<property-value>		::= <bool-literal>
						  | <integer-literal>
						  | <float-literal>
						  | <string-literal>
						  | <reference>
						  | <data-type>
						  | <base64-data>

<property>				::= <identifier> "=" <property-value>
						  | <identifier>

<property-list>			::= <property-seq>
						  | ""

<property-seq>			::= <property>
						  | <property-seq> "," <property>

<bool-list>				::= <bool-literal>
						  | <bool-list> "," <bool-literal>

<integer-list>			::= <integer-literal>
						  | <integer-list> "," <integer-literal>

<float-list>			::= <float-literal>
						  | <float-list> "," <float-literal>

<string-list>			::= <string-literal>
						  | <string-list> "," <string-literal>

<ref-list>				::= <reference>
						  | <ref-list> "," <reference>

<type-list>				::= <data-type>
						  | <type-list> "," <data-type>

<base64-list>			::= <base64-data>
						  | <base64-list> "," <base64-data>

<data-list>				::= <bool-list>
						  | <integer-list>
						  | <float-list>
						  | <string-list>
						  | <ref-list>
						  | <type-list>
						  | <base64-list>
						  | ""

<state-flag>			::= "*"
						  | ""

<state-identifier>		::= <identifier>
						  | ""

<bool-array-list>		::= <state-identifier> "{" <bool-list> "}"
						  | <bool-array-list> "," <state-identifier> "{" <bool-list> "}"

<integer-array-list>	::= <state-identifier> "{" <integer-list> "}"
						  | <integer-array-list> "," <state-identifier> "{" <integer-list> "}"

<float-array-list>		::= <state-identifier> "{" <float-list> "}"
						  | <float-array-list> "," <state-identifier> "{" <float-list> "}"

<string-array-list>		::= <state-identifier> "{" <string-list> "}"
						  | <string-array-list> "," <state-identifier> "{" <string-list> "}"

<ref-array-list>		::= <state-identifier> "{" <ref-list> "}"
						  | <ref-array-list> "," <state-identifier> "{" <ref-list> "}"

<type-array-list>		::= <state-identifier> "{" <type-list> "}"
						  | <type-array-list> "," <state-identifier> "{" <type-list> "}"

<base64-array-list>		::= <state-identifier> "{" <base64-list> "}"
						  | <base64-array-list> "," <state-identifier> "{" <base64-list> "}"

<data-array-list>		::= <bool-array-list>
						  | <integer-array-list>
						  | <float-array-list>
						  | <string-array-list>
						  | <ref-array-list>
						  | <type-array-list>
						  | <base64-array-list>
						  | ""

<struct-name>			::= <name>
						  | ""

<struct-decl>			::= <identifier> <struct-name>
						  | <identifier> <struct-name> "(" <property-list> ")"

<structure>				::= <data-type> <struct-name> "{" <data-list> "}"
						  | <data-type> "[" <integer-literal> "]" <state-flag> <struct-name> "{" <data-array-list> "}"
						  | <struct-decl> "{" <struct-seq> "}"

<struct-seq>			::= <structure>
						  | <struct-seq> <structure>
						  | ""
*/


namespace Terathon
{
	namespace Data
	{
		bool ParseSign(const char *& text);
	}
}


Structure::Structure(StructureType type)
{
	structureType = type;
	baseStructureType = 0;
	globalNameFlag = true;
}

Structure::~Structure()
{
}

Structure *Structure::GetFirstSubstructure(StructureType type) const
{
	Structure *structure = GetFirstSubnode();
	while (structure)
	{
		if (structure->GetStructureType() == type)
		{
			return (structure);
		}

		structure = structure->GetNextSubnode();
	}

	return (nullptr);
}

Structure *Structure::GetLastSubstructure(StructureType type) const
{
	Structure *structure = GetLastSubnode();
	while (structure)
	{
		if (structure->GetStructureType() == type)
		{
			return (structure);
		}

		structure = structure->GetPreviousSubnode();
	}

	return (nullptr);
}

Structure *Structure::FindStructure(const StructureRef& reference, int32 index) const
{
	if ((index != 0) || (!reference.GetGlobalRefFlag()))
	{
		const ImmutableArray<String<>>& nameArray = reference.GetNameArray();

		int32 count = nameArray.GetArrayElementCount();
		if (count != 0)
		{
			Structure *structure = structureMap.FindMapElement(nameArray[index]);
			if (structure)
			{
				if (++index < count)
				{
					structure = structure->FindStructure(reference, index);
				}

				return (structure);
			}
		}
	}

	return (nullptr);
}

bool Structure::ValidateProperty(const DataDescription *dataDescription, const String<>& identifier, DataType *type, void **value)
{
	return (false);
}

bool Structure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	return (true);
}

bool Structure::GetStateValue(const String<>& identifier, uint32 *state) const
{
	return (false);
}

DataResult Structure::ProcessData(DataDescription *dataDescription)
{
	Structure *structure = GetFirstSubnode();
	while (structure)
	{
		DataResult result = structure->ProcessData(dataDescription);
		if (result != kDataOkay)
		{
			if (!dataDescription->errorStructure)
			{
				dataDescription->errorStructure = structure;
			}

			return (result);
		}

		structure = structure->GetNextSubnode();
	}

	return (kDataOkay);
}


PrimitiveStructure::PrimitiveStructure(StructureType type) : Structure(type)
{
	SetBaseStructureType(kStructurePrimitive);

	arraySize = 0;
	stateFlag = false;
}

PrimitiveStructure::PrimitiveStructure(StructureType type, uint32 size, bool state) : Structure(type)
{
	SetBaseStructureType(kStructurePrimitive);

	arraySize = size;
	stateFlag = state;
}

PrimitiveStructure::~PrimitiveStructure()
{
}


template <class type>
DataStructure<type>::DataStructure() : PrimitiveStructure(type::kStructureType)
{
}

template <class type>
DataStructure<type>::DataStructure(uint32 size, bool state) : PrimitiveStructure(type::kStructureType, size, state)
{
}

template <class type>
DataStructure<type>::~DataStructure()
{
}

template <class type>
DataResult DataStructure<type>::ParseData(const char *& text)
{
	int32 count = 0;

	uint32 arraySize = GetArraySize();
	if (arraySize == 0)
	{
		for (;;)
		{
			dataArray.SetArrayElementCount(count + 1);

			DataResult result = type::ParseValue(text, &dataArray[count]);
			if (result != kDataOkay)
			{
				return (result);
			}

			text += Data::GetWhitespaceLength(text);

			if (text[0] == ',')
			{
				text++;
				text += Data::GetWhitespaceLength(text);

				count++;
				continue;
			}

			break;
		}
	}
	else
	{
		const Structure *superStructure = GetSuperNode();
		bool stateFlag = GetStateFlag();
		uint32 stateValue = 0;
		for (;;)
		{
			if (stateFlag)
			{
				int32	length;

				DataResult result = Data::ReadIdentifier(text, &length);
				if (result == kDataOkay)
				{
					String<>	identifier;

					identifier.SetStringLength(length);
					Data::ReadIdentifier(text, &length, identifier);
					if (!superStructure->GetStateValue(identifier, &stateValue))
					{
						return (kDataPrimitiveInvalidState);
					}

					text += length;
					text += Data::GetWhitespaceLength(text);
				}
			}

			if (text[0] != '{')
			{
				return (kDataPrimitiveInvalidFormat);
			}

			text++;
			text += Data::GetWhitespaceLength(text);

			dataArray.SetArrayElementCount((count + 1) * arraySize);
			if (stateFlag)
			{
				stateArray.AppendArrayElement(stateValue);
			}

			for (umachine index = 0; index < arraySize; index++)
			{
				if (index != 0)
				{
					if (text[0] != ',')
					{
						return (kDataPrimitiveArrayUnderSize);
					}

					text++;
					text += Data::GetWhitespaceLength(text);
				}

				DataResult result = type::ParseValue(text, &dataArray[count * arraySize + index]);
				if (result != kDataOkay)
				{
					return (result);
				}

				text += Data::GetWhitespaceLength(text);
			}

			char c = text[0];
			if (c != '}')
			{
				return ((c == ',') ? kDataPrimitiveArrayOverSize : kDataPrimitiveInvalidFormat);
			}

			text++;
			text += Data::GetWhitespaceLength(text);

			if (text[0] == ',')
			{
				text++;
				text += Data::GetWhitespaceLength(text);

				count++;
				continue;
			}

			break;
		}
	}

	return (kDataOkay);
}


template TERATHON_API DataStructure<BoolDataType>::DataStructure(uint32 size, bool state);
template TERATHON_API DataStructure<BoolDataType>::~DataStructure();
template TERATHON_API DataStructure<Int8DataType>::DataStructure(uint32 size, bool state);
template TERATHON_API DataStructure<Int8DataType>::~DataStructure();
template TERATHON_API DataStructure<Int16DataType>::DataStructure(uint32 size, bool state);
template TERATHON_API DataStructure<Int16DataType>::~DataStructure();
template TERATHON_API DataStructure<Int32DataType>::DataStructure(uint32 size, bool state);
template TERATHON_API DataStructure<Int32DataType>::~DataStructure();
template TERATHON_API DataStructure<Int64DataType>::DataStructure(uint32 size, bool state);
template TERATHON_API DataStructure<Int64DataType>::~DataStructure();
template TERATHON_API DataStructure<UInt8DataType>::DataStructure(uint32 size, bool state);
template TERATHON_API DataStructure<UInt8DataType>::~DataStructure();
template TERATHON_API DataStructure<UInt16DataType>::DataStructure(uint32 size, bool state);
template TERATHON_API DataStructure<UInt16DataType>::~DataStructure();
template TERATHON_API DataStructure<UInt32DataType>::DataStructure(uint32 size, bool state);
template TERATHON_API DataStructure<UInt32DataType>::~DataStructure();
template TERATHON_API DataStructure<UInt64DataType>::DataStructure(uint32 size, bool state);
template TERATHON_API DataStructure<UInt64DataType>::~DataStructure();
template TERATHON_API DataStructure<HalfDataType>::DataStructure(uint32 size, bool state);
template TERATHON_API DataStructure<HalfDataType>::~DataStructure();
template TERATHON_API DataStructure<FloatDataType>::DataStructure(uint32 size, bool state);
template TERATHON_API DataStructure<FloatDataType>::~DataStructure();
template TERATHON_API DataStructure<DoubleDataType>::DataStructure(uint32 size, bool state);
template TERATHON_API DataStructure<DoubleDataType>::~DataStructure();
template TERATHON_API DataStructure<StringDataType>::DataStructure(uint32 size, bool state);
template TERATHON_API DataStructure<StringDataType>::~DataStructure();
template TERATHON_API DataStructure<RefDataType>::DataStructure(uint32 size, bool state);
template TERATHON_API DataStructure<RefDataType>::~DataStructure();
template TERATHON_API DataStructure<TypeDataType>::DataStructure(uint32 size, bool state);
template TERATHON_API DataStructure<TypeDataType>::~DataStructure();
template TERATHON_API DataStructure<Base64DataType>::DataStructure(uint32 size, bool state);
template TERATHON_API DataStructure<Base64DataType>::~DataStructure();


RootStructure::RootStructure() : Structure(kStructureRoot)
{
}

RootStructure::~RootStructure()
{
}

bool RootStructure::ValidateSubstructure(const DataDescription *dataDescription, const Structure *structure) const
{
	return (dataDescription->ValidateTopLevelStructure(structure));
}


DataDescription::DataDescription()
{
}

DataDescription::~DataDescription()
{
}

Structure *DataDescription::FindStructure(const StructureRef& reference) const
{
	if (reference.GetGlobalRefFlag())
	{
		const ImmutableArray<String<>>& nameArray = reference.GetNameArray();

		int32 count = nameArray.GetArrayElementCount();
		if (count != 0)
		{
			Structure *structure = structureMap.FindMapElement(nameArray[0]);
			if ((structure) && (count > 1))
			{
				structure = structure->FindStructure(reference, 1);
			}

			return (structure);
		}
	}

	return (nullptr);
}

Structure *DataDescription::CreatePrimitive(const String<>& identifier)
{
	int32		length;
	DataType	value;

	if (Data::ReadDataType(identifier, &length, &value) == kDataOkay)
	{
		switch (value)
		{
			case kDataBool:
				return (new DataStructure<BoolDataType>);
			case kDataInt8:
				return (new DataStructure<Int8DataType>);
			case kDataInt16:
				return (new DataStructure<Int16DataType>);
			case kDataInt32:
				return (new DataStructure<Int32DataType>);
			case kDataInt64:
				return (new DataStructure<Int64DataType>);
			case kDataUInt8:
				return (new DataStructure<UInt8DataType>);
			case kDataUInt16:
				return (new DataStructure<UInt16DataType>);
			case kDataUInt32:
				return (new DataStructure<UInt32DataType>);
			case kDataUInt64:
				return (new DataStructure<UInt64DataType>);
			case kDataHalf:
				return (new DataStructure<HalfDataType>);
			case kDataFloat:
				return (new DataStructure<FloatDataType>);
			case kDataDouble:
				return (new DataStructure<DoubleDataType>);
			case kDataString:
				return (new DataStructure<StringDataType>);
			case kDataRef:
				return (new DataStructure<RefDataType>);
			case kDataType:
				return (new DataStructure<TypeDataType>);
			case kDataBase64:
				return (new DataStructure<Base64DataType>);
		}
	}

	return (nullptr);
}

Structure *DataDescription::CreateStructure(const String<>& identifier) const
{
	return (nullptr);
}

bool DataDescription::ValidateTopLevelStructure(const Structure *structure) const
{
	return (true);
}

DataResult DataDescription::ProcessData(void)
{
	return (rootStructure.ProcessData(this));
}

DataResult DataDescription::ParseProperties(const char *& text, Structure *structure)
{
	for (;;)
	{
		int32		length;
		DataType	type;
		void		*value;

		DataResult result = Data::ReadIdentifier(text, &length);
		if (result != kDataOkay)
		{
			return (result);
		}

		String<>	identifier;

		identifier.SetStringLength(length);
		Data::ReadIdentifier(text, &length, identifier);

		text += length;
		text += Data::GetWhitespaceLength(text);

		if (structure->ValidateProperty(this, identifier, &type, &value))
		{
			if (type != kDataBool)
			{
				if (text[0] != '=')
				{
					return (kDataPropertySyntaxError);
				}

				text++;
				text += Data::GetWhitespaceLength(text);

				switch (type)
				{
					case kDataInt8:
						result = Int8DataType::ParseValue(text, static_cast<Int8DataType::PrimType *>(value));
						break;
					case kDataInt16:
						result = Int16DataType::ParseValue(text, static_cast<Int16DataType::PrimType *>(value));
						break;
					case kDataInt32:
						result = Int32DataType::ParseValue(text, static_cast<Int32DataType::PrimType *>(value));
						break;
					case kDataInt64:
						result = Int64DataType::ParseValue(text, static_cast<Int64DataType::PrimType *>(value));
						break;
					case kDataUInt8:
						result = UInt8DataType::ParseValue(text, static_cast<UInt8DataType::PrimType *>(value));
						break;
					case kDataUInt16:
						result = UInt16DataType::ParseValue(text, static_cast<UInt16DataType::PrimType *>(value));
						break;
					case kDataUInt32:
						result = UInt32DataType::ParseValue(text, static_cast<UInt32DataType::PrimType *>(value));
						break;
					case kDataUInt64:
						result = UInt64DataType::ParseValue(text, static_cast<UInt64DataType::PrimType *>(value));
						break;
					case kDataHalf:
						result = HalfDataType::ParseValue(text, static_cast<HalfDataType::PrimType *>(value));
						break;
					case kDataFloat:
						result = FloatDataType::ParseValue(text, static_cast<FloatDataType::PrimType *>(value));
						break;
					case kDataDouble:
						result = DoubleDataType::ParseValue(text, static_cast<DoubleDataType::PrimType *>(value));
						break;
					case kDataString:
						result = StringDataType::ParseValue(text, static_cast<StringDataType::PrimType *>(value));
						break;
					case kDataRef:
						result = RefDataType::ParseValue(text, static_cast<RefDataType::PrimType *>(value));
						break;
					case kDataType:
						result = TypeDataType::ParseValue(text, static_cast<TypeDataType::PrimType *>(value));
						break;
					case kDataBase64:
						result = Base64DataType::ParseValue(text, static_cast<Base64DataType::PrimType *>(value));
						break;
					default:
						return (kDataPropertyInvalidType);
				}
			}
			else
			{
				if (text[0] == '=')
				{
					text++;
					text += Data::GetWhitespaceLength(text);

					result = BoolDataType::ParseValue(text, static_cast<BoolDataType::PrimType *>(value));
				}
				else
				{
					*static_cast<BoolDataType::PrimType *>(value) = true;
				}
			}
		}
		else
		{
			// Read an arbitrary property value of unknown type and discard it.

			if (text[0] == '=')
			{
				text++;
				text += Data::GetWhitespaceLength(text);

				result = BoolDataType::ParseValue(text, nullptr);
				if (result != kDataOkay)
				{
					result = StringDataType::ParseValue(text, nullptr);
					if (result != kDataOkay)
					{
						result = RefDataType::ParseValue(text, nullptr);
						if (result != kDataOkay)
						{
							result = TypeDataType::ParseValue(text, nullptr);
							if (result != kDataOkay)
							{
								result = UInt64DataType::ParseValue(text, nullptr);
								if (result != kDataOkay)
								{
									result = DoubleDataType::ParseValue(text, nullptr);
									if (result != kDataOkay)
									{
										result = Base64DataType::ParseValue(text, nullptr);
										if (result != kDataOkay)
										{
											result = kDataPropertySyntaxError;
										}
									}
								}
							}
						}
					}
				}
			}
		}

		if (result != kDataOkay)
		{
			return (result);
		}

		if (text[0] == ',')
		{
			text++;
			text += Data::GetWhitespaceLength(text);
			continue;
		}

		break;
	}

	return (kDataOkay);
}

DataResult DataDescription::ParseStructures(const char *& text, Structure *root)
{
	for (;;)
	{
		int32	length;

		DataResult result = Data::ReadIdentifier(text, &length);
		if (result != kDataOkay)
		{
			return (result);
		}

		String<>	identifier;

		identifier.SetStringLength(length);
		Data::ReadIdentifier(text, &length, identifier);

		bool primitiveFlag = false;
		bool unknownFlag = false;

		Structure *structure = CreatePrimitive(identifier);
		if (structure)
		{
			primitiveFlag = true;
		}
		else
		{
			structure = CreateStructure(identifier);
			if (!structure)
			{
				structure = new Structure(kStructureUnknown);
				unknownFlag = true;
			}
		}

		identifier.PurgeString();

		Holder<Structure> structureHolder = structure;
		structure->textLocation = text;
		root->AppendSubnode(structure);

		text += length;
		text += Data::GetWhitespaceLength(text);

		if ((primitiveFlag) && (text[0] == '['))
		{
			uint64		value;

			text++;
			text += Data::GetWhitespaceLength(text);

			if (Data::ParseSign(text))
			{
				return (kDataPrimitiveIllegalArraySize);
			}

			result = Data::ReadIntegerLiteral(text, &length, &value);
			if (result != kDataOkay)
			{
				return (result);
			}

			if ((value == 0) || (value > kDataMaxPrimitiveArraySize))
			{
				return (kDataPrimitiveIllegalArraySize);
			}

			text += length;
			text += Data::GetWhitespaceLength(text);

			if (text[0] != ']')
			{
				return (kDataPrimitiveSyntaxError);
			}

			text++;
			text += Data::GetWhitespaceLength(text);

			PrimitiveStructure *primitiveStructure = static_cast<PrimitiveStructure *>(structure);
			primitiveStructure->arraySize = uint32(value);

			if (text[0] == '*')
			{
				text++;
				text += Data::GetWhitespaceLength(text);

				primitiveStructure->stateFlag = true;
			}
		}

		if ((!unknownFlag) && (!root->ValidateSubstructure(this, structure)))
		{
			return (kDataInvalidStructure);
		}

		char c = text[0];
		if (uint32(c - '$') < 2U)
		{
			text++;

			result = Data::ReadIdentifier(text, &length);
			if (result != kDataOkay)
			{
				return (result);
			}

			Data::ReadIdentifier(text, &length, structure->structureName.SetStringLength(length));

			bool global = (c == '$');
			structure->globalNameFlag = global;

			Map<Structure> *map = (global) ? &structureMap : &root->structureMap;
			if (!map->InsertMapElement(structure))
			{
				return (kDataStructNameExists);
			}

			text += length;
			text += Data::GetWhitespaceLength(text);
		}

		if ((!primitiveFlag) && (text[0] == '('))
		{
			text++;
			text += Data::GetWhitespaceLength(text);

			if (text[0] != ')')
			{
				result = ParseProperties(text, structure);
				if (result != kDataOkay)
				{
					return (result);
				}

				if (text[0] != ')')
				{
					return (kDataPropertySyntaxError);
				}
			}

			text++;
			text += Data::GetWhitespaceLength(text);
		}

		if (text[0] != '{')
		{
			return (kDataSyntaxError);
		}

		text++;
		text += Data::GetWhitespaceLength(text);

		if (text[0] != '}')
		{
			if (primitiveFlag)
			{
				result = static_cast<PrimitiveStructure *>(structure)->ParseData(text);
				if (result != kDataOkay)
				{
					return (result);
				}
			}
			else
			{
				result = ParseStructures(text, structure);
				if (result != kDataOkay)
				{
					return (result);
				}
			}

			if (text[0] != '}')
			{
				return (kDataSyntaxError);
			}
		}

		text++;
		text += Data::GetWhitespaceLength(text);

		if (!unknownFlag)
		{
			// Setting structureHolder to nullptr prevents a valid structure from being auto-deleted.
			// Unknown structures are auto-deleted when structureHolder goes out of scope.

			structureHolder = nullptr;
		}

		c = text[0];
		if ((c == 0) || (c == '}'))
		{
			// Reached either end of file or end of substructures for an enclosing structure.

			break;
		}
	}

	return (kDataOkay);
}

DataResult DataDescription::ProcessText(const char *text)
{
	rootStructure.PurgeSubtree();

	errorStructure = nullptr;
	errorLine = 0;

	const char *start = text;
	text += Data::GetWhitespaceLength(text);

	DataResult result = kDataOkay;
	if (text[0] != 0)
	{
		result = ParseStructures(text, &rootStructure);
		if ((result == kDataOkay) && (text[0] != 0))
		{
			result = kDataSyntaxError;
		}
	}

	if (result == kDataOkay)
	{
		result = ProcessData();
		if ((result != kDataOkay) && (errorStructure))
		{
			text = errorStructure->textLocation;
		}
	}

	if (result != kDataOkay)
	{
		rootStructure.PurgeSubtree();

		int32 line = 1;
		while (text != start)
		{
			if ((--text)[0] == '\n')
			{
				line++;
			}
		}

		errorLine = line;
	}

	return (result);
}
