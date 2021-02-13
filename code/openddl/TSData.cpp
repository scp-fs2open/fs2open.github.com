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


#include "TSData.h"


using namespace Terathon;


namespace Terathon
{
	namespace Data
	{
		alignas(64) const int8 hexadecimalCharValue[55] =
		{
			 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
			-1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, 10, 11, 12, 13, 14, 15
		};

		alignas(64) const int8 identifierCharState[256] =
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
			0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
			0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 2,
			2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
			2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
			2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
			2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
		};

		alignas(64) const double minusPower10[310] =
		{
			   0.0, 1e-308, 1e-307, 1e-306, 1e-305, 1e-304, 1e-303, 1e-302, 1e-301, 1e-300, 1e-299, 1e-298, 1e-297, 1e-296, 1e-295, 1e-294, 1e-293, 1e-292, 1e-291, 1e-290, 1e-289, 1e-288, 1e-287, 1e-286, 1e-285, 1e-284, 1e-283, 1e-282, 1e-281, 1e-280, 1e-279, 1e-278,
			1e-277, 1e-276, 1e-275, 1e-274, 1e-273, 1e-272, 1e-271, 1e-270, 1e-269, 1e-268, 1e-267, 1e-266, 1e-265, 1e-264, 1e-263, 1e-262, 1e-261, 1e-260, 1e-259, 1e-258, 1e-257, 1e-256, 1e-255, 1e-254, 1e-253, 1e-252, 1e-251, 1e-250, 1e-249, 1e-248, 1e-247, 1e-246,
			1e-245, 1e-244, 1e-243, 1e-242, 1e-241, 1e-240, 1e-239, 1e-238, 1e-237, 1e-236, 1e-235, 1e-234, 1e-233, 1e-232, 1e-231, 1e-230, 1e-229, 1e-228, 1e-227, 1e-226, 1e-225, 1e-224, 1e-223, 1e-222, 1e-221, 1e-220, 1e-219, 1e-218, 1e-217, 1e-216, 1e-215, 1e-214,
			1e-213, 1e-212, 1e-211, 1e-210, 1e-209, 1e-208, 1e-207, 1e-206, 1e-205, 1e-204, 1e-203, 1e-202, 1e-201, 1e-200, 1e-199, 1e-198, 1e-197, 1e-196, 1e-195, 1e-194, 1e-193, 1e-192, 1e-191, 1e-190, 1e-189, 1e-188, 1e-187, 1e-186, 1e-185, 1e-184, 1e-183, 1e-182,
			1e-181, 1e-180, 1e-179, 1e-178, 1e-177, 1e-176, 1e-175, 1e-174, 1e-173, 1e-172, 1e-171, 1e-170, 1e-169, 1e-168, 1e-167, 1e-166, 1e-165, 1e-164, 1e-163, 1e-162, 1e-161, 1e-160, 1e-159, 1e-158, 1e-157, 1e-156, 1e-155, 1e-154, 1e-153, 1e-152, 1e-151, 1e-150,
			1e-149, 1e-148, 1e-147, 1e-146, 1e-145, 1e-144, 1e-143, 1e-142, 1e-141, 1e-140, 1e-139, 1e-138, 1e-137, 1e-136, 1e-135, 1e-134, 1e-133, 1e-132, 1e-131, 1e-130, 1e-129, 1e-128, 1e-127, 1e-126, 1e-125, 1e-124, 1e-123, 1e-122, 1e-121, 1e-120, 1e-119, 1e-118,
			1e-117, 1e-116, 1e-115, 1e-114, 1e-113, 1e-112, 1e-111, 1e-110, 1e-109, 1e-108, 1e-107, 1e-106, 1e-105, 1e-104, 1e-103, 1e-102, 1e-101, 1e-100,  1e-99,  1e-98,  1e-97,  1e-96,  1e-95,  1e-94,  1e-93,  1e-92,  1e-91,  1e-90,  1e-89,  1e-88,  1e-87,  1e-86,
			 1e-85,  1e-84,  1e-83,  1e-82,  1e-81,  1e-80,  1e-79,  1e-78,  1e-77,  1e-76,  1e-75,  1e-74,  1e-73,  1e-72,  1e-71,  1e-70,  1e-69,  1e-68,  1e-67,  1e-66,  1e-65,  1e-64,  1e-63,  1e-62,  1e-61,  1e-60,  1e-59,  1e-58,  1e-57,  1e-56,  1e-55,  1e-54,
			 1e-53,  1e-52,  1e-51,  1e-50,  1e-49,  1e-48,  1e-47,  1e-46,  1e-45,  1e-44,  1e-43,  1e-42,  1e-41,  1e-40,  1e-39,  1e-38,  1e-37,  1e-36,  1e-35,  1e-34,  1e-33,  1e-32,  1e-31,  1e-30,  1e-29,  1e-28,  1e-27,  1e-26,  1e-25,  1e-24,  1e-23,  1e-22,
			 1e-21,  1e-20,  1e-19,  1e-18,  1e-17,  1e-16,  1e-15,  1e-14,  1e-13,  1e-12,  1e-11,  1e-10,   1e-9,   1e-8,   1e-7,   1e-6,   1e-5,   1e-4,   1e-3,   1e-2,   1e-1,    1.0
		};

