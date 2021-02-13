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


#include "TSText.h"
#include "TSBasic.h"
#include "TSMath.h"


using namespace Terathon;


alignas(16) const char Text::hexDigit[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};


int32 Text::ValidateUnicodeChar(const char *text)
{
	int32 c = reinterpret_cast<const int8 *>(text)[0];
	if (c >= 0)
	{
		return (1);
	}

	uint32 byte1 = c & 0xFF;
	if (byte1 - 0xC2U < 0x33U)
	{
		uint32 byte2 = reinterpret_cast<const uint8 *>(text)[1];
		if ((byte2 & 0xC0U) == 0x80U)
		{
			if (byte1 < 0xE0U)
			{
				return (2);
			}

			uint32 byte3 = reinterpret_cast<const uint8 *>(text)[2];
			if ((byte3 & 0xC0U) == 0x80U)
			{
				if (byte1 < 0xF0U)
				{
					uint32 x = ((byte1 << 12) & 0x00F000) | ((byte2 << 6) & 0x000FC0) | (byte3 & 0x00003F);
					if (x - 0x0800U < 0xF800U)
					{
						return (3);
					}

					return (0);
				}

				uint32 byte4 = reinterpret_cast<const uint8 *>(text)[3];
				if ((byte4 & 0xC0U) == 0x80U)
				{
					uint32 x = ((byte1 << 18) & 0x1C0000) | ((byte2 << 12) & 0x03F000) | ((byte3 << 6) & 0x000FC0) | (byte4 & 0x00003F);
					if (x - 0x00010000U < 0x00100000U)
					{
						return (4);
					}
				}
			}
		}
	}

	return (0);
}

int32 Text::ReadUnicodeChar(const char *text, uint32 *code)
{
	int32 c = reinterpret_cast<const int8 *>(text)[0];
	if (c >= 0)
	{
		*code = c;
		return (1);
	}

	uint32 byte1 = c & 0xFF;
	if (byte1 - 0xC2U < 0x33U)
	{
		uint32 byte2 = reinterpret_cast<const uint8 *>(text)[1];
		if ((byte2 & 0xC0U) == 0x80U)
		{
			if (byte1 < 0xE0U)
			{
				*code = ((byte1 << 6) & 0x0007C0) | (byte2 & 0x00003F);
				return (2);
			}

			uint32 byte3 = reinterpret_cast<const uint8 *>(text)[2];
			if ((byte3 & 0xC0U) == 0x80U)
			{
				if (byte1 < 0xF0U)
				{
					uint32 x = ((byte1 << 12) & 0x00F000) | ((byte2 << 6) & 0x000FC0) | (byte3 & 0x00003F);
					if (x - 0x0800U < 0xF800U)
					{
						*code = x;
						return (3);
					}

					goto error;
				}

				uint32 byte4 = reinterpret_cast<const uint8 *>(text)[3];
				if ((byte4 & 0xC0U) == 0x80U)
				{
					uint32 x = ((byte1 << 18) & 0x1C0000) | ((byte2 << 12) & 0x03F000) | ((byte3 << 6) & 0x000FC0) | (byte4 & 0x00003F);
					if (x - 0x00010000U < 0x00100000U)
					{
						*code = x;
						return (4);
					}
				}
			}
		}
	}

	error:
	*code = 0xFFFD;
	return (1);
}

int32 Text::WriteUnicodeChar(char *text, uint32 code)
{
	if (code <= 0x00007F)
	{
		text[0] = char(code);
		return (1);
	}

	if (code <= 0x0007FF)
	{
		text[0] = char(((code >> 6) & 0x1F) | 0xC0);
		text[1] = char((code & 0x3F) | 0x80);
		return (2);
	}

	if (code <= 0x00FFFF)
	{
		text[0] = char(((code >> 12) & 0x0F) | 0xE0);
		text[1] = char(((code >> 6) & 0x3F) | 0x80);
		text[2] = char((code & 0x3F) | 0x80);
		return (3);
	}

	if (code <= 0x10FFFF)
	{
		text[0] = char(((code >> 18) & 0x07) | 0xF0);
		text[1] = char(((code >> 12) & 0x3F) | 0x80);
		text[2] = char(((code >> 6) & 0x3F) | 0x80);
		text[3] = char((code & 0x3F) | 0x80);
		return (4);
	}

	return (0);
}

