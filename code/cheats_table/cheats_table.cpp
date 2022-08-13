#include "cheats_table.h"
#include "parse/parselo.h"

void cheat_table_init() {
	customCheats.clear();

	// Add default Spawn Ship Cheat (Volition Bravos).
	// Won't interfere with Total Conversions, since if the ship doesn't exist, the game will just ignore the cheat.
	// The other default cheats do not use the Custom Cheat Framework and are still handled in keycontrol.cpp
	std::unique_ptr<CustomCheat> bravos(new SpawnShipCheat("arrrrwalktheplank", "Walk the plank", false, "Volition Bravos", "Volition Bravos"));
	customCheats.emplace(SCP_string(bravos->cheatCode), std::move(bravos));

	if (cf_exists_full("cheats.tbl", CF_TYPE_TABLES)) {
		parse_cheat_table("cheats.tbl");
	}
	parse_modular_table("*-cht.tbm", parse_cheat_table);
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
			stuff_string(msg, F_RAW);

			if (optional_string("+RequireCheats:")) {
				stuff_boolean(&requireCheats);
			}

			if (optional_string("+SpawnShip:")) {
				shipSpawn = true;
				required_string("+Name:");
				stuff_string(shipName, F_RAW);
				required_string("+Class:");
				stuff_string(shipClass, F_RAW);

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
	char* check_buffer = new char[buffer_length];
	memset (check_buffer, 0, buffer_length * sizeof(char));

	// We construct a checking buffer from the original.
	int idx = 0;
	for (int i = 0; i < buffer_length; i++) {
		if (buffer[i] != '\0') {
			check_buffer[idx] = buffer[i];
			idx++;
		}
	}

	// Loop through our map of custom cheats and check if the code is within the buffer.
	// We don't just check for equality, as the player may have typed other characters before the cheat.
	for (auto &ccheat : customCheats) {
		auto found = std::strstr(check_buffer, ccheat.first.c_str());
		if (found) {
			ccheat.second->runCheat();
			return true;
		}
	}
	return false;
}