		alignas(64) const double plusPower10[310] =
		{
			  1.0,   1e1,   1e2,   1e3,   1e4,   1e5,   1e6,   1e7,   1e8,   1e9,  1e10,  1e11,  1e12,  1e13,  1e14,  1e15,  1e16,  1e17,  1e18,  1e19,  1e20,  1e21,  1e22,  1e23,  1e24,  1e25,  1e26,  1e27,  1e28,  1e29,  1e30,  1e31,
			 1e32,  1e33,  1e34,  1e35,  1e36,  1e37,  1e38,  1e39,  1e40,  1e41,  1e42,  1e43,  1e44,  1e45,  1e46,  1e47,  1e48,  1e49,  1e50,  1e51,  1e52,  1e53,  1e54,  1e55,  1e56,  1e57,  1e58,  1e59,  1e60,  1e61,  1e62,  1e63,
			 1e64,  1e65,  1e66,  1e67,  1e68,  1e69,  1e70,  1e71,  1e72,  1e73,  1e74,  1e75,  1e76,  1e77,  1e78,  1e79,  1e80,  1e81,  1e82,  1e83,  1e84,  1e85,  1e86,  1e87,  1e88,  1e89,  1e90,  1e91,  1e92,  1e93,  1e94,  1e95,
			 1e96,  1e97,  1e98,  1e99, 1e100, 1e101, 1e102, 1e103, 1e104, 1e105, 1e106, 1e107, 1e108, 1e109, 1e110, 1e111, 1e112, 1e113, 1e114, 1e115, 1e116, 1e117, 1e118, 1e119, 1e120, 1e121, 1e122, 1e123, 1e124, 1e125, 1e126, 1e127,
			1e128, 1e129, 1e130, 1e131, 1e132, 1e133, 1e134, 1e135, 1e136, 1e137, 1e138, 1e139, 1e140, 1e141, 1e142, 1e143, 1e144, 1e145, 1e146, 1e147, 1e148, 1e149, 1e150, 1e151, 1e152, 1e153, 1e154, 1e155, 1e156, 1e157, 1e158, 1e159,
			1e160, 1e161, 1e162, 1e163, 1e164, 1e165, 1e166, 1e167, 1e168, 1e169, 1e170, 1e171, 1e172, 1e173, 1e174, 1e175, 1e176, 1e177, 1e178, 1e179, 1e180, 1e181, 1e182, 1e183, 1e184, 1e185, 1e186, 1e187, 1e188, 1e189, 1e190, 1e191,
			1e192, 1e193, 1e194, 1e195, 1e196, 1e197, 1e198, 1e199, 1e200, 1e201, 1e202, 1e203, 1e204, 1e205, 1e206, 1e207, 1e208, 1e209, 1e210, 1e211, 1e212, 1e213, 1e214, 1e215, 1e216, 1e217, 1e218, 1e219, 1e220, 1e221, 1e222, 1e223,
			1e224, 1e225, 1e226, 1e227, 1e228, 1e229, 1e230, 1e231, 1e232, 1e233, 1e234, 1e235, 1e236, 1e237, 1e238, 1e239, 1e240, 1e241, 1e242, 1e243, 1e244, 1e245, 1e246, 1e247, 1e248, 1e249, 1e250, 1e251, 1e252, 1e253, 1e254, 1e255,
			1e256, 1e257, 1e258, 1e259, 1e260, 1e261, 1e262, 1e263, 1e264, 1e265, 1e266, 1e267, 1e268, 1e269, 1e270, 1e271, 1e272, 1e273, 1e274, 1e275, 1e276, 1e277, 1e278, 1e279, 1e280, 1e281, 1e282, 1e283, 1e284, 1e285, 1e286, 1e287,
			1e288, 1e289, 1e290, 1e291, 1e292, 1e293, 1e294, 1e295, 1e296, 1e297, 1e298, 1e299, 1e300, 1e301, 1e302, 1e303, 1e304, 1e305, 1e306, 1e307, 1e308, __builtin_huge_val()
		};

		alignas(64) const int8 base64CharValue[256] =
		{
			-1, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
			-2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
			-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
			-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
		};


		int32 ReadEscapeChar(const char *text, uint32 *value);
		int32 ReadStringEscapeChar(const char *text, int32 *stringLength, char *restrict string);
		DataResult ReadCharLiteral(const char *text, int32 *textLength, uint64 *value);
		DataResult ReadDecimalLiteral(const char *text, int32 *textLength, uint64 *value);
		DataResult ReadHexadecimalLiteral(const char *text, int32 *textLength, uint64 *value);
		DataResult ReadOctalLiteral(const char *text, int32 *textLength, uint64 *value);
		DataResult ReadBinaryLiteral(const char *text, int32 *textLength, uint64 *value);
		bool ParseSign(const char *& text);
	}
}


int32 Data::GetWhitespaceLength(const char *text)
{
	const uint8 *byte = reinterpret_cast<const uint8 *>(text);
	for (;;)
	{
		uint32 c = byte[0];
		if (c == 0)
		{
			break;
		}

		if (c >= 33U)
		{
			if (c != '/')
			{
				break;
			}

			c = byte[1];
			if (c == '/')
			{
				byte += 2;
				for (;;)
				{
					c = byte[0];
					if (c == 0)
					{
						goto end;
					}

					byte++;

					if (c == 10)
					{
						break;
					}
				}

				continue;
			}
			else if (c == '*')
			{
				byte += 2;
				for (;;)
				{
					c = byte[0];
					if (c == 0)
					{
						goto end;
					}

					byte++;

					if ((c == '*') && (byte[0] == '/'))
					{
						byte++;
						break;
					}
				}

				continue;
			}

			break;
		}

		byte++;
	}

	end:
	return (int32(reinterpret_cast<const char *>(byte) - text));
}