int32 Text::GetUnicodeCharByteCount(uint32 code)
{
	if (code <= 0x00007F)
	{
		return (1);
	}

	if (code <= 0x0007FF)
	{
		return (2);
	}

	if (code <= 0x00FFFF)
	{
		return (3);
	}

	if (code <= 0x10FFFF)
	{
		return (4);
	}

	return (0);
}

int32 Text::GetUnicodeCharCount(const char *text)
{
	int32 count = 0;
	for (;; count++)
	{
		uint32 c = *reinterpret_cast<const uint8 *>(text);
		if (c == 0)
		{
			break;
		}

		if ((c < 0xC0U) || (c >= 0xF8U))
		{
			text++;
		}
		else if (c < 0xE0U)
		{
			text += 2;
		}
		else if (c < 0xF0U)
		{
			text += 3;
		}
		else
		{
			text += 4;
		}
	}

	return (count);
}

int32 Text::GetUnicodeCharCount(const char *text, int32 max)
{
	int32 count = 0;
	const char *end = text + max;
	for (; text < end; count++)
	{
		uint32 c = *reinterpret_cast<const uint8 *>(text);
		if (c == 0)
		{
			break;
		}

		if ((c < 0xC0U) || (c >= 0xF8U))
		{
			text++;
		}
		else if (c < 0xE0U)
		{
			text += 2;
		}
		else if (c < 0xF0U)
		{
			text += 3;
		}
		else
		{
			text += 4;
		}
	}

	return (count);
}

int32 Text::GetPreviousUnicodeCharByteCount(const char *text, int32 max)
{
	int32 count = 0;
	while (--max >= 0)
	{
		count--;
		uint32 c = reinterpret_cast<const uint8 *>(text)[count];
		if (c - 0x80U >= 0x40U)
		{
			break;
		}
	}

	return (-count);
}

int32 Text::GetNextUnicodeCharByteCount(const char *text, int32 max)
{
	int32 c = reinterpret_cast<const int8 *>(text)[0];
	if (c < 0)
	{
		uint32 byte = c & 0xFF;
		if (byte >= 0xC0U)
		{
			if (byte < 0xE0U)
			{
				return (Min(max, 2));
			}

			if (byte < 0xF0U)
			{
				return (Min(max, 3));
			}

			if (byte < 0xF8U)
			{
				return (Min(max, 4));
			}
		}
	}

	return (Min(max, 1));
}

int32 Text::GetUnicodeCharStringByteCount(const char *text, int32 charCount)
{
	int32 count = 0;
	for (machine a = 0; a < charCount; a++)
	{
		int32 c = reinterpret_cast<const int8 *>(text)[count];
		if (c == 0)
		{
			break;
		}

		int32 size = 1;
		if (c < 0)
		{
			uint32 byte = c & 0xFF;
			if (byte >= 0xC0U)
			{
				if (byte < 0xE0U)
				{
					size += 1;
				}
				else if (byte < 0xF0U)
				{
					size += 2;
				}
				else if (byte < 0xF8U)
				{
					size += 3;
				}
			}
		}

		count += size;
	}

	return (count);
}

int32 Text::GetUnicodeStringLength(const uint16 *wideText)
{
	int32 length = 0;
	for (;; wideText++)
	{
		uint32 code = wideText[0];
		if (code == 0)
		{
			break;
		}

		uint32 p1 = code - 0xD800U;
		if (p1 < 0x0800U)
		{
			// Character is part of a surrogate pair.

			if (p1 >= 0x0400U)
			{
				// Ignore second half of surrogate pair without first half.

				continue;
			}

			uint32 p2 = wideText[1] - 0xDC00U;
			if (p2 < 0x0400U)
			{
				// There's a valid second half to the surrogate pair.

				code = ((p1 << 10) | p2) + 0x010000;
				wideText++;
			}
			else
			{
				// Ignore first half of a surrogate pair without second half.

				continue;
			}
		}

		length += GetUnicodeCharByteCount(code);
	}

	return (length);
}

