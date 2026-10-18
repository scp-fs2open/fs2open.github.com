
#include "globalincs/utility.h"

#include <gtest/gtest.h>

namespace
{

struct cstr_item
{
	const char* name;
};

struct array_item
{
	char name[NAME_LENGTH];

	array_item(const char* _name)
	{
		strcpy_s(name, _name);
	}
};

struct string_item
{
	SCP_string name;
};

const SCP_vector<cstr_item> Cstr_items = { { "Alpha" }, { nullptr }, { "Beta" }, { "alpha" } };
const SCP_vector<array_item> Array_items = { { "Alpha" }, { "Gamma" }, { "Beta" }, { "alpha" } };
const SCP_vector<string_item> String_items = { { "Alpha" }, { "Gamma" }, { "Beta" }, { "alpha" } };

}

TEST(FindItemWithString, cstr_field)
{
	EXPECT_EQ(0, find_item_with_string(Cstr_items, &cstr_item::name, "ALPHA"));
	EXPECT_EQ(2, find_item_with_string(Cstr_items, &cstr_item::name, SCP_string("beta")));
	EXPECT_EQ(3, find_item_with_string(Cstr_items, &cstr_item::name, "alpha", 1));
	EXPECT_EQ(-1, find_item_with_string(Cstr_items, &cstr_item::name, "alpha", 1, 2));
	EXPECT_EQ(-1, find_item_with_string(Cstr_items, &cstr_item::name, "Delta"));

	const char* null_str = nullptr;
	EXPECT_EQ(1, find_item_with_string(Cstr_items, &cstr_item::name, null_str));
}

TEST(FindItemWithString, array_field)
{
	EXPECT_EQ(0, find_item_with_string(Array_items, &array_item::name, "ALPHA"));
	EXPECT_EQ(2, find_item_with_string(Array_items, &array_item::name, SCP_string("beta")));
	EXPECT_EQ(3, find_item_with_string(Array_items, &array_item::name, "alpha", 1));
	EXPECT_EQ(-1, find_item_with_string(Array_items, &array_item::name, "alpha", 1, 2));
	EXPECT_EQ(-1, find_item_with_string(Array_items, &array_item::name, "Delta"));

	char search[NAME_LENGTH] = "gamma";
	EXPECT_EQ(1, find_item_with_string(Array_items, &array_item::name, search));

	const char* null_str = nullptr;
	EXPECT_EQ(-1, find_item_with_string(Array_items, &array_item::name, null_str));
}

TEST(FindItemWithString, string_field)
{
	EXPECT_EQ(0, find_item_with_string(String_items, &string_item::name, "ALPHA"));
	EXPECT_EQ(2, find_item_with_string(String_items, &string_item::name, SCP_string("beta")));
	EXPECT_EQ(3, find_item_with_string(String_items, &string_item::name, "alpha", 1));
	EXPECT_EQ(-1, find_item_with_string(String_items, &string_item::name, "alpha", 1, 2));
	EXPECT_EQ(-1, find_item_with_string(String_items, &string_item::name, "Delta"));

	const char* null_str = nullptr;
	EXPECT_EQ(-1, find_item_with_string(String_items, &string_item::name, null_str));
}

TEST(FindItemWithString, raw_array)
{
	EXPECT_EQ(3, find_item_with_string(Cstr_items.data(), Cstr_items.size(), &cstr_item::name, "alpha", 1));
	EXPECT_EQ(3, find_item_with_string(Array_items.data(), Array_items.size(), &array_item::name, SCP_string("alpha"), 1));
	EXPECT_EQ(1, find_item_with_string(String_items.data(), String_items.size(), &string_item::name, "GAMMA"));
	EXPECT_EQ(-1, find_item_with_string(String_items.data(), 1, &string_item::name, "Gamma"));

	const array_item* null_array = nullptr;
	EXPECT_EQ(-1, find_item_with_string(null_array, 4, &array_item::name, "Alpha"));
}

TEST(CountItemsWithString, all_field_types)
{
	EXPECT_EQ(2u, count_items_with_string(Cstr_items, &cstr_item::name, "ALPHA"));
	EXPECT_EQ(2u, count_items_with_string(Array_items, &array_item::name, SCP_string("alpha")));
	EXPECT_EQ(2u, count_items_with_string(String_items, &string_item::name, "Alpha"));
	EXPECT_EQ(1u, count_items_with_string(String_items, &string_item::name, SCP_string("alpha"), 1));
	EXPECT_EQ(0u, count_items_with_string(Array_items, &array_item::name, "alpha", 1, 2));

	const char* null_str = nullptr;
	EXPECT_EQ(1u, count_items_with_string(Cstr_items, &cstr_item::name, null_str));
	EXPECT_EQ(0u, count_items_with_string(String_items, &string_item::name, null_str));
}

TEST(CountItemsWithString, raw_array)
{
	EXPECT_EQ(2u, count_items_with_string(Cstr_items.data(), Cstr_items.size(), &cstr_item::name, "alpha"));
	EXPECT_EQ(1u, count_items_with_string(Array_items.data(), Array_items.size(), &array_item::name, "alpha", 1));
	EXPECT_EQ(2u, count_items_with_string(String_items.data(), String_items.size(), &string_item::name, SCP_string("ALPHA")));

	const string_item* null_array = nullptr;
	EXPECT_EQ(0u, count_items_with_string(null_array, 4, &string_item::name, "Alpha"));
}
