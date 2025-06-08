#pragma once

#include "asteroid/asteroid.h"
#include "ship/ship.h"

SCP_map<int, SCP_string> get_docking_point_map(int model_index);

SCP_string get_ship_table_text(ship_info* sip);

SCP_string get_weapon_table_text(weapon_info* wip);

SCP_string get_asteroid_table_text(const asteroid_info* aip);

SCP_string get_directory_or_vp(const char* path);

bool graphics_options_changed();