DataResult Data::ReadDataType(const char *text, int32 *textLength, DataType *value)
{
	const uint8 *byte = reinterpret_cast<const uint8 *>(text);

	uint32 c = byte[0];
	if (c == 'i')
	{
		int32 length = ((byte[1] == 'n') && (byte[2] == 't')) ? 3 : 1;

		if ((byte[length] == '8') && (identifierCharState[byte[length + 1]] == 0))
		{
			*value = kDataInt8;
			*textLength = length + 1;
			return (kDataOkay);
		}

		if ((byte[length] == '1') && (byte[length + 1] == '6') && (identifierCharState[byte[length + 2]] == 0))
		{
			*value = kDataInt16;
			*textLength = length + 2;
			return (kDataOkay);
		}

		if ((byte[length] == '3') && (byte[length + 1] == '2') && (identifierCharState[byte[length + 2]] == 0))
		{
			*value = kDataInt32;
			*textLength = length + 2;
			return (kDataOkay);
		}

		if ((byte[length] == '6') && (byte[length + 1] == '4') && (identifierCharState[byte[length + 2]] == 0))
		{
			*value = kDataInt64;
			*textLength = length + 2;
			return (kDataOkay);
		}
	}
	else if (c == 'u')
	{
		int32 length = (Text::CompareText(&text[1], "nsigned_int", 11)) ? 12 : (Text::CompareText(&text[1], "int", 3)) ? 4 : 1;

		if ((byte[length] == '8') && (identifierCharState[byte[length + 1]] == 0))
		{
			*value = kDataUInt8;
			*textLength = length + 1;
			return (kDataOkay);
		}

		if ((byte[length] == '1') && (byte[length + 1] == '6') && (identifierCharState[byte[length + 2]] == 0))
		{
			*value = kDataUInt16;
			*textLength = length + 2;
			return (kDataOkay);
		}

		if ((byte[length] == '3') && (byte[length + 1] == '2') && (identifierCharState[byte[length + 2]] == 0))
		{
			*value = kDataUInt32;
			*textLength = length + 2;
			return (kDataOkay);
		}

		if ((byte[length] == '6') && (byte[length + 1] == '4') && (identifierCharState[byte[length + 2]] == 0))
		{
			*value = kDataUInt64;
			*textLength = length + 2;
			return (kDataOkay);
		}
	}
	else if (c == 'f')
	{
		int32 length = (Text::CompareText(&text[1], "loat", 4)) ? 5 : 1;

		if (identifierCharState[byte[length]] == 0)
		{
			*value = kDataFloat;
			*textLength = length;
			return (kDataOkay);
		}

		if ((byte[length] == '1') && (byte[length + 1] == '6') && (identifierCharState[byte[length + 2]] == 0))
		{
			*value = kDataHalf;
			*textLength = length + 2;
			return (kDataOkay);
		}

		if ((byte[length] == '3') && (byte[length + 1] == '2') && (identifierCharState[byte[length + 2]] == 0))
		{
			*value = kDataFloat;
			*textLength = length + 2;
			return (kDataOkay);
		}

		if ((byte[length] == '6') && (byte[length + 1] == '4') && (identifierCharState[byte[length + 2]] == 0))
		{
			*value = kDataDouble;
			*textLength = length + 2;
			return (kDataOkay);
		}
	}
	else if (c == 'b')
	{
		int32 length = (Text::CompareText(&text[1], "ool", 3)) ? 4 : 1;

		if (identifierCharState[byte[length]] == 0)
		{
			*value = kDataBool;
			*textLength = length;
			return (kDataOkay);
		}

		if (Text::CompareText(&text[1], "ase64", 5))
		{
			if (identifierCharState[byte[6]] == 0)
			{
				*value = kDataBase64;
				*textLength = 6;
				return (kDataOkay);
			}
		}
	}
	else if (c == 'h')
	{
		int32 length = (Text::CompareText(&text[1], "alf", 3)) ? 4 : 1;

		if (identifierCharState[byte[length]] == 0)
		{
			*value = kDataHalf;
			*textLength = length;
			return (kDataOkay);
		}
	}
	else if (c == 'd')
	{
		int32 length = (Text::CompareText(&text[1], "ouble", 5)) ? 6 : 1;

		if (identifierCharState[byte[length]] == 0)
		{
			*value = kDataDouble;
			*textLength = length;
			return (kDataOkay);
		}
	}
	else if (c == 's')
	{
		int32 length = (Text::CompareText(&text[1], "tring", 5)) ? 6 : 1;

		if (identifierCharState[byte[length]] == 0)
		{
			*value = kDataString;
			*textLength = length;
			return (kDataOkay);
		}
	}
	else if (c == 'r')
	{
		int32 length = ((byte[1] == 'e') && (byte[2] == 'f')) ? 3 : 1;

		if (identifierCharState[byte[length]] == 0)
		{
			*value = kDataRef;
			*textLength = length;
			return (kDataOkay);
		}
	}
	else if (c == 't')
	{
		int32 length = (Text::CompareText(&text[1], "ype", 3)) ? 4 : 1;

		if (identifierCharState[byte[length]] == 0)
		{
			*value = kDataType;
			*textLength = length;
			return (kDataOkay);
		}
	}
	else if (c == 'z')
	{
		if (identifierCharState[byte[1]] == 0)
		{
			*value = kDataBase64;
			*textLength = 1;
			return (kDataOkay);
		}
	}

	return (kDataTypeInvalid);
}

DataResult Data::ReadIdentifier(const char *text, int32 *textLength)
{
	const uint8 *byte = reinterpret_cast<const uint8 *>(text);
	int32 count = 0;

	uint32 c = byte[0];
	int32 state = identifierCharState[c];

	if (state == 1)
	{
		if (c < 'A')
		{
			return (kDataIdentifierIllegalChar);
		}

		count++;
		for (;;)
		{
			c = byte[count];
			state = identifierCharState[c];

			if (state == 1)
			{
				count++;
				continue;
			}
			else if (state == 2)
			{
				return (kDataIdentifierIllegalChar);
			}

			break;
		}

		*textLength = count;
		return (kDataOkay);
	}
	else if (state == 2)
	{
		return (kDataIdentifierIllegalChar);
	}

	return (kDataIdentifierEmpty);
}

