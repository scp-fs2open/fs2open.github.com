#include "globalincs/systemvars.h"

#include "network/multi.h"
#include "ship/ship.h"

#include <gtest/gtest.h>

namespace {

const int FIRST_SECONDARY = MAX_SHIP_PRIMARY_BANKS;

using bank_lists_t = SCP_vector<std::pair<int, SCP_vector<int>>>;

// mirrors parse_allowed_weapons: a bank is restricted only when its bank type has two or more lists
void add_lists(ship_info& sip, const bank_lists_t& bank_lists, ubyte flag)
{
	if (sip.restricted_loadout_flag.empty()) {
		sip.restricted_loadout_flag.resize(MAX_SHIP_WEAPONS);
		sip.allowed_bank_restricted_weapons.resize(MAX_SHIP_WEAPONS);
	}

	int primary_lists = 0;
	int secondary_lists = 0;
	for (const auto& bank_list : bank_lists) {
		if (bank_list.first < FIRST_SECONDARY)
			primary_lists++;
		else
			secondary_lists++;
	}

	for (const auto& bank_list : bank_lists) {
		const int bank = bank_list.first;
		const bool restricted = bank < FIRST_SECONDARY ? primary_lists > 1 : secondary_lists > 1;
		if (restricted)
			sip.restricted_loadout_flag[bank] |= flag;

		for (int weapon_class : bank_list.second) {
			sip.allowed_bank_restricted_weapons[bank].set_flag(weapon_class, flag);
			sip.allowed_weapons.set_flag(weapon_class, flag);
		}
	}
}

ship_info make_class(const bank_lists_t& bank_lists, ubyte flag = REGULAR_WEAPON)
{
	ship_info sip;
	add_lists(sip, bank_lists, flag);
	return sip;
}

class ShipClassBankAllowsWeaponTest : public ::testing::Test {
  protected:
	int _saved_game_mode = 0;
	int _saved_type_flags = 0;

	void SetUp() override
	{
		_saved_game_mode = Game_mode;
		_saved_type_flags = Netgame.type_flags;
		Game_mode = GM_NORMAL;
		Netgame.type_flags = 0;
	}

	void TearDown() override
	{
		Game_mode = _saved_game_mode;
		Netgame.type_flags = _saved_type_flags;
	}
};

} // namespace

TEST_F(ShipClassBankAllowsWeaponTest, retail_single_list_leaves_every_bank_open)
{
	const auto sip = make_class({{0, {0, 1}}, {FIRST_SECONDARY, {2}}});

	EXPECT_TRUE(ship_class_bank_allows_weapon(&sip, 0, 0));
	EXPECT_TRUE(ship_class_bank_allows_weapon(&sip, 1, 1));
	EXPECT_TRUE(ship_class_bank_allows_weapon(&sip, FIRST_SECONDARY + 1, 2));
	EXPECT_FALSE(ship_class_bank_allows_weapon(&sip, 1, 5));
}

TEST_F(ShipClassBankAllowsWeaponTest, restricted_bank_uses_its_own_list)
{
	const auto sip = make_class({{0, {0}}, {1, {1}}, {FIRST_SECONDARY, {2}}});

	EXPECT_TRUE(ship_class_bank_allows_weapon(&sip, 0, 0));
	EXPECT_FALSE(ship_class_bank_allows_weapon(&sip, 0, 1));
	EXPECT_TRUE(ship_class_bank_allows_weapon(&sip, 1, 1));
	EXPECT_FALSE(ship_class_bank_allows_weapon(&sip, 1, 0));
	EXPECT_TRUE(ship_class_bank_allows_weapon(&sip, FIRST_SECONDARY, 2));
}

TEST_F(ShipClassBankAllowsWeaponTest, class_without_lists_allows_nothing)
{
	const ship_info sip;

	EXPECT_FALSE(ship_class_bank_allows_weapon(&sip, 0, 0));
	EXPECT_FALSE(ship_class_bank_allows_weapon(&sip, -1, 0));
}

TEST_F(ShipClassBankAllowsWeaponTest, class_list_without_bank_lists_decides_alone)
{
	ship_info sip;
	sip.allowed_weapons.set_flag(1, REGULAR_WEAPON);

	EXPECT_TRUE(ship_class_bank_allows_weapon(&sip, -1, 1));
	EXPECT_TRUE(ship_class_bank_allows_weapon(&sip, 0, 1));
	EXPECT_FALSE(ship_class_bank_allows_weapon(&sip, 0, 0));
}

TEST_F(ShipClassBankAllowsWeaponTest, empty_bank_is_always_allowed)
{
	const auto restricted = make_class({{0, {0}}, {1, {1}}});
	const ship_info no_lists;

	EXPECT_TRUE(ship_class_bank_allows_weapon(&restricted, 0, -1));
	EXPECT_TRUE(ship_class_bank_allows_weapon(&no_lists, 0, -1));
}

TEST_F(ShipClassBankAllowsWeaponTest, no_bank_checks_only_the_class_list)
{
	const auto sip = make_class({{0, {0}}, {1, {1}}});

	EXPECT_TRUE(ship_class_bank_allows_weapon(&sip, -1, 1));
	EXPECT_FALSE(ship_class_bank_allows_weapon(&sip, -1, 5));
}

TEST_F(ShipClassBankAllowsWeaponTest, dogfight_lists_apply_only_in_dogfight)
{
	const auto sip = make_class({{0, {3}}}, DOGFIGHT_WEAPON);

	EXPECT_FALSE(ship_class_bank_allows_weapon(&sip, 0, 3));

	Game_mode = GM_MULTIPLAYER;
	Netgame.type_flags = NG_TYPE_DOGFIGHT;
	EXPECT_TRUE(ship_class_bank_allows_weapon(&sip, 0, 3));

	const auto regular = make_class({{0, {3}}});
	EXPECT_FALSE(ship_class_bank_allows_weapon(&regular, 0, 3));
}

TEST_F(ShipClassBankAllowsWeaponTest, restrictions_follow_the_game_type)
{
	auto regular_restricted = make_class({{0, {0}}, {1, {1}}});
	add_lists(regular_restricted, {{0, {0, 1}}}, DOGFIGHT_WEAPON);

	auto dogfight_restricted = make_class({{0, {0, 1}}});
	add_lists(dogfight_restricted, {{0, {1}}, {1, {0}}}, DOGFIGHT_WEAPON);

	Game_mode = GM_MULTIPLAYER;
	Netgame.type_flags = NG_TYPE_DOGFIGHT;
	EXPECT_TRUE(ship_class_bank_allows_weapon(&regular_restricted, 1, 0));
	EXPECT_FALSE(ship_class_bank_allows_weapon(&dogfight_restricted, 0, 0));
}