void Text::ConvertWideTextToString(const uint16 *wideText, char *string, int32 max)
{
	for (;; wideText++)
	{
		uint32 code = wideText[0];
		if (code == 0)
		{
			break;
		}

		uint32 p1 = code - 0xD800U;
		if (p1 < 0x0800U)
		{
			// Character is part of a surrogate pair.

			if (p1 >= 0x0400U)
			{
				// Ignore second half of surrogate pair without first half.

				continue;
			}

			uint32 p2 = wideText[1] - 0xDC00U;
			if (p2 < 0x0400U)
			{
				// There's a valid second half to the surrogate pair.

				code = ((p1 << 10) | p2) + 0x010000;
				wideText++;
			}
			else
			{
				// Ignore first half of a surrogate pair without second half.

				continue;
			}
		}

		int32 byteCount = GetUnicodeCharByteCount(code);
		if (byteCount > max)
		{
			break;
		}

		string += Text::WriteUnicodeChar(string, code);
		max -= byteCount;
	}

	string[0] = 0;
}

int32 Text::GetWideTextCharCount(const char *string)
{
	int32 count = 0;
	for (;;)
	{
		uint32	code;

		string += Text::ReadUnicodeChar(string, &code);
		if (code == 0)
		{
			break;
		}

		count += 1 + (code >= 0x010000U);
	}

	return (count);
}

void Text::ConvertStringToWideText(const char *string, uint16 *wideText, int32 max)
{
	for (; max > 0;)
	{
		uint32	code;

		string += Text::ReadUnicodeChar(string, &code);
		if (code == 0)
		{
			break;
		}

		if (code < 0x010000U)
		{
			wideText[0] = uint16(code);
			wideText++;
			max--;
		}
		else
		{
			code -= 0x010000U;
			wideText[0] = uint16((code >> 10) + 0xD800U);
			wideText[1] = uint16((code & 0x03FF) + 0xDC00U);
			wideText += 2;
			max -= 2;
		}
	}

	wideText[0] = 0;
}

int32 Text::GetTextLength(const char *text)
{
	const char *start = text;
	while (*text != 0)
	{
		text++;
	}

	return (int32(text - start));
}

uint32 Text::Hash(const char *text)
{
	uint32 hash = 0;
	for (;;)
	{
		uint32 c = *reinterpret_cast<const uint8 *>(text);
		if (c == 0)
		{
			break;
		}

		hash ^= c;
		hash = hash * 0x6B84DF47U + 1;

		text++;
	}

	return (hash);
}

int32 Text::FindChar(const char *text, uint32 k)
{
	const char *start = text;
	for (;;)
	{
		uint32 c = *reinterpret_cast<const uint8 *>(text);
		if (c == 0)
		{
			break;
		}

		if (c == k)
		{
			return (int32(text - start));
		}

		text++;
	}

	return (-1);
}

int32 Text::FindChar(const char *text, uint32 k, int32 max)
{
	const char *start = text;
	while (--max >= 0)
	{
		uint32 c = *reinterpret_cast<const uint8 *>(text);
		if (c == 0)
		{
			break;
		}

		if (c == k)
		{
			return (int32(text - start));
		}

		text++;
	}

	return (-1);
}

int32 Text::FindUnquotedChar(const char *text, uint32 k)
{
	bool quote = false;
	bool backslash = false;

	const char *start = text;
	for (;;)
	{
		uint32 c = *reinterpret_cast<const uint8 *>(text);
		if (c == 0)
		{
			break;
		}

		if (c == 34)
		{
			if (!quote)
			{
				quote = true;
			}
			else if (!backslash)
			{
				quote = false;
			}
		}

		if ((c == k) && (!quote))
		{
			return (int32(text - start));
		}

		backslash = ((c == 92) && (!backslash));
		text++;
	}

	return (-1);
}