DataResult Data::ReadIdentifier(const char *text, int32 *textLength, char *restrict identifier)
{
	const uint8 *byte = reinterpret_cast<const uint8 *>(text);
	int32 count = 0;

	uint32 c = byte[0];
	int32 state = identifierCharState[c];

	if (state == 1)
	{
		if (c < 'A')
		{
			return (kDataIdentifierIllegalChar);
		}

		identifier[count] = char(c);

		count++;
		for (;;)
		{
			c = byte[count];
			state = identifierCharState[c];

			if (state == 1)
			{
				identifier[count] = char(c);

				count++;
				continue;
			}
			else if (state == 2)
			{
				return (kDataIdentifierIllegalChar);
			}

			break;
		}

		identifier[count] = 0;

		*textLength = count;
		return (kDataOkay);
	}
	else if (state == 2)
	{
		return (kDataIdentifierIllegalChar);
	}

	return (kDataIdentifierEmpty);
}

int32 Data::ReadEscapeChar(const char *text, uint32 *value)
{
	const uint8 *byte = reinterpret_cast<const uint8 *>(text);
	uint32 c = byte[0];

	if ((c == '\"') || (c == '\'') || (c == '?') || (c == '\\'))
	{
		*value = c;
		return (1);
	}
	else if (c == 'a')
	{
		*value = '\a';
		return (1);
	}
	else if (c == 'b')
	{
		*value = '\b';
		return (1);
	}
	else if (c == 'f')
	{
		*value = '\f';
		return (1);
	}
	else if (c == 'n')
	{
		*value = '\n';
		return (1);
	}
	else if (c == 'r')
	{
		*value = '\r';
		return (1);
	}
	else if (c == 't')
	{
		*value = '\t';
		return (1);
	}
	else if (c == 'v')
	{
		*value = '\v';
		return (1);
	}
	else if (c == 'x')
	{
		c = byte[1] - '0';
		if (c < 55U)
		{
			int32 x = hexadecimalCharValue[c];
			if (x >= 0)
			{
				c = byte[2] - '0';
				if (c < 55U)
				{
					int32 y = hexadecimalCharValue[c];
					if (y >= 0)
					{
						*value = char((x << 4) | y);
						return (3);
					}
				}
			}
		}
	}

	return (0);
}

int32 Data::ReadStringEscapeChar(const char *text, int32 *stringLength, char *restrict string)
{
	const uint8 *byte = reinterpret_cast<const uint8 *>(text);
	uint32 c = byte[0];

	if (c == 'u')
	{
		uint32 code = 0;

		for (machine a = 1; a <= 4; a++)
		{
			c = byte[a] - '0';
			if (c >= 55U)
			{
				return (0);
			}

			int32 x = hexadecimalCharValue[c];
			if (x < 0)
			{
				return (0);
			}

			code = (code << 4) | x;
		}

		if (code != 0)
		{
			if (string)
			{
				*stringLength = Text::WriteUnicodeChar(string, code);
			}
			else
			{
				*stringLength = 1 + (code >= 0x000080) + (code >= 0x000800);
			}

			return (5);
		}
	}
	if (c == 'U')
	{
		uint32 code = 0;

		for (machine a = 1; a <= 6; a++)
		{
			c = byte[a] - '0';
			if (c >= 55U)
			{
				return (0);
			}

			int32 x = hexadecimalCharValue[c];
			if (x < 0)
			{
				return (0);
			}

			code = (code << 4) | x;
		}

		if ((code != 0) && (code <= 0x10FFFF))
		{
			if (string)
			{
				*stringLength = Text::WriteUnicodeChar(string, code);
			}
			else
			{
				*stringLength = 1 + (code >= 0x000080) + (code >= 0x000800) + (code >= 0x010000);
			}

			return (7);
		}
	}
	else
	{
		uint32		value;

		int32 textLength = ReadEscapeChar(text, &value);
		if (textLength != 0)
		{
			if (string)
			{
				*string = char(value);
			}

			*stringLength = 1;
			return (textLength);
		}
	}

	return (0);
}

DataResult Data::ReadStringLiteral(const char *text, int32 *textLength, int32 *stringLength, char *restrict string)
{
	const uint8 *byte = reinterpret_cast<const uint8 *>(text);
	int32 count = 0;

	for (;;)
	{
		uint32 c = byte[0];
		if ((c == 0) || (c == '\"'))
		{
			break;
		}

		if ((c < 32U) || (c == 127U))
		{
			return (kDataStringIllegalChar);
		}

		if (c != '\\')
		{
			int32 len = Text::ValidateUnicodeChar(reinterpret_cast<const char *>(byte));
			if (len == 0)
			{
				return (kDataStringIllegalChar);
			}

			if (string)
			{
				for (machine a = 0; a < len; a++)
				{
					string[a] = char(byte[a]);
				}

				string += len;
			}

			byte += len;
			count += len;
		}
		else
		{
			int32	stringLen;

			int32 textLen = ReadStringEscapeChar(reinterpret_cast<const char *>(++byte), &stringLen, string);
			if (textLen == 0)
			{
				return (kDataStringIllegalEscape);
			}

			if (string)
			{
				string += stringLen;
			}

			byte += textLen;
			count += stringLen;
		}
	}

	*textLength = int32(reinterpret_cast<const char *>(byte) - text);
	*stringLength = count;
	return (kDataOkay);
}

DataResult Data::ReadBoolLiteral(const char *text, int32 *textLength, bool *value)
{
	const uint8 *byte = reinterpret_cast<const uint8 *>(text);

	uint32 c = byte[0];
	if (c - '0' < 2U)
	{
		if (identifierCharState[byte[1]] == 0)
		{
			*value = (c != '0');
			*textLength = 1;
			return (kDataOkay);
		}
	}
	else if (c == 'f')
	{
		if ((byte[1] == 'a') && (byte[2] == 'l') && (byte[3] == 's') && (byte[4] == 'e') && (identifierCharState[byte[5]] == 0))
		{
			*value = false;
			*textLength = 5;
			return (kDataOkay);
		}
	}
	else if (c == 't')
	{
		if ((byte[1] == 'r') && (byte[2] == 'u') && (byte[3] == 'e') && (identifierCharState[byte[4]] == 0))
		{
			*value = true;
			*textLength = 4;
			return (kDataOkay);
		}
	}

	return (kDataBoolInvalid);
}

