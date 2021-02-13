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


#include "TSString.h"


using namespace Terathon;


inline uint32 String<0>::GetPhysicalSize(uint32 size)
{
	return ((size + (kStringAllocSize + 3)) & ~(kStringAllocSize - 1));
}

String<0>::String()
{
	logicalSize = 1;
	physicalSize = kStringLocalSize;
	stringPointer = localString;
	localString[0] = 0;
}

String<0>::~String()
{
	if (stringPointer != localString)
	{
		delete[] stringPointer;
	}
}

String<0>::String(const String& s)
{
	int32 size = s.logicalSize;
	logicalSize = size;
	if (size > 1)
	{
		physicalSize = GetPhysicalSize(size);
		stringPointer = new char[physicalSize];
		Text::CopyText(s, stringPointer);
	}
	else
	{
		physicalSize = kStringLocalSize;
		stringPointer = localString;
		localString[0] = 0;
	}
}

String<0>::String(String&& s)
{
	logicalSize = s.logicalSize;
	physicalSize = s.physicalSize;

	if (s.stringPointer != s.localString)
	{
		stringPointer = s.stringPointer;

		s.logicalSize = 1;
		s.physicalSize = kStringLocalSize;
		s.stringPointer = s.localString;
		s.localString[0] = 0;
	}
	else
	{
		stringPointer = localString;
		for (machine a = 0; a < kStringLocalSize; a++)
		{
			localString[a] = s.localString[a];
		}
	}
}

String<0>::String(const char *s)
{
	int32 size = Text::GetTextLength(s) + 1;
	logicalSize = size;

	if (size > kStringLocalSize)
	{
		physicalSize = GetPhysicalSize(size);
		stringPointer = new char[physicalSize];
	}
	else
	{
		physicalSize = kStringLocalSize;
		stringPointer = localString;
	}

	for (machine a = 0; a < size; a++)
	{
		stringPointer[a] = s[a];
	}
}

String<0>::String(const char *s, int32 length)
{
	for (machine a = 0; a < length; a++)
	{
		if (s[a] == 0)
		{
			length = int32(a);
			break;
		}
	}

	int32 size = length + 1;
	logicalSize = size;

	if (size > kStringLocalSize)
	{
		physicalSize = GetPhysicalSize(size);
		stringPointer = new char[physicalSize];
	}
	else
	{
		physicalSize = kStringLocalSize;
		stringPointer = localString;
	}

	stringPointer[length] = 0;
	for (machine a = 0; a < length; a++)
	{
		stringPointer[a] = s[a];
	}
}

String<0>::String(const uint16 *s)
{
	int32 length = Text::GetUnicodeStringLength(s);
	if (length > 0)
	{
		int32 size = length + 1;
		logicalSize = size;

		if (size > kStringLocalSize)
		{
			physicalSize = GetPhysicalSize(size);
			stringPointer = new char[physicalSize];
		}
		else
		{
			physicalSize = kStringLocalSize;
			stringPointer = localString;
		}

		Text::ConvertWideTextToString(s, stringPointer, length);
	}
	else
	{
		logicalSize = 1;
		physicalSize = kStringLocalSize;
		stringPointer = localString;
		localString[0] = 0;
	}
}

String<0>::String(int32 n)
{
	physicalSize = kStringLocalSize;
	stringPointer = localString;
	logicalSize = Text::IntegerToString(n, localString, kStringLocalSize - 1) + 1;
}

String<0>::String(uint32 n)
{
	physicalSize = kStringLocalSize;
	stringPointer = localString;
	logicalSize = Text::IntegerToString(n, localString, kStringLocalSize - 1) + 1;
}

String<0>::String(int64 n)
{
	physicalSize = kStringAllocSize;
	stringPointer = new char[kStringAllocSize];
	logicalSize = Text::Integer64ToString(n, stringPointer, kStringAllocSize - 1) + 1;
}

String<0>::String(float n)
{
	physicalSize = kStringAllocSize;
	stringPointer = new char[kStringAllocSize];
	logicalSize = Text::FloatToString(n, stringPointer, kStringAllocSize - 1) + 1;
}

