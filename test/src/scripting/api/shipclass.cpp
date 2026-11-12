#include "ship/ship.h"
#include "weapon/weapon.h"

#include "scripting/ScriptingTestFixture.h"

extern bool Ships_inited;

class ScriptingShipclassTest : public test::scripting::ScriptingTestFixture {
  public:
	ScriptingShipclassTest() : test::scripting::ScriptingTestFixture(INIT_CFILE)
	{
		pushModDir("shipclass");
	}

  protected:
	void SetUp() override
	{
		test::scripting::ScriptingTestFixture::SetUp();

		Weapon_info.resize(3);

		ship_info restricted;
		restricted.num_primary_banks = 2;
		restricted.num_secondary_banks = 2;
		restricted.restricted_loadout_flag.resize(MAX_SHIP_WEAPONS);
		restricted.allowed_bank_restricted_weapons.resize(MAX_SHIP_WEAPONS);
		for (int bank = 0; bank < 2; bank++) {
			restricted.restricted_loadout_flag[MAX_SHIP_PRIMARY_BANKS + bank] = REGULAR_WEAPON;
			restricted.allowed_bank_restricted_weapons[MAX_SHIP_PRIMARY_BANKS + bank].set_flag(bank, REGULAR_WEAPON);
			restricted.allowed_weapons.set_flag(bank, REGULAR_WEAPON);
		}
		Ship_info.push_back(std::move(restricted));

		ship_info unrestricted;
		unrestricted.num_primary_banks = 2;
		unrestricted.num_secondary_banks = 1;
		Ship_info.push_back(std::move(unrestricted));

		Ships_inited = true;
	}

	void TearDown() override
	{
		Ships_inited = false;
		Ship_info.clear();
		Weapon_info.clear();

		test::scripting::ScriptingTestFixture::TearDown();
	}
};

TEST_F(ScriptingShipclassTest, isWeaponAllowedOnShip_secondaryBanks)
{
	this->EvalTestScript();
}

TEST_F(ScriptingShipclassTest, isWeaponAllowedOnShip_unrestrictedClass)
{
	this->EvalTestScript();
}