DataResult Data::ReadDecimalLiteral(const char *text, int32 *textLength, uint64 *value)
{
	const uint8 *byte = reinterpret_cast<const uint8 *>(text);

	uint64 v = 0;
	bool digitFlag = false;
	for (;; byte++)
	{
		uint32 x = byte[0] - '0';
		if (x < 10U)
		{
			if (v >= 0x199999999999999AULL)
			{
				return (kDataIntegerOverflow);
			}

			uint64 w = v;
			v = v * 10 + x;

			if ((w >= 9U) && (v < 9U))
			{
				return (kDataIntegerOverflow);
			}

			digitFlag = true;
		}
		else
		{
			if ((x != 47) || (!digitFlag))
			{
				break;
			}

			digitFlag = false;
		}
	}

	if (!digitFlag)
	{
		return (kDataSyntaxError);
	}

	*value = v;
	*textLength = int32(reinterpret_cast<const char *>(byte) - text);
	return (kDataOkay);
}

DataResult Data::ReadHexadecimalLiteral(const char *text, int32 *textLength, uint64 *value)
{
	const uint8 *byte = reinterpret_cast<const uint8 *>(text + 2);

	uint64 v = 0;
	bool digitFlag = false;
	for (;; byte++)
	{
		uint32 c = byte[0] - '0';
		if (c >= 55U)
		{
			break;
		}

		int32 x = hexadecimalCharValue[c];
		if (x >= 0)
		{
			if ((v >> 60) != 0)
			{
				return (kDataIntegerOverflow);
			}

			v = (v << 4) | x;
			digitFlag = true;
		}
		else
		{
			if ((c != 47) || (!digitFlag))
			{
				break;
			}

			digitFlag = false;
		}
	}

	if (!digitFlag)
	{
		return (kDataSyntaxError);
	}

	*value = v;
	*textLength = int32(reinterpret_cast<const char *>(byte) - text);
	return (kDataOkay);
}

DataResult Data::ReadOctalLiteral(const char *text, int32 *textLength, uint64 *value)
{
	const uint8 *byte = reinterpret_cast<const uint8 *>(text + 2);

	uint64 v = 0;
	bool digitFlag = false;
	for (;; byte++)
	{
		uint32 x = byte[0] - '0';
		if (x < 8U)
		{
			if (v >= 0x2000000000000000ULL)
			{
				return (kDataIntegerOverflow);
			}

			uint64 w = v;
			v = v * 8 + x;

			if ((w >= 7U) && (v < 7U))
			{
				return (kDataIntegerOverflow);
			}

			digitFlag = true;
		}
		else
		{
			if ((x != 47) || (!digitFlag))
			{
				break;
			}

			digitFlag = false;
		}
	}

	if (!digitFlag)
	{
		return (kDataSyntaxError);
	}

	*value = v;
	*textLength = int32(reinterpret_cast<const char *>(byte) - text);
	return (kDataOkay);
}

DataResult Data::ReadBinaryLiteral(const char *text, int32 *textLength, uint64 *value)
{
	const uint8 *byte = reinterpret_cast<const uint8 *>(text + 2);

	uint64 v = 0;
	bool digitFlag = false;
	for (;; byte++)
	{
		uint32 x = byte[0] - '0';
		if (x < 2U)
		{
			if ((v >> 63) != 0)
			{
				return (kDataIntegerOverflow);
			}

			v = (v << 1) | x;
			digitFlag = true;
		}
		else
		{
			if ((x != 47) || (!digitFlag))
			{
				break;
			}

			digitFlag = false;
		}
	}

	if (!digitFlag)
	{
		return (kDataSyntaxError);
	}

	*value = v;
	*textLength = int32(reinterpret_cast<const char *>(byte) - text);
	return (kDataOkay);
}

DataResult Data::ReadCharLiteral(const char *text, int32 *textLength, uint64 *value)
{
	const uint8 *byte = reinterpret_cast<const uint8 *>(text);

	uint64 v = 0;
	for (;;)
	{
		uint32 c = byte[0];
		if ((c == 0) || (c == '\''))
		{
			break;
		}

		if ((c < 32U) || (c >= 127U))
		{
			return (kDataCharIllegalChar);
		}

		if (c != '\\')
		{
			if ((v >> 56) != 0)
			{
				return (kDataIntegerOverflow);
			}

			v = (v << 8) | c;
			byte++;
		}
		else
		{
			uint32		x;

			int32 length = ReadEscapeChar(reinterpret_cast<const char *>(++byte), &x);
			if (length == 0)
			{
				return (kDataCharIllegalEscape);
			}

			if ((v >> 56) != 0)
			{
				return (kDataIntegerOverflow);
			}

			v = (v << 8) | x;
			byte += length;
		}
	}

	*value = v;
	*textLength = int32(reinterpret_cast<const char *>(byte) - text);
	return (kDataOkay);
}

DataResult Data::ReadIntegerLiteral(const char *text, int32 *textLength, uint64 *value)
{
	const uint8 *byte = reinterpret_cast<const uint8 *>(text);

	uint32 c = byte[0];
	if (c == '0')
	{
		c = byte[1];

		if ((c == 'x') || (c == 'X'))
		{
			return (ReadHexadecimalLiteral(text, textLength, value));
		}

		if ((c == 'o') || (c == 'O'))
		{
			return (ReadOctalLiteral(text, textLength, value));
		}

		if ((c == 'b') || (c == 'B'))
		{
			return (ReadBinaryLiteral(text, textLength, value));
		}
	}
	else if (c == '\'')
	{
		int32	len;

		DataResult result = ReadCharLiteral(reinterpret_cast<const char *>(byte + 1), &len, value);
		if (result == kDataOkay)
		{
			if (byte[len + 1] != '\'')
			{
				return (kDataCharEndOfFile);
			}

			*textLength = len + 2;
		}

		return (result);
	}

	return (ReadDecimalLiteral(text, textLength, value));
}

