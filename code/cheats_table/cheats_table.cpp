#include "cheats_table.h"
#include "parse/parselo.h"

void cheat_table_init() {
	customCheats.clear();
	std::unique_ptr<CustomCheat> bravos(new SpawnShipCheat("arrrrwalktheplank", "Walk the plank", false, "Volition Bravos", "Volition Bravos")); //("Volition Bravos", "Volition Bravos", "Walk the plank", false));
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

			required_string("+Message:");
			stuff_string(msg, F_RAW);

			if (optional_string("+RequireCheats:"))
				stuff_boolean(&requireCheats);

			if (optional_string("+SpawnShip:")) {
				shipSpawn = true;
				required_string("+Name:");
				stuff_string(shipName, F_RAW);
				required_string("+Class:");
				stuff_string(shipClass, F_RAW);

				int shipClassInfo = ship_info_lookup(shipClass.c_str());
				if (shipClassInfo < 0)
				{
					shipSpawn = false;
					Warning(LOCATION, "There is no ship class named '%s'. The '%s' cheat won't summon a ship.", shipClass.c_str(), code.c_str());
				}
			}
			
			if (shipSpawn) {
				std::unique_ptr<CustomCheat> shipCheat(new SpawnShipCheat(code, msg, requireCheats, shipClass, shipName));
				if(customCheats.count(code) == 1)
				{
					Warning(LOCATION, "A cheat for code '%s' already exists. It will be replaced.", code.c_str());
					customCheats[code] = std::move(shipCheat);
				} else {
					customCheats.emplace(code, std::move(shipCheat));
				}
				
			} else {
				std::unique_ptr<CustomCheat> cheat(new CustomCheat(code, msg, requireCheats));
				if(customCheats.count(code) == 1)
				{
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

bool checkForCheats(char buffer[], int buffer_length)
{
	char foo[buffer_length];
	memset (foo, 0, buffer_length * sizeof(char));

	int fooidx = 0;
	for (int i = 0; i < buffer_length; i++)
	{
		if (buffer[i] != '\0')
		{
			foo[fooidx] = buffer[i];
			fooidx++;
		}
	}
	//I'm sorry for the crimes against all stringkind.
	for (auto &ccheat : customCheats)
	{
		auto found = std::strstr(foo, ccheat.first.c_str());
		if (found) {
			ccheat.second->runCheat();
			return true;
		}
	}
	return false;
}