int32 Text::CountChars(const char *text, uint32 k, int32 max)
{
	int32 count = 0;
	while (max > 0)
	{
		uint32 c = *reinterpret_cast<const uint8 *>(text);
		if (c == 0)
		{
			break;
		}

		count += (c == k);
		text++;
		max--;
	}

	return (count);
}

int32 Text::CopyText(const char *source, char *dest)
{
	const char *c = source;
	for (;;)
	{
		uint32 k = *reinterpret_cast<const uint8 *>(c);
		*dest++ = char(k);
		if (k == 0)
		{
			break;
		}

		c++;
	}

	return (int32(c - source));
}

int32 Text::CopyText(const char *source, char *dest, int32 max)
{
	const char *c = source;
	while (--max >= 0)
	{
		uint32 k = *reinterpret_cast<const uint8 *>(c);
		if (k == 0)
		{
			break;
		}

		*dest++ = char(k);
		c++;
	}

	dest[0] = 0;
	return (int32(c - source));
}

bool Text::CompareText(const char *s1, const char *s2)
{
	for (machine a = 0;; a++)
	{
		uint32 x = *reinterpret_cast<const uint8 *>(s1 + a);
		uint32 y = *reinterpret_cast<const uint8 *>(s2 + a);

		if (x != y)
		{
			return (false);
		}

		if (x == 0)
		{
			break;
		}
	}

	return (true);
}

bool Text::CompareText(const char *s1, const char *s2, int32 max)
{
	for (machine a = 0;; a++)
	{
		if (--max < 0)
		{
			break;
		}

		uint32 x = *reinterpret_cast<const uint8 *>(s1 + a);
		uint32 y = *reinterpret_cast<const uint8 *>(s2 + a);

		if (x != y)
		{
			return (false);
		}

		if (x == 0)
		{
			break;
		}
	}

	return (true);
}

bool Text::CompareTextCaseless(const char *s1, const char *s2)
{
	for (machine a = 0;; a++)
	{
		uint32 x = *reinterpret_cast<const uint8 *>(s1 + a);
		uint32 y = *reinterpret_cast<const uint8 *>(s2 + a);

		if (x - 'A' < 26U)
		{
			x += 32;
		}

		if (y - 'A' < 26U)
		{
			y += 32;
		}

		if (x != y)
		{
			return (false);
		}

		if (x == 0)
		{
			break;
		}
	}

	return (true);
}

bool Text::CompareTextCaseless(const char *s1, const char *s2, int32 max)
{
	for (machine a = 0;; a++)
	{
		if (--max < 0)
		{
			break;
		}

		uint32 x = *reinterpret_cast<const uint8 *>(s1 + a);
		uint32 y = *reinterpret_cast<const uint8 *>(s2 + a);

		if (x - 65 < 26U)
		{
			x += 32;
		}

		if (y - 65 < 26U)
		{
			y += 32;
		}

		if (x != y)
		{
			return (false);
		}

		if (x == 0)
		{
			break;
		}
	}

	return (true);
}

bool Text::CompareTextLessThan(const char *s1, const char *s2)
{
	for (machine a = 0;; a++)
	{
		uint32 x = *reinterpret_cast<const uint8 *>(s1 + a);
		uint32 y = *reinterpret_cast<const uint8 *>(s2 + a);

		if ((x != y) || (x == 0))
		{
			return (x < y);
		}
	}
}

bool Text::CompareTextLessThan(const char *s1, const char *s2, int32 max)
{
	for (machine a = 0;; a++)
	{
		if (--max < 0)
		{
			break;
		}

		uint32 x = *reinterpret_cast<const uint8 *>(s1 + a);
		uint32 y = *reinterpret_cast<const uint8 *>(s2 + a);

		if ((x != y) || (x == 0))
		{
			return (x < y);
		}
	}

	return (false);
}

