
#include <string.h>

#include "globalincs/safe_strings.h"

#include "osapi/dialogs.h"

#include <gtest/gtest.h>

TEST(SafeStrcpy, larger_buffer) {
	{
		char strSource[15];
		char strDest[15];

		memset(strSource, 0, sizeof(strSource));
		memset(strDest, 0, sizeof(strDest));

		ASSERT_EQ(0, strcpy_s(strDest, "Hello World"));
		ASSERT_STREQ("Hello World", strDest);
	}
	{
		char strSource[15];
		char strDest[15];

		memset(strSource, 0, sizeof(strSource));
		memset(strDest, 0, sizeof(strDest));

		ASSERT_EQ(0, strcpy_s(strDest, 15, "Hello World"));
		ASSERT_STREQ("Hello World", strDest);
	}
}

TEST(SafeStrcpy, too_small_buffer) {
	{
		char strSource[15];
		char strDest[15];

		memset(strSource, 0, sizeof(strSource));
		memset(strDest, 0, sizeof(strDest));

		ASSERT_THROW(strcpy_s(strDest, "Hello World, this is a test"), os::dialogs::ErrorException);
		ASSERT_EQ(strDest[0], 0);
	}
	{
		char strSource[15];
		char strDest[15];

		memset(strSource, 0, sizeof(strSource));
		memset(strDest, 0, sizeof(strDest));

		ASSERT_THROW(strcpy_s(strDest, 15, "Hello World, this is a test"), os::dialogs::ErrorException);
		ASSERT_EQ(strDest[0], 0);
	}
}

TEST(SafeStrcpy, strsource_null) {
	{
		char strDest[15];

		memset(strDest, 0, sizeof(strDest));

		ASSERT_THROW(strcpy_s(strDest, nullptr), os::dialogs::ErrorException);
	}
	{
		char strDest[15];

		memset(strDest, 0, sizeof(strDest));

		ASSERT_THROW(strcpy_s(strDest, 15, nullptr), os::dialogs::ErrorException);
	}
}

TEST(SafeStrcpy, strdest_null) {
	ASSERT_THROW(strcpy_s(nullptr, 15, "Hello World"), os::dialogs::ErrorException);
}

TEST(SafeStrcpy, both_null) {
	ASSERT_THROW(strcpy_s(nullptr, 15, nullptr), os::dialogs::ErrorException);
}

TEST(SafeStrcpy, exact_same_size) {
	char strDest[15];
	memset(strDest, 0, sizeof(strDest));

	ASSERT_THROW(strcpy_s(strDest, "Hello World thi"), os::dialogs::ErrorException);
}


TEST(SafeStrcap, larger_buffer) {
	{
		char strSource[15];
		char strDest[15];

		memset(strSource, 0, sizeof(strSource));
		memset(strDest, 0, sizeof(strDest));

		ASSERT_EQ(0, strcpy_s(strDest, "Hello "));
		ASSERT_EQ(0, strcat_s(strDest, "World"));
		ASSERT_STREQ("Hello World", strDest);
	}
	{
		char strSource[15];
		char strDest[15];

		memset(strSource, 0, sizeof(strSource));
		memset(strDest, 0, sizeof(strDest));

		ASSERT_EQ(0, strcpy_s(strDest, 15, "Hello "));
		ASSERT_EQ(0, strcat_s(strDest, 15, "World"));
		ASSERT_STREQ("Hello World", strDest);
	}
}

TEST(SafeStrcap, source_null) {
	{
		char strDest[15];

		memset(strDest, 0, sizeof(strDest));

		ASSERT_EQ(0, strcpy_s(strDest, "World"));
		ASSERT_THROW(strcat_s(strDest, nullptr), os::dialogs::ErrorException);
		ASSERT_EQ(strDest[0], 0);
	}
	{
		char strDest[15];

		memset(strDest, 0, sizeof(strDest));

		ASSERT_EQ(0, strcpy_s(strDest, 15, "World"));
		ASSERT_THROW(strcat_s(strDest, 15, nullptr), os::dialogs::ErrorException);
		ASSERT_EQ(strDest[0], 0);
	}
}

TEST(SafeStrcap, dest_null) {
	char strSource[15];

	memset(strSource, 0, sizeof(strSource));

	ASSERT_EQ(0, strcpy_s(strSource, "Hello "));
	ASSERT_THROW(strcat_s(nullptr, 15, strSource), os::dialogs::ErrorException);
}

TEST(SafeStrcap, size_zero) {
	char strDest[15];

	memset(strDest, 0, sizeof(strDest));

	ASSERT_EQ(0, strcpy_s(strDest, "Hello "));
	ASSERT_THROW(strcat_s(strDest, 0, "World"), os::dialogs::ErrorException);
}

TEST(SafeStrcap, exact_same_size) {
	{
		char strDest[15];

		memset(strDest, 0, sizeof(strDest));

		ASSERT_EQ(0, strcpy_s(strDest, "Hello "));
		ASSERT_THROW(strcat_s(strDest, "World thi"), os::dialogs::ErrorException);
	}
	{
		char strDest[15];

		memset(strDest, 0, sizeof(strDest));

		ASSERT_EQ(0, strcpy_s(strDest, 15, "Hello "));
		ASSERT_THROW(strcat_s(strDest, 15, "World thi"), os::dialogs::ErrorException);
	}
}
