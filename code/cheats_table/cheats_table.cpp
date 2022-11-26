#include "cheats_table.h"
#include "parse/parselo.h"

void cheat_table_init() {
	customCheats.clear();

	if (cf_exists_full("cheats.tbl", CF_TYPE_TABLES)) {
		parse_cheat_table("cheats.tbl");
	}
	parse_modular_table("*-cht.tbm", parse_cheat_table);
}

bool CustomCheat::canUseCheat() {
	return !requireCheatsEnabled || Cheats_enabled;
}

void CustomCheat::runCheat() {
	if (!canUseCheat()) return;
	HUD_printf("%s", cheatMsg.c_str());
	scripting::hooks::OnCheat->run(scripting::hook_param_list(scripting::hook_param("Cheat", 's', cheatCode)));
	CheatUsed = cheatCode;
}

void SpawnShipCheat::runCheat() {
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

void parse_cheat_table(const char* filename) {
	try {
		read_file_text(filename, CF_TYPE_TABLES);
		reset_parse();
		optional_string("#CUSTOM CHEATS");

		while (optional_string("$Cheat:")) {
			SCP_string code, msg, shipClass, shipName;
			bool requireCheats = false;
			bool shipSpawn = false;
			required_string("+Code:");
			stuff_string(code, F_RAW);

			//We are limited to the buffer size used for the traditional cheats. We can still use short cheats, though.
			if (code.length() > CHEAT_BUFFER_LEN) {
				Warning(LOCATION, "Cheat code %s is too long. It will be cut off to maximum length (%i characters).", code.c_str(), CHEAT_BUFFER_LEN);
				code = code.substr(0, CHEAT_BUFFER_LEN);
			}

			required_string("+Message:");
			stuff_string(msg, F_MESSAGE);

			if (optional_string("+RequireCheats:")) {
				stuff_boolean(&requireCheats);
			}

			if (optional_string("+SpawnShip:")) {
				shipSpawn = true;
				required_string("+Name:");
				stuff_string(shipName, F_NAME);
				required_string("+Class:");
				stuff_string(shipClass, F_NAME);

				int shipClassInfo = ship_info_lookup(shipClass.c_str());
				if (shipClassInfo < 0) {
					shipSpawn = false;
					Warning(LOCATION, "There is no ship class named '%s'. The '%s' cheat won't summon a ship.", shipClass.c_str(), code.c_str());
				}
			}
			
			if (shipSpawn) {
				std::unique_ptr<CustomCheat> shipCheat(new SpawnShipCheat(code, msg, requireCheats, shipClass, shipName));

				if(customCheats.count(code) == 1) {
					Warning(LOCATION, "A cheat for code '%s' already exists. It will be replaced.", code.c_str());
					customCheats[code] = std::move(shipCheat);
				} else {
					customCheats.emplace(code, std::move(shipCheat));
				}
			} else {
				std::unique_ptr<CustomCheat> cheat(new CustomCheat(code, msg, requireCheats));

				if(customCheats.count(code) == 1) {
					Warning(LOCATION, "A cheat for code '%s' already exists. It will be replaced.", code.c_str());
					customCheats[code] = std::move(cheat);
				} else {
					customCheats.emplace(code, std::move(cheat));
				}
			}
		}
		required_string("#END");
	}
	catch (const parse::ParseException& e) {
		mprintf(("TABLES: Unable to parse '%s'!  Error message = %s.\n", filename, e.what()));
		return;
	}
}

bool checkForCustomCheats(char buffer[], int buffer_length) {
	const char* check_buffer = buffer;
	for (int i = 0; i < buffer_length; i++) {
		if (*check_buffer == '\0') {
			check_buffer++;
		}
		else {
			break;
		}
	}

	// Loop through our map of custom cheats and check if the code is within the buffer.
	// We don't just check for equality, as the player may have typed other characters before the cheat.
	for (auto &ccheat : customCheats) {
		auto found = std::strstr(check_buffer, ccheat.first.c_str());
		if (found != nullptr) {
			ccheat.second->runCheat();
			return true;
		}
	}
	return false;
}