bool Text::CompareTextLessThanCaseless(const char *s1, const char *s2)
{
	for (machine a = 0;; a++)
	{
		uint32 x = *reinterpret_cast<const uint8 *>(s1 + a);
		uint32 y = *reinterpret_cast<const uint8 *>(s2 + a);

		if (x - 'a' < 26U)
		{
			x -= 32;
		}

		if (y - 'a' < 26U)
		{
			y -= 32;
		}

		if ((x != y) || (x == 0))
		{
			return (x < y);
		}
	}
}

bool Text::CompareTextLessThanCaseless(const char *s1, const char *s2, int32 max)
{
	for (machine a = 0;; a++)
	{
		if (--max < 0)
		{
			break;
		}

		uint32 x = *reinterpret_cast<const uint8 *>(s1 + a);
		uint32 y = *reinterpret_cast<const uint8 *>(s2 + a);

		if (x - 'a' < 26U)
		{
			x -= 32;
		}

		if (y - 'a' < 26U)
		{
			y -= 32;
		}

		if ((x != y) || (x == 0))
		{
			return (x < y);
		}
	}

	return (false);
}

bool Text::CompareTextLessEqual(const char *s1, const char *s2)
{
	for (machine a = 0;; a++)
	{
		uint32 x = *reinterpret_cast<const uint8 *>(s1 + a);
		uint32 y = *reinterpret_cast<const uint8 *>(s2 + a);

		if ((x != y) || (x == 0))
		{
			return (x <= y);
		}
	}
}

bool Text::CompareTextLessEqual(const char *s1, const char *s2, int32 max)
{
	for (machine a = 0;; a++)
	{
		if (--max < 0)
		{
			break;
		}

		uint32 x = *reinterpret_cast<const uint8 *>(s1 + a);
		uint32 y = *reinterpret_cast<const uint8 *>(s2 + a);

		if ((x != y) || (x == 0))
		{
			return (x <= y);
		}
	}

	return (true);
}

bool Text::CompareTextLessEqualCaseless(const char *s1, const char *s2)
{
	for (machine a = 0;; a++)
	{
		uint32 x = *reinterpret_cast<const uint8 *>(s1 + a);
		uint32 y = *reinterpret_cast<const uint8 *>(s2 + a);

		if (x - 'a' < 26U)
		{
			x -= 32;
		}

		if (y - 'a' < 26U)
		{
			y -= 32;
		}

		if ((x != y) || (x == 0))
		{
			return (x <= y);
		}
	}
}

bool Text::CompareTextLessEqualCaseless(const char *s1, const char *s2, int32 max)
{
	for (machine a = 0;; a++)
	{
		if (--max < 0)
		{
			break;
		}

		uint32 x = *reinterpret_cast<const uint8 *>(s1 + a);
		uint32 y = *reinterpret_cast<const uint8 *>(s2 + a);

		if (x - 'a' < 26U)
		{
			x -= 32;
		}

		if (y - 'a' < 26U)
		{
			y -= 32;
		}

		if ((x != y) || (x == 0))
		{
			return (x <= y);
		}
	}

	return (true);
}

bool Text::CompareNumberedTextLessThan(const char *s1, const char *s2)
{
	for (;;)
	{
		uint32 x = *reinterpret_cast<const uint8 *>(s1++);
		uint32 y = *reinterpret_cast<const uint8 *>(s2++);

		uint32 xnum = x - '0';
		uint32 ynum = y - '0';

		if ((xnum < 10U) && (ynum < 10U))
		{
			int32 xcount = 1;
			int32 ycount = 1;

			for (;;)
			{
				x = *reinterpret_cast<const uint8 *>(s1) - '0';
				if (x >= 10U)
				{
					break;
				}

				xnum = xnum * 10 + x;
				xcount++;
				s1++;
			}

			for (;;)
			{
				y = *reinterpret_cast<const uint8 *>(s2) - '0';
				if (y >= 10U)
				{
					break;
				}

				ynum = ynum * 10 + y;
				ycount++;
				s2++;
			}

			if (xnum != ynum)
			{
				return (xnum < ynum);
			}
			else if (xcount < ycount)
			{
				return (true);
			}
		}
		else
		{
			if ((x != y) || (x == 0))
			{
				return (x < y);
			}
		}
	}
}