String<0>::String(const char *s1, const char *s2)
{
	int32 len1 = Text::GetTextLength(s1);
	int32 len2 = Text::GetTextLength(s2);

	int32 size = len1 + len2 + 1;
	logicalSize = size;

	if (size > 1)
	{
		if (size > kStringLocalSize)
		{
			physicalSize = GetPhysicalSize(size);
			stringPointer = new char[physicalSize];
		}
		else
		{
			physicalSize = kStringLocalSize;
			stringPointer = localString;
		}

		Text::CopyText(s1, stringPointer);
		Text::CopyText(s2, stringPointer + len1);
	}
	else
	{
		physicalSize = kStringLocalSize;
		stringPointer = localString;
		localString[0] = 0;
	}
}

String<0>::String(int32 n, const char *s1)
{
	int32 len1 = Text::GetTextLength(s1);

	int32 size = len1 + kStringAllocSize;
	physicalSize = GetPhysicalSize(size);
	stringPointer = new char[physicalSize];
	Text::CopyText(s1, stringPointer);
	logicalSize = len1 + Text::IntegerToString(n, stringPointer + len1, kStringAllocSize - 1) + 1;
}

String<0>::String(uint32 n, const char *s1)
{
	int32 len1 = Text::GetTextLength(s1);

	int32 size = len1 + kStringAllocSize;
	physicalSize = GetPhysicalSize(size);
	stringPointer = new char[physicalSize];
	Text::CopyText(s1, stringPointer);
	logicalSize = len1 + Text::IntegerToString(n, stringPointer + len1, kStringAllocSize - 1) + 1;
}

String<0>::String(int64 n, const char *s1)
{
	int32 len1 = Text::GetTextLength(s1);

	int32 size = len1 + kStringAllocSize;
	physicalSize = GetPhysicalSize(size);
	stringPointer = new char[physicalSize];
	Text::CopyText(s1, stringPointer);
	logicalSize = len1 + Text::Integer64ToString(n, stringPointer + len1, kStringAllocSize - 1) + 1;
}

void String<0>::PurgeString(void)
{
	if (stringPointer != localString)
	{
		delete[] stringPointer;
	}

	logicalSize = 1;
	physicalSize = kStringLocalSize;
	stringPointer = localString;
	localString[0] = 0;
}

void String<0>::Resize(int32 size)
{
	logicalSize = size;
	if (size > kStringLocalSize)
	{
		if ((size > physicalSize) || (size < physicalSize / 2))
		{
			if (stringPointer != localString)
			{
				delete[] stringPointer;
			}

			physicalSize = GetPhysicalSize(size);
			stringPointer = new char[physicalSize];
		}
	}
	else
	{
		if (stringPointer != localString)
		{
			delete[] stringPointer;
		}

		physicalSize = kStringLocalSize;
		stringPointer = localString;
	}
}

String<0>& String<0>::Set(const char *s, int32 length)
{
	for (machine a = 0; a < length; a++)
	{
		if (s[a] == 0)
		{
			length = int32(a);
			break;
		}
	}

	if (length > 0)
	{
		Resize(length + 1);
		Text::CopyText(s, stringPointer, length);
	}
	else
	{
		PurgeString();
	}

	return (*this);
}

String<0>& String<0>::operator =(String&& s)
{
	if (stringPointer != localString)
	{
		delete[] stringPointer;
	}

	logicalSize = s.logicalSize;
	physicalSize = s.physicalSize;

	if (s.stringPointer != s.localString)
	{
		stringPointer = s.stringPointer;

		s.logicalSize = 1;
		s.physicalSize = kStringLocalSize;
		s.stringPointer = s.localString;
		s.localString[0] = 0;
	}
	else
	{
		stringPointer = localString;
		for (machine a = 0; a < kStringLocalSize; a++)
		{
			localString[a] = s.localString[a];
		}
	}

	return (*this);
}

String<0>& String<0>::operator =(const String& s)
{
	int32 size = s.logicalSize;
	if (size > 1)
	{
		Resize(size);
		Text::CopyText(s, stringPointer);
	}
	else
	{
		PurgeString();
	}

	return (*this);
}

String<0>& String<0>::operator =(const char *s)
{
	int32 length = Text::GetTextLength(s);
	if (length > 0)
	{
		Resize(length + 1);
		Text::CopyText(s, stringPointer);
	}
	else
	{
		PurgeString();
	}

	return (*this);
}

String<0>& String<0>::operator =(int32 n)
{
	Resize(kStringAllocSize);
	logicalSize = Text::IntegerToString(n, stringPointer, kStringAllocSize - 1) + 1;
	return (*this);
}