template <typename type>
DataResult Data::ReadFloatLiteral(const char *text, int32 *textLength, type *value)
{
	const uint8 *byte = reinterpret_cast<const uint8 *>(text);

	uint32 c = byte[0];
	if (c == '0')
	{
		c = byte[1];

		if ((c == 'x') || (c == 'X'))
		{
			uint64		v;

			DataResult result = ReadHexadecimalLiteral(text, textLength, &v);
			if (result == kDataOkay)
			{
				if (v > ((1ULL << (sizeof(type) * 8 - 1)) - 1) * 2 + 1)
				{
					return (kDataFloatOverflow);
				}

				*value = reinterpret_cast<type&>(v);
			}

			return (result);
		}

		if ((c == 'o') || (c == 'O'))
		{
			uint64		v;

			DataResult result = ReadOctalLiteral(text, textLength, &v);
			if (result == kDataOkay)
			{
				if (v > ((1ULL << (sizeof(type) * 8 - 1)) - 1) * 2 + 1)
				{
					return (kDataFloatOverflow);
				}

				*value = reinterpret_cast<type&>(v);
			}

			return (result);
		}

		if ((c == 'b') || (c == 'B'))
		{
			uint64		v;

			DataResult result = ReadBinaryLiteral(text, textLength, &v);
			if (result == kDataOkay)
			{
				if (v > ((1ULL << (sizeof(type) * 8 - 1)) - 1) * 2 + 1)
				{
					return (kDataFloatOverflow);
				}

				*value = reinterpret_cast<type&>(v);
			}

			return (result);
		}
	}

	double v = 0.0F;
	bool digitFlag = false;
	bool wholeFlag = false;
	for (;; byte++)
	{
		uint32 x = byte[0] - '0';
		if (x < 10U)
		{
			v = v * 10.0F + double(x);
			digitFlag = true;
			wholeFlag = true;
		}
		else if (x == 47)
		{
			if (!digitFlag)
			{
				return (kDataFloatInvalid);
			}

			digitFlag = false;
		}
		else
		{
			break;
		}
	}

	if (wholeFlag & !digitFlag)
	{
		return (kDataFloatInvalid);
	}

	bool fractionFlag = false;

	c = byte[0];
	if (c == '.')
	{
		digitFlag = false;
		double decimal = 10.0F;
		for (++byte;; byte++)
		{
			uint32 x = byte[0] - '0';
			if (x < 10U)
			{
				v += double(x) / decimal;
				digitFlag = true;
				fractionFlag = true;
				decimal *= 10.0F;
			}
			else if (x == 47)
			{
				if (!digitFlag)
				{
					return (kDataFloatInvalid);
				}

				digitFlag = false;
			}
			else
			{
				break;
			}
		}

		if (fractionFlag & !digitFlag)
		{
			return (kDataFloatInvalid);
		}

		c = byte[0];
	}

	if (!(wholeFlag | fractionFlag))
	{
		return (kDataFloatInvalid);
	}

	if ((c == 'e') || (c == 'E'))
	{
		bool negative = false;

		c = (++byte)[0];
		if (c == '-')
		{
			negative = true;
			byte++;
		}
		else if (c == '+')
		{
			byte++;
		}

		int32 exponent = 0;
		digitFlag = false;
		for (;; byte++)
		{
			uint32 x = byte[0] - '0';
			if (x < 10U)
			{
				exponent = Min(exponent * 10 + x, 65535);
				digitFlag = true;
			}
			else if (x == 47)
			{
				if (!digitFlag)
				{
					return (kDataFloatInvalid);
				}

				digitFlag = false;
			}
			else
			{
				break;
			}
		}

		if (!digitFlag)
		{
			return (kDataFloatInvalid);
		}

		if (negative)
		{
			v *= minusPower10[MaxZero(309 - exponent)];
		}
		else
		{
			v *= plusPower10[Min(exponent, 309)];
		}
	}

	*value = type(v);
	*textLength = int32(reinterpret_cast<const char *>(byte) - text);
	return (kDataOkay);
}

template DataResult Data::ReadFloatLiteral(const char *text, int32 *textLength, Half *value);
template DataResult Data::ReadFloatLiteral(const char *text, int32 *textLength, float *value);
template DataResult Data::ReadFloatLiteral(const char *text, int32 *textLength, double *value);

bool Data::ParseSign(const char *& text)
{
	char c = text[0];

	if (c == '-')
	{
		text++;
		text += GetWhitespaceLength(text);
		return (true);
	}

	if (c == '+')
	{
		text++;
		text += GetWhitespaceLength(text);
	}

	return (false);
}


StructureRef::StructureRef(bool global)
{
	globalRefFlag = global;
}

StructureRef::~StructureRef()
{
}

void StructureRef::Reset(bool global)
{
	nameArray.PurgeArray();
	globalRefFlag = global;
}


DataResult BoolDataType::ParseValue(const char *& text, PrimType *value)
{
	int32	length;
	bool	discard;

	if (!value)
	{
		value = &discard;
	}

	DataResult result = Data::ReadBoolLiteral(text, &length, value);
	if (result != kDataOkay)
	{
		return (result);
	}

	text += length;
	text += Data::GetWhitespaceLength(text);

	return (kDataOkay);
}