bool Text::CompareNumberedTextLessThanCaseless(const char *s1, const char *s2)
{
	for (;;)
	{
		uint32 x = *reinterpret_cast<const uint8 *>(s1++);
		uint32 y = *reinterpret_cast<const uint8 *>(s2++);

		uint32 xnum = x - '0';
		uint32 ynum = y - '0';

		if ((xnum < 10U) && (ynum < 10U))
		{
			int32 xcount = 1;
			int32 ycount = 1;

			for (;;)
			{
				x = *reinterpret_cast<const uint8 *>(s1) - '0';
				if (x >= 10U)
				{
					break;
				}

				xnum = xnum * 10 + x;
				xcount++;
				s1++;
			}

			for (;;)
			{
				y = *reinterpret_cast<const uint8 *>(s2) - '0';
				if (y >= 10U)
				{
					break;
				}

				ynum = ynum * 10 + y;
				ycount++;
				s2++;
			}

			if (xnum != ynum)
			{
				return (xnum < ynum);
			}
			else if (xcount < ycount)
			{
				return (true);
			}
		}
		else
		{
			if (x - 'a' < 26U)
			{
				x -= 32;
			}

			if (y - 'a' < 26U)
			{
				y -= 32;
			}

			if ((x != y) || (x == 0))
			{
				return (x < y);
			}
		}
	}
}

int32 Text::FindText(const char *s1, const char *s2)
{
	const char *start = s1;
	int32 first = *reinterpret_cast<const uint8 *>(s2);

	for (;;)
	{
		int32 c = *reinterpret_cast<const uint8 *>(s1++);
		if (c == 0)
		{
			break;
		}

		if (c == first)
		{
			const uint8 *s3 = reinterpret_cast<const uint8 *>(s1);
			const uint8 *s4 = reinterpret_cast<const uint8 *>(s2);

			for (;;)
			{
				int32 x = *++s4;
				if (x == 0)
				{
					return (int32(s1 - start - 1));
				}

				int32 y = *s3++;
				if (y == 0)
				{
					return (-1);
				}

				if (x != y)
				{
					break;
				}
			}
		}
	}

	return (-1);
}

int32 Text::FindTextCaseless(const char *s1, const char *s2)
{
	const char *start = s1;
	int32 first = *reinterpret_cast<const uint8 *>(s2);

	for (;;)
	{
		int32 c = *reinterpret_cast<const uint8 *>(s1++);
		if (c == 0)
		{
			break;
		}

		if (c == first)
		{
			const uint8 *s3 = reinterpret_cast<const uint8 *>(s1);
			const uint8 *s4 = reinterpret_cast<const uint8 *>(s2);

			for (;;)
			{
				uint32 x = *++s4;
				if (x == 0)
				{
					return (int32(s1 - start - 1));
				}

				uint32 y = *s3++;
				if (y == 0)
				{
					return (-1);
				}

				if (x - 'a' < 26U)
				{
					x -= 32;
				}

				if (y - 'a' < 26U)
				{
					y -= 32;
				}

				if (x != y)
				{
					break;
				}
			}
		}
	}

	return (-1);
}

int32 Text::IntegerToString(int32 num, char *text, int32 max)
{
	char	c[16];

	bool negative = (num < 0);
	num = Abs(num) & 0x7FFFFFFF;

	machine length = 0;
	do
	{
		int32 p = num % 10;
		c[length++] = char(p + 48);
		num /= 10;
	} while (num != 0);

	machine a = -1;
	if (negative)
	{
		if (++a < max)
		{
			text[a] = '-';
		}
		else
		{
			text[a] = 0;
			return (int32(a));
		}
	}

	do
	{
		if (++a < max)
		{
			text[a] = c[--length];
		}
		else
		{
			text[a] = 0;
			return (int32(a));
		}
	} while (length != 0);

	text[++a] = 0;
	return (int32(a));
}

