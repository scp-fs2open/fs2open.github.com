//
//

#include "support_rearm_pool.h"

#include "mission/missionparse.h"
#include "scripting/api/objs/weaponclass.h"
#include "weapon/weapon.h"

namespace scripting::api {

//****HANDLE: support_rearm_pool_team
ADE_OBJ(l_SupportRearmPoolTeam, int, "support_rearm_pool_team", "Support rearm pool team handle");

ADE_INDEXER(l_SupportRearmPoolTeam,
	"number/weaponclass IndexOrClass, number amount",
	"Gets/sets support rearm pool value for a weapon class on this team.\n"
	"Values: -1 = unlimited, 0 = unavailable, >0 = limited amount.\n"
	"If a weapon has $Disallow Support Rearm, this always returns 0 and ignores writes.",
	"number",
	"Current pool value for the weapon class.")
{
	int team_idx = -1;
	int idx = -1;
	int amount;
	if (lua_isnumber(L, 2)) {
		if (!ade_get_args(L, "oi|i", l_SupportRearmPoolTeam.Get(&team_idx), &idx, &amount)) {
			return ADE_RETURN_NIL;
		}

		if (idx < 1 || idx > weapon_info_size()) {
			return ADE_RETURN_NIL;
		}

		idx--; // Lua to C++ index
	} else {
		if (!ade_get_args(L, "oo|i", l_SupportRearmPoolTeam.Get(&team_idx), l_Weaponclass.Get(&idx), &amount)) {
			return ADE_RETURN_NIL;
		}

		if (idx < 0 || idx >= weapon_info_size()) {
			return ADE_RETURN_NIL;
		}
	}

	if (team_idx < 0 || team_idx >= Num_teams || team_idx >= MAX_TVT_TEAMS) {
		return ADE_RETURN_NIL;
	}

	auto &team_pool = The_mission.support_ships.rearm_weapon_pool[team_idx];
	int pool_default = The_mission.support_ships.rearm_pool_default();

	if (ADE_SETTING_VAR) {
		// disallow_rearm weapons always read as 0, so there is nothing to store for them
		if (!Weapon_info[idx].disallow_rearm) {
			int new_value = (amount < 0) ? -1 : amount;

			// don't store the default value; absence means the default
			if (new_value == pool_default) {
				team_pool.erase(idx);
			} else {
				team_pool[idx] = new_value;
			}
		}
	}

	if (Weapon_info[idx].disallow_rearm) {
		return ade_set_args(L, "i", 0);
	}

	return ade_set_args(L, "i", team_pool.value_or(idx, pool_default));
}

ADE_FUNC(__len,
	l_SupportRearmPoolTeam,
	nullptr,
	"The number of weapon classes in this support rearm pool.",
	"number",
	"The number of weapon classes.")
{
	return ade_set_args(L, "i", weapon_info_size());
}

} // namespace scripting::api
