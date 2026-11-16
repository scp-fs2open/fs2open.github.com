
#include "utils/encoding.h"

#include <gtest/gtest.h>

using util::Encoding;

// The length excludes the literal's terminator, so embedded nulls are part of the content
template <size_t N>
static Encoding guess(const char (&content)[N], bool assume_utf8 = false)
{
	return util::guess_encoding(content, N - 1, assume_utf8);
}

template <size_t N>
static size_t bom(const char (&content)[N])
{
	return util::bom_length(content, N - 1);
}

TEST(Encoding, byte_order_marks)
{
	ASSERT_EQ(Encoding::UTF8, guess("\xEF\xBB\xBF[Default]"));
	ASSERT_EQ(Encoding::UTF16LE, guess("\xFF\xFE[\0"));
	ASSERT_EQ(Encoding::UTF16BE, guess("\xFE\xFF\0["));
	ASSERT_EQ(Encoding::UTF32LE, guess("\xFF\xFE\0\0[\0\0\0"));
	ASSERT_EQ(Encoding::UTF32BE, guess("\0\0\xFE\xFF\0\0\0["));

	ASSERT_EQ(3u, bom("\xEF\xBB\xBF[Default]"));
	ASSERT_EQ(2u, bom("\xFF\xFE[\0"));
	ASSERT_EQ(2u, bom("\xFE\xFF\0["));
	ASSERT_EQ(4u, bom("\xFF\xFE\0\0[\0\0\0"));
	ASSERT_EQ(4u, bom("\0\0\xFE\xFF\0\0\0["));
}

TEST(Encoding, no_byte_order_mark)
{
	ASSERT_EQ(Encoding::ASCII, guess("[Default]"));
	ASSERT_EQ(Encoding::UTF8, guess("[Default]", true));
	ASSERT_EQ(0u, bom("[Default]"));

	ASSERT_EQ(Encoding::ASCII, guess(""));
	ASSERT_EQ(Encoding::UTF8, guess("", true));
	ASSERT_EQ(0u, bom(""));
}

TEST(Encoding, leading_null_is_not_utf32)
{
	ASSERT_EQ(Encoding::ASCII, guess("\0abc"));
	ASSERT_EQ(Encoding::ASCII, guess("\0\0\0\0"));
	ASSERT_EQ(0u, bom("\0abc"));
}

TEST(Encoding, utf16le_is_not_utf32le)
{
	// U+0100 in UTF-16LE puts a null in the third byte
	ASSERT_EQ(Encoding::UTF16LE, guess("\xFF\xFE\0\x01"));
	ASSERT_EQ(2u, bom("\xFF\xFE\0\x01"));
}

TEST(Encoding, length_is_respected)
{
	const char content[] = "\xEF\xBB\xBF[Default]";

	ASSERT_EQ(Encoding::ASCII, util::guess_encoding(content, 2, false));
	ASSERT_EQ(0u, util::bom_length(content, 2));
	ASSERT_EQ(Encoding::ASCII, util::guess_encoding(content, 0, false));
}