int32 Text::Integer64ToString(int64 num, char *text, int32 max)
{
	char	c[32];

	bool negative = (num < 0);
	num = Abs64(num) & 0x7FFFFFFFFFFFFFFFULL;

	machine length = 0;
	do
	{
		int32 p = num % 10;
		c[length++] = char(p + 48);
		num /= 10;
	} while (num != 0);

	machine a = -1;
	if (negative)
	{
		if (++a < max)
		{
			text[a] = '-';
		}
		else
		{
			text[a] = 0;
			return (int32(a));
		}
	}

	do
	{
		if (++a < max)
		{
			text[a] = c[--length];
		}
		else
		{
			text[a] = 0;
			return (int32(a));
		}
	} while (length != 0);

	text[++a] = 0;
	return (int32(a));
}

int32 Text::FloatToString(float num, char *text, int32 max)
{
	if (max < 1)
	{
		text[0] = 0;
		return (0);
	}

	int32 binary = *reinterpret_cast<int32 *>(&num);
	int32 exponent = (binary >> 23) & 0xFF;

	if (exponent == 0)
	{
		if (max >= 3)
		{
			text[0] = '0';
			text[1] = '.';
			text[2] = '0';
			text[3] = 0;
			return (3);
		}

		text[0] = '0';
		text[1] = 0;
		return (1);
	}

	int32 mantissa = binary & 0x007FFFFF;

	if (exponent == 0xFF)
	{
		if (max >= 4)
		{
			bool b = (binary < 0);
			if (b)
			{
				*text++ = '-';
			}

			if (mantissa == 0)
			{
				text[0] = 'I';
				text[1] = 'N';
				text[2] = 'F';
				text[3] = 0;
			}
			else
			{
				text[0] = 'N';
				text[1] = 'A';
				text[2] = 'N';
				text[3] = 0;
			}

			return (3 + b);
		}

		text[0] = 0;
		return (0);
	}

	int32 power = 0;
	float absolute = Fabs(num);
	if ((absolute < 1.0e-4F) || (!(absolute < 1.0e5F)))
	{
		float f = Floor(Log(absolute) * Math::one_over_ln_10);
		absolute /= Exp(f * Math::ln_10);
		power = int32(f);

		binary = *reinterpret_cast<int32 *>(&absolute);
		exponent = (binary >> 23) & 0xFF;
		mantissa = binary & 0x007FFFFF;
	}

	exponent -= 0x7F;
	mantissa |= 0x00800000;

	int32 len = 0;
	if (num < 0.0F)
	{
		text[0] = '-';
		len = 1;
	}

	if (exponent >= 0)
	{
		int32 whole = mantissa >> (23 - exponent);
		mantissa = (mantissa << exponent) & 0x007FFFFF;

		len += IntegerToString(whole, &text[len], max - len);
		if (len < max)
		{
			text[len++] = '.';
		}

		if (len == max)
		{
			goto end;
		}
	}
	else
	{
		if (len + 2 <= max)
		{
			text[len++] = '0';
			text[len++] = '.';
			if (len == max)
			{
				goto end;
			}
		}
		else
		{
			if (len < max)
			{
				text[len++] = '0';
			}

			goto end;
		}

		mantissa >>= -exponent;
	}

	for (machine a = 0, zeroCount = 0, nineCount = 0; (a < 7) && (len < max); a++)
	{
		mantissa *= 10;
		int32 n = (mantissa >> 23) + 48;
		text[len++] = char(n);

		if (n == '0')
		{
			if ((++zeroCount >= 4) && (a >= 4))
			{
				break;
			}
		}
		else if (n == '9')
		{
			if ((++nineCount >= 4) && (a >= 4))
			{
				break;
			}
		}

		mantissa &= 0x007FFFFF;
		if (mantissa < 2)
		{
			break;
		}
	}

	if ((text[len - 1] == '9') && (text[len - 2] == '9'))
	{
		for (machine a = len - 3;; a--)
		{
			char c = text[a];
			if (c != '9')
			{
				if (c != '.')
				{
					text[a] = c + 1;
					len = int32(a + 1);
				}

				break;
			}
		}
	}
	else
	{
		while (text[len - 1] == '0')
		{
			len--;
		}

		if (text[len - 1] == '.')
		{
			text[len++] = '0';
		}
	}

	if ((power != 0) && (len < max))
	{
		text[len++] = 'e';
		return (IntegerToString(power, &text[len], max - len));
	}

	end:
	text[len] = 0;
	return (len);
}