String<0>& String<0>::operator =(uint32 n)
{
	Resize(kStringAllocSize);
	logicalSize = Text::IntegerToString(n, stringPointer, kStringAllocSize - 1) + 1;
	return (*this);
}

String<0>& String<0>::operator =(int64 n)
{
	Resize(kStringAllocSize);
	logicalSize = Text::Integer64ToString(n, stringPointer, kStringAllocSize - 1) + 1;
	return (*this);
}

String<0>& String<0>::operator =(float n)
{
	Resize(kStringAllocSize);
	logicalSize = Text::FloatToString(n, stringPointer, kStringAllocSize - 1) + 1;
	return (*this);
}

String<0>& String<0>::operator +=(const String<>& s)
{
	int32 length = s.GetStringLength();
	if (length > 0)
	{
		int32 size = logicalSize + length;
		if (size > physicalSize)
		{
			physicalSize = Max(GetPhysicalSize(size), physicalSize + physicalSize / 2);
			char *newPointer = new char[physicalSize];
			Text::CopyText(stringPointer, newPointer);

			if (stringPointer != localString)
			{
				delete[] stringPointer;
			}

			stringPointer = newPointer;
		}

		Text::CopyText(s, stringPointer + logicalSize - 1);
		logicalSize = size;
	}

	return (*this);
}

String<0>& String<0>::operator +=(const char *s)
{
	int32 length = Text::GetTextLength(s);
	if (length > 0)
	{
		int32 size = logicalSize + length;
		if (size > physicalSize)
		{
			physicalSize = Max(GetPhysicalSize(size), physicalSize + physicalSize / 2);
			char *newPointer = new char[physicalSize];
			Text::CopyText(stringPointer, newPointer);

			if (stringPointer != localString)
			{
				delete[] stringPointer;
			}

			stringPointer = newPointer;
		}

		Text::CopyText(s, stringPointer + logicalSize - 1);
		logicalSize = size;
	}

	return (*this);
}

String<0>& String<0>::operator +=(char k)
{
	int32 size = logicalSize + 1;
	if (size > physicalSize)
	{
		physicalSize = Max(GetPhysicalSize(size), physicalSize + physicalSize / 2);
		char *newPointer = new char[physicalSize];
		Text::CopyText(stringPointer, newPointer);

		if (stringPointer != localString)
		{
			delete[] stringPointer;
		}

		stringPointer = newPointer;
	}

	stringPointer[logicalSize - 1] = k;
	stringPointer[logicalSize] = 0;
	logicalSize = size;
	return (*this);
}

String<0>& String<0>::operator +=(int32 n)
{
	int32 size = logicalSize + kStringAllocSize;
	if (size > physicalSize)
	{
		physicalSize = Max(GetPhysicalSize(size), physicalSize + physicalSize / 2);
		char *newPointer = new char[physicalSize];
		Text::CopyText(stringPointer, newPointer);

		if (stringPointer != localString)
		{
			delete[] stringPointer;
		}

		stringPointer = newPointer;
	}

	logicalSize += Text::IntegerToString(n, stringPointer + logicalSize - 1, kStringAllocSize - 1);
	return (*this);
}

String<0>& String<0>::operator +=(uint32 n)
{
	int32 size = logicalSize + kStringAllocSize;
	if (size > physicalSize)
	{
		physicalSize = Max(GetPhysicalSize(size), physicalSize + physicalSize / 2);
		char *newPointer = new char[physicalSize];
		Text::CopyText(stringPointer, newPointer);

		if (stringPointer != localString)
		{
			delete[] stringPointer;
		}

		stringPointer = newPointer;
	}

	logicalSize += Text::IntegerToString(n, stringPointer + logicalSize - 1, kStringAllocSize - 1);
	return (*this);
}

String<0>& String<0>::operator +=(int64 n)
{
	int32 size = logicalSize + kStringAllocSize;
	if (size > physicalSize)
	{
		physicalSize = Max(GetPhysicalSize(size), physicalSize + physicalSize / 2);
		char *newPointer = new char[physicalSize];
		Text::CopyText(stringPointer, newPointer);

		if (stringPointer != localString)
		{
			delete[] stringPointer;
		}

		stringPointer = newPointer;
	}

	logicalSize += Text::Integer64ToString(n, stringPointer + logicalSize - 1, kStringAllocSize - 1);
	return (*this);
}

