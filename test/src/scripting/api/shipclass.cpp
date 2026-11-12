#include "missionui/missionscreencommon.h"
#include "ship/ship.h"
#include "weapon/weapon.h"

#include "scripting/ScriptingTestFixture.h"

extern bool Ships_inited;

static ship_info make_ship_class(int num_primary_banks, int num_secondary_banks)
{
	ship_info sip;
	sip.num_primary_banks = num_primary_banks;
	sip.num_secondary_banks = num_secondary_banks;
	return sip;
}

static void allow_weapon(ship_info& sip, int weapon_class)
{
	sip.allowed_weapons.set_flag(weapon_class, REGULAR_WEAPON);
}

static void restrict_bank(ship_info& sip, int bank, int weapon_class)
{
	if (sip.restricted_loadout_flag.empty()) {
		sip.restricted_loadout_flag.resize(MAX_SHIP_WEAPONS);
		sip.allowed_bank_restricted_weapons.resize(MAX_SHIP_WEAPONS);
	}
	sip.restricted_loadout_flag[bank] = REGULAR_WEAPON;
	sip.allowed_bank_restricted_weapons[bank].set_flag(weapon_class, REGULAR_WEAPON);
}

class ScriptingShipclassTest : public test::scripting::ScriptingTestFixture {
  public:
	ScriptingShipclassTest() : test::scripting::ScriptingTestFixture(INIT_CFILE)
	{
		pushModDir("shipclass");
	}

  protected:
	wss_unit* _saved_wss_slots = nullptr;

	void SetUp() override
	{
		test::scripting::ScriptingTestFixture::SetUp();

		Ship_info.clear();
		Weapon_info.clear();

		Weapon_info.resize(4);
		Weapon_info[0].subtype = WP_LASER;
		Weapon_info[1].subtype = WP_LASER;
		Weapon_info[2].subtype = WP_MISSILE;
		Weapon_info[3].subtype = WP_MISSILE;

		auto two_primaries = make_ship_class(2, 2);
		for (int weapon_class = 0; weapon_class < 4; weapon_class++) {
			allow_weapon(two_primaries, weapon_class);
		}
		restrict_bank(two_primaries, 0, 0);
		restrict_bank(two_primaries, MAX_SHIP_PRIMARY_BANKS, 2);
		restrict_bank(two_primaries, MAX_SHIP_PRIMARY_BANKS + 1, 3);
		Ship_info.push_back(std::move(two_primaries));

		auto one_primary = make_ship_class(1, 3);
		allow_weapon(one_primary, 0);
		allow_weapon(one_primary, 2);
		allow_weapon(one_primary, 3);
		restrict_bank(one_primary, MAX_SHIP_PRIMARY_BANKS, 2);
		restrict_bank(one_primary, MAX_SHIP_PRIMARY_BANKS + 2, 3);
		Ship_info.push_back(std::move(one_primary));

		auto three_primaries = make_ship_class(3, 1);
		allow_weapon(three_primaries, 0);
		allow_weapon(three_primaries, 2);
		allow_weapon(three_primaries, 3);
		restrict_bank(three_primaries, MAX_SHIP_PRIMARY_BANKS, 2);
		Ship_info.push_back(std::move(three_primaries));

		Ship_info.push_back(make_ship_class(2, 1));

		Ships_inited = true;

		_saved_wss_slots = Wss_slots;
		Wss_slots = Wss_slots_teams[0];
		const int slot_classes[] = {0, 3, 1};
		for (int slot = 0; slot < 3; slot++) {
			Wss_slots[slot].ship_class = slot_classes[slot];
			for (int bank = 0; bank < MAX_SHIP_WEAPONS; bank++) {
				Wss_slots[slot].wep[bank] = -1;
				Wss_slots[slot].wep_count[bank] = -1;
			}
		}
	}

	void TearDown() override
	{
		for (int slot = 0; slot < 3; slot++) {
			Wss_slots_teams[0][slot] = wss_unit();
		}
		Wss_slots = _saved_wss_slots;
		Ships_inited = false;
		Ship_info.clear();
		Weapon_info.clear();

		test::scripting::ScriptingTestFixture::TearDown();
	}
};

TEST_F(ScriptingShipclassTest, isWeaponAllowedOnShip_twoPrimaries)
{
	this->EvalTestScript();
}

TEST_F(ScriptingShipclassTest, isWeaponAllowedOnShip_onePrimary)
{
	this->EvalTestScript();
}

TEST_F(ScriptingShipclassTest, isWeaponAllowedOnShip_threePrimaries)
{
	this->EvalTestScript();
}

TEST_F(ScriptingShipclassTest, isWeaponAllowedOnShip_unrestrictedClass)
{
	this->EvalTestScript();
}

TEST_F(ScriptingShipclassTest, loadoutWeapon_bankRestrictions)
{
	this->EvalTestScript();
}

TEST_F(ScriptingShipclassTest, loadoutWeapon_missingPrimaryBank)
{
	try {
		this->EvalTestScript();
		FAIL() << "Expected a Lua error for a primary bank the class does not have";
	} catch (const os::dialogs::LuaErrorException& e) {
		EXPECT_NE(SCP_string(e.what()).find("The provided bank or weapon index is invalid!"), SCP_string::npos)
			<< e.what();
	}

	EXPECT_EQ(-1, Wss_slots[0].wep[2]);
}

TEST_F(ScriptingShipclassTest, loadoutWeapon_weaponPastEnd)
{
	try {
		this->EvalTestScript();
		FAIL() << "Expected a Lua error for a weapon index one past the end";
	} catch (const os::dialogs::LuaErrorException& e) {
		EXPECT_NE(SCP_string(e.what()).find("The provided bank or weapon index is invalid!"), SCP_string::npos)
			<< e.what();
	}

	EXPECT_EQ(-1, Wss_slots[0].wep[1]);
	EXPECT_EQ(-1, Wss_slots[0].wep_count[1]);
}