int32 Text::GetResourceNameLength(const char *text)
{
	int32 len = GetTextLength(text);
	for (machine a = len - 1; a >= 0; a--)
	{
		uint32 c = *reinterpret_cast<const uint8 *>(&text[a]);
		if (c == '.')
		{
			return (int32(a));
		}

		if (c == '/')
		{
			break;
		}
	}

	return (len);
}

int32 Text::GetDirectoryPathLength(const char *text)
{
	int32 len = 0;
	for (;;)
	{
		int32 x = FindChar(&text[len], '/');
		if (x == -1)
		{
			break;
		}

		len += x + 1;
	}

	return (len);
}

int32 Text::ReadInteger(const char *text, char *number, int32 max)
{
	const char *start = text;

	uint32 c = *reinterpret_cast<const uint8 *>(text);
	if (c == '-')
	{
		if (--max >= 0)
		{
			*number++ = char(c);
			text++;
		}
	}

	while (--max >= 0)
	{
		c = *reinterpret_cast<const uint8 *>(text);
		if (c - '0' >= 10U)
		{
			break;
		}

		*number++ = char(c);
		text++;
	}

	*number = 0;
	return (int32(text - start));
}

int32 Text::ReadFloat(const char *text, char *number, int32 max)
{
	const char *start = text;

	uint32 c = *reinterpret_cast<const uint8 *>(text);
	if (c == '-')
	{
		if (--max >= 0)
		{
			*number++ = char(c);
			text++;
		}
	}

	bool decimal = false;
	bool exponent = false;
	bool expneg = true;

	while (--max >= 0)
	{
		c = *reinterpret_cast<const uint8 *>(text);
		if (c == '.')
		{
			if (decimal)
			{
				break;
			}

			decimal = true;
		}
		else if ((c == 'e') || (c == 'E'))
		{
			if (exponent)
			{
				break;
			}

			exponent = true;
			expneg = false;
		}
		else
		{
			if ((c == '-') && (expneg))
			{
				break;
			}
			else if (c - '0' >= 10U)
			{
				break;
			}

			expneg = true;
		}

		*number++ = char(c);
		text++;
	}

	*number = 0;
	return (int32(text - start));
}

int32 Text::ReadString(const char *text, char *string, int32 max)
{
	const char *start = text;

	if (*text == 34)
	{
		text++;
		bool backslash = false;

		while (--max >= 0)
		{
			uint32 c = *reinterpret_cast<const uint8 *>(text);
			if (c == 0)
			{
				break;
			}

			text++;

			if ((c != 92) || (backslash))
			{
				if ((c == 34) && (!backslash))
				{
					break;
				}

				*string++ = char(c);
				backslash = false;
			}
			else
			{
				backslash = true;
			}
		}
	}
	else
	{
		while (--max >= 0)
		{
			uint32 c = *reinterpret_cast<const uint8 *>(text);
			if ((c == 0) || (c < 33) || ((c == '/') && (text[1] == '/')))
			{
				break;
			}

			*string++ = char(c);
			text++;
		}
	}

	*string = 0;
	return (int32(text - start));
}

int32 Text::ReadType(const char *text, uint32 *type)
{
	if (*text == '\'')
	{
		const char *start = text;
		uint32 value = 0;

		text++;
		bool backslash = false;

		for (;;)
		{
			uint32 c = *reinterpret_cast<const uint8 *>(text);
			if (c == 0)
			{
				break;
			}

			text++;

			if ((c != 92) || (backslash))
			{
				if ((c == '\'') && (!backslash))
				{
					break;
				}

				value = (value << 8) | c;
				backslash = false;
			}
			else
			{
				backslash = true;
			}
		}

		*type = value;
		return (int32(text - start));
	}

	*type = 0;
	return (0);
}