DataResult Int8DataType::ParseValue(const char *& text, PrimType *value)
{
	int32		length;
	uint64		unsignedValue;

	bool negative = Data::ParseSign(text);

	DataResult result = Data::ReadIntegerLiteral(text, &length, &unsignedValue);
	if (result != kDataOkay)
	{
		return (result);
	}

	if (!negative)
	{
		if (unsignedValue > 0x7F)
		{
			return (kDataIntegerOverflow);
		}

		if (value)
		{
			*value = int8(unsignedValue);
		}
	}
	else
	{
		if (unsignedValue > 0x80)
		{
			return (kDataIntegerOverflow);
		}

		if (value)
		{
			*value = int8(-int64(unsignedValue));
		}
	}

	text += length;
	text += Data::GetWhitespaceLength(text);

	return (kDataOkay);
}


DataResult Int16DataType::ParseValue(const char *& text, PrimType *value)
{
	int32		length;
	uint64		unsignedValue;

	bool negative = Data::ParseSign(text);

	DataResult result = Data::ReadIntegerLiteral(text, &length, &unsignedValue);
	if (result != kDataOkay)
	{
		return (result);
	}

	if (!negative)
	{
		if (unsignedValue > 0x7FFF)
		{
			return (kDataIntegerOverflow);
		}

		if (value)
		{
			*value = int16(unsignedValue);
		}
	}
	else
	{
		if (unsignedValue > 0x8000)
		{
			return (kDataIntegerOverflow);
		}

		if (value)
		{
			*value = int16(-int64(unsignedValue));
		}
	}

	text += length;
	text += Data::GetWhitespaceLength(text);

	return (kDataOkay);
}


DataResult Int32DataType::ParseValue(const char *& text, PrimType *value)
{
	int32		length;
	uint64		unsignedValue;

	bool negative = Data::ParseSign(text);

	DataResult result = Data::ReadIntegerLiteral(text, &length, &unsignedValue);
	if (result != kDataOkay)
	{
		return (result);
	}

	if (!negative)
	{
		if (unsignedValue > 0x7FFFFFFF)
		{
			return (kDataIntegerOverflow);
		}

		if (value)
		{
			*value = int32(unsignedValue);
		}
	}
	else
	{
		if (unsignedValue > 0x80000000)
		{
			return (kDataIntegerOverflow);
		}

		if (value)
		{
			*value = int32(-int64(unsignedValue));
		}
	}

	text += length;
	text += Data::GetWhitespaceLength(text);

	return (kDataOkay);
}


DataResult Int64DataType::ParseValue(const char *& text, PrimType *value)
{
	int32		length;
	uint64		unsignedValue;

	bool negative = Data::ParseSign(text);

	DataResult result = Data::ReadIntegerLiteral(text, &length, &unsignedValue);
	if (result != kDataOkay)
	{
		return (result);
	}

	if (!negative)
	{
		if (unsignedValue > 0x7FFFFFFFFFFFFFFF)
		{
			return (kDataIntegerOverflow);
		}

		if (value)
		{
			*value = unsignedValue;
		}
	}
	else
	{
		if (unsignedValue > 0x8000000000000000)
		{
			return (kDataIntegerOverflow);
		}

		if (value)
		{
			*value = -int64(unsignedValue);
		}
	}

	text += length;
	text += Data::GetWhitespaceLength(text);

	return (kDataOkay);
}


DataResult UInt8DataType::ParseValue(const char *& text, PrimType *value)
{
	int32		length;
	uint64		unsignedValue;

	bool negative = Data::ParseSign(text);

	DataResult result = Data::ReadIntegerLiteral(text, &length, &unsignedValue);
	if (result != kDataOkay)
	{
		return (result);
	}

	if (negative)
	{
		unsignedValue = uint64(-int64(unsignedValue));
	}

	if (value)
	{
		*value = uint8(unsignedValue);
	}

	text += length;
	text += Data::GetWhitespaceLength(text);

	return (kDataOkay);
}


DataResult UInt16DataType::ParseValue(const char *& text, PrimType *value)
{
	int32		length;
	uint64		unsignedValue;

	bool negative = Data::ParseSign(text);

	DataResult result = Data::ReadIntegerLiteral(text, &length, &unsignedValue);
	if (result != kDataOkay)
	{
		return (result);
	}

	if (negative)
	{
		unsignedValue = uint64(-int64(unsignedValue));
	}

	if (value)
	{
		*value = uint16(unsignedValue);
	}

	text += length;
	text += Data::GetWhitespaceLength(text);

	return (kDataOkay);
}


DataResult UInt32DataType::ParseValue(const char *& text, PrimType *value)
{
	int32		length;
	uint64		unsignedValue;

	bool negative = Data::ParseSign(text);

	DataResult result = Data::ReadIntegerLiteral(text, &length, &unsignedValue);
	if (result != kDataOkay)
	{
		return (result);
	}

	if (negative)
	{
		unsignedValue = uint64(-int64(unsignedValue));
	}

	if (value)
	{
		*value = uint32(unsignedValue);
	}

	text += length;
	text += Data::GetWhitespaceLength(text);

	return (kDataOkay);
}


DataResult UInt64DataType::ParseValue(const char *& text, PrimType *value)
{
	int32		length;
	uint64		unsignedValue;

	bool negative = Data::ParseSign(text);

	DataResult result = Data::ReadIntegerLiteral(text, &length, &unsignedValue);
	if (result != kDataOkay)
	{
		return (result);
	}

	if (negative)
	{
		unsignedValue = uint64(-int64(unsignedValue));
	}

	if (value)
	{
		*value = unsignedValue;
	}

	text += length;
	text += Data::GetWhitespaceLength(text);

	return (kDataOkay);
}


DataResult HalfDataType::ParseValue(const char *& text, PrimType *value)
{
	int32	length;
	Half	floatValue;

	bool negative = Data::ParseSign(text);

	DataResult result = Data::ReadFloatLiteral(text, &length, &floatValue);
	if (result != kDataOkay)
	{
		return (result);
	}

	if (negative)
	{
		floatValue = -floatValue;
	}

	if (value)
	{
		*value = floatValue;
	}

	text += length;
	text += Data::GetWhitespaceLength(text);

	return (kDataOkay);
}