String<0>& String<0>::operator +=(uint64 n)
{
	int32 size = logicalSize + kStringAllocSize;
	if (size > physicalSize)
	{
		physicalSize = Max(GetPhysicalSize(size), physicalSize + physicalSize / 2);
		char *newPointer = new char[physicalSize];
		Text::CopyText(stringPointer, newPointer);

		if (stringPointer != localString)
		{
			delete[] stringPointer;
		}

		stringPointer = newPointer;
	}

	logicalSize += Text::Integer64ToString(n, stringPointer + logicalSize - 1, kStringAllocSize - 1);
	return (*this);
}

String<0>& String<0>::SetStringLength(int32 length)
{
	if (length > 0)
	{
		int32 size = length + 1;
		if (size != logicalSize)
		{
			int32 copyLength = Min(length, logicalSize - 1);
			logicalSize = size;

			if (size > kStringLocalSize)
			{
				if ((size > physicalSize) || (size < physicalSize / 2))
				{
					physicalSize = GetPhysicalSize(size);
					char *newPointer = new char[physicalSize];
					Text::CopyText(stringPointer, newPointer, copyLength);

					if (stringPointer != localString)
					{
						delete[] stringPointer;
					}

					stringPointer = newPointer;
				}
			}
			else if (stringPointer != localString)
			{
				Text::CopyText(stringPointer, localString, copyLength);
				delete[] stringPointer;
				stringPointer = localString;
			}

			stringPointer[length] = 0;
		}
	}
	else
	{
		PurgeString();
	}

	return (*this);
}

String<0>& String<0>::AppendString(const char *s, int32 length)
{
	if (length > 0)
	{
		int32 size = logicalSize + length;
		if (size > physicalSize)
		{
			physicalSize = Max(GetPhysicalSize(size), physicalSize + physicalSize / 2);
			char *newPointer = new char[physicalSize];
			Text::CopyText(stringPointer, newPointer);

			if (stringPointer != localString)
			{
				delete[] stringPointer;
			}

			stringPointer = newPointer;
		}

		Text::CopyText(s, stringPointer + logicalSize - 1, length);
		logicalSize = size;
	}

	return (*this);
}

String<0>& String<0>::ConvertToLowerCase(void)
{
	uint8 *byte = reinterpret_cast<uint8 *>(stringPointer);
	for (;;)
	{
		uint32 c = byte[0];
		if (c == 0)
		{
			break;
		}

		if (c - 'A' < 26U)
		{
			byte[0] = uint8(c + 32);
		}

		byte++;
	}

	return (*this);
}

String<0>& String<0>::ConvertToUpperCase(void)
{
	uint8 *byte = reinterpret_cast<uint8 *>(stringPointer);
	for (;;)
	{
		uint32 c = byte[0];
		if (c == 0)
		{
			break;
		}

		if (c - 'a' < 26U)
		{
			byte[0] = uint8(c - 32);
		}

		byte++;
	}

	return (*this);
}

String<0>& String<0>::ReplaceChar(char x, char y)
{
	for (machine a = 0;; a++)
	{
		char c = stringPointer[a];
		if (c == 0)
		{
			break;
		}

		if (c == x)
		{
			stringPointer[a] = y;
		}
	}

	return (*this);
}

String<0>& String<0>::EncodeEscapeSequences(void)
{
	alignas(64) static const uint8 encodedSize[128] =
	{
		1, 4, 4, 4, 4, 4, 4, 2, 2, 2, 2, 2, 2, 2, 4, 4,
		4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
		1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4
	};

	int32 encodedLength = 0;
	int32 maxCharSize = 1;

	uint8 *byte = reinterpret_cast<uint8 *>(stringPointer);
	for (;;)
	{
		uint32 c = byte[0];
		if (c == 0)
		{
			break;
		}

		if (c < 128U)
		{
			int32 size = encodedSize[c];
			maxCharSize = Max(maxCharSize, size);
			encodedLength += size;
		}
		else
		{
			encodedLength++;
		}

		byte++;
	}

	if (maxCharSize > 1)
	{
		logicalSize = encodedLength;
		physicalSize = GetPhysicalSize(encodedLength);
		char *encodedString = new char[physicalSize];

		byte = reinterpret_cast<uint8 *>(stringPointer);
		uint8 *encodedByte = reinterpret_cast<uint8 *>(encodedString);
		for (;;)
		{
			uint32 c = byte[0];
			if (c == 0)
			{
				break;
			}

			if (c < 128U)
			{
				int32 size = encodedSize[c];
				if (size == 1)
				{
					encodedByte[0] = uint8(c);
				}
				else if (size == 2)
				{
					encodedByte[0] = '\\';

					if (c < 32U)
					{
						static const uint8 encodedChar[7] =
						{
							'a', 'b', 't', 'n', 'v', 'f', 'r'
						};

						encodedByte[1] = encodedChar[c - 7];
					}
					else
					{
						encodedByte[1] = c;
					}
				}
				else
				{
					encodedByte[0] = '\\';
					encodedByte[1] = 'x';
					encodedByte[2] = Text::hexDigit[c >> 4];
					encodedByte[3] = Text::hexDigit[c & 15];
				}

				encodedByte += size;
			}
			else
			{
				encodedByte[0] = uint8(c);
				encodedByte++;
			}

			byte++;
		}

		encodedByte[0] = 0;

		if (stringPointer != localString)
		{
			delete[] stringPointer;
		}

		stringPointer = encodedString;
	}

	return (*this);
}


