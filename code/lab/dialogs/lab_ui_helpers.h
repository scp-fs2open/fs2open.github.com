#pragma once

#include "ship/ship.h"

SCP_string get_ship_table_text(ship_info* sip);

SCP_string get_weapon_table_text(weapon_info* wip);

SCP_string get_directory_or_vp(const char* path);

bool graphics_options_changed();