DataResult FloatDataType::ParseValue(const char *& text, PrimType *value)
{
	int32	length;
	float	floatValue;

	bool negative = Data::ParseSign(text);

	DataResult result = Data::ReadFloatLiteral(text, &length, &floatValue);
	if (result != kDataOkay)
	{
		return (result);
	}

	if (negative)
	{
		floatValue = -floatValue;
	}

	if (value)
	{
		*value = floatValue;
	}

	text += length;
	text += Data::GetWhitespaceLength(text);

	return (kDataOkay);
}


DataResult DoubleDataType::ParseValue(const char *& text, PrimType *value)
{
	int32		length;
	double		floatValue;

	bool negative = Data::ParseSign(text);

	DataResult result = Data::ReadFloatLiteral(text, &length, &floatValue);
	if (result != kDataOkay)
	{
		return (result);
	}

	if (negative)
	{
		floatValue = -floatValue;
	}

	if (value)
	{
		*value = floatValue;
	}

	text += length;
	text += Data::GetWhitespaceLength(text);

	return (kDataOkay);
}


DataResult StringDataType::ParseValue(const char *& text, PrimType *value)
{
	int32	textLength;
	int32	stringLength;

	if (text[0] != '"')
	{
		return (kDataStringInvalid);
	}

	int32 accumLength = 0;
	for (;;)
	{
		text++;

		DataResult result = Data::ReadStringLiteral(text, &textLength, &stringLength);
		if (result != kDataOkay)
		{
			return (result);
		}

		if (value)
		{
			value->SetStringLength(accumLength + stringLength);
			Data::ReadStringLiteral(text, &textLength, &stringLength, &(*value)[accumLength]);
			accumLength += stringLength;
		}

		text += textLength;
		if (text[0] != '"')
		{
			return (kDataStringInvalid);
		}

		text++;
		text += Data::GetWhitespaceLength(text);

		if (text[0] != '"')
		{
			break;
		}
	}

	return (kDataOkay);
}


DataResult RefDataType::ParseValue(const char *& text, PrimType *value)
{
	int32	textLength;

	char c = text[0];

	if (value)
	{
		value->Reset(c != '%');
	}

	if (uint32(c - '$') > 2U)
	{
		const uint8 *byte = reinterpret_cast<const uint8 *>(text);
		if ((byte[0] == 'n') && (byte[1] == 'u') && (byte[2] == 'l') && (byte[3] == 'l') && (Data::identifierCharState[byte[4]] == 0))
		{
			text += 4;
			text += Data::GetWhitespaceLength(text);

			return (kDataOkay);
		}

		return (kDataReferenceInvalid);
	}

	do
	{
		text++;

		DataResult result = Data::ReadIdentifier(text, &textLength);
		if (result != kDataOkay)
		{
			return (result);
		}

		String<>	string;

		string.SetStringLength(textLength);
		Data::ReadIdentifier(text, &textLength, string);

		if (value)
		{
			value->AddName(static_cast<String<>&&>(string));
		}

		text += textLength;
		text += Data::GetWhitespaceLength(text);
	} while (text[0] == '%');

	return (kDataOkay);
}


DataResult TypeDataType::ParseValue(const char *& text, PrimType *value)
{
	int32		length;
	DataType	discard;

	if (!value)
	{
		value = &discard;
	}

	DataResult result = Data::ReadDataType(text, &length, value);
	if (result != kDataOkay)
	{
		return (result);
	}

	text += length;
	text += Data::GetWhitespaceLength(text);

	return (kDataOkay);
}


DataResult Base64DataType::ParseValue(const char *& text, PrimType *value)
{
	int32 textLength = 0;
	int32 codeLength = 0;
	const uint8 *code = reinterpret_cast<const uint8 *>(text);

	for (;; textLength++)
	{
		int32 z = Data::base64CharValue[code[textLength]];
		if (z >= 0)
		{
			codeLength++;
		}
		else if (z == -1)
		{
			break;
		}
	}

	int32 m = codeLength & 3;
	if ((codeLength & 3) == 1)
	{
		return (kDataBase64Invalid);
	}

	if (code[textLength] == '=')
	{
		if (m == 0)
		{
			return (kDataBase64Invalid);
		}

		while (Data::base64CharValue[code[++textLength]] == -2) {}

		if (code[textLength] == '=')
		{
			if (m == 3)
			{
				return (kDataBase64Invalid);
			}

			while (Data::base64CharValue[code[++textLength]] == -2) {}
		}
	}

	text += textLength;

	if (value)
	{
		int32 byteLength = (codeLength * 6) >> 3;
		value->AllocateBuffer(byteLength);
		uint8 *data = value->GetPointer<uint8>();

		textLength = 0;
		codeLength = 0;
		uint32 bits = 0;

		for (;; textLength++)
		{
			int32 z = Data::base64CharValue[code[textLength]];
			if (z >= 0)
			{
				bits = (bits << 6) | z;
				if ((++codeLength & 3) == 0)
				{
					data[0] = (bits >> 16) & 0xFF;
					data[1] = (bits >> 8) & 0xFF;
					data[2] = bits & 0xFF;
					data += 3;
				}
			}
			else if (z == -1)
			{
				break;
			}
		}

		if (m >= 2)
		{
			if (m == 3)
			{
				data[0] = (bits >> 10) & 0xFF;
				data[1] = (bits >> 2) & 0xFF;
			}
			else
			{
				data[0] = (bits >> 4) & 0xFF;
			}
		}
	}

	return (kDataOkay);
}


int32 Data::StringToInt32(const char *text)
{
	int32	value;

	return ((Int32DataType::ParseValue(text, &value) == kDataOkay) ? value : 0);
}

float Data::StringToFloat(const char *text)
{
	float	value;

	return ((FloatDataType::ParseValue(text, &value) == kDataOkay) ? value : 0.0F);
}
