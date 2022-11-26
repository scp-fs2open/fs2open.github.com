#include "globalincs/globals.h"
#include "globalincs/pstypes.h"
#include "globalincs/systemvars.h"
#include "io/key.h"
#include "hud/hudmessage.h"
#include "hud/hudsquadmsg.h"
#include "scripting/global_hooks.h"
#include "scripting/hook_api.h"
#include "ship/ship.h"
#include "ship/shipfx.h"

#define CHEAT_BUFFER_LEN	17

class CustomCheat {
	public:
		SCP_string cheatCode;
		SCP_string cheatMsg;
		bool requireCheatsEnabled;

		CustomCheat(SCP_string cheat_code, SCP_string cheat_msg, bool require_cheats_enabled) : 
			cheatCode(std::move(cheat_code)),
			cheatMsg(std::move(cheat_msg)),
			requireCheatsEnabled(require_cheats_enabled) { }

		virtual ~CustomCheat() = default;

		virtual void runCheat() {
			if (canUseCheat()) {
				HUD_printf("%s", cheatMsg.c_str());
				scripting::hooks::OnCheat->run(scripting::hook_param_list(scripting::hook_param("Cheat", 's', cheatCode)));
				CheatUsed = cheatCode;
			}
		}

		bool canUseCheat() {
			return !requireCheatsEnabled || Cheats_enabled;
		}
};

class SpawnShipCheat : public CustomCheat {
	protected:
	SCP_string shipClassName;
	SCP_string shipName;
	public:

	SpawnShipCheat(SCP_string cheat_code, SCP_string cheat_msg,  bool require_cheats_enabled, SCP_string class_name, SCP_string ship_name) : CustomCheat(cheat_code, cheat_msg, require_cheats_enabled) {
		shipClassName = class_name;
		shipName = ship_name;
	}

	void runCheat() override {
		CustomCheat::runCheat();

		if (canUseCheat() && (Game_mode & GM_IN_MISSION) && Player_obj != nullptr) {
			extern void prevent_spawning_collision(object *new_obj);
			ship_subsys *ptr;
			char name[NAME_LENGTH];
			int ship_idx, ship_class; 

			// if not found, then don't create it :(
			ship_class = ship_info_lookup(shipClassName.c_str());
			if (ship_class < 0)
				return;

			vec3d pos = Player_obj->pos;
			matrix orient = Player_obj->orient;
			pos.xyz.x += frand_range(-700.0f, 700.0f);
			pos.xyz.y += frand_range(-700.0f, 700.0f);
			pos.xyz.z += frand_range(-700.0f, 700.0f);

			int objnum = ship_create(&orient, &pos, ship_class);
			if (objnum < 0)
				return;
			
			ship *shipp = &Ships[Objects[objnum].instance];
			shipp->ship_name[0] = '\0';
			shipp->display_name.clear();
			for (size_t j = 0; j < Player_orders.size(); j++)
				shipp->orders_accepted.insert(j);

			// Goober5000 - stolen from support ship creation
			// create a name for the ship with a number. look for collisions until one isn't found anymore
			ship_idx = 1;
			do {
				sprintf(name, "%s %d", shipName.c_str(), ship_idx);
				if ( (ship_name_lookup(name) == -1) && (ship_find_exited_ship_by_name(name) == -1) )
				{
					strcpy_s(shipp->ship_name, name);
					break;
				}

				ship_idx++;
			} while(1);

			shipp->flags.set(Ship::Ship_Flags::Escort);
			shipp->escort_priority = 1000 - ship_idx;

			// now make sure we're not colliding with anyone
			prevent_spawning_collision(&Objects[objnum]);

			// Goober5000 - beam free
			for (ptr = GET_FIRST(&shipp->subsys_list); ptr != END_OF_LIST(&shipp->subsys_list); ptr = GET_NEXT(ptr))
			{
				// mark all turrets as beam free
				if (ptr->system_info->type == SUBSYSTEM_TURRET)
				{
					ptr->weapons.flags.set(Ship::Weapon_Flags::Beam_Free);
					ptr->turret_next_fire_stamp = timestamp((int)frand_range(50.0f, 4000.0f));
				}
			}

			// Cyborg17 to prevent a nullptr...
			ship_set_warp_effects(&Objects[objnum]);
			// warpin
			shipfx_warpin_start(&Objects[objnum]);
		}
	}
};

static SCP_map<SCP_string, std::unique_ptr<CustomCheat>> customCheats;

void cheat_table_init();
void parse_cheat_table(const char* filename);
bool checkForCustomCheats(char converted_buffer[], int buffer_length);