String<31> Text::Integer64ToHexString16(uint64 num)
{
	String<31>		text;

	text[0] = hexDigit[(num >> 60) & 15];
	text[1] = hexDigit[(num >> 56) & 15];
	text[2] = hexDigit[(num >> 52) & 15];
	text[3] = hexDigit[(num >> 48) & 15];
	text[4] = hexDigit[(num >> 44) & 15];
	text[5] = hexDigit[(num >> 40) & 15];
	text[6] = hexDigit[(num >> 36) & 15];
	text[7] = hexDigit[(num >> 32) & 15];
	text[8] = hexDigit[(num >> 28) & 15];
	text[9] = hexDigit[(num >> 24) & 15];
	text[10] = hexDigit[(num >> 20) & 15];
	text[11] = hexDigit[(num >> 16) & 15];
	text[12] = hexDigit[(num >> 12) & 15];
	text[13] = hexDigit[(num >> 8) & 15];
	text[14] = hexDigit[(num >> 4) & 15];
	text[15] = hexDigit[num & 15];
	text[16] = 0;

	return (text);
}

String<15> Text::IntegerToHexString8(uint32 num)
{
	String<15>		text;

	text[0] = hexDigit[(num >> 28) & 15];
	text[1] = hexDigit[(num >> 24) & 15];
	text[2] = hexDigit[(num >> 20) & 15];
	text[3] = hexDigit[(num >> 16) & 15];
	text[4] = hexDigit[(num >> 12) & 15];
	text[5] = hexDigit[(num >> 8) & 15];
	text[6] = hexDigit[(num >> 4) & 15];
	text[7] = hexDigit[num & 15];
	text[8] = 0;

	return (text);
}

String<7> Text::IntegerToHexString4(uint32 num)
{
	String<7>		text;

	text[0] = hexDigit[(num >> 12) & 15];
	text[1] = hexDigit[(num >> 8) & 15];
	text[2] = hexDigit[(num >> 4) & 15];
	text[3] = hexDigit[num & 15];
	text[4] = 0;

	return (text);
}

String<3> Text::IntegerToHexString2(uint32 num)
{
	String<3>		text;

	text[0] = hexDigit[(num >> 4) & 15];
	text[1] = hexDigit[num & 15];
	text[2] = 0;

	return (text);
}

uint32 Text::StringToType(const char *string)
{
	uint32 type = 0;

	uint32 c = reinterpret_cast<const uint8 *>(string)[0];
	if (c != 0)
	{
		type = c << 24;

		c = reinterpret_cast<const uint8 *>(string)[1];
		if (c != 0)
		{
			type |= c << 16;

			c = reinterpret_cast<const uint8 *>(string)[2];
			if (c != 0)
			{
				type |= c << 8;

				c = reinterpret_cast<const uint8 *>(string)[3];
				if (c != 0)
				{
					type |= c;
				}
			}
		}
	}

	return (type);
}

String<4> Text::TypeToString(uint32 type)
{
	uint32 c = type >> 24;
	if (c != 0)
	{
		return (String<4>(char(c), char(type >> 16), char(type >> 8), char(type)));
	}

	return (String<4>(char(type >> 16), char(type >> 8), char(type), 0));
}

String<31> Text::TypeToHexCharString(uint32 type)
{
	String<31> string("0x");
	string += Text::IntegerToHexString8(type);
	string += " '";
	string += Text::TypeToString(type);
	string += '\'';
	return (string);
}
